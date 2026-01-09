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

#include "ani_window.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <ui_content.h>

#include "ani.h"
#include "ani_err_utils.h"
#include "ani_remote_object.h"
#include "ani_transition_controller.h"
#include "ani_window_utils.h"
#include "ani_window_animation_utils.h"
#include "ani_common_want.h"
#include "ets_request_info.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "permission.h"
#include "pixel_map.h"
#include "pixel_map_taihe_ani.h"
#include "window_helper.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "window_scene.h"
#include "wm_common.h"
#include "wm_math.h"
#include "permission.h"

using OHOS::Rosen::WindowScene;

namespace OHOS {
namespace Rosen {
constexpr int32_t MIN_DECOR_HEIGHT = 37;
constexpr int32_t MAX_DECOR_HEIGHT = 112;
constexpr uint32_t DEFAULT_WINDOW_MAX_WIDTH = 3840;
namespace {
/* used for free, ani has no destructor right now, only free when aniObj freed */
static std::map<ani_ref, AniWindow*> g_localObjs;
constexpr double MIN_GRAY_SCALE = 0.0;
constexpr double MAX_GRAY_SCALE = 1.0;
} // namespace
static std::mutex g_aniWindowMap_mutex;
static std::map<std::string, ani_ref> g_aniWindowMap;

AniWindow::AniWindow(const sptr<Window>& window)
    : windowToken_(window), registerManager_(std::make_unique<AniWindowRegisterManager>())
{
    NotifyNativeWinDestroyFunc func = [this](const std::string& windowName) {
        {
            std::lock_guard<std::mutex> lock(g_aniWindowMap_mutex);
            if (windowName.empty() || g_aniWindowMap.count(windowName) == 0) {
                TLOGE(WmsLogTag::WMS_LIFE, "Can not find window %{public}s ", windowName.c_str());
                return;
            }
            g_aniWindowMap.erase(windowName);
            TLOGI(WmsLogTag::WMS_LIFE, "Remove window %{public}s", windowName.c_str());
        }
        windowToken_ = nullptr;
        TLOGI(WmsLogTag::WMS_LIFE, "Destroy window %{public}s in js window", windowName.c_str());
    };
    windowToken_->RegisterWindowDestroyedListener(func);
}

AniWindow::~AniWindow()
{
    if (windowToken_ != nullptr) {
        windowToken_->UnregisterWindowDestroyedListener();
    }
}

AniWindow* AniWindow::GetWindowObjectFromEnv(ani_env* env, ani_object obj)
{
    ani_class cls = nullptr;
    ani_status ret;
    if ((ret = env->FindClass("@ohos.window.window.Window", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]null env %{public}u", ret);
        return nullptr;
    }
    ani_field nativeObjName {};
    if ((ret = env->Class_FindField(cls, "nativeObj", &nativeObjName)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]obj fetch field %{public}u", ret);
        return nullptr;
    }
    ani_long nativeObj {};
    if ((ret = env->Object_GetField_Long(obj, nativeObjName, &nativeObj)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]obj fetch long %{public}u", ret);
        return nullptr;
    }
    return reinterpret_cast<AniWindow*>(nativeObj);
}

ani_object AniWindow::NativeTransferStatic(ani_env* aniEnv, ani_class cls, ani_object input)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    void *unwrapResult = nullptr;
    if (!arkts_esvalue_unwrap(aniEnv, input, &unwrapResult)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to unwrap input");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    if (unwrapResult == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] unwrapResult is nullptr");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    JsWindow* jsWindow = static_cast<JsWindow*>(unwrapResult);
    sptr<Window> windowToken = jsWindow->GetWindow();
    ani_ref aniWindowObj = CreateAniWindowObject(aniEnv, windowToken);
    if (aniWindowObj == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ani window object is nullptr");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    return static_cast<ani_object>(aniWindowObj);
}

ani_object AniWindow::NativeTransferDynamic(ani_env* aniEnv, ani_class cls, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    napi_env napiEnv {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] napi scope open fail");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    sptr<Window> windowToken = aniWindow->GetWindow();
    napi_value jsWindow = CreateJsWindowObject(napiEnv, windowToken);
    hybridgref ref {};
    if (!hybridgref_create_from_napi(napiEnv, jsWindow, &ref)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create hybridgref fail");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    ani_object result {};
    if (!hybridgref_get_esvalue(aniEnv, ref, &result)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] get esvalue fail");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    if (!hybridgref_delete_from_napi(napiEnv, ref)) {
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        TLOGE(WmsLogTag::DEFAULT, "[ANI] delete hybridgref fail");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] napi close scope fail");
        return AniWindowUtils::CreateAniUndefined(aniEnv);
    }
    return result;
}

void AniWindow::ThrowIfWindowInvalid(ani_env* env, ani_object obj, ani_long nativeObj)
{
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto window = aniWindow->GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::SetWindowColorSpace(ani_env* env, ani_object obj, ani_long nativeObj, ani_int colorSpace)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] colorSpace:%{public}d", static_cast<int32_t>(colorSpace));
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowColorSpace(env, colorSpace);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetWindowColorSpace(ani_env* env, ani_int colorSpace)
{
    if (static_cast<int32_t>(colorSpace) > static_cast<int32_t>(ColorSpace::COLOR_SPACE_WIDE_GAMUT) ||
        static_cast<int32_t>(colorSpace) < static_cast<int32_t>(ColorSpace::COLOR_SPACE_DEFAULT)) {
        TLOGE(WmsLogTag::DEFAULT, "ColorSpace %{public}u invalid!", static_cast<uint32_t>(colorSpace));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] colorSpace:%{public}d", static_cast<int32_t>(colorSpace));
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    window->SetColorSpace(static_cast<ColorSpace>(colorSpace));
    TLOGI(WmsLogTag::DEFAULT, "[ANI] SetWindowColorSpace end");
}

void AniWindow::SetPreferredOrientation(ani_env* env, ani_object obj, ani_long nativeObj, ani_int orientation)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI] orientation:%{public}d", static_cast<int32_t>(orientation));
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetPreferredOrientation(env, orientation);
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetPreferredOrientation(ani_env* env, ani_int orientation)
{
    int32_t orientationValue = static_cast<int32_t>(orientation);
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI] orientation:%{public}d", orientationValue);
    WmErrorCode errCode = WmErrorCode::WM_OK;
    Orientation requestedOrientation = Orientation::UNSPECIFIED;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    auto apiOrientation = static_cast<ApiOrientation>(orientationValue);
    if (apiOrientation < ApiOrientation::BEGIN || apiOrientation > ApiOrientation::END) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Orientation %{public}u invalid!",
            static_cast<uint32_t>(apiOrientation));
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        requestedOrientation = JS_TO_NATIVE_ORIENTATION_MAP.at(apiOrientation);
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    window->SetRequestedOrientation(requestedOrientation);
    window->NotifyPreferredOrientationChange(requestedOrientation);
    TLOGNI(WmsLogTag::WMS_ROTATION,
        "[ANI] SetPreferredOrientation end, window [%{public}u, %{public}s] orientation=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), orientationValue);
}

ani_int AniWindow::GetPreferredOrientation(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnGetPreferredOrientation(env);
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ani_int(0);
    }
}

ani_int AniWindow::OnGetPreferredOrientation(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    Orientation requestedOrientation = Orientation::UNSPECIFIED;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return static_cast<ani_int>(requestedOrientation);
    }
    requestedOrientation = window->GetRequestedOrientation();
    ApiOrientation apiOrientation = ApiOrientation::UNSPECIFIED;
    if (requestedOrientation >= Orientation::BEGIN &&
        requestedOrientation <= Orientation::END) {
        apiOrientation = NATIVE_TO_JS_ORIENTATION_MAP.at(requestedOrientation);
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Orientation %{public}u invalid!",
            static_cast<uint32_t>(requestedOrientation));
    }
    TLOGNI(WmsLogTag::WMS_ROTATION, "[ANI] Window [%{public}u, %{public}s] Get PreferredOrientation end, "
        "orientation=%{public}u", window->GetWindowId(), window->GetWindowName().c_str(),
        static_cast<uint32_t>(apiOrientation));
    return static_cast<ani_int>(apiOrientation);
}

ani_int AniWindow::ConvertOrientationAndRotation(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_int from, ani_int to, ani_int value)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnConvertOrientationAndRotation(env, from, to, value);
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
}

ani_int AniWindow::OnConvertOrientationAndRotation(ani_env* env, ani_int from, ani_int to, ani_int value)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    auto fromRotationInfoType = static_cast<RotationInfoType>(from);
    if (fromRotationInfoType < RotationInfoType::WINDOW_ORIENTATION ||
        fromRotationInfoType > RotationInfoType::DISPLAY_ROTATION) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Invalid from RotationInfoType : %{public}d", from);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return ANI_ERROR;
    }
    auto toRotationInfoType = static_cast<RotationInfoType>(to);
    if (toRotationInfoType < RotationInfoType::WINDOW_ORIENTATION ||
        toRotationInfoType > RotationInfoType::DISPLAY_ROTATION) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Invalid to RotationInfoType : %{public}d", to);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return ANI_ERROR;
    }
    int32_t convertedValue;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        window->ConvertOrientationAndRotation(fromRotationInfoType, toRotationInfoType, value, convertedValue));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Window ConvertOrientationAndRotation falied");
        AniWindowUtils::AniThrowError(env, ret);
        return ANI_ERROR;
    }
    TLOGNI(WmsLogTag::WMS_ROTATION,
           "[ANI] ConvertOrientationAndRotation end, Window [%{public}u, %{public}s] convertedValue=%{public}u",
           window->GetWindowId(), window->GetWindowName().c_str(), convertedValue);
    return static_cast<ani_int>(convertedValue);
}

void AniWindow::Opacity(ani_env* env, ani_object obj, ani_long nativeObj, ani_double opacity)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] opacity: %{public}f", static_cast<double>(opacity));
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnOpacity(env, opacity);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnOpacity(ani_env* env, ani_double opacity)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] OnOpacity in");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Opacity is not allowed since window is not system window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    if (MathHelper::LessNotEqual(static_cast<double>(opacity), 0.0) ||
        MathHelper::GreatNotEqual(static_cast<double>(opacity), 1.0)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Opacity should greater than 0 or smaller than 1.0");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetAlpha(static_cast<double>(opacity)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Window Opacity failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] window [%{public}u, %{public}s] Opacity end, opacity=%{public}f",
        window->GetWindowId(), window->GetWindowName().c_str(), static_cast<double>(opacity));
}

void AniWindow::Scale(ani_env* env, ani_object obj, ani_long nativeObj, ani_object scaleOptions)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Set window Scale in");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnScale(env, scaleOptions);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

static bool IsPivotValid(double data)
{
    if (MathHelper::LessNotEqual(data, 0.0) || (MathHelper::GreatNotEqual(data, 1.0))) {
        return false;
    }
    return true;
}

static bool IsScaleValid(double data)
{
    if (!MathHelper::GreatNotEqual(data, 0.0)) {
        return false;
    }
    return true;
}

bool AniWindow::ParseScaleOption(ani_env* env, ani_object scaleOptions, Transform& trans)
{
    double scaleX;
    ani_double retScaleX = AniWindowUtils::GetPropertyDoubleObject(env, "x", scaleOptions, scaleX);
    if (retScaleX == ANI_OK) {
        if (!IsScaleValid(scaleX)) {
            return false;
        }
        trans.scaleX_ = scaleX;
    }

    double scaleY;
    ani_double retScaleY = AniWindowUtils::GetPropertyDoubleObject(env, "y", scaleOptions, scaleY);
    if (retScaleY == ANI_OK) {
        if (!IsScaleValid(scaleY)) {
            return false;
        }
        trans.scaleY_ = scaleY;
    }

    double pivotX;
    ani_double retPivotX = AniWindowUtils::GetPropertyDoubleObject(env, "pivotX", scaleOptions, pivotX);
    if (retPivotX == ANI_OK) {
        if (!IsPivotValid(pivotX)) {
            return false;
        }
        trans.pivotX_ = pivotX;
    }

    double pivotY;
    ani_double retPivotY = AniWindowUtils::GetPropertyDoubleObject(env, "pivotY", scaleOptions, pivotY);
    if (retPivotY == ANI_OK) {
        if (!IsPivotValid(pivotY)) {
            return false;
        }
        trans.pivotY_ = pivotY;
    }
    TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] ParseScaleOption end");
    return true;
}

void AniWindow::OnScale(ani_env* env, ani_object scaleOptions)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] OnScale in");
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] not system app, permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }

    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    if (!WindowHelper::IsSystemWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Scale is not allowed since window is not system window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    auto trans = window->GetTransform();
    if (!ParseScaleOption(env, scaleOptions, trans)) {
        TLOGE(WmsLogTag::WMS_ANIMATION,
            "[ANI] PivotX or PivotY should between 0.0 ~ 1.0, scale should greater than 0.0");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetTransform(trans));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Window Scale failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Scale end",
        window->GetWindowId(), window->GetWindowName().c_str());
    TLOGNI(WmsLogTag::WMS_ANIMATION,
        "[ANI] scaleX=%{public}f, scaleY=%{public}f, pivotX=%{public}f pivotY=%{public}f",
        trans.scaleX_, trans.scaleY_, trans.pivotX_, trans.pivotY_);
}

void AniWindow::Translate(ani_env* env, ani_object obj, ani_long nativeObj, ani_object translateOptions)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Set window Translate in");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnTranslate(env, translateOptions);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

bool AniWindow::ParseTranslateOption(ani_env* env, ani_object translateOptions, Transform& trans)
{
    double translateX;
    ani_double retTranslateX = AniWindowUtils::GetPropertyDoubleObject(env, "x", translateOptions, translateX);
    if (retTranslateX == ANI_OK) {
        trans.translateX_ = translateX;
    }

    double translateY;
    ani_double retTranslateY = AniWindowUtils::GetPropertyDoubleObject(env, "y", translateOptions, translateY);
    if (retTranslateY == ANI_OK) {
        trans.translateY_ = translateY;
    }

    double translateZ;
    ani_double retTranslateZ = AniWindowUtils::GetPropertyDoubleObject(env, "z", translateOptions, translateZ);
    if (retTranslateZ == ANI_OK) {
        trans.translateZ_ = translateZ;
    }
    TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] ParseTranslateOption end");
    return true;
}

void AniWindow::OnTranslate(ani_env* env, ani_object translateOptions)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Translate in");
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] not system app, permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }

    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    if (!WindowHelper::IsSystemWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Translate is not allowed since window is not system window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    auto trans = window->GetTransform();
    if (!ParseTranslateOption(env, translateOptions, trans)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetTransform(trans));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Window Translate failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Translate end, "
        "translateX=%{public}f, translateY=%{public}f, translateZ=%{public}f",
        window->GetWindowId(), window->GetWindowName().c_str(),
        trans.translateX_, trans.translateY_, trans.translateZ_);
}

void AniWindow::Rotate(ani_env* env, ani_object obj, ani_long nativeObj, ani_object rotateOptions)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Set window Rotate in");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRotate(env, rotateOptions);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

bool AniWindow::ParseRotateOption(ani_env* env, ani_object rotateOptions, Transform& trans)
{
    double pivotX;
    ani_double retPivotX = AniWindowUtils::GetPropertyDoubleObject(env, "pivotX", rotateOptions, pivotX);
    if (retPivotX == ANI_OK) {
        if (!IsPivotValid(pivotX)) {
            return false;
        }
        trans.pivotX_ = pivotX;
    }

    double pivotY;
    ani_double retPivotY = AniWindowUtils::GetPropertyDoubleObject(env, "pivotY", rotateOptions, pivotY);
    if (retPivotY == ANI_OK) {
        if (!IsPivotValid(pivotY)) {
            return false;
        }
        trans.pivotY_ = pivotY;
    }

    double rotationX;
    ani_double retRotationX = AniWindowUtils::GetPropertyDoubleObject(env, "x", rotateOptions, rotationX);
    if (retRotationX == ANI_OK) {
        trans.rotationX_ = rotationX;
    }

    double rotationY;
    ani_double retRotationY = AniWindowUtils::GetPropertyDoubleObject(env, "y", rotateOptions, rotationY);
    if (retRotationY == ANI_OK) {
        trans.rotationY_ = rotationY;
    }

    double rotationZ;
    ani_double retRotationZ = AniWindowUtils::GetPropertyDoubleObject(env, "z", rotateOptions, rotationZ);
    if (retRotationZ == ANI_OK) {
        trans.rotationZ_ = rotationZ;
    }
    TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] ParseScaleOption end");
    return true;
}

void AniWindow::OnRotate(ani_env* env, ani_object rotateOptions)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] OnRotate in");
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] not system app, permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }

    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    if (!WindowHelper::IsSystemWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Rotate is not allowed since window is not system window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    // cannot use sync task since next transform base on current transform
    auto trans = window->GetTransform();
    if (!ParseRotateOption(env, rotateOptions, trans)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] PivotX or PivotY should between 0.0 ~ 1.0");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetTransform(trans));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Window Translate failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Rotate end",
        window->GetWindowId(), window->GetWindowName().c_str());
    TLOGNI(WmsLogTag::WMS_ANIMATION,
        "[ANI] rotateX=%{public}f, rotateY=%{public}f, rotateZ=%{public}f pivotX=%{public}f pivotY=%{public}f",
        trans.rotationX_, trans.rotationY_, trans.rotationZ_, trans.pivotX_, trans.pivotY_);
}

void AniWindow::SetShadow(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius,
    ani_string color, ani_object offsetX, ani_object offsetY)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Set window Shadow in");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetShadow(env, radius, color, offsetX, offsetY);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetShadow(ani_env* env, ani_double radius, ani_string color, ani_object offsetX, ani_object offsetY)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] OnSetShadow in");
    WmErrorCode ret = WmErrorCode::WM_OK;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSystemWindow(window->GetType()) &&
        !WindowHelper::IsSubWindow(window->GetType())) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    double radiusValue = static_cast<double>(radius);
    if (MathHelper::LessNotEqual(radiusValue, 0.0)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetShadowRadius(radiusValue));
    std::string colorValue = "";
    if (ret == WmErrorCode::WM_OK && ANI_OK == AniWindowUtils::GetStdString(env, color, colorValue)) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetShadowColor(colorValue));
    }

    double offsetXValue = 0.0;
    if (ret == WmErrorCode::WM_OK && ANI_OK == AniWindowUtils::GetDoubleObject(env, offsetX, offsetXValue)) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetShadowOffsetX(offsetXValue));
    }

    double offsetYValue = 0.0;
    if (ret == WmErrorCode::WM_OK && ANI_OK == AniWindowUtils::GetDoubleObject(env, offsetY, offsetYValue)) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetShadowOffsetY(offsetYValue));
    }
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set Window Shadow failed");
        AniWindowUtils::AniThrowError(env, ret);
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Set Window Shadow end, "
        "radius=%{public}f, color=%{public}s, offsetX=%{public}f offsetY=%{public}f",
        window->GetWindowId(), window->GetWindowName().c_str(),
        radiusValue, colorValue.c_str(), offsetXValue, offsetYValue);
}

