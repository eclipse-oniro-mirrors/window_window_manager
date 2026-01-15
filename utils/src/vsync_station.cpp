/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "vsync_station.h"

#include <functional>

#include <hitrace_meter.h>
#include <transaction/rs_interfaces.h>
#include <ui/rs_display_node.h>
#include <ui/rs_ui_context.h>
#include <feature/hyper_graphic_manager/rs_ui_display_soloist.h>

#include "window_frame_trace.h"
#include "window_manager_hilog.h"
#include <vsync_receiver.h>

using namespace FRAME_TRACE;

namespace OHOS {
namespace Rosen {
namespace {
const std::string VSYNC_THREAD_ID = "VsyncThread";
const std::string VSYNC_TIME_OUT_TASK = "vsync_time_out_task_";
constexpr int64_t VSYNC_TIME_OUT_MILLISECONDS = 600;
constexpr int32_t DEFAULT_ANIMATOR_EXPECTED_FRAME_RATE = -1;
}

VsyncStation::VsyncStation(NodeId nodeId, const std::shared_ptr<AppExecFwk::EventHandler>& vsyncHandler)
    : nodeId_(nodeId),
      vsyncHandler_(vsyncHandler),
      vsyncTimeoutTaskName_(VSYNC_TIME_OUT_TASK + std::to_string(nodeId)),
      frameRateLinker_(RSFrameRateLinker::Create())
{
    if (!vsyncHandler_) {
        auto mainEventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
        if (mainEventRunner != nullptr) {
            vsyncHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainEventRunner);
        } else {
            // LCOV_EXCL_START
            TLOGW(WmsLogTag::WMS_MAIN, "MainEventRunner is not available");
            vsyncHandler_ = std::make_shared<AppExecFwk::EventHandler>(
                AppExecFwk::EventRunner::Create(VSYNC_THREAD_ID));
            // LCOV_EXCL_STOP
        }
    }
    TLOGI(WmsLogTag::WMS_MAIN, "id %{public}" PRIu64 " created", nodeId_);
}

VsyncStation::~VsyncStation()
{
    TLOGI(WmsLogTag::WMS_MAIN, "id %{public}" PRIu64 " destructed", nodeId_);
}

// LCOV_EXCL_START
void VsyncStation::Destroy()
{
    TLOGI(WmsLogTag::WMS_MAIN, "id %{public}" PRIu64 " destroyed", nodeId_);
    std::lock_guard<std::mutex> lock(mutex_);
    destroyed_ = true;
    receiver_.reset();
    frameRateLinker_.reset();
}
// LCOV_EXCL_STOP

bool VsyncStation::IsVsyncReceiverCreated()
{
    return GetOrCreateVsyncReceiver() != nullptr;
}

std::shared_ptr<VSyncReceiver> VsyncStation::GetOrCreateVsyncReceiver()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetOrCreateVsyncReceiverLocked();
}

std::shared_ptr<VSyncReceiver> VsyncStation::GetOrCreateVsyncReceiverLocked()
{
    if (destroyed_) {
        TLOGW(WmsLogTag::WMS_MAIN, "VsyncStation has been destroyed");
        return nullptr;
    }
    if (receiver_ == nullptr) {
        auto& rsClient = RSInterfaces::GetInstance();
        auto receiver = rsClient.CreateVSyncReceiver("WM_" + std::to_string(::getprocpid()), frameRateLinker_->GetId(),
            vsyncHandler_, nodeId_);
        if (receiver == nullptr) {
            TLOGE(WmsLogTag::WMS_MAIN, "Fail to create vsync receiver, nodeId: %{public}" PRIu64, nodeId_);
            return nullptr;
        }
        auto result = receiver->Init();
        if (result == VSYNC_ERROR_OK) {
            receiver_ = std::move(receiver);
        } else {
            TLOGE(WmsLogTag::WMS_MAIN, "Fail to init vsync receiver, nodeId: %{public}" PRIu64
                ", error %{public}d", nodeId_, static_cast<int>(result));
        }
    }
    return receiver_;
}

