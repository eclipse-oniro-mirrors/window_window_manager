/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "display_manager.h"
#include "display_manager_proxy.h"
#include "screen_manager.h"
#include "screen_manager_utils.h"
#include "mock_display_manager_adapter.h"
#include "singleton_mocker.h"
#include "screen_manager.cpp"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<ScreenManagerAdapter, MockScreenManagerAdapter>;
class DmMockScreenListener : public ScreenManager::IScreenListener {
public:
    void OnConnect(ScreenId) override {}
    void OnDisconnect(ScreenId) override {}
    void OnChange(ScreenId) override {}
};

class TestScreenGroupListener : public ScreenManager::IScreenGroupListener {
public:
    void OnChange(const std::vector<ScreenId>&, ScreenGroupChangeEvent) override {};
};

class TestIVirtualScreenGroupListener : public ScreenManager::IVirtualScreenGroupListener {
public:
    void OnMirrorChange(const ChangeInfo& info) override {};
};
class ScreenManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    const std::string defaultName_ = "virtualScreen01";
    const float defaultDensity_ = 2.0;
    const int32_t defaultFlags_ = 0;
    const ScreenId testVirtualScreenId_ = 2;
    const uint32_t testVirtualScreenWidth_ = 1920;
    const uint32_t testVirtualScreenHeight_ = 1080;
    static sptr<Display> defaultDisplay_;
    static uint32_t defaultWidth_;
    static uint32_t defaultHeight_;
};
sptr<Display> ScreenManagerTest::defaultDisplay_ = nullptr;
uint32_t ScreenManagerTest::defaultWidth_ = 480;
uint32_t ScreenManagerTest::defaultHeight_ = 320;

void ScreenManagerTest::SetUpTestCase()
{
    defaultDisplay_ = DisplayManager::GetInstance().GetDefaultDisplay();
    defaultWidth_ = defaultDisplay_->GetWidth();
    defaultHeight_ = defaultDisplay_->GetHeight();
}

void ScreenManagerTest::TearDownTestCase()
{
}

void ScreenManagerTest::SetUp()
{
}

void ScreenManagerTest::TearDown()
{
}