void AniWindow::SetCornerRadius(ani_env* env, ani_object obj, ani_long nativeObj, ani_double cornerRadius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] In");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetCornerRadius(env, cornerRadius);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetCornerRadius(ani_env* env, ani_double cornerRadius)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Unexpected window type:%{public}d", windowToken_->GetType());
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    double radius = static_cast<double>(cornerRadius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Invalid radius:%{public}f", radius);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetCornerRadius(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SetCornerRadius failed:%{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGI(WmsLogTag::WMS_ANIMATION, "Window [%{public}u, %{public}s] set success, radius=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
}

void AniWindow::SetBlur(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetBlur(env, radius);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetBlur(ani_env* env, ani_double radius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    WmErrorCode ret = WmErrorCode::WM_OK;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSystemWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Unexpected window type:%{public}d", window->GetType());
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    double radiusValue = static_cast<double>(radius);
    if (MathHelper::LessNotEqual(radiusValue, 0.0)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Invalid radius:%{public}f", radiusValue);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetBlur(radiusValue));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set blur failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Set blur end, "
        "radius=%{public}f", window->GetWindowId(), window->GetWindowName().c_str(), radiusValue);
}

void AniWindow::SetBackdropBlurStyle(ani_env* env, ani_object obj, ani_long nativeObj, ani_int blurStyle)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetBackdropBlurStyle(env, blurStyle);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetBackdropBlurStyle(ani_env* env, ani_int blurStyle)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    WmErrorCode ret = WmErrorCode::WM_OK;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSystemWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Unexpected window type:%{public}d", window->GetType());
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    uint32_t resultValue = static_cast<uint32_t>(blurStyle);
    if (resultValue > static_cast<uint32_t>(WindowBlurStyle::WINDOW_BLUR_THICK)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Invalid window blur style:%{public}u", resultValue);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WindowBlurStyle style = static_cast<WindowBlurStyle>(resultValue);
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetBackdropBlurStyle(style));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set backdrop blur style failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Set backdrop blur style end, "
        "style=%{public}u", window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(style));
}

void AniWindow::SetBackdropBlur(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetBackdropBlur(env, radius);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetBackdropBlur(ani_env* env, ani_double radius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    WmErrorCode ret = WmErrorCode::WM_OK;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSystemWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Unexpected window type:%{public}d", window->GetType());
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    double radiusValue = static_cast<double>(radius);
    if (MathHelper::LessNotEqual(radiusValue, 0.0)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Invalid radius:%{public}f", radiusValue);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetBackdropBlur(radiusValue));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set backdrop blur failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Set backdrop blur end, "
        "radius=%{public}f", window->GetWindowId(), window->GetWindowName().c_str(), radiusValue);
}

ani_double AniWindow::GetWindowCornerRadius(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnGetWindowCornerRadius(env);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ani_double(0.0);
    }
}

ani_double AniWindow::OnGetWindowCornerRadius(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    WmErrorCode ret = WmErrorCode::WM_OK;
    float cornerRadius = 0.0f;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return cornerRadius;
    }
    if (!WindowHelper::IsFloatOrSubWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] This is not sub window or float window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return cornerRadius;
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->GetWindowCornerRadius(cornerRadius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get window corner radius failed");
        AniWindowUtils::AniThrowError(env, ret);
        return cornerRadius;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Get window corner radius end, "
        "cornerRadius=%{public}f", window->GetWindowId(), window->GetWindowName().c_str(), cornerRadius);
    return cornerRadius;
}

void AniWindow::SetWindowCornerRadius(ani_env* env, ani_object obj, ani_long nativeObj, ani_double cornerRadius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowCornerRadius(env, cornerRadius);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetWindowCornerRadius(ani_env* env, ani_double cornerRadius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    WmErrorCode ret = WmErrorCode::WM_OK;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    float cornerRadiusValue = static_cast<float>(cornerRadius);
    if (MathHelper::LessNotEqual(cornerRadiusValue, 0.0f)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] The corner radius is less than zero");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowCornerRadius(cornerRadiusValue));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set window corner radius failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Set window corner radius end, "
        "cornerRadius=%{public}f", window->GetWindowId(), window->GetWindowName().c_str(), cornerRadiusValue);
}

void AniWindow::SetWindowShadowRadius(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowShadowRadius(env, radius);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetWindowShadowRadius(ani_env* env, ani_double radius)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    WmErrorCode ret = WmErrorCode::WM_OK;
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsFloatOrSubWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] This is not sub window or float window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    float radiusValue = static_cast<float>(radius);
    std::shared_ptr<ShadowsInfo> shadowsInfo = std::make_shared<ShadowsInfo>();
    if (MathHelper::LessNotEqual(radiusValue, 0.0f)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] The shadow radius is less than zero");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    shadowsInfo->radius_ = radiusValue;
    shadowsInfo->hasRadiusValue_ = true;
    ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowShadowRadius(radiusValue));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set window shadow radius failed");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    WmErrorCode syncShadowsRet = WM_JS_TO_ERROR_CODE_MAP.at(window->SyncShadowsToComponent(*shadowsInfo));
    if (syncShadowsRet != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Sync shadows to component fail! ret: %{public}u",
            static_cast<int32_t>(syncShadowsRet));
        AniWindowUtils::AniThrowError(env, syncShadowsRet);
        return;
    }
    TLOGNI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] Set window shadow radius end, "
        "radius=%{public}f", window->GetWindowId(), window->GetWindowName().c_str(), radiusValue);
}

void AniWindow::SetWindowPrivacyMode(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isPrivacyMode)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowPrivacyMode(env, isPrivacyMode);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetWindowPrivacyMode(ani_env* env, ani_boolean isPrivacyMode)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetPrivacyMode(static_cast<bool>(isPrivacyMode)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] ret:%{public}d", static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindow::Recover(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRecover(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnRecover(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Recover(1));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window recover failed");
    }
}

void AniWindow::SetUIContent(ani_env* env, ani_object obj, ani_long nativeObj, ani_string path)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetUIContent(env, path);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetUIContent(ani_env* env, ani_string path)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string contentPath;
    AniWindowUtils::GetStdString(env, path, contentPath);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->AniSetUIContent(contentPath, env, nullptr));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window load content failed");
    }
}

void AniWindow::SetWindowKeepScreenOn(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isKeepScreenOn)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetWindowKeepScreenOn(env, isKeepScreenOn);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] SetWindowKeepScreenOn end");
}

void AniWindow::OnSetWindowKeepScreenOn(ani_env* env, ani_boolean isKeepScreenOn)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetKeepScreenOn(static_cast<bool>(isKeepScreenOn)));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window set keep screen on failed");
    }
}

void AniWindow::SetWaterMarkFlag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWaterMarkFlag(env, enable);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetWaterMarkFlag(ani_env* env, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (enable) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->AddWindowFlag(WindowFlag::WINDOW_FLAG_WATER_MARK));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_WATER_MARK));
    }
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "SetWaterMarkFlag failed.");
    }
}

ani_int AniWindow::GetSubWindowZLevel(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getSubWindowZLevel]msg: Window is nullptr");
        return ANI_ERROR;
    }
    return aniWindow->OnGetSubWindowZLevel(env);
}

ani_int AniWindow::OnGetSubWindowZLevel(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getSubWindowZLevel]msg: Window is nullptr");
        return ANI_ERROR;
    }
    int32_t zLevel = 0;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetSubWindowZLevel(zLevel));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][getSubWindowZLevel]");
        return ANI_ERROR;
    }
    return static_cast<ani_int>(zLevel);
}

ani_boolean AniWindow::IsFocused(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isFocused]msg: Window is nullptr");
        return ANI_ERROR;
    }
    return aniWindow->OnIsFocused(env);
}

ani_boolean AniWindow::OnIsFocused(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isFocused]msg: Window is nullptr");
        return ANI_ERROR;
    }
    bool isFocused = window->IsFocused();
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI] window isFocused=%{public}u", isFocused);
    return static_cast<ani_boolean>(isFocused);
}

void AniWindow::SetSubWindowZLevel(ani_env* env, ani_object obj, ani_long nativeObj, ani_int zLevel)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetSubWindowZLevel(env, zLevel);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowZLevel]msg: Window is nullptr");
    }
}

void AniWindow::OnSetSubWindowZLevel(ani_env* env, ani_int zLevel)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowZLevel]msg: Window is nullptr");
        return;
    }

    if (!WindowHelper::IsSubWindow(window->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "not allowed since window is not sub window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setSubWindowZLevel]msg: Not allowed since window is not sub window");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSubWindowZLevel(zLevel));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][setSubWindowZLevel]");
    }
}

void AniWindow::RaiseAboveTarget(ani_env* env, ani_object obj, ani_long nativeObj, ani_int windowId)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRaiseAboveTarget(env, windowId);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][raiseAboveTarget]msg: Window is nullptr");
    }
}

void AniWindow::OnRaiseAboveTarget(ani_env* env, ani_int windowId)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][raiseAboveTarget]msg: Window is nullptr");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RaiseAboveTarget(windowId));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][raiseAboveTarget]");
    }
}

void AniWindow::RaiseToAppTop(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRaiseToAppTop(env);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][raiseToAppTop]msg: Window is nullptr");
    }
}

void AniWindow::OnRaiseToAppTop(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][raiseToAppTop]msg: Window is nullptr");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RaiseToAppTop());
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][raiseToAppTop]");
    }
}

void AniWindow::SetTopmost(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isTopmost)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetTopmost(env, isTopmost);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setTopmost]msg:Window is nullptr");
    }
}

void AniWindow::OnSetTopmost(ani_env* env, ani_boolean isTopmost)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] OnSetTopmost permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][setTopmost]msg:setTopmost permission denied");
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setTopmost]msg:Window is nullptr");
        return;
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] OnSetTopmost fail, not main window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setTopmost]msg:setTopmost fail, not main window");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetTopmost(isTopmost));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][setTopmost]");
    }
}

void AniWindow::SetReceiveDragEventEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetReceiveDragEventEnabled(env, enabled);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetReceiveDragEventEnabled(ani_env* env, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetReceiveDragEventEnabled(enabled));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "SetReceiveDragEventEnabled failed.");
    }
}

ani_boolean AniWindow::IsReceiveDragEventEnabled(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return static_cast<ani_boolean>(true);
    }
    return static_cast<ani_boolean>(aniWindow->OnIsReceiveDragEventEnabled(env));
}

bool AniWindow::OnIsReceiveDragEventEnabled(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return true;
    }

    return windowToken_->IsReceiveDragEventEnabled();
}

void AniWindow::SetSeparationTouchEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetSeparationTouchEnabled(env, enabled);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetSeparationTouchEnabled(ani_env* env, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSeparationTouchEnabled(enabled));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "SetSeparationTouchEnabled failed.");
    }
}

ani_boolean AniWindow::IsSeparationTouchEnabled(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return static_cast<ani_boolean>(true);
    }
    return static_cast<ani_boolean>(aniWindow->OnIsSeparationTouchEnabled(env));
}

bool AniWindow::OnIsSeparationTouchEnabled(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return true;
    }

    return windowToken_->IsSeparationTouchEnabled();
}

void AniWindow::RaiseMainWindowAboveTarget(ani_env* env, ani_object obj, ani_long nativeObj, ani_int windowId)
{
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRaiseMainWindowAboveTarget(env, windowId);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][raiseMainWindowAboveTarget]msg: Window is nullptr");
    }
}

void AniWindow::OnRaiseMainWindowAboveTarget(ani_env* env, ani_int windowId)
{
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][raiseMainWindowAboveTarget]msg: Window is nullptr");
        return;
    }
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "permission denied, require system application");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][raiseMainWindowAboveTarget]msg: permission denied, require system application");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RaiseMainWindowAboveTarget(windowId));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][raiseMainWindowAboveTarget]");
        return;
    }
}

void AniWindow::SetMainWindowRaiseByClickEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetMainWindowRaiseByClickEnabled(env, enable);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setMainWindowRaiseByClickEnabled]msg: window is nullptr");
    }
}

void AniWindow::OnSetMainWindowRaiseByClickEnabled(ani_env* env, ani_boolean enable)
{
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setMainWindowRaiseByClickEnabled]msg: window is nullptr");
        return;
    }
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "permission denied, require system application");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][setMainWindowRaiseByClickEnabled]msg: permission denied");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetMainWindowRaiseByClickEnabled(enable));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][setMainWindowRaiseByClickEnabled]");
        return;
    }
}

void AniWindow::SetWindowTopmost(ani_env* env, ani_boolean isWindowTopmost)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTopmost]msg: Window is null");
        return;
    }
    if (!windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] device not support");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][setWindowTopmost]msg: device not support");
        return;
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] not allowed since window is not main window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setWindowTopmost]msg: not allowed since window is not main window");
        return;
    }
    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetMainWindowTopmost(isWindowTopmost));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] Window set main window topmost failed");
        AniWindowUtils::AniThrowError(env, ret,
            "[window][setWindowTopmost]");
        return;
    }
    TLOGI(WmsLogTag::WMS_HIERARCHY,
        "[ANI] id: %{public}u, name: %{public}s, isWindowTopmost: %{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), isWindowTopmost);
}

void AniWindow::SetWindowFocusable(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isFocusable)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowFocusable(env, isFocusable);
    } else {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowFocusable]msg:The window is not created or destroyed");
    }
}

void AniWindow::OnSetWindowFocusable(ani_env* env, ani_boolean isFocusable)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowFocusable]msg:The window is not created or destroyed");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetFocusable(isFocusable));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][setWindowFocusable]");
    }
}

void AniWindow::RequestFocus(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isFocused)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRequestFocus(env, isFocused);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][requestFocus]msg: Window is nullptr");
    }
}

void AniWindow::OnRequestFocus(ani_env* env, ani_boolean isFocused)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] OnRequestFocus permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][requestFocus]msg: RequestFocus permission denied!");
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][requestFocus]msg: Window is nullptr");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RequestFocusByClient(isFocused));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][requestFocus]");
    }
}

void AniWindow::SetSubWindowModal(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isModal)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetSubWindowModal(env, isModal);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowModal]msg: invalid window");
    }
}

void AniWindow::OnSetSubWindowModal(ani_env* env, ani_boolean isModal)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowModal]msg: invalid window");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSubWindowModal(isModal));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][setSubWindowModal]");
    }
}

void AniWindow::SetSubWindowModalType(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_boolean isModal, ani_int modalityType)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetSubWindowModalType(env, isModal, modalityType);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowModal]msg: invalid window");
    }
}

void AniWindow::OnSetSubWindowModalType(ani_env* env, ani_boolean isModal, ani_int modalityType)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowModal]msg: invalid window");
        return;
    }
    if (!isModal) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Normal subwindow not support modalityType");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSubWindowModal]msg: Normal subwindow not support modalityType");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSubWindowModal(isModal,
        static_cast<ModalityType>(modalityType)));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret,
            "[window][setSubWindowModal]");
    }
}

void AniWindow::LoadContent(ani_env* env, ani_object obj, ani_long nativeObj, ani_string path,
    ani_object storage)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnLoadContent(env, path, storage, false);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::LoadContentByName(ani_env* env, ani_object obj, ani_long nativeObj, ani_string path,
    ani_object storage)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnLoadContent(env, path, storage, true);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnLoadContent(ani_env* env, ani_string path, ani_object storage, bool isLoadByName)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string contentPath;
    AniWindowUtils::GetStdString(env, path, contentPath);
    WmErrorCode ret;
    if (isLoadByName) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->AniSetUIContentByName(contentPath, env, storage));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->AniSetUIContent(contentPath, env, storage));
    }
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window load content failed");
    }
}

void AniWindow::SetWindowSystemBarEnable(ani_env* env, ani_object obj, ani_long nativeObj, ani_object nameAry)
{
    TLOGI(WmsLogTag::WMS_MAIN, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowSystemBarEnable(env, nameAry);
    } else {
        TLOGE(WmsLogTag::WMS_MAIN, "[ANI] aniWindow is nullptr");
    }
    TLOGI(WmsLogTag::WMS_MAIN, "[ANI] SetWindowSystemBarEnable end");
}

void AniWindow::OnSetWindowSystemBarEnable(ani_env* env, ani_object nameAry)
{
    TLOGI(WmsLogTag::WMS_MAIN, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::vector<std::string> names;
    AniWindowUtils::GetStdStringVector(env, nameAry, names);
    std::map<WindowType, SystemBarProperty> systemBarProperties;
    std::map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    GetSystemBarStatus(systemBarProperties, systemBarPropertyFlags, names, window);
    UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags, window);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SetSystemBarPropertiesByFlags(systemBarPropertyFlags, systemBarProperties, window));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Set window system bar enable fail");
    }
}

bool AniWindow::GetSystemBarStatus(std::map<WindowType, SystemBarProperty>& systemBarProperties,
    std::map<WindowType, SystemBarPropertyFlag>& systemBarpropertyFlags,
    const std::vector<std::string>& names, sptr<Window>& window)
{
    auto statusProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto navProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    auto navIndicatorProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
    statusProperty.enable_ = false;
    navProperty.enable_ = false;
    navIndicatorProperty.enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = navIndicatorProperty;
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = SystemBarPropertyFlag();
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = SystemBarPropertyFlag();
    for (auto& name : names) {
        if (name.compare("status") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
            systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = true;
        } else if (name.compare("navigation") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
        }
    }
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableFlag = true;
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableFlag = true;
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enableFlag = true;
    return true;
}

void AniWindow::UpdateSystemBarProperties(std::map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags, sptr<Window> windowToken)
{
    for (auto it : systemBarPropertyFlags) {
        WindowType type = it.first;
        SystemBarPropertyFlag flag = it.second;
        auto property = windowToken->GetSystemBarPropertyByType(type);
        if (flag.enableFlag == false) {
            systemBarProperties[type].enable_ = property.enable_;
        }
        if (flag.backgroundColorFlag == false) {
            systemBarProperties[type].backgroundColor_ = property.backgroundColor_;
        }
        if (flag.contentColorFlag == false) {
            systemBarProperties[type].contentColor_ = property.contentColor_;
        }
        if (flag.enableAnimationFlag == false) {
            systemBarProperties[type].enableAnimation_ = property.enableAnimation_;
        }
        if (flag.enableFlag == true) {
            systemBarProperties[type].settingFlag_ =
                static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(property.settingFlag_) |
                static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING));
        }
        if (flag.backgroundColorFlag == true || flag.contentColorFlag == true) {
            systemBarProperties[type].settingFlag_ =
                static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(property.settingFlag_) |
                static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
        }
    }
}

WMError AniWindow::SetSystemBarPropertiesByFlags(std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags,
    std::map<WindowType, SystemBarProperty>& systemBarProperties, sptr<Window> windowToken)
{
    WMError ret = WMError::WM_OK;
    for (auto it : systemBarPropertyFlags) {
        WindowType type = it.first;
        SystemBarPropertyFlag flag = it.second;
        if (flag.enableFlag || flag.backgroundColorFlag || flag.contentColorFlag || flag.enableAnimationFlag) {
            ret = windowToken->UpdateSystemBarPropertyForPage(type, systemBarProperties.at(type), flag);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_IMMS, "[ANI] SetSystemBarProperty failed, ret = %{public}d", ret);
            }
        }
    }
    return ret;
}

