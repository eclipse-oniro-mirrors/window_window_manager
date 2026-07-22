/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "js_pip_utils.h"

#include <string>
#include "window.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr float MAX_PIP_SCALE = 1.0f;
}
napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiThrowInvalidParam(napi_env env, std::string msg, const char* enumerationName,
    const char* boolName)
{
    if (enumerationName != nullptr &&  *enumerationName != '\0') {
        HISTOGRAM_ENUMERATION_ERROR_CODE(enumerationName, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (boolName != nullptr &&  *boolName != '\0') {
        HISTOGRAM_BOOLEAN(boolName, 0);
    }
    napi_throw(env, AbilityRuntime::CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM), msg));
    return NapiGetUndefined(env);
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

static napi_status SetNamedProperty(napi_env env, napi_value& obj, const std::string& name, int32_t value)
{
    napi_value property = nullptr;
    napi_status status = napi_create_int32(env, value, &property);
    if (status != napi_ok) {
        return status;
    }
    status = napi_set_named_property(env, obj, name.c_str(), property);
    if (status != napi_ok) {
        return status;
    }
    return status;
}

static napi_value ExportPictureInPictureTemplateType(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "VIDEO_PLAY", static_cast<int32_t>(PiPTemplateType::VIDEO_PLAY));
    (void)SetNamedProperty(env, result, "VIDEO_CALL", static_cast<int32_t>(PiPTemplateType::VIDEO_CALL));
    (void)SetNamedProperty(env, result, "VIDEO_MEETING", static_cast<int32_t>(PiPTemplateType::VIDEO_MEETING));
    (void)SetNamedProperty(env, result, "VIDEO_LIVE", static_cast<int32_t>(PiPTemplateType::VIDEO_LIVE));
    (void)SetNamedProperty(env, result, "VIDEO_DRIVE", static_cast<int32_t>(PiPTemplateType::VIDEO_DRIVE));
    (void)SetNamedProperty(env, result, "VIDEO_NAVIGATION", static_cast<int32_t>(PiPTemplateType::VIDEO_NAVIGATION));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportPictureInPictureState(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "ABOUT_TO_START", static_cast<int32_t>(PiPState::ABOUT_TO_START));
    (void)SetNamedProperty(env, result, "STARTED", static_cast<int32_t>(PiPState::STARTED));
    (void)SetNamedProperty(env, result, "ABOUT_TO_STOP", static_cast<int32_t>(PiPState::ABOUT_TO_STOP));
    (void)SetNamedProperty(env, result, "STOPPED", static_cast<int32_t>(PiPState::STOPPED));
    (void)SetNamedProperty(env, result, "ABOUT_TO_RESTORE", static_cast<int32_t>(PiPState::ABOUT_TO_RESTORE));
    (void)SetNamedProperty(env, result, "ERROR", static_cast<int32_t>(PiPState::ERROR));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportVideoPlayControlGroup(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "VIDEO_PREVIOUS_NEXT",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT));
    (void)SetNamedProperty(env, result, "FAST_FORWARD_BACKWARD",
        static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportVideoCallControlGroup(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "MICROPHONE_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_MICROPHONE_SWITCH));
    (void)SetNamedProperty(env, result, "HANG_UP_BUTTON",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_HANG_UP_BUTTON));
    (void)SetNamedProperty(env, result, "CAMERA_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_CAMERA_SWITCH));
    (void)SetNamedProperty(env, result, "MUTE_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_MUTE_SWITCH));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportVideoMeetingControlGroup(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "HANG_UP_BUTTON",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_MEETING_HANG_UP_BUTTON));
    (void)SetNamedProperty(env, result, "CAMERA_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_MEETING_CAMERA_SWITCH));
    (void)SetNamedProperty(env, result, "MUTE_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_MEETING_MUTE_SWITCH));
    (void)SetNamedProperty(env, result, "MICROPHONE_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_MEETING_MICROPHONE_SWITCH));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportVideoLiveControlGroup(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "VIDEO_PLAY_PAUSE",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_PLAY_PAUSE));
    (void)SetNamedProperty(env, result, "MUTE_SWITCH",
        static_cast<uint32_t>(PiPControlGroup::VIDEO_LIVE_MUTE_SWITCH));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportControlStatus(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "PLAY",
        static_cast<int32_t>(PiPControlStatus::PLAY));
    (void)SetNamedProperty(env, result, "PAUSE",
        static_cast<int32_t>(PiPControlStatus::PAUSE));
    (void)SetNamedProperty(env, result, "OPEN",
        static_cast<int32_t>(PiPControlStatus::OPEN));
    (void)SetNamedProperty(env, result, "CLOSE",
        static_cast<int32_t>(PiPControlStatus::CLOSE));
    (void)SetNamedProperty(env, result, "ENABLED",
        static_cast<int32_t>(PiPControlStatus::ENABLED));
    (void)SetNamedProperty(env, result, "DISABLED",
        static_cast<int32_t>(PiPControlStatus::DISABLED));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportControlType(napi_env env)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    (void)SetNamedProperty(env, result, "VIDEO_PLAY_PAUSE",
        static_cast<uint32_t>(PiPControlType::VIDEO_PLAY_PAUSE));
    (void)SetNamedProperty(env, result, "VIDEO_PREVIOUS",
        static_cast<uint32_t>(PiPControlType::VIDEO_PREVIOUS));
    (void)SetNamedProperty(env, result, "VIDEO_NEXT",
        static_cast<uint32_t>(PiPControlType::VIDEO_NEXT));
    (void)SetNamedProperty(env, result, "FAST_FORWARD",
        static_cast<uint32_t>(PiPControlType::FAST_FORWARD));
    (void)SetNamedProperty(env, result, "FAST_BACKWARD",
        static_cast<uint32_t>(PiPControlType::FAST_BACKWARD));
    (void)SetNamedProperty(env, result, "HANG_UP_BUTTON",
        static_cast<uint32_t>(PiPControlType::HANG_UP_BUTTON));
    (void)SetNamedProperty(env, result, "MICROPHONE_SWITCH",
        static_cast<uint32_t>(PiPControlType::MICROPHONE_SWITCH));
    (void)SetNamedProperty(env, result, "CAMERA_SWITCH",
        static_cast<uint32_t>(PiPControlType::CAMERA_SWITCH));
    (void)SetNamedProperty(env, result, "MUTE_SWITCH",
        static_cast<uint32_t>(PiPControlType::MUTE_SWITCH));
    napi_object_freeze(env, result);
    return result;
}

napi_status InitEnums(napi_env env, napi_value exports)
{
    const napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("PiPTemplateType", ExportPictureInPictureTemplateType(env)),
        DECLARE_NAPI_PROPERTY("PiPState", ExportPictureInPictureState(env)),
        DECLARE_NAPI_PROPERTY("VideoPlayControlGroup", ExportVideoPlayControlGroup(env)),
        DECLARE_NAPI_PROPERTY("VideoCallControlGroup", ExportVideoCallControlGroup(env)),
        DECLARE_NAPI_PROPERTY("VideoMeetingControlGroup", ExportVideoMeetingControlGroup(env)),
        DECLARE_NAPI_PROPERTY("PiPControlType", ExportControlType(env)),
        DECLARE_NAPI_PROPERTY("PiPControlStatus", ExportControlStatus(env)),
        DECLARE_NAPI_PROPERTY("VideoLiveControlGroup", ExportVideoLiveControlGroup(env)),
    };
    size_t count = sizeof(properties) / sizeof(napi_property_descriptor);
    return napi_define_properties(env, exports, count, properties);
}

napi_value GetPiPWindowSizeAndConvertToJsValue(napi_env env, const Rect& rect, float scale)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    napi_set_named_property(env, objValue, "scale", CreateJsValue(env, scale));
    return objValue;
}

napi_value CreateJsPiPWindowInfoObject(napi_env env, const sptr<Window>& window)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get object");
        return nullptr;
    }
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr");
        return nullptr;
    }
    const auto& windowRect = window->GetRect();
    const auto& layoutTransform = window->GetLayoutTransform();
    float maxScale = std::min(std::max(layoutTransform.scaleX_, layoutTransform.scaleY_), MAX_PIP_SCALE);
    napi_value pipWindowSizeObj = GetPiPWindowSizeAndConvertToJsValue(env, windowRect, maxScale);
    if (pipWindowSizeObj == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Get window size failed!");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "windowId", CreateJsValue(env, window->GetWindowId()));
    napi_set_named_property(env, objValue, "size", pipWindowSizeObj);
    return objValue;
}
} // Rosen
} // OHOS