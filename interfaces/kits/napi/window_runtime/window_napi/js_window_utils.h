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

#ifndef OHOS_JS_WINDOW_UTILS_H
#define OHOS_JS_WINDOW_UTILS_H
#include <array>
#include <map>
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "window.h"

#ifndef WINDOW_PREVIEW
#include "window_manager.h"
#else
#include "mock/window_manager.h"
#endif

#include "window_option.h"
#include "window_visibility_info.h"
#include "wm_common.h"
#include "window_helper.h"
namespace OHOS {
namespace Rosen {
constexpr int32_t RGB_LENGTH = 6;
constexpr int32_t RGBA_LENGTH = 8;

#define CHECK_NAPI_RETCODE(errCode, code, call)                                           \
    do {                                                                                  \
        napi_status retCode = (call);                                                     \
        if (retCode != napi_ok) {                                                         \
            WLOGFE("napi call failed, return %{public}d", static_cast<int32_t>(retCode)); \
            errCode = code;                                                               \
        }                                                                                 \
    } while (0)

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

enum class ApiWindowType : uint32_t {
    TYPE_BASE,
    TYPE_APP = TYPE_BASE,
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
    TYPE_LAUNCHER_RECENT,
    TYPE_LAUNCHER_DOCK,
    TYPE_VOICE_INTERACTION,
    TYPE_POINTER,
    TYPE_FLOAT_CAMERA,
    TYPE_DIALOG,
    TYPE_SCREENSHOT,
    TYPE_SYSTEM_TOAST,
    TYPE_DIVIDER,
    TYPE_GLOBAL_SEARCH,
    TYPE_HANDWRITE,
    TYPE_WALLET_SWIPE_CARD,
    TYPE_SCREEN_CONTROL,
    TYPE_FLOAT_NAVIGATION,
    TYPE_DYNAMIC,
    TYPE_MUTISCREEN_COLLABORATION,
    TYPE_FB,
    TYPE_END
};

enum class LifeCycleEventType : uint32_t {
    FOREGROUND = 1,
    ACTIVE,
    INACTIVE,
    BACKGROUND,
    RESUMED,
    PAUSED,
    DESTROYED,
};

enum class WindowStageLifeCycleEventType : uint32_t {
    FOREGROUND = 1,
    RESUMED,
    PAUSED,
    BACKGROUND,
};

extern const std::map<WindowType, ApiWindowType> NATIVE_JS_TO_WINDOW_TYPE_MAP;
extern const std::map<ApiWindowType, WindowType> JS_TO_NATIVE_WINDOW_TYPE_MAP;

enum class ApiWindowMode : uint32_t {
    UNDEFINED = 1,
    FULLSCREEN,
    PRIMARY,
    SECONDARY,
    FLOATING,
    MODE_END = FLOATING
};

extern const std::map<WindowMode, ApiWindowMode> NATIVE_TO_JS_WINDOW_MODE_MAP;
extern const std::map<ApiWindowMode, WindowMode> JS_TO_NATIVE_WINDOW_MODE_MAP;

enum class ApiOrientation : uint32_t {
    BEGIN = 0,
    UNSPECIFIED = BEGIN,
    PORTRAIT = 1,
    LANDSCAPE = 2,
    PORTRAIT_INVERTED = 3,
    LANDSCAPE_INVERTED = 4,
    AUTO_ROTATION = 5,
    AUTO_ROTATION_PORTRAIT = 6,
    AUTO_ROTATION_LANDSCAPE = 7,
    AUTO_ROTATION_RESTRICTED = 8,
    AUTO_ROTATION_PORTRAIT_RESTRICTED = 9,
    AUTO_ROTATION_LANDSCAPE_RESTRICTED = 10,
    LOCKED = 11,
    AUTO_ROTATION_UNSPECIFIED = 12,
    USER_ROTATION_PORTRAIT = 13,
    USER_ROTATION_LANDSCAPE = 14,
    USER_ROTATION_PORTRAIT_INVERTED = 15,
    USER_ROTATION_LANDSCAPE_INVERTED = 16,
    FOLLOW_DESKTOP = 17,
    END = FOLLOW_DESKTOP,
};

extern const std::map<ApiOrientation, Orientation> JS_TO_NATIVE_ORIENTATION_MAP;
extern const std::map<Orientation, ApiOrientation> NATIVE_TO_JS_ORIENTATION_MAP;

enum class RectChangeReason : uint32_t {
    UNDEFINED = 0,
    MAXIMIZE,
    RECOVER,
    MOVE,
    DRAG,
    DRAG_START,
    DRAG_END,
};

extern const std::map<WindowSizeChangeReason, RectChangeReason> JS_SIZE_CHANGE_REASON;

enum class ApiModalityType : uint32_t {
    BEGIN = 0,
    WINDOW_MODALITY = BEGIN,
    APPLICATION_MODALITY,
    END = APPLICATION_MODALITY,
};

inline const std::map<ApiModalityType, ModalityType> JS_TO_NATIVE_MODALITY_TYPE_MAP {
    { ApiModalityType::WINDOW_MODALITY,         ModalityType::WINDOW_MODALITY      },
    { ApiModalityType::APPLICATION_MODALITY,    ModalityType::APPLICATION_MODALITY },
};

using AsyncCallbackFunc_ = std::function<void(napi_env env, size_t argc, napi_value* argv)>;

class AsyncCallback : virtual public RefBase {
public:
    AsyncCallbackFunc_ resolvedCallback_ = nullptr;
    AsyncCallbackFunc_ rejectedCallback_ = nullptr;
    AsyncCallback(AsyncCallbackFunc_ resolvedCallback, AsyncCallbackFunc_ rejectedCallback)
        : resolvedCallback_(resolvedCallback), rejectedCallback_(rejectedCallback) {}
};

class WsNapiAsyncTask final {
public:
    using ExecuteCallback = std::function<void()>;
    using CompleteCallback = std::function<void(napi_env, WsNapiAsyncTask&, int32_t)>;
 
