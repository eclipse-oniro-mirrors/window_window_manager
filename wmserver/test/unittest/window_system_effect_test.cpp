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

#include <gtest/gtest.h>
#include <transaction/rs_transaction.h>

#include "display_manager.h"
#include "iremote_object_mocker.h"
#include "window_helper.h"
#include "window_system_effect.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSystemEffectTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<WindowProperty> CreateWindowProperty();
private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode(std::string name);
    sptr<WindowNode> node_ = nullptr;
    AppWindowEffectConfig effectConfig_;
    sptr<WindowRoot> windowRoot_;
};

void WindowSystemEffectTest::SetUpTestCase()
{
}

void WindowSystemEffectTest::TearDownTestCase()
{
}

void WindowSystemEffectTest::SetUp()
{
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE((display != nullptr));
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    ASSERT_TRUE((displayInfo != nullptr));
    auto property = CreateWindowProperty();
    node_ = sptr<WindowNode>::MakeSptr(); // 101 is windowId
    node_->SetWindowProperty(property);
    node_->SetWindowRect({0, 0, 100, 100}); // 100 test data
    node_->leashWinSurfaceNode_ = CreateRSSurfaceNode("leashSurfaceNodeTest");
    node_->surfaceNode_ = CreateRSSurfaceNode("SurfaceNodeTest");
    effectConfig_.fullScreenCornerRadius_ = 16.0f; // 16.f test data
    effectConfig_.splitCornerRadius_ = 16.0f; // 16.f test data
    effectConfig_.floatCornerRadius_ = 16.0f; // 16.f test data
    effectConfig_.focusedShadow_ = {80, "#000000", 0, 5, 0.45};
    effectConfig_.unfocusedShadow_ = {55, "#000000", 0, 10, 0.25};
    WindowSystemEffect::SetWindowSystemEffectConfig(effectConfig_);
    windowRoot_ = new WindowRoot([](Event event, const sptr<IRemoteObject>& remoteObject) {});
    ASSERT_NE(nullptr, windowRoot_);
    WindowSystemEffect::SetWindowRoot(windowRoot_);
}

void WindowSystemEffectTest::TearDown()
{
    AppWindowEffectConfig config;
    effectConfig_ = config;
    node_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSystemEffectTest::CreateRSSurfaceNode(std::string name)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = name;
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

sptr<WindowProperty> WindowSystemEffectTest::CreateWindowProperty()
{
    sptr<WindowProperty> property = new WindowProperty();
    return property;
}
namespace {
/**
 * @tc.name: SetWindowEffectAndCornerRadius01
 * @tc.desc: set window corner radius with different parameter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSystemEffectTest, SetWindowEffectAndCornerRadius01, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetWindowEffect(node_));
    // fullscreen
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));
    WindowSystemEffect::windowSystemEffectConfig_.fullScreenCornerRadius_ = 0.0f;
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));

    // splitmode
    node_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));
    WindowSystemEffect::windowSystemEffectConfig_.splitCornerRadius_ = 0.0f;
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));

    // float mode
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));
    WindowSystemEffect::windowSystemEffectConfig_.floatCornerRadius_ = 0.0f;
    ASSERT_EQ(WMError::WM_DO_NOTHING, WindowSystemEffect::SetCornerRadius(node_));

    node_->leashWinSurfaceNode_ = nullptr;
    WindowSystemEffect::windowSystemEffectConfig_.floatCornerRadius_ = 16.0f;
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));

    node_->leashWinSurfaceNode_ = CreateRSSurfaceNode("leashSurfaceNodeTest");
    node_->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_OK, WindowSystemEffect::SetCornerRadius(node_));

    node_->leashWinSurfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, WindowSystemEffect::SetCornerRadius(node_));

    WindowSystemEffect::SetWindowRoot(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, WindowSystemEffect::SetCornerRadius(node_));
}
}
}
}
