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

#ifndef OHOS_ROSEN_SCREEN_SESSION_PUBLISH_H
#define OHOS_ROSEN_SCREEN_SESSION_PUBLISH_H

#include "errors.h"

#include <want.h>
#include <common_event_data.h>
#include <common_event_publish_info.h>

#include "common_event_subscriber.h"
#include "dm_common.h"
#include "screen_session_manager.h"

namespace OHOS::Rosen {
class ScreenSessionPublish {
public:
    static ScreenSessionPublish &GetInstance();
    void InitPublishEvents();
    void PublishCastPlugInEvent();
    void PublishCastPlugOutEvent();
    void PublishDisplayRotationEvent(const ScreenId& screenId, const Rotation& displayRotation);
    void PublishSmartNotificationEvent(const std::string& faultDesc, const std::string& faultSuggest);
    bool RegisterLowTempSubscriber();
    bool UnRegisterLowTempSubscriber();
    bool RegisterUserSwitchedSubscriber();
    bool UnRegisterUserSwitchedSubscriber();
    bool RegisterOnehopDeviceDownSubscriber();
    bool UnRegisterOnehopDeviceDownSubscriber();

private:
    explicit ScreenSessionPublish() = default;
    ~ScreenSessionPublish();

    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        EventSubscriber(const EventFwk::CommonEventSubscribeInfo& info)
            : EventFwk::CommonEventSubscriber(info) {}
        ~EventSubscriber() = default;
        void OnReceiveEvent(const EventFwk::CommonEventData& data) override;
    };

    int32_t PublishEvents(const EventFwk::CommonEventData& eventData, std::string bundleName = "");
    void PublishCastPluggedEvent(const bool& isEnable);

private:
    sptr<EventFwk::CommonEventPublishInfo> publishInfo_;
    static std::map<std::string, sptr<EventFwk::Want>> cesWantMap_;
    std::shared_ptr<EventSubscriber> subscriber_ = nullptr;
    std::shared_ptr<EventSubscriber> userSwitchedSubscriber_ = nullptr;
    std::shared_ptr<EventSubscriber> oneHopDeviceDownSubscriber_ = nullptr;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_SCREEN_SESSION_PUBLISH_H