    WsNapiAsyncTask(napi_deferred deferred, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    WsNapiAsyncTask(napi_ref callbackRef, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    ~WsNapiAsyncTask();
 
    void Resolve(napi_env env, napi_value value);
    void Reject(napi_env env, napi_value error);
private:
    napi_deferred deferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    napi_async_work work_ = nullptr;
    std::unique_ptr<ExecuteCallback> execute_;
    std::unique_ptr<CompleteCallback> complete_;
    napi_env env_ = nullptr;
};
 
std::unique_ptr<WsNapiAsyncTask> CreateEmptyWsNapiAsyncTask(napi_env env, napi_value lastParam, napi_value* result);

    /*
     * Promise
     */
    bool CheckPromise(napi_env env, napi_value promiseObj);
    napi_value ResolvedCallback(napi_env env, napi_callback_info info);
    napi_value RejectedCallback(napi_env env, napi_callback_info info);
    bool CallPromise(napi_env env, napi_value promiseObj, AsyncCallback* asyncCallback);

    napi_value CreateJsWindowLayoutInfoArrayObject(napi_env env, const std::vector<sptr<WindowLayoutInfo>>& infos);
    napi_value CreateJsWindowLayoutInfoObject(napi_env env, const sptr<WindowLayoutInfo>& info);
    napi_value CreateJsPixelMapArrayObject(napi_env env, const std::vector<std::shared_ptr<Media::PixelMap>>& pixelMap);
    napi_value CreateJsPixelMapObject(napi_env env, const std::shared_ptr<Media::PixelMap>& pixelMap);
    napi_value CreateJsMainWindowInfoArrayObject(napi_env env, const std::vector<sptr<MainWindowInfo>>& infos);
    napi_value CreateJsMainWindowInfoObject(napi_env env, const sptr<MainWindowInfo>& info);
    napi_value CreateJsWindowInfoArrayObject(napi_env env, const std::vector<sptr<WindowVisibilityInfo>>& infos);
    napi_value CreateJsWindowInfoObject(napi_env env, const sptr<WindowVisibilityInfo>& window);
    napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect);
    napi_value CreateJsWindowAnimationConfigObject(napi_env env, const KeyboardAnimationCurve& curve);
    napi_value CreateJsWindowPropertiesObject(napi_env env, const WindowPropertyInfo& windowPropertyInfo);
    napi_value CreateJsSystemBarPropertiesObject(napi_env env, sptr<Window>& window);
    napi_value CreateRotationChangeInfoObject(napi_env env, const RotationChangeInfo& rotationChangeInfo);
    bool GetSystemBarPropertiesFromJs(napi_env env, napi_value jsObject,
        std::unordered_map<WindowType, SystemBarProperty>& properties,
        std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags);
    bool SetWindowStatusBarContentColor(napi_env env, napi_value jsObject,
        std::unordered_map<WindowType, SystemBarProperty>& properties,
        std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags);
    bool SetWindowNavigationBarContentColor(napi_env env, napi_value jsObject,
        std::unordered_map<WindowType, SystemBarProperty>& properties,
        std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags);
    bool GetSystemBarStatus(napi_env env, napi_callback_info info,
        std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarpropertyFlags);
    napi_value GetStatusBarPropertyObject(napi_env env, sptr<Window>& window);
    bool ParseColorMetrics(napi_env env, napi_value value, uint32_t& colorValue);
    bool GetWindowBackgroundColorFromJs(napi_env env, napi_value value, std::string& colorStr);
    bool ParseAndCheckRect(napi_env env, napi_value jsObject, const Rect& windowRect, Rect& touchableRect);
    WmErrorCode ParseTouchableAreas(napi_env env, napi_callback_info info, const Rect& windowRect,
        std::vector<Rect>& touchableAreas);
    bool GetSpecificBarStatus(napi_env env, napi_callback_info info,
        bool& systemBarEnable, bool& systemBarEnableAnimation);
    napi_value CreateJsSystemBarRegionTintArrayObject(napi_env env,
        const SystemBarRegionTints& tints);
    napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea,
        AvoidAreaType type, bool useActualVisibility = false);
    bool CheckCallingPermission(std::string permission);
    bool ParseSystemWindowTypeForApiWindowType(int32_t apiWindowType, WindowType& windowType);
    napi_value WindowTypeInit(napi_env env);
    napi_value AvoidAreaTypeInit(napi_env env);
    napi_value WindowModeInit(napi_env env);
    napi_value GlobalWindowModeInit(napi_env env);
    napi_value WindowOcclusionStateInit(napi_env env);
    napi_value ScreenshotEventTypeInit(napi_env env);
    napi_value ColorSpaceInit(napi_env env);
    napi_value OrientationInit(napi_env env);
    napi_value WindowStageEventTypeInit(napi_env env);
    napi_value WindowStageLifecycleEventTypeInit(napi_env env);
    napi_value WindowAnchorInit(napi_env env);
    napi_value PixelUnitInit(napi_env env);
    napi_value WindowEventTypeInit(napi_env env);
    napi_value WindowLayoutModeInit(napi_env env);
    napi_value BlurStyleInit(napi_env env);
    napi_value MaximizePresentationInit(napi_env env);
    napi_value WindowErrorCodeInit(napi_env env);
    napi_value WindowErrorInit(napi_env env);
    napi_value WindowStatusTypeInit(napi_env env);
    napi_value RectChangeReasonInit(napi_env env);
    napi_value RotationChangeTypeInit(napi_env env);
    napi_value RotationInfoTypeInit(napi_env env);
    napi_value RectTypeInit(napi_env env);
    napi_value AnimationTypeInit(napi_env env);
    napi_value WindowTransitionTypeInit(napi_env env);
    napi_value WindowAnimationCurveInit(napi_env env);
    napi_value GetWindowLimitsAndConvertToJsValue(napi_env env, const WindowLimits& windowLimits);
    napi_value ConvertTitleButtonAreaToJsValue(napi_env env, const TitleButtonRect& titleButtonRect);
    napi_value ConvertFrameMetricsToJsValue(napi_env env, const FrameMetrics& metrics);
    napi_value ExtensionWindowAttributeInit(napi_env env);
    napi_value ModalityTypeInit(napi_env env);
    napi_value CreateJsDecorButtonStyleObj(napi_env env, DecorButtonStyle decorButtonStyle);
    napi_value ConvertWindowDensityInfoToJsValue(napi_env env, const WindowDensityInfo& windowDensityInfo);
    bool ConvertDecorButtonStyleFromJs(napi_env env, napi_value jsObject, DecorButtonStyle& decorButtonStyle);
    bool GetAPI7Ability(napi_env env, AppExecFwk::Ability* &ability);
    bool GetWindowMaskFromJsValue(napi_env env, napi_value jsObject, std::vector<std::vector<uint32_t>>& windowMask);
    bool GetWindowIdFromJsValue(napi_env env, napi_value jsObject, std::vector<int32_t>& windowIds);
    bool GetMoveConfigurationFromJsValue(napi_env env, napi_value jsObject, MoveConfiguration& moveConfiguration);
    bool ParseRectAnimationConfig(napi_env env, napi_value jsObject, RectAnimationConfig& rectAnimationConfig);
    void ConvertJSSystemBarStyleToSystemBarProperties(napi_env env, napi_value jsObject,
        std::map<WindowType, SystemBarProperty>& properties,
        std::map<WindowType, SystemBarPropertyFlag>& propertyFlags);
    std::unique_ptr<AbilityRuntime::NapiAsyncTask> CreateAsyncTask(napi_env env, napi_value lastParam,
        std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback>&& execute,
        std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback>&& complete, napi_value* result);
    std::unique_ptr<AbilityRuntime::NapiAsyncTask> CreateEmptyAsyncTask(
        napi_env env, napi_value lastParam, napi_value* result);
    bool ParseSubWindowOptions(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption);
    WmErrorCode ParseShowWindowOptions(napi_env env, napi_value showWindowOptions, bool& focusOnShow);
    bool ParseKeyFramePolicy(napi_env env, napi_value jsObject, KeyFramePolicy& keyFramePolicy);
    napi_value ConvertKeyFramePolicyToJsValue(napi_env env, const KeyFramePolicy& keyFramePolicy);
    bool GetRotationResultFromJs(napi_env env, napi_value jsObject, RotationChangeResult& rotationChangeResult);
    bool ConvertRectFromJsValue(napi_env env, napi_value jsObject, Rect& displayRect);
    bool CheckZIndex(int32_t zIndex);
    bool ParseZIndex(napi_env env, napi_value jsObject, WindowOption& option);
    template<class T>
    bool ParseJsValue(napi_value jsObject, napi_env env, const std::string& name, T& data)
    {
        napi_value value = nullptr;
        napi_get_named_property(env, jsObject, name.c_str(), &value);
        napi_valuetype type = napi_undefined;
        napi_typeof(env, value, &type);
        if (type != napi_undefined) {
            if (!AbilityRuntime::ConvertFromJsValue(env, value, data)) {
                return false;
            }
        } else {
            return false;
        }
        return true;
    }

