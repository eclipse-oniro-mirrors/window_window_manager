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

#include "js_display.h"

#include <cinttypes>
#include <hitrace_meter.h>
#include <map>
#include <set>

#include "cutout_info.h"
#include "display.h"
#include "display_info.h"
#include "window_manager_hilog.h"
#include "display_manager.h"
#include "singleton_container.h"
#include "js_display_listener.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ONE = 1;
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "JsDisplay"};
    const std::map<DisplayState,      DisplayStateMode> NATIVE_TO_JS_DISPLAY_STATE_MAP {
        { DisplayState::UNKNOWN,      DisplayStateMode::STATE_UNKNOWN      },
        { DisplayState::OFF,          DisplayStateMode::STATE_OFF          },
        { DisplayState::ON,           DisplayStateMode::STATE_ON           },
        { DisplayState::DOZE,         DisplayStateMode::STATE_DOZE         },
        { DisplayState::DOZE_SUSPEND, DisplayStateMode::STATE_DOZE_SUSPEND },
        { DisplayState::VR,           DisplayStateMode::STATE_VR           },
        { DisplayState::ON_SUSPEND,   DisplayStateMode::STATE_ON_SUSPEND   },
    };

using GraphicCM_ColorSpaceType = enum {
    GRAPHIC_CM_COLORSPACE_NONE,

    GRAPHIC_CM_BT601_EBU_FULL      = 2 | (1 << 8) | (2 << 16) | (1 << 21),  // COLORPRIMARIES_BT601_P | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_P << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT601_SMPTE_C_FULL  = 3 | (1 << 8) | (3 << 16) | (1 << 21),  // COLORPRIMARIES_BT601_N | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_N << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT709_FULL          = 1 | (1 << 8) | (1 << 16) | (1 << 21),  // COLORPRIMARIES_BT709   | (TRANSFUNC_BT709 << 8) | (MATRIX_BT709   << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT2020_HLG_FULL     = 4 | (5 << 8) | (4 << 16) | (1 << 21),  // COLORPRIMARIES_BT2020  | (TRANSFUNC_HLG   << 8) | (MATRIX_BT2020  << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT2020_PQ_FULL      = 4 | (4 << 8) | (4 << 16) | (1 << 21),  // COLORPRIMARIES_BT2020  | (TRANSFUNC_PQ    << 8) | (MATRIX_BT2020  << 16) | (RANGE_FULL << 21)

    GRAPHIC_CM_BT601_EBU_LIMIT     = 2 | (1 << 8) | (2 << 16) | (2 << 21),  // COLORPRIMARIES_BT601_P | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_P << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT601_SMPTE_C_LIMIT = 3 | (1 << 8) | (3 << 16) | (2 << 21),  // COLORPRIMARIES_BT601_N | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_N << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT709_LIMIT         = 1 | (1 << 8) | (1 << 16) | (2 << 21),  // COLORPRIMARIES_BT709   | (TRANSFUNC_BT709 << 8) | (MATRIX_BT709   << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT2020_HLG_LIMIT    = 4 | (5 << 8) | (4 << 16) | (2 << 21),  // COLORPRIMARIES_BT2020  | (TRANSFUNC_HLG   << 8) | (MATRIX_BT2020  << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT2020_PQ_LIMIT     = 4 | (4 << 8) | (4 << 16) | (2 << 21),  // COLORPRIMARIES_BT2020  | (TRANSFUNC_PQ    << 8) | (MATRIX_BT2020  << 16) | (RANGE_LIMITED << 21)

    GRAPHIC_CM_SRGB_FULL           = 1 | (2 << 8) | (3 << 16) | (1 << 21),  // COLORPRIMARIES_SRGB     | (TRANSFUNC_SRGB     << 8) | (MATRIX_BT601_N  << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_P3_FULL             = 6 | (2 << 8) | (3 << 16) | (1 << 21),  // COLORPRIMARIES_P3_D65   | (TRANSFUNC_SRGB     << 8) | (MATRIX_P3       << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_P3_HLG_FULL         = 6 | (5 << 8) | (3 << 16) | (1 << 21),  // COLORPRIMARIES_P3_D65   | (TRANSFUNC_HLG      << 8) | (MATRIX_P3       << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_P3_PQ_FULL          = 6 | (4 << 8) | (3 << 16) | (1 << 21),  // COLORPRIMARIES_P3_D65   | (TRANSFUNC_PQ       << 8) | (MATRIX_P3       << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_ADOBERGB_FULL       = 23 | (6 << 8) | (0 << 16) | (1 << 21), // COLORPRIMARIES_ADOBERGB | (TRANSFUNC_ADOBERGB << 8) | (MATRIX_ADOBERGB << 16) | (RANGE_FULL << 21)

    GRAPHIC_CM_SRGB_LIMIT          = 1 | (2 << 8) | (3 << 16) | (2 << 21),  // COLORPRIMARIES_SRGB     | (TRANSFUNC_SRGB     << 8) | (MATRIX_BT601_N  << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_P3_LIMIT            = 6 | (2 << 8) | (3 << 16) | (2 << 21),  // COLORPRIMARIES_P3_D65   | (TRANSFUNC_SRGB     << 8) | (MATRIX_P3       << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_P3_HLG_LIMIT        = 6 | (5 << 8) | (3 << 16) | (2 << 21),  // COLORPRIMARIES_P3_D65   | (TRANSFUNC_HLG      << 8) | (MATRIX_P3       << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_P3_PQ_LIMIT         = 6 | (4 << 8) | (3 << 16) | (2 << 21),  // COLORPRIMARIES_P3_D65   | (TRANSFUNC_PQ       << 8) | (MATRIX_P3       << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_ADOBERGB_LIMIT      = 23 | (6 << 8) | (0 << 16) | (2 << 21), // COLORPRIMARIES_ADOBERGB | (TRANSFUNC_ADOBERGB << 8) | (MATRIX_ADOBERGB << 16) | (RANGE_LIMITED << 21)

    GRAPHIC_CM_LINEAR_SRGB         = 1 | (3 << 8),                          // COLORPRIMARIES_SRGB   | (TRANSFUNC_LINEAR << 8)
    GRAPHIC_CM_LINEAR_BT709        = 1 | (3 << 8),                          // equal to GRAPHIC_CM_LINEAR_SRGB
    GRAPHIC_CM_LINEAR_P3           = 6 | (3 << 8),                          // COLORPRIMARIES_P3_D65 | (TRANSFUNC_LINEAR << 8)
    GRAPHIC_CM_LINEAR_BT2020       = 4 | (3 << 8),                          // COLORPRIMARIES_BT2020 | (TRANSFUNC_LINEAR << 8)

    GRAPHIC_CM_DISPLAY_SRGB        = 1 | (2 << 8) | (3 << 16) | (1 << 21),  // equal to GRAPHIC_CM_SRGB_FULL
    GRAPHIC_CM_DISPLAY_P3_SRGB     = 6 | (2 << 8) | (3 << 16) | (1 << 21),  // equal to GRAPHIC_CM_P3_FULL
    GRAPHIC_CM_DISPLAY_P3_HLG      = 6 | (5 << 8) | (3 << 16) | (1 << 21),  // equal to GRAPHIC_CM_P3_HLG_FULL
    GRAPHIC_CM_DISPLAY_P3_PQ       = 6 | (4 << 8) | (3 << 16) | (1 << 21),  // equal to GRAPHIC_CM_P3_PQ_FULL
    GRAPHIC_CM_DISPLAY_BT2020_SRGB = 4 | (2 << 8) | (4 << 16) | (1 << 21),  // COLORPRIMARIES_BT2020   | (TRANSFUNC_SRGB << 8)     | (MATRIX_BT2020 << 16)   | (RANGE_FULL << 21)
    GRAPHIC_CM_DISPLAY_BT2020_HLG  = 4 | (5 << 8) | (4 << 16) | (1 << 21),  // equal to GRAPHIC_CM_BT2020_HLG_FULL
    GRAPHIC_CM_DISPLAY_BT2020_PQ   = 4 | (4 << 8) | (4 << 16) | (1 << 21)   // equal to GRAPHIC_CM_BT2020_PQ_FULL
};

