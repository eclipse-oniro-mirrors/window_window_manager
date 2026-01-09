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

#include "ani_window_manager.h"

#include <ability.h>
#include <transaction/rs_interfaces.h>

#include "ability_context.h"
#include "ani.h"
#include "ani_window.h"
#include "ani_window_utils.h"
#include "common/include/session_permission.h"
#include "permission.h"
#include "pixel_map.h"
#include "pixel_map_taihe_ani.h"
#include "singleton_container.h"
#include "sys_cap_util.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "window_scene.h"
#include "window_helper.h"
#include "window_manager.h"
#include "window_option.h"
#include "permission.h"
#include "singleton_container.h"
#include "pixel_map.h"
#include "../../../../../../wm/include/get_snapshot_callback.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t MAIN_WINDOW_SNAPSGOT_TIMEOUT = 5000;
const std::string PIP_WINDOW = "pip_window";
constexpr int32_t INVALID_COORDINATE = -1;
constexpr uint32_t API_VERSION_18 = 18;
}

AniWindowManager::AniWindowManager() : registerManager_(std::make_unique<AniWindowRegisterManager>())
{
}

ani_status AniWindowManager::AniWindowManagerInit(ani_env* env, ani_namespace windowNameSpace)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_namespace ns;
    ani_status ret;
    if ((ret = env->FindNamespace("@ohos.window.window", &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find ns %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array functions = {
        ani_native_function {"onSync", nullptr,
            reinterpret_cast<void *>(AniWindowManager::RegisterWindowManagerCallback)},
        ani_native_function {"offSync", nullptr,
            reinterpret_cast<void *>(AniWindowManager::UnregisterWindowManagerCallback)},
        ani_native_function {"setWindowLayoutMode", "lC{@ohos.window.window.WindowLayoutMode}:",
            reinterpret_cast<void *>(AniWindowManager::SetWindowLayoutMode)},
        ani_native_function {"shiftAppWindowPointerEventSync", "lii:",
            reinterpret_cast<void *>(AniWindowManager::ShiftAppWindowPointerEvent)},
        ani_native_function {"shiftAppWindowTouchEventSync", "liii:",
            reinterpret_cast<void *>(AniWindowManager::ShiftAppWindowTouchEvent)},
        ani_native_function {"setWatermarkImageForAppWindowsSync",
            "lC{@ohos.multimedia.image.image.PixelMap}:",
            reinterpret_cast<void *>(AniWindowManager::SetWatermarkImageForAppWindows)},
        ani_native_function {"getTopNavDestinationNameSync", "li:C{std.core.String}",
            reinterpret_cast<void *>(AniWindowManager::GetTopNavDestinationName)},
        ani_native_function {"getGlobalWindowModeSync", "lC{std.core.Long}:i",
            reinterpret_cast<void *>(AniWindowManager::GetGlobalWindowMode)},
        ani_native_function {"setStartWindowBackgroundColorSync",
            "lC{std.core.String}C{std.core.String}l:",
            reinterpret_cast<void *>(AniWindowManager::SetStartWindowBackgroundColor)},
        ani_native_function {"notifyScreenshotEventSync", "lC{@ohos.window.window.ScreenshotEventType}:",
            reinterpret_cast<void *>(AniWindowManager::NotifyScreenshotEvent)},
        ani_native_function {"getAllWindowLayoutInfo", "ll:C{std.core.Array}",
            reinterpret_cast<void *>(AniWindowManager::GetAllWindowLayoutInfo)},
        ani_native_function {"getSnapshot", "li:C{@ohos.multimedia.image.image.PixelMap}",
            reinterpret_cast<void *>(AniWindowManager::GetSnapshot)},
        ani_native_function {"getVisibleWindowInfo", "l:C{std.core.Array}",
            reinterpret_cast<void *>(AniWindowManager::GetVisibleWindowInfo)},
        ani_native_function {"setGestureNavigationEnabled", "lz:",
            reinterpret_cast<void *>(AniWindowManager::SetGestureNavigationEnabled)},
        ani_native_function {"setWaterMarkImage", "lC{@ohos.multimedia.image.image.PixelMap}z:",
            reinterpret_cast<void *>(AniWindowManager::SetWaterMarkImage)},
        ani_native_function {"getWindowsByCoordinate",
            "lC{@ohos.window.window.GetWindowsByCoordinateParam}:C{std.core.Array}",
            reinterpret_cast<void *>(AniWindowManager::GetWindowsByCoordinate)},
        ani_native_function {"toggleShownStateForAllAppWindowsSync", "l:",
            reinterpret_cast<void *>(AniWindowManager::ToggleShownStateForAllAppWindows)},
        ani_native_function {"setSpecificSystemWindowZIndexSync", "lC{@ohos.window.window.WindowType}i:",
            reinterpret_cast<void *>(AniWindowManager::SetSpecificSystemWindowZIndex)},
    };
    for (auto method : functions) {
        if ((ret = env->Namespace_BindNativeFunctions(ns, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window static method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }

    ani_function setObjFunc = nullptr;
    ret = env->Namespace_FindFunction(windowNameSpace, "setNativeObj", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    std::unique_ptr<AniWindowManager> aniWinManager = std::make_unique<AniWindowManager>();
    ret = env->Function_Call_Void(setObjFunc, aniWinManager.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    return ret;
}

ani_ref AniWindowManager::GetLastWindow(ani_env* env, ani_long nativeObj, ani_object context)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetLastWindow(env, context) : nullptr;
}

ani_ref AniWindowManager::OnGetLastWindow(ani_env* env, ani_object aniContext)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto contextPtr = AniWindowUtils::GetAbilityContext(env, aniContext);
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] context is nullptr");
        return AniWindowUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR,
            "[window][getLastWindow]msg: Stage mode without context");
    }
    auto window = Window::GetTopWindowWithContext(context->lock());
    if (window == nullptr || window->GetWindowState() == WindowState::STATE_DESTROYED) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr or destroyed");
        return AniWindowUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR,
            "[window][getLastWindow]msg: Get top window failed");
    }
    return CreateAniWindowObject(env, window);
}

