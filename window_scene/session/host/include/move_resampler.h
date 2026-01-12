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

#ifndef OHOS_ROSEN_WINDOW_SCENE_MOVE_RESAMPLER_H
#define OHOS_ROSEN_WINDOW_SCENE_MOVE_RESAMPLER_H

#include <cstdint>
#include <deque>
#include <optional>
#include <sstream>
#include <string>

namespace OHOS {
namespace Rosen {
/**
 * @brief Represents a move event with timestamp and position.
 */
struct MoveEvent {
    int64_t timeUs = 0;
    int32_t posX = 0;
    int32_t posY = 0;

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << timeUs << ": [" << posX << ", " << posY << "]";
        return oss.str();
    }
};

/**
 * @brief Parameters for One Euro Filter.
 */
struct FilterParam {
    /**
     * @brief Baseline cutoff value for the low-pass filter.
     *
     * Represents the minimum smoothing strength. Larger values reduce smoothing
     * and smaller values increase smoothing.
     */
    double minCutoff;

    /**
     * @brief Speed coefficient controlling how much the cutoff increases with motion.
     *
     * Higher beta makes the filter more responsive to fast motion.
     */
    double beta;

    /**
     * @brief Cutoff used for smoothing the derivative term.
     *
     * This controls how aggressively the derivative (dx/dt) is low-pass filtered
     * before being used for adaptive adjustment.
     */
    double dCutoff;
};

/**
 * @brief Recommended One Euro Filter parameters
 */
constexpr FilterParam DEFAULT_FILTER_PARAM = { 0.8, 0.005, 1.0 };

/**
 * @brief One Euro Filter parameters for startup phase (smoother but more laggy).
 */
constexpr FilterParam STARTUP_FILTER_PARAM = { 0.1, 0.005, 1.0 };

/**
 * @brief One Euro Filter implementation for smoothing input signals.
 */
class OneEuroFilter {
public:
    /**
     * @brief Constructs a OneEuroFilter with specified parameters.
     *
     * @param filterParam Parameters for the filter.
     */
    explicit OneEuroFilter(const FilterParam& filterParam = DEFAULT_FILTER_PARAM) : filterParam_(filterParam) {}

    /**
     * @brief Applies the One Euro Filter to the input value.
     *        The filter adapts its smoothing rate based on the derivative magnitude.
     *
     * @param curTimeUs Current timestamp in microseconds.
     * @param curValue Current input value to be filtered.
     * @return The filtered output value.
     */
    double Filter(int64_t curTimeUs, double curValue);

    /**
     * @brief Sets new filter parameters.
     *
     * @param param New parameters for the filter.
     */
    void SetParam(const FilterParam& param)
    {
        filterParam_ = param;
    }

    /**
     * @brief Sets an initial value for the filter.
     *
     * @param initialValue The initial value to use on the first input.
     */
    void SetInitialValue(double initialValue)
    {
        useInitialValue_ = true;
        initialValue_ = initialValue;
    }

    /**
     * @brief Resets the filter state.
     */
    void Reset();

private:
    /**
     * @brief Initializes the filter state if required for the current sample.
     *
     * @return std::optional<double>
     *         Contains the initialized value if initialization occurred,
     *         otherwise std::nullopt.
     */
    std::optional<double> InitializeIfNeeded(int64_t curTimeUs, double curValue);

    /**
     * @brief Computes the smoothing factor α for a given cutoff frequency.
     *
     * @param cutoff Cutoff frequency.
     * @param dt Time difference since last update.
     * @return The smoothing factor α.
     */
    double Alpha(double cutoff, double dt) const;

    /**
     * @brief Applies a first-order low-pass filter.
     *
     * @param prev Previous filtered value.
     * @param curr Current input value.
     * @param alpha Smoothing factor.
     * @return The filtered output value.
     */
    double Lowpass(double prev, double curr, double alpha) const;

    FilterParam filterParam_;
    bool initialized_ = false;
    int64_t preTimeUs_ = 0;
    double preValue_ = 0.0;
    double preDeriv_ = 0.0;

