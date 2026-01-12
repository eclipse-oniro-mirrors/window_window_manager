/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <common/rs_rect.h>
#include <gtest/gtest.h>
#include <iremote_broker.h>
#include <transaction/rs_marshalling_helper.h>

#include "display_manager_adapter.h"
#include "display_manager_agent_default.h"
#include "fold_screen_state_internel.h"
#include "scene_board_judgement.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "zidl/screen_session_manager_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
class ScreenSessionManagerProxyUtTest : public testing::Test {
public:
    static void SetUpTestSuite();
    void SetUp() override;
    sptr<ScreenSessionManagerProxy> screenSessionManagerProxy;
};

void ScreenSessionManagerProxyUtTest::SetUpTestSuite()
{
    ASSERT_TRUE(SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy());
}

void ScreenSessionManagerProxyUtTest::SetUp()
{
    if (screenSessionManagerProxy) {
        return;
    }

    sptr<IRemoteObject> impl;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_, nullptr);
        impl = SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
    } else {
        ASSERT_NE(SingletonContainer::Get<ScreenManagerAdapter>().displayManagerServiceProxy_, nullptr);
        impl = SingletonContainer::Get<ScreenManagerAdapter>().displayManagerServiceProxy_->AsObject();
    }

    screenSessionManagerProxy = new (std::nothrow) ScreenSessionManagerProxy(impl);
    ASSERT_NE(screenSessionManagerProxy, nullptr);
}

