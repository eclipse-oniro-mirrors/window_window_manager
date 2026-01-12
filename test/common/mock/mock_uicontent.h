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

#ifndef OHOS_ROSEN_TEST_COMMON_MOCK_UICONTENT_MOCKER
#define OHOS_ROSEN_TEST_COMMON_MOCK_UICONTENT_MOCKER

#include "ui_content.h"
#include "native_engine/native_value.h"
#include "native_engine/native_engine.h"
#include <gmock/gmock.h>
#include "accessibility_element_info.h"
namespace OHOS {
namespace Ace {
class UIContentMocker : public UIContent {
public:
    MOCK_METHOD3(Initialize, UIContentErrorCode(OHOS::Rosen::Window* window, const std::string& url,
                 napi_value storage));
    MOCK_METHOD3(Initialize, UIContentErrorCode(OHOS::Rosen::Window* window,
                 const std::shared_ptr<std::vector<uint8_t>>& content, napi_value storage));
    MOCK_METHOD3(InitializeByName, UIContentErrorCode(OHOS::Rosen::Window* window, const std::string& name,
                 napi_value storage));
    MOCK_METHOD4(Initialize,
        void(OHOS::Rosen::Window* window, const std::string& url, napi_value storage, uint32_t focusWindowID));
    MOCK_METHOD4(InitializeByName,
        void(OHOS::Rosen::Window* window, const std::string& url, napi_value storage, uint32_t focusWindowID));
    MOCK_METHOD0(Foreground, void());
    MOCK_METHOD0(Background, void());
    MOCK_METHOD0(Focus, void());
    MOCK_METHOD0(UnFocus, void());
    MOCK_METHOD0(Destroy, void());
    MOCK_METHOD1(OnNewWant, void(const OHOS::AAFwk::Want& want));
    MOCK_METHOD4(Restore, UIContentErrorCode(OHOS::Rosen::Window* window, const std::string& contentInfo,
                 napi_value storage, ContentInfoType type));
    MOCK_CONST_METHOD1(GetContentInfo, std::string(ContentInfoType type));
    MOCK_METHOD0(DestroyUIDirector, void());
    MOCK_METHOD0(ProcessBackPressed, bool());
    MOCK_METHOD1(ProcessPointerEvent, bool(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent));
    MOCK_METHOD2(ProcessKeyEvent, bool(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent, bool isPreIme));
    MOCK_METHOD1(ProcessAxisEvent, bool(const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent));
    MOCK_METHOD1(ProcessVsyncEvent, bool(uint64_t timeStampNanos));
    MOCK_METHOD1(UpdateConfiguration, void(const std::shared_ptr<OHOS::AppExecFwk::Configuration>& config));
    MOCK_METHOD2(UpdateConfiguration, void(const std::shared_ptr<OHOS::AppExecFwk::Configuration>& config,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager));
    MOCK_METHOD1(UpdateConfigurationSyncForAll, void(const std::shared_ptr<OHOS::AppExecFwk::Configuration>& config));
    MOCK_METHOD5(UpdateViewportConfig, void(const ViewportConfig& config, OHOS::Rosen::WindowSizeChangeReason reason,
        const std::shared_ptr<OHOS::Rosen::RSTransaction>& rsTransaction,
        const std::map<OHOS::Rosen::AvoidAreaType, OHOS::Rosen::AvoidArea>& avoidAreas,
        const sptr<OHOS::Rosen::OccupiedAreaChangeInfo>& info));
    MOCK_METHOD2(UpdateWindowMode, void(OHOS::Rosen::WindowMode mode, bool hasDeco));
    MOCK_METHOD2(GetTopNavDestinationInfo, std::string(bool onlyFullScreen, bool needParam));
    MOCK_METHOD1(SetFrameMetricsCallBack, void(std::function<void(FrameMetrics info)>&& callback));
    MOCK_METHOD4(HideWindowTitleButton, void(bool hideSplit, bool hideMaximize, bool hideMinimize, bool hideClose));
    MOCK_METHOD2(UpdateTitleInTargetPos, void(bool isShow, int32_t height));
    MOCK_METHOD0(GetBackgroundColor, uint32_t());
    MOCK_METHOD1(SetBackgroundColor, void(uint32_t color));
    MOCK_METHOD2(SetWindowContainerColor, void(uint32_t activeColor, uint32_t inactiveColor));
    MOCK_METHOD2(DumpInfo, void(const std::vector<std::string>& params, std::vector<std::string>& info));
    MOCK_METHOD1(SetNextFrameLayoutCallback, void(std::function<void()>&& callback));
    MOCK_METHOD1(NotifyMemoryLevel, void(int32_t level));
    MOCK_METHOD1(SetAppWindowTitle, void(const std::string& title));
    MOCK_METHOD1(SetAppWindowIcon, void(const std::shared_ptr<Media::PixelMap>& pixelMap));
    MOCK_METHOD3(PreInitializeForm, void(OHOS::Rosen::Window* window, const std::string& url, napi_value storage));
    MOCK_METHOD0(RunFormPage, void());
    MOCK_METHOD0(GetFormRootNode, std::shared_ptr<Rosen::RSSurfaceNode>());
    MOCK_METHOD1(UpdateFormData, void(const std::string& data));
    MOCK_METHOD1(SetFormWidth, void(const float width));
    MOCK_METHOD1(SetFormHeight, void(const float height));
    MOCK_METHOD0(GetFormWidth, float());
    MOCK_METHOD0(GetFormHeight, float());
    MOCK_METHOD1(SetActionEventHandler, void(std::function<void(const std::string& action)>&& actionCallback));
    MOCK_METHOD1(SetErrorEventHandler,
        void(std::function<void(const std::string& code, const std::string& msg)>&& actionCallback));
    MOCK_METHOD1(SetIgnoreViewSafeArea, void(bool ignoreViewSafeArea));
    MOCK_METHOD1(SetIsFocusActive, void(bool isFocusActive));
    MOCK_METHOD3(CreateModalUIExtension, int32_t(const AAFwk::Want& want,
        const ModalUIExtensionCallbacks& callbacks, const ModalUIExtensionConfig& config));
    MOCK_METHOD1(CloseModalUIExtension, void(int32_t sessionId));
    MOCK_METHOD1(SetParentToken, void(sptr<IRemoteObject> token));
    MOCK_METHOD0(GetParentToken, sptr<IRemoteObject>());

