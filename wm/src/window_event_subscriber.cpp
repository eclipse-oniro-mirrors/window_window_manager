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

#include "window_event_subscriber.h"

#include "window_manager_hilog.h"
#include "input_transfer_station.h"

namespace OHOS::Rosen {
namespace {
constexpr int32_t PUBLISHER_UID = 7800;
const std::string TOUCH_PREDICT_ENABLE = "touch.events.touchpredict";
} // namespace

void WindowEventSubscribeProxy::SubscribeEvent()
{
    TLOGI(WmsLogTag::WMS_EVENT, "Subscribe %{public}s Event", TOUCH_PREDICT_ENABLE.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    if (windowEventReceiver_ == nullptr) {
        // create subscribe info
        MatchingSkills matchingSkills;
        // add common events
        matchingSkills.AddEvent(TOUCH_PREDICT_ENABLE);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPublisherUid(PUBLISHER_UID);
        subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::ThreadMode::HANDLER);

        // init Receiver
        windowEventReceiver_ = std::make_shared<WindowEventSubscriber>(subscribeInfo);
        eventReceiver_ = std::shared_ptr<CommonEventSubscriber>(windowEventReceiver_);

        // create subscription
        CommonEventManager::SubscribeCommonEvent(eventReceiver_);
    }
}

void WindowEventSubscribeProxy::UnSubscribeEvent()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (eventReceiver_ != nullptr) {
        CommonEventManager::UnSubscribeCommonEvent(eventReceiver_);
        eventReceiver_ = nullptr;
        windowEventReceiver_ = nullptr;
    }
}

void WindowEventSubscriber::OnReceiveEvent(const CommonEventData& data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    std::string bundleName = want.GetBundle();
    if (bundleName.empty()) {
        TLOGI(WmsLogTag::WMS_EVENT, "OnReceiveEvent empty bundleName");
        return;
    }
    if (bundleName != WindowEventSubscribeProxy::GetInstance()->GetBundleName()) {
        TLOGI(WmsLogTag::WMS_EVENT, "OnReceiveEvent bundleName not match");
        return;
    }
    if (action == TOUCH_PREDICT_ENABLE) {
        TLOGI(WmsLogTag::WMS_EVENT, "OnReceiveEvent received %{public}s", action.c_str());
        InputTransferStation::GetInstance().LoadTouchPredictor();
        return;
    }
}

} // namespace OHOS::Rosen