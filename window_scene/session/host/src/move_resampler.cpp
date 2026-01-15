/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "move_resampler.h"

#include <algorithm>
#include <cmath>

namespace OHOS {
namespace Rosen {
double OneEuroFilter::Filter(int64_t curTimeUs, double curValue)
{
    if (auto initValue = InitializeIfNeeded(curTimeUs, curValue)) {
        return *initValue;
    }

    // Calculate time delta in seconds
    double dt = (curTimeUs - preTimeUs_) / 1e6; // us to s
    // Protect against extremely small time deltas leading to huge noise
    constexpr double MIN_DT = 1e-4;
    if (dt < MIN_DT) {
        dt = MIN_DT;
    }

    const auto& [minCutoff, beta, dCutoff] = filterParam_;

    // Use previously filtered value for derivative estimation to reduce noise
    double rawDeriv = (curValue - preValue_) / dt;

    // First-stage low-pass filter for derivative
    double filteredDeriv = Lowpass(preDeriv_, rawDeriv, Alpha(dCutoff, dt));

    // Adaptive cutoff adjusts smoothing intensity based on motion speed
    double adaptiveCutoff = minCutoff + beta * std::fabs(filteredDeriv);

    // Second-stage low-pass filter for the actual value
    double filteredValue = Lowpass(preValue_, curValue, Alpha(adaptiveCutoff, dt));

    preValue_ = filteredValue;
    preDeriv_ = filteredDeriv;
    preTimeUs_ = curTimeUs;
    return filteredValue;
}

std::optional<double> OneEuroFilter::InitializeIfNeeded(int64_t curTimeUs, double curValue)
{
    const bool needInit = (!initialized_ || curTimeUs <= preTimeUs_);
    if (!needInit) {
        return std::nullopt;
    }

    // If this is the first frame (not yet initialized_) and the caller
    // has provided an explicit initial baseline (useInitialValue_),
    // initialize the internal state using initialValue_. This supports
    // soft-start smoothing at drag begin.
    // For all later resets (initialized_ == true), the filter always
    // initializes from the incoming curValue to avoid unwanted jumps.
    preValue_ = (!initialized_ && useInitialValue_) ? initialValue_ : curValue;

    preTimeUs_ = curTimeUs;
    preDeriv_ = 0.0;
    initialized_ = true;
    return preValue_;
}

double OneEuroFilter::Alpha(double cutoff, double dt) const
{
    constexpr double MIN_VALUE = 1e-6;
    // Clamp values to avoid division by zero and degenerate smoothing
    cutoff = std::max(cutoff, MIN_VALUE);
    dt = std::max(dt, MIN_VALUE);

    const double tau = 1.0 / (2.0 * M_PI * cutoff);
    return 1.0 / (1.0 + tau / dt);
}

double OneEuroFilter::Lowpass(double prev, double curr, double alpha) const
{
    alpha = std::clamp(alpha, 0.0, 1.0);
    return alpha * curr + (1.0 - alpha) * prev;
}

void OneEuroFilter::Reset()
{
    initialized_ = false;
    preTimeUs_ = 0;
    preValue_ = 0.0;
    preDeriv_ = 0.0;

    useInitialValue_ = false;
    initialValue_ = 0.0;
}

void MoveResampler::PushEvent(int64_t timeUs, int32_t posX, int32_t posY)
{
    // Initialize startup phase on first event, using zero initial
    // position instead of first event position to avoid large jumps
    BeginStartup(timeUs, 0.0, 0.0);

    lastRawEvent_ = { timeUs, posX, posY };
    events_.push_back(lastRawEvent_);
    CleanupOldEvents(timeUs);
}

MoveEvent MoveResampler::ResampleAt(int64_t targetTimeUs)
{
    // Apply startup smoothing if in startup phase
    ApplyStartupSmoothing(targetTimeUs);

    auto [rawX, rawY] = ResampleRaw(targetTimeUs);
    auto filteredX = filterX_.Filter(targetTimeUs, rawX);
    auto filteredY = filterY_.Filter(targetTimeUs, rawY);
    return {
        .timeUs = targetTimeUs,
        .posX = static_cast<int32_t>(std::round(filteredX)),
        .posY = static_cast<int32_t>(std::round(filteredY))
    };
}

void MoveResampler::Reset()
{
    events_.clear();
    lastRawEvent_ = {};
    filterX_.Reset();
    filterY_.Reset();

    startupInitialized_ = false;
    startupPhase_ = false;
    startupTimeUs_ = 0;
}

void MoveResampler::BeginStartup(int64_t startupTimeUs, double initialX, double initialY)
{
    if (startupInitialized_) {
        return;
    }

    startupInitialized_ = true;
    startupPhase_ = true;
    startupTimeUs_ = startupTimeUs;
    filterX_.SetInitialValue(initialX);
    filterY_.SetInitialValue(initialY);
}

void MoveResampler::ApplyStartupSmoothing(int64_t targetTimeUs)
{
    if (!startupPhase_) {
        return;
    }

    int64_t dt = targetTimeUs - startupTimeUs_;
    if (dt < startupDurationUs_) {
        double t = static_cast<double>(dt) / startupDurationUs_;

        // Blend only the minCutoff parameter from "very smooth" → "normal smooth".
        // beta and dCutoff remain unchanged to preserve derivative behavior.
        FilterParam blended;
        blended.minCutoff = startupParam_.minCutoff + (normalParam_.minCutoff - startupParam_.minCutoff) * t;
        blended.beta = normalParam_.beta;
        blended.dCutoff = normalParam_.dCutoff;

        filterX_.SetParam(blended);
        filterY_.SetParam(blended);
    } else {
        // Startup finished → use normal filter params
        startupPhase_ = false;
        filterX_.SetParam(normalParam_);
        filterY_.SetParam(normalParam_);
    }
}

std::pair<double, double> MoveResampler::ResampleRaw(int64_t targetTimeUs)
{
    // Clear old events first
    CleanupOldEvents(targetTimeUs);

    if (events_.empty()) {
        return { static_cast<double>(lastRawEvent_.posX), static_cast<double>(lastRawEvent_.posY) };
    }

    // Only one event available → no sampling, direct value return;
    // Or if target time is older than the earliest event → clamp to first event.
    if (events_.size() == 1 || targetTimeUs <= events_.front().timeUs) {
        const auto& e = events_.front();
        return { static_cast<double>(e.posX), static_cast<double>(e.posY) };
    }

    // If target time is inside the recorded range → linear interpolation.
    if (targetTimeUs < events_.back().timeUs) {
        return InterpolateLinear(targetTimeUs);
    }

    // Otherwise: target time is newer than the latest event → extrapolation or fitting.
    return ExtrapolateFit(targetTimeUs);
}

size_t MoveResampler::FindSegmentIndex(int64_t targetTimeUs) const
{
    size_t left = 1;
    size_t right = events_.size() - 1;

    while (left < right) {
        size_t mid = (left + right) / 2;
        if (events_[mid].timeUs < targetTimeUs) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

std::pair<double, double> MoveResampler::LinearFitAt(size_t startIdx, size_t endIdx, int64_t targetTimeUs) const
{
    if (events_.empty()) {
        return { static_cast<double>(lastRawEvent_.posX), static_cast<double>(lastRawEvent_.posY) };
    }

    if (startIdx > endIdx || endIdx >= events_.size()) {
        const auto& e = events_.back();
        return {static_cast<double>(e.posX), static_cast<double>(e.posY)};
    }

    size_t n = endIdx - startIdx + 1;

    // First pass: compute means
    double sumT = 0.0;
    double sumX = 0.0;
    double sumY = 0.0;

    for (size_t i = startIdx; i <= endIdx; ++i) {
        const auto& e = events_[i];
        sumT += static_cast<double>(e.timeUs);
        sumX += static_cast<double>(e.posX);
        sumY += static_cast<double>(e.posY);
    }

    double meanT = sumT / n;
    double meanX = sumX / n;
    double meanY = sumY / n;

    // Second pass: least-squares mean-centered
    double sumTT = 0.0;  // Σ (t^2)
    double sumTdx = 0.0; // Σ (t * (x - meanX))
    double sumTdy = 0.0; // Σ (t * (y - meanY))

    for (size_t i = startIdx; i <= endIdx; ++i) {
        const auto& e = events_[i];
        double t = static_cast<double>(e.timeUs) - meanT;
        double dx = static_cast<double>(e.posX) - meanX;
        double dy = static_cast<double>(e.posY) - meanY;

        sumTT += t * t;
        sumTdx += t * dx;
        sumTdy += t * dy;
    }

    constexpr double EPS = 1e-12;
    double slopeX = 0.0;
    double slopeY = 0.0;

    if (std::abs(sumTT) >= EPS) {
        slopeX = sumTdx / sumTT;
        slopeY = sumTdy / sumTT;
    }

    // Evaluate at target time (mean-centered)
    double tQuery = static_cast<double>(targetTimeUs) - meanT;
    double x = meanX + slopeX * tQuery;
    double y = meanY + slopeY * tQuery;
    return { x, y };
}

std::pair<double, double> MoveResampler::InterpolateLinear(int64_t targetTimeUs) const
{
    if (events_.empty()) {
        return { static_cast<double>(lastRawEvent_.posX), static_cast<double>(lastRawEvent_.posY) };
    }

    if (events_.size() == 1) {
        const auto& e = events_.back();
        return { static_cast<double>(e.posX), static_cast<double>(e.posY) };
    }

    size_t idx = FindSegmentIndex(targetTimeUs);
    const MoveEvent& prev = events_[idx - 1];
    const MoveEvent& next = events_[idx];

    double span = static_cast<double>(next.timeUs - prev.timeUs);
    if (span <= 0.0) {
        return { static_cast<double>(next.posX), static_cast<double>(next.posY) };
    }

    // Smooth prev using LinearFitAt
    size_t prevStart = (idx > 4) ? (idx - 4) : 0; // 4: using up to last 4 points
    size_t prevEnd = idx - 1;

    auto [prevSmoothX, prevSmoothY] = LinearFitAt(prevStart, prevEnd, prev.timeUs);

    // Smooth next using LinearFitAt
    size_t nextStart = (idx > 3) ? (idx - 3) : 0; // 3: using up to last 3 points
    size_t nextEnd = idx;

    auto [nextSmoothX, nextSmoothY] = LinearFitAt(nextStart, nextEnd, next.timeUs);

    // Standard linear interpolation
    double ratio = (static_cast<double>(targetTimeUs - prev.timeUs) / span);
    double x = prevSmoothX + ratio * (nextSmoothX - prevSmoothX);
    double y = prevSmoothY + ratio * (nextSmoothY - prevSmoothY);
    return { x, y };
}

std::pair<double, double> MoveResampler::ExtrapolateFit(int64_t targetTimeUs) const
{
    if (events_.empty()) {
        return { static_cast<double>(lastRawEvent_.posX), static_cast<double>(lastRawEvent_.posY) };
    }
    size_t nEvents = events_.size();
    size_t startIdx = (nEvents > 5) ? (nEvents - 5) : 0; // 5: using up to last 5 points
    size_t endIdx = nEvents - 1;
    return LinearFitAt(startIdx, endIdx, targetTimeUs);
}

void MoveResampler::CleanupOldEvents(int64_t currentTimeUs)
{
    const int64_t threshold = currentTimeUs - maxEventIntervalUs_;
    while (!events_.empty() && events_.front().timeUs < threshold) {
        events_.pop_front();
    }
}
} // namespace Rosen
} // namespace OHOS