typedef enum : uint32_t {
    NOT_SUPPORT_HDR = 0,
    VIDEO_HLG,
    VIDEO_HDR10,
    VIDEO_HDR_VIVID,
    IMAGE_HDR_VIVID_DUAL,
    IMAGE_HDR_VIVID_SINGLE,
    IMAGE_HDR_ISO_DUAL,
    IMAGE_HDR_ISO_SINGLE,
} ScreenHDRFormat;

const std::map<GraphicCM_ColorSpaceType, ColorSpace> NATIVE_TO_JS_COLOR_SPACE_TYPE_MAP {
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_COLORSPACE_NONE,        ColorSpace::UNKNOWN },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_ADOBERGB_FULL,          ColorSpace::ADOBE_RGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_ADOBERGB_LIMIT,         ColorSpace::ADOBE_RGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_HLG_FULL,        ColorSpace::BT2020_HLG },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_HLG_LIMIT,       ColorSpace::BT2020_HLG },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_HLG,     ColorSpace::BT2020_HLG },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_PQ_FULL,         ColorSpace::BT2020_PQ },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_PQ_LIMIT,        ColorSpace::BT2020_PQ },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_PQ,      ColorSpace::BT2020_PQ },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_EBU_FULL,         ColorSpace::BT601_EBU },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_EBU_LIMIT,        ColorSpace::BT601_EBU },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_SMPTE_C_FULL,     ColorSpace::BT601_SMPTE_C },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_SMPTE_C_LIMIT,    ColorSpace::BT601_SMPTE_C },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT709_FULL,             ColorSpace::BT709 },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_BT709_LIMIT,            ColorSpace::BT709 },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_HLG_FULL,            ColorSpace::P3_HLG },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_HLG_LIMIT,           ColorSpace::P3_HLG },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_HLG,         ColorSpace::P3_HLG },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_PQ_FULL,             ColorSpace::P3_PQ },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_PQ_LIMIT,            ColorSpace::P3_PQ },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_PQ,          ColorSpace::P3_PQ },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_FULL,                ColorSpace::DISPLAY_P3 },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_LIMIT,               ColorSpace::DISPLAY_P3 },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_SRGB,        ColorSpace::DISPLAY_P3 },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_SRGB_FULL,              ColorSpace::SRGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_SRGB_LIMIT,             ColorSpace::SRGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_SRGB,           ColorSpace::SRGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_SRGB,            ColorSpace::LINEAR_SRGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_BT709,           ColorSpace::LINEAR_SRGB },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_P3,              ColorSpace::LINEAR_P3 },
    { GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_BT2020,          ColorSpace::LINEAR_BT2020 },
};