void AniWindow::SetWindowTouchable(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isTouchable)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]Enter");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]aniWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetWindowTouchable(env, isTouchable);
}

void AniWindow::OnSetWindowTouchable(ani_env* env, ani_boolean isTouchable)
{
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]window is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(window->SetTouchable(static_cast<bool>(isTouchable)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]SetTouchable failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "Window set touchable on failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]Window [%{public}u, %{public}s]OnSetWindowTouchable end, isTouchable: "
        "%{public}d", window->GetWindowId(), window->GetWindowName().c_str(), isTouchable);
}

void AniWindow::SetDialogBackGestureEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    TLOGD(WmsLogTag::WMS_DIALOG, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetDialogBackGestureEnabled(env, enabled);
    } else {
        TLOGE(WmsLogTag::WMS_DIALOG, "[ANI]aniWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetDialogBackGestureEnabled(ani_env* env, ani_boolean enabled)
{
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[ANI]window is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(window->SetDialogBackGestureEnabled(static_cast<bool>(enabled)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[ANI]SetDialogBackGestureEnabled failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "SetDialogBackGestureEnabled failed!");
        return;
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "[ANI]Window [%{public}u, %{public}s] OnSetDialogBackGestureEnabled end, enabled: "
        "%{public}d", window->GetWindowId(), window->GetWindowName().c_str(), enabled);
}

void AniWindow::SetWindowMask(ani_env* env, ani_object obj, ani_long nativeObj, ani_array windowMask)
{
    TLOGD(WmsLogTag::WMS_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowMask(env, windowMask);
    } else {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]aniWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetWindowMask(ani_env* env, ani_array windowMaskArray)
{
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]window is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!CheckWindowMaskParams(env, windowMaskArray)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    std::vector<std::vector<uint32_t>> windowMask;
    if (!AniWindowUtils::ParseWindowMask(env, windowMaskArray, windowMask)) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]Parse windowMask value failed!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    if (!WindowHelper::IsSubWindow(window->GetType()) && !WindowHelper::IsAppFloatingWindow(window->GetType())) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING, "Invalid window type");
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(window->SetWindowMask(windowMask));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]SetWindowMask failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGI(WmsLogTag::WMS_PC, "[ANI]Window [%{public}u, %{public}s] OnSetWindowMask end",
        window->GetWindowId(), window->GetWindowName().c_str());
}

bool AniWindow::CheckWindowMaskParams(ani_env* env, ani_array windowMask)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]windowToken is nullptr!");
        return false;
    }
    ani_size length;
    ani_status aniRet = env->Array_GetLength(windowMask, &length);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]Get windowMask rows failed, ret: %{public}u", aniRet);
        return false;
    }
    uint32_t size = static_cast<uint32_t>(length);
    WindowLimits windowLimits;
    WmErrorCode ret = AniWindowUtils::ToErrorCode(windowToken_->GetWindowLimits(windowLimits));
    if (ret == WmErrorCode::WM_OK) {
        TLOGI(WmsLogTag::WMS_PC, "[ANI]windowMask rows: %{public}u, windowLimits.maxWidth_: %{public}u, "
            "windowLimits.maxHeight_: %{public}u", size, windowLimits.maxWidth_, windowLimits.maxHeight_);
        if (size == 0 || size > windowLimits.maxWidth_) {
            TLOGE(WmsLogTag::WMS_PC, "[ANI]Invalid windowMask size: %{public}u, vpRatio: %{public}f, maxWidth: "
                "%{public}u", size, windowLimits.vpRatio_, windowLimits.maxWidth_);
            return false;
        }
    } else {
        TLOGW(WmsLogTag::WMS_PC, "[ANI]Get windowLimits failed, errCode: %{public}d", ret);
        if (size == 0 || size > DEFAULT_WINDOW_MAX_WIDTH) {
            TLOGE(WmsLogTag::WMS_PC, "[ANI]Invalid windowMask size: %{public}u", size);
            return false;
        }
    }
    return true;
}

void AniWindow::SetTouchableAreas(ani_env* env, ani_object obj, ani_long nativeObj, ani_array rects)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetTouchableAreas(env, rects);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]aniWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetTouchableAreas(ani_env* env, ani_array rects)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]OnSetTouchableAreas permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]window is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    Rect windowRect = window->GetRect();
    std::vector<Rect> touchableAreas;
    WmErrorCode errCode = AniWindowUtils::ParseTouchableAreas(env, rects, windowRect, touchableAreas);
    if (errCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errCode);
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(window->SetTouchHotAreas(touchableAreas));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]SetTouchHotAreas failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "SetTouchableAreas failed!");
    }
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]Window [%{public}u, %{public}s] OnSetTouchableAreas end",
        window->GetWindowId(), window->GetWindowName().c_str());
}

ani_object AniWindow::GetUIContext(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    return aniWindow != nullptr ? aniWindow->OnGetUIContext(env) : nullptr;
}

ani_object AniWindow::OnGetUIContext(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto uicontent = window->GetUIContent();
    if (uicontent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] uicontent is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return uicontent->GetUIAniContext();
}

ani_object AniWindow::GetWindowAvoidArea(ani_env* env, ani_object obj, ani_long nativeObj, ani_int type)
{
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] type:%{public}d", static_cast<int32_t>(type));
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    return aniWindow != nullptr ? aniWindow->OnGetWindowAvoidArea(env, type) :
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
}

ani_object AniWindow::OnGetWindowAvoidArea(ani_env* env, ani_int type)
{
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] window is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    AvoidArea avoidArea;
    window->GetAvoidAreaByType(static_cast<AvoidAreaType>(type), avoidArea);
    return AniWindowUtils::CreateAniAvoidArea(env, avoidArea, static_cast<AvoidAreaType>(type));
}

ani_object AniWindow::GetWindowAvoidAreaIgnoringVisibility(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_int type)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}d", static_cast<int32_t>(type));
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    return aniWindow != nullptr ? aniWindow->OnGetWindowAvoidAreaIgnoringVisibility(env, type) : nullptr;
}

ani_object AniWindow::OnGetWindowAvoidAreaIgnoringVisibility(ani_env* env, ani_int type)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        window->GetAvoidAreaByTypeIgnoringVisibility(static_cast<AvoidAreaType>(type), avoidArea));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] get failed, ret: %{public}d", ret);
        return AniWindowUtils::AniThrowError(env, ret);
    }
    return AniWindowUtils::CreateAniAvoidArea(env, avoidArea, static_cast<AvoidAreaType>(type));
}

void DropWindowObjectByAni(ani_ref aniObj)
{
    auto obj = g_localObjs.find(reinterpret_cast<ani_ref>(aniObj));
    if (obj != g_localObjs.end()) {
        delete obj->second;
        g_localObjs.erase(obj);
    }
}

AniWindow* GetWindowObjectFromAni(void* aniObj)
{
    auto obj = g_localObjs.find(reinterpret_cast<ani_ref>(aniObj));
    if (obj == g_localObjs.end()) {
        return nullptr;
    }
    return obj->second;
}

ani_ref CreateAniWindowObject(ani_env* env, sptr<Window>& window)
__attribute__((no_sanitize("cfi")))
{
    if (env == nullptr || window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] invalid env or window");
        return nullptr;
    }
    std::string windowName = window->GetWindowName();
    // avoid repeatedly create ani window when getWindow
    ani_ref aniWindowObj = FindAniWindowObject(windowName);
    if (aniWindowObj != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI] FindAniWindowObject %{public}s", windowName.c_str());
        return aniWindowObj;
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] create window obj");
 
    ani_status ret;
    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env %{public}u", ret);
        return nullptr;
    }
    std::unique_ptr<AniWindow> aniWindow = std::make_unique<AniWindow>(window);
 
    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] get ctor fail %{public}u", ret);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] obj new fail %{public}u", ret);
        return nullptr;
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] get setNativeObj fail %{public}u", ret);
        return nullptr;
    }
    env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(aniWindow.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(obj, &ref) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create global ref fail");
        return nullptr;
    };
    aniWindow->SetAniRef(ref);
    g_localObjs.insert(std::pair(ref, aniWindow.release()));
    std::lock_guard<std::mutex> lock(g_aniWindowMap_mutex);
    g_aniWindowMap[windowName] = ref;
    return obj;
}

/** @note @window.layout */
void AniWindow::Resize(ani_env* env, ani_int width, ani_int height)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resize]msg: window is nullptr");
        return;
    }

    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }

    const uint32_t w = static_cast<uint32_t>(width);
    const uint32_t h = static_cast<uint32_t>(height);
    const WMError ret = windowToken_->Resize(w, h);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
            "[ANI] Resize failed, windowId: %{public}u, width: %{public}u, height: %{public}u, ret: %{public}d",
            windowId, w, h, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] Resize success, windowId: %{public}u, width: %{public}u, height: %{public}u",
        windowId, w, h);
    return;
}

/** @note @window.layout */
void AniWindow::MoveWindowTo(ani_env* env, ani_int x, ani_int y)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowTo]msg: window is nullptr");
        return;
    }

    const int32_t targetX = static_cast<int32_t>(x);
    const int32_t targetY = static_cast<int32_t>(y);
    const WMError ret = windowToken_->MoveTo(targetX, targetY);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
            "[ANI] MoveWindowTo failed, windowId: %{public}u, x: %{public}d, y: %{public}d, ret: %{public}d",
            windowId, targetX, targetY, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] MoveWindowTo success, windowId: %{public}u, x: %{public}d, y: %{public}d",
        windowId, targetX, targetY);
    return;
}

/** @note @window.layout */
ani_object AniWindow::GetGlobalRect(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getGlobalRect]msg: window is nullptr");
    }

    Rect globalScaledRect = Rect::EMPTY_RECT;
    const WMError ret = windowToken_->GetGlobalScaledRect(globalScaledRect);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] GetGlobalRect failed, windowId: %{public}u, ret: %{public}d",
            windowId, static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, errorCode);
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] Window [%{public}u, %{public}s], globalScaledRect: %{public}s",
        windowId, windowToken_->GetWindowName().c_str(), globalScaledRect.ToString().c_str());
    return AniWindowUtils::CreateAniRect(env, globalScaledRect);
}

ani_int AniWindow::GetWindowDecorHeight(ani_env* env)
{
    int32_t height { 0 };
    wptr<Window> weakToken(windowToken_);
    auto window = weakToken.promote();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    WMError ret = windowToken_->GetDecorHeight(height);
    if (ret != WMError::WM_OK) {
        if (ret == WMError::WM_ERROR_DEVICE_NOT_SUPPORT) {
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
            return ANI_ERROR;
        }
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return 0;
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] Window [%{public}u, %{public}s] OnGetDecorHeight end, height = %{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), height);
    return static_cast<ani_double>(height);
}

ani_object AniWindow::SetWindowBackgroundColor(ani_env* env, const std::string& color)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBackgroundColor(color));
    if (ret == WmErrorCode::WM_OK) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s] set background color end",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
        return AniWindowUtils::CreateAniUndefined(env);
    } else {
        return AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindow::RegisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    ani_long timeout = 0;
    if (aniWindow != nullptr) {
        aniWindow->OnRegisterWindowCallback(env, type, callback, timeout);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
}

void AniWindow::RegisterNoInteractionDetectedCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_long timeout, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnRegisterWindowCallback(env, type, callback, timeout);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]aniWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
}

void AniWindow::OnRegisterWindowCallback(ani_env* env, ani_string type, ani_ref callback, ani_long timeout)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}s", cbType.c_str());
    WmErrorCode ret = registerManager_->RegisterListener(window, cbType, CaseType::CASE_WINDOW, env, callback, timeout);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindow::UnregisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnUnregisterWindowCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
}

void AniWindow::OnUnregisterWindowCallback(ani_env* env, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}s", cbType.c_str());
    WmErrorCode ret = registerManager_->UnregisterListener(window, cbType, CaseType::CASE_WINDOW, env, callback);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindow::ShowWindow(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnShowWindow(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnShowWindow(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (WindowHelper::IsMainWindowAndNotShown(window->GetType(), window->GetWindowState())) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "window Type %{public}u and window state %{public}u is not supported, [%{public}u, %{public}s]",
            static_cast<uint32_t>(window->GetType()), static_cast<uint32_t>(window->GetWindowState()),
            window->GetWindowId(), window->GetWindowName().c_str());
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Show(0, false, true));
    TLOGI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] show with ret=%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window show failed");
        return;
    }
}

void AniWindow::ShowWindowWithOptions(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_object aniShowWindowOptions)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnShowWindowWithOptions(env, aniShowWindowOptions);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnShowWindowWithOptions(ani_env* env, ani_object aniShowWindowOptions)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] ShowWindowWithOptions in");
    auto window = GetWindow();

    bool focusOnShow = true;
    ani_ref aniFocusOnShow;
    env->Object_GetPropertyByName_Ref(aniShowWindowOptions, "focusOnShow", &aniFocusOnShow);
    ani_boolean isFocusOnShowUndefined;
    env->Reference_IsUndefined(aniFocusOnShow, &isFocusOnShowUndefined);
    if (!isFocusOnShowUndefined) {
        ani_boolean isFocusOnShow;
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(aniFocusOnShow),
            "toBoolean", ":z", &isFocusOnShow);
        focusOnShow = static_cast<bool>(isFocusOnShow);
    }

    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (WindowHelper::IsMainWindowAndNotShown(window->GetType(), window->GetWindowState())) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "window Type %{public}u and window state %{public}u is not supported, [%{public}u, %{public}s]",
            static_cast<uint32_t>(window->GetType()), static_cast<uint32_t>(window->GetWindowState()),
            window->GetWindowId(), window->GetWindowName().c_str());
        return;
    }
    if (focusOnShow == false &&
        (WindowHelper::IsModalSubWindow(window->GetType(), window->GetWindowFlags()) ||
        WindowHelper::IsDialogWindow(window->GetType()))) {
        TLOGNE(WmsLogTag::WMS_FOCUS, "only normal sub window supports setting focusOnShow");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    window->SetShowWithOptions(true);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Show(0, false, focusOnShow, true));
    TLOGI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] show with ret=%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window show failed");
        return;
    }
}

void AniWindow::SetWindowTitle(ani_env* env, ani_object obj, ani_long nativeObj, ani_string titleName)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowTitle(env, titleName);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetWindowTitle(ani_env* env, ani_string titleName)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string name;
    ani_status ret = OHOS::Rosen::AniWindowUtils::GetStdString(env, titleName, name);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] invalid param of name");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode errorCode = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowTitle(name));
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] show with ret=%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), errorCode);
    if (errorCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errorCode, "[ANI] set window title failed");
        return;
    }
}

void AniWindow::SetTitleButtonVisible(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isMaximizeVisible,
    ani_boolean isMinimizeVisible, ani_boolean isSplitVisible, ani_boolean isCloseVisible)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetTitleButtonVisible(env, isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetTitleButtonVisible(ani_env* env, ani_boolean isMaximizeVisible,
    ani_boolean isMinimizeVisible, ani_boolean isSplitVisible, ani_boolean isCloseVisible)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_DECOR, "set title button visible permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetTitleButtonVisible(isMaximizeVisible,
        isMinimizeVisible, isSplitVisible, isCloseVisible));
    TLOGI(WmsLogTag::WMS_DECOR,
        "Window [%{public}u, %{public}s] end [%{public}d, %{public}d, %{public}d, %{public}d]",
        window->GetWindowId(), window->GetWindowName().c_str(), isMaximizeVisible, isMinimizeVisible,
        isSplitVisible, isCloseVisible);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[ANI] set title button visible failed!");
        return;
    }
}

void AniWindow::SetWindowTitleMoveEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetWindowTitleMoveEnabled(env, enable);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnSetWindowTitleMoveEnabled(ani_env* env, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowTitleMoveEnabled(enable));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[ANI] Window set title move enable failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] end",
        window->GetWindowId(), window->GetWindowName().c_str());
}

ani_object AniWindow::GetTitleButtonRect(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnGetTitleButtonRect(env);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

ani_object AniWindow::OnGetTitleButtonRect(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    TitleButtonRect titleButtonRect;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->GetTitleButtonArea(titleButtonRect));
    if (ret != WmErrorCode::WM_OK) {
        return AniWindowUtils::AniThrowError(env, ret, "[ANI] get title button area failed");
    }
    return AniWindowUtils::CreateAniTitleButtonRect(env, titleButtonRect);
}

ani_object AniWindow::GetDecorButtonStyle(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnGetDecorButtonStyle(env);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

ani_object AniWindow::OnGetDecorButtonStyle(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    DecorButtonStyle decorButtonStyle;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->GetDecorButtonStyle(decorButtonStyle));
    if (ret != WmErrorCode::WM_OK) {
        return AniWindowUtils::AniThrowError(env, ret, "[ANI] decorButtonStyle format failed");
    }
    return AniWindowUtils::CreateAniDecorButtonStyle(env, decorButtonStyle);
}

void AniWindow::SetTitleAndDockHoverShown(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_object isTitleHoverShown, ani_object isDockHoverShown)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetTitleAndDockHoverShown(env, isTitleHoverShown, isDockHoverShown);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetTitleAndDockHoverShown(ani_env* env, ani_object isTitleHoverShown, ani_object isDockHoverShown)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    bool isTitleHoverShownValue = true;
    ani_status aniRet = AniWindowUtils::GetBooleanObject(env, isTitleHoverShown, isTitleHoverShownValue);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] invalid param of isTitleHoverShown");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    bool isDockHoverShownValue = true;
    aniRet = AniWindowUtils::GetBooleanObject(env, isDockHoverShown, isDockHoverShownValue);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] invalid param of isDockHoverShown");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WMError errCode = window->SetTitleAndDockHoverShown(isTitleHoverShownValue, isDockHoverShownValue);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[ANI] set title button visible failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "window [%{public}u, %{public}s] [%{public}d, %{public}d]",
        window->GetWindowId(), window->GetWindowName().c_str(), isTitleHoverShownValue, isDockHoverShownValue);
}

void AniWindow::SetHandwritingFlag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnSetHandwritingFlag(env, enable);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

void AniWindow::OnSetHandwritingFlag(ani_env* env, ani_boolean enable)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WMError ret = enable ? window->AddWindowFlag(WindowFlag::WINDOW_FLAG_HANDWRITING) :
        window->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_HANDWRITING);
    WmErrorCode errCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
    if (errCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errCode, "[ANI] SetHandwritingFlag failed.");
        return;
    }
    TLOGI(WmsLogTag::DEFAULT, "Window [%{public}u, %{public}s] set handwriting flag on end",
        window->GetWindowId(), window->GetWindowName().c_str());
}

ani_boolean AniWindow::GetWindowDecorVisible(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnGetWindowDecorVisible(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ani_boolean(false);
    }
}

ani_boolean AniWindow::OnGetWindowDecorVisible(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ani_boolean(false);
    }
    bool isVisible = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->GetDecorVisible(isVisible));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[ANI] Get window decor visibility failed");
        return ani_boolean(false);
    }
    TLOGI(WmsLogTag::WMS_DECOR, "end, window [%{public}u, %{public}s] isVisible=%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), isVisible);
    return ani_boolean(isVisible);
}

