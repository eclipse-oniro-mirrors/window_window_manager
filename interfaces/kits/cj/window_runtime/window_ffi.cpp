/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "window_ffi.h"
#include <transaction/rs_interfaces.h>
#include "pixel_map_impl.h"
#include "window_manager_hilog.h"

using namespace OHOS::FFI;
using namespace OHOS::Media;

namespace OHOS {
namespace Rosen {
extern "C" {
const int32_t WM_ERROR_STATE_ABNORMALLY = 1300002;

RetDataI64 FfiOHOSCreateWindow(char* name, uint32_t windowType, OHOS::AbilityRuntime::Context* context,
    int64_t displayId, int64_t parentId)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] CreateWindow start");
    std::string nm = name;
    RetDataI64 ret = {};
    WindowParameters window;
    window.name = nm;
    window.winType = windowType;
    window.context = context;
    window.displayId = displayId;
    window.parentId = parentId;
    window.windowId = &ret.data;
    ret.code = OHOS::Rosen::WindowManagerImpl::CreateWindow(window);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] CreateWindow success");
    return ret;
}

int32_t FfiOHOSWindowSetWindowLayoutMode(uint32_t mode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowLayoutMode start");
    int32_t ret = OHOS::Rosen::WindowManagerImpl::SetWindowLayoutMode(mode);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowLayoutMode success");
    return ret;
}

int32_t FfiOHOSWindowMinimizeAll(int64_t displayId)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] MinimizeAll start");
    int32_t ret = OHOS::Rosen::WindowManagerImpl::MinimizeAll(displayId);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] MinimizeAll success");
    return ret;
}

RetDataI64 FfiOHOSGetLastWindow(OHOS::AbilityRuntime::Context* ctx)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] GetLastWindow start");
    RetDataI64 ret = {};
    ret.code = OHOS::Rosen::WindowManagerImpl::GetLastWindow(ctx, ret.data);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] GetLastWindow success");
    return ret;
}

int32_t FfiOHOSShiftAppWindowFocus(int32_t sourceWindowId, int32_t targetWindowId)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "ShiftAppWindowFocus start");
    int32_t ret = OHOS::Rosen::WindowManagerImpl::ShiftAppWindowFocus(sourceWindowId, targetWindowId);
    TLOGI(WmsLogTag::WMS_FOCUS, "ShiftAppWindowFocus success");
    return ret;
}

// window
int32_t FfiOHOSWindowHide(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Hide start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Hide();
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Hide success");
    return res;
}

int32_t FfiOHOSWindowHideWithAnimation(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] HideWithAnimation start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->HideWithAnimation();
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] HideWithAnimation success");
    return res;
}

int32_t FfiOHOSWindowShowWindow(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] ShowWindow start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->ShowWindow();
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] ShowWindow success");
    return res;
}

int32_t FfiOHOSWindowShowWithAnimation(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] ShowWithAnimation start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->ShowWithAnimation();
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] ShowWithAnimation success");
    return res;
}

int32_t FfiOHOSWindowDestroyWindow(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] DestroyWindow start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->DestroyWindow();
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] DestroyWindow success");
    return res;
}

int32_t FfiOHOSWindowMoveWindowTo(int64_t id, int32_t x, int32_t y)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] MoveWindowTo start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->MoveWindowTo(x, y);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] MoveWindowTo success");
    return res;
}

int32_t FfiOHOSWindowResize(int64_t id, uint32_t width, uint32_t height)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Resize start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Resize(width, height);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Resize success");
    return res;
}

int32_t FfiOHOSWindowSetWindowMode(int64_t id, uint32_t mode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowMode start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowMode(mode);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowMode success");
    return res;
}

CWindowProperties FfiOHOSWindowGetWindowProperties(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] GetWindowProperties start");
    CWindowProperties res = {};
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return res;
    }
    res = instance->GetWindowProperties(errCode);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] GetWindowProperties success");
    return res;
}

int32_t FfiOHOSWindowSetWindowLayoutFullScreen(int64_t id, bool isLayoutFullScreen)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowLayoutFullScreen start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowLayoutFullScreen(isLayoutFullScreen);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowLayoutFullScreen success");
    return res;
}

