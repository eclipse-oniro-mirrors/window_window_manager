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

#ifndef OHOS_ANI_WINDOW_MANAGER_H
#define OHOS_ANI_WINDOW_MANAGER_H

#include "ani.h"
#include "ani_window_register_manager.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
#ifdef _WIN32
#define WINDOW_EXPORT __attribute__((dllexport))
#else
#define WINDOW_EXPORT __attribute__((visibility("default")))
#endif

class AniWindowManager {
public:
    explicit AniWindowManager();

    static ani_status AniWindowManagerInit(ani_env* env, ani_namespace windowNameSpace);
    static ani_ref GetLastWindow(ani_env* env, ani_long nativeObj, ani_object context);
    static ani_ref FindWindow(ani_env* env, ani_long nativeObj, ani_string windowName);
    static void MinimizeAll(ani_env* env, ani_long nativeObj, ani_long displayId);
    static void MinimizeAllWithExclusion(ani_env* env, ani_long nativeObj, ani_long displayId, ani_int excludeWindowId);
    static void ShiftAppWindowFocus(ani_env* env, ani_long nativeObj,
        ani_int sourceWindowId, ani_int targetWindowId);
    static ani_object GetAllMainWindowInfo(ani_env* env, ani_long nativeObj, ani_object context);
    static ani_object GetMainWindowSnapshot(
        ani_env* env, ani_long nativeObj, ani_object windowId, ani_object config);
    static ani_ref CreateWindow(ani_env* env, ani_long nativeObj, ani_object configuration);
    static void ShiftAppWindowTouchEvent(ani_env* env, ani_long nativeObj,
        ani_int sourceWindowId, ani_int targetWindowId, ani_int fingerId);
    static void ShiftAppWindowPointerEvent(ani_env* env, ani_long nativeObj,
        ani_int sourceWindowId, ani_int targetWindowId);
    static void SetWatermarkImageForAppWindows(ani_env* env, ani_long nativeObj, ani_object pixelMap);
    static ani_object GetAllWindowLayoutInfo(ani_env* env, ani_long nativeObj, ani_long displayId);
    static ani_object GetSnapshot(ani_env* env, ani_long nativeObj, ani_int windowId);
    static ani_object GetVisibleWindowInfo(ani_env* env, ani_long nativeObj);
    static void SetGestureNavigationEnabled(ani_env* env, ani_long nativeObj, ani_boolean enabled);
    static void SetWaterMarkImage(ani_env* env, ani_long nativeObj, ani_object nativePixelMap, ani_boolean enabled);
    static ani_object GetWindowsByCoordinate(ani_env* env, ani_long nativeObj, ani_object getWindowsParam);
    static ani_string GetTopNavDestinationName(ani_env* env, ani_long nativeObj, ani_int windowId);
    static ani_int GetGlobalWindowMode(ani_env* env, ani_long nativeObj, ani_object displayId);
    static void SetStartWindowBackgroundColor(ani_env* env, ani_long nativeObj, ani_string moduleName,
        ani_string abilityName, ani_long color);
    static void NotifyScreenshotEvent(ani_env* env, ani_long nativeObj, ani_enum_item eventType);
    static void RegisterWindowManagerCallback(ani_env* env, ani_long nativeObj, ani_string type, ani_ref callback);
    static void UnregisterWindowManagerCallback(ani_env* env, ani_long nativeObj, ani_string type, ani_ref callback);
    static void SetWindowLayoutMode(ani_env* env, ani_long nativeObj, ani_enum_item mode);
    static void ToggleShownStateForAllAppWindows(ani_env* env, ani_long nativeObj);
    static void SetSpecificSystemWindowZIndex(ani_env* env, ani_long nativeObj, ani_enum_item apiWindowType,
        ani_int zIndex);
private:
    ani_ref OnGetLastWindow(ani_env* env, ani_object context);
    ani_ref OnFindWindow(ani_env* env, ani_string windowName);
    void OnMinimizeAll(ani_env* env, ani_long displayId, ani_int excludeWindowId = 0);
    void OnShiftAppWindowFocus(ani_env* env, ani_int sourceWindowId, ani_int targetWindowId);
    ani_object GetTopWindowTask(ani_env* env, void* contextPtr, bool newApi);
    ani_object OnGetAllMainWindowInfo(ani_env* env, ani_object context);
    ani_object OnGetMainWindowSnapshot(
        ani_env* env, ani_object windowId, ani_object config);
    ani_ref OnCreateWindow(ani_env* env, ani_object configuration);
    void OnShiftAppWindowPointerEvent(ani_env* env, ani_int sourceWindowId, ani_int targetWindowId);
    void OnShiftAppWindowTouchEvent(ani_env* env, ani_int sourceWindowId, ani_int targetWindowId, ani_int fingerId);
    void OnSetWatermarkImageForAppWindows(ani_env* env, ani_object pixelMap);
    ani_string OnGetTopNavDestinationName(ani_env* env, ani_int windowId);
    ani_int OnGetGlobalWindowMode(ani_env* env, ani_object nativeDisplayId);
    void OnSetStartWindowBackgroundColor(ani_env* env, ani_string moduleName, ani_string abilityName,
        ani_long color);
    void OnNotifyScreenshotEvent(ani_env* env, ani_enum_item eventType);
    void OnRegisterWindowManagerCallback(ani_env* env, ani_string type, ani_ref callback);
    void OnUnregisterWindowManagerCallback(ani_env* env, ani_string type, ani_ref callback);
    void OnSetWindowLayoutMode(ani_env* env, ani_enum_item mode);
    void OnToggleShownStateForAllAppWindows(ani_env* env);
    ani_object OnGetAllWindowLayoutInfo(ani_env* env, ani_long displayId);
    ani_object OnGetSnapshot(ani_env* env, ani_int windowId);
    ani_object OnGetVisibleWindowInfo(ani_env* env);
    void OnSetGestureNavigationEnabled(ani_env* env, ani_boolean enabled);
    void OnSetWaterMarkImage(ani_env* env, ani_object nativePixelMap, ani_boolean enabled);
    ani_object OnGetWindowsByCoordinate(ani_env* env, ani_object getWindowsParam);
    void OnSetSpecificSystemWindowZIndex(ani_env* env, ani_enum_item windowType, ani_int zIndex);

    std::unique_ptr<AniWindowRegisterManager> registerManager_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ANI_WINDOW_MANAGER_H