void AniWindow::SetDecorButtonStyle(ani_env* env, ani_object decorStyle)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] WindowToken is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    DecorButtonStyle decorButtonStyle;
    WMError res = windowToken_->GetDecorButtonStyle(decorButtonStyle);
    if (res != WMError::WM_OK) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    if (!AniWindowUtils::SetDecorButtonStyleFromAni(env, decorButtonStyle, decorStyle)) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] Argc is invalid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI] param [%{public}d, %{public}d, %{public}d, %{public}d] to be updated",
        decorButtonStyle.colorMode, decorButtonStyle.spacingBetweenButtons,
        decorButtonStyle.closeButtonRightMargin, decorButtonStyle.buttonBackgroundSize);
    if (!WindowHelper::CheckButtonStyleValid(decorButtonStyle)) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] out of range params");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WMError errCode = windowToken_->SetDecorButtonStyle(decorButtonStyle);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] set decor button fail");
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindow::SetWindowTitleButtonVisible(ani_env* env, ani_object visibleParam)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] WindowToken is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    ani_boolean isMaximizeButtonVisible = false;
    if (ANI_OK != env->Object_GetPropertyByName_Boolean(visibleParam,
        "isMaximizeButtonVisible", &isMaximizeButtonVisible)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ani_boolean isMinimizeButtonVisible = false;
    if (ANI_OK != env->Object_GetPropertyByName_Boolean(visibleParam,
        "isMinimizeButtonVisible", &isMinimizeButtonVisible)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    ani_boolean isCloseButtonVisible = false;
    if (ANI_OK != env->Object_GetPropertyByName_Boolean(visibleParam,
        "isCloseButtonVisible", &isCloseButtonVisible)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI] Window [%{public}u, %{public}s] [%{public}d, %{public}d, %{public}d]",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        isMaximizeButtonVisible, isMinimizeButtonVisible, isCloseButtonVisible);
    WMError errCode = windowToken_->SetTitleButtonVisible(isMaximizeButtonVisible, isMinimizeButtonVisible,
        isMaximizeButtonVisible, isCloseButtonVisible);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] set title button visible failed!");
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindow::DestroyWindow(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnDestroyWindow(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnDestroyWindow(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (WindowHelper::IsMainWindow(window->GetType())) {
        TLOGW(WmsLogTag::WMS_LIFE, "window Type %{public}u is not supported, [%{public}u, %{public}s]",
            static_cast<uint32_t>(window->GetType()),
            window->GetWindowId(), window->GetWindowName().c_str());
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Destroy());
    TLOGI(WmsLogTag::WMS_LIFE, "window [%{public}u, %{public}s] ret=%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window destroy failed");
        return;
    }
    windowToken_ = nullptr;
}

ani_boolean AniWindow::IsWindowShowing(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnIsWindowShowing(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
        return ani_boolean(false);
    }
}

ani_boolean AniWindow::OnIsWindowShowing(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ani_boolean(false);
    }
    return ani_boolean(window->GetWindowState() == WindowState::STATE_SHOWN);
}

ani_boolean AniWindow::IsWindowHighlighted(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnIsWindowHighlighted(env);
    } else {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowHighlighted]msg: WindowToken is nullptr");
        return ani_boolean(false);
    }
}

ani_boolean AniWindow::OnIsWindowHighlighted(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowHighlighted]msg: WindowToken is nullptr");
        return ani_boolean(false);
    }
    bool isHighlighted = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->IsWindowHighlighted(isHighlighted));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "get window highlight failed, ret: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret,
            "[window][isWindowHighlighted]");
        return ani_boolean(false);
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "get window highlight end, isHighlighted: %{public}u", isHighlighted);
    return ani_boolean(isHighlighted);
}

void AniWindow::HideWithAnimation(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnHideWithAnimation(env);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnHideWithAnimation(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto winType = window->GetType();
    if (!WindowHelper::IsSystemWindow(winType)) {
        TLOGE(WmsLogTag::WMS_ANIMATION,
            "window Type %{public}u is not supported", static_cast<uint32_t>(winType));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Hide(0, true, false));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window show failed");
        return;
    }
}

void AniWindow::ShowWithAnimation(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnShowWithAnimation(env);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnShowWithAnimation(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto winType = window->GetType();
    if (!WindowHelper::IsSystemWindow(winType)) {
        TLOGE(WmsLogTag::WMS_ANIMATION,
            "window Type %{public}u is not supported", static_cast<uint32_t>(winType));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Show(0, true, true));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window show failed");
        return;
    }
}

ani_ref AniWindow::GetParentWindow(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] WindowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    sptr<Window> parentWindow = nullptr;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetParentWindow(parentWindow));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] get failed, result=%{public}d", ret);
        return AniWindowUtils::AniThrowError(env, ret);
    }
    if (parentWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] parentWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARENT);
    }
    return CreateAniWindowObject(env, parentWindow);
}

void AniWindow::SetParentWindow(ani_env* env, ani_int windowId)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] WindowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    int32_t newParentWindowId = static_cast<int32_t>(windowId);
    WMError ret = windowToken_->SetParentWindow(newParentWindowId);
    if (ret != WMError::WM_OK) {
        WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] Set parent window failed");
        AniWindowUtils::AniThrowError(env, wmErrorCode);
    } else {
        TLOGI(WmsLogTag::WMS_SUB, "[ANI] windowId: %{public}u set parent window id: %{public}u end",
            windowToken_->GetWindowId(), newParentWindowId);
    }
}

static sptr<IRemoteObject> GetBindDialogToken(ani_env* env, ani_object argv)
{
    sptr<IRemoteObject> token = AniGetNativeRemoteObject(env, argv);
    if (token != nullptr) {
        return token;
    }
    std::shared_ptr<AbilityRuntime::RequestInfo> requestInfo =
        AbilityRuntime::RequestInfo::UnwrapRequestInfo(env, argv);
    return (requestInfo != nullptr) ? requestInfo->GetToken() : nullptr;
}

void AniWindow::BindDialogTarget(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_object argv, ani_ref deathCallback)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnBindDialogTarget(env, argv, deathCallback);
    } else {
        TLOGE(WmsLogTag::WMS_DIALOG, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnBindDialogTarget(ani_env* env, ani_object argv, ani_ref deathCallback)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[ANI] window is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "permission denied, require system application!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }

    sptr<IRemoteObject> token = GetBindDialogToken(env, argv);
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "token is null!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }

    registerManager_->RegisterListener(windowToken_, "dialogDeathRecipient",
        CaseType::CASE_WINDOW, env, deathCallback, 0.0);
    wptr<Window> weakToken(windowToken_);
    auto window = weakToken.promote();
    if (window == nullptr) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->BindDialogTarget(token));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[ANI] bind dialog target failed, result=%{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "Bind Dialog Target failed");
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "BindDialogTarget end, window [%{public}u, %{public}s]",
        window->GetWindowId(), window->GetWindowName().c_str());
}

ani_object AniWindow::GetTransitionController(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] In");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] AniWindow is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->OnGetTransitionController(env, obj);
}

ani_object AniWindow::OnGetTransitionController(ani_env* env, ani_object obj)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Not system app, permission denied");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Window is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Unexpected window type: %{public}d", windowToken_->GetType());
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    if (aniTransControllerObj_ == nullptr) {
        WmErrorCode ret = CreateTransitionController(env, obj);
        if (ret != WmErrorCode::WM_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "CreateTransitionController failed, error code: %{public}d", ret);
            return AniWindowUtils::AniThrowError(env, ret);
        }
    }
    return aniTransControllerObj_;
}

WmErrorCode AniWindow::CreateTransitionController(ani_env* env, ani_object obj)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "windowToken_ is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }

    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Unexpected window type: %{public}d", windowToken_->GetType());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }

    auto windowName = windowToken_->GetWindowName();
    ani_ref aniWindowObj = GetAniRef();
    if (aniWindowObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Find ani window %{public}s object failed", windowName.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }

    sptr<AniTransitionController> transController = AniTransitionController::CreateAniTransitionController(env,
        aniWindowObj, windowToken_);
    if (transController == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "transition controller create failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->RegisterAnimationTransitionController(transController));
    aniTransControllerObj_ = transController->GetAniTransControllerObj();
    TLOGI(WmsLogTag::WMS_ANIMATION, "Transition controller create success for window %{public}s", windowName.c_str());
    return ret;
}

void AniWindow::KeepKeyboardOnFocus(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean keepKeyboardFlag)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "[ANI]In");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        aniWindow->OnKeepKeyboardOnFocus(env, keepKeyboardFlag);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnKeepKeyboardOnFocus(ani_env* env, ani_boolean keepKeyboardFlag)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WindowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][keepKeyboardOnFocus]msg:WindowToken_ is nullptr");
        return;
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "not allowed since window is not system window or app subwindow");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][keepKeyboardOnFocus]msg:not system window or app subwindow");
        return;
    }

    WmErrorCode ret = windowToken_->KeepKeyboardOnFocus(keepKeyboardFlag);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "failed");
        AniWindowUtils::AniThrowError(env, ret,
            "[window][keepKeyboardOnFocus]");
    }
    TLOGE(WmsLogTag::WMS_KEYBOARD, "end, window [%{public}u, %{public}s] keepKeyboardFlag=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), keepKeyboardFlag);
}

ani_object AniWindow::SetImmersiveModeEnabledState(ani_env* env, bool enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "WindowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] OnSetImmersiveModeEnabledState is not allowed since invalid window type");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    if (windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] device not support");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] OnSetImmersiveModeEnabledState to %{public}d", static_cast<int32_t>(enable));
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetImmersiveModeEnabledState(static_cast<bool>(enable)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] Window immersive mode set enabled failed, ret = %{public}d", ret);
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }

    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] window [%{public}u, %{public}s] OnSetImmersiveModeEnabledState end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return AniWindowUtils::CreateAniUndefined(env);
}

ani_object AniWindow::SetWindowDecorVisible(ani_env* env, bool isVisible)
{
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDecorVisible(static_cast<bool>(isVisible)));
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] OnSetWindowDecorVisible to %{public}d", static_cast<int32_t>(isVisible));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Window decor set visible failed");
        return AniWindowUtils::AniThrowError(env, ret);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] Window [%{public}u, %{public}s] OnSetWindowDecorVisible end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return AniWindowUtils::CreateAniUndefined(env);
}

ani_object AniWindow::SetWindowDecorHeight(ani_env* env, ani_int height)
{
    if (height < MIN_DECOR_HEIGHT || height > MAX_DECOR_HEIGHT) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] height should greater than 37 or smaller than 112");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDecorHeight(static_cast<int32_t>(height)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Set window decor height failed");
        return AniWindowUtils::AniThrowError(env, ret);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] Window [%{public}u, %{public}s] OnSetDecorHeight end, height = %{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), static_cast<int32_t>(height));
    return AniWindowUtils::CreateAniUndefined(env);
}

ani_object AniWindow::GetWindowPropertiesSync(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WindowPropertyInfo windowPropertyInfo;
    WMError ret = windowToken_->GetWindowPropertyInfo(windowPropertyInfo);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get window properties failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto objValue = AniWindowUtils::CreateWindowsProperties(env, windowPropertyInfo);
    if (objValue == nullptr) {
        return AniWindowUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR, "[ANI] Window get properties failed");
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] Window [%{public}u, %{public}s] get properties end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return objValue;
}

ani_boolean AniWindow::IsWindowSupportWideGamut(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    ani_boolean res = static_cast<ani_boolean>(windowToken_->IsSupportWideGamut());
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] Window IsWindowSupportWideGamut end");
    return res;
}

ani_object AniWindow::SetWindowLayoutFullScreen(ani_env* env, ani_boolean isLayoutFullScreen)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] device not support");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->SetLayoutFullScreen(static_cast<bool>(isLayoutFullScreen)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] fullscreen set error");
        return AniWindowUtils::AniThrowError(env, ret);
    }
    return 0;
}

ani_object AniWindow::SetRaiseByClickEnabled(ani_env* env, ani_boolean enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setRaiseByClickEnabled]msg:  windowToken_ is nullptr");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetRaiseByClickEnabled(static_cast<bool>(enable)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] SetRaiseByClickEnabled set error");
        return AniWindowUtils::AniThrowError(env, ret,
            "[window][setRaiseByClickEnabled]");
    }
    return 0;
}

ani_object AniWindow::SetExclusivelyHighlighted(ani_env* env, ani_boolean exclusivelyHighlighted)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] windowToken_ is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setExclusivelyHighlighted]msg: windowToken_ is null");
    }
    WmErrorCode ret =
        WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetExclusivelyHighlighted(static_cast<bool>(exclusivelyHighlighted)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] Set exclusively highlighted failed");
        return AniWindowUtils::AniThrowError(env, ret,
            "[window][setExclusivelyHighlighted]");
    }
    return 0;
}

void AniWindow::SetSystemBarProperties(ani_env* env, ani_object aniSystemBarProperties)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::map<WindowType, SystemBarProperty> aniProperties;
    std::map<WindowType, SystemBarPropertyFlag> aniSystemBarPropertyFlags;

    if (!AniWindowUtils::SetSystemBarPropertiesFromAni(env, aniProperties, aniSystemBarPropertyFlags,
        aniSystemBarProperties, windowToken_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] Failed to convert parameter to systemBarProperties");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }

    std::map<WindowType, SystemBarProperty> systemBarProperties;
    std::map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    AniWindowUtils::GetSystemBarPropertiesFromAni(windowToken_, aniProperties, aniSystemBarPropertyFlags,
        systemBarProperties, systemBarPropertyFlags);
    AniWindowUtils::UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags, windowToken_);
    WMError ret = SetSystemBarPropertiesByFlags(
        systemBarPropertyFlags, systemBarProperties, windowToken_);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] Window SetSystemBarProperties failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "[ANI] Window SetSystemBarProperties failed");
        return;
    }

    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] Succeed in setting systemBarProperties");
}

ani_object AniWindow::SetSpecificSystemBarEnabled(ani_env* env, ani_string name, ani_boolean enable,
    ani_object enableAnimation)
{
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    WindowType type = WindowType::WINDOW_TYPE_STATUS_BAR;
    SystemBarProperty property;
    SystemBarPropertyFlag propertyFlag;
    if (!AniWindowUtils::GetSpecificBarStatus(env, name, enable, enableAnimation, type, property, propertyFlag)) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] invalid param or argc");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto systemBarProperty = windowToken_->GetSystemBarPropertyByType(type);
    systemBarProperty.enable_ = propertyFlag.enableFlag ? property.enable_ : systemBarProperty.enable_;
    systemBarProperty.enableAnimation_ =
        propertyFlag.enableAnimationFlag ? property.enableAnimation_ : systemBarProperty.enableAnimation_;
    systemBarProperty.settingFlag_ |= SystemBarSettingFlag::ENABLE_SETTING;
    auto err = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->UpdateSystemBarPropertyForPage(type, systemBarProperty, propertyFlag));
    if (err == WmErrorCode::WM_OK) {
        return AniWindowUtils::CreateAniUndefined(env);
    }
    TLOGE(WmsLogTag::WMS_IMMS, "SetSpecificSystemBarEnabled failed, ret = %{public}d", err);
    return AniWindowUtils::AniThrowError(env, err);
}

ani_ref FindAniWindowObject(const std::string& windowName)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] Try to find window %{public}s in g_aniWindowMap", windowName.c_str());
    std::lock_guard<std::mutex> lock(g_aniWindowMap_mutex);
    if (g_aniWindowMap.find(windowName) == g_aniWindowMap.end()) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI] Can not find window %{public}s in g_aniWindowMap", windowName.c_str());
        return nullptr;
    }
    return g_aniWindowMap[windowName];
}

ani_object AniWindow::SetDragKeyFramePolicy(ani_env* env, ani_object aniKeyFramePolicy)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    KeyFramePolicy keyFramePolicy;
    if (!AniWindowUtils::ParseKeyFramePolicy(env, aniKeyFramePolicy, keyFramePolicy)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed to convert parameter to keyFramePolicy");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
    }

    const WMError ret = windowToken_->SetDragKeyFramePolicy(keyFramePolicy);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowId, static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, errorCode);
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC,
        "[ANI] success, windowId: %{public}u, keyFramePolicy: %{public}s",
        windowToken_->GetWindowId(), keyFramePolicy.ToString().c_str());
    return AniWindowUtils::CreateKeyFramePolicy(env, keyFramePolicy);
}

ani_object AniWindow::Snapshot(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::shared_ptr<Media::PixelMap> pixelMap = windowToken_->Snapshot();
    if (pixelMap == nullptr) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto nativePixelMap = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap);
    if (nativePixelMap == nullptr) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s], WxH=%{public}dx%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        pixelMap->GetWidth(), pixelMap->GetHeight());
    return nativePixelMap;
}

ani_object AniWindow::SnapshotSync(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    auto retCode = windowToken_->Snapshot(pixelMap);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    if (ret != WmErrorCode::WM_OK || pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] winId: %{public}u snapshot end, retCode: %{public}d",
            windowToken_->GetWindowId(), static_cast<int32_t>(retCode));
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto nativePixelMap = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap);
    if (nativePixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] create ets pixel map error, winId: %{public}u",
            windowToken_->GetWindowId());
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s], WxH=%{public}dx%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        pixelMap->GetWidth(), pixelMap->GetHeight());
    return nativePixelMap;
}

void AniWindow::HideNonSystemFloatingWindows(ani_env* env, ani_boolean shouldHide)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (windowToken_->IsFloatingWindowAppType()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is app floating window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "HideNonSystemFloatingWindows is not allowed since window is app floating window");
        return;
    }
    WMError ret = windowToken_->HideNonSystemFloatingWindows(shouldHide);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
        AniWindowUtils::AniThrowError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret),
            "Hide non-system floating windows failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE,
        "end. Window [%{public}u, %{public}s]",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
}

void AniWindow::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        auto window = aniWindow->GetWindow();
        std::lock_guard<std::mutex> lock(g_aniWindowMap_mutex);
        if (window != nullptr) {
            g_aniWindowMap.erase(window->GetWindowName());
        }
        DropWindowObjectByAni(aniWindow->GetAniRef());
        env->GlobalReference_Delete(aniWindow->GetAniRef());
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::SetWindowBrightness(ani_env* env, ani_double brightness)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBrightness(static_cast<float>(brightness)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
        AniWindowUtils::AniThrowError(env, ret, "Window set brightness failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "window [%{public}u, %{public}s] set brightness end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
}

ani_int AniWindow::GetWindowColorSpace(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    ColorSpace colorSpace = windowToken_->GetColorSpace();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "window [%{public}u, %{public}s] colorSpace=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));
    return static_cast<ani_int>(colorSpace);
}

void AniWindow::SetWakeUpScreen(ani_env* env, ani_boolean wakeUp)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set wake up screen permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTurnScreenOn(wakeUp));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set wake up screen failed, code: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "window [%{public}u, %{public}s] set wake up screen %{public}d end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), wakeUp);
}

