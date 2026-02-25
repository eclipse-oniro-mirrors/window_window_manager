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
#include "screen_ani.h"

#include "ani_err_utils.h"
#include "dm_common.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_screen.h"
#include "screen_ani_utils.h"
#include "window_manager_hilog.h"

#ifdef XPOWER_EVENT_ENABLE
#include "xpower_event_js.h"
#endif // XPOWER_EVENT_ENABLE
 
namespace OHOS {
namespace Rosen {
ScreenAni::ScreenAni(const sptr<Screen>& screen) : screen_(screen)
{
}
 
void ScreenAni::SetDensityDpi(ani_env* env, ani_object obj, ani_double densityDpi)
{
    ani_long screenNativeRef;
    if (ANI_OK != env->Object_GetFieldByName_Long(obj, "screenNativeObj", &screenNativeRef)) {
        TLOGE(WmsLogTag::DMS, "[ANI] screenAni native null ptr");
        return;
    }
    ScreenAni* screenAni = reinterpret_cast<ScreenAni*>(screenNativeRef);
    if (screenAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] screenAni is nullptr");
        return;
    }
    screenAni->OnSetDensityDpi(env, obj, densityDpi);
}
 
void ScreenAni::OnSetDensityDpi(ani_env* env, ani_object obj, ani_double densityDpi)
{
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(screen_->SetDensityDpi(static_cast<uint32_t>(densityDpi)));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnSetDensityDpi fail");
        AniErrUtils::ThrowBusinessError(env, ret, "JsScreen::OnSetDensityDpi failed.");
    }
}

ani_boolean ScreenAni::TransferStatic(ani_env* env, ani_object obj, ani_object input, ani_object screenAniObj)
{
    TLOGI(WmsLogTag::DMS, "begin");
    void* unwrapResult = nullptr;
    auto ret = arkts_esvalue_unwrap(env, input, &unwrapResult);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to unwrap input, ret: %{public}d", ret);
        return false;
    }
    if (unwrapResult == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] unwrapResult is nullptr");
        return false;
    }
    JsScreen* jsScreen = static_cast<JsScreen*>(unwrapResult);
    if (jsScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] jsScreen is nullptr");
        return false;
    }
    
    sptr<Screen> screen = jsScreen->GetScreen();
    if (ScreenAniUtils::ConvertScreen(env, screen, screenAniObj) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] convert screen failed");
        return false;
    }
    return true;
}
 
ani_object ScreenAni::TransferDynamic(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DMS, "begin");
    ScreenAni* aniScreen = reinterpret_cast<ScreenAni*>(nativeObj);
    if (aniScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] aniScreen is nullptr");
        return nullptr;
    }
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(env, &napiEnv)) {
        TLOGE(WmsLogTag::DMS, "arkts_napi_scope_open failed");
        return nullptr;
    }
    
    sptr<OHOS::Rosen::Screen> screen = aniScreen->GetScreen();
    napi_value jsScreen = CreateJsScreenObject(napiEnv, screen);
    hybridgref ref = nullptr;
    if (!hybridgref_create_from_napi(napiEnv, jsScreen, &ref)) {
        TLOGE(WmsLogTag::DMS, "hybridgref_create_from_napi failed");
        return nullptr;
    }
    ani_object result = nullptr;
    if (!hybridgref_get_esvalue(env, ref, &result)) {
        hybridgref_delete_from_napi(napiEnv, ref);
        TLOGE(WmsLogTag::DMS, "hybridgref_get_esvalue failed");
        return nullptr;
    }
    hybridgref_delete_from_napi(napiEnv, ref);
    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TLOGE(WmsLogTag::DMS, "arkts_napi_scope_close_n failed");
        return nullptr;
    }
    return result;
}

void ScreenAni::SetScreenActiveMode(ani_env* env, ani_object obj, ani_long modeIndex)
{
    TLOGI(WmsLogTag::DMS, "begin");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    ani_long screenNativeRef;
    ani_status ret = env->Object_GetFieldByName_Long(obj, "screenNativeObj", &screenNativeRef);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenAni native failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "get screenNativeObj failed.");
        return;
    }
#ifdef XPOWER_EVENT_ENABLE
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "EPS_LCD_FREQ");
#endif // XPOWER_EVENT_ENABLE
    ScreenAni* screenAni = reinterpret_cast<ScreenAni*>(screenNativeRef);
    if (screenAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] screenAni is nullptr");
        return;
    }
    screenAni->OnSetScreenActiveMode(env, obj, modeIndex);
}

void ScreenAni::OnSetScreenActiveMode(ani_env* env, ani_object obj, ani_long modeIndex)
{
    if (screen_ ==nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(screen_->SetScreenActiveMode(static_cast<uint32_t>(modeIndex)));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Set screen active mode fail, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, ret, "Screen::SetScreenActiveMode failed.");
    }
}

void ScreenAni::SetOrientation(ani_env* env, ani_object obj, ani_enum_item orientationAni)
{
    TLOGI(WmsLogTag::DMS, "begin");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    if (orientationAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] orientationAni is nullptr");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "orientationAni is nullptr.");
        return;
    }
    ani_long screenNativeRef;
    ani_status ret = env->Object_GetFieldByName_Long(obj, "screenNativeObj", &screenNativeRef);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenAni native failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "get screenNativeObj failed.");
        return;
    }
    ScreenAni* screenAni = reinterpret_cast<ScreenAni*>(screenNativeRef);
    if (screenAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] screenAni is nullptr");
        return;
    }
    screenAni->OnSetOrientation(env, obj, orientationAni);
}

void ScreenAni::OnSetOrientation(ani_env* env, ani_object obj, ani_enum_item orientationAni)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    ani_int orientationInt = 0;
    ani_status ret = env->EnumItem_GetValue_Int(orientationAni, &orientationInt);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Get orientation failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to Get orientation");
        return;
    }
    Orientation orientation = static_cast<Orientation>(orientationInt);
    if (orientation < Orientation::BEGIN || orientation > Orientation::END) {
        TLOGE(WmsLogTag::DMS, "Orientation param error! orientation value must from enum Orientation");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            "orientation value must from enum Orientation");
        return;
    }
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(screen_->SetOrientation(orientation));
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Set screen orientation failed");
        AniErrUtils::ThrowBusinessError(env, res, "Screen::SetOrientation failed.");
    }
}
}
}