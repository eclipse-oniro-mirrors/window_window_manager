/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <functional>
#include <map>
#include <memory>
#include <unordered_set>

#include <refbase.h>
#include <event_handler.h>
#include <vsync_receiver.h>
#include <ui/rs_display_node.h>
#include <ui/rs_ui_display_soloist.h>

#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
class RSFrameRateLinker;
class VsyncStation {
public:
    explicit VsyncStation(NodeId nodeId);
    ~VsyncStation();
    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback);
    int64_t GetVSyncPeriod();
    FrameRateLinkerId GetFrameRateLinkerId();
    void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType = 0);
    void SetFrameRateLinkerEnable(bool enabled);
    void SetDisplaySoloistFrameRateLinkerEnable(bool enabled);
    void RemoveCallback();
    void SetIsMainHandlerAvailable(bool available)
    {
        isMainHandlerAvailable_ = available;
    }

    void SetVsyncEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& eventHandler)
    {
        vsyncHandler_ = eventHandler;
    }
    void SetUiDvsyncSwitch(bool dvsyncSwitch);
    bool IsResourceEnough();

private:
    static void OnVsync(int64_t nanoTimestamp, int64_t frameCount, void* client);
    void VsyncCallbackInner(int64_t nanoTimestamp, int64_t frameCount);
    void OnVsyncTimeOut();
    void Init();

    std::mutex mtx_;
    bool hasRequestedVsync_ = false;
    bool hasInitVsyncReceiver_ = false;
    bool isMainHandlerAvailable_ = true;
    bool destroyed_ = false;
    const std::string VSYNC_THREAD_ID = "VsyncThread";
    NodeId nodeId_ = 0;
    std::string vsyncTimeoutTaskName_ = "";
    std::shared_ptr<OHOS::Rosen::VSyncReceiver> receiver_ = nullptr;
    std::shared_ptr<RSFrameRateLinker> frameRateLinker_;
    std::unordered_set<std::shared_ptr<VsyncCallback>> vsyncCallbacks_;
    VSyncReceiver::FrameCallback frameCallback_ = {
        .userData_ = this,
        .callbackWithId_ = OnVsync,
    };
    std::shared_ptr<AppExecFwk::EventHandler> vsyncHandler_ = nullptr;
    AppExecFwk::EventHandler::Callback vsyncTimeoutCallback_ = [this] { this->OnVsyncTimeOut(); };
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_VSYNC_STATION_H