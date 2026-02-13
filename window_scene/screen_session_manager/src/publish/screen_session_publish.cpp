/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "publish/screen_session_publish.h"

#include <common_event_manager.h>

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
const std::string CAST_PLUG_IN_FLAG_DATA = "1";
const std::string CAST_PLUG_OUT_FLAG_DATA = "0";
const std::string COMMON_EVENT_DISPLAY_ROTATION_CHANGED = "usual.event.dms.rotation_changed";
const std::string COMMON_EVENT_CAST_PLUGGED_CHANGED = "usual.event.dms.cast_plugged_changed";
const std::string COMMON_EVENT_SMART_NOTIFICATION = "hicare.event.SMART_NOTIFICATION";
const std::string COMMON_EVENT_LOW_TEMP_WARNING = "usual.event.thermal.LOW_TEMP_WARNING";
const std::string COMMON_EVENT_USER_SWITCHED = "usual.event.USER_SWITCHED";
const std::string COMMON_EVENT_ONEHOP_DEVICE_DOWN = "onehop.event.ONEHOP_DEVICE_DOWN_EVENT";
constexpr int32_t PUBLISH_SUCCESS = 0;
constexpr int32_t PUBLISH_FAILURE = -1;
constexpr int32_t TRANS_CODE_CAST_PLUGGED_CHANGED = 0;
constexpr int32_t TRANS_CODE_ROTATION_CHANGED = 1005;
constexpr int32_t lOW_TEMP_UID = 1096;
constexpr int32_t ONEHOP_UID = 5520;
constexpr int32_t RECEIVE_FAILURE = -1;

std::map<std::string, sptr<EventFwk::Want>> ScreenSessionPublish::cesWantMap_ = {
    {COMMON_EVENT_CAST_PLUGGED_CHANGED, nullptr},
    {COMMON_EVENT_DISPLAY_ROTATION_CHANGED, nullptr},
    {COMMON_EVENT_SMART_NOTIFICATION, nullptr}
};

ScreenSessionPublish::~ScreenSessionPublish()
{
    TLOGI(WmsLogTag::DMS, "destory");
    UnRegisterLowTempSubscriber();
    UnRegisterUserSwitchedSubscriber();
#ifdef FOLD_ABILITY_ENABLE
    UnRegisterOnehopDeviceDownSubscriber();
#endif
}

ScreenSessionPublish &ScreenSessionPublish::GetInstance()
{
    static ScreenSessionPublish screenSessionPublish;
    return screenSessionPublish;
}

void ScreenSessionPublish::InitPublishEvents()
{
    if (publishInfo_ != nullptr) {
        TLOGE(WmsLogTag::DMS, "ScreenSessionPublish has been initialized");
        return;
    }
    publishInfo_ = new (std::nothrow) EventFwk::CommonEventPublishInfo();
    if (publishInfo_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "publishInfo new failed");
        return;
    }
    publishInfo_->SetOrdered(false);
    for (auto &[event, want] : cesWantMap_) {
        want = new (std::nothrow) EventFwk::Want();
        if (want == nullptr) {
            TLOGE(WmsLogTag::DMS, "common event: %{publish}s new want failed", event.c_str());
            continue;
        }
        want->SetAction(event);
    }
}

int32_t ScreenSessionPublish::PublishEvents(
    const EventFwk::CommonEventData& eventData, std::string bundleName)
{
    if (publishInfo_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "publishInfo is nullptr");
        return PUBLISH_FAILURE;
    }
    if (bundleName != "") {
        publishInfo_->SetBundleName(bundleName);
    }
    bool ret = EventFwk::CommonEventManager::PublishCommonEvent(eventData, *publishInfo_, nullptr);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "failed");
        return PUBLISH_FAILURE;
    }
    TLOGI(WmsLogTag::DMS, "succeed");
    return PUBLISH_SUCCESS;
}

void ScreenSessionPublish::PublishCastPluggedEvent(const bool& isEnable)
{
    TLOGI(WmsLogTag::DMS, "start to publish cast plugged event");
    EventFwk::CommonEventData eventData;
    eventData.SetCode(TRANS_CODE_CAST_PLUGGED_CHANGED);
    if (isEnable) {
        eventData.SetData(CAST_PLUG_IN_FLAG_DATA);
    } else {
        eventData.SetData(CAST_PLUG_OUT_FLAG_DATA);
    }
    auto want = cesWantMap_[COMMON_EVENT_CAST_PLUGGED_CHANGED];
    if (want == nullptr) {
        TLOGE(WmsLogTag::DMS, "want is nullptr");
        return;
    }
    eventData.SetWant(*want);
    int32_t ret = PublishEvents(eventData);
    if (ret != PUBLISH_SUCCESS) {
        TLOGE(WmsLogTag::DMS, "PublishEvents failed");
        return;
    }
    TLOGI(WmsLogTag::DMS, "end of publish cast plugged event");
}