    template<class T>
    bool ParseJsValueOrGetDefault(napi_value jsObject, napi_env env,
        const std::string& name, T& data, const T& defaultVal)
    {
        napi_value value = nullptr;
        napi_get_named_property(env, jsObject, name.c_str(), &value);
        napi_valuetype type = napi_undefined;
        napi_typeof(env, value, &type);
        if (type != napi_undefined) {
            if (!AbilityRuntime::ConvertFromJsValue(env, value, data)) {
                return false;
            }
        } else {
            data = defaultVal;
            return true;
        }
        return true;
    }

    template<class T>
    inline bool ConvertNativeValueToVector(napi_env env, napi_value nativeArray, std::vector<T>& out)
    {
        if (nativeArray == nullptr) {
            return false;
        }
        T value;
        uint32_t size = 0;
        napi_get_array_length(env, nativeArray, &size);
        for (uint32_t i = 0; i < size; i++) {
            napi_value getElementValue = nullptr;
            napi_get_element(env, nativeArray, i, &getElementValue);
            if (!AbilityRuntime::ConvertFromJsValue(env, getElementValue, value)) {
                return false;
            }
            out.emplace_back(value);
        }
        return true;
    }

    /**
     * @brief Create a JS object representing a window rectangle change event.
     *
     * The resulting object has the following structure:
     * {
     *   rect: { x, y, width, height },
     *   reason: <RectChangeReason enum value>
     * }
     *
     * Used to pass rectangle change info from native code to JavaScript.
     *
     * @param env The NAPI environment.
     * @param rect The window rectangle.
     * @param reason The change reason.
     * @return The JS object on success, or nullptr on failure.
     */
    napi_value BuildJsRectChangeOptions(napi_env env, const Rect& rect, RectChangeReason reason);

    /**
     * @brief Create a JS object from a native Position.
     *
     * The resulting object has the form:
     * {
     *   x: <number>,
     *   y: <number>
     * }
     *
     * @param env The NAPI environment.
     * @param position The Position to convert.
     * @return The JS object on success, or nullptr on failure.
     */
    napi_value BuildJsPosition(napi_env env, const Position& position);

    /**
     * @brief Safely converts WMError to WmErrorCode.
     *
     * @param err WMError value to be converted.
     * @return The corresponding WmErrorCode enumeration value, or WmErrorCode::WM_ERROR_STATE_ABNORMALLY if unmapped.
     */
    WmErrorCode MappingWmErrorCodeSafely(WMError err);
}
}
#endif