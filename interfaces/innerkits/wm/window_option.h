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

#ifndef OHOS_ROSEN_WINDOW_OPTION_H
#define OHOS_ROSEN_WINDOW_OPTION_H
#include <refbase.h>
#include <string>
#include <unordered_map>

#include "../dm/dm_common.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
/**
 * @class WindowOption
 * WindowOption is used to create a window.
 */
class WindowOption : public RefBase {
public:
    /**
     * @brief Default construct of WindowOption.
     */
    WindowOption();

    /**
     * @brief Deconstruct of WindowOption.
     */
    virtual ~WindowOption() = default;

    /**
     * @brief Set window rect.
     *
     * @param rect The rect of window to set window position and size.
     */
    void SetWindowRect(const struct Rect& rect);

    /**
     * @brief Set window type.
     *
     * @param type The type of window.
     */
    void SetWindowType(WindowType type);

    /**
     * @brief Set window mode.
     *
     * @param mode The mode of window.
     */
    void SetWindowMode(WindowMode mode);

    /**
     * @brief Set window focusable.
     *
     * @param isFocusable True means the window can be focused, otherwise not.
     */
    void SetFocusable(bool isFocusable);

    /**
     * @brief Set window touchable.
     *
     * @param isTouchable True means the window can be touched, otherwise not.
     */
    void SetTouchable(bool isTouchable);

    /**
     * @brief Set display id.
     *
     * @param displayId The display id of window.
     */
    void SetDisplayId(DisplayId displayId);

    /**
     * @brief Set parent id.
     *
     * @param parentId The parent window id.
     */
    void SetParentId(uint32_t parentId);

    /**
     * @brief Set window name.
     *
     * @param windowName The window name.
     */
    void SetWindowName(const std::string& windowName);

    /**
     * @brief Set bundle name.
     *
     * @param bundleName The bundle name.
     */
    void SetBundleName(const std::string bundleName);

    /**
     * @brief Add window flag.
     *
     * @param flag The flag value added.
     */
    void AddWindowFlag(WindowFlag flag);

    /**
     * @brief Remove winodw flag.
     *
     * @param flag The flag value removed.
     */
    void RemoveWindowFlag(WindowFlag flag);

    /**
     * @brief Set window flags.
     *
     * @param flags The flag value.
     */
    void SetWindowFlags(uint32_t flags);

    /**
     * @brief Set system bar property.
     *
     * @param type The system bar window type.
     * @param property The system bar property.
     */
    void SetSystemBarProperty(WindowType type, const SystemBarProperty& property);

    /**
     * @brief Set hit offset.
     *
     * @param x The position x of hit offset.
     * @param y The position y of hit offset.
     */
    void SetHitOffset(int32_t x, int32_t y);
    /**
     * @brief Set window tag.
     *
     * @param windowTag The tag of window.
     */
    void SetWindowTag(WindowTag windowTag);

    /**
     * @brief Set window session type.
     *
     * @param sessionType The session type of window.
     */
    void SetWindowSessionType(WindowSessionType sessionType);

    /**
     * @brief Set keep screen on.
     *
     * @param keepScreenOn The window keep screen on or not.
     */
    void SetKeepScreenOn(bool keepScreenOn);

    /**
     * @brief Is keep screen on.
     *
     * @return Return true means the window would keep screen on, otherwise not.
     */
    bool IsKeepScreenOn() const;

    /**
     * @brief Set screen on.
     *
     * @param turnScreenOn mark the window to turn the screen on or not.
     */
    void SetTurnScreenOn(bool turnScreenOn);

    /**
     * @brief Is turned screen on.
     *
     * @return The window is marked to turn the screen on or not.
     */
    bool IsTurnScreenOn() const;

    /**
     * @brief Set window brightness.
     *
     * @param brightness The brightness of screen. the value is between 0.0 ~ 1.0.
     */
    void SetBrightness(float brightness);

    /**
     * @brief Set window requested orientation.
     *
     * @param orientation The requested orientation of window.
     */
    void SetRequestedOrientation(Orientation orientation);

