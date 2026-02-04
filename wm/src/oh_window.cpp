/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "oh_window.h"

#include <cstdint>
#include <functional>
#include <mutex>

#include "image/pixelmap_native.h"
#include "pixelmap_native_impl.h"
#include "get_snapshot_callback.h"
#include "ui_content.h"

#include <event_handler.h>
#include <event_runner.h>

#include "oh_input_manager.h"
#include "oh_window_comm.h"
#include "singleton_container.h"
#include "window.h"
#include "window_manager.h"
#include "window_manager_hilog.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t NORMAL_STATE_CHANGE = 0;
constexpr bool SHOW_WITH_NO_ANIMATION = false;
constexpr bool SHOW_WITH_FOCUS = true;
const int32_t MAX_SESSION_LIMIT_ALL_APP = 512;
std::shared_ptr<OHOS::AppExecFwk::EventHandler> g_eventHandler;
std::once_flag g_onceFlagForInitEventHandler;

inline bool IsMainWindow(WindowType type)
{
    return (type >= WindowType::APP_MAIN_WINDOW_BASE && type < WindowType::APP_MAIN_WINDOW_END);
}

inline bool IsMainWindowAndNotShown(WindowType type, WindowState state)
{
    return (IsMainWindow(type) && state != WindowState::STATE_SHOWN);
}
}

std::shared_ptr<OHOS::AppExecFwk::EventHandler> GetMainEventHandler()
{
    std::call_once(g_onceFlagForInitEventHandler, [] {
        g_eventHandler =
            std::make_shared<OHOS::AppExecFwk::EventHandler>(OHOS::AppExecFwk::EventRunner::GetMainEventRunner());
    });
    return g_eventHandler;
}

WindowManager_ErrorCode ShowWindowInner(int32_t windowId)
{
    auto eventHandler = GetMainEventHandler();
    WindowManager_ErrorCode ret = WindowManager_ErrorCode::OK;
    eventHandler->PostSyncTask([windowId, &ret] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is null, windowId:%{public}d", windowId);
            ret = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (IsMainWindowAndNotShown(window->GetType(), window->GetWindowState())) {
            TLOGNW(WmsLogTag::WMS_LIFE,
                "window Type %{public}u and window state %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(window->GetType()), static_cast<uint32_t>(window->GetWindowState()),
                window->GetWindowId(), window->GetWindowName().c_str());
            ret = WindowManager_ErrorCode::OK;
            return;
        }
        if (window->Show(NORMAL_STATE_CHANGE, SHOW_WITH_NO_ANIMATION, SHOW_WITH_FOCUS) == WMError::WM_OK) {
            ret = WindowManager_ErrorCode::OK;
        } else {
            ret = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] show with ret=%{public}d",
            window->GetWindowId(), window->GetWindowName().c_str(), ret);
    }, __func__);
    return ret;
}

WindowManager_ErrorCode IsWindowShownInner(int32_t windowId, bool* isShow)
{
    if (isShow == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "isShow is null");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    WindowManager_ErrorCode ret = WindowManager_ErrorCode::OK;
    eventHandler->PostSyncTask([windowId, isShow, &ret] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is null, windowId:%{public}d", windowId);
            ret = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        *isShow = window->GetWindowState() == WindowState::STATE_SHOWN;
    }, __func__);
    return ret;
}
} // namespace Rosen
} // namespace OHOS

int32_t OH_WindowManager_ShowWindow(int32_t windowId)
{
    return static_cast<int32_t>(OHOS::Rosen::ShowWindowInner(windowId));
}

int32_t OH_WindowManager_IsWindowShown(int32_t windowId, bool* isShow)
{
    return static_cast<int32_t>(OHOS::Rosen::IsWindowShownInner(windowId, isShow));
}

