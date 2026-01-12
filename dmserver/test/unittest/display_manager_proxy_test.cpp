/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <iremote_broker.h>
#include <iremote_object.h>

#include "display_manager_agent_default.h"
#include "display_manager_proxy.h"
#include "iconsumer_surface.h"
#include "iremote_object_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using RemoteMocker = MockIRemoteObject;
class DisplayManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    int32_t dmError_ = static_cast<int32_t>(DMError::DM_ERROR_UNKNOWN);
};

void DisplayManagerProxyTest::SetUpTestCase()
{
}

void DisplayManagerProxyTest::TearDownTestCase()
{
}

void DisplayManagerProxyTest::SetUp()
{
}

void DisplayManagerProxyTest::TearDown()
{
    dmError_ = static_cast<int32_t>(DMError::DM_ERROR_UNKNOWN);
}

namespace {
/**
 * @tc.name: GetDefaultDisplayInfo
 * @tc.desc: test DisplayManagerProxy::GetDefaultDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetDefaultDisplayInfo01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = proxy1.GetDefaultDisplayInfo(displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetDefaultDisplayInfo(displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetDefaultDisplayInfo(displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetDisplayInfoById01
 * @tc.desc: test DisplayManagerProxy::GetDisplayInfoById
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetDisplayInfoById01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = proxy1.GetDisplayInfoById(0, displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetDisplayInfoById(0, displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetDisplayInfoById(0, displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetDisplayInfoByScreen01
 * @tc.desc: test DisplayManagerProxy::GetDisplayInfoByScreen
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetDisplayInfoByScreen01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = proxy1.GetDisplayInfoByScreen(0, displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetDisplayInfoByScreen(0, displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetDisplayInfoByScreen(0, displayInfo);
    ASSERT_EQ(nullptr, displayInfo);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: CreateVirtualScreen01
 * @tc.desc: test DisplayManagerProxy::CreateVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, CreateVirtualScreen01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    VirtualScreenOption virtualOption1{};
    virtualOption1.name_ = "testVirtualOption";
    DmVirtualScreenOption dmVirtualScreenOption1(virtualOption1);
    sptr<IRemoteObject> displayManagerAgent = new RemoteMocker();
    ScreenId screenId = SCREEN_ID_INVALID;
    ErrCode errCode = proxy1.CreateVirtualScreen(dmVirtualScreenOption1, displayManagerAgent, screenId);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    VirtualScreenOption virtualOption2{};
    virtualOption2.name_ = "testVirtualOption";
    DmVirtualScreenOption dmVirtualScreenOption2(virtualOption2);
    errCode = proxy2.CreateVirtualScreen(dmVirtualScreenOption2, nullptr, screenId);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);
    errCode = proxy2.CreateVirtualScreen(dmVirtualScreenOption2, displayManagerAgent, screenId);
    EXPECT_EQ(ERR_OK, errCode);

    screenId = SCREEN_ID_INVALID;
    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.CreateVirtualScreen(dmVirtualScreenOption2, displayManagerAgent, screenId);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: CreateVirtualScreen02
 * @tc.desc: test DisplayManagerProxy::CreateVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, CreateVirtualScreen02, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    VirtualScreenOption virtualOption1{};
    virtualOption1.name_ = "testVirtualOption";
    DmVirtualScreenOption dmVirtualScreenOption1(virtualOption1);
    sptr<IRemoteObject> displayManagerAgent = new RemoteMocker();
    ScreenId screenId = SCREEN_ID_INVALID;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    ErrCode errCode = proxy1.CreateVirtualScreen(dmVirtualScreenOption1, displayManagerAgent, screenId,
        surface->GetProducer());
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    VirtualScreenOption virtualOption2{};
    virtualOption2.name_ = "testVirtualOption";
    DmVirtualScreenOption dmVirtualScreenOption2(virtualOption2);
    errCode = proxy2.CreateVirtualScreen(dmVirtualScreenOption2, nullptr, screenId, surface->GetProducer());
    EXPECT_EQ(ERR_INVALID_DATA, errCode);
    errCode = proxy2.CreateVirtualScreen(dmVirtualScreenOption2, displayManagerAgent, screenId, surface->GetProducer());
    EXPECT_EQ(ERR_OK, errCode);

    screenId = SCREEN_ID_INVALID;
    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.CreateVirtualScreen(dmVirtualScreenOption2, displayManagerAgent, screenId, surface->GetProducer());
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: DestroyVirtualScreen01
 * @tc.desc: test DisplayManagerProxy::DestroyVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, DestroyVirtualScreen01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.DestroyVirtualScreen(0, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.DestroyVirtualScreen(0, dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.DestroyVirtualScreen(0, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetVirtualScreenSurface01
 * @tc.desc: test DisplayManagerProxy::SetVirtualScreenSurface
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetVirtualScreenSurface01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetVirtualScreenSurface(0, nullptr, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetVirtualScreenSurface(0, nullptr, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    errCode = proxy2.SetVirtualScreenSurface(0, surface->GetProducer(), dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetVirtualScreenSurface(0, surface->GetProducer(), dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetOrientation01
 * @tc.desc: test DisplayManagerProxy::SetOrientation
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetOrientation01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetOrientation(0, static_cast<uint32_t>(Orientation::VERTICAL), dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetOrientation(0, static_cast<uint32_t>(Orientation::VERTICAL), dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetOrientation(0, static_cast<uint32_t>(Orientation::VERTICAL), dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetDisplaySnapshot01
 * @tc.desc: test DisplayManagerProxy::GetDisplaySnapshot
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetDisplaySnapshot01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    int32_t errorCode;
    std::shared_ptr<Media::PixelMap> pixelMap;
    ErrCode errCode = proxy1.GetDisplaySnapshot(0, errorCode, false, false, pixelMap);
    ASSERT_EQ(nullptr, pixelMap);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetDisplaySnapshot(0, errorCode, false, false, pixelMap);
    ASSERT_EQ(nullptr, pixelMap);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetDisplaySnapshot(0, errorCode, false, false, pixelMap);
    ASSERT_EQ(nullptr, pixelMap);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetScreenSupportedColorGamuts01
 * @tc.desc: test DisplayManagerProxy::GetScreenSupportedColorGamuts
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetScreenSupportedColorGamuts01, TestSize.Level1)
{
    std::vector<uint32_t> gamutVector;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.GetScreenSupportedColorGamuts(0, gamutVector, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);
    gamutVector.clear();

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetScreenSupportedColorGamuts(0, gamutVector, dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetScreenSupportedColorGamuts(0, gamutVector, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetScreenColorGamut01
 * @tc.desc: test DisplayManagerProxy::GetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetScreenColorGamut01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    uint32_t screenColorGamut;
    ErrCode errCode = proxy1.GetScreenColorGamut(0, screenColorGamut, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    screenColorGamut = static_cast<uint32_t>(ScreenColorGamut::COLOR_GAMUT_ADOBE_RGB);
    errCode = proxy2.GetScreenColorGamut(0, screenColorGamut, dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    EXPECT_EQ(ERR_OK, errCode);
    ASSERT_EQ(static_cast<uint32_t>(ScreenColorGamut::COLOR_GAMUT_NATIVE), screenColorGamut);

    screenColorGamut = static_cast<uint32_t>(ScreenColorGamut::COLOR_GAMUT_ADOBE_RGB);
    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetScreenColorGamut(0, screenColorGamut, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenColorGamut01
 * @tc.desc: test DisplayManagerProxy::SetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenColorGamut01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetScreenColorGamut(0, 3, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetScreenColorGamut(0, 3, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenColorGamut(0, 3, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetScreenGamutMap01
 * @tc.desc: test DisplayManagerProxy::GetScreenGamutMap
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetScreenGamutMap01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto gamutMap = static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION);
    ErrCode errCode = proxy1.GetScreenGamutMap(0, gamutMap, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    gamutMap = static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION);
    errCode = proxy2.GetScreenGamutMap(0, gamutMap, dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    ASSERT_EQ(static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_CONSTANT), gamutMap);
    EXPECT_EQ(ERR_OK, errCode);

    gamutMap = static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION);
    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetScreenGamutMap(0, gamutMap, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenGamutMap01
 * @tc.desc: test DisplayManagerProxy::SetScreenGamutMap
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenGamutMap01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto gamutMap = static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION);
    ErrCode errCode = proxy1.SetScreenGamutMap(0, gamutMap, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    gamutMap = static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION);
    errCode = proxy2.SetScreenGamutMap(0, gamutMap, dmError_);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    gamutMap = static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION);
    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenGamutMap(0, gamutMap, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenColorTransform01
 * @tc.desc: test DisplayManagerProxy::SetScreenColorTransform
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenColorTransform01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetScreenColorTransform(0);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetScreenColorTransform(0);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenColorTransform(0);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: RegisterDisplayManagerAgent01
 * @tc.desc: test DisplayManagerProxy::RegisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, RegisterDisplayManagerAgent01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    auto type = static_cast<uint32_t>(DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
    ErrCode errCode = proxy1.RegisterDisplayManagerAgent(displayManagerAgent, type, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.RegisterDisplayManagerAgent(nullptr, type, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);
    errCode = proxy2.RegisterDisplayManagerAgent(displayManagerAgent, type, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.RegisterDisplayManagerAgent(displayManagerAgent, type, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: UnregisterDisplayManagerAgent01
 * @tc.desc: test DisplayManagerProxy::UnregisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, UnregisterDisplayManagerAgent01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    auto type = static_cast<uint32_t>(DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
    ErrCode errCode = proxy1.UnregisterDisplayManagerAgent(displayManagerAgent, type, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.UnregisterDisplayManagerAgent(nullptr, type, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);
    errCode = proxy2.UnregisterDisplayManagerAgent(displayManagerAgent, type, dmError_);
    EXPECT_EQ(ERR_OK, errCode);
    ASSERT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError_);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.UnregisterDisplayManagerAgent(displayManagerAgent, type, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: WakeUpBegin01
 * @tc.desc: test DisplayManagerProxy::WakeUpBegin
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, WakeUpBegin01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto reason = static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON);
    bool isSucc = false;
    ErrCode errCode = proxy1.WakeUpBegin(reason, isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.WakeUpBegin(reason, isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.WakeUpBegin(reason, isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: WakeUpEnd01
 * @tc.desc: test DisplayManagerProxy::WakeUpEnd
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, WakeUpEnd01, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    bool isSucc = false;
    ErrCode errCode = proxy1.WakeUpEnd(isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.WakeUpEnd(isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.WakeUpEnd(isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SuspendBegin
 * @tc.desc: test DisplayManagerProxy::SuspendBegin
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SuspendBegin, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto reason = static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON);
    bool isSucc = false;
    ErrCode errCode = proxy1.SuspendBegin(reason, isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SuspendBegin(reason, isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SuspendBegin(reason, isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SuspendEnd
 * @tc.desc: test DisplayManagerProxy::SuspendEnd
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SuspendEnd, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    bool isSucc = false;
    ErrCode errCode = proxy1.SuspendEnd(isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SuspendEnd(isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SuspendEnd(isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenPowerForAll
 * @tc.desc: test DisplayManagerProxy::SetScreenPowerForAll
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenPowerForAll, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto state = static_cast<uint32_t>(ScreenPowerState::POWER_ON);
    auto reason = static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON);
    bool isSucc = false;
    ErrCode errCode = proxy1.SetScreenPowerForAll(state, reason, isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetScreenPowerForAll(state, reason, isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenPowerForAll(state, reason, isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: test DisplayManagerProxy::SetSpecifiedScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetSpecifiedScreenPower, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto state = static_cast<uint32_t>(ScreenPowerState::POWER_ON);
    auto reason = static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON);
    bool isSucc = false;
    ErrCode errCode = proxy1.SetSpecifiedScreenPower(0, state, reason, isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetSpecifiedScreenPower(0, state, reason, isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetSpecifiedScreenPower(0, state, reason, isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetDisplayState
 * @tc.desc: test DisplayManagerProxy::SetDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetDisplayState, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto state = static_cast<uint32_t>(DisplayState::UNKNOWN);
    bool isSucc = false;
    ErrCode errCode = proxy1.SetDisplayState(state, isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetDisplayState(state, isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetDisplayState(state, isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: AddSurfaceNodeToDisplay
 * @tc.desc: test DisplayManagerProxy::AddSurfaceNodeToDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, AddSurfaceNodeToDisplay, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = std::make_shared<RSSurfaceNode>(rsSurfaceNodeConfig, true, 0);
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    ErrCode errCode = proxy1.AddSurfaceNodeToDisplay(0, dmRsSurfaceNode, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.AddSurfaceNodeToDisplay(0, dmRsSurfaceNode, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.AddSurfaceNodeToDisplay(0, dmRsSurfaceNode, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: RemoveSurfaceNodeFromDisplay
 * @tc.desc: test DisplayManagerProxy::RemoveSurfaceNodeFromDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, RemoveSurfaceNodeFromDisplay, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = std::make_shared<RSSurfaceNode>(rsSurfaceNodeConfig, true, 0);
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    ErrCode errCode = proxy1.RemoveSurfaceNodeFromDisplay(0, dmRsSurfaceNode, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.RemoveSurfaceNodeFromDisplay(0, dmRsSurfaceNode, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.RemoveSurfaceNodeFromDisplay(0, dmRsSurfaceNode, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: test DisplayManagerProxy::HasPrivateWindow
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, HasPrivateWindow, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    bool hasPrivateWindow = false;
    ErrCode errCode = proxy1.HasPrivateWindow(0, hasPrivateWindow, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.HasPrivateWindow(0, hasPrivateWindow, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.HasPrivateWindow(0, hasPrivateWindow, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetFreeze
 * @tc.desc: test DisplayManagerProxy::SetFreeze
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetFreeze, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    std::vector<uint64_t> displayIds;
    bool isFreeze = true;
    bool isSucc = false;
    ErrCode errCode = proxy1.SetFreeze(displayIds, isFreeze, isSucc);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetFreeze(displayIds, isFreeze, isSucc);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetFreeze(displayIds, isFreeze, isSucc);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetDisplayState
 * @tc.desc: test DisplayManagerProxy::GetDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetDisplayState, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    DisplayId displayId = 0;
    uint32_t displayState = 0;
    ErrCode errCode = proxy1.GetDisplayState(displayId, displayState);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetDisplayState(displayId, displayState);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetDisplayState(displayId, displayState);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetScreenPower
 * @tc.desc: test DisplayManagerProxy::GetScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetScreenPower, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    DisplayId displayId = 0;
    uint32_t screenPowerState = 0;
    ErrCode errCode = proxy1.GetScreenPower(displayId, screenPowerState);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetScreenPower(displayId, screenPowerState);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetScreenPower(displayId, screenPowerState);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetAllDisplayIds
 * @tc.desc: test DisplayManagerProxy::GetAllDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetAllDisplayIds, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    std::vector<DisplayId> displayIds;
    ErrCode errCode = proxy1.GetAllDisplayIds(displayIds);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetAllDisplayIds(displayIds);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetAllDisplayIds(displayIds);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetCutoutInfo
 * @tc.desc: test DisplayManagerProxy::GetCutoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetCutoutInfo, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<CutoutInfo> cutoutInfo;
    ErrCode errCode = proxy1.GetCutoutInfo(0, cutoutInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetCutoutInfo(0, cutoutInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetCutoutInfo(0, cutoutInfo);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: NotifyDisplayEvent
 * @tc.desc: test DisplayManagerProxy::NotifyDisplayEvent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, NotifyDisplayEvent, TestSize.Level1)
{
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    auto event = static_cast<uint32_t>(DisplayEvent::UNLOCK);
    ErrCode errCode = proxy1.NotifyDisplayEvent(event);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.NotifyDisplayEvent(event);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.NotifyDisplayEvent(event);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: test DisplayManagerProxy::MakeMirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, MakeMirror, TestSize.Level1)
{
    ScreenId mainScreenId = 0;
    std::vector<ScreenId> mirrorScreenId;
    mirrorScreenId.emplace_back(1001);
    ScreenId screenGroupId = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.MakeMirror(mainScreenId, mirrorScreenId, screenGroupId, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.MakeMirror(mainScreenId, mirrorScreenId, screenGroupId, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.MakeMirror(mainScreenId, mirrorScreenId, screenGroupId, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: StopMirror
 * @tc.desc: test DisplayManagerProxy::StopMirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, StopMirror, TestSize.Level1)
{
    std::vector<ScreenId> mirrorScreenId;
    mirrorScreenId.emplace_back(1001);
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.StopMirror(mirrorScreenId, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.StopMirror(mirrorScreenId, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.StopMirror(mirrorScreenId, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetScreenInfoById
 * @tc.desc: test DisplayManagerProxy::GetScreenInfoById
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetScreenInfoById, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<ScreenInfo> screenInfo;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.GetScreenInfoById(screenId, screenInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetScreenInfoById(screenId, screenInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetScreenInfoById(screenId, screenInfo);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetScreenGroupInfoById
 * @tc.desc: test DisplayManagerProxy::GetScreenGroupInfoById
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetScreenGroupInfoById, TestSize.Level1)
{
    ScreenId screenId = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    sptr<ScreenGroupInfo> screenGroupInfo;
    ErrCode errCode = proxy1.GetScreenGroupInfoById(screenId, screenGroupInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetScreenGroupInfoById(screenId, screenGroupInfo);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetScreenGroupInfoById(screenId, screenGroupInfo);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetAllScreenInfos
 * @tc.desc: test DisplayManagerProxy::GetAllScreenInfos
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetAllScreenInfos, TestSize.Level1)
{
    std::vector<sptr<ScreenInfo>> screenInfos;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.GetAllScreenInfos(screenInfos, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetAllScreenInfos(screenInfos, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetAllScreenInfos(screenInfos, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: test DisplayManagerProxy::MakeExpand
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, MakeExpand, TestSize.Level1)
{
    ScreenId screenId_ = 0;
    std::vector<ScreenId> screenId;
    screenId.push_back(screenId_);
    std::vector<Point> startPoint;
    Point point{0, 0};
    startPoint.push_back(point);
    ScreenId screenGroupId = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.MakeExpand(screenId, startPoint, screenGroupId, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.MakeExpand(screenId, startPoint, screenGroupId, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.MakeExpand(screenId, startPoint, screenGroupId, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: StopExpand
 * @tc.desc: test DisplayManagerProxy::StopExpand
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, StopExpand, TestSize.Level1)
{
    std::vector<ScreenId> expandScreenIds;
    expandScreenIds.push_back(0);
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.StopExpand(expandScreenIds, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.StopExpand(expandScreenIds, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.StopExpand(expandScreenIds, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: RemoveVirtualScreenFromGroup
 * @tc.desc: test DisplayManagerProxy::RemoveVirtualScreenFromGroup
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, RemoveVirtualScreenFromGroup, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    screens.push_back(0);
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.RemoveVirtualScreenFromGroup(screens);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.RemoveVirtualScreenFromGroup(screens);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.RemoveVirtualScreenFromGroup(screens);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenActiveMode
 * @tc.desc: test DisplayManagerProxy::SetScreenActiveMode
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenActiveMode, TestSize.Level1)
{
    ScreenId screenId = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetScreenActiveMode(screenId, 0, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetScreenActiveMode(screenId, 0, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenActiveMode(screenId, 0, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: test DisplayManagerProxy::SetVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetVirtualPixelRatio, TestSize.Level1)
{
    ScreenId screenId = 0;
    float virtualPixelRatio = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetVirtualPixelRatio(screenId, virtualPixelRatio, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetVirtualPixelRatio(screenId, virtualPixelRatio, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetVirtualPixelRatio(screenId, virtualPixelRatio, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetResolution
 * @tc.desc: test DisplayManagerProxy::SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetResolution, TestSize.Level1)
{
    ScreenId screenId = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetResolution(screenId, 50, 100, 1.00, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetResolution(screenId, 50, 100, 1.00, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetResolution(screenId, 50, 100, 1.00, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: GetDensityInCurResolution
 * @tc.desc: test DisplayManagerProxy::GetDensityInCurResolution
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, GetDensityInCurResolution, TestSize.Level1)
{
    ScreenId screenId = 0;
    float virtualPixelRatio = 0;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.GetDensityInCurResolution(screenId, virtualPixelRatio, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.GetDensityInCurResolution(screenId, virtualPixelRatio, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.GetDensityInCurResolution(screenId, virtualPixelRatio, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: test DisplayManagerProxy::IsScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, IsScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = true;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.IsScreenRotationLocked(isLocked, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.IsScreenRotationLocked(isLocked, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.IsScreenRotationLocked(isLocked, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: test DisplayManagerProxy::SetScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = true;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetScreenRotationLocked(isLocked, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetScreenRotationLocked(isLocked, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenRotationLocked(isLocked, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}

/**
 * @tc.name: SetScreenRotationLockedFromJs
 * @tc.desc: test DisplayManagerProxy::SetScreenRotationLockedFromJs
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerProxyTest, SetScreenRotationLockedFromJs, TestSize.Level1)
{
    bool isLocked = true;
    DisplayManagerProxy proxy1(nullptr);
    ASSERT_EQ(nullptr, proxy1.remoteObject_);
    ErrCode errCode = proxy1.SetScreenRotationLockedFromJs(isLocked, dmError_);
    EXPECT_EQ(ERR_INVALID_DATA, errCode);

    sptr<RemoteMocker> remoteMocker = new RemoteMocker();
    DisplayManagerProxy proxy2(remoteMocker);
    ASSERT_EQ(static_cast<sptr<IRemoteObject>>(remoteMocker), proxy2.remoteObject_);
    errCode = proxy2.SetScreenRotationLockedFromJs(isLocked, dmError_);
    EXPECT_EQ(ERR_OK, errCode);

    remoteMocker->sendRequestResult_ = 1;
    errCode = proxy2.SetScreenRotationLockedFromJs(isLocked, dmError_);
    EXPECT_EQ(remoteMocker->sendRequestResult_, errCode);
}
}
} // namespace Rosen
} // namespace OHOS