void AniWindowManager::ShiftAppWindowFocus(ani_env* env, ani_long nativeObj,
    ani_int sourceWindowId, ani_int targetWindowId)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnShiftAppWindowFocus(env, sourceWindowId, targetWindowId);
    } else {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnShiftAppWindowFocus(ani_env* env, ani_int sourceWindowId, ani_int targetWindowId)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI] sourceWindowId: %{public}d targetWindowId: %{public}d",
        static_cast<int32_t>(sourceWindowId), static_cast<int32_t>(targetWindowId));
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().ShiftAppWindowFocus(sourceWindowId, targetWindowId));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][shiftAppWindowFocus]msg:ShiftAppWindowFocus failed");
    }
    return ;
}

ani_object AniWindowManager::GetAllMainWindowInfo(ani_env* env, ani_long nativeObj, ani_object context)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetAllMainWindowInfo(env, context) : nullptr;
}

ani_object AniWindowManager::OnGetAllMainWindowInfo(ani_env* env, ani_object context)
{
    std::vector<sptr<MainWindowInfo>> infos;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().GetAllMainWindowInfo(infos));
    if (ret != WmErrorCode::WM_OK) {
        TLOGI(WmsLogTag::WMS_LIFE, "Get All MainWindowInfo failed, ret: %{public}d", static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, ret, "Window Get All Main Window failed");
    }
    return AniWindowUtils::CreateAniMainWindowInfoArray(env, infos);
}

ani_object AniWindowManager::GetMainWindowSnapshot(
    ani_env* env, ani_long nativeObj, ani_object windowId, ani_object config)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ?
        aniWindowManager->OnGetMainWindowSnapshot(env, windowId, config) : nullptr;
}

ani_object AniWindowManager::OnGetMainWindowSnapshot(
    ani_env* env, ani_object windowId, ani_object config)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    sptr<GetSnapshotCallback> getSnapshotCallback = sptr<GetSnapshotCallback>::MakeSptr();
    auto pixelMaps = std::make_shared<std::vector<std::shared_ptr<Media::PixelMap>>>();
    std::shared_ptr<WMError> errCode = std::make_shared<WMError>(WMError::WM_OK);
    getSnapshotCallback->RegisterFunc([env, errCode, pixelMaps, getSnapshotCallback]
        (WMError errCodeResult, const std::vector<std::shared_ptr<Media::PixelMap>>& pixelMapResult) {
            TLOGI(WmsLogTag::WMS_LIFE, "getSnapshotCallback errCodeResult: %{public}d",
                static_cast<int32_t>(errCodeResult));
            if (errCodeResult != WMError::WM_OK) {
                *errCode = errCodeResult;
            }
            *pixelMaps = pixelMapResult;
            getSnapshotCallback->OnNotifyResult();
        });
    std::vector<int32_t> windowIdList;
    WindowSnapshotConfiguration windowSnapshotConfiguration;
    AniWindowUtils::GetIntVector(env, windowId, windowIdList);
    AniWindowUtils::GetWindowSnapshotConfiguration(env, config, windowSnapshotConfiguration);
    TLOGI(WmsLogTag::WMS_LIFE, "windowIdList size: %{public}d", static_cast<int32_t>(windowIdList.size()));
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().
        GetMainWindowSnapshot(windowIdList, windowSnapshotConfiguration, getSnapshotCallback->AsObject()));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "Get snapshot not ok!");
        return AniWindowUtils::AniThrowError(env, ret);
    } else {
        getSnapshotCallback->GetSyncResult(static_cast<int32_t>(MAIN_WINDOW_SNAPSGOT_TIMEOUT));
        if (*errCode == WMError::WM_OK) {
            return AniWindowUtils::CreateAniPixelMapArray(env, *pixelMaps);
        }
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
}