const std::map<ScreenHDRFormat, HDRFormat> NATIVE_TO_JS_HDR_FORMAT_TYPE_MAP {
    { ScreenHDRFormat::NOT_SUPPORT_HDR,             HDRFormat::NONE },
    { ScreenHDRFormat::VIDEO_HLG,                   HDRFormat::VIDEO_HLG },
    { ScreenHDRFormat::VIDEO_HDR10,                 HDRFormat::VIDEO_HDR10 },
    { ScreenHDRFormat::VIDEO_HDR_VIVID,             HDRFormat::VIDEO_HDR_VIVID },
    { ScreenHDRFormat::IMAGE_HDR_VIVID_DUAL,        HDRFormat::IMAGE_HDR_VIVID_DUAL },
    { ScreenHDRFormat::IMAGE_HDR_VIVID_SINGLE,      HDRFormat::IMAGE_HDR_VIVID_SINGLE },
    { ScreenHDRFormat::IMAGE_HDR_ISO_DUAL,          HDRFormat::IMAGE_HDR_ISO_DUAL },
    { ScreenHDRFormat::IMAGE_HDR_ISO_SINGLE,        HDRFormat::IMAGE_HDR_ISO_SINGLE },
};
}

static thread_local std::map<DisplayId, std::shared_ptr<NativeReference>> g_JsDisplayMap;
std::map<std::string, std::map<std::unique_ptr<NativeReference>, sptr<JsDisplayListener>>> jsCbMap_;
std::mutex mtx_;
std::recursive_mutex g_mutex;

JsDisplay::JsDisplay(const sptr<Display>& display) : display_(display)
{
}