void AniWindow::SetSnapshotSkip(ani_env* env, ani_boolean isSkip)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetSnapshotSkip(static_cast<bool>(isSkip)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set snapshot skip failed, code: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "Window SetSnapshotSkip failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "window [%{public}u, %{public}s] SetSnapshotSkip end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
}

ani_object AniWindow::SnapshotIgnorePrivacy(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SnapshotIgnorePrivacy(pixelMap));
    if (ret == WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "device not support");
        return AniWindowUtils::AniThrowError(env, ret);
    } else if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get pixelmap failed, code: %{public}d", ret);
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "pixelMap is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto nativePixelMap = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap);
    if (nativePixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "native pixelMap is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "windowId:%{public}u, WxH=%{public}dx%{public}d",
        windowToken_->GetWindowId(), pixelMap->GetWidth(), pixelMap->GetHeight());
    return nativePixelMap;
}

ani_object AniWindow::GetStatusBarProperty(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only main window is allowed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    SystemBarProperty statusBarProperty = windowToken_->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    return AniWindowUtils::CreateAniStatusBarProperty(env, statusBarProperty);
}

void AniWindow::SetStatusBarColor(ani_env* env, ani_long color)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto property = windowToken_->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    property.contentColor_ = static_cast<uint32_t>(color);
    property.settingFlag_ = static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(property.settingFlag_) |
        static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
    SystemBarPropertyFlag flag = { false, false, true, false };
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, property, flag));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "set status bar property error: %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "SetStatusBarProperty failed");
        return;
    }
}

ani_boolean AniWindow::IsSystemAvoidAreaEnabled(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only system window is valid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return false;
    }
    uint32_t avoidAreaOption = 0;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetAvoidAreaOption(avoidAreaOption));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return false;
    }
    bool enabled = avoidAreaOption & static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW);
    return static_cast<ani_boolean>(enabled);
}

void AniWindow::SetSystemAvoidAreaEnabled(ani_env* env, ani_boolean enabled)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only system window is valid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    uint32_t option = 0;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetAvoidAreaOption(option));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    if (enabled) {
        option |= static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW);
    } else {
        option &= ~static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW);
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetAvoidAreaOption(option));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "set system avoid area enabled failed.");
        return;
    }
}

ani_object AniWindow::GetWindowDensityInfo(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WindowDensityInfo densityInfo;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowDensityInfo(densityInfo));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get failed, result=%{public}d", ret);
        return AniWindowUtils::AniThrowError(env, ret);
    }
    return AniWindowUtils::CreateAniWindowDensityInfo(env, densityInfo);
}

ani_boolean AniWindow::GetImmersiveModeEnabledState(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "not allowed since invalid window type");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return false;
    }
    bool isEnabled = windowToken_->GetImmersiveModeEnabledState();
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isEnabled %{public}u get end", windowToken_->GetWindowId(), isEnabled);
    return static_cast<ani_boolean>(isEnabled);
}

void AniWindow::SetWindowGrayScale(ani_env* env, ani_double grayScale)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    constexpr double eps = 1e-6;
    if (grayScale < (MIN_GRAY_SCALE - eps) || grayScale > (MAX_GRAY_SCALE + eps)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE,
            "grayScale should be greater than or equal to 0.0, and should be smaller than or equal to 1.0");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetGrayScale(static_cast<float>(grayScale)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Set window gray scale failed %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "Set window gray scale failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "end, window [%{public}u, %{public}s] grayScale=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), grayScale);
}

ani_object AniWindow::GetWindowSystemBarProperties(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only main window is allowed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    SystemBarProperty status = windowToken_->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty navi = windowToken_->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    return AniWindowUtils::CreateAniWindowSystemBarProperties(env, status, navi);
}

ani_boolean AniWindow::IsGestureBackEnabled(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed since invalid window type");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return false;
    }
    bool enabled = true;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetGestureBackEnabled(enabled));
    if (ret == WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT) {
        TLOGE(WmsLogTag::WMS_IMMS, "device is not support");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        return false;
    } else if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        return false;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u, %{public}s] enable %{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), enabled);
    return enabled;
}

void AniWindow::SetGestureBackEnabled(ani_env* env, ani_boolean enabled)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid window type");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetGestureBackEnabled(enabled));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "set failed ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "set failed.");
        return;
    }
}

void AniWindow::SetSingleFrameComposerEnabled(ani_env* env, ani_boolean enabled)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set single frame composer enabled permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken_ is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetSingleFrameComposerEnabled(enabled));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Set single frame composer enabled failed, ret is %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "end, window [%{public}u, %{public}s] enabled flag=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), enabled);
}

void AniWindow::SetContentAspectRatio(ani_env* env, ani_object obj, ani_long nativeObj,
                                      ani_double ratio, ani_boolean isPersistent, ani_boolean needUpdateRect)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetContentAspectRatio(env, ratio, isPersistent, needUpdateRect);
}

void AniWindow::OnSetContentAspectRatio(
    ani_env* env, ani_double ratio, ani_boolean isPersistent, ani_boolean needUpdateRect)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WMError ret = windowToken_->SetContentAspectRatio(
        static_cast<float>(ratio), static_cast<bool>(isPersistent), static_cast<bool>(needUpdateRect));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowToken_->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, AniWindowUtils::ToErrorCode(ret));
        return;
    }
}

void AniWindow::SetDefaultDensityEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetDefaultDensityEnabled(env, enabled);
}


void AniWindow::OnSetDefaultDensityEnabled(ani_env* env, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->SetWindowDefaultDensityEnabled(static_cast<bool>(enabled)));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] winId: %{public}u set enabled=%{public}u result=%{public}d",
        windowToken_->GetWindowId(), enabled, ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "[window][setDefaultDensityEnabled]msg: set default density failed");
        return;
    }
    return;
}

void AniWindow::SetWindowContainerColor(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string activeColor, ani_string inactiveColor)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetWindowContainerColor(env, activeColor, inactiveColor);
}

void AniWindow::OnSetWindowContainerColor(ani_env* env, ani_string activeColor, ani_string inactiveColor)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowContainerColor]msg: invalid window");
        return;
    }
    std::string stdActiveColor;
    std::string stdInactiveColor;
    AniWindowUtils::GetStdString(env, activeColor, stdActiveColor);
    AniWindowUtils::GetStdString(env, inactiveColor, stdInactiveColor);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->SetWindowContainerColor(stdActiveColor, stdInactiveColor));
    TLOGI(WmsLogTag::WMS_DECOR, "winId: %{public}u set activeColor: %{public}s, inactiveColor: %{public}s"
        ", result: %{public}d", windowToken_->GetWindowId(), stdActiveColor.c_str(), stdInactiveColor.c_str(), ret);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "set window container color failed!");
        AniWindowUtils::AniThrowError(env, ret,
            "[window][setWindowContainerColor]msg: set window container color failed");
        return;
    }
    return;
}

void AniWindow::SetWindowContainerModalColor(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string activeColor, ani_string inactiveColor)
{
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetWindowContainerModalColor(env, activeColor, inactiveColor);
}

void AniWindow::OnSetWindowContainerModalColor(ani_env* env, ani_string activeColor, ani_string inactiveColor)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowContainerModalColor]msg: invalid window");
        return;
    }
    std::string stdActiveColor;
    std::string stdInactiveColor;
    AniWindowUtils::GetStdString(env, activeColor, stdActiveColor);
    AniWindowUtils::GetStdString(env, inactiveColor, stdInactiveColor);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->SetWindowContainerModalColor(stdActiveColor, stdInactiveColor));
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] set activeColor: %{public}s,"
        " inactiveColor: %{public}s, result: %{public}d", windowToken_->GetWindowId(),
        windowToken_->GetWindowName().c_str(), stdActiveColor.c_str(), stdInactiveColor.c_str(), ret);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "set window container modal color failed!");
        AniWindowUtils::AniThrowError(env, ret,
            "[window][setWindowContainerModalColor]msg: set container modal color failed");
        return;
    }
    return;
}

bool AniWindow::IsMainWindowFullScreenAcrossDisplays(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return aniWindow->OnIsMainWindowFullScreenAcrossDisplays(env);
}

bool AniWindow::OnIsMainWindowFullScreenAcrossDisplays(ani_env* env)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowContainerModalColor]msg: invalid window");
        return false;
    }
    bool isAcrossDisplaysPtr = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplaysPtr));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret,
            "[window][IsMainWindowFullScreenAcrossDisplays]msg: get immersive layout failed");
        return isAcrossDisplaysPtr;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u, isAcrossDisplays: %{public}u, ",
        windowToken_->GetWindowId(), isAcrossDisplaysPtr);
    return isAcrossDisplaysPtr;
}

void AniWindow::SetWindowShadowEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetWindowShadowEnabled(env, enable);
}

void AniWindow::OnSetWindowShadowEnabled(ani_env* env, ani_boolean enable)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][SetWindowShadowEnabled]msg: invalid window");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetWindowShadowEnabled(enable));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u, set enable: %{public}u", windowToken_->GetWindowId(), enable);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "[window][setWindowShadowEnabled]msg: set window shadow failed");
        return;
    }
    return;
}

bool AniWindow::IsImmersiveLayout(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return aniWindow->OnIsImmersiveLayout(env);
}

bool AniWindow::OnIsImmersiveLayout(ani_env* env)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isImmersiveLayout]msg: invalid window");
        return false;
    }
    bool isImmersiveLayout = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->IsImmersiveLayout(isImmersiveLayout));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed, ret %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret, "[window][isImmersiveLayout]msg: get immersive layout failed");
        return isImmersiveLayout;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isImmersiveLayout %{public}u end",
        windowToken_->GetWindowId(), isImmersiveLayout);
    return isImmersiveLayout;
}

/** @note @window.layout */
void AniWindow::ResizeAsync(ani_env* env, ani_int width, ani_int height)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resizeAsync]msg: window is nullptr");
        return;
    }

    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][resizeAsync]msg: width or height should greater than 0!");
        return;
    }

    const uint32_t w = static_cast<uint32_t>(width);
    const uint32_t h = static_cast<uint32_t>(height);
    const WMError ret = windowToken_->ResizeAsync(w, h);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
            "[ANI] resize failed, windowId: %{public}u, width: %{public}u, height: %{public}u, ret: %{public}d",
            windowId, w, h, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, width: %{public}u, height: %{public}u",
        windowId, w, h);
}

/** @note @window.layout */
ani_object AniWindow::SetWindowLimits(ani_env* env, ani_object inWindowLimits, ani_object forcible)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowLimits]msg: window is nullptr");
    }

    WindowLimits windowLimits;
    if (!AniWindowUtils::ParseWindowLimits(env, inWindowLimits, windowLimits)) {
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    
    if (windowLimits.maxWidth_ < 0 || windowLimits.maxHeight_ < 0 ||
        windowLimits.minWidth_ < 0 || windowLimits.minHeight_ < 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should be greater than or equal to 0");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowLimits]msg: Width or height should be greater than or equal to 0");
    }

    bool isForcible = false;
    if (!AniWindowUtils::CheckParaIsUndefined(env, forcible)) {
        if (!windowToken_->IsPhonePadOrPcWindow()) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] device not support");
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
                "[window][setWindowLimits]msg: device not support");
        }
        ani_boolean aniIsForcible;
        ani_status aniRet = env->Object_CallMethodByName_Boolean(forcible, "toBoolean", ":Z", &aniIsForcible);
        if (aniRet != ANI_OK) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed to convert parameter to isForcible");
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][setWindowLimits]msg: failed to convert parameter to isForcible");
        }
        isForcible = static_cast<bool>(aniIsForcible);
    }

    const WMError ret = windowToken_->SetWindowLimits(windowLimits, isForcible);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, maxW: %{public}u, maxH: %{public}u, "
            "minW: %{public}u, minH: %{public}u, ret: %{public}d",
            windowId, windowLimits.maxWidth_, windowLimits.maxHeight_,
            windowLimits.minWidth_, windowLimits.minHeight_, static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, errorCode);
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, maxW: %{public}u, maxH: %{public}u, "
        "minW: %{public}u, minH: %{public}u", windowId, windowLimits.maxWidth_, windowLimits.maxHeight_,
        windowLimits.minWidth_, windowLimits.minHeight_);
    return AniWindowUtils::CreateAniWindowLimits(env, windowLimits);
}

/** @note @window.layout */
ani_object AniWindow::GetWindowLimits(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowLimits]msg: window is nullptr");
    }

    WindowLimits windowLimits;
    const WMError ret = windowToken_->GetWindowLimits(windowLimits);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowId, static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, errorCode);
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, maxW: %{public}u, maxH: %{public}u, "
        "minW: %{public}u, minH: %{public}u", windowId, windowLimits.maxWidth_, windowLimits.maxHeight_,
        windowLimits.minWidth_, windowLimits.minHeight_);
    return AniWindowUtils::CreateAniWindowLimits(env, windowLimits);
}

/** @note @window.layout */
ani_object AniWindow::GetWindowLimitsVP(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    WindowLimits windowLimits;
    const WMError ret = windowToken_->GetWindowLimits(windowLimits, true);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowId, static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, errorCode);
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, maxW: %{public}u, maxH: %{public}u, "
        "minW: %{public}u, minH: %{public}u", windowId, windowLimits.maxWidth_, windowLimits.maxHeight_,
        windowLimits.minWidth_, windowLimits.minHeight_);
    return AniWindowUtils::CreateAniWindowLimits(env, windowLimits);
}

/** @note @window.layout */
void AniWindow::SetAspectRatio(ani_env* env, ani_double ratio)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setAspectRatio]msg: Window is nullptr");
        return;
    }

    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] only main window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setAspectRatio]msg: setAspectRatio is only allowed for the main window");
        return;
    }

    double aspectRatio = static_cast<double>(ratio);
    if (aspectRatio <= 0.0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] invalid param, ratio: %{public}f", aspectRatio);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setAspectRatio]msg: the ratio parameter is invalid, it must be greater than 0.0");
        return;
    }
    const WMError ret = windowToken_->SetAspectRatio(aspectRatio);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, ratio: %{public}f, ret: %{public}d",
            windowId, aspectRatio, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, ratio: %{public}f", windowId, aspectRatio);
}

/** @note @window.layout */
void AniWindow::ResetAspectRatio(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resetAspectRatio]msg: window is nullptr");
        return;
    }

    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] only main window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][resetAspectRatio]msg: resetAspectRatio is only allowed for the main window");
        return;
    }

    const WMError ret = windowToken_->ResetAspectRatio();
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowId, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u", windowId);
}

ani_int AniWindow::GetWindowStatus(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    return aniWindow->OnGetWindowStatus(env);
}

ani_int AniWindow::OnGetWindowStatus(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    WindowStatus windowStatus;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowStatus(windowStatus));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI] Get window status failed, ret=%{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
        return ANI_ERROR;
    }
    TLOGI(WmsLogTag::WMS_PC, "[ANI] window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return static_cast<ani_int>(windowStatus);
}

void AniWindow::Minimize(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] windowToken is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnMinimize(env);
}

void AniWindow::OnMinimize(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    if (WindowHelper::IsFloatOrSubWindow(windowToken_->GetType())) {
        if (!windowToken_->IsSceneBoardEnabled()) {
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
            return;
        }
        TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI] subWindow or float window use hide");
        HideWindowFunction(env, WmErrorCode::WM_OK);
        return;
    }

    WMError ret = windowToken_->Minimize();
    TLOGNI(WmsLogTag::WMS_PC, "[ANI] Window [%{public}u, %{public}s] minimize end, ret=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    if (ret != WMError::WM_OK) {
        WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        AniWindowUtils::AniThrowError(env, wmErrorCode);
    }
}

void AniWindow::HideWindowFunction(ani_env* env, WmErrorCode errCode)
{
    if (errCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errCode);
        return;
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGW(WmsLogTag::WMS_LIFE, "[ANI] window type %{public}u is not supported, [%{public}u, %{public}s]",
            static_cast<uint32_t>(windowToken_->GetType()),
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->Hide(0, false, false, true));
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI] end, window [%{public}u] ret=%{public}d",
        windowToken_->GetWindowId(), ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindow::Maximize(
    ani_env* env, ani_object obj, ani_long nativeObj, ani_object aniPresentation, ani_object aniAcrossDisplay)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnMaximize(env, aniPresentation, aniAcrossDisplay);
}

std::optional<MaximizePresentation> ParsePresentation(ani_env* env, ani_object aniPresentation)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] env is nullptr");
        return std::nullopt;
    }
    if (AniWindowUtils::CheckParaIsUndefined(env, aniPresentation)) {
        return MaximizePresentation::ENTER_IMMERSIVE;
    }
    uint32_t value = 0;
    ani_status ret = AniWindowUtils::GetEnumValue(env, static_cast<ani_enum_item>(aniPresentation), value);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Invalid presentation param, ret: %{public}d", ret);
        return std::nullopt;
    }
    return static_cast<MaximizePresentation>(value);
}

std::optional<WaterfallResidentState> ParseWaterfallResidentState(ani_env* env, ani_object aniAcrossDisplay)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] env is nullptr");
        return std::nullopt;
    }
    if (AniWindowUtils::CheckParaIsUndefined(env, aniAcrossDisplay)) {
        return WaterfallResidentState::UNCHANGED;
    }
    ani_boolean acrossDisplay = ANI_FALSE;
    ani_status ret = env->Object_CallMethodByName_Boolean(aniAcrossDisplay, "toBoolean", ":z", &acrossDisplay);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Invalid acrossDisplay param, ret: %{public}d", ret);
        return std::nullopt;
    }
    return acrossDisplay ? WaterfallResidentState::OPEN : WaterfallResidentState::CLOSE;
}

void AniWindow::OnMaximize(ani_env* env, ani_object aniPresentation, ani_object aniAcrossDisplay)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!(WindowHelper::IsMainWindow(windowToken_->GetType()) || windowToken_->IsSubWindowMaximizeSupported())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Unsupported window type");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    auto presentationOpt = ParsePresentation(env, aniPresentation);
    if (!presentationOpt) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    auto waterfallResidentStateOpt = ParseWaterfallResidentState(env, aniAcrossDisplay);
    if (!waterfallResidentStateOpt) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    auto ret = windowToken_->Maximize(*presentationOpt, *waterfallResidentStateOpt);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC,
              "[ANI] Failed, windowId: %{public}u, presentation: %{public}d, waterfallResidentState: %{public}u, ret: "
              "%{public}d",
              windowToken_->GetWindowId(),
              static_cast<int32_t>(*presentationOpt),
              static_cast<uint32_t>(*waterfallResidentStateOpt),
              static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, AniWindowUtils::ToErrorCode(ret));
        return;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC,
          "[ANI] Success, windowId: %{public}u, presentation: %{public}d, waterfallResidentState: %{public}d",
          windowToken_->GetWindowId(),
          static_cast<int32_t>(*presentationOpt),
          static_cast<uint32_t>(*waterfallResidentStateOpt));
}

void AniWindow::StartMoving(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnStartMoving(env);
}

void AniWindow::OnStartMoving(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (WindowHelper::IsInputWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] is not allowed since input window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    WmErrorCode ret = windowToken_->StartMoveWindow();
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed, windowId: %{public}u, ret: %{public}d",
              windowToken_->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Success, windowId: %{public}u", windowToken_->GetWindowId());
}

