/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WINDOW_SCENE_JS_SCENE_UTILS_H
#define OHOS_WINDOW_SCENE_JS_SCENE_UTILS_H

#include <js_runtime_utils.h>
#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>
#include <pointer_event.h>

#include "dm_common.h"
#include "interfaces/include/ws_common.h"
#include "common/include/window_session_property.h"
#include "wm_common.h"
#include "hitrace_meter.h"

namespace OHOS::Rosen {

#define CHECK_NAPI_ENV_RETURN_IF_NULL(env)               \
    do {                                                 \
        if ((env) == nullptr) {                          \
            TLOGE(WmsLogTag::DEFAULT, "env is invalid"); \
            return nullptr;                              \
        }                                                \
    } while (0)

#define CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue) \
    do {                                                       \
        napi_create_object((env), &(objValue));                \
        if ((objValue) == nullptr) {                           \
            TLOGE(WmsLogTag::DEFAULT, "Failed to get object"); \
            return nullptr;                                    \
        }                                                      \
    } while (0)

#define NAPI_CHECK_RETURN_IF_NULL(func, msg)                                                            \
    do {                                                                                                \
        napi_status status = func;                                                                      \
        if (status != napi_ok) {                                                                        \
            TLOGE(WmsLogTag::DEFAULT, "Failed with reason %{public}s, code: %{public}d", msg, status);  \
            return nullptr;                                                                             \
        }                                                                                               \
    } while (0)

enum class JsSessionType : uint32_t {
    TYPE_UNDEFINED = 0,
    TYPE_APP,
    TYPE_SUB_APP,
    TYPE_SYSTEM_ALERT,
    TYPE_INPUT_METHOD,
    TYPE_STATUS_BAR,
    TYPE_PANEL,
    TYPE_KEYGUARD,
    TYPE_VOLUME_OVERLAY,
    TYPE_NAVIGATION_BAR,
    TYPE_FLOAT,
    TYPE_WALLPAPER,
    TYPE_DESKTOP,
    TYPE_LAUNCHER_DOCK,
    TYPE_FLOAT_CAMERA,
    TYPE_DIALOG,
    TYPE_SCREENSHOT,
    TYPE_TOAST,
    TYPE_POINTER,
    TYPE_LAUNCHER_RECENT,
    TYPE_SCENE_BOARD,
    TYPE_DRAGGING_EFFECT,
    TYPE_INPUT_METHOD_STATUS_BAR,
    TYPE_GLOBAL_SEARCH,
    TYPE_NEGATIVE_SCREEN,
    TYPE_VOICE_INTERACTION,
    TYPE_SYSTEM_TOAST,
    TYPE_SYSTEM_FLOAT,
    TYPE_THEME_EDITOR,
    TYPE_PIP,
    TYPE_NAVIGATION_INDICATOR,
    TYPE_SEARCHING_BAR,
    TYPE_SYSTEM_SUB_WINDOW,
    TYPE_HANDWRITE,
    TYPE_KEYBOARD_PANEL,
    TYPE_DIVIDER,
    TYPE_TRANSPARENT_VIEW,
    TYPE_WALLET_SWIPE_CARD,
    TYPE_SCREEN_CONTROL,
    TYPE_FLOAT_NAVIGATION,
    TYPE_MUTISCREEN_COLLABORATION,
    TYPE_DYNAMIC = 41,
    TYPE_MAGNIFICATION = 42,
    TYPE_MAGNIFICATION_MENU = 43,
    TYPE_SELECTION = 44,
    TYPE_FLOATING_BALL = 45,
};

extern const std::map<WindowType, JsSessionType> WINDOW_TO_JS_SESSION_TYPE_MAP;
extern const std::map<JsSessionType, WindowType> JS_SESSION_TO_WINDOW_TYPE_MAP;

enum class ThrowSlipMode;

JsSessionType GetApiType(WindowType type);
bool ConvertSessionInfoFromJs(napi_env env, napi_value jsObject, SessionInfo& sessionInfo);
bool ConvertPointerEventFromJs(napi_env env, napi_value jsObject, MMI::PointerEvent& pointerEvent);
bool ConvertDeviceIdFromJs(napi_env env, napi_value jsObject, MMI::PointerEvent& pointerEvent);
bool ConvertInt32ArrayFromJs(napi_env env, napi_value jsObject, std::vector<int32_t>& intList);
bool ConvertStringMapFromJs(napi_env env, napi_value value, std::unordered_map<std::string, std::string>& stringMap);
bool ConvertJsonFromJs(napi_env env, napi_value value, nlohmann::json& payload);
bool ParseArrayStringValue(napi_env env, napi_value array, std::vector<std::string>& vector);
bool ConvertProcessOptionFromJs(napi_env env, napi_value jsObject,
    std::shared_ptr<AAFwk::ProcessOptions> processOptions);
napi_value CreateJsSessionPendingConfigs(napi_env env, const PendingSessionActivationConfig& config);
napi_value CreateJsSessionInfo(napi_env env, const SessionInfo& sessionInfo,
    const std::shared_ptr<PendingSessionActivationConfig>& config = nullptr);
void ProcessPendingSessionActivationResult(napi_env env, napi_value callResult,
    const std::shared_ptr<SessionInfo>& sessionInfo);
bool ConvertSessionResultFromJsValue(
    napi_env env, napi_value jsResult, uint32_t& resultCode, std::string& resultMessage);
napi_value CreateJsExceptionInfo(napi_env env, const ExceptionInfo& exceptionInfo);
napi_value CreateSupportWindowModes(napi_env env,
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes);
napi_value CreateJsAtomicServiceInfo(napi_env env, const AtomicServiceInfo& atomicServiceInfo);
napi_value CreateJsSessionRecoverInfo(
    napi_env env, const SessionInfo& sessionInfo, const sptr<WindowSessionProperty> property);
void SetJsSessionInfoByWant(napi_env env, const SessionInfo& sessionInfo, napi_value objValue);
napi_value CreateJsProcessOption(napi_env env, std::shared_ptr<AAFwk::ProcessOptions> processOptions);
napi_value CreateJsSessionStartupVisibility(napi_env env);
napi_value CreateJsWindowVisibility(napi_env env);
napi_value CreateJsSessionProcessMode(napi_env env);
napi_value GetWindowRectIntValue(napi_env env, int val);
napi_value CreateJsSessionState(napi_env env);
napi_value CreateJsSessionSizeChangeReason(napi_env env);
napi_value CreateJsRSUIFirstSwitch(napi_env env);
napi_value CreateJsSessionPiPControlType(napi_env env);
napi_value CreateJsSessionPiPControlStatus(napi_env env);
napi_value CreateJsSessionGravity(napi_env env);
napi_value CreateJsSessionDragResizeType(napi_env env);
void CreatePiPSizeChangeReason(napi_env env, napi_value objValue);
template<typename T>
napi_value CreateJsSessionRect(napi_env env, const T& rect);
napi_value CreateJsRectAnimationConfig(napi_env env, const RectAnimationConfig& rectAnimationConfig);
napi_value CreateJsSessionEventParam(napi_env env, const SessionEventParam& param);
napi_value CreateRotationChangeType(napi_env env);
napi_value CreateRectType(napi_env env);
napi_value CreateWindowAnchorType(napi_env env);
napi_value CreatePixelUnitType(napi_env env);
napi_value CreateJsWindowAnchorInfo(napi_env env, const WindowAnchorInfo& windowAnchorInfo);
napi_value CreateSupportType(napi_env env);
napi_value SubWindowModalTypeInit(napi_env env);
napi_value CreateJsSystemBarPropertyArrayObject(
    napi_env env, const std::unordered_map<WindowType, SystemBarProperty>& propertyMap);
napi_value CreateJsKeyboardLayoutParams(napi_env env, const KeyboardLayoutParams& params);
napi_value CreateJsShadowsInfo(napi_env env, const ShadowsInfo& shadowsInfo);
napi_value SessionTypeInit(napi_env env);
napi_value SceneTypeInit(napi_env env);
napi_value KeyboardGravityInit(napi_env env);
napi_value KeyboardViewModeInit(napi_env env);
napi_value KeyboardFlowLightModeInit(napi_env env);
napi_value KeyboardGradientModeInit(napi_env env);
napi_value AnimationTypeInit(napi_env env);
napi_value WindowTransitionTypeInit(napi_env env);
napi_value WindowAnimationCurveInit(napi_env env);
napi_value CreateResultMapToJsValue(napi_env env,
    const std::unordered_map<int32_t, RotationChangeResult>& rotationChangeResultMap);
napi_value CreateJsRotationChangeResultMapObject(napi_env env, const int32_t persistentId,
    const RotationChangeResult& rotationChangeResult);
napi_value ConvertResultToJsValue(napi_env env, RotationChangeResult& rotationChangeResult);
napi_value NapiGetUndefined(napi_env env);
napi_valuetype GetType(napi_env env, napi_value value);
napi_value ConvertWindowAnimationOptionToJsValue(napi_env env,
    const WindowAnimationOption& animationConfig);
napi_value ConvertTransitionAnimationToJsValue(napi_env env,
    std::shared_ptr<TransitionAnimation> transitionAnimation);
napi_value ConvertWindowAnimationPropertyToJsValue(napi_env env,
    const WindowAnimationProperty& animationProperty);
napi_value ConvertKeyboardEffectOptionToJsValue(napi_env env, const KeyboardEffectOption& effectOption);
napi_value CreateWaterfallResidentState(napi_env env);
napi_value CreateCompatibleStyleMode(napi_env env);
bool NapiIsCallable(napi_env env, napi_value value);
bool ConvertRectInfoFromJs(napi_env env, napi_value jsObject, WSRect& rect);
bool ConvertSessionRectInfoFromJs(napi_env env, napi_value jsObject, WSRect& rect);
bool ConvertSingleHandScreenInfoFromJs(napi_env env, napi_value jsObject,
    SingleHandScreenInfo& singleHandHandScreenInfo);
bool ConvertHookInfoFromJs(napi_env env, napi_value jsObject, HookInfo& hookInfo);
bool ConvertHookWindowInfoFromJs(napi_env env, napi_value jsObject, HookWindowInfo& hookWindowInfo);
bool ConvertRotateAnimationConfigFromJs(napi_env env, napi_value value, RotateAnimationConfig& config);
bool ConvertDragResizeTypeFromJs(napi_env env, napi_value value, DragResizeType& dragResizeType);
bool ConvertRectFromJsValue(napi_env env, napi_value jsObject, Rect& displayRect);
bool ConvertInfoFromJsValue(napi_env env, napi_value jsObject, RotationChangeInfo& rotationChangeInfo);
bool ConvertThrowSlipModeFromJs(napi_env env, napi_value value, ThrowSlipMode& throwSlipMode);
bool convertAnimConfigFromJs(napi_env env, napi_value value, SceneAnimationConfig& config);
bool ConvertSupportRotationInfoFromJsValue(napi_env env, napi_value jsObject,
    SupportRotationInfo& suppoortRotationInfo);
bool ParseBoolArrayValueFromJsValue(napi_env env, napi_value array, std::vector<bool>& vector);
template<class T>
bool ParseJsValue(napi_env env, napi_value jsObject, const std::string& name, T& data)
{
    napi_value value = nullptr;
    napi_get_named_property(env, jsObject, name.c_str(), &value);
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type != napi_undefined) {
        return AbilityRuntime::ConvertFromJsValue(env, value, data);
    }
    return false;
}
bool ConvertCompatibleModePropertyFromJs(napi_env env, napi_value value, CompatibleModeProperty& property);
WSError GetIntValueFromString(const std::string& str, uint32_t& value);
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr size_t ARGC_FIVE = 5;

class MainThreadScheduler {
public:
    using Task = std::function<void()>;
    explicit MainThreadScheduler(napi_env env);
    void PostMainThreadTask(Task&& localTask, std::string traceInfo = "Unnamed", int64_t delayTime = 0);
    void RemoveMainThreadTaskByName(const std::string taskName);

private:
    void GetMainEventHandler();
    napi_env env_;
    std::shared_ptr<int> envChecker_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_;
};
} // namespace OHOS::Rosen
#endif // OHOS_WINDOW_SCENE_JS_SCENE_UTILS_H