JsDisplay::~JsDisplay()
{
    WLOGI("JsDisplay::~JsDisplay is called");
}

void JsDisplay::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("JsDisplay::Finalizer is called");
    auto jsDisplay = std::unique_ptr<JsDisplay>(static_cast<JsDisplay*>(data));
    if (jsDisplay == nullptr) {
        WLOGFE("JsDisplay::Finalizer jsDisplay is null");
        return;
    }
    sptr<Display> display = jsDisplay->display_;
    if (display == nullptr) {
        WLOGFE("JsDisplay::Finalizer display is null");
        return;
    }
    DisplayId displayId = display->GetId();
    WLOGI("JsDisplay::Finalizer displayId : %{public}" PRIu64"", displayId);
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    if (g_JsDisplayMap.find(displayId) != g_JsDisplayMap.end()) {
        WLOGI("JsDisplay::Finalizer Display is destroyed: %{public}" PRIu64"", displayId);
        g_JsDisplayMap.erase(displayId);
    }
}

napi_value JsDisplay::GetCutoutInfo(napi_env env, napi_callback_info info)
{
    WLOGD("GetCutoutInfo is called");
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnGetCutoutInfo(env, info) : nullptr;
}

napi_value JsDisplay::GetAvailableArea(napi_env env, napi_callback_info info)
{
    WLOGI("GetAvailableArea is called");
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnGetAvailableArea(env, info) : nullptr;
}

napi_value JsDisplay::RegisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnRegisterDisplayManagerCallback(env, info) : nullptr;
}

napi_value JsDisplay::UnregisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnUnregisterDisplayManagerCallback(env, info) : nullptr;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

bool IfCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("IfCallbackRegistered methodName %{public}s not registered!", type.c_str());
        return false;
    }

    for (auto& iter : jsCbMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter.first->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("IfCallbackRegistered callback already registered!");
            return true;
        }
    }
    return false;
}

