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
#include "root_scene_session.h"
#include "scene_session_manager.h"
#include "session_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class RootSceneSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<SceneSessionManager> ssm_;
};

sptr<SceneSessionManager> RootSceneSessionTest::ssm_ = nullptr;

void LoadContentFuncTest(const std::string&, napi_env, napi_value, AbilityRuntime::Context*) {}

void RootSceneSessionTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void RootSceneSessionTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void RootSceneSessionTest::SetUp() {}

void RootSceneSessionTest::TearDown() {}

namespace {
/**
 * @tc.name: SetLoadContentFunc
 * @tc.desc: test function : SetLoadContentFunc
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, SetLoadContentFunc, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    RootSceneSession::LoadContentFunc loadContentFunc_ = LoadContentFuncTest;
    rootSceneSession.SetLoadContentFunc(loadContentFunc_);
    EXPECT_FALSE(rootSceneSession.IsVisible());
}

/**
 * @tc.name: LoadContentFunc
 * @tc.desc: test function : LoadContentFunc
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, LoadContentFunc, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    std::string strTest("LoadContentFuncTest");
    napi_env nativeEnv_ = nullptr;
    napi_value nativeValue_ = nullptr;
    AbilityRuntime::Context* conText_ = nullptr;
    rootSceneSession.LoadContent(strTest, nativeEnv_, nativeValue_, conText_);
    EXPECT_FALSE(rootSceneSession.IsVisible());
}

/**
 * @tc.name: LoadContent
 * @tc.desc: test function : LoadContent
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, LoadContent, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    std::string strTest("LoadContentFuncTest");
    napi_env nativeEnv_ = nullptr;
    napi_value nativeValue_ = nullptr;
    AbilityRuntime::Context* conText_ = nullptr;
    RootSceneSession::LoadContentFunc loadContentFunc_ = LoadContentFuncTest;
    rootSceneSession.SetLoadContentFunc(loadContentFunc_);
    rootSceneSession.LoadContent(strTest, nativeEnv_, nativeValue_, conText_);
    EXPECT_FALSE(rootSceneSession.IsVisible());
}

/**
 * @tc.name: GetUIContent
 * @tc.desc: get UIContent
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetUIContent, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    std::string strTest("GetUIContentTest");
    napi_env nativeEnv_ = nullptr;
    napi_value nativeValue_ = nullptr;
    int isCalled = false;
    rootSceneSession.SetGetUIContentFunc([&isCalled](DisplayId displayId) -> Ace::UIContent* {
        isCalled = true;
        return nullptr;
    });
    auto uiContent = rootSceneSession.GetUIContent(1);
    EXPECT_EQ(uiContent, nullptr);
    EXPECT_TRUE(isCalled);
}

/**
 * @tc.name: GetSystemAvoidAreaForRoot
 * @tc.desc: test function : GetSystemAvoidAreaForRoot
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetSystemAvoidAreaForRoot, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    AvoidArea avoidArea;
    WSRect rect;
    rootSceneSession.GetSystemAvoidAreaForRoot(rect, avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
}

/**
 * @tc.name: GetKeyboardAvoidAreaForRoot
 * @tc.desc: test function : GetKeyboardAvoidAreaForRoot
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetKeyboardAvoidAreaForRoot, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    AvoidArea avoidArea;
    WSRect rect;
    rootSceneSession.GetKeyboardAvoidAreaForRoot(rect, avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
}

/**
 * @tc.name: GetAINavigationBarAreaForRoot
 * @tc.desc: test function : GetAINavigationBarAreaForRoot
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetAINavigationBarAreaForRoot, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    AvoidArea avoidArea;
    WSRect rect;
    rootSceneSession.GetAINavigationBarAreaForRoot(rect, avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
}

/**
 * @tc.name: GetCutoutAvoidAreaForRoot
 * @tc.desc: test function : GetCutoutAvoidAreaForRoot
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetCutoutAvoidAreaForRoot, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    AvoidArea avoidArea;
    WSRect rect;
    rootSceneSession.GetCutoutAvoidAreaForRoot(rect, avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
}

/**
 * @tc.name: SetRootSessionRect
 * @tc.desc: test function : SetRootSessionRect
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, SetRootSessionRect, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    AvoidArea avoidArea;
    WSRect rect;
    rootSceneSession.SetRootSessionRect(rect);
    EXPECT_EQ(rootSceneSession.GetSessionRect(), rect);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: test function : UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, UpdateAvoidArea, TestSize.Level1)
{
    RootSceneSession rootSceneSession;
    AvoidArea avoidArea;
    auto ret = rootSceneSession.UpdateAvoidArea(sptr<AvoidArea>::MakeSptr(avoidArea), AvoidAreaType::TYPE_SYSTEM);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: test function : GetAvoidAreaByType
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetAvoidAreaByType, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    auto avoidArea = ssm_->rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM, { 0, 0, 0, 0 }, 1);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    avoidArea = ssm_->rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, { 0, 0, 0, 0 }, 1);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    avoidArea = ssm_->rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM_GESTURE, { 0, 0, 0, 0 }, 1);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    avoidArea = ssm_->rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_KEYBOARD, { 0, 0, 0, 0 }, 1);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    avoidArea =
        ssm_->rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, { 0, 0, 0, 0 }, 1);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    avoidArea = ssm_->rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_END, { 0, 0, 0, 0 }, 1);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
}

/**
 * @tc.name: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.desc: test function : GetAvoidAreaByTypeIgnoringVisibility
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetAvoidAreaByTypeIgnoringVisibility, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    using T = std::underlying_type_t<AvoidAreaType>;
    for (T avoidAreaType = static_cast<T>(AvoidAreaType::TYPE_START);
        avoidAreaType < static_cast<T>(AvoidAreaType::TYPE_END); avoidAreaType++) {
        auto type = static_cast<AvoidAreaType>(avoidAreaType);
        auto avoidArea = ssm_->rootSceneSession_->GetAvoidAreaByTypeIgnoringVisibility(type, {0, 0, 0, 0});
        EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    }
}

/**
 * @tc.name: GetSystemAvoidAreaForRoot_01
 * @tc.desc: test function : GetSystemAvoidAreaForRoot_01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetSystemAvoidAreaForRoot_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetSceneSessionVectorByTypeAndDisplayId_ = [](WindowType type, uint64_t displayId) {
        return ssm_->GetSceneSessionVectorByTypeAndDisplayId(type, displayId);
    };
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->rootSceneSession_->onGetStatusBarAvoidHeightFunc_ = [](DisplayId displayId, WSRect& barArea) {};
    SessionInfo statusBarSessionInfo;
    statusBarSessionInfo.abilityName_ = "statusBar";
    statusBarSessionInfo.bundleName_ = "statusBar";
    statusBarSessionInfo.screenId_ = 0;
    sptr<SceneSession> statusBarSession = sptr<SceneSession>::MakeSptr(statusBarSessionInfo, nullptr);
    statusBarSession->property_->SetPersistentId(2);
    statusBarSession->property_->type_ = WindowType::WINDOW_TYPE_STATUS_BAR;
    statusBarSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 123 });
    statusBarSession->isVisible_ = true;
    ssm_->sceneSessionMap_.insert({ statusBarSession->GetPersistentId(), statusBarSession });
    AvoidArea avoidArea;
    ssm_->rootSceneSession_->GetSystemAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    Rect rect = { 0, 0, 1260, 123 };
    EXPECT_EQ(avoidArea.topRect_, rect);
    statusBarSession->isVisible_ = false;
    avoidArea.topRect_ = { 0, 0, 0, 0 };
    ssm_->rootSceneSession_->GetSystemAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    ssm_->rootSceneSession_->GetSystemAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea, true);
    EXPECT_EQ(avoidArea.topRect_, rect);
}

/**
 * @tc.name: GetKeyboardAvoidAreaForRoot_01
 * @tc.desc: test function : GetKeyboardAvoidAreaForRoot_01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetKeyboardAvoidAreaForRoot_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetSceneSessionVectorByType_ = [](WindowType type) {
        return ssm_->GetSceneSessionVectorByType(type);
    };
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->rootSceneSession_->isKeyboardPanelEnabled_ = false;
    SessionInfo keyboardSessionInfo;
    keyboardSessionInfo.abilityName_ = "keyboard";
    keyboardSessionInfo.bundleName_ = "keyboard";
    keyboardSessionInfo.screenId_ = 0;
    sptr<SceneSession> keyboardSession = sptr<SceneSession>::MakeSptr(keyboardSessionInfo, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->property_->type_ = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    keyboardSession->GetLayoutController()->SetSessionRect({ 0, 1700, 1260, 1020 });
    keyboardSession->property_->SetPersistentId(2);
    ssm_->sceneSessionMap_.insert({ keyboardSession->GetPersistentId(), keyboardSession });
    AvoidArea avoidArea;
    ssm_->rootSceneSession_->GetKeyboardAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    Rect rect = { 0, 1700, 1260, 1020 };
    EXPECT_EQ(avoidArea.bottomRect_, rect);
    avoidArea.bottomRect_ = { 0, 0, 0, 0 };
    ssm_->rootSceneSession_->GetKeyboardAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    EXPECT_TRUE(!avoidArea.isEmptyAvoidArea());
    ssm_->rootSceneSession_->isKeyboardPanelEnabled_ = true;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    avoidArea.bottomRect_ = { 0, 0, 0, 0 };
    ssm_->rootSceneSession_->GetKeyboardAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
}

/**
 * @tc.name: GetCutoutAvoidAreaForRoot_01
 * @tc.desc: test function : GetCutoutAvoidAreaForRoot_01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetCutoutAvoidAreaForRoot_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->rootSceneSession_->property_->displayId_ = -1;
    AvoidArea avoidArea;
    ssm_->rootSceneSession_->GetCutoutAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    ssm_->rootSceneSession_->property_->displayId_ = 0;
    ssm_->rootSceneSession_->GetCutoutAvoidAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
}

/**
 * @tc.name: GetAINavigationBarAreaForRoot_01
 * @tc.desc: test function : GetAINavigationBarAreaForRoot_01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetAINavigationBarAreaForRoot_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetAINavigationBarArea_ = [](uint64_t displayId, bool ignoreVisibility) {
        return ssm_->GetAINavigationBarArea(displayId, ignoreVisibility);
    };
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    AvoidArea avoidArea;
    ssm_->rootSceneSession_->GetAINavigationBarAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    EXPECT_TRUE(avoidArea.isEmptyAvoidArea());
    ssm_->isAINavigationBarVisible_[0] = true;
    ssm_->currAINavigationBarAreaMap_[0] = { 409, 2629, 442, 91 };
    ssm_->rootSceneSession_->GetAINavigationBarAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea);
    Rect rect = { 409, 2629, 442, 91 };
    EXPECT_EQ(avoidArea.bottomRect_, rect);
    ssm_->isAINavigationBarVisible_[0] = false;
    ssm_->rootSceneSession_->GetAINavigationBarAreaForRoot(ssm_->rootSceneSession_->GetSessionRect(), avoidArea, true);
    EXPECT_EQ(avoidArea.bottomRect_, rect);
}

/**
 * @tc.name: SetRootSessionRect_01
 * @tc.desc: test function : SetRootSessionRect_01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, SetRootSessionRect_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->rootSceneSession_->SetRootSessionRect({ 0, 0, 1260, 2720 });
    WSRect rect = { 0, 0, 1260, 2720 };
    EXPECT_EQ(ssm_->rootSceneSession_->GetSessionRect(), rect);
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 2720, 1260 });
    ssm_->rootSceneSession_->SetRootSessionRect({ 0, 0, 1260, 2720 });
    rect = { 0, 0, 1260, 2720 };
    EXPECT_EQ(ssm_->rootSceneSession_->GetSessionRect(), rect);
}

/**
 * @tc.name: UpdateAvoidArea_01
 * @tc.desc: test function : UpdateAvoidArea_01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, UpdateAvoidArea_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onNotifyAvoidAreaChange_ = [](const sptr<AvoidArea>& avoidArea, AvoidAreaType type) {};
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    AvoidArea avoidArea;
    auto ret = ssm_->rootSceneSession_->UpdateAvoidArea(new AvoidArea(avoidArea), AvoidAreaType::TYPE_SYSTEM);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetStatusBarHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneSessionTest, GetStatusBarHeight, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    auto height = ssm_->rootSceneSession_->GetStatusBarHeight();
    EXPECT_EQ(0, height);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCb, nullptr);
    specificCb->onGetSceneSessionVectorByTypeAndDisplayId_ = [](WindowType type, uint64_t displayId) {
        return ssm_->GetSceneSessionVectorByTypeAndDisplayId(type, displayId);
    };
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    ssm_->rootSceneSession_->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });

    SessionInfo statusBarSessionInfo;
    statusBarSessionInfo.abilityName_ = "statusBar";
    statusBarSessionInfo.bundleName_ = "statusBar";
    statusBarSessionInfo.screenId_ = 0;
    sptr<SceneSession> statusBarSession = sptr<SceneSession>::MakeSptr(statusBarSessionInfo, nullptr);
    statusBarSession->property_->SetPersistentId(2);
    statusBarSession->property_->type_ = WindowType::WINDOW_TYPE_STATUS_BAR;
    statusBarSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 123 });
    statusBarSession->isVisible_ = true;
    ssm_->sceneSessionMap_.insert({ statusBarSession->GetPersistentId(), statusBarSession });
    height = ssm_->rootSceneSession_->GetStatusBarHeight();
    EXPECT_EQ(123, height);
    ssm_->rootSceneSession_->onGetStatusBarAvoidHeightFunc_ = [](DisplayId displayId, WSRect& barArea)
        { barArea.height_ = 100; };
    height = ssm_->rootSceneSession_->GetStatusBarHeight();
    EXPECT_EQ(100, height);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