    /**
     * @brief Indicates whether an explicit initial filter value is provided.
     *
     * When true, the filter’s internal state (preValue_) will be initialized
     * using initialValue_ on the first frame. When false, the first frame
     * initializes preValue_ using the incoming curValue.
     *
     * This mechanism allows the filter to start from a controlled baseline
     * instead of the raw input, enabling soft-start smoothing during drag
     * initialization.
     */
    bool useInitialValue_ = false;

    /**
     * @brief Explicit starting value used for initializing the internal filter state.
     *
     * On the first frame (or after a reset), the filter returns this value directly
     * if useInitialValue_ is true. This forces the smoothing process to begin from
     * a predefined baseline and prevents an abrupt jump from the first input sample.
     *
     * Subsequent frames will transition smoothly from this baseline toward the
     * actual incoming values according to One Euro filtering rules.
     */
    double initialValue_ = 0.0;
};

/**
 * @brief Default maximum event retention time in microseconds.
 */
constexpr int64_t DEFAULT_MAX_EVENT_INTERVAL_US = 60'000; // 60ms

/**
 * @brief Duration of startup smoothing (microseconds).
 */
constexpr int64_t STARTUP_DURATION_US = 120'000; // 120ms

/**
 * @brief Move resampler that buffers input events and produces smooth positions
 *        at requested timestamps via interpolation, extrapolation, and filtering.
 *
 * The resampler can generate positions for timestamps within or near the range
 * of buffered events, depending on available data.
 */
class MoveResampler {
public:
    /**
     * @brief Creates a MoveResampler with the given event retention time and filter parameters.
     *
     * @param maxEventIntervalUs Maximum age of events to keep, in microseconds.
     * @param startupDurationUs Duration of the startup phase, in microseconds.
     * @param startupParam Filter parameters to use during startup phase.
     * @param normalParam Filter parameters to use during normal operation.
     */
    explicit MoveResampler(int64_t maxEventIntervalUs = DEFAULT_MAX_EVENT_INTERVAL_US,
                           int64_t startupDurationUs = STARTUP_DURATION_US,
                           const FilterParam& startupParam = STARTUP_FILTER_PARAM,
                           const FilterParam& normalParam = DEFAULT_FILTER_PARAM)
        : maxEventIntervalUs_(maxEventIntervalUs),
          startupDurationUs_(startupDurationUs),
          startupParam_(startupParam),
          normalParam_(normalParam) {}

    /**
     * @brief Pushs a new input event to the buffer and discards outdated events.
     *
     * @param timeUs Timestamp of the event in microseconds.
     * @param posX X coordinate of the event.
     * @param posY Y coordinate of the event.
     */
    void PushEvent(int64_t timeUs, int32_t posX, int32_t posY);

    /**
     * @brief Returns a smoothed position at the given timestamp.
     *        Internally performs interpolation or extrapolation, then applies filtering.
     *
     * @param targetTimeUs The target timestamp to sample at (in microseconds).
     * @return The resampled MoveEvent at the target time.
     */
    MoveEvent ResampleAt(int64_t targetTimeUs);

    /**
     * @brief Resets the resampler state, clearing all buffered events and filter states.
     */
    void Reset();

private:
    /**
     * @brief Begins the startup phase for stronger smoothing.
     *
     * @param startupTimeUs The timestamp when startup begins (in microseconds).
     * @param initialX The initial X position for the filter.
     * @param initialY The initial Y position for the filter.
     */
    void BeginStartup(int64_t startupTimeUs, double initialX, double initialY);

    /**
     * @brief Gradually transitions filter parameters from the startup profile
     *        to the normal profile based on elapsed time since startup began.
     *
     * This method is called on every resampling request during the startup
     * phase. As time progresses, the filter becomes progressively less smooth
     * and more responsive, avoiding sudden jumps at the beginning of a move.
     *
     * @param targetTimeUs The target timestamp to sample at (in microseconds).
     */
    void ApplyStartupSmoothing(int64_t targetTimeUs);