napi_value JsDisplay::OnRegisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGD("OnRegisterDisplayManagerCallback is called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("JsDisplayManager Params not match: %{public}zu", argc);
        std::string errMsg = "Invalid args count, need 2 args";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        std::string errMsg = "Failed to convert parameter to callbackType";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        WLOGFE("Failed to convert parameter to callbackType");
        return NapiGetUndefined(env);
    }
    napi_value value = argv[INDEX_ONE];
    if (value == nullptr) {
        WLOGI("OnRegisterDisplayManagerCallback info->argv[1] is nullptr");
        std::string errMsg = "OnRegisterDisplayManagerCallback is nullptr";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    if (!NapiIsCallable(env, value)) {
        WLOGI("OnRegisterDisplayManagerCallback info->argv[1] is not callable");
        std::string errMsg = "OnRegisterDisplayManagerCallback is not callable";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(RegisterDisplayListenerWithType(env, cbType, value));
    if (ret != DmErrorCode::DM_OK) {
        WLOGFE("Failed to register display listener with type");
        std::string errMsg = "Failed to register display listener with type";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

DMError JsDisplay::RegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (IfCallbackRegistered(env, type, value)) {
        WLOGFE("RegisterDisplayListenerWithType callback already registered!");
        return DMError::DM_OK;
    }
    std::unique_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    sptr<JsDisplayListener> displayListener = new(std::nothrow) JsDisplayListener(env);
    DMError ret = DMError::DM_OK;
    if (displayListener == nullptr) {
        WLOGFE("displayListener is nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (type == EVENT_AVAILABLE_AREA_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterAvailableAreaListener(displayListener);
    } else {
        WLOGFE("RegisterDisplayListenerWithType failed, %{public}s not support", type.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (ret != DMError::DM_OK) {
        WLOGFE("RegisterDisplayListenerWithType failed, ret: %{public}u", ret);
        return ret;
    }
    displayListener->AddCallback(type, value);
    jsCbMap_[type][std::move(callbackRef)] = displayListener;
    return DMError::DM_OK;
}

napi_value JsDisplay::OnUnregisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGI("OnUnregisterDisplayCallback is called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        WLOGFE("JsDisplayManager Params not match %{public}zu", argc);
        std::string errMsg = "Invalid args count, need one arg at least!";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        std::string errMsg = "Failed to convert parameter to string";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret;
    if (argc == ARGC_ONE) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(cbType));
    } else {
        napi_value value = argv[INDEX_ONE];
        if ((value == nullptr) || (!NapiIsCallable(env, value))) {
            ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(cbType));
        } else {
            ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterDisplayListenerWithType(env, cbType, value));
        }
    }
    if (ret != DmErrorCode::DM_OK) {
        WLOGFW("failed to unregister display listener with type");
        std::string errMsg = "failed to unregister display listener with type";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

DMError JsDisplay::UnregisterAllDisplayListenerWithType(const std::string& type)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("UnregisterAllDisplayListenerWithType methodName %{public}s not registered!",
            type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == EVENT_AVAILABLE_AREA_CHANGED) {
            sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
        } else {
            ret = DMError::DM_ERROR_INVALID_PARAM;
        }
        jsCbMap_[type].erase(it++);
        WLOGFI("unregister display listener with type %{public}s  ret: %{public}u", type.c_str(), ret);
    }
    jsCbMap_.erase(type);
    return ret;
}

DMError JsDisplay::UnRegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("UnRegisterDisplayListenerWithType methodName %{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        bool isEquals = false;
        napi_strict_equals(env, value, it->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            it->second->RemoveCallback(env, type, value);
            if (type == EVENT_AVAILABLE_AREA_CHANGED) {
                sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
            } else {
                ret = DMError::DM_ERROR_INVALID_PARAM;
            }
            jsCbMap_[type].erase(it++);
            WLOGFI("unregister display listener with type %{public}s  ret: %{public}u", type.c_str(), ret);
            break;
        } else {
            it++;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return ret;
}

napi_value JsDisplay::HasImmersiveWindow(napi_env env, napi_callback_info info)
{
    WLOGI("HasImmersiveWindow is called");
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnHasImmersiveWindow(env, info) : nullptr;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

napi_value JsDisplay::OnGetCutoutInfo(napi_env env, napi_callback_info info)
{
    WLOGD("OnGetCutoutInfo is called");
    napi_value result = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr && GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplay::OnGetCutoutInfo");
        sptr<CutoutInfo> cutoutInfo = display_->GetCutoutInfo();
        if (cutoutInfo != nullptr) {
            task->Resolve(env, CreateJsCutoutInfoObject(env, cutoutInfo));
            WLOGD("JsDisplay::OnGetCutoutInfo success");
        } else {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN), "JsDisplay::OnGetCutoutInfo failed."));
        }
        delete task;
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN), "Send event failed!"));
    } else {
        napiAsyncTask.release();
    }
    return result;
}

std::unique_ptr<NapiAsyncTask> JsDisplay::CreateEmptyAsyncTask(napi_env env, napi_value lastParam, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    }
}