int32_t FfiOHOSSetWindowBackgroundColor(int64_t id, const char* color)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowBackgroundColor start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SetWindowBackgroundColor(color);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowBackgroundColor success");
    return ret;
}

int32_t FfiOHOSWindowSetWindowBrightness(int64_t id, float brightness)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowBrightness start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowBrightness(brightness);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowBrightness success");
    return res;
}

int32_t FfiOHOSWindowSetWindowFocusable(int64_t id, bool focusable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowFocusable start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowFocusable(focusable);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowFocusable success");
    return res;
}

int32_t FfiOHOSWindowSetWindowKeepScreenOn(int64_t id, bool keepScreenOn)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowKeepScreenOn start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowKeepScreenOn(keepScreenOn);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowKeepScreenOn success");
    return res;
}

int32_t FfiOHOSWindowSetWindowPrivacyMode(int64_t id, bool isPrivacyMode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowPrivacyMode start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowPrivacyMode(isPrivacyMode);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowPrivacyMode success");
    return res;
}

int32_t FfiOHOSWindowSetWindowTouchable(int64_t id, bool touchable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowTouchable start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowTouchable(touchable);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowTouchable success");
    return res;
}

int32_t FfiOHOSWindowSetForbidSplitMove(int64_t id, bool isForbidSplitMove)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetForbidSplitMove start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetForbidSplitMove(isForbidSplitMove);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetForbidSplitMove success");
    return res;
}

bool FfiOHOSWindowIsWindowSupportWideGamut(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] IsWindowSupportWideGamut start");
    bool res = false;
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return res;
    }
    res = instance->IsWindowSupportWideGamut(errCode);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] IsWindowSupportWideGamut success");
    return res;
}

bool FfiOHOSWindowIsWindowShowing(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] IsWindowShowing start");
    bool res = false;
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] instance not exist %{public}" PRId64, id);
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return res;
    }
    res = instance->IsWindowShowing(errCode);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] IsWindowShowing success");
    return res;
}

RetDataI64 FFiOHOSWindowFindWindow(char* name)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowFindWindow start");
    RetDataI64 ret = {};
    ret.code = WindowManagerImpl::FindWindow(name, ret.data);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowFindWindow success");
    return ret;
}

int32_t FFiOHOSWindowToggleShownStateForAllAppWindows()
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowToggleShownStateForAllAppWindows start");
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().ToggleShownStateForAllAppWindows());
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowToggleShownStateForAllAppWindows error %{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowToggleShownStateForAllAppWindows success");
    return SUCCESS_CODE;
}

int32_t FFiOHOSWindowSetGestureNavigationEnabled(bool enable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetGestureNavigationEnabled start");
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().SetGestureNavigationEnabled(enable));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetGestureNavigationEnabled error %{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetGestureNavigationEnabled success");
    return SUCCESS_CODE;
}

int32_t FFiOHOSWindowSetWaterMarkImage(int64_t pixelMapId, bool enable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetWaterMarkImage start");
    auto pixelMapImpl = FFIData::GetData<PixelMapImpl>(pixelMapId);
    if (pixelMapImpl == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    auto pixelMap = pixelMapImpl->GetRealPixelMap();
    RSInterfaces::GetInstance().ShowWatermark(pixelMap, enable);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetWaterMarkImage success");
    return SUCCESS_CODE;
}

int32_t FFiOHOSWindowSetBackdropBlurStyle(int64_t id, uint32_t blurStyle)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetBackdropBlurStyle start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetBackdropBlurStyle(blurStyle);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetBackdropBlurStyle success");
    }
    return res;
}

int32_t FFiOHOSWindowSetPreferredOrientation(int64_t id, uint32_t orientation)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetPreferredOrientation start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetPreferredOrientation(orientation);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetPreferredOrientation success");
    }
    return res;
}

int32_t FFiOHOSWindowGetWindowAvoidArea(int64_t id, uint32_t areaType, OHOS::Rosen::CAvoidArea* retPtr)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowGetWindowAvoidArea start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->GetWindowAvoidArea(areaType, retPtr);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowGetWindowAvoidArea success");
    }
    return res;
}