ani_ref AniWindowManager::CreateWindow(ani_env* env, ani_long nativeObj, ani_object configuration)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnCreateWindow(env, configuration) : nullptr;
}

void AniWindowManager::SetWatermarkImageForAppWindows(ani_env* env, ani_long nativeObj, ani_object pixelMap)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnSetWatermarkImageForAppWindows(env, pixelMap);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnSetWatermarkImageForAppWindows(ani_env* env, ani_object pixelMap)
{
    std::shared_ptr<Media::PixelMap> localPixelMap = nullptr;
    int32_t imgWidth = 0;
    int32_t imgHeight = 0;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(pixelMap, &isUndefined);
    if (!isUndefined) {
        localPixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMap);
        if (localPixelMap == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "parse image failed");
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return;
        }
        imgWidth = localPixelMap->GetWidth();
        imgHeight = localPixelMap->GetHeight();
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "isSetWatermark=%{public}d, imgWidth=%{public}d, imgHeight=%{public}d",
        localPixelMap != nullptr, imgWidth, imgHeight);
    auto retCode = SingletonContainer::Get<WindowManager>().SetWatermarkImageForApp(localPixelMap);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] retCode: %{public}d", static_cast<int32_t>(retCode));
        AniWindowUtils::AniThrowError(env, ret, "setWatermarkImageForAppWindowsSync failed.");
        return;
    }
}

ani_string AniWindowManager::GetTopNavDestinationName(ani_env* env, ani_long nativeObj, ani_int windowId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetTopNavDestinationName(env, windowId) : nullptr;
}

ani_string AniWindowManager::OnGetTopNavDestinationName(ani_env* env, ani_int windowId)
{
    ani_string result = nullptr;
    if (static_cast<int32_t>(windowId) < 1) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid windowId: %{public}d", static_cast<int32_t>(windowId));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
        return result;
    }
    std::string topNavDestName;
    auto retCode = SingletonContainer::Get<WindowManager>().GetTopNavDestinationName(windowId, topNavDestName);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] winId: %{public}d, topNavDestName: %{public}s, retCode: %{public}d",
            static_cast<int32_t>(windowId), topNavDestName.c_str(), static_cast<int32_t>(retCode));
        AniWindowUtils::AniThrowError(env, ret, "getTopNavDestinationNameSync failed.");
        return result;
    }
    AniWindowUtils::GetAniString(env, topNavDestName, &result);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] winId: %{public}d, topNavDestName: %{public}s",
        static_cast<int32_t>(windowId), topNavDestName.c_str());
    return result;
}

ani_int AniWindowManager::GetGlobalWindowMode(ani_env* env, ani_long nativeObj, ani_object displayId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    ani_int result = 0;
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetGlobalWindowMode(env, displayId) : result;
}

ani_int AniWindowManager::OnGetGlobalWindowMode(ani_env* env, ani_object nativeDisplayId)
{
    ani_int result = 0;
    DisplayId displayId = DISPLAY_ID_INVALID;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(nativeDisplayId, &isUndefined);
    if (!isUndefined) {
        ani_long aniDisplayId;
        env->Object_CallMethodByName_Long(nativeDisplayId, "toLong", ":l", &aniDisplayId);
        if (aniDisplayId < 0) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid displayId value: %{public}" PRId64,
                static_cast<int64_t>(aniDisplayId));
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return result;
        }
        displayId = static_cast<DisplayId>(aniDisplayId);
    }
    GlobalWindowMode globalWinMode = GlobalWindowMode::UNKNOWN;
    auto retCode = SingletonContainer::Get<WindowManager>().GetGlobalWindowMode(displayId, globalWinMode);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "globalWinMode: %{public}u, retCode: %{public}d, displayId: %{public}" PRIu64,
            static_cast<uint32_t>(globalWinMode), static_cast<int32_t>(retCode), displayId);
        AniWindowUtils::AniThrowError(env, ret, "getTopNavDestinationNameSync failed.");
        return result;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "globalWinMode: %{public}u, displayId: %{public}" PRIu64,
        static_cast<uint32_t>(globalWinMode), displayId);
    result = static_cast<ani_int>(globalWinMode);
    return result;
}