napi_value JsDisplay::OnGetAvailableArea(napi_env env, napi_callback_info info)
{
    WLOGI("OnGetAvailableArea is called");
    NapiAsyncTask::CompleteCallback complete =
        [this](napi_env env, NapiAsyncTask& task, int32_t status) {
            DMRect area;
            DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display_->GetAvailableArea(area));
            if (ret == DmErrorCode::DM_OK) {
                task.Resolve(env, CreateJsRectObject(env, area));
                WLOGI("JsDisplay::OnGetAvailableArea success");
            } else {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(ret),
                    "JsDisplay::OnGetAvailableArea failed."));
            }
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsDisplay::OnGetAvailableArea",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsDisplay::OnHasImmersiveWindow(napi_env env, napi_callback_info info)
{
    WLOGI("OnHasImmersiveWindow is called");
    NapiAsyncTask::CompleteCallback complete =
        [this](napi_env env, NapiAsyncTask& task, int32_t status) {
            bool immersive = false;
            DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display_->HasImmersiveWindow(immersive));
            if (ret == DmErrorCode::DM_OK) {
                task.Resolve(env, CreateJsValue(env, immersive));
                WLOGI("JsDisplay::OnHasImmersiveWindow success - immersive window exists: %{public}d", immersive);
            } else {
                task.Reject(env, CreateJsError(env,
                    static_cast<int32_t>(ret), "JsDisplay::OnHasImmersiveWindow failed."));
            }
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsDisplay::OnHasImmersiveWindow",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsDisplay::GetSupportedColorSpaces(napi_env env, napi_callback_info info)
{
    WLOGI("GetSupportedColorSpaces is called");
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnGetSupportedColorSpaces(env, info) : nullptr;
}

static napi_value CreateJsColorSpaceArray(napi_env env, const std::vector<uint32_t>& colorSpaces)
{
    WLOGD("JsDisplay::CreateJsColorSpaceArray is called");
    std::set<uint32_t> nativeColorSpaces;
    for (const auto colorSpace : colorSpaces) {
        GraphicCM_ColorSpaceType colorSpaceValue = static_cast<GraphicCM_ColorSpaceType>(colorSpace);
        if (NATIVE_TO_JS_COLOR_SPACE_TYPE_MAP.count(colorSpaceValue) == 0) {
            WLOGFE("Get color space name %{public}u, but not in api type", colorSpace);
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(DMError::DM_ERROR_DEVICE_NOT_SUPPORT)));
            return NapiGetUndefined(env);
        }
        nativeColorSpaces.insert(static_cast<uint32_t>(NATIVE_TO_JS_COLOR_SPACE_TYPE_MAP.at(colorSpaceValue)));
    }
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, nativeColorSpaces.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("Failed to create color space array");
        return NapiGetUndefined(env);
    }
    uint32_t index = 0;
    for (const auto nativeColorSpace : nativeColorSpaces) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, nativeColorSpace));
    }
    return arrayValue;
}

napi_value JsDisplay::OnGetSupportedColorSpaces(napi_env env, napi_callback_info info)
{
    WLOGI("OnGetSupportedColorSpaces is called");
    NapiAsyncTask::CompleteCallback complete =
        [=](napi_env env, NapiAsyncTask& task, int32_t status) {
            std::vector<uint32_t> colorSpaces;
            DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display_->GetSupportedColorSpaces(colorSpaces));
            if (ret == DmErrorCode::DM_OK) {
                task.Resolve(env, CreateJsColorSpaceArray(env, colorSpaces));
                WLOGI("OnGetSupportedColorSpaces success");
            } else {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(ret),
                                               "JsDisplay::OnGetSupportedColorSpaces failed."));
                WLOGFE("OnGetSupportedColorSpaces failed");
            }
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsDisplay::OnGetSupportedColorSpaces",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsDisplay::GetSupportedHDRFormats(napi_env env, napi_callback_info info)
{
    WLOGI("GetSupportedHDRFormats is called");
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnGetSupportedHDRFormats(env, info) : nullptr;
}

static napi_value CreateJsHDRFormatArray(napi_env env, const std::vector<uint32_t>& hdrFormats)
{
    WLOGD("JsDisplay::CreateJsHDRFormatArray is called");
    std::set<uint32_t> nativeHDRFormats;
    for (const auto hdrFormat : hdrFormats) {
        ScreenHDRFormat hdrFormatValue = static_cast<ScreenHDRFormat>(hdrFormat);
        if (NATIVE_TO_JS_HDR_FORMAT_TYPE_MAP.count(hdrFormatValue) == 0) {
            WLOGFE("Get HDR format name %{public}u, but not in api type", hdrFormat);
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(DMError::DM_ERROR_DEVICE_NOT_SUPPORT)));
            return NapiGetUndefined(env);
        }
        nativeHDRFormats.insert(static_cast<uint32_t>(NATIVE_TO_JS_HDR_FORMAT_TYPE_MAP.at(hdrFormatValue)));
    }
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, hdrFormats.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("Failed to create HDR format array");
        return NapiGetUndefined(env);
    }
    uint32_t index = 0;
    for (const auto nativeHDRFormat : nativeHDRFormats) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, nativeHDRFormat));
    }
    return arrayValue;
}

