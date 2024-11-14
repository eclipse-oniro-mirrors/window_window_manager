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

#include <gtest/gtest.h>
#include <message_option.h>
#include <message_parcel.h>
#include <iremote_broker.h>
#include "display_manager_lite_proxy.h"
#include "display_manager_adapter_lite.h"
#include "display_manager_agent_default.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class DisplayManagerLiteProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DisplayManagerLiteProxyTest::SetUpTestCase()
{
}

void DisplayManagerLiteProxyTest::TearDownTestCase()
{
}

void DisplayManagerLiteProxyTest::SetUp()
{
}

void DisplayManagerLiteProxyTest::TearDown()
{
}

namespace {
/**
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: RegisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, RegisterDisplayManagerAgent, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(DMError::DM_OK, displayManagerLiteProxy->RegisterDisplayManagerAgent(displayManagerAgent, type));
    } else {
        EXPECT_NE(DMError::DM_OK, displayManagerLiteProxy->RegisterDisplayManagerAgent(displayManagerAgent, type));
    }
    displayManagerAgent = nullptr;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM,
              displayManagerLiteProxy->RegisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: UnregisterDisplayManagerAgent
 * @tc.desc: UnregisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, UnregisterDisplayManagerAgent, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(DMError::DM_OK, displayManagerLiteProxy->UnregisterDisplayManagerAgent(displayManagerAgent, type));
    } else {
        EXPECT_NE(DMError::DM_OK, displayManagerLiteProxy->UnregisterDisplayManagerAgent(displayManagerAgent, type));
    }
    displayManagerAgent = nullptr;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM,
              displayManagerLiteProxy->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: GetFoldDisplayMode
 * @tc.desc: GetFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetFoldDisplayMode, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    EXPECT_EQ(FoldDisplayMode::UNKNOWN,
              displayManagerLiteProxy->GetFoldDisplayMode());
}

/**
 * @tc.name: SetFoldDisplayMode
 * @tc.desc: SetFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, SetFoldDisplayMode, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    int resultValue = 0;
    const FoldDisplayMode displayMode {0};
    std::function<void()> func = [&]()
    {
        displayManagerLiteProxy->SetFoldDisplayMode(displayMode);
        resultValue = 1;
    };
    func();
    EXPECT_EQ(resultValue, 1);
}

/**
 * @tc.name: IsFoldable
 * @tc.desc: IsFoldable
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, IsFoldable, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        displayManagerLiteProxy->IsFoldable();
        resultValue = 1;
    };
    func();
    EXPECT_EQ(resultValue, 1);
}

/**
 * @tc.name: GetFoldStatus
 * @tc.desc: GetFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetFoldStatus, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);
    ASSERT_NE(displayManagerLiteProxy, nullptr);
    displayManagerLiteProxy->GetFoldStatus();
}

/**
 * @tc.name: GetDefaultDisplayInfo
 * @tc.desc: GetDefaultDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetDefaultDisplayInfo, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();

    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    sptr<DisplayInfo> expectation = nullptr;
    sptr<DisplayInfo> res = nullptr;
    std::function<void()> func = [&]()
    {
        res = displayManagerLiteProxy->GetDefaultDisplayInfo();
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: GetDisplayInfoById
 * @tc.desc: GetDisplayInfoById
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetDisplayInfoById, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();

    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    sptr<DisplayInfo> expectation = nullptr;
    sptr<DisplayInfo> res = nullptr;
    DisplayId displayId {0};
    std::function<void()> func = [&]()
    {
        res = displayManagerLiteProxy->GetDisplayInfoById(displayId);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: GetCutoutInfo
 * @tc.desc: GetCutoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetCutoutInfo, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();

    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    sptr<CutoutInfo> expectation = nullptr;
    sptr<CutoutInfo> res = nullptr;
    DisplayId displayId = 0;
    std::function<void()> func = [&]()
    {
        res = displayManagerLiteProxy->GetCutoutInfo(displayId);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}


/**
 * @tc.name: WakeUpBegin
 * @tc.desc: WakeUpBegin
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, WakeUpBegin, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    PowerStateChangeReason reason {0};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, displayManagerLiteProxy->WakeUpBegin(reason));
    } else {
        EXPECT_NE(expectation, displayManagerLiteProxy->WakeUpBegin(reason));
    }
}

/**
 * @tc.name: WakeUpEnd
 * @tc.desc: WakeUpEnd
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, WakeUpEnd, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, displayManagerLiteProxy->WakeUpEnd());
    } else {
        EXPECT_NE(expectation, displayManagerLiteProxy->WakeUpEnd());
    }
}

/**
 * @tc.name: SuspendEnd
 * @tc.desc: SuspendEnd
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, SuspendEnd, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, displayManagerLiteProxy->SuspendEnd());
    } else {
        EXPECT_NE(expectation, displayManagerLiteProxy->SuspendEnd());
    }
}

/**
 * @tc.name: SetDisplayState
 * @tc.desc: SetDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, SetDisplayState, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    DisplayState state {1};
    displayManagerLiteProxy->SetDisplayState(state);
    int resultValue = 0;
    ASSERT_EQ(resultValue, 0);
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: SetSpecifiedScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, SetSpecifiedScreenPower, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    ScreenPowerState state {0};
    ScreenId id = 1001;
    PowerStateChangeReason reason {1};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, displayManagerLiteProxy->SetSpecifiedScreenPower(id, state, reason));
    } else {
        EXPECT_NE(expectation, displayManagerLiteProxy->SetSpecifiedScreenPower(id, state, reason));
    }
}

/**
 * @tc.name: SetScreenPowerForAll
 * @tc.desc: SetScreenPowerForAll
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, SetScreenPowerForAll, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    ScreenPowerState state {0};
    PowerStateChangeReason reason {1};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, displayManagerLiteProxy->SetScreenPowerForAll(state, reason));
    } else {
        EXPECT_NE(expectation, displayManagerLiteProxy->SetScreenPowerForAll(state, reason));
    }
}

/**
 * @tc.name: GetScreenPower
 * @tc.desc: GetScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetScreenPower, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    ScreenId dmsScreenId = 1001;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(ScreenPowerState::INVALID_STATE, displayManagerLiteProxy->GetScreenPower(dmsScreenId));
    } else {
        EXPECT_EQ(ScreenPowerState::INVALID_STATE, displayManagerLiteProxy->GetScreenPower(dmsScreenId));
    }
}

/**
 * @tc.name: GetDisplayState
 * @tc.desc: GetDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetDisplayState, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    DisplayId displayId {0};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DisplayState::UNKNOWN, displayManagerLiteProxy->GetDisplayState(displayId));
    } else {
        EXPECT_EQ(DisplayState::UNKNOWN, displayManagerLiteProxy->GetDisplayState(displayId));
    }
}

/**
 * @tc.name: GetAllDisplayIds
 * @tc.desc: GetAllDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyTest, GetAllDisplayIds, Function | SmallTest | Level1)
{
    SingletonContainer::Get<DisplayManagerAdapterLite>().InitDMSProxy();
    
    sptr<IRemoteObject> impl =
        SingletonContainer::Get<DisplayManagerAdapterLite>().displayManagerServiceProxy_->AsObject();
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy = new DisplayManagerLiteProxy(impl);

    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        displayManagerLiteProxy->GetAllDisplayIds();
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}
}
}
}