    /**
     * @brief Set window calling window id.
     *
     * @param windowId The window id of calling window.
     */
    void SetCallingWindow(uint32_t windowId);

    /**
     * @brief Set window main handler available.
     *
     * @param isMainHandlerAvailable is window main handler available.
     */
    void SetMainHandlerAvailable(bool isMainHandlerAvailable);

    /**
     * @brief Set subwindow title.
     *
     * @param subWindowTitle the subwindow title.
     */
    void SetSubWindowTitle(const std::string& subWindowTitle);

    /**
     * @brief Set subwindow decor enable.
     *
     * @param subWindowDecorEnable the subwindow decor enable.
     */
    void SetSubWindowDecorEnable(bool subWindowDecorEnable);

    /**
     * @brief Set only sceneboard supported.
     *
     * @param onlySupportSceneBoard only sceneboard supported.
     */
    void SetOnlySupportSceneBoard(bool onlySupportSceneBoard);

    /**
     * @brief Set ExtensionSubWindow tag.
     *
     * @param isExtensionTag ExtensionSubWindow tag.
     */
    void SetExtensionTag(bool isExtensionTag);

    /**
     * @brief Set UIExtension usage.
     *
     * @param uiExtensionUsage UIExtension usage.
     */
    void SetUIExtensionUsage(uint32_t uiExtensionUsage);

    /**
     * @brief Set Dialog Decor Enable Or Not.
     *
     * @param decorEnable true means enable, default disabled.
     */
    void SetDialogDecorEnable(bool decorEnable);

    /**
     * @brief Set Dialog title.
     *
     * @param dialogTitle true means enable, default disabled.
     */
    void SetDialogTitle(const std::string& dialogTitle);

    /**
     * @brief Set window topmost.
     *
     * @param isTopmost true means enable, default disabled.
     */
    void SetWindowTopmost(bool isTopmost);

    /**
     * @brief Set parent window type of UIExtension
     *
     * @param parentWindowType Parent window type of UIExtension
     */
    void SetParentWindowType(WindowType parentWindowType);

    /**
     * @brief Get window rect.
     *
     * @return The rect of window.
     */
    Rect GetWindowRect() const;

    /**
     * @brief Get window type.
     *
     * @return The type of window.
     */
    WindowType GetWindowType() const;

    /**
     * @brief Get window mode.
     *
     * @return The mode of window.
     */
    WindowMode GetWindowMode() const;

    /**
     * @brief Get window focusable.
     *
     * @return Return true means the window is focusable, otherwise not.
     */
    bool GetFocusable() const;

    /**
     * @brief Get window touchable.
     *
     * @return Return true means the window is touchable, otherwise not.
     */
    bool GetTouchable() const;

    /**
     * @brief Get display id.
     *
     * @return Return diplay id.
     */
    DisplayId GetDisplayId() const;

    /**
     * @brief Get parent id.
     *
     * @return Return parent window id.
     */
    uint32_t GetParentId() const;

    /**
     * @brief Get window name.
     *
     * @return Return the window name.
     */
    const std::string& GetWindowName() const;

    /**
     * @brief Get bundle name.
     *
     * @return Return the bundle name.
     */
    const std::string GetBundleName() const;

    /**
     * @brief Get window flags.
     *
     * @return Return the window flags.
     */
    uint32_t GetWindowFlags() const;

    /**
     * @brief Get system bar property.
     *
     * @return Return system bar property map.
     */
    const std::unordered_map<WindowType, SystemBarProperty>& GetSystemBarProperty() const;

    /**
     * @brief Get window hit offset.
     *
     * @return Return hit offset value as PointInfo.
     */
    const PointInfo& GetHitOffset() const;

    /**
     * @brief Get window tag.
     *
     * @return Return window tag.
     */
    WindowTag GetWindowTag() const;

    /**
     * @brief Get window session type.
     *
     * @return Return window session type.
     */
    WindowSessionType GetWindowSessionType() const;

    /**
     * @brief Get window brightness.
     *
     * @return Return screen brightness.
     */
    float GetBrightness() const;

    /**
     * @brief Get window request orientation.
     *
     * @return Return window requested orientation.
     */
    Orientation GetRequestedOrientation() const;

