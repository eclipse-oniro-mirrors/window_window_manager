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

#ifndef OHOS_WINDOW_PREDICT_SUBSCRIBER_H
#define OHOS_WINDOW_PREDICT_SUBSCRIBER_H

#include <mutex>

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "singleton.h"

namespace OHOS::Rosen {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventManager;
using OHOS::EventFwk::CommonEventSubscribeInfo;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::MatchingSkills;

class WindowEventSubscriber : public CommonEventSubscriber {
public:
    explicit WindowEventSubscriber(const CommonEventSubscribeInfo& info)
        : CommonEventSubscriber(info)
    {}

    ~WindowEventSubscriber() {}
    void OnReceiveEvent(const CommonEventData& data) override;
};

class WindowEventSubscribeProxy : public DelayedSingleton<WindowEventSubscribeProxy> {
public:
    ~WindowEventSubscribeProxy()
    {
        UnSubscribeEvent();
    }
    void SubscribeEvent();
    void UnSubscribeEvent();
    std::string GetBundleName() const
    {
        return bundleName_;
    }
    void SetBundleName(const std::string &bundleName)
    {
        bundleName_ = bundleName;
    }

private:
    std::string bundleName_;
    std::shared_ptr<CommonEventSubscriber> eventReceiver_;
    std::shared_ptr<WindowEventSubscriber> windowEventReceiver_;
    std::mutex mutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_PREDICT_SUBSCRIBER_H