void AniWindow::StartMoveWindowWithCoordinate(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_int offsetX, ani_int offsetY)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnStartMoveWindowWithCoordinate(env, offsetX, offsetY);
}

void AniWindow::OnStartMoveWindowWithCoordinate(ani_env* env, ani_int offsetX, ani_int offsetY)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = windowToken_->StartMoveWindowWithCoordinate(static_cast<int>(offsetX), static_cast<int>(offsetY));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed, windowId: %{public}u, ret: %{public}d",
              windowToken_->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Success, windowId: %{public}u", windowToken_->GetWindowId());
}

void AniWindow::StopMoving(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnStopMoving(env);
}

void AniWindow::OnStopMoving(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = windowToken_->StopMoveWindow();
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed, windowId: %{public}u, ret: %{public}d",
              windowToken_->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Success, windowId: %{public}u", windowToken_->GetWindowId());
}

/** @note @window.layout */
void AniWindow::SetResizeByDragEnabled(ani_env* env, ani_boolean enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setResizeByDragEnabled]msg: window is nullptr");
        return;
    }

    bool dragEnabled = static_cast<bool>(enable);
    const WMError ret = windowToken_->SetResizeByDragEnabled(dragEnabled);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, enable: %{public}u, ret: %{public}d",
            windowId, dragEnabled, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, enable: %{public}u", windowId, dragEnabled);
}

/** @note @window.layout */
void AniWindow::EnableDrag(ani_env* env, ani_boolean enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][enableDrag]msg: window is nullptr");
        return;
    }

    bool dragEnabled = static_cast<bool>(enable);
    const WMError ret = windowToken_->EnableDrag(dragEnabled);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, enable: %{public}u, ret: %{public}d",
            windowId, dragEnabled, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, enable: %{public}u", windowId, dragEnabled);
}

/** @note @window.layout */
void AniWindow::MoveWindowToGlobal(ani_env* env, ani_int x, ani_int y, ani_object inMoveConfiguration)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowToGlobal]msg: window is nullptr");
        return;
    }

    MoveConfiguration moveConfiguration;
    if (!AniWindowUtils::CheckParaIsUndefined(env, inMoveConfiguration)) {
        int64_t displayId;
        ani_status aniRet = AniWindowUtils::GetPropertyLongObject(env, "displayId", inMoveConfiguration, displayId);
        if (aniRet == ANI_OK) {
            moveConfiguration.displayId = static_cast<DisplayId>(displayId);
        }
    }

    int32_t posX = static_cast<int32_t>(x);
    int32_t posY = static_cast<int32_t>(y);
    const WMError ret = windowToken_->MoveWindowToGlobal(posX, posY, moveConfiguration);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, displayId: %{public}" PRIu64", "
            "ret: %{public}d", windowId, moveConfiguration.displayId, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, displayId: %{public}" PRIu64" ",
        windowId, moveConfiguration.displayId);
}

/** @note @window.layout */
void AniWindow::MoveWindowToAsync(ani_env* env, ani_int x, ani_int y, ani_object inMoveConfiguration)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowToAsync]msg: window is nullptr");
        return;
    }

    MoveConfiguration moveConfiguration;
    if (!AniWindowUtils::CheckParaIsUndefined(env, inMoveConfiguration)) {
        int64_t displayId;
        ani_status aniRet = AniWindowUtils::GetPropertyLongObject(env, "displayId", inMoveConfiguration, displayId);
        if (aniRet == ANI_OK) {
            moveConfiguration.displayId = static_cast<DisplayId>(displayId);
        }
    }

    int32_t posX = static_cast<int32_t>(x);
    int32_t posY = static_cast<int32_t>(y);
    const WMError ret = windowToken_->MoveToAsync(posX, posY, moveConfiguration);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, displayId: %{public}" PRIu64", "
            "ret: %{public}d", windowId, moveConfiguration.displayId, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, displayId: %{public}" PRIu64" ",
        windowId, moveConfiguration.displayId);
}

/** @note @window.layout */
void AniWindow::SetWindowMode(ani_env* env, ani_enum_item mode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setwindowMode]msg: window is nullptr");
        return;
    }

    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][setwindowMode]msg: permission denied!");
        return;
    }

    uint32_t modeType;
    ani_status aniRet = AniWindowUtils::GetEnumValue(env, mode, modeType);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] get enum value failed, ret: %{public}d", static_cast<int32_t>(aniRet));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setwindowMode]msg: failed to convert parameter to mode");
        return;
    }
    WindowMode winMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    if (modeType >= static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY)) {
        winMode = static_cast<WindowMode>(modeType);
    } else if (modeType >= static_cast<uint32_t>(ApiWindowMode::UNDEFINED) &&
               modeType <= static_cast<uint32_t>(ApiWindowMode::MODE_END)) {
        winMode = JS_TO_NATIVE_WINDOW_MODE_MAP.at(static_cast<ApiWindowMode>(modeType));
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] invalid modeType: %{public}u", modeType);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setwindowMode]msg: invalid window mode");
        return;
    }

    const WMError ret = windowToken_->SetWindowMode(winMode);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, modeType: %{public}u, ret: %{public}d",
            windowId, modeType, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, modeType: %{public}u", windowId, modeType);
}

/** @note @window.layout */
void AniWindow::SetForbidSplitMove(ani_env* env, ani_boolean isForbidSplitMove)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setForbidSplitMove]msg: window is nullptr");
        return;
    }

    bool isMove = static_cast<bool>(isForbidSplitMove);
    WMError ret = WMError::WM_OK;
    if (isMove) {
        ret = windowToken_->AddWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE);
    } else {
        ret = windowToken_->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE);
    }
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, isMove: %{public}u, ret: %{public}d",
            windowId, isMove, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, isMove: %{public}u", windowId, isMove);
}

/** @note @window.layout */
void AniWindow::SetFollowParentWindowLayoutEnabled(ani_env* env, ani_boolean enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setFollowParentWindowLayoutEnabled]msg: window is nullptr");
        return;
    }
    if (!WindowHelper::IsSubWindow(windowToken_->GetType()) && !WindowHelper::IsDialogWindow(windowToken_->GetType())) {
        TLOGNE(WmsLogTag::WMS_SUB, "[ANI] only sub window and dialog is valid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setFollowParentWindowLayoutEnabled]msg: only sub window and dialog is valid");
        return;
    }

    bool isFollow = static_cast<bool>(enable);
    WMError ret = windowToken_->SetFollowParentWindowLayoutEnabled(isFollow);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] failed, windowId: %{public}u, enable: %{public}u, ret: %{public}d",
            windowId, isFollow, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_SUB, "[ANI] success, windowId: %{public}u, enable: %{public}u", windowId, isFollow);
}

/** @note @window.layout */
void AniWindow::SetFollowParentMultiScreenPolicy(ani_env* env, ani_boolean enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setFollowParentMultiScreenPolicy]msg: window is nullptr");
        return;
    }
    if (!WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGNE(WmsLogTag::WMS_SUB, "[ANI] invalid call type:%{public}d",
            windowToken_->GetType());
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setFollowParentMultiScreenPolicy]msg: invalid window type");
        return;
    }

    bool boEnabled = static_cast<bool>(enable);
    WMError ret = windowToken_->SetFollowParentMultiScreenPolicy(boEnabled);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);

    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] failed, windowId: %{public}u, enable: %{public}u, ret: %{public}d",
            windowId, boEnabled, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_SUB, "[ANI] success, windowId: %{public}u, enable: %{public}u", windowId, boEnabled);
}

/** @note @window.layout */
void AniWindow::MoveWindowToGlobalDisplay(ani_env* env, ani_int x, ani_int y)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowToGlobalDisplay]msg: window is nullptr");
        return;
    }

    int32_t posX = static_cast<int32_t>(x);
    int32_t posY = static_cast<int32_t>(y);
    WMError ret = windowToken_->MoveWindowToGlobalDisplay(posX, posY);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, x: %{public}d, y: %{public}d, "
            "ret: %{public}d", windowId, posX, posY, static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, errorCode);
        return;
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, windowId: %{public}u, x: %{public}d, y: %{public}d",
        windowId, posX, posY);
}

ani_object AniWindow::HandlePositionTransform(
    ani_env* env, ani_int x, ani_int y,
    std::function<WMError(sptr<Window>&, const Position&, Position&)> transformFunc)
{
    if (windowToken_ == nullptr || transformFunc == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window or transformFunc is null");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][handlePositionTransform]msg: window or transformFunc is nullptr");
    }

    int32_t posX = static_cast<int32_t>(x);
    int32_t posY = static_cast<int32_t>(y);
    Position inPosition { posX, posY };
    Position outPosition;
    WMError ret = transformFunc(windowToken_, inPosition, outPosition);
    const WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    const uint32_t windowId = windowToken_->GetWindowId();
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] failed, windowId: %{public}u, x: %{public}d, y: %{public}d, "
            "ret: %{public}d", windowId, posX, posY, static_cast<int32_t>(ret));
        return AniWindowUtils::AniThrowError(env, errorCode);
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "[ANI] success, x: %{public}d, y: %{public}d", posX, posY);
    return AniWindowUtils::CreateAniPosition(env, outPosition);
}

ani_object AniWindow::CreateAniWindow(ani_env* env, OHOS::sptr<OHOS::Rosen::Window>& window)
__attribute__((no_sanitize("cfi")))
{
    using namespace OHOS::Rosen;
    if (env == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return nullptr;
    }
    TLOGD(WmsLogTag::DEFAULT, "[ANI] create wstage");

    ani_status ret;
    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env %{public}u", ret);
        return cls;
    }

    std::unique_ptr<AniWindow> uniqueWindow = std::make_unique<AniWindow>(window);

    ani_field contextField;
    if ((ret = env->Class_FindField(cls, "nativeObj", &contextField)) != ANI_OK) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] get field fail %{public}u", ret);
        return nullptr;
    }

    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] get ctor fail %{public}u", ret);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] obj new fail %{public}u", ret);
        return nullptr;
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] get setNativeObj fail %{public}u", ret);
        return nullptr;
    }
    env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(uniqueWindow.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(obj, &ref) == ANI_OK) {
        uniqueWindow->SetAniRef(ref);
        g_localObjs.insert(std::pair(ref, uniqueWindow.release()));
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] create global ref fail");
    }

    return obj;
}

void AniWindow::SetRotationLocked(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean locked)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetRotationLocked(env, locked);
}
 
void AniWindow::OnSetRotationLocked(ani_env* env, ani_boolean locked)
{
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "permission denied, require system application!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    WMError ret = windowToken_->SetRotationLocked(locked);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowToken_->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, AniWindowUtils::ToErrorCode(ret));
        return;
    }
}

ani_boolean AniWindow::GetRotationLocked(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return static_cast<ani_boolean>(aniWindow->OnGetRotationLocked(env));
    }
    TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] aniWindow is nullptr");
    AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    return ANI_FALSE;
}

bool AniWindow::OnGetRotationLocked(ani_env* env)
{
    bool locked = false;
    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return locked;
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "permission denied, require system application!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return locked;
    }
    WMError ret = windowToken_->GetRotationLocked(locked);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] failed, windowId: %{public}u, ret: %{public}d",
            windowToken_->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, AniWindowUtils::ToErrorCode(ret));
        return locked;
    }
    return locked;
}

ani_boolean AniWindow::IsInFreeWindowMode(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    return aniWindow != nullptr ? static_cast<ani_boolean>(aniWindow->OnIsInFreeWindowMode(env)) :
        static_cast<ani_boolean>(false);
}

bool AniWindow::OnIsInFreeWindowMode(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return windowToken_->IsPcOrPadFreeMultiWindowMode();
}

ani_string AniWindow::GetWindowStateSnapshot(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    return aniWindow != nullptr ? aniWindow->OnGetWindowStateSnapshot(env) : nullptr;
}

ani_string AniWindow::OnGetWindowStateSnapshot(ani_env* env)
{
    ani_string result = nullptr;
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return result;
    }
    std::string winStateSnapshot;
    WMError errCode = windowToken_->GetWindowStateSnapshot(winStateSnapshot);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get window state snapshot failed, errCode=%{public}d",
            static_cast<int32_t>(errCode));
        auto retErrCode = WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY;
        if (WM_JS_TO_ERROR_CODE_MAP.count(errCode) > 0) {
            retErrCode = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
        }
        AniWindowUtils::AniThrowError(env, retErrCode, "get window state snapshot failed");
        return result;
    }
    AniWindowUtils::GetAniString(env, winStateSnapshot, &result);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winStateSnapshot=%{public}s", winStateSnapshot.c_str());
    return result;
}

void AniWindow::SetWindowDelayRaiseOnDrag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isEnabled)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowDelayRaiseOnDrag]msg: WindowToken is nullptr");
        return;
    }
    aniWindow->OnSetWindowDelayRaiseOnDrag(env, isEnabled);
}

void AniWindow::OnSetWindowDelayRaiseOnDrag(ani_env* env, ani_boolean isEnabled)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowDelayRaiseOnDrag]msg: WindowToken is nullptr");
        return;
    }
    WMError ret = windowToken_->SetWindowDelayRaiseEnabled(static_cast<bool>(isEnabled));
    WmErrorCode errorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "failed");
        AniWindowUtils::AniThrowError(env, errorCode, "[window][setWindowDelayRaiseOnDrag]");
        return;
    }
}

void AniWindow::SetRelativePositionToParentWindowEnabled(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_boolean enabled, ani_int anchor, ani_int offsetX, ani_int offsetY)
{
    TLOGD(WmsLogTag::WMS_SUB, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetRelativePositionToParentWindowEnabled(env, enabled, anchor, offsetX, offsetY);
}

void AniWindow::OnSetRelativePositionToParentWindowEnabled(ani_env* env, ani_boolean enabled,
    ani_int anchor, ani_int offsetX, ani_int offsetY)
{
    TLOGI(WmsLogTag::WMS_SUB, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] only sub window is valid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    WindowAnchorInfo windowAnchorInfo = { static_cast<bool>(enabled), static_cast<WindowAnchor>(anchor),
        static_cast<int32_t>(offsetX), static_cast<int32_t>(offsetY) };
    WmErrorCode errorCode = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetWindowAnchorInfo(windowAnchorInfo));
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] failed");
        AniWindowUtils::AniThrowError(env, errorCode, "[ANI] set window anchor info failed.");
        return;
    }
}

void AniWindow::SetWindowTransitionAnimation(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_enum_item transitionType, ani_object animation)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnSetWindowTransitionAnimation(env, transitionType, animation);
}

void AniWindow::OnSetWindowTransitionAnimation(ani_env* env, ani_enum_item transitionType, ani_object animation)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Window instance not exist");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(windowToken_->IsTransitionAnimationSupported());
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Transition animation is not supported");
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    uint32_t type = 0;
    ani_status aniRet = AniWindowUtils::GetEnumValue(env, transitionType, type);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] get enum value failed, ret: %{public}d", static_cast<int32_t>(aniRet));
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    if (type >= static_cast<uint32_t>(WindowTransitionType::END)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to type");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
        return;
    }

    TransitionAnimation transitionAnimation;
    WmErrorCode convertResult = WmErrorCode::WM_OK;
    if (!ConvertTransitionAnimationFromAniValue(env, animation, transitionAnimation, convertResult)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to animation");
        AniWindowUtils::AniThrowError(env, convertResult);
        return;
    }

    ret = AniWindowUtils::ToErrorCode(windowToken_->SetWindowTransitionAnimation(
        static_cast<WindowTransitionType>(type), transitionAnimation));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Set window transition animation failed, ret is %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret);
    }
}

ani_object AniWindow::GetWindowTransitionAnimation(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_enum_item transitionType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->OnGetWindowTransitionAnimation(env, transitionType);
}

ani_object AniWindow::OnGetWindowTransitionAnimation(ani_env* env, ani_enum_item transitionType)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
        TLOGD(WmsLogTag::WMS_ANIMATION, "[NAPI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Window instance not exist");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowTransitionAnimation]msg:transition animation is not enable");
    }
    WmErrorCode ret = AniWindowUtils::ToErrorCode(windowToken_->IsTransitionAnimationSupported());
    if (ret != WmErrorCode::WM_OK) {
        return AniWindowUtils::AniThrowError(env, ret,
            "[window][getWindowTransitionAnimation]msg:transition animation is not enable");
    }

    uint32_t enumValue = 0;
    ani_status aniRet = AniWindowUtils::GetEnumValue(env, transitionType, enumValue);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get transitionType value failed: %{public}u", aniRet);
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Get value failed");
    }

    WindowTransitionType type = static_cast<WindowTransitionType>(enumValue);
    if (type >= WindowTransitionType::END) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "type %{public}u is out of range", static_cast<uint32_t>(type));
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to convert parameter to type");
    }
    ani_object result = ConvertTransitionAnimationToAniValue(env, windowToken_->GetWindowTransitionAnimation(type));
    if (result != nullptr) {
        TLOGI(WmsLogTag::WMS_ANIMATION, "Get window transition animation success with type %{public}u",
            static_cast<uint32_t>(type));
        return result;
    }
    return AniWindowUtils::CreateAniUndefined(env);
}

ani_object AniWindow::CreateSubWindowWithOptions(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string name, ani_object options)
{
    TLOGD(WmsLogTag::WMS_SUB, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnCreateSubWindowWithOptions(env, name, options);
    } else {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }
}

ani_object AniWindow::OnCreateSubWindowWithOptions(ani_env* env, ani_string name, ani_object options)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "window is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (!windowToken_->IsPcOrFreeMultiWindowCapabilityEnabled()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::string windowName;
    AniWindowUtils::GetStdString(env, name, windowName);
    sptr<WindowOption> windowOption = sptr<WindowOption>::MakeSptr();
    if (!AniWindowUtils::ParseSubWindowOption(env, options, windowOption)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to options");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if ((windowOption->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (windowOption->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (!WindowHelper::IsFloatOrSubWindow(windowToken_->GetType()) &&
        !WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "%{public}d", windowToken_->GetType());
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "invalid window type");
    }
    windowOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowOption->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowOption->SetOnlySupportSceneBoard(true);
    windowOption->SetParentId(windowToken_->GetWindowId());
    windowOption->SetWindowTag(WindowTag::SUB_WINDOW);
    auto subWindow = Window::Create(windowName, windowOption, windowToken_->GetContext());
    if (subWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "create sub window failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "create sub window failed");
    }
    TLOGI(WmsLogTag::WMS_SUB, "Create sub window %{public}s end", windowName.c_str());
    return static_cast<ani_object>(CreateAniWindowObject(env, subWindow));
}

void AniWindow::Hide(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow != nullptr) {
        return aniWindow->OnHide(env);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindow is nullptr");
    }
}

void AniWindow::OnHide(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    auto window = GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto winType = window->GetType();
    if (WindowHelper::IsMainWindow(winType)) {
        TLOGW(WmsLogTag::WMS_LIFE, "window Type %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(window->GetType()), window->GetWindowId(), window->GetWindowName().c_str());
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Hide(0, false, false, true));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window hide failed");
    }
}