namespace {
#define WINDOW_MANAGER_FREE_MEMORY(ptr) \
    do { \
        if ((ptr)) { \
            free((void*)(ptr)); \
            (ptr) = NULL; \
        } \
    } while (0)

/*
 * Used to map from WMError to WindowManager_ErrorCode.
 */
const std::unordered_map<WMError, WindowManager_ErrorCode> OH_WINDOW_TO_ERROR_CODE_MAP {
    { WMError::WM_OK,                          WindowManager_ErrorCode::OK                                            },
    { WMError::WM_ERROR_INVALID_PARAM,         WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM        },
    { WMError::WM_ERROR_DEVICE_NOT_SUPPORT,    WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED },
    { WMError::WM_ERROR_INVALID_WINDOW,        WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL       },
    { WMError::WM_ERROR_INVALID_CALLING,       WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL      },
    { WMError::WM_ERROR_NULLPTR,               WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL      },
    { WMError::WM_ERROR_SYSTEM_ABNORMALLY,     WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL      },
    { WMError::WM_ERROR_INVALID_PERMISSION,    WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_NO_PERMISSION        },
};

/*
 * Used to map from WindowType to WindowManager_WindowType.
 */
const std::unordered_map<WindowType, WindowManager_WindowType> OH_WINDOW_TO_WINDOW_TYPE_MAP {
    { WindowType::WINDOW_TYPE_APP_SUB_WINDOW,      WindowManager_WindowType::WINDOW_MANAGER_WINDOW_TYPE_APP    },
    { WindowType::WINDOW_TYPE_DIALOG,              WindowManager_WindowType::WINDOW_MANAGER_WINDOW_TYPE_DIALOG },
    { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,     WindowManager_WindowType::WINDOW_MANAGER_WINDOW_TYPE_MAIN   },
    { WindowType::WINDOW_TYPE_FLOAT,               WindowManager_WindowType::WINDOW_MANAGER_WINDOW_TYPE_FLOAT  },
};

void TransformedToWindowManagerRect(const Rect& rect, WindowManager_Rect& wmRect)
{
    wmRect.posX = rect.posX_;
    wmRect.posY = rect.posY_;
    wmRect.width = rect.width_;
    wmRect.height = rect.height_;
}

void TransformedToMainWindowInfo(const OHOS::sptr<MainWindowInfo> mainWindowInfo,
    WindowManager_MainWindowInfo& wmMainWindowInfo)
{
    wmMainWindowInfo.displayId = mainWindowInfo->displayId_;
    wmMainWindowInfo.windowId = mainWindowInfo->persistentId_;
    wmMainWindowInfo.showing = mainWindowInfo->showing_;
    wmMainWindowInfo.label = mainWindowInfo->label_.c_str();
}
 
void TransformedToWindowSnapshotConfig(WindowSnapshotConfiguration& windowSnapshotConfiguration,
    const WindowManager_WindowSnapshotConfig& wmWindowSnapshotConfiguration)
{
    windowSnapshotConfiguration.useCache = wmWindowSnapshotConfiguration.useCache;
}

void TransformedToWindowManagerAvoidArea(const AvoidArea& allAvoidArea, WindowManager_AvoidArea* avoidArea)
{
    if (avoidArea == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "avoidArea is nullptr");
        return;
    }
    TransformedToWindowManagerRect(allAvoidArea.topRect_, avoidArea->topRect);
    TransformedToWindowManagerRect(allAvoidArea.leftRect_, avoidArea->leftRect);
    TransformedToWindowManagerRect(allAvoidArea.rightRect_, avoidArea->rightRect);
    TransformedToWindowManagerRect(allAvoidArea.bottomRect_, avoidArea->bottomRect);
}

void RegisterCallBackFunc(const OHOS::sptr<GetSnapshotCallback>& getSnapshotCallback,
    OH_WindowManager_WindowSnapshotCallback callback)
{
    getSnapshotCallback->RegisterFunc([callback]
        (WMError errCode, const std::vector<std::shared_ptr<OHOS::Media::PixelMap>>& pixelMaps) {
            if (!callback) {
                TLOGNE(WmsLogTag::WMS_LIFE, "callback is nullptr");
                return;
            }
            if (errCode != WMError::WM_OK) {
                TLOGNE(WmsLogTag::WMS_LIFE, "error: %{public}d", static_cast<int32_t>(errCode));
                callback(nullptr, 0);
                return;
            }
            const OH_PixelmapNative** pixelmapArray = new const OH_PixelmapNative* [pixelMaps.size()];
            if (pixelmapArray == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "alloc failed");
                callback(nullptr, 0);
                return;
            }
            for (size_t i = 0; i < pixelMaps.size(); i++) {
                if (!pixelMaps[i]) {
                    pixelmapArray[i] = nullptr;
                } else {
                    pixelmapArray[i] = new OH_PixelmapNative(pixelMaps[i]);
                }
            }
            size_t count = pixelMaps.size();
            callback(pixelmapArray, count);
        });
}
} // namespace