    MOCK_METHOD4(
        SearchElementInfoByAccessibilityId, void(int64_t elementId,
        int32_t mode, int64_t baseParent, std::list<Accessibility::AccessibilityElementInfo>& output));
    MOCK_METHOD4(
        SearchElementInfosByText, void(int64_t elementId, const std::string& text,
        int64_t baseParent, std::list<Accessibility::AccessibilityElementInfo>& output));
    MOCK_METHOD4(
        FindFocusedElementInfo, void(int64_t elementId,
        int32_t focusType, int64_t baseParent, Accessibility::AccessibilityElementInfo& output));
    MOCK_METHOD4(
        FocusMoveSearch, void(int64_t elementId, int32_t direction, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& output));
    MOCK_METHOD1(GetAppPaintSize, void(OHOS::Rosen::Rect& drawableRect));
    MOCK_METHOD2(SetContainerModalTitleVisible, void(bool customTitleSettedShow, bool floatingTitleSettedShow));
    MOCK_METHOD1(GetContainerModalTitleVisible, bool(bool isImmersive));
    MOCK_METHOD1(SetContainerModalTitleHeight, void(int height));
    MOCK_METHOD0(GetContainerModalTitleHeight, int());
    MOCK_METHOD2(GetContainerModalButtonsRect, bool(OHOS::Rosen::Rect& containerModal, OHOS::Rosen::Rect& buttons));
    MOCK_METHOD1(SubscribeContainerModalButtonsRectChange,
        void(std::function<void(OHOS::Rosen::Rect& containerModal, OHOS::Rosen::Rect& buttons)>&& callback));
    MOCK_METHOD1(UpdateTransform, void(const OHOS::Rosen::Transform& transform));
    MOCK_METHOD1(SetUIExtensionSubWindow, void(bool isUIExtensionSubWindow));
    MOCK_METHOD1(SetUIExtensionAbilityProcess, void(bool isUIExtensionAbilityProcess));
    MOCK_METHOD0(IsUIExtensionSubWindow, bool());
    MOCK_METHOD0(IsUIExtensionAbilityProcess, bool());
    MOCK_METHOD4(NotifyExecuteAction, bool(int64_t elementId, const std::map<std::string, std::string>& actionAguments,
        int32_t action, int64_t baseParent));
    MOCK_METHOD1(SetContainerButtonStyle, void(const Rosen::DecorButtonStyle& decorButtonStyle));
};
} // namespace Ace
} // namespace OHOS

#endif