void AniWindowManager::SetStartWindowBackgroundColor(ani_env* env, ani_long nativeObj, ani_string moduleName,
    ani_string abilityName, ani_long color)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnSetStartWindowBackgroundColor(env, moduleName, abilityName, color);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnSetStartWindowBackgroundColor(ani_env* env, ani_string moduleName, ani_string abilityName,
    ani_long color)
{
    constexpr uint32_t maxNameLength = 200;
    std::string moduleNameStr;
    AniWindowUtils::GetStdString(env, moduleName, moduleNameStr);
    if (moduleNameStr.length() > maxNameLength) {
        TLOGE(WmsLogTag::WMS_PATTERN, "[ANI] moduleName length out of range");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
        return;
    }
    std::string abilityNameStr;
    AniWindowUtils::GetStdString(env, abilityName, abilityNameStr);
    if (abilityNameStr.length() > maxNameLength) {
        TLOGE(WmsLogTag::WMS_PATTERN, "abilityName length out of range");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
        return;
    }
    uint32_t colorValue = static_cast<uint32_t>(color);
    auto retCode = SingletonContainer::Get<WindowManager>().SetStartWindowBackgroundColor(
        moduleNameStr, abilityNameStr, colorValue);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] module=%{public}s, ability=%{public}s, color=%{public}u, ret=%{public}d",
            moduleNameStr.c_str(), abilityNameStr.c_str(), colorValue, static_cast<int32_t>(retCode));
        AniWindowUtils::AniThrowError(env, ret, "setStartWindowBackgroundColorSync failed.");
        return;
    }
}

void AniWindowManager::NotifyScreenshotEvent(ani_env* env, ani_long nativeObj, ani_enum_item eventType)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnNotifyScreenshotEvent(env, eventType);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnNotifyScreenshotEvent(ani_env* env, ani_enum_item eventType)
{
    uint32_t tempEventType;
    ani_status ani_ret = AniWindowUtils::GetEnumValue(env, eventType, tempEventType);
    if (ani_ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] GetEnumValue failed, ret: %{public}d", ani_ret);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ScreenshotEventType screenshotEventType = static_cast<ScreenshotEventType>(tempEventType);
    auto retCode = SingletonContainer::Get<WindowManager>().NotifyScreenshotEvent(screenshotEventType);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] eventType: %{public}u, retCode: %{public}d",
            tempEventType, static_cast<int32_t>(retCode));
        AniWindowUtils::AniThrowError(env, ret, "notifyScreenshotEventSync failed.");
    }
}

ani_ref CreateAniSystemWindow(ani_env* env, void* contextPtr, sptr<WindowOption> windowOption)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    if (windowOption == nullptr) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,
            "[ANI] Context is nullptr");
    }
    if (windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT ||
        windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context->lock());
        if (abilityContext != nullptr) {
            if (!Permission::CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
                return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NO_PERMISSION,
                    "[ANI] TYPE_FLOAT CheckCallingPermission failed");
            }
        }
    }
    WMError wmError = WMError::WM_OK;
    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption, context->lock(), wmError);
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    if (window != nullptr && wmErrorCode == WmErrorCode::WM_OK) {
        return CreateAniWindowObject(env, window);
    } else {
        return AniWindowUtils::AniThrowError(env, wmErrorCode, "Create window failed");
    }
}

ani_ref CreateAniSubWindow(ani_env* env, sptr<WindowOption> windowOption)
{
    if (windowOption == nullptr) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    if (windowOption->GetParentId() == INVALID_WINDOW_ID) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[ANI] Parent window missed");
    }

    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption);
    if (window == nullptr) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    } else {
        return CreateAniWindowObject(env, window);
    }
}