namespace {
/**
 * @tc.name: CreateAndDestroy01
 * @tc.desc: CreateVirtualScreen with invalid option and return invalid screen id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, CreateAndDestroy01, Function | SmallTest | Level1)
{
    VirtualScreenOption wrongOption = {defaultName_, defaultWidth_, defaultHeight_,
                                       defaultDensity_, nullptr, defaultFlags_};
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), CreateVirtualScreen(_, _)).Times(1).WillOnce(Return(SCREEN_ID_INVALID));
    EXPECT_CALL(m->Mock(), DestroyVirtualScreen(_)).Times(1).WillOnce(Return(DMError::DM_ERROR_INVALID_PARAM));
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(wrongOption);
    DMError ret = ScreenManager::GetInstance().DestroyVirtualScreen(id);
    ASSERT_EQ(SCREEN_ID_INVALID, id);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: CreateAndDestroy02
 * @tc.desc: CreateVirtualScreen with valid option and return valid screen id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, CreateAndDestroy02, Function | SmallTest | Level1)
{
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, utils.psurface_, defaultFlags_};
    ScreenId validId = 0;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), CreateVirtualScreen(_, _)).Times(1).WillOnce(Return(validId));
    EXPECT_CALL(m->Mock(), DestroyVirtualScreen(_)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError ret = ScreenManager::GetInstance().DestroyVirtualScreen(id);
    ASSERT_EQ(validId, id);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: MakeExpand_001
 * @tc.desc: Create a virtual screen as expansion of default screen, return default screen id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, MakeExpand_001, Function | SmallTest | Level1)
{
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, utils.psurface_, defaultFlags_};
    ScreenId validId = 0; // default srceenId(0)
    ScreenId virtualScreenId = 1; // VirtualScreen is the second screen(1)
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), CreateVirtualScreen(_, _)).Times(1).WillOnce(Return(virtualScreenId));
    EXPECT_CALL(m->Mock(), DestroyVirtualScreen(_)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), MakeExpand(_, _, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenId vScreenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    std::vector<ExpandOption> options = {{validId, 0, 0}, {vScreenId, defaultWidth_, 0}};
    ScreenId expansionId;
    ScreenManager::GetInstance().MakeExpand(options, expansionId);
    ASSERT_EQ(expansionId, validId);
    DMError ret = ScreenManager::GetInstance().DestroyVirtualScreen(vScreenId);
    ASSERT_EQ(vScreenId, virtualScreenId);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: MakeExpand_002
 * @tc.desc: Makepand with empty ExpandOption, return SCREEN_ID_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, MakeExpand_002, Function | SmallTest | Level1)
{
    ScreenId invalidId = SCREEN_ID_INVALID;
    std::vector<ExpandOption> options = {};
    ScreenId expansionId = SCREEN_ID_INVALID;
    ScreenManager::GetInstance().MakeExpand(options, expansionId);
    ASSERT_EQ(expansionId, invalidId);
}

/**
 * @tc.name: SetSurface01
 * @tc.desc: SetVirtualScreenSurface with valid option and return success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetSurface01, Function | SmallTest | Level1)
{
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId validId = 0;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), CreateVirtualScreen(_, _)).Times(1).WillOnce(Return(validId));
    EXPECT_CALL(m->Mock(), SetVirtualScreenSurface(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), DestroyVirtualScreen(_)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError surfaceRes = ScreenManager::GetInstance().SetVirtualScreenSurface(id, utils.psurface_);
    DMError destroyRes = ScreenManager::GetInstance().DestroyVirtualScreen(id);
    ASSERT_EQ(validId, id);
    ASSERT_EQ(DMError::DM_OK, surfaceRes);
    ASSERT_EQ(DMError::DM_OK, destroyRes);
}

/**
 * @tc.name: SetSurface02
 * @tc.desc: SetVirtualScreenSurface with invalid option and return failed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetSurface02, Function | SmallTest | Level1)
{
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), CreateVirtualScreen(_, _)).Times(1).WillOnce(Return(SCREEN_ID_INVALID));
    EXPECT_CALL(m->Mock(), SetVirtualScreenSurface(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_INVALID_PARAM));
    EXPECT_CALL(m->Mock(), DestroyVirtualScreen(_)).Times(1).WillOnce(Return(DMError::DM_ERROR_INVALID_PARAM));
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError surfaceRes = ScreenManager::GetInstance().SetVirtualScreenSurface(id, utils.psurface_);
    DMError destroyRes = ScreenManager::GetInstance().DestroyVirtualScreen(id);
    ASSERT_EQ(SCREEN_ID_INVALID, id);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, surfaceRes);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, destroyRes);
}

/**
 * @tc.name: OnScreenConnect01
 * @tc.desc: OnScreenConnect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, OnScreenConnect01, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    sptr<ScreenManager::IScreenListener> listener = new DmMockScreenListener();
    ScreenManager::GetInstance().RegisterScreenListener(listener);
    auto screenManagerListener = ScreenManager::GetInstance().pImpl_->screenManagerListener_;
    ASSERT_NE(screenManagerListener, nullptr);
    screenManagerListener->OnScreenConnect(nullptr);
    sptr<ScreenInfo> screenInfo = new ScreenInfo();
    screenInfo->SetScreenId(SCREEN_ID_INVALID);
    screenManagerListener->OnScreenConnect(screenInfo);
    screenInfo->SetScreenId(0);
    screenManagerListener->OnScreenConnect(screenInfo);
    ASSERT_NE(screenManagerListener->pImpl_, nullptr);
    screenManagerListener->pImpl_ = nullptr;
    screenManagerListener->OnScreenConnect(screenInfo);
    ScreenManager::GetInstance().pImpl_->screenManagerListener_ = nullptr;
}

/**
 * @tc.name: OnScreenDisconnect01
 * @tc.desc: OnScreenDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, OnScreenDisconnect01, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    sptr<ScreenManager::IScreenListener> listener = new DmMockScreenListener();
    ScreenManager::GetInstance().RegisterScreenListener(listener);
    auto screenManagerListener = ScreenManager::GetInstance().pImpl_->screenManagerListener_;
    ASSERT_NE(screenManagerListener, nullptr);
    screenManagerListener->OnScreenDisconnect(SCREEN_ID_INVALID);
    ASSERT_NE(screenManagerListener->pImpl_, nullptr);
    screenManagerListener->pImpl_ = nullptr;
    screenManagerListener->OnScreenDisconnect(0);
    ScreenManager::GetInstance().pImpl_->screenManagerListener_ = nullptr;
}

/**
 * @tc.name: OnScreenChange01
 * @tc.desc: OnScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, OnScreenChange01, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    sptr<ScreenManager::IScreenListener> listener = new DmMockScreenListener();
    ScreenManager::GetInstance().RegisterScreenListener(listener);
    auto screenManagerListener = ScreenManager::GetInstance().pImpl_->screenManagerListener_;
    ASSERT_NE(screenManagerListener, nullptr);
    screenManagerListener->OnScreenChange(nullptr, ScreenChangeEvent::UPDATE_ORIENTATION);
    ASSERT_NE(screenManagerListener->pImpl_, nullptr);
    sptr<ScreenInfo> screenInfo = new ScreenInfo();
    screenManagerListener->pImpl_ = nullptr;
    screenManagerListener->OnScreenChange(screenInfo, ScreenChangeEvent::UPDATE_ORIENTATION);
    ScreenManager::GetInstance().pImpl_->screenManagerListener_ = nullptr;
}

/**
 * @tc.name: OnScreenGroupChange01
 * @tc.desc: OnScreenGroupChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, OnScreenGroupChange01, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    sptr<ScreenManager::IScreenListener> listener = new DmMockScreenListener();
    ScreenManager::GetInstance().RegisterScreenListener(listener);
    auto screenManagerListener = ScreenManager::GetInstance().pImpl_->screenManagerListener_;
    ASSERT_NE(screenManagerListener, nullptr);
    std::string trigger;
    std::vector<sptr<ScreenInfo>> screenInfos;
    ScreenGroupChangeEvent groupEvent = ScreenGroupChangeEvent::CHANGE_GROUP;
    screenManagerListener->OnScreenGroupChange(trigger, screenInfos, groupEvent);
    ASSERT_NE(screenManagerListener->pImpl_, nullptr);
    sptr<ScreenInfo> screenInfo = new ScreenInfo();
    screenInfo->SetScreenId(1);
    sptr<ScreenInfo> screenInfo2 = new ScreenInfo();
    screenInfos.emplace_back(screenInfo);
    screenInfos.emplace_back(screenInfo2);
    screenManagerListener->OnScreenGroupChange(trigger, screenInfos, groupEvent);
    screenManagerListener->pImpl_ = nullptr;
    screenManagerListener->OnScreenGroupChange(trigger, screenInfos, groupEvent);
    ScreenManager::GetInstance().pImpl_->screenManagerListener_ = nullptr;
}
/**
 * @tc.name: RemoveVirtualScreenFromGroup
 * @tc.desc: for interface coverage & check func RemoveVirtualScreenFromGroup
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, RemoveVirtualScreenFromGroup, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<ScreenId> testScreens(33, 1);
    auto result = ScreenManager::GetInstance().RemoveVirtualScreenFromGroup(testScreens);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, result);

    testScreens.clear();
    result = ScreenManager::GetInstance().RemoveVirtualScreenFromGroup(testScreens);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, result);

    testScreens.emplace_back(static_cast<ScreenId>(1));
    EXPECT_CALL(m->Mock(), RemoveVirtualScreenFromGroup(_)).Times(1);
    result = ScreenManager::GetInstance().RemoveVirtualScreenFromGroup(testScreens);
    ASSERT_EQ(DMError::DM_OK, result);
}
/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: for interface coverage & check SetScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetScreenRotationLocked, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), SetScreenRotationLocked(_)).Times(1);
    auto result = ScreenManager::GetInstance().SetScreenRotationLocked(true);
    ASSERT_EQ(DMError::DM_OK, result);
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: for interface coverage & check IsScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, IsScreenRotationLocked, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), IsScreenRotationLocked(_)).Times(1).WillOnce(Return(DMError::DM_OK));
    bool result;
    DMError ret;
    ret = ScreenManager::GetInstance().IsScreenRotationLocked(result);
    ASSERT_EQ(DMError::DM_OK, ret);
    EXPECT_CALL(m->Mock(), IsScreenRotationLocked(_)).Times(1).WillOnce(Return(DMError::DM_ERROR_NULLPTR));
    ret = ScreenManager::GetInstance().IsScreenRotationLocked(result);
    ASSERT_TRUE(DMError::DM_OK != ret);
}

/**
 * @tc.name: RegisterScreenGroupListener
 * @tc.desc: for interface coverage and
 *           check RegisterScreenGroupListener & UnregisterScreenGroupListener
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, RegisterScreenGroupListener, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    auto& screenManager = ScreenManager::GetInstance();
    auto result = screenManager.RegisterScreenGroupListener(nullptr);
    ASSERT_TRUE(DMError::DM_OK != result);

    sptr<ScreenManager::IScreenGroupListener> listener = new (std::nothrow)TestScreenGroupListener();
    if (screenManager.pImpl_->screenManagerListener_ == nullptr) {
        EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    }
    result = screenManager.RegisterScreenGroupListener(listener);
    ASSERT_EQ(DMError::DM_OK, result);

    result = screenManager.UnregisterScreenGroupListener(nullptr);
    ASSERT_TRUE(DMError::DM_OK != result);

    auto sizeScreen = screenManager.pImpl_->screenListeners_.size();
    auto sizeScreenGroup = screenManager.pImpl_->screenGroupListeners_.size();
    auto sizeVirtualScreen = screenManager.pImpl_->virtualScreenGroupListeners_.size();
    if (sizeScreenGroup > 1) {
        result = screenManager.UnregisterScreenGroupListener(listener);
        ASSERT_EQ(DMError::DM_OK, result);
    } else if (sizeScreenGroup == 1) {
        if (sizeScreen == 0 && sizeVirtualScreen == 0) {
            EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
        }
        result = screenManager.UnregisterScreenGroupListener(listener);
        ASSERT_EQ(DMError::DM_OK, result);
    }
}
/**
 * @tc.name: RegisterVirtualScreenGroupListener
 * @tc.desc: for interface coverage and
 *           check RegisterVirtualScreenGroupListener & UnregisterVirtualScreenGroupListener
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, RegisterVirtualScreenGroupListener, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    auto& screenManager = ScreenManager::GetInstance();
    auto result = screenManager.RegisterVirtualScreenGroupListener(nullptr);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, result);

    sptr<ScreenManager::IVirtualScreenGroupListener> listener = new (std::nothrow)TestIVirtualScreenGroupListener();
    if (screenManager.pImpl_->screenManagerListener_ == nullptr) {
        EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    }
    result = screenManager.RegisterVirtualScreenGroupListener(listener);
    ASSERT_EQ(DMError::DM_OK, result);

    result = screenManager.UnregisterVirtualScreenGroupListener(nullptr);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, result);

    auto sizeScreen = screenManager.pImpl_->screenListeners_.size();
    auto sizeScreenGroup = screenManager.pImpl_->screenGroupListeners_.size();
    auto sizeVirtualScreen = screenManager.pImpl_->virtualScreenGroupListeners_.size();

    if (sizeVirtualScreen > 1) {
        result = screenManager.UnregisterVirtualScreenGroupListener(listener);
        ASSERT_EQ(DMError::DM_OK, result);
    } else if (sizeVirtualScreen == 1) {
        if (sizeScreen == 0 && sizeScreenGroup == 0) {
            EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
        }
        result = screenManager.UnregisterVirtualScreenGroupListener(listener);
        ASSERT_EQ(DMError::DM_OK, result);
    }
}

/**
 * @tc.name: StopExpand_001
 * @tc.desc: StopExpand_001 id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, StopExpand_001, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds;
    ASSERT_EQ(DMError::DM_OK, ScreenManager::GetInstance().StopExpand(screenIds));
    std::vector<ScreenId> expandScreenIds {0, 1, 2, 3, 4, 5};
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().StopExpand(expandScreenIds);
    ASSERT_EQ(DMError::DM_OK, err);
}

/**
 * @tc.name: StopMirror
 * @tc.desc: StopMirror id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, StopMirror, Function | SmallTest | Level1)
{
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());
    std::vector<ScreenId> screenIds;
    ASSERT_EQ(DMError::DM_OK, ScreenManager::GetInstance().StopMirror(screenIds));
    std::vector<ScreenId> mirrorScreenIds {0, 1, 2, 3, 4, 5};
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().StopMirror(mirrorScreenIds);
    ASSERT_EQ(DMError::DM_OK, err);
}

/**
 * @tc.name: RegisterScreenListener
 * @tc.desc: RegisterScreenListener fun
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, RegisterScreenListener, Function | SmallTest | Level1)
{
    sptr<ScreenManager::IScreenListener> listener = nullptr;
    auto ret = ScreenManager::GetInstance().RegisterScreenListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: RegisterVirtualScreenGroupListener02
 * @tc.desc: RegisterVirtualScreenGroupListener02 fun
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, RegisterVirtualScreenGroupListener02, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    auto& screenManager = ScreenManager::GetInstance();
    auto result = screenManager.RegisterVirtualScreenGroupListener(nullptr);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, result);
    sptr<ScreenManager::IVirtualScreenGroupListener> listener = new (std::nothrow)TestIVirtualScreenGroupListener();
    if (screenManager.pImpl_->screenManagerListener_ == nullptr) {
        EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    }
    result = ScreenManager::GetInstance().RegisterVirtualScreenGroupListener(listener);
    ASSERT_EQ(DMError::DM_OK, result);
}

/**
 * @tc.name: SetVirtualScreenFlag01
 * @tc.desc: SetVirtualScreenFlag01 cast flag
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetVirtualScreenFlag01, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError ret = ScreenManager::GetInstance().SetVirtualScreenFlag(screenId, VirtualScreenFlag::CAST);
    ASSERT_EQ(DMError::DM_OK, ret);
    ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: SetVirtualScreenFlag02
 * @tc.desc: SetVirtualScreenFlag02 max flag
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetVirtualScreenFlag02, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError ret = ScreenManager::GetInstance().SetVirtualScreenFlag(screenId, VirtualScreenFlag::MAX);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ret);
    ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: GetVirtualScreenFlag01
 * @tc.desc: GetVirtualScreenFlag01 get cast
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, GetVirtualScreenFlag01, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError ret = ScreenManager::GetInstance().SetVirtualScreenFlag(screenId, VirtualScreenFlag::CAST);
    ASSERT_EQ(DMError::DM_OK, ret);
    VirtualScreenFlag screenFlag = ScreenManager::GetInstance().GetVirtualScreenFlag(screenId);
    ASSERT_EQ(VirtualScreenFlag::DEFAULT, screenFlag);
    ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: SetVirtualMirrorScreenScaleMode01
 * @tc.desc: SetVirtualMirrorScreenScaleMode01 fun
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetVirtualMirrorScreenScaleMode01, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError ret = ScreenManager::GetInstance().SetVirtualMirrorScreenScaleMode(screenId,
        ScreenScaleMode::FILL_MODE);
    ASSERT_EQ(DMError::DM_OK, ret);
    ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: SetVirtualMirrorScreenScaleMode02
 * @tc.desc: SetVirtualMirrorScreenScaleMode02 fun
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, SetVirtualMirrorScreenScaleMode02, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError ret = ScreenManager::GetInstance().SetVirtualMirrorScreenScaleMode(screenId,
        ScreenScaleMode::UNISCALE_MODE);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(DMError::DM_ERROR_DEVICE_NOT_SUPPORT, ret);
    } else {
        ASSERT_EQ(DMError::DM_OK, ret);
        ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
        ASSERT_EQ(DMError::DM_OK, ret);
    }
}

/**
 * @tc.name: IsCaptured02
 * @tc.desc: IsCaptured02 fun
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, IsCaptured02, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    bool isCapture = DisplayManager::GetInstance().IsCaptured();
    ASSERT_TRUE(isCapture);
    auto ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: IsCaptured03
 * @tc.desc: IsCaptured03 fun
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerTest, IsCaptured03, Function | SmallTest | Level1)
{
    VirtualScreenOption defaultOption = {defaultName_, defaultWidth_, defaultHeight_,
                                         defaultDensity_, nullptr, defaultFlags_};
    ScreenId screenId = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    auto ret = ScreenManager::GetInstance().DestroyVirtualScreen(screenId);
    ASSERT_EQ(DMError::DM_OK, ret);
    bool isCapture = DisplayManager::GetInstance().IsCaptured();
    ASSERT_FALSE(isCapture);
}
}
} // namespace Rosen
} // namespace OHOS