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
#include "window_session_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class WindowSessionPropertyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void WindowSessionPropertyTest::SetUpTestCase()
{
}

void WindowSessionPropertyTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: SetDragEnabled001
 * @tc.desc: SetDragEnabled and GetDragEnabled to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetDragEnabled001, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetDragEnabled(true);
    ASSERT_EQ(property->GetDragEnabled(), true);
    property->SetDragEnabled(false);
    ASSERT_EQ(property->GetDragEnabled(), false);
}

/**
 * @tc.name: SetRaiseEnabled001
 * @tc.desc: SetRaiseEnabled and GetRaiseEnabled to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetRaiseEnabled001, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetRaiseEnabled(true);
    ASSERT_EQ(property->GetRaiseEnabled(), true);
    property->SetRaiseEnabled(false);
    ASSERT_EQ(property->GetRaiseEnabled(), false);
}

/**
 * @tc.name: WindowSessionProperty
 * @tc.desc: WindowSessionProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, WindowSessionProperty, Function | SmallTest | Level2)
{
    const sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sptr<WindowSessionProperty> targetProperty = sptr<WindowSessionProperty>::MakeSptr(property);
    ASSERT_NE(nullptr, targetProperty);
    ASSERT_EQ(property->GetDragEnabled(), targetProperty->GetDragEnabled());
}

/**
 * @tc.name: SetSessionInfo
 * @tc.desc: SetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSessionInfo, Function | SmallTest | Level2)
{
    SessionInfo* info = new SessionInfo();
    ASSERT_NE(nullptr, info);
    info->bundleName_ = "test";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetSessionInfo(*info);
    auto result = property->GetSessionInfo();
    ASSERT_EQ(result.bundleName_, info->bundleName_);
    delete info;
}

/**
 * @tc.name: SetRequestedOrientation
 * @tc.desc: SetRequestedOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetRequestedOrientation, Function | SmallTest | Level2)
{
    Orientation orientation = Orientation::REVERSE_HORIZONTAL;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetRequestedOrientation(orientation);
    Orientation ret = property->GetRequestedOrientation();
    ASSERT_EQ(ret, orientation);

    property->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = property->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    property->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = property->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    property->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = property->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    property->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = property->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    property->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = property->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    property->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = property->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
}

/**
 * @tc.name: SetDefaultRequestedOrientation
 * @tc.desc: SetDefaultRequestedOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetDefaultRequestedOrientation, Function | SmallTest | Level2)
{
    Orientation orientation = Orientation::REVERSE_HORIZONTAL;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetDefaultRequestedOrientation(orientation);
    Orientation ret = property->GetDefaultRequestedOrientation();
    ASSERT_EQ(ret, orientation);
}

/**
 * @tc.name: SetPrivacyMode
 * @tc.desc: SetPrivacyMode as true and false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetPrivacyMode, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    ASSERT_EQ(property->GetPrivacyMode(), false);
    property->SetPrivacyMode(true);
    ASSERT_EQ(property->GetPrivacyMode(), true);
    property->SetPrivacyMode(false);
    ASSERT_EQ(property->GetPrivacyMode(), false);
}

/**
 * @tc.name: SetSystemPrivacyMode
 * @tc.desc: SetSystemPrivacyMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSystemPrivacyMode, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetSystemPrivacyMode(false);
    ASSERT_EQ(property->GetSystemPrivacyMode(), false);
    property->SetSystemPrivacyMode(true);
    ASSERT_EQ(property->GetSystemPrivacyMode(), true);
}

/**
 * @tc.name: SetBrightness
 * @tc.desc: SetBrightness test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetBrightness, Function | SmallTest | Level2)
{
    float brightness = 0.02;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetBrightness(brightness);
    ASSERT_EQ(brightness, property->GetBrightness());
}

/**
 * @tc.name: SetTopmost
 * @tc.desc: SetTopmost test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTopmost, Function | SmallTest | Level2)
{
    bool topmost = true;
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetTopmost(topmost);
    ASSERT_TRUE(windowSessionProperty.IsTopmost());
}

/**
 * @tc.name: SetMainWindowTopmost
 * @tc.desc: SetMainWindowTopmost test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetMainWindowTopmost, Function | SmallTest | Level2)
{
    bool isTopmost = true;
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetMainWindowTopmost(isTopmost);
    ASSERT_TRUE(windowSessionProperty.IsMainWindowTopmost());
}

/**
 * @tc.name: GetParentId
 * @tc.desc: GetParentId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetParentId, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetParentId(0);
    int32_t result = windowSessionProperty.GetParentId();
    ASSERT_EQ(0, result);
}

/**
 * @tc.name: SetWindowFlags
 * @tc.desc: SetWindowFlags test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetWindowFlags, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowFlags(0);
    ASSERT_EQ(property->GetWindowFlags(), 0);
}

/**
 * @tc.name: SetAndGetPipTemplateInfo
 * @tc.desc: SetAndGetPipTemplateInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndGetPipTemplateInfo, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    PiPTemplateInfo pipTemplateInfo;
    pipTemplateInfo.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL);
    property->SetPiPTemplateInfo(pipTemplateInfo);
    ASSERT_EQ(property->GetPiPTemplateInfo().pipTemplateType,
        static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL));
}

/**
 * @tc.name: SetAndGetRealParentId
 * @tc.desc: SetRealParentId and GetRealParentId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndGetRealParentId, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetRealParentId(1919);
    EXPECT_EQ(1919, property->GetRealParentId());
    property->SetRealParentId(810);
    EXPECT_EQ(810, property->GetRealParentId());
}

/**
 * @tc.name: SetAndGetUIExtensionUsage
 * @tc.desc: SetUIExtensionUsage and GetUIExtensionUsage test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndGetUIExtensionUsage, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    EXPECT_EQ(UIExtensionUsage::MODAL, property->GetUIExtensionUsage());
    property->SetUIExtensionUsage(UIExtensionUsage::EMBEDDED);
    EXPECT_EQ(UIExtensionUsage::EMBEDDED, property->GetUIExtensionUsage());
}

/**
 * @tc.name: SetParentWindowType
 * @tc.desc: SetParentWindowType and GetParentWindowType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetParentWindowType, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetParentWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, property->GetParentWindowType());
    property->SetParentWindowType(WindowType::WINDOW_TYPE_TOAST);
    EXPECT_EQ(WindowType::WINDOW_TYPE_TOAST, property->GetParentWindowType());
}

/**
 * @tc.name: SetAndGetIsUIExtensionAbilityProcess
 * @tc.desc: SetIsUIExtensionAbilityProcess and GetIsUIExtensionAbilityProcess test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndGetIsUIExtensionAbilityProcess, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetIsUIExtensionAbilityProcess(true);
    EXPECT_EQ(true, property->GetIsUIExtensionAbilityProcess());
    property->SetIsUIExtensionAbilityProcess(false);
    EXPECT_EQ(false, property->GetIsUIExtensionAbilityProcess());
}

/**
 * @tc.name: AddWindowFlag
 * @tc.desc: AddWindowFlag test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, AddWindowFlag, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    uint32_t windowFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    ASSERT_EQ(property->GetWindowFlags(), windowFlags);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_PARENT_LIMIT);
    windowFlags |= static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_PARENT_LIMIT);
    ASSERT_EQ(property->GetWindowFlags(), windowFlags);
}

/**
 * @tc.name: IsTurnScreenOn
 * @tc.desc: IsTurnScreenOn test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsTurnScreenOn, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetTurnScreenOn(false);
    bool result = windowSessionProperty.IsTurnScreenOn();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: IsKeepScreenOn
 * @tc.desc: IsKeepScreenOn test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsKeepScreenOn, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetKeepScreenOn(false);
    bool result = windowSessionProperty.IsKeepScreenOn();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: GetAccessTokenId
 * @tc.desc: GetAccessTokenId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetAccessTokenId, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetAccessTokenId(false);
    auto result = windowSessionProperty.GetAccessTokenId();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: SetTokenState
 * @tc.desc: SetTokenState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTokenState, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(false);
    ASSERT_EQ(property->GetTokenState(), false);
    property->SetTokenState(true);
    ASSERT_EQ(property->GetTokenState(), true);
}

/**
 * @tc.name: SetMaximizeMode
 * @tc.desc: SetMaximizeMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetMaximizeMode, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    MaximizeMode mode = MaximizeMode::MODE_RECOVER;
    windowSessionProperty.SetMaximizeMode(mode);
    ASSERT_EQ(windowSessionProperty.GetMaximizeMode(), mode);
}

/**
 * @tc.name: SetSystemBarProperty
 * @tc.desc: SetSystemBarProperty test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSystemBarProperty, Function | SmallTest | Level2)
{
    SystemBarProperty* systemBarProperty = new SystemBarProperty();
    ASSERT_NE(nullptr, systemBarProperty);
    WindowType windowType = WindowType::WINDOW_TYPE_STATUS_BAR;
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetSystemBarProperty(windowType, *systemBarProperty);
    auto sysBarPropMap = windowSessionProperty.GetSystemBarProperty();
    auto sysBarProperty = sysBarPropMap[windowType];
    ASSERT_EQ(sysBarProperty, *systemBarProperty);
    delete systemBarProperty;
}

/**
 * @tc.name: IsDecorEnable
 * @tc.desc: IsDecorEnable test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsDecorEnable, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetDecorEnable(false);
    auto result = windowSessionProperty.IsDecorEnable();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: SetWindowModeSupportType
 * @tc.desc: SetWindowModeSupportType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetWindowModeSupportType, Function | SmallTest | Level2)
{
    uint32_t windowModeSupportType = static_cast<uint32_t>(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetWindowModeSupportType(windowModeSupportType);
    ASSERT_EQ(windowSessionProperty.GetWindowModeSupportType(), windowModeSupportType);
}

/**
 * @tc.name: IsFloatingWindowAppType
 * @tc.desc: IsFloatingWindowAppType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsFloatingWindowAppType, Function | SmallTest | Level2)
{
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetFloatingWindowAppType(false);
    auto result = windowSessionProperty.IsFloatingWindowAppType();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: SetTouchHotAreas
 * @tc.desc: SetTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTouchHotAreas, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    Rect rect { 4, 4, 4, 4 };
    std::vector<Rect> vRect { rect };
    property->SetPersistentId(0);
    property->SetSessionPropertyChangeCallback(nullptr);
    EXPECT_EQ(nullptr, property->touchHotAreasChangeCallback_);
    property->SetTouchHotAreas(vRect);

    auto func = []() {};
    property->SetPersistentId(1);
    property->SetSessionPropertyChangeCallback(func);
    property->SetTouchHotAreas(vRect);
    EXPECT_NE(nullptr, property->touchHotAreasChangeCallback_);

    Rect rect1 { 5, 5, 5, 5 };
    vRect.emplace_back(rect1);
    property->SetTouchHotAreas(vRect);
}

/**
 * @tc.name: SetKeyboardTouchHotAreas
 * @tc.desc: SetKeyboardTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetKeyboardTouchHotAreas, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas hotAreas;
    Rect rect { 4, 4, 4, 4 };
    hotAreas.landscapeKeyboardHotAreas_.push_back(rect);
    hotAreas.landscapePanelHotAreas_.push_back(rect);
    hotAreas.portraitKeyboardHotAreas_.push_back(rect);
    hotAreas.portraitPanelHotAreas_.push_back(rect);
    property->SetPersistentId(0);
    property->SetSessionPropertyChangeCallback(nullptr);
    EXPECT_EQ(nullptr, property->touchHotAreasChangeCallback_);
    property->SetKeyboardTouchHotAreas(hotAreas);

    auto func = []() {};
    property->SetPersistentId(1);
    property->SetSessionPropertyChangeCallback(func);
    property->SetKeyboardTouchHotAreas(hotAreas);
    EXPECT_NE(nullptr, property->touchHotAreasChangeCallback_);

    Rect rect1 { 5, 5, 5, 5 };
    hotAreas.landscapeKeyboardHotAreas_.push_back(rect1);
    hotAreas.landscapePanelHotAreas_.push_back(rect1);
    hotAreas.portraitKeyboardHotAreas_.push_back(rect1);
    hotAreas.portraitPanelHotAreas_.push_back(rect1);
    property->SetKeyboardTouchHotAreas(hotAreas);
}

/**
 * @tc.name: UnmarshallingWindowLimits
 * @tc.desc: UnmarshallingWindowLimits test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingWindowLimits, Function | SmallTest | Level2)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.UnmarshallingWindowLimits(parcel, property);
}

/**
 * @tc.name: UnMarshallingSystemBarMap
 * @tc.desc: UnMarshallingSystemBarMap test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnMarshallingSystemBarMap, Function | SmallTest | Level2)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.MarshallingSystemBarMap(parcel);
    windowSessionProperty.UnMarshallingSystemBarMap(parcel, property);
}

/**
 * @tc.name: UnmarshallingTouchHotAreas
 * @tc.desc: UnmarshallingTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingTouchHotAreas, Function | SmallTest | Level2)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    Rect rect { 4, 4, 4, 4 };
    std::vector<Rect> vRect { rect };
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetTouchHotAreas(vRect);
    windowSessionProperty.MarshallingTouchHotAreas(parcel);
    windowSessionProperty.UnmarshallingTouchHotAreas(parcel, property);
    ASSERT_NE(0, property->touchHotAreas_.size());
}

/**
 * @tc.name: UnmarshallingKeyboardTouchHotAreas
 * @tc.desc: UnmarshallingKeyboardTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingKeyboardTouchHotAreas, Function | SmallTest | Level2)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas hotAreas;
    Rect rect { 4, 4, 4, 4 };
    hotAreas.landscapeKeyboardHotAreas_.push_back(rect);
    hotAreas.landscapePanelHotAreas_.push_back(rect);
    hotAreas.portraitKeyboardHotAreas_.push_back(rect);
    hotAreas.portraitPanelHotAreas_.push_back(rect);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetKeyboardTouchHotAreas(hotAreas);
    windowSessionProperty.MarshallingKeyboardTouchHotAreas(parcel);
    windowSessionProperty.UnmarshallingKeyboardTouchHotAreas(parcel, property);
    ASSERT_NE(0, property->keyboardTouchHotAreas_.landscapeKeyboardHotAreas_.size());
}

/**
 * @tc.name: UnmarshallingPiPTemplateInfo
 * @tc.desc: UnmarshallingPiPTemplateInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingPiPTemplateInfo, Function | SmallTest | Level2)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    EXPECT_EQ(WindowType::WINDOW_TYPE_PIP, property->GetWindowType());
    PiPTemplateInfo pipTemplateInfo;
    pipTemplateInfo.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL);
    property->SetPiPTemplateInfo(pipTemplateInfo);
    property->MarshallingPiPTemplateInfo(parcel);
    property->UnmarshallingPiPTemplateInfo(parcel, property);
}

/**
 * @tc.name: CopyFrom
 * @tc.desc: CopyFrom test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, CopyFrom, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(nullptr, property);
    std::string name = "test";
    property->SetWindowName(name);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.CopyFrom(property);
    ASSERT_EQ(windowSessionProperty.GetWindowName(), name);
}

/**
 * @tc.name: SetFocusable
 * @tc.desc: SetFocusable and GetFocusable to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetFocusable, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetFocusable(true);
    ASSERT_EQ(property->GetFocusable(), true);
    property->SetFocusable(false);
    ASSERT_EQ(property->GetFocusable(), false);
}

/**
 * @tc.name: SetTouchable
 * @tc.desc: SetTouchable and GetTouchable to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTouchable, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTouchable(true);
    ASSERT_EQ(property->GetTouchable(), true);
    property->SetTouchable(false);
    ASSERT_EQ(property->GetTouchable(), false);
}

/**
 * @tc.name: SetForceHide
 * @tc.desc: SetForceHide and GetForceHide to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetForceHide, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetForceHide(false);
    ASSERT_EQ(property->GetForceHide(), false);
    property->SetForceHide(true);
    ASSERT_EQ(property->GetForceHide(), true);
}

/**
 * @tc.name: SetSystemCalling
 * @tc.desc: SetSystemCalling and GetSystemCalling to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSystemCalling, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetSystemCalling(false);
    ASSERT_EQ(property->GetSystemCalling(), false);
    property->SetSystemCalling(true);
    ASSERT_EQ(property->GetSystemCalling(), true);
}

/**
 * @tc.name: SetIsNeedUpdateWindowMode
 * @tc.desc: SetIsNeedUpdateWindowMode and GetIsNeedUpdateWindowMode to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetIsNeedUpdateWindowMode, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetIsNeedUpdateWindowMode(false);
    ASSERT_EQ(property->GetIsNeedUpdateWindowMode(), false);
    property->SetIsNeedUpdateWindowMode(true);
    ASSERT_EQ(property->GetIsNeedUpdateWindowMode(), true);
}

/**
 * @tc.name: SetIsShaped
 * @tc.desc: SetIsShaped and GetIsShaped to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetIsShaped, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetIsShaped(false);
    ASSERT_EQ(property->GetIsShaped(), false);
    property->SetIsShaped(true);
    ASSERT_EQ(property->GetIsShaped(), true);
}

/**
 * @tc.name: SetCollaboratorType
 * @tc.desc: SetCollaboratorType and GetCollaboratorType to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetCollaboratorType, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    int32_t typeValue = static_cast<int32_t>(CollaboratorType::DEFAULT_TYPE);
    property->SetCollaboratorType(typeValue);
    ASSERT_EQ(property->GetCollaboratorType(), typeValue);
    typeValue = static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
    property->SetCollaboratorType(typeValue);
    ASSERT_EQ(property->GetCollaboratorType(), typeValue);
    typeValue = static_cast<int32_t>(CollaboratorType::OTHERS_TYPE);
    property->SetCollaboratorType(typeValue);
    ASSERT_EQ(property->GetCollaboratorType(), typeValue);
}

/**
 * @tc.name: SetUserWindowLimits
 * @tc.desc: SetUserWindowLimits and GetUserWindowLimits to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetUserWindowLimits, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowLimits limits;
    limits.minWidth_ = 10;
    property->SetUserWindowLimits(limits);
    WindowLimits result = property->GetUserWindowLimits();
    ASSERT_EQ(result.minWidth_, limits.minWidth_);
}

/**
 * @tc.name: SetConfigWindowLimitsVP
 * @tc.desc: SetConfigWindowLimitsVP and GetConfigWindowLimitsVP to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetConfigWindowLimitsVP, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowLimits limits;
    limits.minWidth_ = 10;
    property->SetConfigWindowLimitsVP(limits);
    WindowLimits result = property->GetConfigWindowLimitsVP();
    ASSERT_EQ(result.minWidth_, limits.minWidth_);
}

/**
 * @tc.name: SetLastLimitsVpr
 * @tc.desc: SetLastLimitsVpr and GetLastLimitsVpr to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetLastLimitsVpr, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    float vpr = 1.0f;
    property->SetLastLimitsVpr(vpr);
    auto result = property->GetLastLimitsVpr();
    ASSERT_EQ(result, vpr);
}

/**
 * @tc.name: SetFullScreenStart
 * @tc.desc: SetFullScreenStart and GetFullScreenStart to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetFullScreenStart, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetFullScreenStart(true);
    ASSERT_EQ(property->GetFullScreenStart(), true);
    property->SetFullScreenStart(false);
    ASSERT_EQ(property->GetFullScreenStart(), false);
}

/**
 * @tc.name: SetHideNonSystemFloatingWindows
 * @tc.desc: SetHideNonSystemFloatingWindows and GetHideNonSystemFloatingWindows to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetHideNonSystemFloatingWindows, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetHideNonSystemFloatingWindows(false);
    ASSERT_EQ(property->GetHideNonSystemFloatingWindows(), false);
    property->SetHideNonSystemFloatingWindows(true);
    ASSERT_EQ(property->GetHideNonSystemFloatingWindows(), true);
}

/**
 * @tc.name: KeepKeyboardOnFocus
 * @tc.desc: KeepKeyboardOnFocus and GetKeepKeyboardFlag to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, KeepKeyboardOnFocus, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->KeepKeyboardOnFocus(false);
    ASSERT_EQ(property->GetKeepKeyboardFlag(), false);
    property->KeepKeyboardOnFocus(true);
    ASSERT_EQ(property->GetKeepKeyboardFlag(), true);
}

/**
 * @tc.name: SetTextFieldPositionY
 * @tc.desc: SetTextFieldPositionY and GetTextFieldPositionY to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTextFieldPositionY, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTextFieldPositionY(5.5);
    ASSERT_EQ(property->GetTextFieldPositionY(), 5.5);
}

/**
 * @tc.name: SetTextFieldHeight
 * @tc.desc: SetTextFieldHeight and GetTextFieldHeight to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTextFieldHeight, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTextFieldHeight(5.5);
    ASSERT_EQ(property->GetTextFieldHeight(), 5.5);
}

/**
 * @tc.name: SetIsLayoutFullScreen
 * @tc.desc: SetIsLayoutFullScreen and IsLayoutFullScreen to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetIsLayoutFullScreen, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetIsLayoutFullScreen(false);
    ASSERT_EQ(property->IsLayoutFullScreen(), false);
    property->SetIsLayoutFullScreen(true);
    ASSERT_EQ(property->IsLayoutFullScreen(), true);
}

/**
 * @tc.name: Read
 * @tc.desc: Read test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, Read, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    Parcel parcel = Parcel();
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_RECT);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_FLAGS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_MODE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST);
    ASSERT_EQ(property->GetPersistentId(), INVALID_SESSION_ID);
}

/**
 * @tc.name: Write
 * @tc.desc: Write and Read to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, Write, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    Parcel parcel = Parcel();
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_RECT);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_FLAGS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_MODE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST);
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_AVOID_AREA_OPTION);
    ASSERT_EQ(property->GetPersistentId(), INVALID_SESSION_ID);
}

/**
 * @tc.name: GetWindowName
 * @tc.desc: GetWindowName
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowName, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    std::string name = "test";
    property->SetWindowName(name);
    auto result = property->GetWindowName();
    ASSERT_EQ(result, name);
}

/**
 * @tc.name: GetSessionInfo
 * @tc.desc: GetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetSessionInfo, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    SessionInfo* info = new SessionInfo();
    ASSERT_NE(nullptr, info);
    info->bundleName_ = "test";
    property->SetSessionInfo(*info);
    auto result = property->GetSessionInfo();
    ASSERT_EQ(result.bundleName_, info->bundleName_);
    delete info;
}

/**
 * @tc.name: EditSessionInfo
 * @tc.desc: EditSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, EditSessionInfo, Function | SmallTest | Level2)
{
    std::string abilityName = "1234";
    std::string abilityNameNew = "12345";
    SessionInfo info;
    info.abilityName_ = abilityName;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetSessionInfo(info);
    property->EditSessionInfo().abilityName_ = abilityNameNew;
    ASSERT_EQ(property->EditSessionInfo().abilityName_, abilityNameNew);
}

/**
 * @tc.name: GetWindowRect
 * @tc.desc: GetWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowRect, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    Rect rect = {0, 0, 0, 0};
    property->SetWindowRect(rect);
    auto result = property->GetWindowRect();
    ASSERT_EQ(result, rect);
}

/**
 * @tc.name: GetWindowSizeLimits
 * @tc.desc: GetWindowSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowSizeLimits, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WindowSizeLimits windowSizeLimits = { 0, 0, 0, 0 };
    property->SetWindowSizeLimits(windowSizeLimits);
    auto result = property->GetWindowSizeLimits();
    ASSERT_EQ(result, windowSizeLimits);
}

/**
 * @tc.name: GetRequestRect
 * @tc.desc: GetRequestRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetRequestRect, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    Rect requestRect = {0, 0, 0, 0};
    property->SetRequestRect(requestRect);
    auto result = property->GetRequestRect();
    ASSERT_EQ(result, requestRect);
}

/**
 * @tc.name: GetWindowType
 * @tc.desc: GetWindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowType, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowType type = WindowType::APP_WINDOW_BASE;
    property->SetWindowType(type);
    auto result = property->GetWindowType();
    ASSERT_EQ(result, type);
}

/**
 * @tc.name: GetDisplayId
 * @tc.desc: GetDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetDisplayId, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    DisplayId displayId = 1;
    property->SetDisplayId(displayId);
    auto result = property->GetDisplayId();
    ASSERT_EQ(result, displayId);
}

/**
 * @tc.name: GetPersistentId
 * @tc.desc: GetPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetPersistentId, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    int32_t persistentId = 1;
    property->SetPersistentId(persistentId);
    auto result = property->GetPersistentId();
    ASSERT_EQ(result, persistentId);
}

/**
 * @tc.name: GetParentPersistentId
 * @tc.desc: GetParentPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetParentPersistentId, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    int32_t persistentId = 1;
    property->SetParentPersistentId(persistentId);
    auto result = property->GetParentPersistentId();
    ASSERT_EQ(result, persistentId);
}

/**
 * @tc.name: SetTurnScreenOn
 * @tc.desc: SetTurnScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTurnScreenOn, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool turnScreenOn = false;
    property->SetTurnScreenOn(turnScreenOn);
    ASSERT_EQ(property->IsTurnScreenOn(), turnScreenOn);
}

/**
 * @tc.name: SetKeepScreenOn
 * @tc.desc: SetKeepScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetKeepScreenOn, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool keepScreenOn = true;
    property->SetKeepScreenOn(keepScreenOn);
    ASSERT_EQ(keepScreenOn, property->IsKeepScreenOn());
    keepScreenOn = false;
    property->SetKeepScreenOn(keepScreenOn);
    ASSERT_EQ(keepScreenOn, property->IsKeepScreenOn());
}

/**
 * @tc.name: MarshallingSessionInfo
 * @tc.desc: MarshallingSessionInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, MarshallingSessionInfo, Function | SmallTest | Level2)
{
    Parcel parcel;
    SessionInfo info = { "testBundleName", "testModuleName", "testAbilityName" };
    info.want = std::make_shared<AAFwk::Want>();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool result = property->MarshallingSessionInfo(parcel);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: UnMarshallingSessionInfo
 * @tc.desc: UnMarshallingSessionInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnMarshallingSessionInfo, Function | SmallTest | Level2)
{
    Parcel parcel;
    WindowSessionProperty windowSessionProperty;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    SessionInfo info = { "testBundleName", "testModuleName", "testAbilityName" };
    info.want = std::make_shared<AAFwk::Want>();
    bool result = property->MarshallingSessionInfo(parcel);
    ASSERT_EQ(result, true);
    result = property->UnmarshallingSessionInfo(parcel, &windowSessionProperty);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: SetAccessTokenId
 * @tc.desc: SetAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAccessTokenId, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    uint32_t accessTokenId = 1;
    property->SetAccessTokenId(accessTokenId);
    ASSERT_EQ(property->accessTokenId_, accessTokenId);
}

/**
 * @tc.name: GetWindowState
 * @tc.desc: GetWindowState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowState, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowState state = WindowState::STATE_INITIAL;
    property->SetWindowState(state);
    auto result = property->GetWindowState();
    ASSERT_EQ(result, state);
}

/**
 * @tc.name: SetSystemPrivacyMode02
 * @tc.desc: SetSystemPrivacyMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSystemPrivacyMode02, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool isSystemPrivate = false;
    property->SetSystemPrivacyMode(isSystemPrivate);
    ASSERT_EQ(property->GetSystemPrivacyMode(), isSystemPrivate);
}

/**
 * @tc.name: SetTokenState02
 * @tc.desc: SetTokenState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetTokenState02, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool hasToken = false;
    property->SetTokenState(hasToken);
    ASSERT_EQ(property->GetTokenState(), hasToken);
}

/**
 * @tc.name: MarshallingTouchHotAreas
 * @tc.desc: MarshallingTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, MarshallingTouchHotAreas, Function | SmallTest | Level2)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    std::vector<Rect> rects;
    for (int i = 0; i < 55; i++) {
        Rect rect { i, i, i, i };
        rects.push_back(rect);
    }
    property->SetTouchHotAreas(rects);
    bool result = property->MarshallingTouchHotAreas(parcel);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: MarshallingKeyboardTouchHotAreas
 * @tc.desc: MarshallingKeyboardTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, MarshallingKeyboardTouchHotAreas, Function | SmallTest | Level2)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas hotAreas;
    for (int i = 0; i < 55; i++) {
        Rect rect { i, i, i, i };
        hotAreas.landscapeKeyboardHotAreas_.push_back(rect);
        hotAreas.landscapePanelHotAreas_.push_back(rect);
        hotAreas.portraitKeyboardHotAreas_.push_back(rect);
        hotAreas.portraitPanelHotAreas_.push_back(rect);
    }
    property->SetKeyboardTouchHotAreas(hotAreas);
    bool result = property->MarshallingKeyboardTouchHotAreas(parcel);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: UnmarshallingPiPTemplateInfo02
 * @tc.desc: UnmarshallingPiPTemplateInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingPiPTemplateInfo02, Function | SmallTest | Level2)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.UnmarshallingPiPTemplateInfo(parcel, property);
}

/**
 * @tc.name: MarshallingPiPTemplateInfo
 * @tc.desc: MarshallingPiPTemplateInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, MarshallingPiPTemplateInfo, Function | SmallTest | Level2)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    auto info = property->GetPiPTemplateInfo();
    for (int i = 0; i < 10; i++) {
        info.controlGroup.push_back(i);
    }
    property->SetPiPTemplateInfo(info);
    bool result = property->MarshallingPiPTemplateInfo(parcel);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: SetIsPcAppInPad/GetIsPcAppInPad
 * @tc.desc: SetIsPcAppInPad/GetIsPcAppInPad
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetIsPcAppInPad, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool isPcAppInPad = true;
    property->SetIsPcAppInPad(isPcAppInPad);
    auto result = property->GetIsPcAppInPad();
    ASSERT_EQ(result, isPcAppInPad);
}

/**
 * @tc.name: SetSubWindowLevel
 * @tc.desc: SetSubWindowLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSubWindowLevel, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    uint32_t level = 4;
    property->SetSubWindowLevel(level);
    ASSERT_EQ(level, property->GetSubWindowLevel());
}

/**
 * @tc.name: GetSubWindowLevel
 * @tc.desc: GetSubWindowLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetSubWindowLevel, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetSubWindowLevel(1);
    ASSERT_EQ(1, property->GetSubWindowLevel());
}

/**
 * @tc.name: GetSubWindowZLevel
 * @tc.desc: GetSubWindowZLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetSubWindowZLevel, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t zLevel = 1;
    property->zLevel_ = zLevel;
    ASSERT_EQ(zLevel, property->GetSubWindowZLevel());
}

/**
 * @tc.name: SetSubWindowZLevel
 * @tc.desc: SetSubWindowZLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSubWindowZLevel, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t zLevel = 1;
    property->SetSubWindowZLevel(zLevel);
    ASSERT_EQ(zLevel, property->zLevel_);
}

/**
 * @tc.name: SetAndIsSystemKeyboard
 * @tc.desc: SetIsSystemKeyboard and IsSystemKeyboard Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndIsSystemKeyboard, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_EQ(false, property->IsSystemKeyboard());
    property->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, property->IsSystemKeyboard());
}

/**
 * @tc.name: SetAvoidAreaOption
 * @tc.desc: SetAvoidAreaOption Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAvoidAreaOption, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint32_t avoidAreaOption = 0;
    property->SetAvoidAreaOption(avoidAreaOption);
    ASSERT_EQ(avoidAreaOption, property->GetAvoidAreaOption());
    avoidAreaOption = 2;
    property->SetAvoidAreaOption(avoidAreaOption);
    ASSERT_EQ(avoidAreaOption, property->GetAvoidAreaOption());
}

/**
 * @tc.name: GetAvoidAreaOption
 * @tc.desc: GetAvoidAreaOption Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetAvoidAreaOption, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint32_t avoidAreaOption = 2;
    property->SetAvoidAreaOption(avoidAreaOption);
    ASSERT_EQ(2, property->GetAvoidAreaOption());
}

/**
 * @tc.name: SetBackgroundAlpha
 * @tc.desc: SetBackgroundAlpha Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetBackgroundAlpha, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint8_t backgroundAlpha = 0;
    property->SetBackgroundAlpha(backgroundAlpha);
    ASSERT_EQ(backgroundAlpha, property->GetBackgroundAlpha());
    backgroundAlpha = 2;
    property->SetBackgroundAlpha(backgroundAlpha);
    ASSERT_EQ(backgroundAlpha, property->GetBackgroundAlpha());
}

/**
 * @tc.name: GetBackgroundAlpha
 * @tc.desc: GetBackgroundAlpha Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetBackgroundAlpha, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint8_t backgroundAlpha = 2;
    property->SetBackgroundAlpha(backgroundAlpha);
    ASSERT_EQ(2, property->GetBackgroundAlpha());
}

/**
 * @tc.name: SetWindowCornerRadius
 * @tc.desc: SetWindowCornerRadius
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetWindowCornerRadius, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    float cornerRadius = 1.0f;
    property->SetWindowCornerRadius(cornerRadius);
    ASSERT_EQ(cornerRadius, property->GetWindowCornerRadius());
}

/**
 * @tc.name: GetIsAtomicService
 * @tc.desc: GetIsAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetIsAtomicService, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isAtomicService = true;
    property->SetIsAtomicService(isAtomicService);
    auto result = property->GetIsAtomicService();
    ASSERT_EQ(result, isAtomicService);
}

/**
 * @tc.name: SetIsSaveBySpecifiedFlag
 * @tc.desc: SetIsSaveBySpecifiedFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetIsSaveBySpecifiedFlag, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isSaveBySpecifiedFlag = false;
    property->SetIsSaveBySpecifiedFlag(isSaveBySpecifiedFlag);
    ASSERT_EQ(isSaveBySpecifiedFlag, property->GetIsSaveBySpecifiedFlag());
}
} // namespace
} // namespace Rosen
} // namespace OHOS
