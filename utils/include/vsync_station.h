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

#ifndef OHOS_VSYNC_STATION_H
#define OHOS_VSYNC_STATION_H

#include <atomic>
#include <memory>
#include <optional>
#include <unordered_set>

#include <event_handler.h>

#include "wm_common.h"

namespace OHOS {
namespace Rosen {
class RSFrameRateLinker;
class RSUIContext;
class VSyncReceiver;
class FrameRateRange;

using FrameRateLinkerId = uint64_t;
using NodeId = uint64_t;

class VsyncStation : public std::enable_shared_from_this<VsyncStation> {
public:
    explicit VsyncStation(NodeId nodeId,
        const std::shared_ptr<AppExecFwk::EventHandler>& vsyncHandler = nullptr);
    virtual ~VsyncStation();

    bool HasRequestedVsync() const { return hasRequestedVsync_; }
    bool IsVsyncReceiverCreated();

    /**
     * @brief Request a one-shot vsync for the given callback.
     *
     * Registers the callback and triggers a vsync request if none is currently
     * pending. Multiple callbacks may be coalesced into a single vsync request.
     *
     * @param vsyncCallback Callback to be invoked on the next vsync.
     */
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback);

    /**
     * @brief Run a callback once on the next vsync.
     *
     * Schedules the given callback to be executed exactly once when the next
     * vsync occurs. This is a convenience wrapper that internally creates a
     * VsyncCallback and requests a one-shot vsync.
     *
     * @param callback Callback to be executed on the next vsync.
     */
    void RunOnceOnNextVsync(OnCallback&& callback);

    /**
     * @brief Run a callback once after a specified number of vsyncs.
     *
     * Schedules the given callback to be executed exactly once after the specified
     * number of upcoming vsyncs have occurred, relative to the time this method
     * is called.
     *
     * For example, a delayVsyncCount of 1 is equivalent to executing the callback
     * on the next vsync.
     *
     * @param delayVsyncCount Number of vsyncs to wait before executing the callback.
     * @param callback        Callback to be executed after the specified vsync delay.
     */
    void RunOnceAfterNVsyncs(uint32_t delayVsyncCount, OnCallback&& callback);

    /**
     * @brief Get the vsync period.
     *
     * @return Vsync period in nanoseconds (ns), or 0 if unavailable.
     */
    virtual int64_t GetVSyncPeriod();

    /**
     * @brief Get the current display refresh rate in FPS.
     *
     * The FPS is derived from the vsync period. Returns std::nullopt
     * if vsync timing information is unavailable.
     *
     * @return Optional FPS value.
     */
    virtual std::optional<uint32_t> GetFps();

    void RemoveCallback();
    void Destroy();

    FrameRateLinkerId GetFrameRateLinkerId();
    void FlushFrameRate(const std::shared_ptr<RSUIContext>& rsUIContext, uint32_t rate,
        int32_t animatorExpectedFrameRate, uint32_t rateType = 0);
    void SetFrameRateLinkerEnable(const std::shared_ptr<RSUIContext>& rsUIContext, bool enabled);
    void SetDisplaySoloistFrameRateLinkerEnable(bool enabled);
    void SetUiDvsyncSwitch(bool dvsyncSwitch);
    void SetTouchEvent(int32_t touchType);

    void DecreaseRequestVsyncTimes();
    int32_t GetRequestVsyncTimes() { return requestVsyncTimes_.load(); }

private:
    std::shared_ptr<VSyncReceiver> GetOrCreateVsyncReceiver();
    std::shared_ptr<VSyncReceiver> GetOrCreateVsyncReceiverLocked();
    std::shared_ptr<RSFrameRateLinker> GetFrameRateLinker();
    std::shared_ptr<RSFrameRateLinker> GetFrameRateLinkerLocked();
    void VsyncCallbackInner(int64_t nanoTimestamp, int64_t frameCount);
    void OnVsyncTimeOut();

    NodeId nodeId_ = 0;
    std::shared_ptr<AppExecFwk::EventHandler> vsyncHandler_ = nullptr;
    std::string vsyncTimeoutTaskName_;

    std::mutex mutex_;
    bool isFirstVsyncRequest_ = true;
    bool isFirstVsyncBack_ = true;
    bool destroyed_ = false;
    bool hasRequestedVsync_ = false;
    std::shared_ptr<VSyncReceiver> receiver_ = nullptr;
    std::shared_ptr<RSFrameRateLinker> frameRateLinker_ = nullptr;
    using Callbacks = std::unordered_set<std::shared_ptr<VsyncCallback>>;
    Callbacks vsyncCallbacks_;
    std::shared_ptr<FrameRateRange> lastFrameRateRange_ = nullptr;
    int32_t lastAnimatorExpectedFrameRate_ = 0;
    // Above guarded by mutex_

    std::atomic<int32_t> requestVsyncTimes_ {0};
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_VSYNC_STATION_H