    /**
     * @brief Get calling window id.
     *
     * @return Return the calling window id of window.
     */
    uint32_t GetCallingWindow() const;

    /**
     * @brief Get main handler available
     *
     * @return Return true means the main handler available, otherwise not.
     */
    bool GetMainHandlerAvailable() const;

    /**
     * @brief Get subwindow title
     *
     * @return Return the subwindow title
    */
    std::string GetSubWindowTitle() const;

    /**
     * @brief Get subwindow decor enable
     *
     * @return Return ture means the subwindow decor enabled, otherwise not.
    */
    bool GetSubWindowDecorEnable() const;

    /**
     * @brief Get only sceneboard supported
     *
     * @return Return ture means only sceneboard supported, otherwise not.
    */
    bool GetOnlySupportSceneBoard() const;

    /**
     * @brief Get only sceneboard supported
     *
     * @return Return ExtensionTag of subwindow.
    */
    bool GetExtensionTag() const;

    /**
     * @brief Get UIExtension usage.
     *
     * @param Return UIExtension usage.
     */
    uint32_t GetUIExtensionUsage() const;

    /**
     * @brief Get dialog decor enable
     *
     * @return true means the dialog decor is enabled, otherwise not.
    */
    bool GetDialogDecorEnable() const;

    /**
     * @brief Get dialog title
     *
     * @return Return the dialog title
    */
    std::string GetDialogTitle() const;

    /**
     * @brief Get window topmost
     *
     * @return true means the window is topmost, otherwise not.
    */
    bool GetWindowTopmost() const;

    /*
     * @brief Get the parent window type of UIExtension
     *
     * @return Parent window type of UIExtension
     */
    virtual WindowType GetParentWindowType() const;

    /**
     * @brief Set whether this window is a sub window of the UIExtension.
     *
     * @return isUIExtensionSubWindowFlag.
    */
    void SetIsUIExtensionSubWindowFlag(bool isUIExtensionSubWindowFlag);
    /**
     * @brief Get IsUIExtensionSubWindowFlag of window.
     *
     * @return true - is UIExtension sub window, false - is not UIExtension sub window.
     */
    bool GetIsUIExtensionSubWindowFlag() const;

private:
    Rect windowRect_ { 0, 0, 0, 0 };
    WindowType type_ { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW };
    WindowMode mode_ { WindowMode::WINDOW_MODE_UNDEFINED };
    bool focusable_ { true };
    bool touchable_ { true };
    DisplayId displayId_ { DISPLAY_ID_INVALID };
    uint32_t parentId_ = INVALID_WINDOW_ID;
    std::string windowName_ { "" };
    std::string bundleName_ { "" };
    uint32_t flags_ { 0 };
    PointInfo hitOffset_ { 0, 0 };
    WindowTag windowTag_;
    WindowSessionType sessionType_ { WindowSessionType::SCENE_SESSION };
    bool keepScreenOn_ = false;
    bool turnScreenOn_ = false;
    bool isMainHandlerAvailable_ = true;
    float brightness_ = UNDEFINED_BRIGHTNESS;
    uint32_t callingWindow_ = INVALID_WINDOW_ID;
    std::unordered_map<WindowType, SystemBarProperty> sysBarPropMap_ {
        { WindowType::WINDOW_TYPE_STATUS_BAR,     SystemBarProperty() },
        { WindowType::WINDOW_TYPE_NAVIGATION_BAR, SystemBarProperty() },
    };
    Orientation requestedOrientation_ { Orientation::UNSPECIFIED };
    std::string subWindowTitle_ = { "" };
    bool subWindowDecorEnable_ = false;
    bool onlySupportSceneBoard_ = false;
    bool dialogDecorEnable_ = false;
    std::string dialogTitle_ = { "" };
    bool isTopmost_ = false;

    /*
     * UIExtension
     */
    uint32_t uiExtensionUsage_ = static_cast<uint32_t>(UIExtensionUsage::EMBEDDED);
    bool isExtensionTag_ = false;
    bool isUIExtensionSubWindowFlag_ = false;
    WindowType parentWindowType_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_OPTION_H