int32_t OH_WindowManager_GetWindowAvoidArea(
    int32_t windowId, WindowManager_AvoidAreaType type, WindowManager_AvoidArea* avoidArea)
{
    if (avoidArea == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "avoidArea is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, type, avoidArea, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        AvoidArea allAvoidArea;
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(
            window->GetAvoidAreaByType(static_cast<AvoidAreaType>(type), allAvoidArea));
        TransformedToWindowManagerAvoidArea(allAvoidArea, avoidArea);
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowStatusBarEnabled(int32_t windowId, bool enabled, bool enableAnimation)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, enabled, enableAnimation, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (window->IsPcWindow()) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s device is not support, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED;
            return;
        }
        auto property = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        property.enable_ = enabled;
        property.settingFlag_ = static_cast<SystemBarSettingFlag>(
            static_cast<uint32_t>(property.settingFlag_) | static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING));
        property.enableAnimation_ = enableAnimation;
        SystemBarPropertyFlag propertyFlag = { true, false, false, true };
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(
            window->UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, property, propertyFlag));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowStatusBarColor(int32_t windowId, int32_t color)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, color, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (window->IsPcWindow()) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s device is not support, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED;
            return;
        }
        auto property = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        property.contentColor_ = color;
        property.settingFlag_ = static_cast<SystemBarSettingFlag>(
            static_cast<uint32_t>(property.settingFlag_) | static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
        SystemBarPropertyFlag propertyFlag = { false, false, true, false };
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(
            window->UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, property, propertyFlag));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowNavigationBarEnabled(int32_t windowId, bool enabled, bool enableAnimation)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, enabled, enableAnimation, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (window->IsPcWindow()) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s device is not support, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED;
            return;
        }
        auto property = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
        property.enable_ = enabled;
        property.settingFlag_ = static_cast<SystemBarSettingFlag>(
            static_cast<uint32_t>(property.settingFlag_) | static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING));
        property.enableAnimation_ = enableAnimation;
        SystemBarPropertyFlag propertyFlag = { true, false, false, true };
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(
            window->UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_NAVIGATION_BAR, property, propertyFlag));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_Snapshot(int32_t windowId, OH_PixelmapNative* pixelMap)
{
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "pixelMap is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, pixelMap, &errCode, where = __func__]() mutable {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        *pixelMap = OH_PixelmapNative(window->Snapshot());
    }, __func__);
    return pixelMap != nullptr ? WindowManager_ErrorCode::OK : errCode;
}