int32_t FFiOHOSWindowSetWaterMarkFlag(int64_t id, bool enable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetWaterMarkFlag start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWaterMarkFlag(enable);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetWaterMarkFlag success");
    }
    return res;
}

int32_t FFiOHOSWindowSetAspectRatio(int64_t id, double ratio)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetAspectRatio start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetAspectRatio(ratio);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetAspectRatio success");
    }
    return res;
}

int32_t FFiOHOSWindowSetShadowRadius(int64_t id, double radius)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowRadius start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetShadowRadius(radius);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowRadius success");
    }
    return res;
}

int32_t FFiOHOSWindowSetShadowColor(int64_t id, char* color)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowColor start");
    std::string windowColor = color;
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetShadowColor(windowColor);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowColor success");
    }
    return res;
}

int32_t FFiOHOSWindowSetShadowOffsetX(int64_t id, double offsetX)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowOffsetX start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetShadowOffsetX(offsetX);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowOffsetX success");
    }
    return res;
}

int32_t FFiOHOSWindowSetShadowOffsetY(int64_t id, double offsetY)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowOffsetY start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetShadowOffsetY(offsetY);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetShadowOffsetY success");
    }
    return res;
}

int32_t FFiOHOSWindowSetBackdropBlur(int64_t id, double radius)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetBackdropBlur start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetBackdropBlur(radius);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetBackdropBlur success");
    }
    return res;
}

int32_t FFiOHOSWindowSetBlur(int64_t id, double radius)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetBlur start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetBlur(radius);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowSetBlur success");
    }
    return res;
}

int32_t FFiOHOSWindowResetAspectRatio(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowResetAspectRatio start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->ResetAspectRatio();
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] FFiOHOSWindowResetAspectRatio success");
    }
    return res;
}

int32_t FFiOHOSWindowMinimize(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Minimize start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Minimize();
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Minimize success");
    }
    return res;
}

int32_t FFiOHOSWindowSetWindowColorSpace(int64_t id, uint32_t colorSpace)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowColorSpace start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowColorSpace(colorSpace);
    if (res == SUCCESS_CODE) {
        TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowColorSpace success");
    }
    return res;
}

int32_t FFiOHOSWindowSetCornerRadius(int64_t id, float radius)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetCornerRadius start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetCornerRadius(radius);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetCornerRadius success");
    return res;
}

int32_t FFiOHOSWindowSetResizeByDragEnabled(int64_t id, bool enable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetResizeByDragEnabled start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetResizeByDragEnabled(enable);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetResizeByDragEnabled success");
    return res;
}

int32_t FFiOHOSWindowRaiseToAppTop(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] RaiseToAppTop start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->RaiseToAppTop();
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] RaiseToAppTop success");
    return res;
}

int32_t FFiOHOSWindowSetSnapshotSkip(int64_t id, bool isSkip)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetSnapshotSkip start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetSnapshotSkip(isSkip);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetSnapshotSkip success");
    return res;
}

int32_t FFiOHOSWindowSetWakeUpScreen(int64_t id, bool wakeUp)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWakeUpScreen start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = static_cast<int32_t>(instance->SetWakeUpScreen(wakeUp));
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWakeUpScreen success");
    return res;
}

int32_t FFiOHOSWindowSetRaiseByClickEnabled(int64_t id, bool enable)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetRaiseByClickEnabled start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetRaiseByClickEnabled(enable);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetRaiseByClickEnabled success");
    return res;
}

int32_t FFiOHOSWindowGetWindowColorSpace(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] GetWindowColorSpace start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return 0;
    }
    int32_t res = static_cast<int32_t>(instance->GetWindowColorSpace(errCode));
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] GetWindowColorSpace success");
    return res;
}

int32_t FFiOHOSWindowRaiseAboveTarget(int64_t id, int32_t windowId)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] RaiseAboveTarget start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->RaiseAboveTarget(windowId);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] RaiseAboveTarget success");
    return res;
}