// LCOV_EXCL_START
__attribute__((no_sanitize("cfi"))) void VsyncStation::RequestVsync(
    const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    std::shared_ptr<VSyncReceiver> receiver;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // check if receiver is ready
        receiver = GetOrCreateVsyncReceiverLocked();
        if (receiver == nullptr) {
            return;
        }

        vsyncCallbacks_.insert(vsyncCallback);

        // if Vsync has been requested, just wait callback or timeout
        if (hasRequestedVsync_) {
            TLOGD(WmsLogTag::WMS_MAIN, "Vsync has requested, nodeId: %{public}" PRIu64, nodeId_);
            return;
        }
        hasRequestedVsync_ = true;

        if (isFirstVsyncRequest_) {
            isFirstVsyncRequest_ = false;
            TLOGI(WmsLogTag::WMS_MAIN, "First vsync has requested, nodeId: %{public}" PRIu64, nodeId_);
        }

        // post timeout task for a new vsync
        vsyncHandler_->RemoveTask(vsyncTimeoutTaskName_);
        auto task = [weakThis = weak_from_this()] {
            if (auto sp = weakThis.lock()) {
                sp->OnVsyncTimeOut();
            }
        };
        vsyncHandler_->PostTask(task, vsyncTimeoutTaskName_, VSYNC_TIME_OUT_MILLISECONDS);
    }

    requestVsyncTimes_++;
    WindowFrameTraceImpl::GetInstance()->VsyncStartFrameTrace();
    auto task = [weakThis = weak_from_this()]
        (int64_t timestamp, int64_t frameCount, void* client) {
        if (auto sp = weakThis.lock()) {
            sp->VsyncCallbackInner(timestamp, frameCount);
            WindowFrameTraceImpl::GetInstance()->VsyncStopFrameTrace();
        }
    };
    receiver->RequestNextVSync({
        .userData_ = nullptr, .callbackWithId_ = std::move(task),
    });
}

void VsyncStation::RunOnceOnNextVsync(OnCallback&& callback)
{
    auto vsyncCallback = std::make_shared<VsyncCallback>();
    vsyncCallback->onCallback = std::move(callback);
    RequestVsync(vsyncCallback);
}

void VsyncStation::RunOnceAfterNVsyncs(uint32_t delayVsyncCount, OnCallback&& callback)
{
    if (delayVsyncCount == 0) { // 0: invalid delay (must delay at least 1 vsync)
        TLOGE(WmsLogTag::DEFAULT, "delayVsyncCount must be >= 1");
        return;
    }
    if (delayVsyncCount == 1) { // 1: just run on next vsync
        RunOnceOnNextVsync(std::move(callback));
        return;
    }

    auto vsyncCallback = std::make_shared<VsyncCallback>();
    std::weak_ptr<VsyncCallback> weakVsyncCallback = vsyncCallback;
    auto remaining = std::make_shared<uint32_t>(delayVsyncCount);
    vsyncCallback->onCallback = [weakThis = weak_from_this(),
                                 callback = std::move(callback),
                                 weakVsyncCallback,
                                 remaining](int64_t timestamp, int64_t frameCount) mutable {
        if (--(*remaining) == 0) {
            callback(timestamp, frameCount);
            return;
        }
        // Re-register the callback for the next vsync
        auto vsyncStation = weakThis.lock();
        auto vsyncCallback = weakVsyncCallback.lock();
        if (vsyncStation && vsyncCallback) {
            vsyncStation->RequestVsync(vsyncCallback);
        }
    };
    RequestVsync(vsyncCallback);
}

int64_t VsyncStation::GetVSyncPeriod()
{
    int64_t period = 0;
    if (auto receiver = GetOrCreateVsyncReceiver()) {
        receiver->GetVSyncPeriod(period);
    }
    return period;
}

std::optional<uint32_t> VsyncStation::GetFps()
{
    int64_t periodNs = GetVSyncPeriod();
    if (periodNs <= 0) {
        return std::nullopt;
    }

    constexpr int64_t NS_PER_SEC = 1'000'000'000;
    return static_cast<uint32_t>((NS_PER_SEC + periodNs / 2) / periodNs); // 2: round to nearest FPS.
}

void VsyncStation::RemoveCallback()
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    std::lock_guard<std::mutex> lock(mutex_);
    vsyncCallbacks_.clear();
}

void VsyncStation::VsyncCallbackInner(int64_t timestamp, int64_t frameCount)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "OnVsyncCallback %" PRId64 ":%" PRId64, timestamp, frameCount);
    Callbacks vsyncCallbacks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        hasRequestedVsync_ = false;
        vsyncCallbacks = std::move(vsyncCallbacks_);
        vsyncCallbacks_.clear();
        vsyncHandler_->RemoveTask(vsyncTimeoutTaskName_);
        if (isFirstVsyncBack_) {
            isFirstVsyncBack_ = false;
            TLOGI(WmsLogTag::WMS_MAIN, "First vsync has come back, nodeId: %{public}" PRIu64, nodeId_);
        }
    }
    for (const auto& callback: vsyncCallbacks) {
        if (callback && callback->onCallback) {
            callback->onCallback(timestamp, frameCount);
        }
    }
}