void AniWindow::Restore(ani_env* env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Restore fail, not main window");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->Restore());
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Window restore failed");
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindow::RestoreMainWindow(ani_env* env, ani_object obj, ani_long nativeObj, ani_object wantParameters)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->OnRestoreMainWindow(env, wantParameters);
}

void AniWindow::OnRestoreMainWindow(ani_env* env, ani_object wantParameters)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] Window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    if (windowToken_->GetType() != WindowType::WINDOW_TYPE_FLOAT) {
        TLOGE(WmsLogTag::WMS_LIFE, "only TYPE_FLOAT can use interface, type:%{public}u", windowToken_->GetType());
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
        return;
    }

    AAFwk::WantParams wantParams;
    if (wantParameters != nullptr) {
        ani_boolean isUndefined = false;
        ani_boolean isNull = false;
        env->Reference_IsUndefined(wantParameters, &isUndefined);
        env->Reference_IsNull(wantParameters, &isNull);
        if (!isUndefined && !isNull && !AppExecFwk::UnwrapWantParams(env, wantParameters, wantParams)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameters to wantParameters");
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
            return;
        }
    }

    std::shared_ptr<AAFwk::WantParams> parameters = std::make_shared<AAFwk::WantParams>(wantParams);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->RestoreMainWindow(parameters));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
    }
    return;
}
}  // namespace Rosen
}  // namespace OHOS

static void Restore(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] windowToken is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->Restore(env);
}

static void SetWindowTopmost(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isWindowTopmost)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindow is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTopmost]msg: Window is null");
        return;
    }
    aniWindow->SetWindowTopmost(env, isWindowTopmost);
}

static void WindowResize(ani_env* env, ani_object obj, ani_long nativeObj, ani_int width, ani_int height)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resize]msg: window is nullptr");
        return;
    }
    aniWindow->Resize(env, width, height);
}

static void WindowMoveWindowTo(ani_env* env, ani_object obj, ani_long nativeObj, ani_int x, ani_int y)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowTo]msg: window is nullptr");
        return;
    }
    aniWindow->MoveWindowTo(env, x, y);
}

static ani_object WindowGetGlobalRect(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getGlobalRect]msg: window is nullptr");
    }
    return aniWindow->GetGlobalRect(env);
}

static ani_double WindowGetWindowDecorHeight(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    return aniWindow->GetWindowDecorHeight(env);
}

static void SetDecorButtonStyle(ani_env* env, ani_object obj, ani_long nativeObj, ani_object decorStyle)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] windowToken is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetDecorButtonStyle(env, decorStyle);
}

static void SetWindowTitleButtonVisible(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_object visibleParam)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_DECOR, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] windowToken is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetWindowTitleButtonVisible(env, visibleParam);
}

static ani_int WindowSetWindowBackgroundColor(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string color)
{
    using namespace OHOS::Rosen;
    std::string colorStr;
    OHOS::Rosen::AniWindowUtils::GetStdString(env, color, colorStr);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[ANI] SetWindowBackgroundColor %{public}s", colorStr.c_str());
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetWindowBackgroundColor(env, colorStr);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] SetWindowBackgroundColor end");
    return ANI_OK;
}


static ani_int WindowSetImmersiveModeEnabledState(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_boolean enable)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetImmersiveModeEnabledState(env, static_cast<bool>(enable));
    return ANI_OK;
}

static ani_int WindowSetWindowDecorVisible(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isVisible)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetWindowDecorVisible(env, static_cast<bool>(isVisible));
    return ANI_OK;
}

static ani_int WindowSetWindowDecorHeight(ani_env* env, ani_object obj, ani_long nativeObj, ani_int height)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetWindowDecorHeight(env, height);
    return ANI_OK;
}

static ani_object WindowGetWindowProperties(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] windowToken_ is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->GetWindowPropertiesSync(env);
}

static ani_boolean WindowIsWindowSupportWideGamut(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return aniWindow->IsWindowSupportWideGamut(env);
}
static ani_int WindowSetWindowLayoutFullScreen(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_boolean isLayoutFullScreen)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetWindowLayoutFullScreen(env, isLayoutFullScreen);
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] SetWindowLayoutFullScreen end");
    return ANI_OK;
}

static ani_int WindowSetRaiseByClickEnabled(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_boolean enable)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setRaiseByClickEnabled]msg: windowToken_ is nullptr");
        return ANI_ERROR;
    }
    aniWindow->SetRaiseByClickEnabled(env, enable);
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI] setRaiseByClickEnabled end");
    return ANI_OK;
}

static ani_int WindowSetExclusivelyHighlighted(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_boolean exclusivelyHighlighted)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setExclusivelyHighlighted]msg: Window is nullptr");
        return ANI_ERROR;
    }
    aniWindow->SetExclusivelyHighlighted(env, exclusivelyHighlighted);
    TLOGI(WmsLogTag::WMS_FOCUS, "[ANI] setExclusivelyHighlighted end");
    return ANI_OK;
}

static ani_int WindowSetSystemBarProperties(ani_env* env, ani_object obj,
    ani_long nativeObj, ani_object aniSystemBarProperties)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetSystemBarProperties(env, aniSystemBarProperties);
    return ANI_OK;
}

static ani_int WindowSetSpecificSystemBarEnabled(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string name, ani_boolean enable, ani_object enableAnimation)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] windowToken_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    aniWindow->SetSpecificSystemBarEnabled(env, name, enable, enableAnimation);
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI] SetSpecificSystemBarEnabled end");
    return ANI_OK;
}

static ani_object WindowSetDragKeyFramePolicy(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_object aniKeyFramePolicy)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->SetDragKeyFramePolicy(env, aniKeyFramePolicy);
}

static ani_object Snapshot(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->Snapshot(env);
}

static ani_object SnapshotSync(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->SnapshotSync(env);
}

static void HideNonSystemFloatingWindows(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean shouldHide)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->HideNonSystemFloatingWindows(env, shouldHide);
}

static void SetWindowBrightness(ani_env* env, ani_object obj, ani_long nativeObj, ani_double brightness)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetWindowBrightness(env, brightness);
}

static ani_int GetWindowColorSpace(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return ANI_ERROR;
    }
    return aniWindow->GetWindowColorSpace(env);
}

static void SetWakeUpScreen(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean wakeUp)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    return aniWindow->SetWakeUpScreen(env, wakeUp);
}

static void SetSnapshotSkip(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isSkip)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetSnapshotSkip(env, isSkip);
}

static ani_object SnapshotIgnorePrivacy(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->SnapshotIgnorePrivacy(env);
}

static ani_object GetStatusBarProperty(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->GetStatusBarProperty(env);
}

static void SetStatusBarColor(ani_env* env, ani_object obj, ani_long nativeObj, ani_long color)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetStatusBarColor(env, color);
}

static ani_boolean IsSystemAvoidAreaEnabled(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return aniWindow->IsSystemAvoidAreaEnabled(env);
}

static void SetSystemAvoidAreaEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetSystemAvoidAreaEnabled(env, enabled);
}

static ani_object GetWindowDensityInfo(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->GetWindowDensityInfo(env);
}

static ani_boolean GetImmersiveModeEnabledState(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return aniWindow->GetImmersiveModeEnabledState(env);
}

static void SetWindowGrayScale(ani_env* env, ani_object obj, ani_long nativeObj, ani_double grayScale)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetWindowGrayScale(env, grayScale);
}

static ani_object GetWindowSystemBarProperties(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->GetWindowSystemBarProperties(env);
}

static ani_boolean IsGestureBackEnabled(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    return aniWindow->IsGestureBackEnabled(env);
}

static void SetGestureBackEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_IMMS, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetGestureBackEnabled(env, enabled);
}

static void SetSingleFrameComposerEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set single frame composer enabled permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindow->SetSingleFrameComposerEnabled(env, enabled);
}

static ani_object WindowCreate(ani_env* env, ani_long window)
{
    using namespace OHOS::Rosen;
    Rect baseWindowRect = { 150, 150, 400, 600 };
    OHOS::sptr<WindowOption> baseOp = new WindowOption();
    SystemBarProperty barProperty = SystemBarProperty();
    WindowType windowType { WindowType::WINDOW_TYPE_FLOAT };
    baseOp->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    baseOp->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    baseOp->SetWindowRect(baseWindowRect);
    baseOp->SetSystemBarProperty(windowType, barProperty);
    OHOS::sptr<Window> windowPtr = Window::Create("TestWindow", baseOp, nullptr);
    return AniWindow::CreateAniWindow(env, windowPtr); // just for test
}

static void WindowResizeAsync(ani_env* env, ani_object obj, ani_long nativeObj, ani_int width, ani_int height)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resizeAsync]msg: window is nullptr");
        return;
    }
    aniWindow->ResizeAsync(env, width, height);
}

static ani_object WindowSetWindowLimits(ani_env* env, ani_object obj, ani_long nativeObj, ani_object windowLimits,
                                        ani_object forcible)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowLimits]msg: window is nullptr");
    }
    return aniWindow->SetWindowLimits(env, windowLimits, forcible);
}

static ani_object WindowGetWindowLimits(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowLimits]msg: window is nullptr");
    }
    return aniWindow->GetWindowLimits(env);
}

static ani_object WindowGetWindowLimitsVP(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return aniWindow->GetWindowLimitsVP(env);
}

static ani_ref GetParentWindow(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_SUB, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] windowToken is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return aniWindow->GetParentWindow(env);
}

static void SetParentWindow(ani_env* env, ani_object obj, ani_long nativeObj, ani_int windowId)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_SUB, "[ANI] start");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (aniWindow == nullptr || aniWindow->GetWindow() == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] windowToken is nullptr");
        return;
    }
    aniWindow->SetParentWindow(env, windowId);
}

static void WindowSetAspectRatio(ani_env* env, ani_object obj, ani_long nativeObj, ani_double ratio)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setAspectRatio]msg: Window is nullptr");
        return;
    }
    aniWindow->SetAspectRatio(env, ratio);
}

static void WindowResetAspectRatio(ani_env* env, ani_object obj, ani_long nativeObj)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resetAspectRatio]msg: Window is nullptr");
        return;
    }
    aniWindow->ResetAspectRatio(env);
}

static void WindowSetResizeByDragEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setResizeByDragEnabled]msg: window is nullptr");
        return;
    }
    aniWindow->SetResizeByDragEnabled(env, enable);
}

static void WindowEnableDrag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][enableDrag]msg: window is nullptr");
        return;
    }
    aniWindow->EnableDrag(env, enable);
}

static void WindowMoveWindowToGlobal(ani_env* env, ani_object obj, ani_long nativeObj, ani_int x, ani_int y,
                                     ani_object moveConfiguration)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowToGlobal]msg: window is nullptr");
        return;
    }
    aniWindow->MoveWindowToGlobal(env, x, y, moveConfiguration);
}

static void WindowMoveWindowToAsync(ani_env* env, ani_object obj, ani_long nativeObj, ani_int x, ani_int y,
                                    ani_object moveConfiguration)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowToAsync]msg: window is nullptr");
        return;
    }
    aniWindow->MoveWindowToAsync(env, x, y, moveConfiguration);
}

static void WindowSetWindowMode(ani_env* env, ani_object obj, ani_long nativeObj, ani_enum_item mode)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setwindowMode]msg: window is nullptr");
        return;
    }
    aniWindow->SetWindowMode(env, mode);
}

static void WindowSetForbidSplitMove(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isForbidSplitMove)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setForbidSplitMove]msg: window is nullptr");
        return;
    }
    aniWindow->SetForbidSplitMove(env, isForbidSplitMove);
}

static void WindowSetFollowParentWindowLayoutEnabled(ani_env* env, ani_object obj, ani_long nativeObj,
                                                     ani_boolean enable)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setFollowParentWindowLayoutEnabled]msg: window is nullptr");
        return;
    }
    aniWindow->SetFollowParentWindowLayoutEnabled(env, enable);
}

static void WindowSetFollowParentMultiScreenPolicy(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setFollowParentMultiScreenPolicy]msg: window is nullptr");
        return;
    }
    aniWindow->SetFollowParentMultiScreenPolicy(env, enable);
}

static void WindowMoveWindowToGlobalDisplay(ani_env* env, ani_object obj, ani_long nativeObj,
                                            ani_int x, ani_int y)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][moveWindowToGlobalDisplay]msg: window is nullptr");
        return;
    }
    aniWindow->MoveWindowToGlobalDisplay(env, x, y);
}

static ani_object WindowClientToGlobalDisplay(ani_env* env, ani_object obj, ani_long nativeObj,
                                              ani_int winX, ani_int winY)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][clientToGlobalDisplay]msg: window is nullptr");
    }
    return aniWindow->HandlePositionTransform(env, winX, winY,
        [](const OHOS::sptr<Window>& window, const Position& inPos, Position& outPos) {
            if (window == nullptr) {
                return WMError::WM_ERROR_NULLPTR;
            }
            return window->ClientToGlobalDisplay(inPos, outPos);
        });
}

static ani_object WindowGlobalDisplayToClient(ani_env* env, ani_object obj, ani_long nativeObj,
                                              ani_int globalDisplayX, ani_int globalDisplayY)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI]");
    AniWindow* aniWindow = reinterpret_cast<AniWindow*>(nativeObj);
    if (!aniWindow || !aniWindow->GetWindow()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] windowToken is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][globalDisplayToClient]msg: window is nullptr");
    }
    return aniWindow->HandlePositionTransform(env, globalDisplayX, globalDisplayY,
        [](const OHOS::sptr<Window>& window, const Position& inPos, Position& outPos) {
            if (window == nullptr) {
                return WMError::WM_ERROR_NULLPTR;
            }
            return window->GlobalDisplayToClient(inPos, outPos);
        });
}