napi_value JsDisplay::OnGetSupportedHDRFormats(napi_env env, napi_callback_info info)
{
    WLOGI("OnGetSupportedHDRFormats is called");
    NapiAsyncTask::CompleteCallback complete =
        [=](napi_env env, NapiAsyncTask& task, int32_t status) {
            std::vector<uint32_t> hdrFormats;
            DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display_->GetSupportedHDRFormats(hdrFormats));
            if (ret == DmErrorCode::DM_OK) {
                task.Resolve(env, CreateJsHDRFormatArray(env, hdrFormats));
                WLOGI("OnGetSupportedHDRFormats success");
            } else {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(ret),
                                               "JsDisplay::OnGetSupportedHDRFormats failed."));
                WLOGFE("OnGetSupportedHDRFormats failed");
            }
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsDisplay::OnGetSupportedHDRFormats",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

std::shared_ptr<NativeReference> FindJsDisplayObject(DisplayId displayId)
{
    WLOGD("[NAPI]Try to find display %{public}" PRIu64" in g_JsDisplayMap", displayId);
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    if (g_JsDisplayMap.find(displayId) == g_JsDisplayMap.end()) {
        WLOGD("[NAPI]Can not find display %{public}" PRIu64" in g_JsDisplayMap", displayId);
        return nullptr;
    }
    return g_JsDisplayMap[displayId];
}

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value CreateJsCutoutInfoObject(napi_env env, sptr<CutoutInfo> cutoutInfo)
{
    WLOGD("JsDisplay::CreateJsCutoutInfoObject is called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to convert prop to jsObject");
        return NapiGetUndefined(env);
    }
    if (cutoutInfo == nullptr) {
        WLOGFE("Get null cutout info");
        return NapiGetUndefined(env);
    }
    std::vector<DMRect> boundingRects = cutoutInfo->GetBoundingRects();
    WaterfallDisplayAreaRects waterfallDisplayAreaRects = cutoutInfo->GetWaterfallDisplayAreaRects();
    napi_set_named_property(env, objValue, "boundingRects", CreateJsBoundingRectsArrayObject(env, boundingRects));
    napi_set_named_property(env, objValue, "waterfallDisplayAreaRects",
        CreateJsWaterfallDisplayAreaRectsObject(env, waterfallDisplayAreaRects));
    return objValue;
}

napi_value CreateJsDisplayPhysicalInfoObject(napi_env env, DisplayPhysicalResolution physicalInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "foldDisplayMode", CreateJsValue(env, physicalInfo.foldDisplayMode_));
    napi_set_named_property(env, objValue, "physicalWidth", CreateJsValue(env, physicalInfo.physicalWidth_));
    napi_set_named_property(env, objValue, "physicalHeight", CreateJsValue(env, physicalInfo.physicalHeight_));
    return objValue;
}

napi_value CreateJsRectObject(napi_env env, DMRect rect)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value CreateJsWaterfallDisplayAreaRectsObject(napi_env env,
    WaterfallDisplayAreaRects waterfallDisplayAreaRects)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "left", CreateJsRectObject(env, waterfallDisplayAreaRects.left));
    napi_set_named_property(env, objValue, "top", CreateJsRectObject(env, waterfallDisplayAreaRects.top));
    napi_set_named_property(env, objValue, "right", CreateJsRectObject(env, waterfallDisplayAreaRects.right));
    napi_set_named_property(env, objValue, "bottom", CreateJsRectObject(env, waterfallDisplayAreaRects.bottom));
    return objValue;
}

napi_value CreateJsBoundingRectsArrayObject(napi_env env, std::vector<DMRect> boundingRects)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, boundingRects.size(), &arrayValue);
    size_t i = 0;
    for (const auto& rect : boundingRects) {
        napi_set_element(env, arrayValue, i++, CreateJsRectObject(env, rect));
    }
    return arrayValue;
}

