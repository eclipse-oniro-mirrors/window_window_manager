/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "window_property.h"
#include "wm_common_inner.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowPropertyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void WindowPropertyTest::SetUpTestCase() {}

void WindowPropertyTest::TearDownTestCase() {}

void WindowPropertyTest::SetUp() {}

void WindowPropertyTest::TearDown() {}

namespace {
/**
 * @tc.name: MarshallingUnmarshalling
 * @tc.desc: Marshalling Unmarshalling test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, MarshallingUnmarshalling, TestSize.Level1)
{
    WindowProperty winPropSrc;
    winPropSrc.SetPrivacyMode(true);
    winPropSrc.SetTransparent(true);
    winPropSrc.SetTransform(Transform::Identity());
    winPropSrc.SetFollowScreenChange(true);

    Parcel parcel;
    winPropSrc.Marshalling(parcel);
    sptr<WindowProperty> winPropDst = sptr<WindowProperty>::MakeSptr(winPropSrc.Unmarshalling(parcel));

    EXPECT_EQ(winPropDst->GetPrivacyMode(), true);
    EXPECT_EQ(winPropDst->GetTransparent(), true);
    EXPECT_EQ(winPropDst->GetTransform(), Transform::Identity());
    EXPECT_EQ(winPropDst->GetFollowScreenChange(), true);
}

/**
 * @tc.name: CopyFrom
 * @tc.desc: CopyFrom test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, CopyFrom, TestSize.Level1)
{
    const sptr<WindowProperty> winPropSrc = new (std::nothrow) WindowProperty();
    winPropSrc->SetPrivacyMode(true);
    winPropSrc->SetTransparent(true);
    winPropSrc->SetTransform(Transform::Identity());

    WindowProperty winPropDst(winPropSrc); // winPropDst.CopyFrom(winPropSrc);

    ASSERT_EQ(winPropSrc->GetPrivacyMode(), winPropDst.GetPrivacyMode());
    ASSERT_EQ(winPropSrc->GetTransparent(), winPropDst.GetTransparent());
    ASSERT_EQ(winPropSrc->GetTransform(), winPropDst.GetTransform());
}

/**
 * @tc.name: Read
 * @tc.desc: Read test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, Read, TestSize.Level1)
{
    WindowProperty winPropSrc;
    winPropSrc.SetPrivacyMode(true);
    winPropSrc.SetTransparent(true);

    Parcel parcel;
    winPropSrc.Marshalling(parcel);

    WindowProperty winPropDst;
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_RECT);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_MODE);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_FLAGS);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_FOCUSABLE);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_TOUCHABLE);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_CALLING_WINDOW);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_ORIENTATION);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE);

    ASSERT_EQ(false, winPropDst.GetPrivacyMode());
    ASSERT_EQ(false, winPropDst.GetTransparent());
}

/**
 * @tc.name: Write
 * @tc.desc: Write test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, Write, TestSize.Level1)
{
    Parcel parcel;
    WindowProperty winPropDst;
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_RECT));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_MODE));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_FLAGS));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_OTHER_PROPS));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_FOCUSABLE));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_TOUCHABLE));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_CALLING_WINDOW));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_ORIENTATION));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE));
}

/**
 * @tc.name: SetAbilityInfo
 * @tc.desc: Test SetAbilityInfo and GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, SetAbilityInfo, TestSize.Level1)
{
    WindowProperty winPropDst;
    AbilityInfo info;
    info.bundleName_ = "testBundleName";
    info.abilityName_ = "testAbilityName";
    winPropDst.SetAbilityInfo(info);
    ASSERT_EQ("testBundleName", winPropDst.GetAbilityInfo().bundleName_);
    ASSERT_EQ("testAbilityName", winPropDst.GetAbilityInfo().abilityName_);
}

/**
 * @tc.name: ResumeLastWindowMode
 * @tc.desc: Test ResumeLastWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, ResumeLastWindowMode, TestSize.Level1)
{
    WindowProperty winPropDst;
    winPropDst.windowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_PIP;
    winPropDst.lastMode_ = WindowMode::WINDOW_MODE_PIP;
    winPropDst.mode_ = WindowMode::WINDOW_MODE_UNDEFINED;
    winPropDst.ResumeLastWindowMode();
    ASSERT_EQ(WindowMode::WINDOW_MODE_PIP, winPropDst.mode_);

    winPropDst.windowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY;
    winPropDst.lastMode_ = WindowMode::WINDOW_MODE_PIP;
    winPropDst.mode_ = WindowMode::WINDOW_MODE_UNDEFINED;
    winPropDst.ResumeLastWindowMode();
    ASSERT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, winPropDst.GetWindowMode());

    winPropDst.windowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
    winPropDst.lastMode_ = WindowMode::WINDOW_MODE_PIP;
    winPropDst.mode_ = WindowMode::WINDOW_MODE_UNDEFINED;
    winPropDst.ResumeLastWindowMode();
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, winPropDst.mode_);
}

/**
 * @tc.name: AddWindowFlag001
 * @tc.desc: AddWindowFlag test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, AddWindowFlag001, TestSize.Level1)
{
    WindowProperty winPropSrc;
    int resultValue = 0;
    WindowFlag flag = WindowFlag::WINDOW_FLAG_NEED_AVOID;
    std::function<void()> func = [&]() {
        winPropSrc.AddWindowFlag(flag);
        resultValue = 1;
    };
    func();
    ASSERT_NE(resultValue, 0);
}

/**
 * @tc.name: GetRequestRect002
 * @tc.desc: GetRequestRect test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetRequestRect001, TestSize.Level1)
{
    WindowProperty winPropSrc;
    Rect requestRect{ 0, 0, 0, 0 };
    winPropSrc.SetRequestRect(requestRect);
    Rect res = winPropSrc.GetRequestRect();
    ASSERT_EQ(res, requestRect);
}

/**
 * @tc.name: GetWindowSizeChangeReason003
 * @tc.desc: GetWindowSizeChangeReason test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetWindowSizeChangeReason003, TestSize.Level1)
{
    WindowProperty winPropSrc;
    WindowSizeChangeReason reason = WindowSizeChangeReason::UNDEFINED;
    winPropSrc.SetWindowSizeChangeReason(reason);
    WindowSizeChangeReason res = winPropSrc.GetWindowSizeChangeReason();
    ASSERT_EQ(res, reason);
}

/**
 * @tc.name: GetFullScreen004
 * @tc.desc: GetFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetFullScreen004, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isFullScreen = true;
    winPropSrc.SetFullScreen(isFullScreen);
    bool res = winPropSrc.GetFullScreen();
    ASSERT_EQ(res, isFullScreen);
}

/**
 * @tc.name: GetFocusable005
 * @tc.desc: GetFocusable test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetFocusable005, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isFocusable = true;
    winPropSrc.SetFocusable(isFocusable);
    bool res = winPropSrc.GetFocusable();
    ASSERT_EQ(res, isFocusable);
}

/**
 * @tc.name: GetTouchable006
 * @tc.desc: GetTouchable test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetTouchable006, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isTouchable = true;
    winPropSrc.SetTouchable(isTouchable);
    bool res = winPropSrc.GetFocusable();
    ASSERT_EQ(res, isTouchable);
}

/**
 * @tc.name: GetCallingWindow007
 * @tc.desc: GetCallingWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetCallingWindow007, TestSize.Level1)
{
    WindowProperty winPropSrc;
    uint32_t windowId = 1;
    winPropSrc.ChangeCallingWindowId(windowId);
    uint32_t res = winPropSrc.GetCallingWindow();
    ASSERT_EQ(res, windowId);
}

/**
 * @tc.name: GetPrivacyMode008
 * @tc.desc: GetPrivacyMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetPrivacyMode008, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isPrivate = true;
    winPropSrc.SetPrivacyMode(isPrivate);
    bool res = winPropSrc.GetPrivacyMode();
    ASSERT_EQ(res, isPrivate);
}

/**
 * @tc.name: GetSystemPrivacyMode009
 * @tc.desc: GetSystemPrivacyMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetSystemPrivacyMode009, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isSystemPrivate = true;
    winPropSrc.SetSystemPrivacyMode(isSystemPrivate);
    bool res = winPropSrc.GetSystemPrivacyMode();
    ASSERT_EQ(res, isSystemPrivate);
}

/**
 * @tc.name: GetSnapshotSkip010
 * @tc.desc: GetSnapshotSkip test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetSnapshotSkip010, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isSkip = true;
    winPropSrc.SetSnapshotSkip(isSkip);
    bool res = winPropSrc.GetSnapshotSkip();
    ASSERT_EQ(res, isSkip);
}

/**
 * @tc.name: GetAlpha011
 * @tc.desc: GetAlpha test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetAlpha011, TestSize.Level1)
{
    WindowProperty winPropSrc;
    float alpha = 1;
    winPropSrc.SetAlpha(alpha);
    float res = winPropSrc.GetAlpha();
    ASSERT_EQ(res, alpha);
}

/**
 * @tc.name: GetBrightness012
 * @tc.desc: GetBrightness test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetBrightness012, TestSize.Level1)
{
    WindowProperty winPropSrc;
    float brightness = 1;
    winPropSrc.SetBrightness(brightness);
    float res = winPropSrc.GetBrightness();
    ASSERT_EQ(res, brightness);
}

/**
 * @tc.name: IsTurnScreenOn013
 * @tc.desc: IsTurnScreenOn test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, IsTurnScreenOn013, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool turnScreenOn = true;
    winPropSrc.SetTurnScreenOn(turnScreenOn);
    bool res = winPropSrc.IsTurnScreenOn();
    ASSERT_EQ(res, turnScreenOn);
}

/**
 * @tc.name: IsKeepScreenOn014
 * @tc.desc: IsKeepScreenOn test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, IsKeepScreenOn014, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool keepScreenOn = true;
    winPropSrc.SetKeepScreenOn(keepScreenOn);
    bool res = winPropSrc.IsKeepScreenOn();
    ASSERT_EQ(res, keepScreenOn);
}

/**
 * @tc.name: GetWindowFlags015
 * @tc.desc: GetWindowFlags test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetWindowFlags015, TestSize.Level1)
{
    WindowProperty winPropSrc;
    uint32_t flags = 1;
    winPropSrc.SetWindowFlags(flags);
    uint32_t res = winPropSrc.GetWindowFlags();
    ASSERT_EQ(res, flags);
}

/**
 * @tc.name: GetSystemBarProperty016
 * @tc.desc: GetSystemBarProperty test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetSystemBarProperty016, TestSize.Level1)
{
    WindowProperty winPropSrc;
    SystemBarProperty property;
    WindowType type = WindowType::WINDOW_TYPE_STATUS_BAR;
    winPropSrc.SetSystemBarProperty(type, property);
    std::unordered_map<WindowType, SystemBarProperty> res = winPropSrc.GetSystemBarProperty();
    ASSERT_EQ(res[type], property);
}

/**
 * @tc.name: GetStretchable017
 * @tc.desc: GetHitOffset test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetStretchable017, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool stretchable = true;
    winPropSrc.SetStretchable(stretchable);
    bool res = winPropSrc.GetStretchable();
    ASSERT_EQ(res, stretchable);
}

/**
 * @tc.name: GetAnimationFlag018
 * @tc.desc: GetAnimationFlag test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetAnimationFlag018, TestSize.Level1)
{
    WindowProperty winPropSrc;
    uint32_t animationFlag = 1;
    winPropSrc.SetAnimationFlag(animationFlag);
    uint32_t res = winPropSrc.GetAnimationFlag();
    ASSERT_EQ(res, animationFlag);
}

/**
 * @tc.name: GetWindowModeSupportType019
 * @tc.desc: GetWindowModeSupportType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetWindowModeSupportType019, TestSize.Level1)
{
    WindowProperty winPropSrc;
    uint32_t windowModeSupportType = 1;
    winPropSrc.SetWindowModeSupportType(windowModeSupportType);
    uint32_t res = winPropSrc.GetWindowModeSupportType();
    ASSERT_EQ(res, windowModeSupportType);
}

/**
 * @tc.name: GetRequestWindowModeSupportType020
 * @tc.desc: GetRequestWindowModeSupportType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetRequestWindowModeSupportType020, TestSize.Level1)
{
    WindowProperty winPropSrc;
    uint32_t requestWindowModeSupportType = 1;
    winPropSrc.SetRequestWindowModeSupportType(requestWindowModeSupportType);
    uint32_t res = winPropSrc.GetRequestWindowModeSupportType();
    ASSERT_EQ(res, requestWindowModeSupportType);
}

/**
 * @tc.name: GetTokenState021
 * @tc.desc: GetTokenState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetTokenState021, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool hasToken = true;
    winPropSrc.SetTokenState(hasToken);
    bool res = winPropSrc.GetTokenState();
    ASSERT_EQ(res, hasToken);
}

/**
 * @tc.name: GetDragType022
 * @tc.desc: GetDragType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetDragType022, TestSize.Level1)
{
    WindowProperty winPropSrc;
    DragType dragType = DragType::DRAG_UNDEFINED;
    winPropSrc.SetDragType(dragType);
    DragType res = winPropSrc.GetDragType();
    ASSERT_EQ(res, dragType);
}

/**
 * @tc.name: GetOriginRect023
 * @tc.desc: GetOriginRect test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetOriginRect023, TestSize.Level1)
{
    WindowProperty winPropSrc;
    Rect rect = { 0, 0, 0, 0 };
    winPropSrc.SetOriginRect(rect);
    Rect res = winPropSrc.GetOriginRect();
    ASSERT_EQ(res, rect);
}

/**
 * @tc.name: SetTouchHotAreas028
 * @tc.desc: SetTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, SetTouchHotAreas028, TestSize.Level1)
{
    WindowProperty winPropSrc;
    std::vector<Rect> rects;
    winPropSrc.SetTouchHotAreas(rects);
    winPropSrc.GetTouchHotAreas(rects);
    ASSERT_EQ(rects, winPropSrc.touchHotAreas_);
}

/**
 * @tc.name: SetAspectRatio029
 * @tc.desc: SetAspectRatio test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, SetAspectRatio029, TestSize.Level1)
{
    WindowProperty winPropSrc;
    float ratio = 1;
    winPropSrc.SetAspectRatio(ratio);
    float res = winPropSrc.GetAspectRatio();
    ASSERT_EQ(res, ratio);
}

/**
 * @tc.name: SetMaximizeMode030
 * @tc.desc: SetMaximizeMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, SetMaximizeMode030, TestSize.Level1)
{
    WindowProperty winPropSrc;
    MaximizeMode maximizeMode = { MaximizeMode::MODE_RECOVER };
    winPropSrc.SetMaximizeMode(maximizeMode);
    MaximizeMode res = winPropSrc.GetMaximizeMode();
    ASSERT_EQ(res, maximizeMode);
}

/**
 * @tc.name: GetAccessTokenId031
 * @tc.desc: GetAccessTokenId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetAccessTokenId031, TestSize.Level1)
{
    WindowProperty winPropSrc;
    uint32_t accessTokenId = 1;
    winPropSrc.SetAccessTokenId(accessTokenId);
    uint32_t res = winPropSrc.GetAccessTokenId();
    ASSERT_EQ(res, accessTokenId);
}

/**
 * @tc.name: SetWindowGravity032
 * @tc.desc: SetWindowGravity test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetAccessTokenId032, TestSize.Level1)
{
    WindowProperty winPropSrc;
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    uint32_t percent = 1;
    winPropSrc.SetWindowGravity(gravity, percent);
    winPropSrc.GetWindowGravity(gravity, percent);
    ASSERT_EQ(gravity, winPropSrc.windowGravity_);
    ASSERT_EQ(percent, winPropSrc.windowGravitySizePercent_);
}

/**
 * @tc.name: Write033
 * @tc.desc: Write test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, Write033, TestSize.Level1)
{
    WindowProperty winPropDst;
    Parcel parcel;
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO));
    ASSERT_EQ(true, winPropDst.Write(parcel, PropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE));
}

/**
 * @tc.name: Read034
 * @tc.desc: Read test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, Read034, TestSize.Level1)
{
    WindowProperty winPropSrc;
    winPropSrc.SetPrivacyMode(true);
    winPropSrc.SetTransparent(true);

    Parcel parcel;
    winPropSrc.Marshalling(parcel);

    WindowProperty winPropDst;
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO);
    winPropDst.Read(parcel, PropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);

    ASSERT_EQ(false, winPropDst.GetPrivacyMode());
    ASSERT_EQ(false, winPropDst.GetTransparent());
}

/**
 * @tc.name: GetTextFieldPositionY035
 * @tc.desc: GetTextFieldPositionY test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetTextFieldPositionY035, TestSize.Level1)
{
    WindowProperty winPropSrc;
    double textFieldPositionY = 1;
    winPropSrc.SetTextFieldPositionY(textFieldPositionY);
    double res = winPropSrc.GetTextFieldPositionY();
    ASSERT_EQ(res, textFieldPositionY);
}

/**
 * @tc.name: GetTextFieldHeight036
 * @tc.desc: GetTextFieldHeight test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetTextFieldHeight36, TestSize.Level1)
{
    WindowProperty winPropSrc;
    double textFieldHeight = 1;
    winPropSrc.SetTextFieldHeight(textFieldHeight);
    double res = winPropSrc.GetTextFieldHeight();
    ASSERT_EQ(res, textFieldHeight);
}

/**
 * @tc.name: GetFollowScreenChange37
 * @tc.desc: GetFollowScreenChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowPropertyTest, GetFollowScreenChange37, TestSize.Level1)
{
    WindowProperty winPropSrc;
    bool isFollowScreenChange = true;
    winPropSrc.SetFollowScreenChange(isFollowScreenChange);
    bool res = winPropSrc.GetFollowScreenChange();
    ASSERT_EQ(res, isFollowScreenChange);
}
}
} // namespace Rosen
} // namespace OHOS