ani_status OHOS::Rosen::ANI_Window_Constructor(ani_vm *vm, uint32_t *result)
{
    using namespace OHOS::Rosen;
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array methods = {
        ani_native_function {"throwIfWindowInvalid", "l:",
            reinterpret_cast<void *>(AniWindow::ThrowIfWindowInvalid)},
        ani_native_function {"resize", "lii:",
            reinterpret_cast<void *>(WindowResize)},
        ani_native_function {"moveWindowTo", "lii:",
            reinterpret_cast<void *>(WindowMoveWindowTo)},
        ani_native_function {"getGlobalRect", "l:C{@ohos.window.window.Rect}",
            reinterpret_cast<void *>(WindowGetGlobalRect)},
        ani_native_function {"getWindowDecorHeight", "l:i",
            reinterpret_cast<void *>(WindowGetWindowDecorHeight)},
        ani_native_function {"setWindowBackgroundColor", "lC{std.core.String}:i",
            reinterpret_cast<void *>(WindowSetWindowBackgroundColor)},
        ani_native_function {"setImmersiveModeEnabledState", "lz:i",
            reinterpret_cast<void *>(WindowSetImmersiveModeEnabledState)},
        ani_native_function {"setWindowDecorVisible", "lz:i",
            reinterpret_cast<void *>(WindowSetWindowDecorVisible)},
        ani_native_function {"setWindowDecorHeight", "li:i",
            reinterpret_cast<void *>(WindowSetWindowDecorHeight)},
        ani_native_function {"getWindowProperties", "l:C{@ohos.window.window.WindowProperties}",
            reinterpret_cast<void *>(WindowGetWindowProperties)},
        ani_native_function {"getProperties", "l:C{@ohos.window.window.WindowProperties}",
            reinterpret_cast<void *>(WindowGetWindowProperties)},
        ani_native_function {"isWindowSupportWideGamut", "l:z",
            reinterpret_cast<void *>(WindowIsWindowSupportWideGamut)},
        ani_native_function {"setWindowLayoutFullScreen", "lz:i",
            reinterpret_cast<void *>(WindowSetWindowLayoutFullScreen)},
        ani_native_function {"setWindowSystemBarProperties", "lC{@ohos.window.window.SystemBarProperties}:i",
            reinterpret_cast<void *>(WindowSetSystemBarProperties)},
        ani_native_function {"setRaiseByClickEnabled", "lz:i",
            reinterpret_cast<void *>(WindowSetRaiseByClickEnabled)},
        ani_native_function {"setExclusivelyHighlighted", "lz:i",
            reinterpret_cast<void *>(WindowSetExclusivelyHighlighted)},
        ani_native_function {"setSpecificSystemBarEnabled", "lC{std.core.String}zC{std.core.Boolean}:i",
            reinterpret_cast<void *>(WindowSetSpecificSystemBarEnabled)},
        ani_native_function {"setDragKeyFramePolicy",
            "lC{@ohos.window.window.KeyFramePolicy}:C{@ohos.window.window.KeyFramePolicy}",
            reinterpret_cast<void *>(WindowSetDragKeyFramePolicy)},
        ani_native_function {"snapshot", "l:C{@ohos.multimedia.image.image.PixelMap}",
            reinterpret_cast<void *>(Snapshot)},
        ani_native_function {"snapshotSync", "l:C{@ohos.multimedia.image.image.PixelMap}",
            reinterpret_cast<void *>(SnapshotSync)},
        ani_native_function {"hideNonSystemFloatingWindows", "lz:",
            reinterpret_cast<void *>(HideNonSystemFloatingWindows)},
        ani_native_function {"setWindowColorSpaceSync", "li:",
            reinterpret_cast<void *>(AniWindow::SetWindowColorSpace)},
        ani_native_function {"setPreferredOrientationSync", "li:",
            reinterpret_cast<void *>(AniWindow::SetPreferredOrientation)},
        ani_native_function {"getPreferredOrientation", "l:i",
            reinterpret_cast<void *>(AniWindow::GetPreferredOrientation)},
        ani_native_function {"convertOrientationAndRotation", "liii:i",
            reinterpret_cast<void *>(AniWindow::ConvertOrientationAndRotation)},
        ani_native_function {"setWindowPrivacyModeSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowPrivacyMode)},
        ani_native_function {"recoverSync", "l:",
            reinterpret_cast<void *>(AniWindow::Recover)},
        ani_native_function {"setUIContentSync", "lC{std.core.String}:",
            reinterpret_cast<void *>(AniWindow::SetUIContent)},
        ani_native_function {"loadContentSync",
            "lC{std.core.String}C{arkui.stateManagement.storage.localStorage.LocalStorage}:",
            reinterpret_cast<void *>(AniWindow::LoadContent)},
        ani_native_function {"loadContentByNameSync",
            "lC{std.core.String}C{arkui.stateManagement.storage.localStorage.LocalStorage}:",
            reinterpret_cast<void *>(AniWindow::LoadContentByName)},
        ani_native_function {"setWindowKeepScreenOnSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowKeepScreenOn)},
        ani_native_function {"setWindowSystemBarEnableSync", "lC{std.core.Array}:",
            reinterpret_cast<void *>(AniWindow::SetWindowSystemBarEnable)},
        ani_native_function {"setWindowTouchableSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowTouchable)},
        ani_native_function {"setDialogBackGestureEnabledSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetDialogBackGestureEnabled)},
        ani_native_function {"setWindowMaskSync", "lC{std.core.Array}:",
            reinterpret_cast<void *>(AniWindow::SetWindowMask)},
        ani_native_function {"setTouchableAreas", "lC{std.core.Array}:",
            reinterpret_cast<void *>(AniWindow::SetTouchableAreas)},
        ani_native_function {"getUIContextSync", "l:C{@ohos.arkui.UIContext.UIContext}",
            reinterpret_cast<void *>(AniWindow::GetUIContext)},
        ani_native_function {"getWindowAvoidAreaSync", "li:C{@ohos.window.window.AvoidArea}",
            reinterpret_cast<void *>(AniWindow::GetWindowAvoidArea)},
        ani_native_function {"getWindowAvoidAreaIgnoringVisibilitySync", "li:C{@ohos.window.window.AvoidArea}",
            reinterpret_cast<void *>(AniWindow::GetWindowAvoidAreaIgnoringVisibility)},
        ani_native_function {"setWaterMarkFlagSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWaterMarkFlag)},
        ani_native_function {"raiseMainWindowAboveTargetSync", "li:",
            reinterpret_cast<void *>(AniWindow::RaiseMainWindowAboveTarget)},
        ani_native_function {"setMainWindowRaiseByClickEnabledSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetMainWindowRaiseByClickEnabled)},
        ani_native_function {"setWindowTopmost", "lz:",
            reinterpret_cast<void *>(SetWindowTopmost)},
        ani_native_function {"setWindowFocusableSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowFocusable)},
        ani_native_function {"getSubWindowZLevelSync", "l:i",
            reinterpret_cast<void *>(AniWindow::GetSubWindowZLevel)},
        ani_native_function {"isFocusedSync", "l:z",
            reinterpret_cast<void *>(AniWindow::IsFocused)},
        ani_native_function {"setSubWindowZLevelSync", "li:",
            reinterpret_cast<void *>(AniWindow::SetSubWindowZLevel)},
        ani_native_function {"raiseAboveTargetSync", "li:",
            reinterpret_cast<void *>(AniWindow::RaiseAboveTarget)},
        ani_native_function {"raiseToAppTopSync", "l:",
            reinterpret_cast<void *>(AniWindow::RaiseToAppTop)},
        ani_native_function {"setTopmostSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetTopmost)},
        ani_native_function {"setReceiveDragEventEnabled", "lz:",
            reinterpret_cast<void *>(AniWindow::SetReceiveDragEventEnabled)},
        ani_native_function {"isReceiveDragEventEnabled", "l:z",
            reinterpret_cast<void *>(AniWindow::IsReceiveDragEventEnabled)},
        ani_native_function {"setSeparationTouchEnabled", "lz:",
            reinterpret_cast<void *>(AniWindow::SetSeparationTouchEnabled)},
        ani_native_function {"isSeparationTouchEnabled", "l:z",
            reinterpret_cast<void *>(AniWindow::IsSeparationTouchEnabled)},
        ani_native_function {"requestFocusSync", "lz:",
            reinterpret_cast<void *>(AniWindow::RequestFocus)},
        ani_native_function {"setSubWindowModal", "lz:",
            reinterpret_cast<void *>(AniWindow::SetSubWindowModal)},
        ani_native_function {"setSubWindowModalType", "lzi:",
            reinterpret_cast<void *>(AniWindow::SetSubWindowModalType)},
        ani_native_function {"isWindowHighlightedSync", "l:z",
            reinterpret_cast<void *>(AniWindow::IsWindowHighlighted)},
        ani_native_function {"setContentAspectRatio", "ldzz:",
            reinterpret_cast<void *>(AniWindow::SetContentAspectRatio)},
        ani_native_function {"opacity", "ld:",
            reinterpret_cast<void *>(AniWindow::Opacity)},
        ani_native_function {"scale", "lC{@ohos.window.window.ScaleOptions}:",
            reinterpret_cast<void *>(AniWindow::Scale)},
        ani_native_function {"translate", "lC{@ohos.window.window.TranslateOptions}:",
            reinterpret_cast<void *>(AniWindow::Translate)},
        ani_native_function {"rotate", "lC{@ohos.window.window.RotateOptions}:",
            reinterpret_cast<void *>(AniWindow::Rotate)},
        ani_native_function {"setShadow", "ldC{std.core.String}C{std.core.Double}C{std.core.Double}:",
            reinterpret_cast<void *>(AniWindow::SetShadow)},
        ani_native_function {"setCornerRadiusSync", "ld:",
            reinterpret_cast<void *>(AniWindow::SetCornerRadius)},
        ani_native_function {"setBlur", "ld:",
            reinterpret_cast<void *>(AniWindow::SetBlur)},
        ani_native_function {"setBackdropBlurStyle", "li:",
            reinterpret_cast<void *>(AniWindow::SetBackdropBlurStyle)},
        ani_native_function {"setBackdropBlur", "ld:",
            reinterpret_cast<void *>(AniWindow::SetBackdropBlur)},
        ani_native_function {"getWindowCornerRadius", "l:d",
            reinterpret_cast<void *>(AniWindow::GetWindowCornerRadius)},
        ani_native_function {"setWindowCornerRadiusSync", "ld:",
            reinterpret_cast<void *>(AniWindow::SetWindowCornerRadius)},
        ani_native_function {"setWindowShadowRadius", "ld:",
            reinterpret_cast<void *>(AniWindow::SetWindowShadowRadius)},
        ani_native_function {"onSync", nullptr,
            reinterpret_cast<void *>(AniWindow::RegisterWindowCallback)},
        ani_native_function {"onNoInteractionDetected", "lC{std.core.String}lC{std.core.Object}:",
            reinterpret_cast<void *>(AniWindow::RegisterNoInteractionDetectedCallback)},
        ani_native_function {"offSync", nullptr,
            reinterpret_cast<void *>(AniWindow::UnregisterWindowCallback)},
        ani_native_function {"showWindowSync", nullptr,
            reinterpret_cast<void *>(AniWindow::ShowWindow)},
        ani_native_function {"showWindowWithOptions", "lC{@ohos.window.window.ShowWindowOptions}:",
            reinterpret_cast<void *>(AniWindow::ShowWindowWithOptions)},
        ani_native_function {"getParentWindow", "l:C{@ohos.window.window.Window}",
            reinterpret_cast<void *>(GetParentWindow)},
        ani_native_function {"setParentWindow", "li:",
            reinterpret_cast<void *>(SetParentWindow)},
        ani_native_function {"setWindowTitle", "lC{std.core.String}:",
            reinterpret_cast<void *>(AniWindow::SetWindowTitle)},
        ani_native_function {"setWindowTitleMoveEnabled", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowTitleMoveEnabled)},
        ani_native_function {"setHandwritingFlag", "lz:",
            reinterpret_cast<void *>(AniWindow::SetHandwritingFlag)},
        ani_native_function {"getWindowDecorVisible", "l:z",
            reinterpret_cast<void *>(AniWindow::GetWindowDecorVisible)},
        ani_native_function {"getTitleButtonRect", "l:C{@ohos.window.window.TitleButtonRect}",
            reinterpret_cast<void *>(AniWindow::GetTitleButtonRect)},
        ani_native_function {"setTitleButtonVisible", "lzzzz:",
            reinterpret_cast<void *>(AniWindow::SetTitleButtonVisible)},
        ani_native_function {"setTitleAndDockHoverShown", "lC{std.core.Boolean}C{std.core.Boolean}:",
            reinterpret_cast<void *>(AniWindow::SetTitleAndDockHoverShown)},
        ani_native_function {"getDecorButtonStyle", "l:C{@ohos.window.window.DecorButtonStyle}",
            reinterpret_cast<void *>(AniWindow::GetDecorButtonStyle)},
        ani_native_function {"setDecorButtonStyle", "lC{@ohos.window.window.DecorButtonStyle}:",
            reinterpret_cast<void *>(SetDecorButtonStyle)},
        ani_native_function {"setWindowTitleButtonVisible", "lC{@ohos.window.window.WindowTitleButtonVisibleParam}:",
            reinterpret_cast<void *>(SetWindowTitleButtonVisible)},
        ani_native_function {"bindDialogTargetWithRemoteObjectSync", nullptr,
            reinterpret_cast<void *>(AniWindow::BindDialogTarget)},
        ani_native_function {"bindDialogTargetWithRequestInfoSync", nullptr,
            reinterpret_cast<void *>(AniWindow::BindDialogTarget)},
        ani_native_function {"destroyWindowSync", nullptr,
            reinterpret_cast<void *>(AniWindow::DestroyWindow)},
        ani_native_function {"isWindowShowingSync", nullptr,
            reinterpret_cast<void *>(AniWindow::IsWindowShowing)},
        ani_native_function {"hideWithAnimationSync", nullptr,
            reinterpret_cast<void *>(AniWindow::HideWithAnimation)},
        ani_native_function {"showWithAnimationSync", nullptr,
            reinterpret_cast<void *>(AniWindow::ShowWithAnimation)},
        ani_native_function {"getTransitionControllerSync", "l:C{@ohos.window.window.TransitionController}",
            reinterpret_cast<void *>(AniWindow::GetTransitionController)},
        ani_native_function {"resizeAsync", "lii:",
            reinterpret_cast<void *>(WindowResizeAsync)},
        ani_native_function {"setWindowLimits",
            "lC{@ohos.window.window.WindowLimits}C{std.core.Boolean}:C{@ohos.window.window.WindowLimits}",
            reinterpret_cast<void *>(WindowSetWindowLimits)},
        ani_native_function {"getWindowLimits", "l:C{@ohos.window.window.WindowLimits}",
            reinterpret_cast<void *>(WindowGetWindowLimits)},
        ani_native_function {"getWindowLimitsVP", "l:C{@ohos.window.window.WindowLimits}",
            reinterpret_cast<void *>(WindowGetWindowLimitsVP)},
        ani_native_function {"setAspectRatio", "ld:",
            reinterpret_cast<void *>(WindowSetAspectRatio)},
        ani_native_function {"resetAspectRatio", "l:",
            reinterpret_cast<void *>(WindowResetAspectRatio)},
        ani_native_function {"getWindowStatus", "l:i",
            reinterpret_cast<void *>(AniWindow::GetWindowStatus)},
        ani_native_function {"minimize", "l:",
            reinterpret_cast<void *>(AniWindow::Minimize)},
        ani_native_function {"maximize", "lC{@ohos.window.window.MaximizePresentation}C{std.core.Boolean}:",
            reinterpret_cast<void *>(AniWindow::Maximize)},
        ani_native_function {"startMoving", "l:",
            reinterpret_cast<void *>(AniWindow::StartMoving)},
        ani_native_function {"startMoveWindowWithCoordinate", "lii:",
            reinterpret_cast<void *>(AniWindow::StartMoveWindowWithCoordinate)},
        ani_native_function {"stopMoving", "l:",
            reinterpret_cast<void *>(AniWindow::StopMoving)},
        ani_native_function {"setResizeByDragEnabled", "lz:",
            reinterpret_cast<void *>(WindowSetResizeByDragEnabled)},
        ani_native_function {"enableDrag", "lz:",
            reinterpret_cast<void *>(WindowEnableDrag)},
        ani_native_function {"moveWindowToGlobal", "liiC{@ohos.window.window.MoveConfiguration}:",
            reinterpret_cast<void *>(WindowMoveWindowToGlobal)},
        ani_native_function {"moveWindowToAsync", "liiC{@ohos.window.window.MoveConfiguration}:",
            reinterpret_cast<void *>(WindowMoveWindowToAsync)},
        ani_native_function {"setWindowMode", "lC{@ohos.window.window.WindowMode}:",
            reinterpret_cast<void *>(WindowSetWindowMode)},
        ani_native_function {"setForbidSplitMove", "lz:",
            reinterpret_cast<void *>(WindowSetForbidSplitMove)},
        ani_native_function {"setFollowParentWindowLayoutEnabled", "lz:",
            reinterpret_cast<void *>(WindowSetFollowParentWindowLayoutEnabled)},
        ani_native_function {"setFollowParentMultiScreenPolicy", "lz:",
            reinterpret_cast<void *>(WindowSetFollowParentMultiScreenPolicy)},
        ani_native_function {"moveWindowToGlobalDisplay", "lii:",
            reinterpret_cast<void *>(WindowMoveWindowToGlobalDisplay)},
        ani_native_function {"clientToGlobalDisplay", "lii:C{@ohos.window.window.Position}",
            reinterpret_cast<void *>(WindowClientToGlobalDisplay)},
        ani_native_function {"globalDisplayToClient", "lii:C{@ohos.window.window.Position}",
            reinterpret_cast<void *>(WindowGlobalDisplayToClient)},
        ani_native_function {"setRotationLocked", "lz:",
            reinterpret_cast<void *>(AniWindow::SetRotationLocked)},
        ani_native_function {"getRotationLocked", "l:z",
            reinterpret_cast<void *>(AniWindow::GetRotationLocked)},
        ani_native_function {"isInFreeWindowMode", "l:z",
            reinterpret_cast<void *>(AniWindow::IsInFreeWindowMode)},
        ani_native_function {"setWindowDelayRaiseOnDrag", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowDelayRaiseOnDrag)},
        ani_native_function {"setRelativePositionToParentWindowEnabled",
            "lzC{@ohos.window.window.WindowAnchor}C{std.core.Int}C{std.core.Int}:",
            reinterpret_cast<void *>(AniWindow::SetRelativePositionToParentWindowEnabled)},
        ani_native_function {"setDefaultDensityEnabled", "lz:",
            reinterpret_cast<void *>(AniWindow::SetDefaultDensityEnabled)},
        ani_native_function {"setWindowContainerColor", "lC{std.core.String}C{std.core.String}:",
            reinterpret_cast<void *>(AniWindow::SetWindowContainerColor)},
        ani_native_function {"setWindowContainerModalColor", "lC{std.core.String}C{std.core.String}:",
            reinterpret_cast<void *>(AniWindow::SetWindowContainerModalColor)},
        ani_native_function {"getWindowStateSnapshotSync", "l:C{std.core.String}",
            reinterpret_cast<void *>(AniWindow::GetWindowStateSnapshot)},
        ani_native_function {"isMainWindowFullScreenAcrossDisplaysSync", "l:z",
            reinterpret_cast<void *>(AniWindow::IsMainWindowFullScreenAcrossDisplays)},
        ani_native_function {"setWindowShadowEnabledSync", "lz:",
            reinterpret_cast<void *>(AniWindow::SetWindowShadowEnabled)},
        ani_native_function {"isImmersiveLayoutSync", "l:z",
            reinterpret_cast<void *>(AniWindow::IsImmersiveLayout)},
        ani_native_function {"setWindowTransitionAnimation",
            "lC{@ohos.window.window.WindowTransitionType}C{@ohos.window.window.TransitionAnimation}:",
            reinterpret_cast<void *>(AniWindow::SetWindowTransitionAnimation)},
        ani_native_function {"getWindowTransitionAnimationSync",
            "lC{@ohos.window.window.WindowTransitionType}:C{@ohos.window.window.TransitionAnimation}",
            reinterpret_cast<void *>(AniWindow::GetWindowTransitionAnimation)},
        ani_native_function {"keepKeyboardOnFocusSync", "lz:",
            reinterpret_cast<void *>(AniWindow::KeepKeyboardOnFocus)},
        ani_native_function {"setWindowBrightness", "ld:",
            reinterpret_cast<void *>(SetWindowBrightness)},
        ani_native_function {"getWindowColorSpace", "l:i",
            reinterpret_cast<void *>(GetWindowColorSpace)},
        ani_native_function {"setWakeUpScreen", "lz:",
            reinterpret_cast<void *>(SetWakeUpScreen)},
        ani_native_function {"setSnapshotSkip", "lz:",
            reinterpret_cast<void *>(SetSnapshotSkip)},
        ani_native_function {"snapshotIgnorePrivacy", "l:C{@ohos.multimedia.image.image.PixelMap}",
            reinterpret_cast<void *>(SnapshotIgnorePrivacy)},
        ani_native_function {"getStatusBarProperty", "l:C{@ohos.window.window.StatusBarProperty}",
            reinterpret_cast<void *>(GetStatusBarProperty)},
        ani_native_function {"setStatusBarColor", "ll:",
            reinterpret_cast<void *>(SetStatusBarColor)},
        ani_native_function {"isSystemAvoidAreaEnabled", "l:z",
            reinterpret_cast<void *>(IsSystemAvoidAreaEnabled)},
        ani_native_function {"setSystemAvoidAreaEnabled", "lz:",
            reinterpret_cast<void *>(SetSystemAvoidAreaEnabled)},
        ani_native_function {"getWindowDensityInfo", "l:C{@ohos.window.window.WindowDensityInfo}",
            reinterpret_cast<void *>(GetWindowDensityInfo)},
        ani_native_function {"getImmersiveModeEnabledState", "l:z",
            reinterpret_cast<void *>(GetImmersiveModeEnabledState)},
        ani_native_function {"setWindowGrayScale", "ld:",
            reinterpret_cast<void *>(SetWindowGrayScale)},
        ani_native_function {"getWindowSystemBarProperties", "l:C{@ohos.window.window.SystemBarProperties}",
            reinterpret_cast<void *>(GetWindowSystemBarProperties)},
        ani_native_function {"isGestureBackEnabled", "l:z",
            reinterpret_cast<void *>(IsGestureBackEnabled)},
        ani_native_function {"setGestureBackEnabled", "lz:",
            reinterpret_cast<void *>(SetGestureBackEnabled)},
        ani_native_function {"setSingleFrameComposerEnabled", "lz:",
            reinterpret_cast<void *>(SetSingleFrameComposerEnabled)},
        ani_native_function {"createSubWindowWithOptionsSync",
            "lC{std.core.String}C{@ohos.window.window.SubWindowOptions}:C{@ohos.window.window.Window}",
            reinterpret_cast<void *>(AniWindow::CreateSubWindowWithOptions)},
        ani_native_function {"hideSync", "l:",
            reinterpret_cast<void *>(AniWindow::Hide)},
        ani_native_function {"restore", "l:",
            reinterpret_cast<void *>(Restore)},
        ani_native_function {"restoreMainWindow", "lC{std.core.Record}:",
            reinterpret_cast<void *>(AniWindow::RestoreMainWindow)},
    };
    for (auto method : methods) {
        if ((ret = env->Class_BindNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    std::array staticMethods = {
        ani_native_function {"nativeTransferStatic", "C{std.interop.ESValue}:C{std.core.Object}",
            reinterpret_cast<void *>(AniWindow::NativeTransferStatic)},
        ani_native_function {"nativeTransferDynamic", "l:C{std.interop.ESValue}",
            reinterpret_cast<void *>(AniWindow::NativeTransferDynamic)}
    };
    for (auto method : staticMethods) {
        if ((ret = env->Class_BindStaticNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window static method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    *result = ANI_VERSION_1;

    ani_namespace ns;
    if ((ret = env->FindNamespace("@ohos.window.window", &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find ns %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array functions = {
        ani_native_function {"CreateWindow", "l:C{@ohos.window.window.WindowInternal}",
            reinterpret_cast<void *>(WindowCreate)},
    };
    if ((ret = env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size())) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] bind ns window func %{public}u", ret);
    }
    return ANI_OK;
}