int32_t FFiOHOSWindowTranslate(int64_t id, double x, double y, double z)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Translate start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Translate(x, y, z);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Translate success");
    return res;
}

int32_t FFiOHOSWindowRotate(int64_t id, WindowRotate window)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Rotate start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Rotate(window.x, window.y, window.z, window.pivotX, window.pivotY);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Rotate success");
    return res;
}

int32_t FFiOHOSWindowScale(int64_t id, double x, double y, double pivotX, double pivotY)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Scale start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Scale(x, y, pivotX, pivotY);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Scale success");
    return res;
}

int32_t FFiOHOSWindowOpacity(int64_t id, double opacity)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Opacity start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->Opacity(opacity);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Opacity success");
    return res;
}

int64_t FFiOHOSWindowSnapshot(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Snapshot start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return 0;
    }
    std::shared_ptr<Media::PixelMap> pmap = instance->Snapshot(errCode);
    if (*errCode != 0) {
        return 0;
    }
    auto nativeImage = FFIData::Create<PixelMapImpl>(move(pmap));
    if (nativeImage == nullptr) {
        TLOGI(WmsLogTag::WMS_DIALOG, "create image failed");
        return 0;
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] Snapshot success");
    return nativeImage->GetID();
}

int32_t FFiOHOSWindowSetWindowSystemBarEnable(int64_t id, CArrString arr)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowSystemBarEnable start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t res = instance->SetWindowSystemBarEnable(arr.head, (uint32_t)arr.size);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowSystemBarEnable success");
    return res;
}

int32_t FFiOHOSWindowSetWindowSystemBarProperties(int64_t id, CJBarProperties properties)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowSystemBarProperties start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    CBarProperties cProperties;
    cProperties.statusBarColor = properties.statusBarColor;
    cProperties.isStatusBarLightIcon = properties.isStatusBarLightIcon;
    cProperties.statusBarContentColor = properties.statusBarContentColor;
    cProperties.navigationBarColor = properties.navigationBarColor;
    cProperties.navigationBarContentColor = properties.navigationBarContentColor;
    cProperties.isNavigationBarLightIcon = properties.isNavigationBarLightIcon;
    cProperties.enableStatusBarAnimation = properties.enableStatusBarAnimation;
    cProperties.enableNavigationBarAnimation = properties.enableNavigationBarAnimation;
    int32_t res = instance->SetWindowSystemBarProperties(cProperties);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] SetWindowSystemBarProperties success");
    return res;
}

int32_t FfiOHOSOnKeyboardHeightChange(int64_t id, int64_t callbackId)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] OnKeyboardHeightChange start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->OnRegisterWindowCallback("keyboardHeightChange", callbackId);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] OnKeyboardHeightChange success");
    return ret;
}

int32_t FfiOHOSOffKeyboardHeightChange(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] OffKeyboardHeightChange start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->UnregisterWindowCallback("keyboardHeightChange", (-1));
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] OffKeyboardHeightChange success");
    return ret;
}

int32_t FfiOHOSOnWindowEvent(int64_t id, int64_t callbackId)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] OnWindowEvent start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->OnRegisterWindowCallback("windowEvent", callbackId);
    TLOGI(WmsLogTag::WMS_DIALOG, "[Window] OnWindowEvent success");
    return ret;
}

int32_t FfiOHOSOnCallback(int64_t id, int64_t callbackId, const char* callbackType)
{
    TLOGI(WmsLogTag::DEFAULT, "On %{public}s start", callbackType);
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    const std::string type = callbackType;
    int32_t ret = instance->OnRegisterWindowCallback(type, callbackId);
    TLOGI(WmsLogTag::DEFAULT, "On %{public}s success", callbackType);
    return ret;
}

int32_t FfiOHOSOffCallback(int64_t id, int64_t callbackId, const char* callbackType)
{
    TLOGI(WmsLogTag::DEFAULT, "Off %{public}s start", callbackType);
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    const std::string type = callbackType;
    int32_t ret = instance->UnregisterWindowCallback(type, callbackId);
    TLOGI(WmsLogTag::DEFAULT, "Off %{public}s success", callbackType);
    return ret;
}