namespace {
/**
 * @tc.name: SetPrivacyStateByDisplayId
 * @tc.desc: SetPrivacyStateByDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetPrivacyStateByDisplayId, TestSize.Level1)
{
    DisplayId id = 0;
    bool hasPrivate = false;

    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        std::unordered_map<DisplayId, bool> privacyBundleDisplayId = {{id, hasPrivate}};
        screenSessionManagerProxy->SetPrivacyStateByDisplayId(privacyBundleDisplayId);
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: SetScreenPrivacyWindowList
 * @tc.desc: SetScreenPrivacyWindowList
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenPrivacyWindowList, TestSize.Level1)
{
    DisplayId id = 0;
    std::vector<std::string> privacyWindowList{"win0", "win1"};

    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        screenSessionManagerProxy->SetScreenPrivacyWindowList(id, privacyWindowList);
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: SetVirtualScreenBlackList
 * @tc.desc: SetVirtualScreenBlackList
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualScreenBlackList, TestSize.Level1)
{
    ScreenId id = 1001;
    std::vector<uint64_t> windowIdList{10, 20, 30};

    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        screenSessionManagerProxy->SetVirtualScreenBlackList(id, windowIdList);
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: SetVirtualDisplayMuteFlag
 * @tc.desc: SetVirtualDisplayMuteFlag
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualDisplayMuteFlag, Function | SmallTest | Level1)
{
    ScreenId id = 1001;
    bool muteFlag = false;
    screenSessionManagerProxy->SetVirtualDisplayMuteFlag(id, muteFlag);
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(2000);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: ProxyForFreeze
 * @tc.desc: ProxyForFreeze
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, ProxyForFreeze, TestSize.Level1)
{
    const std::set<int32_t>& pidList = {1, 2, 3};
    bool isProxy = true;

    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->ProxyForFreeze(pidList, isProxy));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->ProxyForFreeze(pidList, isProxy));
    }
}

/**
 * @tc.name: SetVirtualScreenStatus
 * @tc.desc: SetVirtualScreenStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualScreenStatus, TestSize.Level1)
{
    ScreenId id = 1001;
    VirtualScreenStatus screenStatus = VirtualScreenStatus::VIRTUAL_SCREEN_PAUSE;

    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(expectation, screenSessionManagerProxy->SetVirtualScreenStatus(id, screenStatus));
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->SetVirtualScreenStatus(id, screenStatus));
    }
}

/**
 * @tc.name: SetVirtualScreenSecurityExemption
 * @tc.desc: SetVirtualScreenSecurityExemption
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualScreenSecurityExemption, TestSize.Level1)
{
    ScreenId id = 1001;
    uint32_t pid = 1;
    std::vector<uint64_t> windowIdList{10, 20, 30};

    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetVirtualScreenSecurityExemption(id, pid, windowIdList));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetVirtualScreenSecurityExemption(id, pid, windowIdList));
    }
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetAllDisplayPhysicalResolution, TestSize.Level1)
{
    std::vector<DisplayPhysicalResolution> allSize {};
    std::function<void()> func = [&]()
    {
        allSize = screenSessionManagerProxy->GetAllDisplayPhysicalResolution();
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_TRUE(!allSize.empty());
    } else {
        ASSERT_FALSE(!allSize.empty());
    }
}

/**
 * @tc.name: GetDefaultDisplayInfo
 * @tc.desc: GetDefaultDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDefaultDisplayInfo, TestSize.Level1)
{
    sptr<DisplayInfo> expectation = nullptr;
    sptr<DisplayInfo> res = nullptr;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetDefaultDisplayInfo();
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: SetScreenActiveMode
 * @tc.desc: SetScreenActiveMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenActiveMode, TestSize.Level1)
{
    ScreenId id = 1001;
    uint32_t modeId = 1;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetScreenActiveMode(id, modeId);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: SetVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualPixelRatio, TestSize.Level1)
{
    ScreenId id = 1001;
    float virtualPixelRatio = 1.0;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetVirtualPixelRatio(id, virtualPixelRatio);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetVirtualPixelRatioSystem
 * @tc.desc: SetVirtualPixelRatioSystem
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualPixelRatioSystem, TestSize.Level1)
{
    ScreenId id = 1001;
    float virtualPixelRatio = 1.0;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetVirtualPixelRatioSystem(id, virtualPixelRatio);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetDefaultDensityDpi
 * @tc.desc: SetDefaultDensityDpi
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetDefaultDensityDpi, TestSize.Level1)
{
    ScreenId id = 1001;
    float virtualPixelRatio = 1.0;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetDefaultDensityDpi(id, virtualPixelRatio);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetResolution, TestSize.Level1)
{
    ScreenId id = 1001;
    uint32_t width = 1024;
    uint32_t height = 1024;
    float virtualPixelRatio = 1.0;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetResolution(id, width, height, virtualPixelRatio);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: GetDensityInCurResolution
 * @tc.desc: GetDensityInCurResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDensityInCurResolution, TestSize.Level1)
{
    ScreenId id = 1001;
    float virtualPixelRatio = 1.0;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetDensityInCurResolution(id, virtualPixelRatio);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: GetScreenColorGamut
 * @tc.desc: GetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenColorGamut, TestSize.Level1)
{
    ScreenId id = 1001;
    ScreenColorGamut colorGamut {0};


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetScreenColorGamut(id, colorGamut);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetScreenColorGamut
 * @tc.desc: SetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenColorGamut, TestSize.Level1)
{
    ScreenId id = 1001;
    uint32_t colorGamut = 0;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetScreenColorGamut(id, colorGamut);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: GetScreenGamutMap
 * @tc.desc: GetScreenGamutMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenGamutMap, TestSize.Level1)
{
    ScreenId id = 1001;
    ScreenGamutMap colorGamut;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetScreenGamutMap(id, colorGamut);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetScreenGamutMap
 * @tc.desc: SetScreenGamutMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenGamutMap, TestSize.Level1)
{
    ScreenId id = 1001;
    ScreenGamutMap colorGamut {0};


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetScreenGamutMap(id, colorGamut);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetScreenColorTransform
 * @tc.desc: SetScreenColorTransform
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenColorTransform, TestSize.Level1)
{
    ScreenId id = 1001;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetScreenColorTransform(id);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: GetPixelFormat
 * @tc.desc: GetPixelFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetPixelFormat, TestSize.Level1)
{
    ScreenId id = 1001;
    GraphicPixelFormat pixelFormat = GraphicPixelFormat {GRAPHIC_PIXEL_FMT_CLUT8};


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetPixelFormat(id, pixelFormat);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetPixelFormat
 * @tc.desc: SetPixelFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetPixelFormat, TestSize.Level1)
{
    ScreenId id = 1001;
    GraphicPixelFormat pixelFormat = GraphicPixelFormat {GRAPHIC_PIXEL_FMT_CLUT8};


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetPixelFormat(id, pixelFormat);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: GetSupportedHDRFormats
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetSupportedHDRFormats, TestSize.Level1)
{
    ScreenId id = 1001;
    vector<ScreenHDRFormat> hdrFormats;


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetSupportedHDRFormats(id, hdrFormats);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: GetScreenHDRFormat
 * @tc.desc: GetScreenHDRFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenHDRFormat, TestSize.Level1)
{
    ScreenId id = 1001;
    ScreenHDRFormat hdrFormatS {0};


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetScreenHDRFormat(id, hdrFormatS);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetScreenHDRFormat
 * @tc.desc: SetScreenHDRFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenHDRFormat, TestSize.Level1)
{
    ScreenId id = 1001;
    ScreenHDRFormat hdrFormatS {0};


    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetScreenHDRFormat(id, hdrFormatS);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: RegisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, RegisterDisplayManagerAgent, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(DMError::DM_OK, screenSessionManagerProxy->RegisterDisplayManagerAgent(displayManagerAgent, type));
    } else {
        EXPECT_NE(DMError::DM_OK, screenSessionManagerProxy->RegisterDisplayManagerAgent(displayManagerAgent, type));
    }
    displayManagerAgent = nullptr;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM,
              screenSessionManagerProxy->RegisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: UnregisterDisplayManagerAgent
 * @tc.desc: UnregisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, UnregisterDisplayManagerAgent, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(DMError::DM_OK, screenSessionManagerProxy->UnregisterDisplayManagerAgent(displayManagerAgent, type));
    } else {
        EXPECT_NE(DMError::DM_OK, screenSessionManagerProxy->UnregisterDisplayManagerAgent(displayManagerAgent, type));
    }
    displayManagerAgent = nullptr;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM,
              screenSessionManagerProxy->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: WakeUpBegin
 * @tc.desc: WakeUpBegin
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, WakeUpBegin, TestSize.Level1)
{
    PowerStateChangeReason reason {0};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->WakeUpBegin(reason));
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->WakeUpBegin(reason));
    }
}

/**
 * @tc.name: WakeUpEnd
 * @tc.desc: WakeUpEnd
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, WakeUpEnd, TestSize.Level1)
{
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->WakeUpEnd());
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->WakeUpEnd());
    }
}

/**
 * @tc.name: SuspendBegin
 * @tc.desc: SuspendBegin
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SuspendBegin, TestSize.Level1)
{
    PowerStateChangeReason reason {0};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->SuspendBegin(reason));
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->SuspendBegin(reason));
    }
}

/**
 * @tc.name: SuspendEnd
 * @tc.desc: SuspendEnd
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SuspendEnd, TestSize.Level1)
{
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->SuspendEnd());
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->SuspendEnd());
    }
}

/**
 * @tc.name: SetScreenPowerById
 * @tc.desc: SetScreenPowerById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenPowerById, TestSize.Level1)
{
    ScreenPowerState state {0};
    ScreenId id = 1001;
    PowerStateChangeReason reason {1};

    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->SetScreenPowerById(id, state, reason));
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->SetScreenPowerById(id, state, reason));
    }
}

/**
 * @tc.name: SetDisplayState
 * @tc.desc: SetDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetDisplayState, TestSize.Level1)
{
    DisplayState state {1};
    screenSessionManagerProxy->SetDisplayState(state);
    int resultValue = 0;
    ASSERT_EQ(resultValue, 0);
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: SetSpecifiedScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetSpecifiedScreenPower, TestSize.Level1)
{
    ScreenPowerState state {0};
    ScreenId id = 1001;
    PowerStateChangeReason reason {1};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->SetSpecifiedScreenPower(id, state, reason));
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->SetSpecifiedScreenPower(id, state, reason));
    }
}

/**
 * @tc.name: SetScreenPowerForAll
 * @tc.desc: SetScreenPowerForAll
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenPowerForAll, TestSize.Level1)
{
    ScreenPowerState state {0};
    PowerStateChangeReason reason {1};
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(expectation, screenSessionManagerProxy->SetScreenPowerForAll(state, reason));
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->SetScreenPowerForAll(state, reason));
    }
}

/**
 * @tc.name: GetDisplayState
 * @tc.desc: GetDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDisplayState, TestSize.Level1)
{
    DisplayId displayId {0};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DisplayState::UNKNOWN, screenSessionManagerProxy->GetDisplayState(displayId));
    } else {
        EXPECT_EQ(DisplayState::UNKNOWN, screenSessionManagerProxy->GetDisplayState(displayId));
    }
}

/**
 * @tc.name: TryToCancelScreenOff
 * @tc.desc: TryToCancelScreenOff
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, TryToCancelScreenOff, TestSize.Level1)
{
    bool expectation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(expectation, screenSessionManagerProxy->TryToCancelScreenOff());
    } else {
        EXPECT_NE(expectation, screenSessionManagerProxy->TryToCancelScreenOff());
    }
}

/**
 * @tc.name: NotifyDisplayEvent
 * @tc.desc: NotifyDisplayEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, NotifyDisplayEvent, TestSize.Level1)
{
    DisplayEvent event {0};
    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        screenSessionManagerProxy->NotifyDisplayEvent(event);
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: GetScreenPower
 * @tc.desc: GetScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenPower, TestSize.Level1)
{
    ScreenId dmsScreenId = 1001;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(ScreenPowerState::INVALID_STATE, screenSessionManagerProxy->GetScreenPower(dmsScreenId));
    } else {
        EXPECT_EQ(ScreenPowerState::INVALID_STATE, screenSessionManagerProxy->GetScreenPower(dmsScreenId));
    }
}

/**
 * @tc.name: AddVirtualScreenBlockList
 * @tc.desc: AddVirtualScreenBlockList
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, AddVirtualScreenBlockList, TestSize.Level1)
{
    std::vector<int32_t> persistentIds {0, 1, 2};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->AddVirtualScreenBlockList(persistentIds));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->AddVirtualScreenBlockList(persistentIds));
    }
}

/**
 * @tc.name: RemoveVirtualScreenBlockList
 * @tc.desc: RemoveVirtualScreenBlockList
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, RemoveVirtualScreenBlockList, TestSize.Level1)
{
    std::vector<int32_t> persistentIds {0, 1, 2};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->RemoveVirtualScreenBlockList(persistentIds));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->RemoveVirtualScreenBlockList(persistentIds));
    }
}

/**
 * @tc.name: SetVirtualMirrorScreenCanvasRotation
 * @tc.desc: SetVirtualMirrorScreenCanvasRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualMirrorScreenCanvasRotation, TestSize.Level1)
{
    ScreenId ScreenId = 1001;
    bool canvasRotation = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
            screenSessionManagerProxy->SetVirtualMirrorScreenCanvasRotation(ScreenId, canvasRotation));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
            screenSessionManagerProxy->SetVirtualMirrorScreenCanvasRotation(ScreenId, canvasRotation));
    }
}

/**
 * @tc.name: SetVirtualMirrorScreenScaleMode
 * @tc.desc: SetVirtualMirrorScreenScaleMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualMirrorScreenScaleMode, TestSize.Level1)
{
    ScreenId ScreenId = 1001;
    ScreenScaleMode scaleMode {0};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetVirtualMirrorScreenScaleMode(ScreenId, scaleMode));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetVirtualMirrorScreenScaleMode(ScreenId, scaleMode));
    }
}

/**
 * @tc.name: ResizeVirtualScreen
 * @tc.desc: ResizeVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, ResizeVirtualScreen, TestSize.Level1)
{
    ScreenId ScreenId = 1001;
    uint32_t width = 1024;
    uint32_t height = 1024;
    EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
              screenSessionManagerProxy->ResizeVirtualScreen(ScreenId, width, height));
}

/**
 * @tc.name: DestroyVirtualScreen
 * @tc.desc: DestroyVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, DestroyVirtualScreen, TestSize.Level1)
{
    ScreenId ScreenId = 1001;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->DestroyVirtualScreen(ScreenId));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->DestroyVirtualScreen(ScreenId));
    }
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, MakeMirror, TestSize.Level1)
{
    ScreenId Id = 1001;
    std::vector<ScreenId> mirrorScreenIds = {1003, 1004, 1005};
    ScreenId screenGroupId = 1002;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->MakeMirror(Id, mirrorScreenIds, screenGroupId));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->MakeMirror(Id, mirrorScreenIds, screenGroupId));
    }
}

/**
 * @tc.name: StopMirror
 * @tc.desc: StopMirror
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, StopMirror, TestSize.Level1)
{
    const std::vector<ScreenId> mirrorScreenIds = {1003, 1004, 1005};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->StopMirror(mirrorScreenIds));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->StopMirror(mirrorScreenIds));
    }
}

/**
 * @tc.name: DisableMirror
 * @tc.desc: DisableMirror
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, DisableMirror, TestSize.Level1)
{
    bool disableOrNot = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->DisableMirror(disableOrNot));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->DisableMirror(disableOrNot));
    }
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: MakeExpand
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, MakeExpand, TestSize.Level1)
{
    std::vector<ScreenId> screenId = {1003, 1004, 1005};
    std::vector<Point> startPoint {};
    ScreenId screenGroupId = 1001;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->MakeExpand(screenId, startPoint, screenGroupId));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->MakeExpand(screenId, startPoint, screenGroupId));
    }
}

/**
 * @tc.name: StopExpand
 * @tc.desc: StopExpand
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, StopExpand, TestSize.Level1)
{
    const std::vector<ScreenId> expandScreenIds = {1003, 1004, 1005};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->StopExpand(expandScreenIds));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->StopExpand(expandScreenIds));
    }
}

/**
 * @tc.name: GetScreenGroupInfoById
 * @tc.desc: GetScreenGroupInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenGroupInfoById, TestSize.Level1)
{
    sptr<ScreenGroupInfo> expectation = nullptr;
    sptr<ScreenGroupInfo> res = nullptr;
    ScreenId Id {0};
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetScreenGroupInfoById(Id);
    };
    func();
    ASSERT_EQ(res, expectation);
}

/**
 * @tc.name: RemoveVirtualScreenFromGroup
 * @tc.desc: RemoveVirtualScreenFromGroup
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, RemoveVirtualScreenFromGroup, TestSize.Level1)
{
    std::vector<ScreenId> screens = {1002, 1003, 1004};
    screenSessionManagerProxy->RemoveVirtualScreenFromGroup(screens);
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(2000);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: GetDisplaySnapshot
 * @tc.desc: GetDisplaySnapshot
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDisplaySnapshot, TestSize.Level1)
{
    std::shared_ptr<Media::PixelMap> expectation = nullptr;
    std::shared_ptr<Media::PixelMap> res = nullptr;
    DisplayId displayId {0};
    DmErrorCode* errorCode = nullptr;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetDisplaySnapshot(displayId, errorCode, false, false);
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
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDisplayInfoById, TestSize.Level1)
{
    sptr<DisplayInfo> expectation = nullptr;
    sptr<DisplayInfo> res = nullptr;
    DisplayId displayId {0};
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetDisplayInfoById(displayId);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: GetDisplayInfoByScreen
 * @tc.desc: GetDisplayInfoByScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDisplayInfoByScreen, TestSize.Level1)
{
    sptr<DisplayInfo> expectation = nullptr;
    sptr<DisplayInfo> res = nullptr;
    ScreenId screenId {0};
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetDisplayInfoByScreen(screenId);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: GetAllDisplayIds
 * @tc.desc: GetAllDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetAllDisplayIds, TestSize.Level1)
{
    int resultValue = 0;
    std::function<void()> func = [&]()
    {
        screenSessionManagerProxy->GetAllDisplayIds();
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: GetScreenInfoById
 * @tc.desc: GetScreenInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenInfoById, TestSize.Level1)
{
    sptr<ScreenInfo> expectation = nullptr;
    sptr<ScreenInfo> res = nullptr;
    ScreenId Id {0};
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetScreenInfoById(Id);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: GetAllScreenInfos
 * @tc.desc: GetAllScreenInfos
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetAllScreenInfos, TestSize.Level1)
{
    std::vector<sptr<ScreenInfo>> screenInfos {};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->GetAllScreenInfos(screenInfos));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->GetAllScreenInfos(screenInfos));
    }
}

/**
 * @tc.name: GetScreenSupportedColorGamuts
 * @tc.desc: GetScreenSupportedColorGamuts
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenSupportedColorGamuts, TestSize.Level1)
{
    ScreenId screenId = 1001;
    std::vector<ScreenColorGamut> colorGamuts {};
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->GetScreenSupportedColorGamuts(screenId, colorGamuts));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->GetScreenSupportedColorGamuts(screenId, colorGamuts));
    }
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetOrientation, TestSize.Level1)
{
    ScreenId screenId = 1001;
    Orientation orientation = Orientation::BEGIN;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetOrientation(screenId, orientation, false));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetOrientation(screenId, orientation, false));
    }
}

/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: SetScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetScreenRotationLocked(isLocked));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetScreenRotationLocked(isLocked));
    }
}

/**
 * @tc.name: SetScreenRotationLockedFromJs
 * @tc.desc: SetScreenRotationLockedFromJs
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenRotationLockedFromJs, TestSize.Level1)
{
    bool isLocked = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetScreenRotationLockedFromJs(isLocked));
    } else {
        ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->SetScreenRotationLockedFromJs(isLocked));
    }
}

/**
 * @tc.name: SetMultiScreenMode
 * @tc.desc: SetMultiScreenMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetMultiScreenMode, TestSize.Level1)
{
    ScreenId mainScreenId = 0;
    ScreenId secondaryScreenId = 1;
    MultiScreenMode screenMode = MultiScreenMode::SCREEN_EXTEND;

    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetMultiScreenMode(mainScreenId, secondaryScreenId, screenMode);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: SetMultiScreenRelativePosition
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetMultiScreenRelativePosition, TestSize.Level1)
{
    ScreenId testId = 2060;
    ScreenId testId1 = 3060;
    MultiScreenPositionOptions mainScreenOptions = {testId, 100, 100};
    MultiScreenPositionOptions secondScreenOption = {testId1, 100, 100};

    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: SetVirtualScreenMaxRefreshRate
 * @tc.desc: SetVirtualScreenMaxRefreshRate
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetVirtualScreenMaxRefreshRate, TestSize.Level1)
{
    ScreenId id = 0;
    uint32_t refreshRate = 12;
    uint32_t actualRefreshRate = 0;

    DMError res = DMError::DM_ERROR_NOT_SYSTEM_APP;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->SetVirtualScreenMaxRefreshRate(id, refreshRate, actualRefreshRate);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_NE(res, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: IsScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, IsScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->IsScreenRotationLocked(isLocked));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->IsScreenRotationLocked(isLocked));
    }
}

/**
 * @tc.name: GetCutoutInfo
 * @tc.desc: GetCutoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetCutoutInfo, TestSize.Level1)
{
    sptr<CutoutInfo> expectation = nullptr;
    sptr<CutoutInfo> res = nullptr;
    DisplayId displayId = 0;
    std::function<void()> func = [&]()
    {
        res = screenSessionManagerProxy->GetCutoutInfo(displayId);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, expectation);
    } else {
        ASSERT_EQ(res, expectation);
    }
}

/**
 * @tc.name: HasImmersiveWindow
 * @tc.desc: HasImmersiveWindow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, HasImmersiveWindow, TestSize.Level1)
{
    bool immersive = true;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->HasImmersiveWindow(0u, immersive));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->HasImmersiveWindow(0u, immersive));
    }
}

/**
 * @tc.name: ConvertScreenIdToRsScreenId
 * @tc.desc: ConvertScreenIdToRsScreenId
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, ConvertScreenIdToRsScreenId, TestSize.Level1)
{
    ScreenId screenId = 1001;
    ScreenId rsScreenId = 1002;
    auto ret = screenSessionManagerProxy->ConvertScreenIdToRsScreenId(screenId, rsScreenId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: HasPrivateWindow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, HasPrivateWindow, TestSize.Level1)
{
    bool hasPrivateWindow = true;
    DisplayId displayId = 0;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->HasPrivateWindow(displayId, hasPrivateWindow));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->HasPrivateWindow(displayId, hasPrivateWindow));
    }
}

/**
 * @tc.name: DumpAllScreensInfo
 * @tc.desc: DumpAllScreensInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, DumpAllScreensInfo, TestSize.Level1)
{
    std::string dumpInfo;
    screenSessionManagerProxy->DumpAllScreensInfo(dumpInfo);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(dumpInfo, "");
    } else {
        EXPECT_EQ(dumpInfo, "");
    }
}

/**
 * @tc.name: DumpSpecialScreenInfo
 * @tc.desc: DumpSpecialScreenInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, DumpSpecialScreenInfo, TestSize.Level1)
{
    ScreenId id = 1001;
    std::string dumpInfo;
    screenSessionManagerProxy->DumpSpecialScreenInfo(id, dumpInfo);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(dumpInfo, "");
    } else {
        EXPECT_EQ(dumpInfo, "");
    }
}

/**
 * @tc.name: SetFoldDisplayMode
 * @tc.desc: SetFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetFoldDisplayMode, TestSize.Level1)
{
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    screenSessionManagerProxy->SetFoldDisplayMode(displayMode);
    if (screenSessionManagerProxy->IsFoldable() && !FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_NE(ScreenSessionManager::GetInstance().foldScreenController_, nullptr);
    } else {
        EXPECT_EQ(ScreenSessionManager::GetInstance().foldScreenController_, nullptr);
    }
}

/**
 * @tc.name: SetFoldDisplayModeFromJs
 * @tc.desc: SetFoldDisplayModeFromJs
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetFoldDisplayModeFromJs, TestSize.Level1)
{
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->SetFoldDisplayModeFromJs(displayMode));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED, screenSessionManagerProxy->SetFoldDisplayModeFromJs(displayMode));
    }
}

/**
 * @tc.name: SetFoldStatusLocked
 * @tc.desc: SetFoldStatusLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetFoldStatusLocked, TestSize.Level1)
{
    bool locked = true;
    screenSessionManagerProxy->SetFoldStatusLocked(locked);
    if (screenSessionManagerProxy->IsFoldable() && !FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_NE(ScreenSessionManager::GetInstance().foldScreenController_, nullptr);
    } else {
        EXPECT_EQ(ScreenSessionManager::GetInstance().foldScreenController_, nullptr);
    }
}

/**
 * @tc.name: GetFoldDisplayMode
 * @tc.desc: GetFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetFoldDisplayMode, TestSize.Level1)
{
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, screenSessionManagerProxy->GetFoldDisplayMode());
}

/**
 * @tc.name: IsFoldable
 * @tc.desc: IsFoldable
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, IsFoldable, TestSize.Level1)
{
    screenSessionManagerProxy->IsFoldable();
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(2000);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: IsCaptured
 * @tc.desc: IsCaptured
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, IsCaptured, TestSize.Level1)
{
    auto ret = screenSessionManagerProxy->IsCaptured();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetFoldStatus
 * @tc.desc: GetFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetFoldStatus, TestSize.Level1)
{
    auto foldStatus = screenSessionManagerProxy->GetFoldStatus();
    EXPECT_EQ(foldStatus, FoldStatus::UNKNOWN);
}

/**
 * @tc.name: GetSuperFoldStatus
 * @tc.desc: GetSuperFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetSuperFoldStatus, TestSize.Level1)
{
    auto superFoldStatus = screenSessionManagerProxy->GetSuperFoldStatus();
    EXPECT_EQ(superFoldStatus, SuperFoldStatus::UNKNOWN);
}

/**
 * @tc.name: SetLandscapeLockStatus
 * @tc.desc: SetLandscapeLockStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetLandscapeLockStatus, Function | SmallTest | Level1)
{
    bool isLocked = false;
    screenSessionManagerProxy->SetLandscapeLockStatus(isLocked);
    EXPECT_EQ(isLocked, false);
}

/**
 * @tc.name: GetCurrentFoldCreaseRegion
 * @tc.desc: GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
{
    auto foldCreaseRegion = screenSessionManagerProxy->GetCurrentFoldCreaseRegion();
    EXPECT_EQ(foldCreaseRegion, nullptr);
}

/**
 * @tc.name: MakeUniqueScreen
 * @tc.desc: MakeUniqueScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, MakeUniqueScreen, TestSize.Level1)
{
    const std::vector<ScreenId> screenIds {1001, 1002, 1003};
    std::vector<DisplayId> displayIds;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, screenSessionManagerProxy->MakeUniqueScreen(screenIds, displayIds,
        UniqueScreenRotationOptions()));
}

/**
 * @tc.name: SetClient
 * @tc.desc: SetClient
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetClient, TestSize.Level1)
{
    const sptr<IScreenSessionManagerClient> client = nullptr;
    screenSessionManagerProxy->SetClient(client);
    EXPECT_EQ(client, nullptr);
}

/**
 * @tc.name: SwitchUser
 * @tc.desc: SwitchUser
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SwitchUser, TestSize.Level1)
{
    screenSessionManagerProxy->SwitchUser();
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(2000);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: GetScreenProperty
 * @tc.desc: GetScreenProperty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenProperty, TestSize.Level1)
{
    ScreenId screenId = 1001;
    screenSessionManagerProxy->GetScreenProperty(screenId);
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(2000);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: GetDisplayHookInfo
 * @tc.desc: GetDisplayHookInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDisplayHookInfo, Function | SmallTest | Level1)
{
    int32_t uid = 0;
    DMHookInfo hookInfo;
    hookInfo.height_ = 1344;
    hookInfo.width_ = 2772;
    std::function<void()> func = [&]()
    {
        screenSessionManagerProxy->GetDisplayHookInfo(uid, hookInfo);
    };
    func();

    EXPECT_EQ(hookInfo.height_, 1344);
    EXPECT_EQ(hookInfo.width_, 2772);
}

/**
 * @tc.name: GetScreenCapture
 * @tc.desc: GetScreenCapture test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetScreenCapture, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerProxy != nullptr);

    std::shared_ptr<Media::PixelMap> res = nullptr;
    CaptureOption option;
    option.displayId_ = 0;
    DmErrorCode errorCode = DmErrorCode::DM_OK;
    std::function<void()> func = [&]() {
        res = screenSessionManagerProxy->GetScreenCapture(option, &errorCode);
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (errorCode == DmErrorCode::DM_OK) {
            ASSERT_NE(res, nullptr);
        } else {
            ASSERT_EQ(res, nullptr);
        }
    } else {
        ASSERT_EQ(res, nullptr);
    }
}

/**
 * @tc.name: GetPrimaryDisplayInfo
 * @tc.desc: GetPrimaryDisplayInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetPrimaryDisplayInfo, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerProxy != nullptr);

    sptr<DisplayInfo> res = nullptr;
    std::function<void()> func = [&]() {
        res = screenSessionManagerProxy->GetPrimaryDisplayInfo();
    };
    func();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(res, nullptr);
    } else {
        ASSERT_EQ(res, nullptr);
    }
}

/**
 * @tc.name: SetScreenSkipProtectedWindow
 * @tc.desc: SetScreenSkipProtectedWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, SetScreenSkipProtectedWindow, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerProxy != nullptr);

    const std::vector<ScreenId> screenIds = {1001, 1002};
    bool isEnable = true;
    int resultValue = 0;
    std::function<void()> func = [&]() {
        screenSessionManagerProxy->SetScreenSkipProtectedWindow(screenIds, isEnable);
        resultValue = 1;
    };
    func();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: GetDisplayCapability
 * @tc.desc: GetDisplayCapability test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyUtTest, GetDisplayCapability, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerProxy != nullptr);
    std::string capabilitInfo;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->GetDisplayCapability(capabilitInfo));
    } else {
        EXPECT_EQ(DMError::DM_ERROR_IPC_FAILED,
                screenSessionManagerProxy->GetDisplayCapability(capabilitInfo));
    }
}
}
}