void ScreenSessionPublish::PublishCastPlugInEvent()
{
    TLOGI(WmsLogTag::DMS, "start to publish cast plug in event");
    PublishCastPluggedEvent(true);
}

void ScreenSessionPublish::PublishCastPlugOutEvent()
{
    TLOGI(WmsLogTag::DMS, "start to publish cast plug out event");
    PublishCastPluggedEvent(false);
}

void ScreenSessionPublish::PublishDisplayRotationEvent(
    const ScreenId& screenId, const Rotation& displayRotation)
{
    TLOGI(WmsLogTag::DMS, "start event, screenId: %{public}d, displayRotation: %{public}d",
        static_cast<int32_t>(screenId), static_cast<int32_t>(displayRotation));
    EventFwk::CommonEventData eventData;
    eventData.SetCode(TRANS_CODE_ROTATION_CHANGED);
    auto want = cesWantMap_[COMMON_EVENT_DISPLAY_ROTATION_CHANGED];
    if (want == nullptr) {
        TLOGE(WmsLogTag::DMS, "want is nullptr");
        return;
    }
    want->SetParam("screenid", static_cast<int32_t>(screenId));
    want->SetParam("rotation", static_cast<int32_t>(displayRotation));
    eventData.SetWant(*want);
    int32_t ret = PublishEvents(eventData);
    if (ret != PUBLISH_SUCCESS) {
        TLOGE(WmsLogTag::DMS, "PublishEvents failed");
        return;
    }
    TLOGD(WmsLogTag::DMS, "end event");
}

void ScreenSessionPublish::PublishSmartNotificationEvent(const std::string& faultDesc, const std::string& faultSuggest)
{
    EventFwk::CommonEventData eventData;
    auto want = cesWantMap_[COMMON_EVENT_SMART_NOTIFICATION];
    if (want == nullptr) {
        TLOGE(WmsLogTag::DMS, "want is nullptr");
        return;
    }
    eventData.SetWant(*want);
    std::string eventDataStr = "{\"faultDescription\":\"" + faultDesc + "\"," +
        "\"faultSuggestion\":\"" + faultSuggest + "\"}";
    eventData.SetData(eventDataStr);
    int32_t ret = PublishEvents(eventData);
    if (ret != PUBLISH_SUCCESS) {
        TLOGE(WmsLogTag::DMS, "PublishEvents failed");
        return;
    }
    TLOGI(WmsLogTag::DMS, "end event, send massage is %{public}s", eventDataStr.c_str());
}

bool ScreenSessionPublish::RegisterLowTempSubscriber()
{
    if (subscriber_ != nullptr) {
        TLOGE(WmsLogTag::DMS, "low temp is registered");
        return false;
    }
    EventFwk::MatchingSkills lowTempSkills = EventFwk::MatchingSkills();
    lowTempSkills.AddEvent(COMMON_EVENT_LOW_TEMP_WARNING);
    EventFwk::CommonEventSubscribeInfo lowTempInfo(lowTempSkills);
    lowTempInfo.SetPublisherUid(lOW_TEMP_UID);
    lowTempInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    subscriber_ = std::make_shared<EventSubscriber>(lowTempInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        TLOGE(WmsLogTag::DMS, "subscribe common event:%{public}s failed!", COMMON_EVENT_LOW_TEMP_WARNING.c_str());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "subscribe common event:%{public}s success.", COMMON_EVENT_LOW_TEMP_WARNING.c_str());
    return true;
}

bool ScreenSessionPublish::UnRegisterLowTempSubscriber()
{
    if (subscriber_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "subscriber_ is nullptr");
        return false;
    }
    EventFwk::MatchingSkills lowTempSkills = EventFwk::MatchingSkills();
    lowTempSkills.RemoveEvent(COMMON_EVENT_LOW_TEMP_WARNING);
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
        TLOGE(WmsLogTag::DMS, "unsubscribe common event:%{public}s failed!", COMMON_EVENT_LOW_TEMP_WARNING.c_str());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "unsubscribe common event:%{public}s success.", COMMON_EVENT_LOW_TEMP_WARNING.c_str());
    return true;
}

