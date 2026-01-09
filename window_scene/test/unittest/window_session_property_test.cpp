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

void WindowSessionPropertyTest::SetUpTestCase() {}

void WindowSessionPropertyTest::TearDownTestCase() {}

namespace {
/**
 * @tc.name: SetDragEnabled001
 * @tc.desc: SetDragEnabled and GetDragEnabled to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetDragEnabled001, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetRaiseEnabled001, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, WindowSessionProperty, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetSessionInfo, TestSize.Level1)
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
 * @tc.name: GetRealTimeSwitchInfo
 * @tc.desc: GetRealTimeSwitchInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetRealTimeSwitchInfo, TestSize.Level1)
{
    WindowSessionProperty *propertyWindow = new WindowSessionProperty();
    ASSERT_EQ(propertyWindow->GetRealTimeSwitchInfo().isNeedChange_, false);
    ASSERT_EQ(propertyWindow->GetRealTimeSwitchInfo().showTypes_, 0);
}

/**
 * @tc.name: IsAdaptToCompatibleDevice
 * @tc.desc: IsAdaptToCompatibleDevice
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsAdaptToCompatibleDevice, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->compatibleModeProperty_ = nullptr;
    EXPECT_EQ(property->IsAdaptToCompatibleDevice(), false);
    property->compatibleModeProperty_ = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToCompatibleDevice(), false);
    property->compatibleModeProperty_->SetIsAdaptToCompatibleDevice(true);
    EXPECT_EQ(property->compatibleModeProperty_->IsAdaptToCompatibleDevice(), true);
    EXPECT_EQ(property->IsAdaptToCompatibleDevice(), true);
}

/**
 * @tc.name: SetPrivacyMode
 * @tc.desc: SetPrivacyMode as true and false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetPrivacyMode, TestSize.Level0)
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
HWTEST_F(WindowSessionPropertyTest, SetSystemPrivacyMode, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetBrightness, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTopmost, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetMainWindowTopmost, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetParentId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetWindowFlags, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetAndGetPipTemplateInfo, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    PiPTemplateInfo pipTemplateInfo;
    pipTemplateInfo.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL);
    property->SetPiPTemplateInfo(pipTemplateInfo);
    ASSERT_EQ(property->GetPiPTemplateInfo().pipTemplateType, static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL));
}

/**
 * @tc.name: SetAndGetRealParentId
 * @tc.desc: SetRealParentId and GetRealParentId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndGetRealParentId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetAndGetUIExtensionUsage, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetParentWindowType, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetAndGetIsUIExtensionAbilityProcess, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, AddWindowFlag, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, IsTurnScreenOn, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, IsKeepScreenOn, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetAccessTokenId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTokenState, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetMaximizeMode, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetSystemBarProperty, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, IsDecorEnable, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetWindowModeSupportType, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, IsFloatingWindowAppType, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTouchHotAreas, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    Rect rect{ 4, 4, 4, 4 };
    std::vector<Rect> vRect{ rect };
    property->SetPersistentId(0);
    property->SetSessionPropertyChangeCallback(nullptr);
    EXPECT_EQ(nullptr, property->touchHotAreasChangeCallback_);
    property->SetTouchHotAreas(vRect);

    auto func = []() {};
    property->SetPersistentId(1);
    property->SetSessionPropertyChangeCallback(func);
    property->SetTouchHotAreas(vRect);
    EXPECT_NE(nullptr, property->touchHotAreasChangeCallback_);

    Rect rect1{ 5, 5, 5, 5 };
    vRect.emplace_back(rect1);
    property->SetTouchHotAreas(vRect);
}

/**
 * @tc.name: SetKeyboardTouchHotAreas
 * @tc.desc: SetKeyboardTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetKeyboardTouchHotAreas, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas hotAreas;
    Rect rect{ 4, 4, 4, 4 };
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

    Rect rect1{ 5, 5, 5, 5 };
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
HWTEST_F(WindowSessionPropertyTest, UnmarshallingWindowLimits, TestSize.Level1)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.UnmarshallingWindowLimits(parcel, property);
}

/**
 * @tc.name: Unmarshalling
 * @tc.desc: test whether unmarshalling property is ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, Unmarshalling, TestSize.Level1)
{
    std::string winName = "test";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowName(winName);
    Parcel parcel = Parcel();
    property->Marshalling(parcel);
    sptr<WindowSessionProperty> property2 = property->Unmarshalling(parcel);
    ASSERT_NE(property2, nullptr);
    EXPECT_EQ(property2->GetWindowName(), winName);

    property->compatibleModeProperty_ = sptr<CompatibleModeProperty>::MakeSptr();
    property->compatibleModeProperty_->SetIsAdaptToCompatibleDevice(true);
    EXPECT_EQ(property->IsAdaptToCompatibleDevice(), true);
    Parcel parcel2 = Parcel();
    property->compatibleModeProperty_->Marshalling(parcel2);
    sptr<CompatibleModeProperty> compatibleModeProperty = property->compatibleModeProperty_->Unmarshalling(parcel2);
    ASSERT_NE(compatibleModeProperty, nullptr);
    EXPECT_EQ(compatibleModeProperty->IsAdaptToCompatibleDevice(), true);
}

/**
 * @tc.name: UnMarshallingSystemBarMap
 * @tc.desc: UnMarshallingSystemBarMap test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnMarshallingSystemBarMap, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, UnmarshallingTouchHotAreas, TestSize.Level1)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    Rect rect{ 4, 4, 4, 4 };
    std::vector<Rect> vRect{ rect };
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
HWTEST_F(WindowSessionPropertyTest, UnmarshallingKeyboardTouchHotAreas, TestSize.Level1)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas hotAreas;
    Rect rect{ 4, 4, 4, 4 };
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
HWTEST_F(WindowSessionPropertyTest, UnmarshallingPiPTemplateInfo, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, CopyFrom, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetFocusable, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetFocusable(true);
    ASSERT_EQ(property->GetFocusable(), true);
    property->SetFocusable(false);
    ASSERT_EQ(property->GetFocusable(), false);
}

/**
 * @tc.name: SetForceHide
 * @tc.desc: SetForceHide and GetForceHide to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetForceHide, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetSystemCalling, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetIsNeedUpdateWindowMode, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetIsShaped, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetCollaboratorType, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetUserWindowLimits, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetConfigWindowLimitsVP, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetLastLimitsVpr, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetFullScreenStart, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetHideNonSystemFloatingWindows, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, KeepKeyboardOnFocus, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTextFieldPositionY, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTextFieldHeight, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetIsLayoutFullScreen, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, Read, TestSize.Level1)
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
    property->Read(parcel, WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE);
    ASSERT_EQ(property->GetPersistentId(), INVALID_SESSION_ID);
}

/**
 * @tc.name: Write
 * @tc.desc: Write and Read to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, Write, TestSize.Level1)
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
    property->Write(parcel, WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE);
    ASSERT_EQ(property->GetPersistentId(), INVALID_SESSION_ID);
}

/**
 * @tc.name: GetWindowName
 * @tc.desc: GetWindowName
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowName, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetSessionInfo, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, EditSessionInfo, TestSize.Level1)
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
 * @tc.name: SetGlobalDisplayRect
 * @tc.desc: test whether get the value that set before
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetGlobalDisplayRect, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    Rect rect = { 10, 10, 20, 20 };
    property->SetGlobalDisplayRect(rect);
    auto result = property->GetGlobalDisplayRect();
    EXPECT_EQ(result, rect);
}

/**
 * @tc.name: GetWindowRect
 * @tc.desc: GetWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowRect, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    Rect rect = { 0, 0, 0, 0 };
    property->SetWindowRect(rect);
    auto result = property->GetWindowRect();
    ASSERT_EQ(result, rect);
}

/**
 * @tc.name: GetWindowSizeLimits
 * @tc.desc: GetWindowSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowSizeLimits, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetRequestRect, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    Rect requestRect = { 0, 0, 0, 0 };
    property->SetRequestRect(requestRect);
    auto result = property->GetRequestRect();
    ASSERT_EQ(result, requestRect);
}

/**
 * @tc.name: GetWindowType
 * @tc.desc: GetWindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowType, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetDisplayId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetPersistentId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetParentPersistentId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTurnScreenOn, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetKeepScreenOn, TestSize.Level1)
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
 * @tc.name: SetViewKeepScreenOn
 * @tc.desc: SetViewKeepScreenOn And IsViewKeepScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetViewKeepScreenOn, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool keepScreenOn = true;
    property->SetViewKeepScreenOn(keepScreenOn);
    ASSERT_EQ(keepScreenOn, property->IsViewKeepScreenOn());
    keepScreenOn = false;
    property->SetViewKeepScreenOn(keepScreenOn);
    ASSERT_EQ(keepScreenOn, property->IsViewKeepScreenOn());
}

/**
 * @tc.name: SetWindowShadowEnabled
 * @tc.desc: SetWindowShadowEnabled And GetWindowShadowEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetWindowShadowEnabled, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool isEnabled = true;
    property->SetWindowShadowEnabled(isEnabled);
    EXPECT_NE(isEnabled, property->GetWindowShadowEnabled());
    isEnabled = false;
    property->SetWindowShadowEnabled(isEnabled);
    EXPECT_NE(isEnabled, property->GetWindowShadowEnabled());
}

/**
 * @tc.name: MarshallingSessionInfo
 * @tc.desc: MarshallingSessionInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, MarshallingSessionInfo, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, UnMarshallingSessionInfo, TestSize.Level1)
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
 * @tc.name: MarshallingTransitionAnimationMap
 * @tc.desc: MarshallingTransitionAnimationMap test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, MarshallingTransitionAnimationMap, TestSize.Level1)
{
    Parcel parcel;
    TransitionAnimation animation;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool result = property->MarshallingTransitionAnimationMap(parcel);
    ASSERT_EQ(result, true);
    property->transitionAnimationConfig_[WindowTransitionType::DESTROY] =
        std::make_shared<TransitionAnimation>(animation);
    result = property->MarshallingTransitionAnimationMap(parcel);
    ASSERT_EQ(result, true);
    property->transitionAnimationConfig_[WindowTransitionType::DESTROY] = nullptr;
    result = property->MarshallingTransitionAnimationMap(parcel);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: UnmarshallingTransitionAnimationMap
 * @tc.desc: UnmarshallingTransitionAnimationMap test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingTransitionAnimationMap, TestSize.Level1)
{
    Parcel parcel;
    TransitionAnimation animation;
    WindowSessionProperty windowSessionProperty;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->transitionAnimationConfig_[WindowTransitionType::DESTROY] =
        std::make_shared<TransitionAnimation>(animation);
    bool result = property->MarshallingTransitionAnimationMap(parcel);
    ASSERT_EQ(result, true);
    result = property->UnmarshallingTransitionAnimationMap(parcel, &windowSessionProperty);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: SetAccessTokenId
 * @tc.desc: SetAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAccessTokenId, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetWindowState, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetSystemPrivacyMode02, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetTokenState02, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, MarshallingTouchHotAreas, TestSize.Level1)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    std::vector<Rect> rects;
    for (int i = 0; i < 55; i++) {
        Rect rect{ i, i, i, i };
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
HWTEST_F(WindowSessionPropertyTest, MarshallingKeyboardTouchHotAreas, TestSize.Level1)
{
    Parcel parcel;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas hotAreas;
    for (int i = 0; i < 55; i++) {
        Rect rect{ i, i, i, i };
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
HWTEST_F(WindowSessionPropertyTest, UnmarshallingPiPTemplateInfo02, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, MarshallingPiPTemplateInfo, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetIsPcAppInPad, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    bool isPcAppInLargeScreenDevice = true;
    property->SetIsPcAppInPad(isPcAppInLargeScreenDevice);
    auto result = property->GetIsPcAppInPad();
    ASSERT_EQ(result, isPcAppInLargeScreenDevice);
}

/**
 * @tc.name: SetSubWindowLevel
 * @tc.desc: SetSubWindowLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetSubWindowLevel, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetSubWindowLevel, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetSubWindowZLevel, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetSubWindowZLevel, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t zLevel = 1;
    property->SetSubWindowZLevel(zLevel);
    ASSERT_EQ(zLevel, property->zLevel_);
}

/**
 * @tc.name: GetWindowAnchorInfo
 * @tc.desc: GetWindowAnchorInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetWindowAnchorInfo, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowAnchorInfo anchorInfo = { true, WindowAnchor::TOP_START, 0, 0 };
    property->windowAnchorInfo_ = anchorInfo;
    EXPECT_EQ(anchorInfo, property->GetWindowAnchorInfo());
}

/**
 * @tc.name: SetWindowAnchorInfo
 * @tc.desc: SetWindowAnchorInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetWindowAnchorInfo, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowAnchorInfo anchorInfo = { true, WindowAnchor::TOP_START, 0, 0 };
    property->SetWindowAnchorInfo(anchorInfo);
    EXPECT_EQ(anchorInfo, property->windowAnchorInfo_);
}

/**
 * @tc.name: UnmarshallingWindowAnchorInfo
 * @tc.desc: UnmarshallingWindowAnchorInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingWindowAnchorInfo, TestSize.Level1)
{
    Parcel parcel = Parcel();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.UnmarshallingWindowAnchorInfo(parcel, property);
    EXPECT_EQ(property->GetTokenState(), false);
}

/**
 * @tc.name: GetZIndex
 * @tc.desc: GetZIndex Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetZIndex, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t zIndex = 1;
    property->zIndex_ = zIndex;
    ASSERT_EQ(zIndex, property->GetZIndex());
}

/**
 * @tc.name: SetZIndex
 * @tc.desc: SetZIndex Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetZIndex, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t zIndex = 1;
    property->SetZIndex(zIndex);
    ASSERT_EQ(zIndex, property->zIndex_);
}
/**
 * @tc.name: SetAndIsSystemKeyboard
 * @tc.desc: SetIsSystemKeyboard and IsSystemKeyboard Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetAndIsSystemKeyboard, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetAvoidAreaOption, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetAvoidAreaOption, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, SetBackgroundAlpha, TestSize.Level1)
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
HWTEST_F(WindowSessionPropertyTest, GetBackgroundAlpha, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint8_t backgroundAlpha = 2;
    property->SetBackgroundAlpha(backgroundAlpha);
    ASSERT_EQ(2, property->GetBackgroundAlpha());
}

/**
 * @tc.name: GetIsAtomicService
 * @tc.desc: GetIsAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetIsAtomicService, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isAtomicService = true;
    property->SetIsAtomicService(isAtomicService);
    auto result = property->GetIsAtomicService();
    ASSERT_EQ(result, isAtomicService);
}

/**
 * @tc.name: SetPcAppInpadCompatibleMode
 * @tc.desc: SetPcAppInpadCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetPcAppInpadCompatibleMode, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool enabled = true;
    property->SetPcAppInpadCompatibleMode(enabled);
    auto result = property->GetPcAppInpadCompatibleMode();
    ASSERT_EQ(result, enabled);
}

/**
 * @tc.name: SetPcAppInpadSpecificSystemBarInvisible
 * @tc.desc: SetPcAppInpadSpecificSystemBarInvisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetPcAppInpadSpecificSystemBarInvisible, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isPcAppInpadSpecificSystemBarInvisible = true;
    property->SetPcAppInpadSpecificSystemBarInvisible(isPcAppInpadSpecificSystemBarInvisible);
    auto result = property->GetPcAppInpadSpecificSystemBarInvisible();
    ASSERT_EQ(result, isPcAppInpadSpecificSystemBarInvisible);
}

/**
 * @tc.name: SetPcAppInpadOrientationLandscape
 * @tc.desc: SetPcAppInpadOrientationLandscape
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetPcAppInpadOrientationLandscape, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isPcAppInpadOrientationLandscape = true;
    property->SetPcAppInpadOrientationLandscape(isPcAppInpadOrientationLandscape);
    auto result = property->GetPcAppInpadOrientationLandscape();
    ASSERT_EQ(result, isPcAppInpadOrientationLandscape);
}

/**
 * @tc.name: SetMobileAppInPadLayoutFullScreen
 * @tc.desc: SetMobileAppInPadLayoutFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetMobileAppInPadLayoutFullScreen, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isMobileAppInPadLayoutFullScreen = true;
    property->SetMobileAppInPadLayoutFullScreen(isMobileAppInPadLayoutFullScreen);
    auto result = property->GetMobileAppInPadLayoutFullScreen();
    EXPECT_EQ(result, isMobileAppInPadLayoutFullScreen);
}

/**
 * @tc.name: UnmarshallingFbTemplateInfoTest
 * @tc.desc: UnmarshallingFbTemplateInfoTest
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, UnmarshallingFbTemplateInfoTest, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_FB);

    Parcel parcel;
    std::shared_ptr<Media::PixelMap> icon;
    FloatingBallTemplateInfo fbTemplateInfo {{1, "fb", "fb_content", "red"}, icon};
    property->UnmarshallingFbTemplateInfo(parcel, property);
    ASSERT_NE(property->GetFbTemplateInfo().template_, fbTemplateInfo.template_);
    ASSERT_NE(property->GetFbTemplateInfo().title_, fbTemplateInfo.title_);
    ASSERT_NE(property->GetFbTemplateInfo().content_, fbTemplateInfo.content_);
    ASSERT_NE(property->GetFbTemplateInfo().backgroundColor_, fbTemplateInfo.backgroundColor_);
    ASSERT_EQ(property->GetFbTemplateInfo().icon_, fbTemplateInfo.icon_);

    parcel.WriteParcelable(&fbTemplateInfo);
    property->UnmarshallingFbTemplateInfo(parcel, property);
    ASSERT_EQ(property->GetFbTemplateInfo().template_, fbTemplateInfo.template_);
    ASSERT_EQ(property->GetFbTemplateInfo().title_, fbTemplateInfo.title_);
    ASSERT_EQ(property->GetFbTemplateInfo().content_, fbTemplateInfo.content_);
    ASSERT_EQ(property->GetFbTemplateInfo().backgroundColor_, fbTemplateInfo.backgroundColor_);
    ASSERT_EQ(property->GetFbTemplateInfo().icon_, fbTemplateInfo.icon_);
}

/**
 * @tc.name: SetIsShowDecorInFreeMultiWindow
 * @tc.desc: Test SetIsShowDecorInFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetIsShowDecorInFreeMultiWindow, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isShow = true;
    property->SetIsShowDecorInFreeMultiWindow(isShow);
    ASSERT_EQ(isShow, property->GetIsShowDecorInFreeMultiWindow());
    isShow = false;
    property->SetIsShowDecorInFreeMultiWindow(isShow);
    ASSERT_EQ(isShow, property->GetIsShowDecorInFreeMultiWindow());
}

/**
 * @tc.name: GetIsShowDecorInFreeMultiWindow
 * @tc.desc: Test GetIsShowDecorInFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetIsShowDecorInFreeMultiWindow, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    bool isShow = true;
    property->SetIsShowDecorInFreeMultiWindow(isShow);
    ASSERT_EQ(true, property->GetIsShowDecorInFreeMultiWindow());
}

/**
 * @tc.name: SetRotationLocked
 * @tc.desc: Test SetRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, SetRotationLocked, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetRotationLocked(true);
    EXPECT_EQ(property->isRotationLock_, true);
    property->SetRotationLocked(false);
    EXPECT_EQ(property->isRotationLock_, false);
}
 
/**
 * @tc.name: GetRotationLocked
 * @tc.desc: Test GetRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetRotationLocked, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isRotationLock_ = true;
    EXPECT_EQ(property->GetRotationLocked(), true);
    property->isRotationLock_ = false;
    EXPECT_EQ(property->GetRotationLocked(), false);
}

/**
 * @tc.name: FrameNum
 * @tc.desc: Test FrameNum
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, FrameNum, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetFrameNum(3);
    EXPECT_EQ(property->GetFrameNum(), 3);
}

/**
 * @tc.name: Prelaunch
 * @tc.desc: Test Prelaunch
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, Prelaunch, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrelaunch(true);
    EXPECT_TRUE(property->IsPrelaunch());
}

/**
 * @tc.name: AddKeyboardLayoutParams
 * @tc.desc: Test AddKeyboardLayoutParams
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, AddKeyboardLayoutParams, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_TRUE(property->keyboardLayoutParamsMap_.empty());
    uint64_t screenId = 1;
    KeyboardLayoutParams params;
    const Rect expectedRect = { 1, 2, 3, 4 };
    params.LandscapeKeyboardRect_ = expectedRect;
    params.LandscapePanelRect_ = expectedRect;
    params.PortraitKeyboardRect_ = expectedRect;
    params.PortraitPanelRect_ = expectedRect;
    params.displayId_ = 10;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    property->AddKeyboardLayoutParams(screenId, params);
    ASSERT_EQ(property->keyboardLayoutParamsMap_.size(), 1);
    const auto value = property->keyboardLayoutParamsMap_[screenId];
    EXPECT_EQ(value.displayId_, 10);
    EXPECT_EQ(value.gravity_, WindowGravity::WINDOW_GRAVITY_BOTTOM);
    EXPECT_EQ(value.LandscapeKeyboardRect_, expectedRect);
    EXPECT_EQ(value.LandscapePanelRect_, expectedRect);
    EXPECT_EQ(value.PortraitKeyboardRect_, expectedRect);
    EXPECT_EQ(value.PortraitPanelRect_, expectedRect);
}

HWTEST_F(WindowSessionPropertyTest, AddKeyboardLayoutParams_InvalidDisplayId, TestSize.Level0)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_TRUE(property->keyboardLayoutParamsMap_.empty());

    // adding with invalid displayid results in a no-op
    KeyboardLayoutParams params;
    property->AddKeyboardLayoutParams(DISPLAY_ID_INVALID, params);
    ASSERT_TRUE(property->keyboardLayoutParamsMap_.empty());
}

/**
 * @tc.name: ClearCachedKeyboardParamsOnScreenDisconnected
 * @tc.desc: Test ClearCachedKeyboardParamsOnScreenDisconnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, ClearCachedKeyboardParamsOnScreenDisconnected, TestSize.Level0)
{
    auto property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_TRUE(property->keyboardLayoutParamsMap_.empty());
    property->ClearCachedKeyboardParamsOnScreenDisconnected(1); // should be no-op
    ASSERT_TRUE(property->keyboardLayoutParamsMap_.empty());

    // insert two entries
    KeyboardLayoutParams params1, params2;
    property->AddKeyboardLayoutParams(1, params1);
    property->AddKeyboardLayoutParams(2, params2);
    ASSERT_EQ(property->keyboardLayoutParamsMap_.size(), 2);

    property->ClearCachedKeyboardParamsOnScreenDisconnected(1);
    ASSERT_EQ(property->keyboardLayoutParamsMap_.size(), 1);
    EXPECT_TRUE(property->keyboardLayoutParamsMap_.find(1) == property->keyboardLayoutParamsMap_.end());
    EXPECT_TRUE(property->keyboardLayoutParamsMap_.find(2) != property->keyboardLayoutParamsMap_.end());

    // clear an invalid screenId results in no-op
    property->ClearCachedKeyboardParamsOnScreenDisconnected(3);
    ASSERT_EQ(property->keyboardLayoutParamsMap_.size(), 1);
    EXPECT_TRUE(property->keyboardLayoutParamsMap_.find(2) != property->keyboardLayoutParamsMap_.end());
}

/**
 * @tc.name: GetKeyboardLayoutParamsByScreenId
 * @tc.desc: Test GetKeyboardLayoutParamsByScreenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, GetKeyboardLayoutParamsByScreenId, TestSize.Level0)
{
    auto property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_TRUE(property->keyboardLayoutParamsMap_.empty());

    KeyboardLayoutParams params;
    const Rect defaultRect = { 1, 2, 3, 4 };
    params.LandscapeKeyboardRect_ = defaultRect;
    params.displayId_ = 1234;
    property->GetKeyboardLayoutParamsByScreenId(1, params); // should be no-op
    ASSERT_EQ(params.LandscapeKeyboardRect_, defaultRect);

    KeyboardLayoutParams insertedParams;
    insertedParams.LandscapeKeyboardRect_ = { 5, 6, 7, 8 };
    insertedParams.displayId_ = 5678;
    property->AddKeyboardLayoutParams(1, insertedParams);
    ASSERT_EQ(property->keyboardLayoutParamsMap_.size(), 1);
    property->GetKeyboardLayoutParamsByScreenId(1, params);
    EXPECT_EQ(params.LandscapeKeyboardRect_, insertedParams.LandscapeKeyboardRect_);
    EXPECT_EQ(params.displayId_, insertedParams.displayId_);
}

/**
 * @tc.name: IsSameForceSplitConfig01
 * @tc.desc: Test IsSameForceSplitConfig when configs are identical
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig01, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = false;
    preconfig.containsAppConfig_ = false;

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = false;
    config.containsAppConfig_ = false;

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSameForceSplitConfig02
 * @tc.desc: Test IsSameForceSplitConfig when mode differs
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig02, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = false;
    preconfig.containsAppConfig_ = false;

    AppForceLandscapeConfig config;
    config.mode_ = 6;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = false;
    config.containsAppConfig_ = false;

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSameForceSplitConfig03
 * @tc.desc: Test IsSameForceSplitConfig when supportSplit differs
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig03, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = false;
    preconfig.containsAppConfig_ = false;

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 2;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = false;
    config.containsAppConfig_ = false;

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSameForceSplitConfig04
 * @tc.desc: Test IsSameForceSplitConfig when containsSysConfig is true and sys configs match
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig04, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = true;
    preconfig.isSysRouter_ = true;
    preconfig.sysHomePage_ = "home";
    preconfig.sysConfigJsonStr_ = "sysConfig";
    preconfig.containsAppConfig_ = false;

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = true;
    config.isSysRouter_ = true;
    config.sysHomePage_ = "home";
    config.sysConfigJsonStr_ = "sysConfig";
    config.containsAppConfig_ = false;

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSameForceSplitConfig05
 * @tc.desc: Test IsSameForceSplitConfig when containsSysConfig is true and sysHomePage differs
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig05, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = true;
    preconfig.isSysRouter_ = true;
    preconfig.sysHomePage_ = "home1";
    preconfig.sysConfigJsonStr_ = "sysConfig";
    preconfig.containsAppConfig_ = false;

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = true;
    config.isSysRouter_ = true;
    config.sysHomePage_ = "home2";
    config.sysConfigJsonStr_ = "sysConfig";
    config.containsAppConfig_ = false;

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSameForceSplitConfig06
 * @tc.desc: Test IsSameForceSplitConfig when containsAppConfig is true and app configs match
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig06, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = false;
    preconfig.containsAppConfig_ = true;
    preconfig.isAppRouter_ = true;
    preconfig.appConfigJsonStr_ = "appConfig";

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = false;
    config.containsAppConfig_ = true;
    config.isAppRouter_ = true;
    config.appConfigJsonStr_ = "appConfig";

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSameForceSplitConfig07
 * @tc.desc: Test IsSameForceSplitConfig when containsAppConfig is true and appConfigJsonStr differs
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig07, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = false;
    preconfig.containsAppConfig_ = true;
    preconfig.isAppRouter_ = true;
    preconfig.appConfigJsonStr_ = "appConfig1";

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = false;
    config.containsAppConfig_ = true;
    config.isAppRouter_ = true;
    config.appConfigJsonStr_ = "appConfig2";

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSameForceSplitConfig08
 * @tc.desc: Test IsSameForceSplitConfig when containsSysConfig and containsAppConfig are both true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyTest, IsSameForceSplitConfig08, TestSize.Level1)
{
    AppForceLandscapeConfig preconfig;
    preconfig.mode_ = 5;
    preconfig.supportSplit_ = 1;
    preconfig.ignoreOrientation_ = false;
    preconfig.containsSysConfig_ = true;
    preconfig.isSysRouter_ = true;
    preconfig.sysHomePage_ = "home";
    preconfig.sysConfigJsonStr_ = "sysConfig";
    preconfig.containsAppConfig_ = true;
    preconfig.isAppRouter_ = true;
    preconfig.appConfigJsonStr_ = "appConfig";

    AppForceLandscapeConfig config;
    config.mode_ = 5;
    config.supportSplit_ = 1;
    config.ignoreOrientation_ = false;
    config.containsSysConfig_ = true;
    config.isSysRouter_ = true;
    config.sysHomePage_ = "home";
    config.sysConfigJsonStr_ = "sysConfig";
    config.containsAppConfig_ = true;
    config.isAppRouter_ = true;
    config.appConfigJsonStr_ = "appConfig";

    bool result = AppForceLandscapeConfig::IsSameForceSplitConfig(preconfig, config);
    EXPECT_EQ(result, true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
