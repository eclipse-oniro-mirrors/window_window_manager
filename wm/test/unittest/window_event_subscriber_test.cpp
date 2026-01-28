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

#include <gtest/gtest.h>

#include "window_event_subscriber.h"
#include "common_event_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
class WindowEventSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
void WindowEventSubscriberTest::SetUpTestCase() {}

void WindowEventSubscriberTest::TearDownTestCase() {}

void WindowEventSubscriberTest::SetUp() {}

void WindowEventSubscriberTest::TearDown() {}

const std::string TEST_BUNDLE_NAME = "com.TestBundleName";

sptr<OHOS::EventFwk::CommonEventData> CreateCommonEventData(const std::string& action,
    const std::string& bundleName = TEST_BUNDLE_NAME)
{
    sptr<OHOS::EventFwk::CommonEventData> data = new OHOS::EventFwk::CommonEventData();
    sptr<OHOS::AAFwk::Want> want = new OHOS::AAFwk::Want();
    sptr<OHOS::AppExecFwk::ElementName> elementName = new OHOS::AppExecFwk::ElementName();
    if (want != nullptr && data != nullptr && elementName != nullptr) {
        elementName->SetBundleName(bundleName);
        want->SetAction(action);
        want->SetElement(*elementName);
        data->SetWant(*want);
    }
    return data;
}

namespace {

const std::string TOUCH_PREDICT_ENABLE = "touch.events.touchpredict";
/**
 * @tc.name: WindowEventSubscriber::OnReceiveEvent
 * @tc.desc: Test OnReceiveEvent function, enable touch predict.
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventSubscriberTest, OnReceiveEvent, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    CommonEventSubscribeInfo subscribeInfo;
    auto subscriber = std::make_shared<WindowEventSubscriber>(subscribeInfo);
    WindowEventSubscribeProxy::GetInstance()->SetBundleName(TEST_BUNDLE_NAME);
    ASSERT_NE(subscriber, nullptr);
    auto data = CreateCommonEventData(TOUCH_PREDICT_ENABLE);
    subscriber->OnReceiveEvent(*data);
    data = CreateCommonEventData("DUMMY_ACTION");
    subscriber->OnReceiveEvent(*data);
    data = CreateCommonEventData(TOUCH_PREDICT_ENABLE, "DUMMY_BUNDLE_NAME");
    subscriber->OnReceiveEvent(*data);
    EXPECT_TRUE(g_errLog.find("OnReceiveEvent bundleName not match") != std::string::npos);
    data = CreateCommonEventData(TOUCH_PREDICT_ENABLE, "");
    subscriber->OnReceiveEvent(*data);
    EXPECT_TRUE(g_errLog.find("OnReceiveEvent empty bundleName") != std::string::npos);
    WindowEventSubscribeProxy::GetInstance()->UnSubscribeEvent();
    LOG_SetCallback(nullptr);
}

} // namespace
}
} // namespace Rosen
} // namespace OHOS