bool ScreenSessionPublish::RegisterUserSwitchedSubscriber()
{
    if (userSwitchedSubscriber_ != nullptr) {
        TLOGE(WmsLogTag::DMS, "user switched is registered");
        return false;
    }
    EventFwk::MatchingSkills userSwitchedSkills = EventFwk::MatchingSkills();
    userSwitchedSkills.AddEvent(COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo userSwitchedInfo(userSwitchedSkills);
    userSwitchedInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    userSwitchedSubscriber_ = std::make_shared<EventSubscriber>(userSwitchedInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(userSwitchedSubscriber_)) {
        TLOGE(WmsLogTag::DMS, "subscribe common event:%{public}s failed!", COMMON_EVENT_USER_SWITCHED.c_str());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "subscribe common event:%{public}s success.", COMMON_EVENT_USER_SWITCHED.c_str());
    return true;
}

bool ScreenSessionPublish::UnRegisterUserSwitchedSubscriber()
{
    if (userSwitchedSubscriber_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "userSwitchedSubscriber_ is nullptr");
        return false;
    }
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(userSwitchedSubscriber_)) {
        TLOGE(WmsLogTag::DMS, "unsubscribe common event:%{public}s failed!", COMMON_EVENT_USER_SWITCHED.c_str());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "unsubscribe common event:%{public}s success.", COMMON_EVENT_USER_SWITCHED.c_str());
    return true;
}

bool ScreenSessionPublish::RegisterOnehopDeviceDownSubscriber()
{
    if (oneHopDeviceDownSubscriber_ != nullptr) {
        TLOGE(WmsLogTag::DMS, "oneHopDeviceDownSubscriber_ is registered");
        return false;
    }
    EventFwk::MatchingSkills skills = EventFwk::MatchingSkills();
    skills.AddEvent(COMMON_EVENT_ONEHOP_DEVICE_DOWN);
    EventFwk::CommonEventSubscribeInfo info(skills);
    info.SetPublisherUid(ONEHOP_UID);
    info.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    oneHopDeviceDownSubscriber_ = std::make_shared<EventSubscriber>(info);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(oneHopDeviceDownSubscriber_)) {
        oneHopDeviceDownSubscriber_ = nullptr;
        TLOGE(WmsLogTag::DMS, "subscribe common event:%{public}s failed!", COMMON_EVENT_ONEHOP_DEVICE_DOWN.c_str());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "subscribe common event:%{public}s success.", COMMON_EVENT_ONEHOP_DEVICE_DOWN.c_str());
    return true;
}

bool ScreenSessionPublish::UnRegisterOnehopDeviceDownSubscriber()
{
    if (oneHopDeviceDownSubscriber_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "oneHopDeviceDownSubscriber_ is nullptr");
        return false;
    }
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(oneHopDeviceDownSubscriber_)) {
        TLOGE(WmsLogTag::DMS, "unsubscribe common event:%{public}s failed!", COMMON_EVENT_ONEHOP_DEVICE_DOWN.c_str());
        return false;
    }
    oneHopDeviceDownSubscriber_ = nullptr;
    TLOGI(WmsLogTag::DMS, "unsubscribe common event:%{public}s success.", COMMON_EVENT_ONEHOP_DEVICE_DOWN.c_str());
    return true;
}

void ScreenSessionPublish::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    std::string action = data.GetWant().GetAction();
    if (action == COMMON_EVENT_LOW_TEMP_WARNING) {
        int32_t lowTemp = data.GetWant().GetIntParam("lowTempWarning", RECEIVE_FAILURE);
        ScreenSessionManager::GetInstance().SetLowTemp(static_cast<LowTempMode>(lowTemp));
        TLOGI(WmsLogTag::DMS, "receive common event:%{public}s sucess, lowTempWarning is:%{public}d",
            COMMON_EVENT_LOW_TEMP_WARNING.c_str(), lowTemp);
    } else if (action == COMMON_EVENT_USER_SWITCHED) {
        TLOGI(WmsLogTag::DMS, "receive common event:%{public}s sucess", COMMON_EVENT_USER_SWITCHED.c_str());
        ScreenSessionManager::GetInstance().NotifyCastWhenSwitchScbNode();
    } else if (action == COMMON_EVENT_ONEHOP_DEVICE_DOWN) {
        TLOGI(WmsLogTag::DMS, "receive common event:%{public}s sucess", COMMON_EVENT_ONEHOP_DEVICE_DOWN.c_str());
        ScreenSessionManager::GetInstance().NotifyOnehopDeviceDown();
    }
}
} // namespace OHOS::Rosen