void NapiSetNamedProperty(napi_env env, napi_value objValue, sptr<DisplayInfo> info)
{
    napi_set_named_property(env, objValue, "id", CreateJsValue(env, static_cast<uint32_t>(info->GetDisplayId())));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info->GetName()));
    napi_set_named_property(env, objValue, "alive", CreateJsValue(env, info->GetAliveStatus()));
    if (NATIVE_TO_JS_DISPLAY_STATE_MAP.count(info->GetDisplayState()) != 0) {
        napi_set_named_property(env, objValue, "state",
            CreateJsValue(env, NATIVE_TO_JS_DISPLAY_STATE_MAP.at(info->GetDisplayState())));
    } else {
        napi_set_named_property(env, objValue, "state", CreateJsValue(env, DisplayStateMode::STATE_UNKNOWN));
    }
    napi_set_named_property(env, objValue, "refreshRate", CreateJsValue(env, info->GetRefreshRate()));
    napi_set_named_property(env, objValue, "rotation", CreateJsValue(env, info->GetRotation()));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, info->GetWidth()));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, info->GetHeight()));
    napi_set_named_property(env, objValue, "densityDPI",
        CreateJsValue(env, info->GetVirtualPixelRatio() * DOT_PER_INCH));
    napi_set_named_property(env, objValue, "orientation", CreateJsValue(env, info->GetDisplayOrientation()));
    napi_set_named_property(env, objValue, "densityPixels", CreateJsValue(env, info->GetVirtualPixelRatio()));
    napi_set_named_property(env, objValue, "scaledDensity", CreateJsValue(env, info->GetVirtualPixelRatio()));
    napi_set_named_property(env, objValue, "xDPI", CreateJsValue(env, info->GetXDpi()));
    napi_set_named_property(env, objValue, "yDPI", CreateJsValue(env, info->GetYDpi()));
    napi_set_named_property(env, objValue, "colorSpaces", CreateJsColorSpaceArray(env, info->GetColorSpaces()));
    napi_set_named_property(env, objValue, "hdrFormats", CreateJsHDRFormatArray(env, info->GetHdrFormats()));
    napi_set_named_property(env, objValue, "availableWidth", CreateJsValue(env, info->GetAvailableWidth()));
    napi_set_named_property(env, objValue, "availableHeight", CreateJsValue(env, info->GetAvailableHeight()));
}

napi_value CreateJsDisplayObject(napi_env env, sptr<Display>& display)
{
    WLOGD("CreateJsDisplay is called");
    napi_value objValue = nullptr;
    std::shared_ptr<NativeReference> jsDisplayObj = FindJsDisplayObject(display->GetId());
    if (jsDisplayObj != nullptr && jsDisplayObj->GetNapiValue() != nullptr) {
        WLOGD("[NAPI]FindJsDisplayObject %{public}" PRIu64"", display->GetId());
        objValue = jsDisplayObj->GetNapiValue();
    }
    if (objValue == nullptr) {
        napi_create_object(env, &objValue);
    }
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return NapiGetUndefined(env);
    }
    auto info = display->GetDisplayInfoByJs();
    if (info == nullptr) {
        WLOGFE("Failed to GetDisplayInfo");
        return NapiGetUndefined(env);
    }

    NapiSetNamedProperty(env, objValue, info);

    if (jsDisplayObj == nullptr || jsDisplayObj->GetNapiValue() == nullptr) {
        std::unique_ptr<JsDisplay> jsDisplay = std::make_unique<JsDisplay>(display);
        napi_wrap(env, objValue, jsDisplay.release(), JsDisplay::Finalizer, nullptr, nullptr);
        BindNativeFunction(env, objValue, "getCutoutInfo", "JsDisplay", JsDisplay::GetCutoutInfo);
        BindNativeFunction(env, objValue, "hasImmersiveWindow", "JsDisplay", JsDisplay::HasImmersiveWindow);
        BindNativeFunction(env, objValue, "getAvailableArea", "JsDisplay", JsDisplay::GetAvailableArea);
        BindNativeFunction(env, objValue, "getSupportedColorSpaces", "JsDisplay", JsDisplay::GetSupportedColorSpaces);
        BindNativeFunction(env, objValue, "getSupportedHDRFormats", "JsDisplay", JsDisplay::GetSupportedHDRFormats);
        BindNativeFunction(env, objValue, "on", "JsDisplay", JsDisplay::RegisterDisplayManagerCallback);
        BindNativeFunction(env, objValue, "off", "JsDisplay", JsDisplay::UnregisterDisplayManagerCallback);
        std::shared_ptr<NativeReference> jsDisplayRef;
        napi_ref result = nullptr;
        napi_create_reference(env, objValue, 1, &result);
        jsDisplayRef.reset(reinterpret_cast<NativeReference*>(result));
        DisplayId displayId = display->GetId();
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        g_JsDisplayMap[displayId] = jsDisplayRef;
    }
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
