/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <unistd.h>
#include "transaction/rs_interfaces.h"
#include "window_manager_hilog.h"


namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "VsyncStation"};
}

VsyncStation::VsyncStation(NodeId nodeId) : nodeId_(nodeId)
{
    TLOGI(WmsLogTag::WMS_MAIN, "Vsync Constructor");
}

void VsyncStation::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (destroyed_) {
            return;
        }
        vsyncCallbacks_.insert(vsyncCallback);

        if (!hasInitVsyncReceiver_) {
            auto& rsClient = OHOS::Rosen::RSInterfaces::GetInstance();
            while (receiver_ == nullptr) {
                receiver_ = rsClient.CreateVSyncReceiver("WM_" + std::to_string(getpid()), nodeId_);
                TLOGI(WmsLogTag::WMS_MAIN, "Create vsync receiver for nodeId:%{public}" PRIu64"", nodeId_);
            }
            receiver_->Init();
            hasInitVsyncReceiver_ = true;
        }
        if (hasRequestedVsync_) {
            return;
        }
        hasRequestedVsync_ = true;
    }
    receiver_->RequestNextVSync(frameCallback_);
}

int64_t VsyncStation::GetVSyncPeriod()
{
    return 0;
}

void VsyncStation::Init()
{
}

void VsyncStation::RemoveCallback()
{
    WLOGI("Remove Vsync callback");
    std::lock_guard<std::mutex> lock(mtx_);
    vsyncCallbacks_.clear();
}

void VsyncStation::VsyncCallbackInner(int64_t timestamp, int64_t frameCount)
{
    std::unordered_set<std::shared_ptr<VsyncCallback>> vsyncCallbacks;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        hasRequestedVsync_ = false;
        vsyncCallbacks = vsyncCallbacks_;
        vsyncCallbacks_.clear();
    }
    for (const auto& callback: vsyncCallbacks) {
        callback->onCallback(timestamp, frameCount);
    }
}

void VsyncStation::OnVsync(int64_t timestamp, int64_t frameCount, void* client)
{
    auto vsyncClient = static_cast<VsyncStation*>(client);
    if (vsyncClient) {
        vsyncClient->VsyncCallbackInner(timestamp, frameCount);
    } else {
        WLOGFE("VsyncClient is null");
    }
}

void VsyncStation::OnVsyncTimeOut()
{
    WLOGW("Vsync time out");
    std::lock_guard<std::mutex> lock(mtx_);
    hasRequestedVsync_ = false;
}
}
}