int32_t FfiOHOSOnNoInteractionDetected(int64_t id, int64_t timeout, int64_t callbackId)
{
    TLOGI(WmsLogTag::DEFAULT, "OnNoInteractionDetected start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    const std::string type = "noInteractionDetected";
    int32_t ret = instance->OnRegisterWindowCallback(type, callbackId, timeout);
    TLOGI(WmsLogTag::DEFAULT, "OnNoInteractionDetected success");
    return ret;
}

int32_t FfiOHOSSetSubWindowModal(int64_t id, bool isModal)
{
    TLOGI(WmsLogTag::WMS_SUB, "SetSubWindowModal start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SetSubWindowModal(isModal);
    TLOGI(WmsLogTag::WMS_SUB, "SetSubWindowModal success");
    return ret;
}

int32_t FfiOHOSIsFocused(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "IsFocused start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return 0;
    }
    int32_t ret = instance->GetWindowProperties(errCode).focusable;
    TLOGI(WmsLogTag::WMS_FOCUS, "IsFocused success");
    return ret;
}

int32_t FfiOHOSSetWindowLimits(int64_t id,
                               OHOS::Rosen::CWindowLimits windloLimits,
                               OHOS::Rosen::CWindowLimits* retPtr)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "SetWindowLimits start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }

    CWindowLimits cWindowLimits;
    cWindowLimits.maxWidth = windloLimits.maxWidth;
    cWindowLimits.maxHeight = windloLimits.maxHeight;
    cWindowLimits.minWidth = windloLimits.minWidth;
    cWindowLimits.minHeight = windloLimits.minHeight;

    int32_t ret = instance->SetWindowLimits(cWindowLimits, *retPtr);
    TLOGI(WmsLogTag::WMS_LAYOUT, "SetWindowLimits success");
    return ret;
}

int32_t FfiOHOSGetWindowLimits(int64_t id, OHOS::Rosen::CWindowLimits* retPtr)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "GetWindowLimits start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }

    int32_t ret = instance->GetWindowLimits(*retPtr);
    TLOGI(WmsLogTag::WMS_LAYOUT, "GetWindowLimits success");
    return ret;
}

bool FfiOHOSGetImmersiveModeEnabledState(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_IMMS, "GetImmersiveModeEnabledState start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        *errCode = WM_ERROR_STATE_ABNORMALLY;
        return false;
    }
    bool ret = instance->GetImmersiveModeEnabledState(*errCode);
    TLOGI(WmsLogTag::WMS_IMMS, "GetImmersiveModeEnabledState success");
    return ret;
}

int32_t FfiOHOSSetImmersiveModeEnabledState(int64_t id, bool enabled)
{
    TLOGI(WmsLogTag::WMS_IMMS, "SetImmersiveModeEnabledState start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SetImmersiveModeEnabledState(enabled);
    TLOGI(WmsLogTag::WMS_IMMS, "SetImmersiveModeEnabledState success");
    return ret;
}

int32_t FfiOHOSKeepKeyboardOnFocus(int64_t id, bool keepKeyboardFlag)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "KeepKeyboardOnFocus start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->KeepKeyboardOnFocus(keepKeyboardFlag);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "KeepKeyboardOnFocus success");
    return ret;
}