void VsyncStation::OnVsyncTimeOut()
{
    TLOGW(WmsLogTag::WMS_MAIN, "in");
    std::lock_guard<std::mutex> lock(mutex_);
    hasRequestedVsync_ = false;
}

std::shared_ptr<RSFrameRateLinker> VsyncStation::GetFrameRateLinker()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetFrameRateLinkerLocked();
}

std::shared_ptr<RSFrameRateLinker> VsyncStation::GetFrameRateLinkerLocked()
{
    if (destroyed_) {
        TLOGW(WmsLogTag::WMS_MAIN, "VsyncStation has been destroyed");
        return nullptr;
    }
    return frameRateLinker_;
}

FrameRateLinkerId VsyncStation::GetFrameRateLinkerId()
{
    if (auto frameRateLinker = GetFrameRateLinker()) {
        return frameRateLinker->GetId();
    }
    return 0;
}

void VsyncStation::FlushFrameRate(const std::shared_ptr<RSUIContext>& rsUIContext, uint32_t rate,
    int32_t animatorExpectedFrameRate, uint32_t rateType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto frameRateLinker = GetFrameRateLinkerLocked()) {
        if (lastFrameRateRange_ == nullptr) {
            lastFrameRateRange_ = std::make_shared<FrameRateRange>(0, RANGE_MAX_REFRESHRATE, rate, rateType);
        } else {
            lastFrameRateRange_->Set(0, RANGE_MAX_REFRESHRATE, rate, rateType);
        }
        lastAnimatorExpectedFrameRate_ = animatorExpectedFrameRate;
        if (frameRateLinker->IsEnable()) {
            TLOGD(WmsLogTag::WMS_MAIN, "rate %{public}d, linkerId %{public}" PRIu64, rate, frameRateLinker->GetId());
            frameRateLinker->UpdateFrameRateRange(*lastFrameRateRange_, lastAnimatorExpectedFrameRate_, rsUIContext);
        }
    }
}

void VsyncStation::SetFrameRateLinkerEnable(const std::shared_ptr<RSUIContext>& rsUIContext, bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto frameRateLinker = GetFrameRateLinkerLocked()) {
        if (!enabled) {
            // clear frameRate vote
            FrameRateRange range = {0, RANGE_MAX_REFRESHRATE, 0};
            TLOGI(WmsLogTag::WMS_MAIN, "rate %{public}d, linkerId %{public}" PRIu64,
                range.preferred_, frameRateLinker->GetId());
            frameRateLinker->UpdateFrameRateRange(range, DEFAULT_ANIMATOR_EXPECTED_FRAME_RATE, rsUIContext);
            frameRateLinker->UpdateFrameRateRangeImme(range);
        } else if (lastFrameRateRange_) {
            // to resolve these cases:
            // case 1: when app go backGround and haven't cleared the vote itself, the vote will be invalid forever,
            //         so we restore the vote which is cleared here.
            // case 2: when frameRateLinker is disabled, the frameRate vote by app will be delayed until linker enable.
            frameRateLinker->UpdateFrameRateRange(*lastFrameRateRange_, lastAnimatorExpectedFrameRate_, rsUIContext);
        }
        frameRateLinker->SetEnable(enabled);
    }
}

void VsyncStation::SetDisplaySoloistFrameRateLinkerEnable(bool enabled)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (destroyed_) {
            TLOGW(WmsLogTag::WMS_MAIN, "VsyncStation has been destroyed");
            return;
        }
    }
    RSDisplaySoloistManager& soloistManager = RSDisplaySoloistManager::GetInstance();
    soloistManager.SetMainFrameRateLinkerEnable(enabled);
}

void VsyncStation::SetUiDvsyncSwitch(bool dvsyncSwitch)
{
    if (auto receiver = GetOrCreateVsyncReceiver()) {
        receiver->SetUiDvsyncSwitch(dvsyncSwitch);
    }
}

void VsyncStation::SetTouchEvent(int32_t touchType)
{
    if (auto receiver = GetOrCreateVsyncReceiver()) {
        receiver->SetTouchEvent(touchType);
    }
}

void VsyncStation::DecreaseRequestVsyncTimes()
{
    int32_t current = 0;
    int32_t desired = 0;
    do {
        current = requestVsyncTimes_.load();
        if (current == 0) {
            break;
        }
        desired = current - 1;
    } while (!requestVsyncTimes_.compare_exchange_weak(current, desired));
}
// LCOV_EXCL_STOP

} // namespace Rosen
} // namespace OHOS