    /**
     * @brief Computes an unfiltered sample at the given timestamp using
     *        interpolation or extrapolation.
     *
     * @param targetTimeUs The target timestamp to sample at (in microseconds).
     * @return A pair of (posX, posY) representing the raw resampled position.
     */
    std::pair<double, double> ResampleRaw(int64_t targetTimeUs);

    /**
     * @brief Finds the index of the first event with timestamp >= targetTimeUs.
     */
    size_t FindSegmentIndex(int64_t targetTimeUs) const;

    /**
     * @brief Performs linear regression over the given event range and evaluates
     *        the fitted line at targetTimeUs.
     *
     * @param startIdx     Start index of the event range (inclusive).
     * @param endIdx       End index of the event range (inclusive).
     * @param targetTimeUs The timestamp to evaluate the fitted line at (in microseconds).
     * @return A pair of (posX, posY) representing the fitted position.
     */
    std::pair<double, double> LinearFitAt(size_t startIdx, size_t endIdx, int64_t targetTimeUs) const;

    /**
     * @brief Computes an interpolated position at targetTimeUs using nearby events.
     *
     * @param targetTimeUs The timestamp to interpolate at (in microseconds).
     * @return A pair of (posX, posY) representing the interpolated position.
     */
    std::pair<double, double> InterpolateLinear(int64_t targetTimeUs) const;

    /**
     * @brief Computes an extrapolated position at targetTimeUs using linear fitting.
     *
     * @param targetTimeUs The timestamp to extrapolate at (in microseconds).
     * @return A pair of (posX, posY) representing the extrapolated position.
     */
    std::pair<double, double> ExtrapolateFit(int64_t targetTimeUs) const;

    /**
     * @brief Removes events older than (currentTimeUs - maxEventIntervalUs_).
     *
     * @param currentTimeUs The current timestamp in microseconds.
     */
    void CleanupOldEvents(int64_t currentTimeUs);

    /**
     * @brief Last raw pointer event (non-resampled and unfiltered).
     *
     * When the pointer stops moving but remains down, maybe no new events
     * will arrive, and the buffered events would be cleared by time pruning.
     * This stored raw event preserves the true last position to avoid jumps
     * during resampling. Using the raw value prevents recursive filtering
     * and keeps the state stable during idle periods.
     */
    MoveEvent lastRawEvent_;

    /**
     * @brief Buffered raw move events used for interpolation and fitting.
     * Cleared by time pruning; may become empty during idle periods.
     */
    std::deque<MoveEvent> events_;

    /**
     * @brief Maximum time window (in microseconds) for retaining events_.
     * Older events are pruned to keep the history fresh and stable.
     */
    int64_t maxEventIntervalUs_;

    /**
     * @brief Indicates whether the startup phase has been activated.
     *
     * This flag prevents BeginStartup() from being invoked multiple times
     * during a single move sequence.
     */
    bool startupInitialized_ = false;

    /**
     * @brief Indicates whether the resampler is actively in the startup phase.
     *
     * During this phase, filter parameters gradually transition from
     * a heavily-smoothed profile to the normal profile.
     */
    bool startupPhase_ = false;

    /**
     * @brief Timestamp when startup phase began.
     */
    int64_t startupTimeUs_ = 0;

    /**
     * @brief Duration (in microseconds) of the startup smoothing phase.
     *
     * A value of zero disables the startup phase entirely:
     * no transitional smoothing will be applied, and the filter
     * will immediately use the normal filter parameters.
     */
    int64_t startupDurationUs_ = 0;

    /**
     * @brief Filter parameters used for the initial smoothing during startup.
     */
    FilterParam startupParam_;

    /**
     * @brief Steady-state filter parameters used after startup completes.
     */
    FilterParam normalParam_;

    /**
     * @brief One Euro filter for smoothing X coordinates during resampling.
     */
    OneEuroFilter filterX_;

    /**
     * @brief One Euro filter for smoothing Y coordinates during resampling.
     */
    OneEuroFilter filterY_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_SCENE_MOVE_RESAMPLER_H