int32_t FfiOHOSGetWindowDecorHeight(int64_t id, int32_t* height)
{
    TLOGI(WmsLogTag::WMS_DECOR, "GetWindowDecorHeight start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }

    int32_t ret = instance->GetWindowDecorHeight(*height);
    TLOGI(WmsLogTag::WMS_DECOR, "GetWindowDecorHeight success");
    return ret;
}

int32_t FfiOHOSSetWindowDecorHeight(int64_t id, int32_t height)
{
    TLOGI(WmsLogTag::WMS_DECOR, "SetWindowDecorHeight start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }

    int32_t ret = instance->SetWindowDecorHeight(height);
    TLOGI(WmsLogTag::WMS_DECOR, "SetWindowDecorHeight success");
    return ret;
}

int32_t FfiOHOSRecover(int64_t id)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "Recover start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }

    int32_t ret = instance->Recover();
    TLOGI(WmsLogTag::WMS_RECOVER, "Recover success");
    return ret;
}

int32_t FfiOHOSSetWindowDecorVisible(int64_t id, bool isVisible)
{
    TLOGI(WmsLogTag::WMS_DECOR, "SetWindowDecorVisible start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SetWindowDecorVisible(isVisible);
    TLOGI(WmsLogTag::WMS_DECOR, "SetWindowDecorVisible success");
    return ret;
}

int32_t FfiOHOSGetTitleButtonRect(int64_t id,
                                  OHOS::Rosen::CTitleButtonRect* retPtr)
{
    TLOGI(WmsLogTag::WMS_DECOR, "GetTitleButtonRect start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }

    int32_t ret = instance->GetTitleButtonRect(*retPtr);
    TLOGI(WmsLogTag::WMS_DECOR, "GetTitleButtonRect success");
    return ret;
}

int32_t FfiOHOSSetDialogBackGestureEnabled(int64_t id, bool enabled)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "SetDialogBackGestureEnabled start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SetDialogBackGestureEnabled(enabled);
    TLOGI(WmsLogTag::WMS_DIALOG, "SetDialogBackGestureEnabled success");
    return ret;
}

int32_t FfiOHOSDisableLandscapeMultiWindow(int64_t id)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "DisableLandscapeMultiWindow start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->DisableLandscapeMultiWindow();
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "DisableLandscapeMultiWindow success");
    return ret;
}

int32_t FfiOHOSEnableLandscapeMultiWindow(int64_t id)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "EnableLandscapeMultiWindow start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->EnableLandscapeMultiWindow();
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "EnableLandscapeMultiWindow success");
    return ret;
}

int32_t FfiOHOSSetWindowGrayScale(int64_t id, float grayScale)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "SetWindowGrayScale start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SetWindowGrayScale(grayScale);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "SetWindowGrayScale success");
    return ret;
}

int32_t FfiOHOSSpecificSystemBarEnabled(int64_t id, int32_t name, bool enable,
                                        bool enableAnimation)
{
    TLOGI(WmsLogTag::WMS_IMMS, "SpecificSystemBarEnabled start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->SpecificSystemBarEnabled(name, enable, enableAnimation);
    TLOGI(WmsLogTag::WMS_IMMS, "SpecificSystemBarEnabled success");
    return ret;
}

int32_t FfiOHOSGetWindowSystemBarProperties(int64_t id,
                                            CJBarProperties* systemBarProperty)
{
    TLOGI(WmsLogTag::WMS_IMMS, "GetWindowSystemBarProperties start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->GetWindowSystemBarProperties(*systemBarProperty);
    TLOGI(WmsLogTag::WMS_IMMS, "GetWindowSystemBarProperties success");
    return ret;
}

uint32_t FfiOHOSGetPreferredOrientation(int64_t id, int32_t* errCode)
{
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    uint32_t ret = instance->GetPreferredOrientation(*errCode);
    return ret;
}

int32_t FfiOHOSGetWindowStatus(int64_t id, int32_t* errCode)
{
    TLOGI(WmsLogTag::WMS_PC, "GetWindowStatus start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->GetWindowStatus(*errCode);
    TLOGI(WmsLogTag::WMS_PC, "GetWindowStatus success");
    return ret;
}

int32_t FfiOHOSMaximize(int64_t id, int32_t presentation)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "Maximize start");
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->Maximize(presentation);
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "Maximize success");
    return ret;
}

RetDataI64 FfiOHOSCreateSubWindowWithOptions(int64_t id, char* name,
                                             CSubWindowOptions option)
{
    TLOGI(WmsLogTag::WMS_SUB, "CreateSubWindowWithOptions start");
    RetDataI64 ret = {};
    std::string nm = name;
    auto instance = FFIData::GetData<CJWindowImpl>(id);
    if (instance == nullptr) {
        ret.code = WM_ERROR_STATE_ABNORMALLY;
        return ret;
    }
    ret.code = instance->CreateSubWindowWithOptions(nm, ret.data, option);
    TLOGI(WmsLogTag::WMS_SUB, "CreateSubWindowWithOptions success");
    return ret;
}