bool ParseRequiredConfigOption(ani_env* env, ani_object configuration, WindowOption &option)
{
    ani_ref result;
    env->Object_GetPropertyByName_Ref(configuration, "name", &result);
    ani_string aniWindowName = reinterpret_cast<ani_string>(result);
    std::string windowName;
    AniWindowUtils::GetStdString(env, aniWindowName, windowName);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] WindowName: %{public}s", windowName.c_str());
    option.SetWindowName(windowName);

    ani_int ret;
    env->Object_GetPropertyByName_Ref(configuration, "windowType", &result);
    auto status = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(result), &ret);
    if (status != ANI_OK) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI] Fail to throw err, status: %{public}d", static_cast<int32_t>(status));
        return false;
    }
    uint32_t winType = static_cast<uint32_t>(ret);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] winType: %{public}u", winType);
    if (winType >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
        winType < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
        option.SetWindowType(JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(winType)));
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Invalid winType");
        return false;
    }
    return true;
}

bool ParseOptionalConfigOption(ani_env* env, ani_object configuration, WindowOption &option)
{
    bool dialogDecorEnable;
    ani_status status = AniWindowUtils::GetPropertyBoolObject(env, "decorEnabled", configuration, dialogDecorEnable);
    if (ANI_OK != status) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get property decorEnabled %{public}d",
            static_cast<int32_t>(status));
        return false;
    }
    option.SetDialogDecorEnable(dialogDecorEnable);

    int32_t ret = 0;
    status = AniWindowUtils::GetPropertyIntObject(env, "displayId", configuration, ret);
    if (ANI_OK != status) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get property displayId %{public}d", static_cast<int32_t>(status));
        return false;
    }
    int64_t displayId = static_cast<int64_t>(ret);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] displayId: %{public}d", static_cast<int32_t>(displayId));
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] displayId is invalid");
        return false;
    }
    option.SetDisplayId(displayId);

    status = AniWindowUtils::GetPropertyIntObject(env, "parentId", configuration, ret);
    if (ANI_OK != status) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get property parentId %{public}d", static_cast<int32_t>(status));
        return false;
    }
    int64_t parentId = static_cast<int64_t>(ret);
    option.SetParentId(parentId);
    
    return true;
}

bool ParseConfigOption(ani_env* env, ani_object configuration, WindowOption &option, void*& contextPtr)
{
    if (!ParseRequiredConfigOption(env, configuration, option)) {
        return false;
    }

    ani_ref result;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(configuration, "title", &result)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get property title");
        return false;
    }
    ani_boolean isTitleUndefined = false;
    if (ANI_OK != env->Reference_IsUndefined(result, &isTitleUndefined)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to judge isTitleUndefined");
        return false;
    }
    if (!isTitleUndefined) {
        ani_string aniDialogTitle = reinterpret_cast<ani_string>(result);
        std::string dialogTitle;
        AniWindowUtils::GetStdString(env, aniDialogTitle, dialogTitle);
        TLOGI(WmsLogTag::DEFAULT, "[ANI] dialogTitle: %{public}s", dialogTitle.c_str());
        option.SetDialogTitle(dialogTitle);
    }

    if (ANI_OK != env->Object_GetPropertyByName_Ref(configuration, "ctx", &result)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get property ctx");
        return false;
    }
    ani_boolean isCtxUndefined = false;
    if (ANI_OK != env->Reference_IsUndefined(result, &isCtxUndefined)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to judge isCtxUndefined");
        return false;
    }
    if (!isCtxUndefined) {
        ani_object aniContextPtr = reinterpret_cast<ani_object>(result);
        contextPtr = AniWindowUtils::GetAbilityContext(env, aniContextPtr);
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
        if (context == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] context is nullptr");
            return AniWindowUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR, "Stage mode without context");
        }
    }

    return ParseOptionalConfigOption(env, configuration, option);
}

ani_ref AniWindowManager::OnCreateWindow(ani_env* env, ani_object configuration)
{
    WindowOption option;
    void* contextPtr = nullptr;
    if (!ParseConfigOption(env, configuration, option, contextPtr)) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "[ANI] Failed to parse config");
    }
    sptr<WindowOption> windowOption = new WindowOption(option);
    if (WindowHelper::IsSystemWindow(option.GetWindowType())) {
        return CreateAniSystemWindow(env, contextPtr, windowOption);
    } else if (WindowHelper::IsSubWindow(option.GetWindowType())) {
        return CreateAniSubWindow(env, windowOption);
    } else {
        return AniWindowUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR, "[ANI] Create window failed");
    }
}

ani_ref AniWindowManager::FindWindow(ani_env* env, ani_long nativeObj, ani_string windowName)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnFindWindow(env, windowName) : nullptr;
}