int32_t OH_WindowManager_SetWindowBackgroundColor(int32_t windowId, const char* color)
{
    if (color == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "color is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, color, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->SetBackgroundColor(std::string(color)));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowBrightness(int32_t windowId, float brightness)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, brightness, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->SetBrightness(brightness));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowKeepScreenOn(int32_t windowId, bool isKeepScreenOn)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, isKeepScreenOn, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->SetKeepScreenOn(isKeepScreenOn));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowPrivacyMode(int32_t windowId, bool isPrivacy)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, isPrivacy, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->SetPrivacyMode(isPrivacy));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_GetWindowProperties(
    int32_t windowId, WindowManager_WindowProperties* windowProperties)
{
    if (windowProperties == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowProperties is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::OK;
    eventHandler->PostSyncTask([windowId, windowProperties, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (OH_WINDOW_TO_WINDOW_TYPE_MAP.count(window->GetType()) != 0) {
            windowProperties->type = OH_WINDOW_TO_WINDOW_TYPE_MAP.at(window->GetType());
        } else {
            windowProperties->type = static_cast<WindowManager_WindowType>(window->GetType());
        }
        TransformedToWindowManagerRect(window->GetRect(), windowProperties->windowRect);
        windowProperties->isLayoutFullScreen = window->IsLayoutFullScreen();
        windowProperties->isFullScreen = window->IsFullScreen();
        windowProperties->touchable = window->GetTouchable();
        windowProperties->focusable = window->GetFocusable();
        windowProperties->isPrivacyMode = window->IsPrivacyMode();
        windowProperties->isKeepScreenOn = window->IsKeepScreenOn();
        windowProperties->brightness = window->GetBrightness();
        windowProperties->isTransparent = window->IsTransparent();
        windowProperties->id = window->GetWindowId();
        windowProperties->displayId = window->GetDisplayId();
        Rect drawableRect = { 0, 0, 0, 0 };
        auto uicontent = window->GetUIContent();
        if (uicontent == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s uicontent is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            return;
        }
        uicontent->GetAppPaintSize(drawableRect);
        TransformedToWindowManagerRect(drawableRect, windowProperties->drawableRect);
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_SetWindowTouchable(int32_t windowId, bool touchable)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "eventHandler null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, touchable, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->SetTouchable(touchable));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_GetAllWindowLayoutInfoList(
    int64_t displayId, WindowManager_Rect** windowLayoutInfoList, size_t* windowLayoutInfoSize)
{
    if (displayId < 0) {
        TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "displayId is invalid, displayId:%{public}" PRIu64, displayId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    if (windowLayoutInfoList == nullptr || windowLayoutInfoSize == nullptr) {
        TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "param is nullptr, displayId:%{public}" PRIu64, displayId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::OK;
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "eventHandler is null, displayId:%{public}" PRIu64, displayId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    eventHandler->PostSyncTask([displayId, windowLayoutInfoList, windowLayoutInfoSize, &errCode, where = __func__] {
        std::vector<OHOS::sptr<WindowLayoutInfo>> infos;
        auto ret =
            SingletonContainer::Get<WindowManager>().GetAllWindowLayoutInfo(static_cast<uint64_t>(displayId), infos);
        if (OH_WINDOW_TO_ERROR_CODE_MAP.find(ret) == OH_WINDOW_TO_ERROR_CODE_MAP.end()) {
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s get failed, errCode: %{public}d", where, errCode);
            return;
        } else if (OH_WINDOW_TO_ERROR_CODE_MAP.at(ret) != WindowManager_ErrorCode::OK) {
            errCode = (ret == WMError::WM_ERROR_DEVICE_NOT_SUPPORT) ? OH_WINDOW_TO_ERROR_CODE_MAP.at(ret) :
                WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s get failed, errCode: %{public}d", where, errCode);
            return;
        }
        WindowManager_Rect* infosInner = (WindowManager_Rect*)malloc(sizeof(WindowManager_Rect) * infos.size());
        if (infosInner == nullptr) {
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s infosInner is nullptr", where);
            return;
        }
        for (size_t i = 0; i < infos.size(); i++) {
            TransformedToWindowManagerRect(infos[i]->rect, infosInner[i]);
            TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s rect: %{public}d %{public}d %{public}d %{public}d",
                where, infosInner[i].posX, infosInner[i].posY, infosInner[i].width, infosInner[i].height);
        }
        *windowLayoutInfoList = infosInner;
        *windowLayoutInfoSize = infos.size();
    }, __func__);
    return errCode;
}

void OH_WindowManager_ReleaseAllWindowLayoutInfoList(WindowManager_Rect* windowLayoutInfoList)
{
    WINDOW_MANAGER_FREE_MEMORY(windowLayoutInfoList);
}

int32_t OH_WindowManager_SetWindowFocusable(int32_t windowId, bool isFocusable)
{
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "eventHandler is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, isFocusable, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->SetFocusable(isFocusable));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_InjectTouchEvent(
    int32_t windowId, Input_TouchEvent* touchEvent, int32_t windowX, int32_t windowY)
{
    if (touchEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "touchEvent is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    if (windowId <= 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowId is invalid, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    if (OH_Input_GetTouchEventWindowId(touchEvent) == -1) { // -1: invalid window id
        OH_Input_SetTouchEventWindowId(touchEvent, windowId);
        TLOGI(WmsLogTag::WMS_EVENT, "windowId is default");
    }
    if (OH_Input_GetTouchEventWindowId(touchEvent) != windowId) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowIds are not equal, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "eventHandler is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::OK;
    eventHandler->PostSyncTask([windowId, touchEvent, windowX, windowY, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        std::shared_ptr<OHOS::MMI::PointerEvent> pointerEvent =
            OH_Input_TouchEventToPointerEvent(touchEvent, windowX, windowY);
        if (pointerEvent == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s pointerEvent is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            return;
        }
        TLOGND(WmsLogTag::WMS_EVENT, "%{public}s, windowId:%{public}d", where, windowId);
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(window->InjectTouchEvent(pointerEvent));
    }, __func__);
    return errCode;
}

int32_t OH_WindowManager_GetAllMainWindowInfo(
    WindowManager_MainWindowInfo** infoList, size_t* mainWindowInfoSize)
{
    if (infoList == nullptr || mainWindowInfoSize == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "param is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::OK;
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "eventHandler is null");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    eventHandler->PostSyncTask([infoList, mainWindowInfoSize, &errCode, where = __func__] {
        std::vector<OHOS::sptr<MainWindowInfo>> infos;
        auto ret = SingletonContainer::Get<WindowManager>().GetAllMainWindowInfo(infos);
        if (OH_WINDOW_TO_ERROR_CODE_MAP.find(ret) == OH_WINDOW_TO_ERROR_CODE_MAP.end()) {
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s get failed, errCode: %{public}d", where, errCode);
            return;
        } else if (OH_WINDOW_TO_ERROR_CODE_MAP.at(ret) != WindowManager_ErrorCode::OK) {
            errCode = OH_WINDOW_TO_ERROR_CODE_MAP.at(ret);
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s get failed, errCode: %{public}d", where, errCode);
            return;
        }
        WindowManager_MainWindowInfo* infosInner = new WindowManager_MainWindowInfo[infos.size()];
        if (infosInner == nullptr) {
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s infosInner is nullptr", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s infos size: %{public}d", where, static_cast<int32_t>(infos.size()));
        for (size_t i = 0; i < infos.size(); i++) {
            TransformedToMainWindowInfo(infos[i], infosInner[i]);
        }
        *infoList = infosInner;
        *mainWindowInfoSize = infos.size();
        }, __func__);
    return errCode;
}
 
void OH_WindowManager_ReleaseAllMainWindowInfo(WindowManager_MainWindowInfo* infoList)
{
    WINDOW_MANAGER_FREE_MEMORY(infoList);
}
 
int32_t OH_WindowManager_GetMainWindowSnapshot(int32_t* windowIdList, size_t windowIdListSize,
    WindowManager_WindowSnapshotConfig config, OH_WindowManager_WindowSnapshotCallback callback)
{
    if (windowIdList == nullptr || windowIdListSize <= 0 || windowIdListSize > MAX_SESSION_LIMIT_ALL_APP) {
        TLOGNE(WmsLogTag::WMS_LIFE, "param is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::OK;
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "eventHandler is null");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    eventHandler->PostSyncTask([windowIdList, windowIdListSize, config, &errCode, callback, where = __func__] {
        WindowSnapshotConfiguration windowSnapshotConfiguration;
        TransformedToWindowSnapshotConfig(windowSnapshotConfiguration, config);
        std::vector<int32_t> windowIdVector(windowIdList, windowIdList + windowIdListSize);
        OHOS::sptr<GetSnapshotCallback> getSnapshotCallback = OHOS::sptr<GetSnapshotCallback>::MakeSptr();
        RegisterCallBackFunc(getSnapshotCallback, callback);
        auto ret = SingletonContainer::Get<WindowManager>().GetMainWindowSnapshot(
            windowIdVector, windowSnapshotConfiguration, getSnapshotCallback->AsObject());
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.find(ret) == OH_WINDOW_TO_ERROR_CODE_MAP.end() ?
            WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL : OH_WINDOW_TO_ERROR_CODE_MAP.at(ret);
        }, __func__);
    return errCode;
}
 
void OH_WindowManager_ReleaseMainWindowSnapshot(const OH_PixelmapNative* snapshotPixelMapList)
{
    WINDOW_MANAGER_FREE_MEMORY(snapshotPixelMapList);
}

int32_t OH_WindowManager_LockCursor(int32_t windowId, bool isCursorFollowMovement)
{
    if (windowId <= 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowId is invalid, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "eventHandler is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, isCursorFollowMovement, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (!window->IsFocused()) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is not focused, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        auto ret = window->LockCursor(windowId, isCursorFollowMovement);
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.find(ret) == OH_WINDOW_TO_ERROR_CODE_MAP.end() ?
            WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL : OH_WINDOW_TO_ERROR_CODE_MAP.at(ret);
        }, __func__);
    return errCode;
}

int32_t OH_WindowManager_UnlockCursor(int32_t windowId)
{
    if (windowId <= 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowId is invalid, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    auto eventHandler = GetMainEventHandler();
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "eventHandler is null, windowId:%{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    }
    WindowManager_ErrorCode errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL;
    eventHandler->PostSyncTask([windowId, &errCode, where = __func__] {
        auto window = Window::GetWindowWithId(windowId);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is null, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        if (!window->IsFocused()) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is not focused, windowId:%{public}d", where, windowId);
            errCode = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL;
            return;
        }
        auto ret = window->UnlockCursor(windowId);
        errCode = OH_WINDOW_TO_ERROR_CODE_MAP.find(ret) == OH_WINDOW_TO_ERROR_CODE_MAP.end() ?
            WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL : OH_WINDOW_TO_ERROR_CODE_MAP.at(ret);
        }, __func__);
    return errCode;
}