// WindowStage
RetDataI64 FfiOHOSBindWindowStage(int64_t windowStageImplPtr)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] bind to created WindowStage");
    RetDataI64 ret = {};
    auto actualWs = reinterpret_cast<CJWindowStageImpl*>(windowStageImplPtr);
    if (actualWs == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[WindowStage] Bind null obj");
        ret.code = static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
        return ret;
    }
    ret.code = static_cast<int32_t>(WMError::WM_OK);
    ret.data = actualWs->GetID();
    return ret;
}

RetDataI64 FfiOHOSGetMainWindow(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] getMainWindow start");
    RetDataI64 ret = {};
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        ret.code = WM_ERROR_STATE_ABNORMALLY;
        return ret;
    }
    ret.code = instance->GetMainWindow(ret.data);
    return ret;
}

RetDataI64 FfiOHOSCreateSubWindow(int64_t id, char* name)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] createSubWindow start");
    RetDataI64 ret = {};
    std::string nm = name;
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        ret.code = WM_ERROR_STATE_ABNORMALLY;
        return ret;
    }
    ret.code = instance->CreateSubWindow(nm, ret.data);
    return ret;
}

RetStruct FfiOHOSGetSubWindow(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] getSubWindow start");
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        return {WM_ERROR_STATE_ABNORMALLY, 0, nullptr};
    }
    auto ret = instance->GetSubWindow();
    return ret;
}

int32_t FfiOHOSLoadContent(int64_t id, char* path)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] loadContent start");
    std::string url = path;
    std::string storageJson = " ";
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    return instance->OnLoadContent(url, storageJson, false);
}

int32_t FfiOHOSLoadContentByName(int64_t id, char* name)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] loadContentByName start");
    return SUCCESS_CODE;
}

int32_t FfiOHOSDisableWindowDecor(int64_t id)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] disableWindowDecor start");
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    return instance->DisableWindowDecor();
}

int32_t FfiOHOSSetShowOnLockScreen(int64_t id, bool showOnLockScreen)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowStage] setShowOnLockScreen start");
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    return instance->SetShowOnLockScreen(showOnLockScreen);
}

int32_t FfiOHOSSetDefaultDensityEnabled(int64_t id, bool enabled)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "setDefaultDensityEnabled start");
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        return WM_ERROR_STATE_ABNORMALLY;
    }
    return instance->SetDefaultDensityEnabled(enabled);
}

int32_t FfiOHOSStageOn(int64_t id, int64_t callbackId)
{
    TLOGI(WmsLogTag::DEFAULT, "on windowStageEvent start");
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "on windowStageEvent is nullptr");
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->OnEvent(callbackId);
    TLOGI(WmsLogTag::DEFAULT, "on windowStageEvent success");
    return ret;
}

int32_t FfiOHOSStageOff(int64_t id, int64_t callbackId)
{
    TLOGI(WmsLogTag::DEFAULT, "off windowStageEvent start");
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "off windowStageEvent is nullptr");
        return WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t ret = instance->OffEvent(callbackId);
    TLOGI(WmsLogTag::DEFAULT, "off windowStageEvent success");
    return ret;
}

RetDataI64 FfiOHOSCreateSubWindowWithOptionsStage(int64_t id, const char* name,
    const char* title, bool decorEnabled, bool isModal)
{
    TLOGI(WmsLogTag::WMS_SUB, "CreateSubWindowWithOptionsStage start");
    RetDataI64 ret = {};
    const std::string nameStr = name;
    const std::string titleStr = title;
    auto instance = FFIData::GetData<CJWindowStageImpl>(id);
    if (instance == nullptr) {
        ret.code = WM_ERROR_STATE_ABNORMALLY;
        return ret;
    }
    ret.code = instance->CreateSubWindowWithOptions(ret.data, nameStr, titleStr, decorEnabled, isModal);
    return ret;
}

} // extern "C"
}
}