ani_ref AniWindowManager::OnFindWindow(ani_env* env, ani_string windowName)
{
    std::string name;
    AniWindowUtils::GetStdString(env, windowName, name);
    TLOGI(WmsLogTag::DEFAULT, "[ANI]Window name=%{public}s", name.c_str());
    if (name.compare(PIP_WINDOW) == 0) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    ani_ref aniWindowObj = FindAniWindowObject(name);
    if (aniWindowObj != nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI]Find window: %{public}s, use exist js window", name.c_str());
        return aniWindowObj;
    } else {
        sptr<Window> window = Window::Find(name);
        if (window == nullptr) {
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        } else {
            return CreateAniWindowObject(env, window);
        }
    }
}

void AniWindowManager::MinimizeAll(ani_env* env, ani_long nativeObj, ani_long displayId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnMinimizeAll(env, displayId);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindowManager is nullptr");
    }
}


void AniWindowManager::MinimizeAllWithExclusion(ani_env* env, ani_long nativeObj,
    ani_long displayId, ani_int excludeWindowId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    if (static_cast<int32_t>(excludeWindowId) <= 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] Minimize all failed, Invalidate params excludeWindowId : %{public}d.",
            static_cast<int32_t>(excludeWindowId));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnMinimizeAll(env, displayId, excludeWindowId);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnMinimizeAll(ani_env* env, ani_long displayId, ani_int excludeWindowId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    if (static_cast<int64_t>(displayId) < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] Minimize all failed, Invalidate params.");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().MinimizeAllAppWindows(
            static_cast<uint64_t>(displayId), static_cast<int32_t>(excludeWindowId)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] Minimize all failed, ret:%{public}d", static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret, "OnMinimizeAll failed");
    }
}

void AniWindowManager::RegisterWindowManagerCallback(ani_env* env, ani_long nativeObj,
    ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnRegisterWindowManagerCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnRegisterWindowManagerCallback(ani_env* env, ani_string type, ani_ref callback)
{
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}s", cbType.c_str());
    WmErrorCode ret = registerManager_->RegisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER,
        env, callback, ani_double(0));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindowManager::UnregisterWindowManagerCallback(ani_env* env, ani_long nativeObj,
    ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnUnregisterWindowManagerCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnUnregisterWindowManagerCallback(ani_env* env, ani_string type, ani_ref callback)
{
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}s", cbType.c_str());
    WmErrorCode ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER,
        env, callback);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindowManager::OnSetWindowLayoutMode(ani_env* env, ani_enum_item mode)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }

    uint32_t modeType;
    ani_status ani_ret = AniWindowUtils::GetEnumValue(env, mode, modeType);
    if (ani_ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] GetEnumValue failed, ret: %{public}d", ani_ret);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }

    WindowLayoutMode winLayoutMode = static_cast<WindowLayoutMode>(modeType);
    if (winLayoutMode != WindowLayoutMode::CASCADE && winLayoutMode != WindowLayoutMode::TILE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] Invalid winLayoutMode: %{public}u", winLayoutMode);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }

    const WMError ret = SingletonContainer::Get<WindowManager>().SetWindowLayoutMode(winLayoutMode);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] Failed, modeType: %{public}u, ret: %{public}d",
            modeType, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] Success, modeType: %{public}u", modeType);
}

void AniWindowManager::SetWindowLayoutMode(ani_env* env, ani_long nativeObj, ani_enum_item mode)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnSetWindowLayoutMode(env, mode);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] aniWindowManager is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindowManager::ShiftAppWindowPointerEvent(ani_env* env, ani_long nativeObj,
    ani_int sourceWindowId, ani_int targetWindowId)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnShiftAppWindowPointerEvent(env, sourceWindowId, targetWindowId);
    } else {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]aniWindowManager is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindowManager::OnShiftAppWindowPointerEvent(ani_env* env, ani_int sourceWindowId, ani_int targetWindowId)
{
    TLOGI(WmsLogTag::WMS_PC, "[ANI]sourceWindowId: %{public}d, targetWindowId: %{public}d",
        sourceWindowId, targetWindowId);
    if (sourceWindowId == static_cast<int32_t>(INVALID_WINDOW_ID) ||
        targetWindowId == static_cast<int32_t>(INVALID_WINDOW_ID)) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]invalid sourceWindowId or targetWindowId");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(
        SingletonContainer::Get<WindowManager>().ShiftAppWindowPointerEvent(sourceWindowId, targetWindowId));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]ShiftAppWindowPointerEvent failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "ShiftAppWindowPointerEvent failed!");
    }
    return;
}

void AniWindowManager::ShiftAppWindowTouchEvent(ani_env* env, ani_long nativeObj, ani_int sourceWindowId,
    ani_int targetWindowId, ani_int fingerId)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnShiftAppWindowTouchEvent(env, sourceWindowId, targetWindowId, fingerId);
    } else {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]aniWindowManager is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindowManager::OnShiftAppWindowTouchEvent(ani_env* env, ani_int sourceWindowId, ani_int targetWindowId,
    ani_int fingerId)
{
    TLOGI(WmsLogTag::WMS_PC, "[ANI]sourceWindowId: %{public}d, targetWindowId: %{public}d, fingerId: %{public}d",
        sourceWindowId, targetWindowId, fingerId);
    if (sourceWindowId <= static_cast<int32_t>(INVALID_WINDOW_ID) ||
        targetWindowId <= static_cast<int32_t>(INVALID_WINDOW_ID) ||
        (fingerId <= static_cast<int32_t>(INVALID_FINGER_ID))) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]invalid sourceWindowId or targetWindowId or fingerId");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM, "shiftAppWindowTouchEvent failed");
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(
        SingletonContainer::Get<WindowManager>().ShiftAppWindowPointerEvent(sourceWindowId, targetWindowId, fingerId));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]ShiftAppWindowPointerEvent failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "shiftAppWindowTouchEvent failed!");
    }
    return;
}

ani_object AniWindowManager::GetAllWindowLayoutInfo(ani_env* env, ani_long nativeObj, ani_long displayId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetAllWindowLayoutInfo(env, displayId) : nullptr;
}

ani_object AniWindowManager::OnGetAllWindowLayoutInfo(ani_env* env, ani_long displayId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    if (static_cast<int64_t>(displayId) < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] GetAllWindowLayoutInfo failed, Invalidate params.");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::vector<sptr<WindowLayoutInfo>> infos;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().GetAllWindowLayoutInfo(static_cast<uint64_t>(displayId), infos));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] GetAllWindowLayoutInfo failed, ret:%{public}d", ret);
        return AniWindowUtils::AniThrowError(env, ret, "failed");
    }
    return AniWindowUtils::CreateAniWindowLayoutInfoArray(env, infos);
}

ani_object AniWindowManager::GetSnapshot(ani_env* env, ani_long nativeObj, ani_int windowId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetSnapshot(env, windowId) : nullptr;
}

ani_object AniWindowManager::OnGetSnapshot(ani_env* env, ani_int windowId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WmErrorCode result = WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().GetSnapshotByWindowId(
        static_cast<int>(windowId), pixelMap));
    if (result != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Get snapshot not ok!");
        return AniWindowUtils::AniThrowError(env, result);
    }
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Get snapshot is nullptr!");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto nativePixelMap = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap);
    if (nativePixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Create native pixelmap is nullptr!");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return nativePixelMap;
}

ani_object AniWindowManager::GetVisibleWindowInfo(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetVisibleWindowInfo(env) : nullptr;
}

ani_object AniWindowManager::OnGetVisibleWindowInfo(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    uint32_t apiVersion = SysCapUtil::GetApiCompatibleVersion();
    if (apiVersion < API_VERSION_18 && !Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied! api%{public}u", apiVersion);
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    } else if (apiVersion >= API_VERSION_18 &&
               !CheckCallingPermission(PermissionConstants::PERMISSION_VISIBLE_WINDOW_INFO)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied! api%{public}u", apiVersion);
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NO_PERMISSION);
    }
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().GetVisibilityWindowInfo(infos));
    if (ret == WmErrorCode::WM_OK) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "OnGetVisibleWindowInfo success");
        for (auto it = infos.begin(); it != infos.end();) {
            auto windowType = (*it)->GetWindowType();
            if (windowType >= WindowType::APP_MAIN_WINDOW_BASE && windowType < WindowType::APP_MAIN_WINDOW_END) {
                it++;
            } else {
                it = infos.erase(it);
            }
        }
        return AniWindowUtils::CreateAniWindowInfoArray(env, infos);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "OnGetVisibleWindowInfo failed");
        return AniWindowUtils::AniThrowError(env, ret, "OnGetVisibleWindowInfo failed");
    }
}

void AniWindowManager::SetGestureNavigationEnabled(ani_env* env, ani_long nativeObj, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnSetGestureNavigationEnabled(env, enabled);
    } else {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindowManager is nullptr");
        return;
    }
}

void AniWindowManager::OnSetGestureNavigationEnabled(ani_env* env, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_IMMS, "Set gesture navigation enable as %{public}d", enabled);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().SetGestureNavigationEnabled(enabled));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "SetGestureNavigationEnabled failed");
        return;
    }
}

void AniWindowManager::SetWaterMarkImage(ani_env* env, ani_long nativeObj,
    ani_object nativePixelMap, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnSetWaterMarkImage(env, nativePixelMap, enable);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindowManager is nullptr");
        return;
    }
}

void AniWindowManager::OnSetWaterMarkImage(ani_env* env, ani_object nativePixelMap, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    std::shared_ptr<Media::PixelMap> pixelMap;
    pixelMap = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, nativePixelMap);
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to PixelMap");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set watermark image permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    RSInterfaces::GetInstance().ShowWatermark(pixelMap, enable);
}

ani_object AniWindowManager::GetWindowsByCoordinate(ani_env* env, ani_long nativeObj, ani_object getWindowsParam)
{
    TLOGI(WmsLogTag::WMS_PC, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    return aniWindowManager != nullptr ? aniWindowManager->OnGetWindowsByCoordinate(env, getWindowsParam) : nullptr;
}

ani_object AniWindowManager::OnGetWindowsByCoordinate(ani_env* env, ani_object getWindowsParam)
{
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    ani_long aniDisplayId;
    if (ANI_OK != env->Object_GetPropertyByName_Long(getWindowsParam, "displayId", &aniDisplayId)) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI] Failed to convert parameter to displayId");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    displayId = static_cast<int64_t>(aniDisplayId);
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId) == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI] invalid displayId");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t windowNumber = 0;
    ani_int aniWindowNumber;
    if (ANI_OK == env->Object_GetPropertyByName_Int(getWindowsParam, "windowNumber", &aniWindowNumber)) {
        windowNumber = static_cast<int32_t>(aniWindowNumber);
    }
    int32_t x = INVALID_COORDINATE;
    ani_int aniX;
    if (ANI_OK == env->Object_GetPropertyByName_Int(getWindowsParam, "x", &aniX)) {
        x = static_cast<int32_t>(aniX);
    }
    int32_t y = INVALID_COORDINATE;
    ani_int aniY;
    if (ANI_OK == env->Object_GetPropertyByName_Int(getWindowsParam, "y", &aniY)) {
        y = static_cast<int32_t>(aniY);
    }
    std::vector<int32_t> windowIds;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().
        GetWindowIdsByCoordinate(static_cast<uint64_t>(displayId), windowNumber, x, y, windowIds));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI] getWindowsByCoordinate failed");
        return AniWindowUtils::AniThrowError(env, ret);
    }
    std::vector<ani_ref> windows(windowIds.size());
    for (size_t i = 0; i < windowIds.size(); i++) {
        sptr<Window> window = Window::GetWindowWithId(windowIds[i]);
        windows[i] = CreateAniWindowObject(env, window);
    }
    return AniWindowUtils::CreateAniWindowArray(env, windows);
}

void AniWindowManager::OnToggleShownStateForAllAppWindows(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().ToggleShownStateForAllAppWindows());
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "ToggleShownStateForAllAppWindows failed.");
    }
}

void AniWindowManager::ToggleShownStateForAllAppWindows(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        return aniWindowManager->OnToggleShownStateForAllAppWindows(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::SetSpecificSystemWindowZIndex(ani_env* env, ani_long nativeObj,
    ani_enum_item apiWindowType, ani_int zIndex)
{
    AniWindowManager* aniWindowManager = reinterpret_cast<AniWindowManager*>(nativeObj);
    if (aniWindowManager != nullptr) {
        aniWindowManager->OnSetSpecificSystemWindowZIndex(env, apiWindowType, zIndex);
    } else {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] aniWindowManager is nullptr");
    }
}

void AniWindowManager::OnSetSpecificSystemWindowZIndex(ani_env* env, ani_enum_item apiWindowType, ani_int zIndex)
{
    ani_int enumValue;
    env->EnumItem_GetValue_Int(apiWindowType, &enumValue);
    uint32_t apiWindowTypeValue = static_cast<uint32_t>(enumValue);
    WindowType windowType;
    if (apiWindowTypeValue >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
        apiWindowTypeValue < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
        windowType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(apiWindowTypeValue));
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] invalid windowType");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSpecificSystemWindowZIndex]msg:failed to convert paramter to windowType");
        return;
    }
    if (!WindowHelper::IsSupportSetZIndexWindow(windowType)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] windowType not support %{public}d", windowType);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setSpecificSystemWindowZIndex]msg:windowType not support");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().
        SetSpecificSystemWindowZIndex(windowType, zIndex));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][setSpecificSystemWindowZIndex]msg:set failed");
    }
}
}  // namespace Rosen
}  // namespace OHOS