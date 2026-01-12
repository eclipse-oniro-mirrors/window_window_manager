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

#include "js_scene_utils.h"

#include <iomanip>

#include <event_handler.h>
#include <js_runtime_utils.h>
#include <napi_common_want.h>

#include "js_window_animation_utils.h"
#include "property/rs_properties_def.h"
#include "root_scene.h"
#include "session/host/include/pc_fold_screen_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"
#include "window_visibility_info.h"
#include "process_options.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsSceneUtils" };
constexpr int32_t US_PER_NS = 1000;
constexpr int32_t INVALID_VAL = -9999;

const std::unordered_map<int32_t, ThrowSlipMode> FINGERS_TO_THROWSLIPMODE_MAP = {
    { 3, ThrowSlipMode::THREE_FINGERS_SWIPE },
    { 4, ThrowSlipMode::FOUR_FINGERS_SWIPE },
    { 5, ThrowSlipMode::FIVE_FINGERS_SWIPE }
};

// Refer to OHOS::Ace::TouchType
enum class AceTouchType : int32_t {
    DOWN = 0,
    UP,
    MOVE,
    CANCEL,
    HOVER_ENTER = 9,
    HOVER_MOVE = 10,
    HOVER_EXIT = 11,
    HOVER_CANCEL = 12,
};

// Refer to OHOS::Ace::SourceType
enum class AceSourceType : int32_t {
    NONE = 0,
    MOUSE = 1,
    TOUCH = 2,
    TOUCH_PAD = 3,
    KEYBOARD = 4
};

const std::map<int32_t, int32_t> TOUCH_ACTION_MAP = {
    { (int32_t)AceTouchType::DOWN, MMI::PointerEvent::POINTER_ACTION_DOWN },
    { (int32_t)AceTouchType::UP, MMI::PointerEvent::POINTER_ACTION_UP },
    { (int32_t)AceTouchType::MOVE, MMI::PointerEvent::POINTER_ACTION_MOVE },
    { (int32_t)AceTouchType::CANCEL, MMI::PointerEvent::POINTER_ACTION_CANCEL },
    { (int32_t)AceTouchType::HOVER_ENTER, MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER },
    { (int32_t)AceTouchType::HOVER_MOVE, MMI::PointerEvent::POINTER_ACTION_HOVER_MOVE },
    { (int32_t)AceTouchType::HOVER_EXIT, MMI::PointerEvent::POINTER_ACTION_HOVER_EXIT },
    { (int32_t)AceTouchType::HOVER_CANCEL, MMI::PointerEvent::POINTER_ACTION_HOVER_CANCEL }
};

const std::map<int32_t, int32_t> MOUSE_ACTION_MAP = {
    { (int32_t)AceTouchType::DOWN, MMI::PointerEvent::POINTER_ACTION_DOWN },
    { (int32_t)AceTouchType::UP, MMI::PointerEvent::POINTER_ACTION_UP },
    { (int32_t)AceTouchType::MOVE, MMI::PointerEvent::POINTER_ACTION_MOVE },
    { (int32_t)AceTouchType::CANCEL, MMI::PointerEvent::POINTER_ACTION_CANCEL },
    { (int32_t)AceTouchType::HOVER_ENTER, MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER },
    { (int32_t)AceTouchType::HOVER_MOVE, MMI::PointerEvent::POINTER_ACTION_HOVER_MOVE },
    { (int32_t)AceTouchType::HOVER_EXIT, MMI::PointerEvent::POINTER_ACTION_HOVER_EXIT },
    { (int32_t)AceTouchType::HOVER_CANCEL, MMI::PointerEvent::POINTER_ACTION_HOVER_CANCEL }
};

int32_t GetMMITouchType(int32_t aceType)
{
    auto it = TOUCH_ACTION_MAP.find(aceType);
    if (it == TOUCH_ACTION_MAP.end()) {
        return MMI::PointerEvent::POINTER_ACTION_UNKNOWN;
    }
    return it->second;
}
} // namespace

const std::map<WindowType, JsSessionType> WINDOW_TO_JS_SESSION_TYPE_MAP {
    { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,          JsSessionType::TYPE_APP                     },
    { WindowType::WINDOW_TYPE_APP_SUB_WINDOW,           JsSessionType::TYPE_SUB_APP                 },
    { WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,      JsSessionType::TYPE_SYSTEM_ALERT            },
    { WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT,       JsSessionType::TYPE_INPUT_METHOD            },
    { WindowType::WINDOW_TYPE_STATUS_BAR,               JsSessionType::TYPE_STATUS_BAR              },
    { WindowType::WINDOW_TYPE_PANEL,                    JsSessionType::TYPE_PANEL                   },
    { WindowType::WINDOW_TYPE_KEYGUARD,                 JsSessionType::TYPE_KEYGUARD                },
    { WindowType::WINDOW_TYPE_VOLUME_OVERLAY,           JsSessionType::TYPE_VOLUME_OVERLAY          },
    { WindowType::WINDOW_TYPE_NAVIGATION_BAR,           JsSessionType::TYPE_NAVIGATION_BAR          },
    { WindowType::WINDOW_TYPE_FLOAT,                    JsSessionType::TYPE_FLOAT                   },
    { WindowType::WINDOW_TYPE_WALLPAPER,                JsSessionType::TYPE_WALLPAPER               },
    { WindowType::WINDOW_TYPE_DESKTOP,                  JsSessionType::TYPE_DESKTOP                 },
    { WindowType::WINDOW_TYPE_LAUNCHER_DOCK,            JsSessionType::TYPE_LAUNCHER_DOCK           },
    { WindowType::WINDOW_TYPE_FLOAT_CAMERA,             JsSessionType::TYPE_FLOAT_CAMERA            },
    { WindowType::WINDOW_TYPE_DIALOG,                   JsSessionType::TYPE_DIALOG                  },
    { WindowType::WINDOW_TYPE_SCREENSHOT,               JsSessionType::TYPE_SCREENSHOT              },
    { WindowType::WINDOW_TYPE_TOAST,                    JsSessionType::TYPE_TOAST                   },
    { WindowType::WINDOW_TYPE_POINTER,                  JsSessionType::TYPE_POINTER                 },
    { WindowType::WINDOW_TYPE_LAUNCHER_RECENT,          JsSessionType::TYPE_LAUNCHER_RECENT         },
    { WindowType::WINDOW_TYPE_SCENE_BOARD,              JsSessionType::TYPE_SCENE_BOARD             },
    { WindowType::WINDOW_TYPE_DRAGGING_EFFECT,          JsSessionType::TYPE_DRAGGING_EFFECT         },
    { WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR,  JsSessionType::TYPE_INPUT_METHOD_STATUS_BAR },
    { WindowType::WINDOW_TYPE_GLOBAL_SEARCH,            JsSessionType::TYPE_GLOBAL_SEARCH           },
    { WindowType::WINDOW_TYPE_NEGATIVE_SCREEN,          JsSessionType::TYPE_NEGATIVE_SCREEN         },
    { WindowType::WINDOW_TYPE_VOICE_INTERACTION,        JsSessionType::TYPE_VOICE_INTERACTION       },
    { WindowType::WINDOW_TYPE_SYSTEM_TOAST,             JsSessionType::TYPE_SYSTEM_TOAST            },
    { WindowType::WINDOW_TYPE_SYSTEM_FLOAT,             JsSessionType::TYPE_SYSTEM_FLOAT            },
    { WindowType::WINDOW_TYPE_THEME_EDITOR,             JsSessionType::TYPE_THEME_EDITOR            },
    { WindowType::WINDOW_TYPE_PIP,                      JsSessionType::TYPE_PIP                     },
    { WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR,     JsSessionType::TYPE_NAVIGATION_INDICATOR    },
    { WindowType::WINDOW_TYPE_SEARCHING_BAR,            JsSessionType::TYPE_SEARCHING_BAR           },
    { WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW,        JsSessionType::TYPE_SYSTEM_SUB_WINDOW       },
    { WindowType::WINDOW_TYPE_HANDWRITE,                JsSessionType::TYPE_HANDWRITE               },
    { WindowType::WINDOW_TYPE_KEYBOARD_PANEL,           JsSessionType::TYPE_KEYBOARD_PANEL          },
    { WindowType::WINDOW_TYPE_DOCK_SLICE,               JsSessionType::TYPE_DIVIDER                 },
    { WindowType::WINDOW_TYPE_TRANSPARENT_VIEW,         JsSessionType::TYPE_TRANSPARENT_VIEW        },
    { WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD,        JsSessionType::TYPE_WALLET_SWIPE_CARD       },
    { WindowType::WINDOW_TYPE_SCREEN_CONTROL,           JsSessionType::TYPE_SCREEN_CONTROL          },
    { WindowType::WINDOW_TYPE_FLOAT_NAVIGATION,         JsSessionType::TYPE_FLOAT_NAVIGATION        },
    { WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION, JsSessionType::TYPE_MUTISCREEN_COLLABORATION},
    { WindowType::WINDOW_TYPE_DYNAMIC,                  JsSessionType::TYPE_DYNAMIC                 },
    { WindowType::WINDOW_TYPE_MAGNIFICATION,            JsSessionType::TYPE_MAGNIFICATION           },
    { WindowType::WINDOW_TYPE_MAGNIFICATION_MENU,       JsSessionType::TYPE_MAGNIFICATION_MENU      },
    { WindowType::WINDOW_TYPE_SELECTION,                JsSessionType::TYPE_SELECTION               },
    { WindowType::WINDOW_TYPE_FB,                       JsSessionType::TYPE_FLOATING_BALL           },
};

const std::map<JsSessionType, WindowType> JS_SESSION_TO_WINDOW_TYPE_MAP {
    { JsSessionType::TYPE_APP,                      WindowType::WINDOW_TYPE_APP_MAIN_WINDOW         },
    { JsSessionType::TYPE_SUB_APP,                  WindowType::WINDOW_TYPE_APP_SUB_WINDOW          },
    { JsSessionType::TYPE_SYSTEM_ALERT,             WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW     },
    { JsSessionType::TYPE_INPUT_METHOD,             WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT      },
    { JsSessionType::TYPE_STATUS_BAR,               WindowType::WINDOW_TYPE_STATUS_BAR              },
    { JsSessionType::TYPE_PANEL,                    WindowType::WINDOW_TYPE_PANEL                   },
    { JsSessionType::TYPE_KEYGUARD,                 WindowType::WINDOW_TYPE_KEYGUARD                },
    { JsSessionType::TYPE_VOLUME_OVERLAY,           WindowType::WINDOW_TYPE_VOLUME_OVERLAY          },
    { JsSessionType::TYPE_NAVIGATION_BAR,           WindowType::WINDOW_TYPE_NAVIGATION_BAR          },
    { JsSessionType::TYPE_FLOAT,                    WindowType::WINDOW_TYPE_FLOAT                   },
    { JsSessionType::TYPE_WALLPAPER,                WindowType::WINDOW_TYPE_WALLPAPER               },
    { JsSessionType::TYPE_DESKTOP,                  WindowType::WINDOW_TYPE_DESKTOP                 },
    { JsSessionType::TYPE_LAUNCHER_DOCK,            WindowType::WINDOW_TYPE_LAUNCHER_DOCK           },
    { JsSessionType::TYPE_FLOAT_CAMERA,             WindowType::WINDOW_TYPE_FLOAT_CAMERA            },
    { JsSessionType::TYPE_DIALOG,                   WindowType::WINDOW_TYPE_DIALOG                  },
    { JsSessionType::TYPE_SCREENSHOT,               WindowType::WINDOW_TYPE_SCREENSHOT              },
    { JsSessionType::TYPE_TOAST,                    WindowType::WINDOW_TYPE_TOAST                   },
    { JsSessionType::TYPE_POINTER,                  WindowType::WINDOW_TYPE_POINTER                 },
    { JsSessionType::TYPE_LAUNCHER_RECENT,          WindowType::WINDOW_TYPE_LAUNCHER_RECENT         },
    { JsSessionType::TYPE_SCENE_BOARD,              WindowType::WINDOW_TYPE_SCENE_BOARD             },
    { JsSessionType::TYPE_DRAGGING_EFFECT,          WindowType::WINDOW_TYPE_DRAGGING_EFFECT         },
    { JsSessionType::TYPE_INPUT_METHOD_STATUS_BAR,  WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR },
    { JsSessionType::TYPE_GLOBAL_SEARCH,            WindowType::WINDOW_TYPE_GLOBAL_SEARCH           },
    { JsSessionType::TYPE_NEGATIVE_SCREEN,          WindowType::WINDOW_TYPE_NEGATIVE_SCREEN         },
    { JsSessionType::TYPE_VOICE_INTERACTION,        WindowType::WINDOW_TYPE_VOICE_INTERACTION       },
    { JsSessionType::TYPE_SYSTEM_TOAST,             WindowType::WINDOW_TYPE_SYSTEM_TOAST,           },
    { JsSessionType::TYPE_SYSTEM_FLOAT,             WindowType::WINDOW_TYPE_SYSTEM_FLOAT,           },
    { JsSessionType::TYPE_THEME_EDITOR,             WindowType::WINDOW_TYPE_THEME_EDITOR            },
    { JsSessionType::TYPE_PIP,                      WindowType::WINDOW_TYPE_PIP,                    },
    { JsSessionType::TYPE_NAVIGATION_INDICATOR,     WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR    },
    { JsSessionType::TYPE_SEARCHING_BAR,            WindowType::WINDOW_TYPE_SEARCHING_BAR           },
    { JsSessionType::TYPE_SYSTEM_SUB_WINDOW,        WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW       },
    { JsSessionType::TYPE_HANDWRITE,                WindowType::WINDOW_TYPE_HANDWRITE               },
    { JsSessionType::TYPE_KEYBOARD_PANEL,           WindowType::WINDOW_TYPE_KEYBOARD_PANEL          },
    { JsSessionType::TYPE_DIVIDER,                  WindowType::WINDOW_TYPE_DOCK_SLICE              },
    { JsSessionType::TYPE_TRANSPARENT_VIEW,         WindowType::WINDOW_TYPE_TRANSPARENT_VIEW        },
    { JsSessionType::TYPE_WALLET_SWIPE_CARD,        WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD       },
    { JsSessionType::TYPE_SCREEN_CONTROL,           WindowType::WINDOW_TYPE_SCREEN_CONTROL          },
    { JsSessionType::TYPE_FLOAT_NAVIGATION,         WindowType::WINDOW_TYPE_FLOAT_NAVIGATION        },
    { JsSessionType::TYPE_MUTISCREEN_COLLABORATION, WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION},
    { JsSessionType::TYPE_DYNAMIC,                  WindowType::WINDOW_TYPE_DYNAMIC                 },
    { JsSessionType::TYPE_MAGNIFICATION,            WindowType::WINDOW_TYPE_MAGNIFICATION           },
    { JsSessionType::TYPE_MAGNIFICATION_MENU,       WindowType::WINDOW_TYPE_MAGNIFICATION_MENU      },
    { JsSessionType::TYPE_SELECTION,                WindowType::WINDOW_TYPE_SELECTION               },
    { JsSessionType::TYPE_FLOATING_BALL,            WindowType::WINDOW_TYPE_FB                      },
};

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

template<class T>
bool ConvertFromJsValueProperty(napi_env env, napi_value jsValue, const char* name, T& value)
{
    napi_value jsProperty = nullptr;
    napi_get_named_property(env, jsValue, name, &jsProperty);
    if (GetType(env, jsProperty) != napi_undefined) {
        T propertyValue;
        if (!ConvertFromJsValue(env, jsProperty, propertyValue)) {
            TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to %{public}s", name);
            return false;
        }
        value = propertyValue;
    }
    return true;
}

napi_value ConvertTransitionAnimationToJsValue(napi_env env, std::shared_ptr<TransitionAnimation> transitionAnimation)
{
    napi_value objValue = nullptr;
    if (!transitionAnimation) {
        return objValue;
    }
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_value configJsValue = ConvertWindowAnimationOptionToJsValue(env, transitionAnimation->config);
    if (!configJsValue) {
        return nullptr;
    }
    napi_set_named_property(env, objValue, "config", configJsValue);
    napi_set_named_property(env, objValue, "opacity", CreateJsValue(env, transitionAnimation->opacity));

    return objValue;
}

napi_value ConvertWindowAnimationPropertyToJsValue(napi_env env, const WindowAnimationProperty& animationProperty)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "targetScale", CreateJsValue(env, animationProperty.targetScale));
    return objValue;
}

napi_value ConvertWindowAnimationOptionToJsValue(napi_env env,
    const WindowAnimationOption& animationConfig)
{
    napi_value configJsValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, configJsValue);
    napi_set_named_property(env, configJsValue, "curve", CreateJsValue(env, animationConfig.curve));
    switch (animationConfig.curve) {
        case WindowAnimationCurve::LINEAR: {
            napi_set_named_property(env, configJsValue, "duration", CreateJsValue(env, animationConfig.duration));
            break;
        }
        case WindowAnimationCurve::CUBIC_BEZIER: {
            napi_set_named_property(env, configJsValue, "duration", CreateJsValue(env, animationConfig.duration));
            [[fallthrough]];
        }
        case WindowAnimationCurve::INTERPOLATION_SPRING: {
            napi_value params = nullptr;
            napi_create_array(env, &params);
            for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
                napi_value element;
                napi_create_double(env, static_cast<double>(animationConfig.param[i]), &element);
                napi_set_element(env, params, i, element);
            }
            napi_set_named_property(env, configJsValue, "param", params);
            break;
        }
        default:
            break;
    }
    return configJsValue;
}

napi_value AnimationTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "FADE_IN_OUT",
        CreateJsValue(env, static_cast<uint32_t>(AnimationType::FADE_IN_OUT)));
    napi_set_named_property(env, objValue, "FADE_IN",
        CreateJsValue(env, static_cast<uint32_t>(AnimationType::FADE_IN)));
    napi_set_named_property(env, objValue, "SEE_THE_WORLD",
        CreateJsValue(env, static_cast<uint32_t>(AnimationType::SEE_THE_WORLD)));
    return objValue;
}

napi_value WindowTransitionTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "DESTROY",
        CreateJsValue(env, static_cast<uint32_t>(WindowTransitionType::DESTROY)));
    return objValue;
}

napi_value WindowAnimationCurveInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "LINEAR",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnimationCurve::LINEAR)));
    napi_set_named_property(env, objValue, "INTERPOLATION_SPRING",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnimationCurve::INTERPOLATION_SPRING)));
    return objValue;
}

WSError GetIntValueFromString(const std::string& str, uint32_t& value)
{
    char* end;
    value = strtoul(str.c_str(), &end, 10); // 10 number convert base
    if (*end == '\0' && value != 0) {
        return WSError::WS_OK;
    }
    TLOGE(WmsLogTag::WMS_PC, "param %{public}s convert int failed", str.c_str());
    return WSError::WS_ERROR_INVALID_PARAM;
}

bool IsJsBundleNameUndefind(napi_env env, napi_value jsBundleName, SessionInfo& sessionInfo)
{
    if (GetType(env, jsBundleName) != napi_undefined) {
        std::string bundleName;
        if (!ConvertFromJsValue(env, jsBundleName, bundleName)) {
            WLOGFE("Failed to convert parameter to bundleName");
            return false;
        }
        sessionInfo.bundleName_ = bundleName;
    }
    return true;
}

bool IsJsModuleNameUndefind(napi_env env, napi_value jsModuleName, SessionInfo& sessionInfo)
{
    if (GetType(env, jsModuleName) != napi_undefined) {
        std::string moduleName;
        if (!ConvertFromJsValue(env, jsModuleName, moduleName)) {
            WLOGFE("Failed to convert parameter to moduleName");
            return false;
        }
        sessionInfo.moduleName_ = moduleName;
    }
    return true;
}

bool IsJsAbilityUndefind(napi_env env, napi_value jsAbilityName, SessionInfo& sessionInfo)
{
    if (GetType(env, jsAbilityName) != napi_undefined) {
        std::string abilityName;
        if (!ConvertFromJsValue(env, jsAbilityName, abilityName)) {
            WLOGFE("Failed to convert parameter to abilityName");
            return false;
        }
        sessionInfo.abilityName_ = abilityName;
    }
    return true;
}

bool IsJsAppIndexUndefind(napi_env env, napi_value jsAppIndex, SessionInfo& sessionInfo)
{
    if (GetType(env, jsAppIndex) != napi_undefined) {
        int32_t appIndex;
        if (!ConvertFromJsValue(env, jsAppIndex, appIndex)) {
            WLOGFE("Failed to convert parameter to appIndex");
            return false;
        }
        sessionInfo.appIndex_ = appIndex;
    }
    return true;
}

bool IsJsIsSystemUndefind(napi_env env, napi_value jsIsSystem, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsSystem) != napi_undefined) {
        bool isSystem;
        if (!ConvertFromJsValue(env, jsIsSystem, isSystem)) {
            WLOGFE("Failed to convert parameter to isSystem");
            return false;
        }
        sessionInfo.isSystem_ = isSystem;
    }
    return true;
}

bool IsJsSceneTypeUndefined(napi_env env, napi_value jsSceneType, SessionInfo& sessionInfo)
{
    if (GetType(env, jsSceneType) != napi_undefined) {
        uint32_t sceneType;
        if (!ConvertFromJsValue(env, jsSceneType, sceneType)) {
            WLOGFE("Failed to convert parameter to sceneType");
            return false;
        }
        sessionInfo.sceneType_ = static_cast<SceneType>(sceneType);
    } else if (sessionInfo.isSystem_) {
        sessionInfo.sceneType_ = SceneType::SYSTEM_WINDOW_SCENE;
    }
    return true;
}

bool IsJsPersistentIdUndefind(napi_env env, napi_value jsPersistentId, SessionInfo& sessionInfo)
{
    if (GetType(env, jsPersistentId) != napi_undefined) {
        int32_t persistentId;
        if (!ConvertFromJsValue(env, jsPersistentId, persistentId)) {
            WLOGFE("Failed to convert parameter to persistentId");
            return false;
        }
        sessionInfo.persistentId_ = persistentId;
    }
    return true;
}

bool IsJsCallStateUndefind(napi_env env, napi_value jsCallState, SessionInfo& sessionInfo)
{
    if (GetType(env, jsCallState) != napi_undefined) {
        int32_t callState;
        if (!ConvertFromJsValue(env, jsCallState, callState)) {
            WLOGFE("Failed to convert parameter to callState");
            return false;
        }
        sessionInfo.callState_ = static_cast<uint32_t>(callState);
    }
    return true;
}

bool IsJsWindowInputTypeUndefind(napi_env env, napi_value jsWindowInputType, SessionInfo& sessionInfo)
{
    if (GetType(env, jsWindowInputType) != napi_undefined) {
        uint32_t windowInputType = 0;
        if (!ConvertFromJsValue(env, jsWindowInputType, windowInputType)) {
            WLOGFE("Failed to convert parameter to windowInputType");
            return false;
        }
        sessionInfo.windowInputType_ = static_cast<uint32_t>(windowInputType);
    }
    return true;
}

bool IsJsSessionTypeUndefind(napi_env env, napi_value jsSessionType, SessionInfo& sessionInfo)
{
    uint32_t windowType = 0;
    if (GetType(env, jsSessionType) != napi_undefined) {
        if (!ConvertFromJsValue(env, jsSessionType, windowType)) {
            WLOGFE("Failed to convert parameter to windowType");
            return false;
        }
        if (JS_SESSION_TO_WINDOW_TYPE_MAP.count(static_cast<JsSessionType>(windowType)) != 0) {
            sessionInfo.windowType_ = static_cast<uint32_t>(
                JS_SESSION_TO_WINDOW_TYPE_MAP.at(static_cast<JsSessionType>(windowType)));
        }
    }
    if (windowType == 0 && sessionInfo.isSystem_) {
        sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SCB_DEFAULT);
    }
    return true;
}

bool IsJsScreenIdUndefind(napi_env env, napi_value jsScreenId, SessionInfo& sessionInfo)
{
    if (GetType(env, jsScreenId) != napi_undefined) {
        int32_t screenId = -1;
        if (!ConvertFromJsValue(env, jsScreenId, screenId)) {
            WLOGFE("Failed to convert parameter to screenId");
            return false;
        }
        sessionInfo.screenId_ = static_cast<uint64_t>(screenId);
    }
    return true;
}

bool IsJsIsPersistentRecoverUndefined(napi_env env, napi_value jsIsPersistentRecover, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsPersistentRecover) != napi_undefined) {
        bool isPersistentRecover = false;
        if (!ConvertFromJsValue(env, jsIsPersistentRecover, isPersistentRecover)) {
            WLOGFE("Failed to convert parameter to isPersistentRecover");
            return false;
        }
        sessionInfo.isPersistentRecover_ = isPersistentRecover;
    }
    return true;
}

bool IsJsIsRotatableUndefined(napi_env env, napi_value jsIsRotatable, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsRotatable) != napi_undefined) {
        bool isRotable = false;
        if (!ConvertFromJsValue(env, jsIsRotatable, isRotable)) {
            WLOGFE("Failed to convert parameter to isRotable");
            return false;
        }
        sessionInfo.isRotable_ = isRotable;
    }
    return true;
}

bool IsJsSpecifiedReasonUndefined(napi_env env, napi_value jsSpecifiedReason, SessionInfo& sessionInfo)
{
    if (GetType(env, jsSpecifiedReason) != napi_undefined) {
        int32_t specifiedReason = static_cast<int32_t>(SpecifiedReason::BY_SCB);
        if (!ConvertFromJsValue(env, jsSpecifiedReason, specifiedReason)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to specifiedReason");
            return false;
        }
        sessionInfo.specifiedReason_ = static_cast<SpecifiedReason>(specifiedReason);
    }
    return true;
}

bool IsJsProcessOptionUndefined(napi_env env, napi_value jsProcessOption, SessionInfo& sessionInfo)
{
    if (GetType(env, jsProcessOption) != napi_undefined) {
        std::shared_ptr<AAFwk::ProcessOptions> processOptions = std::make_shared<AAFwk::ProcessOptions>();
        if (!ConvertProcessOptionFromJs(env, jsProcessOption, processOptions)) {
            WLOGFE("Failed to convert parameter to processOptions");
            return false;
        }
        sessionInfo.processOptions = processOptions;
    }
    return true;
}

bool IsJsIsSetPointerAreasUndefined(napi_env env, napi_value jsIsSetPointerAreas, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsSetPointerAreas) != napi_undefined) {
        bool isSetPointerAreas = false;
        if (!ConvertFromJsValue(env, jsIsSetPointerAreas, isSetPointerAreas)) {
            WLOGFE("Failed to convert parameter to isSetPointerAreas");
            return false;
        }
        sessionInfo.isSetPointerAreas_ = isSetPointerAreas;
    }
    return true;
}

bool IsJsFullScreenStartUndefined(napi_env env, napi_value jsFullscreenStart, SessionInfo& sessionInfo)
{
    if (GetType(env, jsFullscreenStart) != napi_undefined) {
        bool fullScreenStart = false;
        if (!ConvertFromJsValue(env, jsFullscreenStart, fullScreenStart)) {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to fullScreenStart");
            return false;
        }
        sessionInfo.fullScreenStart_ = fullScreenStart;
    }
    return true;
}

bool IsJsIsNewAppInstanceUndefined(napi_env env, napi_value jsIsNewAppInstance, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsNewAppInstance) != napi_undefined) {
        bool isNewAppInstance = false;
        if (!ConvertFromJsValue(env, jsIsNewAppInstance, isNewAppInstance)) {
            TLOGI(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isNewAppInstance");
            return false;
        }
        sessionInfo.isNewAppInstance_ = isNewAppInstance;
    }
    return true;
}

bool IsJsInstanceKeyUndefined(napi_env env, napi_value jsInstanceKey, SessionInfo& sessionInfo)
{
    if (GetType(env, jsInstanceKey) != napi_undefined) {
        std::string instanceKey;
        if (!ConvertFromJsValue(env, jsInstanceKey, instanceKey)) {
            TLOGI(WmsLogTag::WMS_LIFE, "Failed to convert parameter to instanceKey");
            return false;
        }
        sessionInfo.appInstanceKey_ = instanceKey;
    }
    return true;
}

static bool IsJsIsUseControlSessionUndefined(napi_env env, napi_value jsIsUseControlSession, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsUseControlSession) != napi_undefined) {
        bool isUseControlSession = false;
        if (!ConvertFromJsValue(env, jsIsUseControlSession, isUseControlSession)) {
            TLOGI(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isUseControlSession");
            return false;
        }
        sessionInfo.isUseControlSession = isUseControlSession;
    }
    return true;
}

static bool IsJsIsAbilityHookUndefind(napi_env env, napi_value jsIsAbilityHook, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsAbilityHook) != napi_undefined) {
        bool isAbilityHook = false;
        if (!ConvertFromJsValue(env, jsIsAbilityHook, isAbilityHook)) {
            TLOGI(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isAbilityHook");
            return false;
        }
        sessionInfo.isAbilityHook_ = isAbilityHook;
    }
    return true;
}

static bool IsJsIsAncoApplicationUndefind(napi_env env, napi_value jsIsAncoApplication, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsAncoApplication) != napi_undefined) {
        bool isAncoApplication = false;
        if (!ConvertFromJsValue(env, jsIsAncoApplication, isAncoApplication)) {
            TLOGI(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isAncoApplication");
            return false;
        }
        sessionInfo.isAncoApplication_ = isAncoApplication;
    }
    return true;
}

static napi_value CreateJsValueFromStringArray(napi_env env, const std::vector<std::string>& stringArray)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, stringArray.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    int32_t index = 0;
    for (const auto& iter : stringArray) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, iter));
    }
    return arrayValue;
}

napi_value CreateJsAtomicServiceInfo(napi_env env, const AtomicServiceInfo& atomicServiceInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "resizable",
        CreateJsValue(env, atomicServiceInfo.resizable_));
    napi_set_named_property(env, objValue, "deviceTypes",
        CreateJsValueFromStringArray(env, atomicServiceInfo.deviceTypes_));
    napi_set_named_property(env, objValue, "supportWindowMode",
        CreateJsValueFromStringArray(env, atomicServiceInfo.supportWindowMode_));

    return objValue;
}

static bool IsJsRequestIdUndefind(napi_env env, napi_value jsRequestId, SessionInfo& sessionInfo)
{
    if (GetType(env, jsRequestId) != napi_undefined) {
        int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID;
        if (!ConvertFromJsValue(env, jsRequestId, requestId)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to requestId");
            return false;
        }
        sessionInfo.requestId = requestId;
    }
    return true;
}

bool ConvertSessionInfoName(napi_env env, napi_value jsObject, SessionInfo& sessionInfo)
{
    napi_value jsBundleName = nullptr;
    napi_get_named_property(env, jsObject, "bundleName", &jsBundleName);
    napi_value jsModuleName = nullptr;
    napi_get_named_property(env, jsObject, "moduleName", &jsModuleName);
    napi_value jsAbilityName = nullptr;
    napi_get_named_property(env, jsObject, "abilityName", &jsAbilityName);
    napi_value jsAppIndex = nullptr;
    napi_get_named_property(env, jsObject, "appIndex", &jsAppIndex);
    napi_value jsIsSystem = nullptr;
    napi_get_named_property(env, jsObject, "isSystem", &jsIsSystem);
    napi_value jsSceneType = nullptr;
    napi_get_named_property(env, jsObject, "sceneType", &jsSceneType);
    napi_value jsWindowInputType = nullptr;
    napi_get_named_property(env, jsObject, "windowInputType", &jsWindowInputType);
    napi_value jsFullScreenStart = nullptr;
    napi_get_named_property(env, jsObject, "fullScreenStart", &jsFullScreenStart);
    napi_value jsIsNewAppInstance = nullptr;
    napi_get_named_property(env, jsObject, "isNewAppInstance", &jsIsNewAppInstance);
    napi_value jsInstanceKey = nullptr;
    napi_get_named_property(env, jsObject, "instanceKey", &jsInstanceKey);
    napi_value jsIsAbilityHook = nullptr;
    napi_get_named_property(env, jsObject, "isAbilityHook", &jsIsAbilityHook);
    napi_value jsRequestId = nullptr;
    napi_get_named_property(env, jsObject, "requestId", &jsRequestId);
    napi_value jsSpecifiedReason = nullptr;
    napi_get_named_property(env, jsObject, "specifiedReason", &jsRequestId);
    napi_value jsIsAncoApplication = nullptr;
    napi_get_named_property(env, jsObject, "isAncoApplication", &jsIsAncoApplication);
    if (!IsJsBundleNameUndefind(env, jsBundleName, sessionInfo)) {
        return false;
    }
    if (!IsJsModuleNameUndefind(env, jsModuleName, sessionInfo)) {
        return false;
    }
    if (!IsJsAbilityUndefind(env, jsAbilityName, sessionInfo)) {
        return false;
    }
    if (!IsJsAppIndexUndefind(env, jsAppIndex, sessionInfo) ||
        !IsJsIsSystemUndefind(env, jsIsSystem, sessionInfo)) {
        return false;
    }
    if (!IsJsSceneTypeUndefined(env, jsSceneType, sessionInfo) ||
        !IsJsFullScreenStartUndefined(env, jsFullScreenStart, sessionInfo) ||
        !IsJsIsNewAppInstanceUndefined(env, jsIsNewAppInstance, sessionInfo) ||
        !IsJsInstanceKeyUndefined(env, jsInstanceKey, sessionInfo) ||
        !IsJsWindowInputTypeUndefind(env, jsWindowInputType, sessionInfo) ||
        !IsJsIsAbilityHookUndefind(env, jsIsAbilityHook, sessionInfo) ||
        !IsJsRequestIdUndefind(env, jsRequestId, sessionInfo) ||
        !IsJsSpecifiedReasonUndefined(env, jsSpecifiedReason, sessionInfo) ||
        !IsJsIsAncoApplicationUndefind(env, jsIsAncoApplication, sessionInfo)) {
        return false;
    }
    return true;
}

bool ConvertProcessOptionFromJs(napi_env env, napi_value jsObject,
    std::shared_ptr<AAFwk::ProcessOptions> processOptions)
{
    napi_value jsProcessMode = nullptr;
    napi_get_named_property(env, jsObject, "processMode", &jsProcessMode);
    napi_value jsStartupVisibility = nullptr;
    napi_get_named_property(env, jsObject, "startupVisibility", &jsStartupVisibility);

    int32_t processMode;
    if (!ConvertFromJsValue(env, jsProcessMode, processMode)) {
        WLOGFE("Failed to convert parameter to processMode");
        return false;
    }

    int32_t startupVisibility;
    if (!ConvertFromJsValue(env, jsStartupVisibility, startupVisibility)) {
        WLOGFE("Failed to convert parameter to startupVisibility");
        return false;
    }
    processOptions->processMode = static_cast<AAFwk::ProcessMode>(processMode);
    processOptions->startupVisibility = static_cast<AAFwk::StartupVisibility>(startupVisibility);

    return true;
}

bool ConvertSessionInfoState(napi_env env, napi_value jsObject, SessionInfo& sessionInfo)
{
    napi_value jsPersistentId = nullptr;
    napi_get_named_property(env, jsObject, "persistentId", &jsPersistentId);
    napi_value jsCallState = nullptr;
    napi_get_named_property(env, jsObject, "callState", &jsCallState);
    napi_value jsSessionType = nullptr;
    napi_get_named_property(env, jsObject, "sessionType", &jsSessionType);
    napi_value jsIsPersistentRecover = nullptr;
    napi_get_named_property(env, jsObject, "isPersistentRecover", &jsIsPersistentRecover);
    napi_value jsScreenId = nullptr;
    napi_get_named_property(env, jsObject, "screenId", &jsScreenId);
    napi_value jsIsRotable = nullptr;
    napi_get_named_property(env, jsObject, "isRotatable", &jsIsRotable);
    napi_value jsIsSetPointerAreas = nullptr;
    napi_get_named_property(env, jsObject, "isSetPointerAreas", &jsIsSetPointerAreas);
    napi_value jsProcessOption = nullptr;
    napi_get_named_property(env, jsObject, "processOptions", &jsProcessOption);
    napi_value jsIsUseControlSession = nullptr;
    napi_get_named_property(env, jsObject, "isAppUseControl", &jsIsUseControlSession);

    if (!IsJsPersistentIdUndefind(env, jsPersistentId, sessionInfo)) {
        return false;
    }
    if (!IsJsCallStateUndefind(env, jsCallState, sessionInfo)) {
        return false;
    }
    if (!IsJsSessionTypeUndefind(env, jsSessionType, sessionInfo)) {
        return false;
    }
    if (!IsJsScreenIdUndefind(env, jsScreenId, sessionInfo)) {
        return false;
    }
    if (!IsJsIsPersistentRecoverUndefined(env, jsIsPersistentRecover, sessionInfo)) {
        return false;
    }
    if (!IsJsIsRotatableUndefined(env, jsIsRotable, sessionInfo)) {
        return false;
    }
    if (!IsJsIsSetPointerAreasUndefined(env, jsIsSetPointerAreas, sessionInfo)) {
        return false;
    }
    if (!IsJsProcessOptionUndefined(env, jsProcessOption, sessionInfo)) {
        return false;
    }
    if (!IsJsIsUseControlSessionUndefined(env, jsIsUseControlSession, sessionInfo)) {
        return false;
    }
    if (!ConvertFromJsValueProperty(env, jsObject, "hasPrivacyModeControl", sessionInfo.hasPrivacyModeControl)) {
        return false;
    }
    if (!ConvertFromJsValueProperty(env, jsObject, "windowMode", sessionInfo.windowMode)) {
        return false;
    }
    if (!ConvertFromJsValueProperty(env, jsObject, "pageConfig", sessionInfo.pageConfig)) {
        return false;
    }
    return true;
}

napi_value CreateJsProcessOption(napi_env env, std::shared_ptr<AAFwk::ProcessOptions> processOptions)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    int32_t processMode = static_cast<int32_t>(processOptions->processMode);
    int32_t startupVisibility = static_cast<int32_t>(processOptions->startupVisibility);

    napi_set_named_property(env, objValue, "processMode", CreateJsValue(env, processMode));
    napi_set_named_property(env, objValue, "startupVisibility", CreateJsValue(env, startupVisibility));

    return objValue;
}

bool ConvertSessionInfoFromJs(napi_env env, napi_value jsObject, SessionInfo& sessionInfo)
{
    if (!ConvertSessionInfoName(env, jsObject, sessionInfo)) {
        return false;
    }
    if (!ConvertSessionInfoState(env, jsObject, sessionInfo)) {
        return false;
    }
    return true;
}

bool IsJsObjNameUndefind(napi_env env, napi_value jsObjName, int32_t& objName)
{
    if (GetType(env, jsObjName) != napi_undefined) {
        if (!ConvertFromJsValue(env, jsObjName, objName)) {
            WLOGFE("Failed to convert parameter to objName");
            return false;
        }
    }
    return true;
}

bool ConvertSessionRectInfoFromJs(napi_env env, napi_value jsObject, WSRect& rect)
{
    napi_value jsPosX = nullptr;
    napi_get_named_property(env, jsObject, "posX_", &jsPosX);
    napi_value jsPosY = nullptr;
    napi_get_named_property(env, jsObject, "posY_", &jsPosY);
    napi_value jsWidth = nullptr;
    napi_get_named_property(env, jsObject, "width_", &jsWidth);
    napi_value jsHeight = nullptr;
    napi_get_named_property(env, jsObject, "height_", &jsHeight);

    if (!IsJsObjNameUndefind(env, jsPosX, rect.posX_)) {
        return false;
    }

    if (!IsJsObjNameUndefind(env, jsPosY, rect.posY_)) {
        return false;
    }

    if (!IsJsObjNameUndefind(env, jsWidth, rect.width_)) {
        return false;
    }

    if (!IsJsObjNameUndefind(env, jsHeight, rect.height_)) {
        return false;
    }
    return true;
}

bool ConvertSingleHandScreenInfoFromJs(napi_env env, napi_value jsObject, SingleHandScreenInfo& singleHandScreenInfo)
{
    napi_value jsScaleRatio = nullptr;
    napi_get_named_property(env, jsObject, "scaleRatio", &jsScaleRatio);
    napi_value jsPivotX = nullptr;
    napi_get_named_property(env, jsObject, "scalePivotX", &jsPivotX);
    napi_value jsPivotY = nullptr;
    napi_get_named_property(env, jsObject, "scalePivotY", &jsPivotY);
    napi_value jsSingleHandMode = nullptr;
    napi_get_named_property(env, jsObject, "singleHandMode", &jsSingleHandMode);

    if (!IsJsObjNameUndefind(env, jsScaleRatio, singleHandScreenInfo.scaleRatio)) {
        return false;
    }

    if (!IsJsObjNameUndefind(env, jsPivotX, singleHandScreenInfo.scalePivotX)) {
        return false;
    }

    if (!IsJsObjNameUndefind(env, jsPivotY, singleHandScreenInfo.scalePivotY)) {
        return false;
    }

    int32_t singleHandmode = 0;
    if (!IsJsObjNameUndefind(env, jsSingleHandMode, singleHandmode)) {
        return false;
    }
    singleHandScreenInfo.mode = static_cast<SingleHandMode>(singleHandmode);
    return true;
}


bool ConvertRectInfoFromJs(napi_env env, napi_value jsObject, WSRect& rect)
{
    napi_value jsLeftName = nullptr;
    napi_get_named_property(env, jsObject, "left", &jsLeftName);
    napi_value jsTopName = nullptr;
    napi_get_named_property(env, jsObject, "top", &jsTopName);
    napi_value jsRightName = nullptr;
    napi_get_named_property(env, jsObject, "right", &jsRightName);
    napi_value jsBottomName = nullptr;
    napi_get_named_property(env, jsObject, "bottom", &jsBottomName);

    int32_t leftName = 0;
    if (!IsJsObjNameUndefind(env, jsLeftName, leftName)) {
        return false;
    }
    rect.posX_ = leftName;

    int32_t topName = 0;
    if (!IsJsObjNameUndefind(env, jsTopName, topName)) {
        return false;
    }
    rect.posY_ = topName;

    int32_t rightName = 0;
    if (!IsJsObjNameUndefind(env, jsRightName, rightName)) {
        return false;
    }
    rect.width_ = rightName - rect.posX_;

    int32_t bottomName = 0;
    if (!IsJsObjNameUndefind(env, jsBottomName, bottomName)) {
        return false;
    }
    rect.height_ = bottomName - rect.posY_;
    return true;
}

bool ConvertHookInfoFromJs(napi_env env, napi_value jsObject, HookInfo& hookInfo)
{
    napi_value jsWidth = nullptr;
    napi_get_named_property(env, jsObject, "width", &jsWidth);
    napi_value jsHeight = nullptr;
    napi_get_named_property(env, jsObject, "height", &jsHeight);
    napi_value jsDensity = nullptr;
    napi_get_named_property(env, jsObject, "density", &jsDensity);
    napi_value jsRotation = nullptr;
    napi_get_named_property(env, jsObject, "rotation", &jsRotation);
    napi_value jsEnableHookRotation = nullptr;
    napi_get_named_property(env, jsObject, "enableHookRotation", &jsEnableHookRotation);
    napi_value jsDisplayOrientation = nullptr;
    napi_get_named_property(env, jsObject, "displayOrientation", &jsDisplayOrientation);
    napi_value jsEnableHookDisplayOrientation = nullptr;
    napi_get_named_property(env, jsObject, "enableHookDisplayOrientation", &jsEnableHookDisplayOrientation);

    uint32_t width = 0;
    if (!ConvertFromJsValue(env, jsWidth, width)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to width");
        return false;
    }
    hookInfo.width_ = width;

    uint32_t height = 0;
    if (!ConvertFromJsValue(env, jsHeight, height)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to height");
        return false;
    }
    hookInfo.height_ = height;

    double_t density = 1.0;
    if (!ConvertFromJsValue(env, jsDensity, density)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to density");
        return false;
    }
    hookInfo.density_ = static_cast<float_t>(density);

    uint32_t rotation = 0;
    if (!ConvertFromJsValue(env, jsRotation, rotation)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to rotation");
        return false;
    }
    hookInfo.rotation_ = rotation;

    bool enableHookRotation = false;
    if (!ConvertFromJsValue(env, jsEnableHookRotation, enableHookRotation)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to enableHookRotation");
        return false;
    }
    hookInfo.enableHookRotation_ = enableHookRotation;

    uint32_t displayOrientation = 0;
    if (!ConvertFromJsValue(env, jsDisplayOrientation, displayOrientation)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to displayOrientation");
        return false;
    }
    hookInfo.displayOrientation_ = displayOrientation;

    bool enableHookDisplayOrientation = false;
    if (!ConvertFromJsValue(env, jsEnableHookDisplayOrientation, enableHookDisplayOrientation)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to enableHookDisplayOrientation");
        return false;
    }
    hookInfo.enableHookDisplayOrientation_ = enableHookDisplayOrientation;
    return true;
}

bool ConvertHookWindowInfoFromJs(napi_env env, napi_value jsObject, HookWindowInfo& hookWindowInfo)
{
    napi_value jsEnableHookWindow = nullptr;
    napi_get_named_property(env, jsObject, "enableHookWindow", &jsEnableHookWindow);
    napi_value jsWidthHookRatio = nullptr;
    napi_get_named_property(env, jsObject, "widthHookRatio", &jsWidthHookRatio);

    bool enableHookWindow = false;
    if (!ConvertFromJsValue(env, jsEnableHookWindow, enableHookWindow)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to enableHookWindow");
        return false;
    }
    hookWindowInfo.enableHookWindow = enableHookWindow;

    double widthHookRatio = 1.0;
    if (!ConvertFromJsValue(env, jsWidthHookRatio, widthHookRatio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to widthHookRatio");
        return false;
    }
    hookWindowInfo.widthHookRatio = static_cast<float>(widthHookRatio);
    return true;
}

bool ConvertPointerItemFromJs(napi_env env, napi_value touchObject, MMI::PointerEvent& pointerEvent)
{
    auto vpr = RootScene::staticRootScene_->GetDisplayDensity();
    MMI::PointerEvent::PointerItem pointerItem;
    napi_value jsId = nullptr;
    napi_get_named_property(env, touchObject, "id", &jsId);
    napi_value jsWindowX = nullptr;
    napi_get_named_property(env, touchObject, "windowX", &jsWindowX);
    napi_value jsWindowY = nullptr;
    napi_get_named_property(env, touchObject, "windowY", &jsWindowY);
    napi_value jsDisplayX = nullptr;
    napi_get_named_property(env, touchObject, "displayX", &jsDisplayX);
    napi_value jsDisplayY = nullptr;
    napi_get_named_property(env, touchObject, "displayY", &jsDisplayY);
    napi_value jsPressure = nullptr;
    napi_get_named_property(env, touchObject, "pressure", &jsPressure);
    int32_t id;
    if (!ConvertFromJsValue(env, jsId, id)) {
        WLOGFE("Failed to convert parameter to id");
        return false;
    }
    pointerItem.SetPointerId(id);
    pointerEvent.SetPointerId(id);
    double windowX;
    if (!ConvertFromJsValue(env, jsWindowX, windowX)) {
        WLOGFE("Failed to convert parameter to windowX");
        return false;
    }
    pointerItem.SetWindowX(std::round(windowX * vpr));
    pointerItem.SetWindowXPos(windowX * vpr);
    double windowY;
    if (!ConvertFromJsValue(env, jsWindowY, windowY)) {
        WLOGFE("Failed to convert parameter to windowY");
        return false;
    }
    pointerItem.SetWindowY(std::round(windowY * vpr));
    pointerItem.SetWindowYPos(windowY * vpr);
    double displayX;
    if (!ConvertFromJsValue(env, jsDisplayX, displayX)) {
        WLOGFE("Failed to convert parameter to displayX");
        return false;
    }
    pointerItem.SetDisplayXPos(displayX * vpr);
    double displayY;
    if (!ConvertFromJsValue(env, jsDisplayY, displayY)) {
        WLOGFE("Failed to convert parameter to displayY");
        return false;
    }
    pointerItem.SetDisplayYPos(displayY * vpr);
    double pressure;
    if (!ConvertFromJsValue(env, jsPressure, pressure)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to pressure");
        return false;
    }
    pointerItem.SetPressure(pressure);
    pointerEvent.AddPointerItem(pointerItem);
    return true;
}

bool ConvertTouchesObjectFromJs(napi_env env, napi_value jsTouches, int32_t pointerId, MMI::PointerEvent& pointerEvent)
{
    // iterator touches
    if (jsTouches == nullptr) {
        WLOGFE("Failed to convert to touchesObject list");
        return false;
    }
    bool isArray = false;
    napi_is_array(env, jsTouches, &isArray);
    if (!isArray) {
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, jsTouches, &length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value touchesObject = nullptr;
        napi_get_element(env, jsTouches, i, &touchesObject);
        if (touchesObject == nullptr) {
            WLOGFE("Failed get to touchesObject");
            return false;
        }
        napi_value jsNoChangedId = nullptr;
        napi_get_named_property(env, touchesObject, "id", &jsNoChangedId);
        int32_t noChangedId;
        if (!ConvertFromJsValue(env, jsNoChangedId, noChangedId)) {
            WLOGFE("Failed to convert parameter to jsNoChangeId");
            return false;
        }
        if (pointerId == noChangedId) {
            continue;
        }
        if (!ConvertPointerItemFromJs(env, touchesObject, pointerEvent)) {
            return false;
        }
    }
    return true;
}

bool ConvertPointerEventFromJs(napi_env env, napi_value jsObject, MMI::PointerEvent& pointerEvent)
{
    napi_value jsSourceType = nullptr;
    napi_get_named_property(env, jsObject, "source", &jsSourceType);
    napi_value jsTimestamp = nullptr;
    napi_get_named_property(env, jsObject, "timestamp", &jsTimestamp);
    napi_value jsChangedTouches = nullptr;
    napi_get_named_property(env, jsObject, "changedTouches", &jsChangedTouches);
    napi_value jsTouches = nullptr;
    napi_get_named_property(env, jsObject, "touches", &jsTouches);
    int32_t sourceType;
    if (!ConvertFromJsValue(env, jsSourceType, sourceType)) {
        WLOGFE("Failed to convert parameter to sourceType");
        return false;
    }
    pointerEvent.SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    if (sourceType == static_cast<int32_t>(AceSourceType::MOUSE)) {
        pointerEvent.AddFlag(MMI::InputEvent::EVENT_FLAG_GESTURE_SUPPLEMENT);
    }
    double timestamp;
    if (!ConvertFromJsValue(env, jsTimestamp, timestamp)) {
        WLOGFE("Failed to convert parameter to timestamp");
        return false;
    }
    pointerEvent.SetActionTime(std::round(timestamp / US_PER_NS));
    if (jsChangedTouches == nullptr) {
        WLOGFE("Failed to convert parameter to touchesArray");
        return false;
    }
    // use changedTouches[0] only
    napi_value touchObject = nullptr;
    napi_get_element(env, jsChangedTouches, 0, &touchObject);
    if (touchObject == nullptr) {
        WLOGFE("Failed get to touchObject");
        return false;
    }
    napi_value jsTouchType = nullptr;
    napi_get_named_property(env, touchObject, "type", &jsTouchType);
    int32_t touchType;
    if (!ConvertFromJsValue(env, jsTouchType, touchType)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to touchType");
        return false;
    }
    pointerEvent.SetPointerAction(GetMMITouchType(touchType));
    napi_value jsId = nullptr;
    napi_get_named_property(env, touchObject, "id", &jsId);
    int32_t pointerId;
    if (!ConvertFromJsValue(env, jsId, pointerId)) {
        WLOGFE("Failed to convert parameter to id");
        return false;
    }
    if (!ConvertPointerItemFromJs(env, touchObject, pointerEvent)) {
        return false;
    }
    if (!ConvertTouchesObjectFromJs(env, jsTouches, pointerId, pointerEvent)) {
        return false;
    }
    pointerEvent.SetPointerId(pointerId);
    if (!ConvertDeviceIdFromJs(env, jsObject, pointerEvent)) {
        return false;
    }
    return true;
}

bool ConvertDeviceIdFromJs(napi_env env, napi_value jsObject, MMI::PointerEvent& pointerEvent)
{
    napi_value jsDeviceId = nullptr;
    napi_get_named_property(env, jsObject, "deviceId", &jsDeviceId);
    int32_t deviceId = 0;
    if (!ConvertFromJsValue(env, jsDeviceId, deviceId)) {
        WLOGFE("Failed to convert parameter to deviceId");
        return false;
    }
    pointerEvent.SetDeviceId(deviceId);
    return true;
}

bool ConvertInt32ArrayFromJs(napi_env env, napi_value jsObject, std::vector<int32_t>& intList)
{
    bool isArray = false;
    napi_is_array(env, jsObject, &isArray);
    if (jsObject == nullptr || !isArray) {
        WLOGFE("Failed to convert to integer list");
        return false;
    }

    uint32_t length = 0;
    napi_get_array_length(env, jsObject, &length);
    for (uint32_t i = 0; i < length; i++) {
        int32_t persistentId;
        napi_value elementVal = nullptr;
        napi_get_element(env, jsObject, i, &elementVal);
        if (!ConvertFromJsValue(env, elementVal, persistentId)) {
            WLOGFE("Failed to convert to index %{public}u to integer", i);
            return false;
        }
        intList.push_back(persistentId);
    }

    return true;
}

bool ConvertStringMapFromJs(napi_env env, napi_value value, std::unordered_map<std::string, std::string>& stringMap)
{
    if (value == nullptr) {
        WLOGFE("value is nullptr");
        return false;
    }

    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        WLOGFE("The type of value is not napi_object.");
        return false;
    }

    std::vector<std::string> propNames;
    napi_value array = nullptr;
    napi_get_property_names(env, value, &array);
    if (!ParseArrayStringValue(env, array, propNames)) {
        WLOGFE("Failed to property names");
        return false;
    }

    for (const auto &propName : propNames) {
        napi_value prop = nullptr;
        napi_get_named_property(env, value, propName.c_str(), &prop);
        if (prop == nullptr) {
            WLOGFW("prop is null: %{public}s", propName.c_str());
            continue;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            WLOGFW("prop is not string: %{public}s", propName.c_str());
            continue;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            WLOGFW("Failed to ConvertFromJsValue: %{public}s", propName.c_str());
            continue;
        }
        stringMap.emplace(propName, valName);
    }
    return true;
}

bool ConvertJsonFromJs(napi_env env, napi_value value, nlohmann::json& payload)
{
    if (value == nullptr || !CheckTypeForNapiValue(env, value, napi_object)) {
        WLOGFE("The type of value is not napi_object or is nullptr.");
        return false;
    }

    napi_value array = nullptr;
    napi_get_property_names(env, value, &array);
    std::vector<std::string> propNames;
    if (!ParseArrayStringValue(env, array, propNames)) {
        WLOGFE("Failed to property names");
        return false;
    }

    for (const auto& propName : propNames) {
        napi_value prop = nullptr;
        napi_get_named_property(env, value, propName.c_str(), &prop);
        if (prop == nullptr) {
            WLOGFW("prop is null: %{public}s", propName.c_str());
            continue;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            WLOGFW("prop is not string: %{public}s", propName.c_str());
            continue;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            WLOGFW("Failed to ConvertFromJsValue: %{public}s", propName.c_str());
            continue;
        }
        payload[propName] = std::move(valName);
    }
    return true;
}

bool ConvertRotateAnimationConfigFromJs(napi_env env, napi_value value, RotateAnimationConfig& config)
{
    napi_value jsDuration = nullptr;
    napi_get_named_property(env, value, "duration", &jsDuration);
    if (GetType(env, jsDuration) != napi_undefined) {
        int32_t duration = ROTATE_ANIMATION_DURATION;
        if (!ConvertFromJsValue(env, jsDuration, duration)) {
            TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to duration");
            return false;
        }
        config.duration_ = duration;
    }
    return true;
}
bool ConvertRectFromJsValue(napi_env env, napi_value jsObject, Rect& displayRect)
{
    napi_value jsPosX_ = nullptr;
    napi_value jsPosY_ = nullptr;
    napi_value jsWidth_ = nullptr;
    napi_value jsHeight_ = nullptr;
    napi_get_named_property(env, jsObject, "posX_", &jsPosX_);
    napi_get_named_property(env, jsObject, "posY_", &jsPosY_);
    napi_get_named_property(env, jsObject, "width_", &jsWidth_);
    napi_get_named_property(env, jsObject, "height_", &jsHeight_);
    if (GetType(env, jsPosX_) != napi_undefined) {
        int32_t posX;
        if (!ConvertFromJsValue(env, jsPosX_, posX)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to posX_");
            return false;
        }
        displayRect.posX_ = posX;
    }
    if (GetType(env, jsPosY_) != napi_undefined) {
        int32_t posY;
        if (!ConvertFromJsValue(env, jsPosY_, posY)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to posY_");
            return false;
        }
        displayRect.posY_ = posY;
    }
    if (GetType(env, jsWidth_) != napi_undefined) {
        uint32_t width = 0;
        if (!ConvertFromJsValue(env, jsWidth_, width)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to width_");
            return false;
        }
        displayRect.width_ = width;
    }
    if (GetType(env, jsHeight_) != napi_undefined) {
        uint32_t height = 0;
        if (!ConvertFromJsValue(env, jsHeight_, height)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to height_");
            return false;
        }
        displayRect.height_ = height;
    }
    return true;
}

bool ConvertInfoFromJsValue(napi_env env, napi_value jsObject, RotationChangeInfo& rotationChangeInfo)
{
    napi_value jsType = nullptr;
    napi_value jsOrientation = nullptr;
    napi_value jsDisplayId = nullptr;
    napi_value jsDisplayRect = nullptr;
    napi_get_named_property(env, jsObject, "type", &jsType);
    napi_get_named_property(env, jsObject, "orientation", &jsOrientation);
    napi_get_named_property(env, jsObject, "displayId", &jsDisplayId);
    napi_get_named_property(env, jsObject, "displayRect", &jsDisplayRect);
    if (GetType(env, jsType) != napi_undefined) {
        uint32_t type;
        if (!ConvertFromJsValue(env, jsType, type)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to type");
            return false;
        }
        rotationChangeInfo.type_ = static_cast<RotationChangeType>(type);
    }
    if (GetType(env, jsOrientation) != napi_undefined) {
        uint32_t orientation;
        if (!ConvertFromJsValue(env, jsOrientation, orientation)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to orientation");
            return false;
        }
        rotationChangeInfo.orientation_ = orientation;
    }
    if (GetType(env, jsDisplayId) != napi_undefined) {
        int32_t displayId;
        if (!ConvertFromJsValue(env, jsDisplayId, displayId)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to displayId");
            return false;
        }
        rotationChangeInfo.displayId_ = static_cast<uint64_t>(displayId);
    }
    if (GetType(env, jsDisplayRect) != napi_undefined) {
        Rect displayRect;
        if (!ConvertRectFromJsValue(env, jsDisplayRect, displayRect)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to displayRect");
            return false;
        }
        rotationChangeInfo.displayRect_ = displayRect;
    }
    return true;
}


bool ConvertSupportRotationInfoFromJsValue(napi_env env, napi_value jsObject,
    SupportRotationInfo& suppoortRotationInfo)
{
    napi_value jsDisplayId = nullptr;
    napi_value jsPersistentId = nullptr;
    napi_value jsContainerSupportRotation = nullptr;
    napi_value jsSceneSupportRotation = nullptr;
    napi_value jsSupportRotationChangeReason = nullptr;
    napi_get_named_property(env, jsObject, "displayId", &jsDisplayId);
    napi_get_named_property(env, jsObject, "persistentId", &jsPersistentId);
    napi_get_named_property(env, jsObject, "containerSupportRotation", &jsContainerSupportRotation);
    napi_get_named_property(env, jsObject, "sceneSupportRotation", &jsSceneSupportRotation);
    napi_get_named_property(env, jsObject, "supportRotationChangeReason", &jsSupportRotationChangeReason);
    if (GetType(env, jsDisplayId) != napi_undefined) {
        uint32_t displayId;
        if (!ConvertFromJsValue(env, jsDisplayId, displayId)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to displayId");
            return false;
        }
        suppoortRotationInfo.displayId_ = static_cast<uint64_t>(displayId);
    }
    if (GetType(env, jsPersistentId) != napi_undefined) {
        int32_t persistentId;
        if (!ConvertFromJsValue(env, jsPersistentId, persistentId)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to persistentId");
            return false;
        }
        suppoortRotationInfo.persistentId_ = persistentId;
    }
    if (GetType(env, jsContainerSupportRotation) != napi_undefined) {
        std::vector<bool> containerSupportRotation;
        if (!ParseBoolArrayValueFromJsValue(env, jsContainerSupportRotation, containerSupportRotation)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to containerSupportRotation");
            return false;
        }
        suppoortRotationInfo.containerSupportRotation_ = containerSupportRotation;
    }
    if (GetType(env, jsSceneSupportRotation) != napi_undefined) {
        std::vector<bool> sceneSupportRotation;
        if (!ParseBoolArrayValueFromJsValue(env, jsSceneSupportRotation, sceneSupportRotation)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to sceneSupportRotation");
            return false;
        }
        suppoortRotationInfo.sceneSupportRotation_ = sceneSupportRotation;
    }
    if (GetType(env, jsSupportRotationChangeReason) != napi_undefined) {
        std::string supportRotationChangeReason;
        if (!ConvertFromJsValue(env, jsSupportRotationChangeReason, supportRotationChangeReason)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to supportRotationChangeReason");
            return false;
        }
        suppoortRotationInfo.supportRotationChangeReason_ = supportRotationChangeReason;
    }
    return true;
}

bool ConvertDragResizeTypeFromJs(napi_env env, napi_value value, DragResizeType& dragResizeType)
{
    uint32_t dragResizeTypeValue;
    if (!ConvertFromJsValue(env, value, dragResizeTypeValue)) {
        return false;
    }
    if (dragResizeTypeValue >= static_cast<uint32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to dragResizeType");
        return false;
    }
    dragResizeType = static_cast<DragResizeType>(dragResizeTypeValue);
    return true;
}

bool ConvertThrowSlipModeFromJs(napi_env env, napi_value value, ThrowSlipMode& throwSlipMode)
{
    int32_t fingers = 0;
    if (!ConvertFromJsValue(env, value, fingers)) {
        return false;
    }
    auto it = FINGERS_TO_THROWSLIPMODE_MAP.find(fingers);
    if (it == FINGERS_TO_THROWSLIPMODE_MAP.end()) {
        return false;
    }
    throwSlipMode = it->second;
    return true;
}

bool ConvertRealTimeSwitchInfoFromJs(napi_env env, napi_value value, RealTimeSwitchInfo& switchInfo)
{
    napi_value realTimeSwitchInfo = nullptr;
    napi_value jsIsNeedChange = nullptr;
    napi_value jsShowTypes = nullptr;
    napi_get_named_property(env, value, "realTimeSwitchInfo", &realTimeSwitchInfo);
    if (!realTimeSwitchInfo) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to realTimeSwitchInfo");
        return false;
    }
    napi_get_named_property(env, realTimeSwitchInfo, "isNeedChange", &jsIsNeedChange);
    napi_get_named_property(env, realTimeSwitchInfo, "showTypes", &jsShowTypes);
    bool isNeedChange = false;
    if (realTimeSwitchInfo == nullptr || !ConvertFromJsValue(env, jsIsNeedChange, isNeedChange)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to isNeedChange");
        return false;
    }
    switchInfo.isNeedChange_ = isNeedChange;
    uint32_t showTypes = 0;
    if (realTimeSwitchInfo == nullptr || !ConvertFromJsValue(env, jsShowTypes, showTypes)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to showTypes");
        return false;
    }
    switchInfo.showTypes_ = showTypes;
    return true;
}

bool ConvertCompatibleModePropertyFromJs(napi_env env, napi_value value, CompatibleModeProperty& compatibleModeProperty)
{
    std::map<std::string, void (CompatibleModeProperty::*)(bool)> funcs = {
        {"isAdaptToImmersive", &CompatibleModeProperty::SetIsAdaptToImmersive},
        {"isAdaptToEventMapping", &CompatibleModeProperty::SetIsAdaptToProportionalScale},
        {"isAdaptToProportionalScale", &CompatibleModeProperty::SetIsAdaptToProportionalScale},
        {"isAdaptToBackButton", &CompatibleModeProperty::SetIsAdaptToBackButton},
        {"isAdaptToDragScale", &CompatibleModeProperty::SetIsAdaptToDragScale},
        {"disableDragResize", &CompatibleModeProperty::SetDisableDragResize},
        {"disableResizeWithDpi", &CompatibleModeProperty::SetDisableResizeWithDpi},
        {"disableFullScreen", &CompatibleModeProperty::SetDisableFullScreen},
        {"disableSplit", &CompatibleModeProperty::SetDisableSplit},
        {"disableWindowLimit", &CompatibleModeProperty::SetDisableWindowLimit},
        {"disableDecorFullscreen", &CompatibleModeProperty::SetDisableDecorFullscreen},
        {"isFullScreenStart", &CompatibleModeProperty::SetIsFullScreenStart},
        {"isSupportRotateFullScreen", &CompatibleModeProperty::SetIsSupportRotateFullScreen},
        {"isAdaptToSubWindow", &CompatibleModeProperty::SetIsAdaptToSubWindow},
        {"isAdaptToSimulationScale", &CompatibleModeProperty::SetIsAdaptToSimulationScale},
        {"isAdaptToCompatibleDevice", &CompatibleModeProperty::SetIsAdaptToCompatibleDevice},
    };
    bool atLeastOneParam = false;
    std::map<std::string, void (CompatibleModeProperty::*)(bool)>::iterator iter;
    for (iter = funcs.begin(); iter != funcs.end(); ++iter) {
        std::string paramStr = iter->first;
        bool ret = false;
        if (ParseJsValue(env, value, paramStr, ret)) {
            void (CompatibleModeProperty::*func)(bool) = iter->second;
            (compatibleModeProperty.*func)(ret);
            atLeastOneParam = true;
        }
    }
    RealTimeSwitchInfo realTimeSwitchInfo;
    if (ConvertRealTimeSwitchInfoFromJs(env, value, realTimeSwitchInfo)) {
        compatibleModeProperty.SetRealTimeSwitchInfo(realTimeSwitchInfo);
        atLeastOneParam = true;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "property: %{public}s", compatibleModeProperty.ToString().c_str());
    return atLeastOneParam;
}

bool ParseArrayStringValue(napi_env env, napi_value array, std::vector<std::string>& vector)
{
    if (array == nullptr) {
        WLOGFE("array is nullptr!");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || isArray == false) {
        WLOGFE("not array!");
        return false;
    }

    uint32_t arrayLen = 0;
    napi_get_array_length(env, array, &arrayLen);
    if (arrayLen == 0) {
        return true;
    }
    vector.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        std::string strItem;
        napi_value jsValue = nullptr;
        napi_get_element(env, array, i, &jsValue);
        if (!ConvertFromJsValue(env, jsValue, strItem)) {
            WLOGFW("Failed to ConvertFromJsValue, index: %{public}u", i);
            continue;
        }
        vector.emplace_back(std::move(strItem));
    }
    return true;
}

bool ParseBoolArrayValueFromJsValue(napi_env env, napi_value array, std::vector<bool>& vector)
{
    if (array == nullptr) {
        WLOGFE("array is nullptr!");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || isArray == false) {
        WLOGFE("not array!");
        return false;
    }

    uint32_t arrayLen = 0;
    napi_get_array_length(env, array, &arrayLen);
    if (arrayLen == 0) {
        return true;
    }
    vector.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        bool strItem;
        napi_value jsValue = nullptr;
        napi_get_element(env, array, i, &jsValue);
        if (!ConvertFromJsValue(env, jsValue, strItem)) {
            WLOGFW("Failed to ConvertFromJsValue, index: %{public}u", i);
            return false;
        }
        vector.emplace_back(std::move(strItem));
    }
    return true;
}

JsSessionType GetApiType(WindowType type)
{
    auto iter = WINDOW_TO_JS_SESSION_TYPE_MAP.find(type);
    if (iter == WINDOW_TO_JS_SESSION_TYPE_MAP.end()) {
        WLOGFE("Window type: %{public}u cannot map to api type!", type);
        return JsSessionType::TYPE_UNDEFINED;
    } else {
        return iter->second;
    }
}

napi_value CreateSupportWindowModes(napi_env env,
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, supportedWindowModes.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    int32_t index = 0;
    for (const auto supportWindowMode : supportedWindowModes) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, static_cast<int32_t>(supportWindowMode)));
    }
    return arrayValue;
}

napi_value CreateJsSessionPendingConfigs(napi_env env, const PendingSessionActivationConfig &config)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "forceStart", CreateJsValue(env, config.forceStart));
    napi_set_named_property(env, objValue, "forceNewWant", CreateJsValue(env, config.forceNewWant));
    return objValue;
}

napi_value CreateJsSessionInfo(napi_env env, const SessionInfo& sessionInfo,
    const std::shared_ptr<PendingSessionActivationConfig>& config)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, sessionInfo.bundleName_));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, sessionInfo.moduleName_));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, sessionInfo.abilityName_));
    napi_set_named_property(env, objValue, "appIndex", CreateJsValue(env, sessionInfo.appIndex_));
    napi_set_named_property(env, objValue, "isSystem", CreateJsValue(env, sessionInfo.isSystem_));
    napi_set_named_property(env, objValue, "isCastSession", CreateJsValue(env, sessionInfo.isCastSession_));
    napi_set_named_property(env, objValue, "persistentId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.persistentId_)));
    napi_set_named_property(env, objValue, "callerPersistentId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.callerPersistentId_)));
    napi_set_named_property(env, objValue, "callerBundleName", CreateJsValue(env, sessionInfo.callerBundleName_));
    napi_set_named_property(env, objValue, "callerAbilityName", CreateJsValue(env, sessionInfo.callerAbilityName_));
    napi_set_named_property(env, objValue, "callState",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.callState_)));
    napi_set_named_property(env, objValue, "windowMode",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.windowMode)));
    napi_set_named_property(env, objValue, "screenId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.screenId_)));
    napi_set_named_property(env, objValue, "sessionType",
        CreateJsValue(env, static_cast<uint32_t>(GetApiType(static_cast<WindowType>(sessionInfo.windowType_)))));
    napi_set_named_property(env, objValue, "sessionState",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.sessionState_)));
    napi_set_named_property(env, objValue, "startWindowType",
        CreateJsValue(env, sessionInfo.startWindowType_));
    napi_set_named_property(env, objValue, "requestOrientation",
        CreateJsValue(env, sessionInfo.requestOrientation_));
    napi_set_named_property(env, objValue, "isCalledRightlyByCallerId",
        CreateJsValue(env, sessionInfo.isCalledRightlyByCallerId_));
    napi_set_named_property(env, objValue, "isAtomicService",
        CreateJsValue(env, sessionInfo.isAtomicService_));
    napi_set_named_property(env, objValue, "isBackTransition",
        CreateJsValue(env, sessionInfo.isBackTransition_));
    napi_set_named_property(env, objValue, "requestId",
        CreateJsValue(env, sessionInfo.requestId));
    napi_set_named_property(env, objValue, "specifiedReason",
        CreateJsValue(env, sessionInfo.specifiedReason_));
    napi_set_named_property(env, objValue, "needClearInNotShowRecent",
        CreateJsValue(env, sessionInfo.needClearInNotShowRecent_));
    if (sessionInfo.processOptions != nullptr) {
        napi_set_named_property(env, objValue, "processOptions",
            CreateJsProcessOption(env, sessionInfo.processOptions));
    }
    napi_set_named_property(env, objValue, "errorReason",
        CreateJsValue(env, sessionInfo.errorReason));

    SetJsSessionInfoByWant(env, sessionInfo, objValue);
    napi_set_named_property(env, objValue, "supportWindowModes",
        CreateSupportWindowModes(env, sessionInfo.supportedWindowModes));

    if (config != nullptr) {
        napi_set_named_property(env, objValue, "extendPendingActivationConfig",
            CreateJsSessionPendingConfigs(env, *(config)));
    }

    napi_set_named_property(env, objValue, "specifiedFlag", CreateJsValue(env, sessionInfo.specifiedFlag_));
    if (sessionInfo.want != nullptr) {
        napi_set_named_property(env, objValue, "want", AppExecFwk::WrapWant(env, sessionInfo.GetWantSafely()));
    }
    if (sessionInfo.startAnimationOptions != nullptr) {
        napi_status status = napi_set_named_property(env, objValue, "startAnimationOptions",
            ConvertStartAnimationOptionsToJsValue(env, sessionInfo.startAnimationOptions));
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to set startAnimationOptions");
        }
    }
    if (sessionInfo.startAnimationSystemOptions != nullptr) {
        napi_status status = napi_set_named_property(env, objValue, "startAnimationSystemOptions",
            ConvertStartAnimationSystemOptionsToJsValue(env, sessionInfo.startAnimationSystemOptions));
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to set startAnimationSystemOptions");
        }
    }
    napi_set_named_property(env, objValue, "atomicServiceInfo",
        CreateJsAtomicServiceInfo(env, sessionInfo.atomicServiceInfo_));
    return objValue;
}

void ProcessPendingSessionActivationResult(napi_env env, napi_value callResult,
    const std::shared_ptr<SessionInfo>& sessionInfo)
{
    uint32_t resultCode = 0;
    std::string resultMessage = "";
    if (!ConvertSessionResultFromJsValue(env, callResult, resultCode, resultMessage)) {
        return;
    }
    if (resultCode >= static_cast<uint32_t>(RequestResultCode::FAIL)) {
        SceneSessionManager::GetInstance().NotifyAmsPendingSessionWhenFail(resultCode,
            resultMessage, sessionInfo->requestId, sessionInfo->persistentId_);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "persistentId:%{public}d, requestId:%{public}d,"
        "resultCode:%{public}d, resultMessage:%{public}s",
        sessionInfo->persistentId_, sessionInfo->requestId, resultCode, resultMessage.c_str());
}

bool ConvertSessionResultFromJsValue(napi_env env, napi_value jsResult, uint32_t& resultCode,
    std::string& resultMessage)
{
    uint32_t tempResultCode = 0;
    std::string tempResultMessage = "";
    if (!ParseJsValue(env, jsResult, "resultCode", tempResultCode) ||
        !ParseJsValue(env, jsResult, "resultMessage", tempResultMessage)) {
        TLOGE(WmsLogTag::WMS_LIFE, "ConvertSessionResultFromJsValue fail");
        return false;
    }
    resultCode = tempResultCode;
    resultMessage = tempResultMessage;
    return true;
}

napi_value CreateJsExceptionInfo(napi_env env, const ExceptionInfo& exceptionInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to get jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "needRemoveSession",
        CreateJsValue(env, exceptionInfo.needRemoveSession));
    napi_set_named_property(env, objValue, "needClearCallerLink",
        CreateJsValue(env, exceptionInfo.needClearCallerLink));
    return objValue;
}

napi_value CreateJsSessionRecoverInfo(
    napi_env env, const SessionInfo& sessionInfo, const sptr<WindowSessionProperty> property)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, sessionInfo.bundleName_));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, sessionInfo.moduleName_));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, sessionInfo.abilityName_));
    napi_set_named_property(env, objValue, "appIndex", CreateJsValue(env, sessionInfo.appIndex_));
    napi_set_named_property(env, objValue, "screenId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.screenId_)));
    napi_set_named_property(env, objValue, "windowMode",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.windowMode)));
    napi_set_named_property(env, objValue, "sessionState",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.sessionState_)));
    napi_set_named_property(env, objValue, "sessionType",
        CreateJsValue(env, static_cast<uint32_t>(GetApiType(static_cast<WindowType>(sessionInfo.windowType_)))));
    napi_set_named_property(env, objValue, "requestOrientation",
        CreateJsValue(env, sessionInfo.requestOrientation_));
    Rect rect = property->GetWindowRect();
    WSRect wsRect = { rect.posX_, rect.posY_, rect.width_, rect.height_ };
    napi_set_named_property(env, objValue, "recoverRect", CreateJsSessionRect(env, wsRect));
    napi_set_named_property(env, objValue, "layoutFullScreen", CreateJsValue(env, property->IsLayoutFullScreen()));
    napi_set_named_property(env, objValue, "mainWindowTopmost", CreateJsValue(env, property->IsMainWindowTopmost()));
    napi_set_named_property(env, objValue, "isFullScreenWaterfallMode",
        CreateJsValue(env, property->GetIsFullScreenWaterfallMode()));
    napi_set_named_property(env, objValue, "currentRotation", CreateJsValue(env, sessionInfo.currentRotation_));
    napi_set_named_property(env, objValue, "supportWindowModes",
        CreateSupportWindowModes(env, sessionInfo.supportedWindowModes));
    napi_set_named_property(env, objValue,
        "pageCompatibleMode", CreateJsValue(env, property->GetPageCompatibleMode()));

    napi_value jsTransitionAnimationMapValue = nullptr;
    napi_create_object(env, &jsTransitionAnimationMapValue);
    if (jsTransitionAnimationMapValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to get jsObject");
        return nullptr;
    }
    for (const auto& [transitionType, animation] : property->GetTransitionAnimationConfig()) {
        napi_set_property(env, jsTransitionAnimationMapValue, CreateJsValue(env, static_cast<uint32_t>(transitionType)),
            ConvertTransitionAnimationToJsValue(env, animation));
    }
    napi_set_named_property(env, objValue, "transitionAnimationMap", jsTransitionAnimationMapValue);
    napi_value jsPropertyArrayObject = CreateJsSystemBarPropertyArrayObject(env, property->GetSystemBarProperty());
    if (jsPropertyArrayObject == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "jsPropertyArrayObject is nullptr");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "systemBarProperties", jsPropertyArrayObject);

    return objValue;
}

static void SetWindowSizeLimits(napi_env env, const SessionInfo& sessionInfo, napi_value objValue)
{
    if (sessionInfo.windowSizeLimits.maxWindowWidth > 0) {
        napi_set_named_property(env, objValue, "maxWindowWidth",
            CreateJsValue(env, sessionInfo.windowSizeLimits.maxWindowWidth));
    }
    if (sessionInfo.windowSizeLimits.minWindowWidth > 0) {
        napi_set_named_property(env, objValue, "minWindowWidth",
            CreateJsValue(env, sessionInfo.windowSizeLimits.minWindowWidth));
    }
    if (sessionInfo.windowSizeLimits.maxWindowHeight > 0) {
        napi_set_named_property(env, objValue, "maxWindowHeight",
            CreateJsValue(env, sessionInfo.windowSizeLimits.maxWindowHeight));
    }
    if (sessionInfo.windowSizeLimits.minWindowHeight > 0) {
        napi_set_named_property(env, objValue, "minWindowHeight",
            CreateJsValue(env, sessionInfo.windowSizeLimits.minWindowHeight));
    }
}

void SetJsSessionInfoByWant(napi_env env, const SessionInfo& sessionInfo, napi_value objValue)
{
    if (sessionInfo.want != nullptr) {
        SetWindowSizeLimits(env, sessionInfo, objValue);
        napi_set_named_property(env, objValue, "windowTop",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_TOP, INVALID_VAL)));
        napi_set_named_property(env, objValue, "windowLeft",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_LEFT, INVALID_VAL)));
        napi_set_named_property(env, objValue, "windowWidth",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_WIDTH, INVALID_VAL)));
        napi_set_named_property(env, objValue, "windowHeight",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_HEIGHT, INVALID_VAL)));
        napi_set_named_property(env, objValue, "withAnimation",
            CreateJsValue(env, sessionInfo.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WITH_ANIMATION, true)));
        napi_set_named_property(env, objValue, "focusedOnShow",
            CreateJsValue(env, sessionInfo.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WINDOW_FOCUSED, true)));
        napi_set_named_property(env, objValue, "isStartupInstallFree",
            CreateJsValue(env, (sessionInfo.want->GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) ==
                AAFwk::Want::FLAG_INSTALL_ON_DEMAND));
        auto params = sessionInfo.want->GetParams();
        napi_set_named_property(env, objValue, "fileManagerMode",
            CreateJsValue(env, params.GetStringParam("fileManagerMode")));
        napi_set_named_property(env, objValue, "floatingDisplayMode",
            CreateJsValue(env, params.GetIntParam("floatingDisplayMode", INVALID_VAL)));
        auto executeParams = params.GetWantParams("ohos.insightIntent.executeParam.param");
        napi_set_named_property(env, objValue, "extraFormIdentity",
            CreateJsValue(env, executeParams.GetStringParam("ohos.extra.param.key.form_identity")));
        if (params.HasParam("expectWindowMode")) {
            napi_set_named_property(env, objValue, "expectWindowMode",
                CreateJsValue(env, params.GetIntParam("expectWindowMode", INVALID_VAL)));
        }
        if (params.HasParam("isStartFromAppDock")) {
            napi_set_named_property(env, objValue, "isStartFromAppDock",
                CreateJsValue(env, params.GetIntParam("isStartFromAppDock", INVALID_VAL)));
        }
        if (params.HasParam("dockAppDirection")) {
            napi_set_named_property(env, objValue, "dockAppDirection",
                CreateJsValue(env, params.GetIntParam("dockAppDirection", INVALID_VAL)));
        }
        if (params.HasParam("isAppFromRecentAppsOrDockApps")) {
            napi_set_named_property(env, objValue, "isAppFromRecentAppsOrDockApps",
                CreateJsValue(env, params.GetIntParam("isAppFromRecentAppsOrDockApps", INVALID_VAL)));
        }
    }
}

napi_value GetWindowRectIntValue(napi_env env, int val)
{
    if (val != INVALID_VAL) {
        return  CreateJsValue(env, val);
    } else {
        return NapiGetUndefined(env);
    }
}

napi_value CreateJsSessionState(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "STATE_DISCONNECT", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_DISCONNECT)));
    napi_set_named_property(env, objValue, "STATE_CONNECT", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_CONNECT)));
    napi_set_named_property(env, objValue, "STATE_FOREGROUND", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_FOREGROUND)));
    napi_set_named_property(env, objValue, "STATE_ACTIVE", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_ACTIVE)));
    napi_set_named_property(env, objValue, "STATE_INACTIVE", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_INACTIVE)));
    napi_set_named_property(env, objValue, "STATE_BACKGROUND", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_BACKGROUND)));
    napi_set_named_property(env, objValue, "STATE_END", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_END)));

    return objValue;
}

napi_value CreateJsSessionSizeChangeReason(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::UNDEFINED)));
    napi_set_named_property(env, objValue, "MAXIMIZE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MAXIMIZE)));
    napi_set_named_property(env, objValue, "RECOVER", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RECOVER)));
    napi_set_named_property(env, objValue, "ROTATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::ROTATION)));
    napi_set_named_property(env, objValue, "DRAG", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG)));
    napi_set_named_property(env, objValue, "DRAG_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG_START)));
    napi_set_named_property(env, objValue, "DRAG_MOVE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG_MOVE)));
    napi_set_named_property(env, objValue, "DRAG_END", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG_END)));
    napi_set_named_property(env, objValue, "RESIZE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RESIZE)));
    napi_set_named_property(env, objValue, "RESIZE_WITH_ANIMATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RESIZE_WITH_ANIMATION)));
    napi_set_named_property(env, objValue, "MOVE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MOVE)));
    napi_set_named_property(env, objValue, "MOVE_WITH_ANIMATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MOVE_WITH_ANIMATION)));
    napi_set_named_property(env, objValue, "HIDE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::HIDE)));
    napi_set_named_property(env, objValue, "TRANSFORM", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::TRANSFORM)));
    napi_set_named_property(env, objValue, "CUSTOM_ANIMATION_SHOW", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::CUSTOM_ANIMATION_SHOW)));
    napi_set_named_property(env, objValue, "FULL_TO_SPLIT", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::FULL_TO_SPLIT)));
    napi_set_named_property(env, objValue, "SPLIT_TO_FULL", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SPLIT_TO_FULL)));
    napi_set_named_property(env, objValue, "FULL_TO_FLOATING", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::FULL_TO_FLOATING)));
    napi_set_named_property(env, objValue, "FLOATING_TO_FULL", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::FLOATING_TO_FULL)));
    CreatePiPSizeChangeReason(env, objValue);
    napi_set_named_property(env, objValue, "MAXIMIZE_TO_SPLIT", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MAXIMIZE_TO_SPLIT)));
    napi_set_named_property(env, objValue, "SPLIT_TO_MAXIMIZE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SPLIT_TO_MAXIMIZE)));
    napi_set_named_property(env, objValue, "PAGE_ROTATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PAGE_ROTATION)));
    napi_set_named_property(env, objValue, "SPLIT_DRAG_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SPLIT_DRAG_START)));
    napi_set_named_property(env, objValue, "SPLIT_DRAG", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SPLIT_DRAG)));
    napi_set_named_property(env, objValue, "SPLIT_DRAG_END", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SPLIT_DRAG_END)));
    napi_set_named_property(env, objValue, "RESIZE_BY_LIMIT", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RESIZE_BY_LIMIT)));
    napi_set_named_property(env, objValue, "MAXIMIZE_IN_IMPLICT", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MAXIMIZE_IN_IMPLICT)));
    napi_set_named_property(env, objValue, "RECOVER_IN_IMPLICIT", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RECOVER_IN_IMPLICIT)));
    napi_set_named_property(env, objValue, "SNAPSHOT_ROTATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SNAPSHOT_ROTATION)));
    napi_set_named_property(env, objValue, "SCENE_WITH_ANIMATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SCENE_WITH_ANIMATION)));
    napi_set_named_property(env, objValue, "LS_STATE_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::LS_STATE_CHANGE)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::END)));

    return objValue;
}

napi_value CreateJsRSUIFirstSwitch(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "NONE", CreateJsValue(env,
        static_cast<int32_t>(RSUIFirstSwitch::NONE)));
    napi_set_named_property(env, objValue, "MODAL_WINDOW_CLOSE", CreateJsValue(env,
        static_cast<int32_t>(RSUIFirstSwitch::MODAL_WINDOW_CLOSE)));
    napi_set_named_property(env, objValue, "FORCE_DISABLE", CreateJsValue(env,
        static_cast<int32_t>(RSUIFirstSwitch::FORCE_DISABLE)));
    napi_set_named_property(env, objValue, "FORCE_ENABLE", CreateJsValue(env,
        static_cast<int32_t>(RSUIFirstSwitch::FORCE_ENABLE)));
    napi_set_named_property(env, objValue, "FORCE_ENABLE_LIMIT", CreateJsValue(env,
        static_cast<int32_t>(RSUIFirstSwitch::FORCE_ENABLE_LIMIT)));
    napi_set_named_property(env, objValue, "FORCE_DISABLE_NONFOCUS", CreateJsValue(env,
        static_cast<int32_t>(RSUIFirstSwitch::FORCE_DISABLE_NONFOCUS)));
    return objValue;
}

void CreatePiPSizeChangeReason(napi_env env, napi_value objValue)
{
    napi_set_named_property(env, objValue, "PIP_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_START)));
    napi_set_named_property(env, objValue, "PIP_SHOW", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_SHOW)));
    napi_set_named_property(env, objValue, "PIP_AUTO_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_AUTO_START)));
    napi_set_named_property(env, objValue, "PIP_RATIO_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_RATIO_CHANGE)));
    napi_set_named_property(env, objValue, "PIP_RESTORE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_RESTORE)));
}

napi_value CreateJsSessionStartupVisibility(napi_env env)
{
    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "STARTUP_HIDE", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::STARTUP_HIDE)));
    napi_set_named_property(env, objValue, "STARTUP_SHOW", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::STARTUP_SHOW)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::END)));
    return objValue;
}

napi_value CreateJsWindowVisibility(napi_env env)
{
    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "NO_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION)));
    napi_set_named_property(env, objValue, "PARTIAL_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION)));
    napi_set_named_property(env, objValue, "COMPLETE_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_LAYER_STATE_MAX)));
    return objValue;
}

napi_value CreateJsSessionProcessMode(napi_env env)
{
    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "NEW_PROCESS_ATTACH_TO_PARENT", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT)));
    napi_set_named_property(env, objValue, "NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM)));
    napi_set_named_property(env, objValue, "NEW_HIDDEN_PROCESS", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::NEW_HIDDEN_PROCESS)));
    napi_set_named_property(env, objValue, "NO_ATTACHMENT", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::NO_ATTACHMENT)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::END)));
    return objValue;
}

napi_value CreateJsSessionPiPControlType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "VIDEO_PLAY_PAUSE", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::VIDEO_PLAY_PAUSE)));
    napi_set_named_property(env, objValue, "VIDEO_PREVIOUS", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::VIDEO_PREVIOUS)));
    napi_set_named_property(env, objValue, "VIDEO_NEXT", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::VIDEO_NEXT)));
    napi_set_named_property(env, objValue, "FAST_FORWARD", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::FAST_FORWARD)));
    napi_set_named_property(env, objValue, "FAST_BACKWARD", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::FAST_BACKWARD)));
    napi_set_named_property(env, objValue, "HANG_UP_BUTTON", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::HANG_UP_BUTTON)));
    napi_set_named_property(env, objValue, "MICROPHONE_SWITCH", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::MICROPHONE_SWITCH)));
    napi_set_named_property(env, objValue, "CAMERA_SWITCH", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::CAMERA_SWITCH)));
    napi_set_named_property(env, objValue, "MUTE_SWITCH", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::MUTE_SWITCH)));
    return objValue;
}

napi_value CreateJsSessionPiPControlStatus(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "PLAY", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::PLAY)));
    napi_set_named_property(env, objValue, "PAUSE", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::PAUSE)));
    napi_set_named_property(env, objValue, "OPEN", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::OPEN)));
    napi_set_named_property(env, objValue, "CLOSE", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::CLOSE)));
    napi_set_named_property(env, objValue, "ENABLED", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::ENABLED)));
    napi_set_named_property(env, objValue, "DISABLED", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::DISABLED)));
    return objValue;
}

napi_value CreateJsSessionGravity(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }
    using T = std::underlying_type_t<Gravity>;
    napi_set_named_property(env, objValue, "CENTER", CreateJsValue(env,
        static_cast<T>(Gravity::CENTER)));
    napi_set_named_property(env, objValue, "TOP", CreateJsValue(env,
        static_cast<T>(Gravity::TOP)));
    napi_set_named_property(env, objValue, "BOTTOM", CreateJsValue(env,
        static_cast<T>(Gravity::BOTTOM)));
    napi_set_named_property(env, objValue, "LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::LEFT)));
    napi_set_named_property(env, objValue, "RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::RIGHT)));
    napi_set_named_property(env, objValue, "TOP_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::TOP_LEFT)));
    napi_set_named_property(env, objValue, "TOP_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::TOP_RIGHT)));
    napi_set_named_property(env, objValue, "BOTTOM_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::BOTTOM_LEFT)));
    napi_set_named_property(env, objValue, "BOTTOM_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::BOTTOM_RIGHT)));
    napi_set_named_property(env, objValue, "RESIZE", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_TOP_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_TOP_LEFT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_BOTTOM_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_BOTTOM_RIGHT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_FILL", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_FILL)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_FILL_TOP_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_FILL_TOP_LEFT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_FILL_BOTTOM_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_FILL_BOTTOM_RIGHT)));
    return objValue;
}

napi_value CreateJsSessionDragResizeType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "RESIZE_TYPE_UNDEFINED", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_TYPE_UNDEFINED)));
    napi_set_named_property(env, objValue, "RESIZE_EACH_FRAME", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_EACH_FRAME)));
    napi_set_named_property(env, objValue, "RESIZE_WHEN_DRAG_END", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_WHEN_DRAG_END)));
    napi_set_named_property(env, objValue, "RESIZE_SCALE", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_SCALE)));
    return objValue;
}

template<typename T>
napi_value CreateJsSessionRect(napi_env env, const T& rect)
{
    WLOGFD("CreateJsSessionRect.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "posX_", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "posY_", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width_", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height_", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value CreateJsRectAnimationConfig(napi_env env, const RectAnimationConfig& rectAnimationConfig)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "duration", CreateJsValue(env, rectAnimationConfig.duration));
    napi_set_named_property(env, objValue, "x1", CreateJsValue(env, rectAnimationConfig.x1));
    napi_set_named_property(env, objValue, "y1", CreateJsValue(env, rectAnimationConfig.y1));
    napi_set_named_property(env, objValue, "x2", CreateJsValue(env, rectAnimationConfig.x2));
    napi_set_named_property(env, objValue, "y2", CreateJsValue(env, rectAnimationConfig.y2));
    return objValue;
}

napi_value CreateJsSessionEventParam(napi_env env, const SessionEventParam& param)
{
    WLOGFD("CreateJsSessionEventParam.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "pointerX", CreateJsValue(env, param.pointerX_));
    napi_set_named_property(env, objValue, "pointerY", CreateJsValue(env, param.pointerY_));
    napi_set_named_property(env, objValue, "sessionWidth", CreateJsValue(env, param.sessionWidth_));
    napi_set_named_property(env, objValue, "sessionHeight", CreateJsValue(env, param.sessionHeight_));
    napi_set_named_property(env, objValue, "dragResizeType", CreateJsValue(env, param.dragResizeType));
    napi_set_named_property(env, objValue, "gravity", CreateJsValue(env, param.gravity));
    napi_set_named_property(env, objValue, "waterfallResidentState", CreateJsValue(env, param.waterfallResidentState));
    napi_set_named_property(env, objValue, "compatibleStyleMode", CreateJsValue(env, param.compatibleStyleMode));
    return objValue;
}

napi_value SubWindowModalTypeInit(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "TYPE_UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_UNDEFINED)));
    napi_set_named_property(env, objValue, "TYPE_NORMAL", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_NORMAL)));
    napi_set_named_property(env, objValue, "TYPE_DIALOG", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_DIALOG)));
    napi_set_named_property(env, objValue, "TYPE_WINDOW_MODALITY", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_WINDOW_MODALITY)));
    napi_set_named_property(env, objValue, "TYPE_TOAST", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_TOAST)));
    napi_set_named_property(env, objValue, "TYPE_TEXT_MENU", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_TEXT_MENU)));
    napi_set_named_property(env, objValue, "TYPE_APPLICATION_MODALITY", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_APPLICATION_MODALITY)));
    return objValue;
}

static std::string GetHexColor(uint32_t color)
{
    const int32_t rgbaLength = 8;

    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = rgbaLength - static_cast<int>(temp.length());
    std::string finalColor("#");
    std::string tmpColor(count, '0');
    tmpColor += temp;
    finalColor += tmpColor;

    return finalColor;
}

static napi_value CreateJsSystemBarPropertyObject(
    napi_env env, const WindowType type, const SystemBarProperty& property)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }
    if (WINDOW_TO_JS_SESSION_TYPE_MAP.count(type) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, WINDOW_TO_JS_SESSION_TYPE_MAP.at(type)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, type));
    }
    napi_set_named_property(env, objValue, "enable", CreateJsValue(env, property.enable_));
    std::string bkgColor = GetHexColor(property.backgroundColor_);
    napi_set_named_property(env, objValue, "backgroundcolor", CreateJsValue(env, bkgColor));
    std::string contentColor = GetHexColor(property.contentColor_);
    napi_set_named_property(env, objValue, "contentcolor", CreateJsValue(env, contentColor));
    napi_set_named_property(env, objValue, "enableAnimation", CreateJsValue(env, property.enableAnimation_));
    napi_set_named_property(
        env, objValue, "settingFlag", CreateJsValue(env, static_cast<uint32_t>(property.settingFlag_)));

    return objValue;
}

napi_value CreateJsShadowsInfo(napi_env env, const ShadowsInfo& shadowsInfo)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "CreateJsShadowsInfo.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_status ret = napi_ok;
    if (shadowsInfo.hasRadiusValue_) {
        ret = napi_set_named_property(env, objValue, "radius", CreateJsValue(env, shadowsInfo.radius_));
    }
    if (ret != napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    if (shadowsInfo.hasColorValue_) {
        ret = napi_set_named_property(env, objValue, "color", CreateJsValue(env, shadowsInfo.color_));
    }
    if (ret != napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    if (shadowsInfo.hasOffsetXValue_) {
        ret = napi_set_named_property(env, objValue, "offsetX", CreateJsValue(env, shadowsInfo.offsetX_));
    }
    if (ret != napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    if (shadowsInfo.hasOffsetYValue_) {
        ret = napi_set_named_property(env, objValue, "offsetY", CreateJsValue(env, shadowsInfo.offsetY_));
    }
    if (ret != napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    if (!shadowsInfo.hasRadiusValue_ && !shadowsInfo.hasColorValue_ && !shadowsInfo.hasOffsetXValue_ &&
        !shadowsInfo.hasOffsetYValue_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Obeject is undefined!");
        return NapiGetUndefined(env);
    }
    return objValue;
}

napi_value CreateJsKeyboardLayoutParams(napi_env env, const KeyboardLayoutParams& params)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "gravity",
        CreateJsValue(env, params.gravity_));
    napi_set_named_property(env, objValue, "landscapeAvoidHeight",
        CreateJsValue(env, params.landscapeAvoidHeight_));
    napi_set_named_property(env, objValue, "portraitAvoidHeight",
        CreateJsValue(env, params.portraitAvoidHeight_));
    napi_set_named_property(env, objValue, "landscapeKeyboardRect",
        CreateJsSessionRect(env, params.LandscapeKeyboardRect_));
    napi_set_named_property(env, objValue, "portraitKeyboardRect",
        CreateJsSessionRect(env, params.PortraitKeyboardRect_));
    napi_set_named_property(env, objValue, "landscapePanelRect",
        CreateJsSessionRect(env, params.LandscapePanelRect_));
    napi_set_named_property(env, objValue, "portraitPanelRect",
        CreateJsSessionRect(env, params.PortraitPanelRect_));
    napi_set_named_property(env, objValue, "displayId",
        CreateJsValue(env, static_cast<int64_t>(params.displayId_)));
    return objValue;
}

napi_value CreateJsSystemBarPropertyArrayObject(
    napi_env env, const std::unordered_map<WindowType, SystemBarProperty>& propertyMap)
{
    if (propertyMap.empty()) {
        WLOGFE("Empty propertyMap");
        return nullptr;
    }
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, propertyMap.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("Failed to get jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (auto iter: propertyMap) {
        napi_set_element(env, arrayValue, index++, CreateJsSystemBarPropertyObject(env, iter.first, iter.second));
    }
    return arrayValue;
}

napi_value ConvertResultToJsValue(napi_env env, RotationChangeResult& rotationChangeResult)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "rectType",
        CreateJsValue(env, static_cast<uint32_t>(rotationChangeResult.rectType_)));
    napi_set_named_property(env, objValue, "windowRect",
        CreateJsSessionRect(env, rotationChangeResult.windowRect_));
    return objValue;
}

napi_value CreateJsRotationChangeResultMapObject(napi_env env, const int32_t persistentId,
    const RotationChangeResult& rotationChangeResult)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "persistentId", CreateJsValue(env, persistentId));
    napi_set_named_property(env, objValue, "rectType",
        CreateJsValue(env, static_cast<uint32_t>(rotationChangeResult.rectType_)));
    napi_set_named_property(env, objValue, "windowRect",
        CreateJsSessionRect(env, rotationChangeResult.windowRect_));
    return objValue;
}

napi_value CreateResultMapToJsValue(napi_env env,
    const std::unordered_map<int32_t, RotationChangeResult>& rotationChangeResultMap)
{
    if (rotationChangeResultMap.empty()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "rotationChangeResultMap is empty");
        return nullptr;
    }
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, rotationChangeResultMap.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    uint32_t index = 0;
    for (auto iter : rotationChangeResultMap) {
        TLOGD(WmsLogTag::WMS_ROTATION, "CreateResultMapToJsValue index %{public}d", index);
        napi_set_element(env, arrayValue, index++, CreateJsRotationChangeResultMapObject(env, iter.first, iter.second));
    }
    return arrayValue;
}

static void SetTypeProperty(napi_value object, napi_env env, const std::string& name, JsSessionType type)
{
    napi_set_named_property(env, object, name.c_str(), CreateJsValue(env, static_cast<int32_t>(type)));
}

napi_value KeyboardGravityInit(napi_env env)
{
    TLOGD(WmsLogTag::DEFAULT, "KeyboardGravityInit");

    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "GRAVITY_FLOAT", CreateJsValue(env,
        static_cast<int32_t>(SessionGravity::SESSION_GRAVITY_FLOAT)));
    napi_set_named_property(env, objValue, "GRAVITY_BOTTOM", CreateJsValue(env,
        static_cast<int32_t>(SessionGravity::SESSION_GRAVITY_BOTTOM)));
    napi_set_named_property(env, objValue, "GRAVITY_DEFAULT", CreateJsValue(env,
        static_cast<int32_t>(SessionGravity::SESSION_GRAVITY_DEFAULT)));
    return objValue;
}

napi_value KeyboardViewModeInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "In");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "NON_IMMERSIVE_MODE", CreateJsValue(env,
        static_cast<int32_t>(KeyboardViewMode::NON_IMMERSIVE_MODE)));
    napi_set_named_property(env, objValue, "IMMERSIVE_MODE", CreateJsValue(env,
        static_cast<int32_t>(KeyboardViewMode::IMMERSIVE_MODE)));
    napi_set_named_property(env, objValue, "LIGHT_IMMERSIVE_MODE", CreateJsValue(env,
        static_cast<int32_t>(KeyboardViewMode::LIGHT_IMMERSIVE_MODE)));
    napi_set_named_property(env, objValue, "DARK_IMMERSIVE_MODE", CreateJsValue(env,
        static_cast<int32_t>(KeyboardViewMode::DARK_IMMERSIVE_MODE)));
    napi_set_named_property(env, objValue, "VIEW_MODE_END", CreateJsValue(env,
        static_cast<int32_t>(KeyboardViewMode::VIEW_MODE_END)));
    return objValue;
}

napi_value KeyboardFlowLightModeInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "In");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get object");
        return nullptr;
    }
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "NONE", CreateJsValue(env, \
        static_cast<uint32_t>(KeyboardFlowLightMode::NONE))), "NONE set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "BACKGROUND_FLOW_LIGHT", CreateJsValue(env, \
        static_cast<uint32_t>(KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT))), "BACKGROUND_FLOW_LIGHT set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "END", CreateJsValue(env, \
        static_cast<uint32_t>(KeyboardFlowLightMode::END))), "END set failed");
    return objValue;
}

napi_value KeyboardGradientModeInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "In");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get object");
        return nullptr;
    }
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "NONE", CreateJsValue(env, \
        static_cast<uint32_t>(KeyboardGradientMode::NONE))), "NONE set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "LINEAR_GRADIENT", CreateJsValue(env, \
        static_cast<uint32_t>(KeyboardGradientMode::LINEAR_GRADIENT))), "LINEAR_GRADIENT set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "END", CreateJsValue(env, \
        static_cast<uint32_t>(KeyboardGradientMode::END))), "END set failed");
    return objValue;
}

napi_value ConvertKeyboardEffectOptionToJsValue(napi_env env, const KeyboardEffectOption& effectOption)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "viewMode", CreateJsValue(env, \
        static_cast<uint32_t>(effectOption.viewMode_))), "viewMode set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "flowLightMode", CreateJsValue(env, \
        static_cast<uint32_t>(effectOption.flowLightMode_))), "flowLightMode set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "gradientMode", CreateJsValue(env, \
        static_cast<uint32_t>(effectOption.gradientMode_))), "gradientMode set failed");
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "blurHeight", CreateJsValue(env, \
        effectOption.blurHeight_)), "blurHeight set failed");
    return objValue;
}

napi_value SessionTypeInit(napi_env env)
{
    WLOGFD("in");

    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    SetTypeProperty(objValue, env, "TYPE_UNDEFINED", JsSessionType::TYPE_UNDEFINED);
    SetTypeProperty(objValue, env, "TYPE_APP", JsSessionType::TYPE_APP);
    SetTypeProperty(objValue, env, "TYPE_SUB_APP", JsSessionType::TYPE_SUB_APP);
    SetTypeProperty(objValue, env, "TYPE_SYSTEM_ALERT", JsSessionType::TYPE_SYSTEM_ALERT);
    SetTypeProperty(objValue, env, "TYPE_INPUT_METHOD", JsSessionType::TYPE_INPUT_METHOD);
    SetTypeProperty(objValue, env, "TYPE_STATUS_BAR", JsSessionType::TYPE_STATUS_BAR);
    SetTypeProperty(objValue, env, "TYPE_PANEL", JsSessionType::TYPE_PANEL);
    SetTypeProperty(objValue, env, "TYPE_KEYGUARD", JsSessionType::TYPE_KEYGUARD);
    SetTypeProperty(objValue, env, "TYPE_VOLUME_OVERLAY", JsSessionType::TYPE_VOLUME_OVERLAY);
    SetTypeProperty(objValue, env, "TYPE_NAVIGATION_BAR", JsSessionType::TYPE_NAVIGATION_BAR);
    SetTypeProperty(objValue, env, "TYPE_FLOAT", JsSessionType::TYPE_FLOAT);
    SetTypeProperty(objValue, env, "TYPE_WALLPAPER", JsSessionType::TYPE_WALLPAPER);
    SetTypeProperty(objValue, env, "TYPE_DESKTOP", JsSessionType::TYPE_DESKTOP);
    SetTypeProperty(objValue, env, "TYPE_LAUNCHER_DOCK", JsSessionType::TYPE_LAUNCHER_DOCK);
    SetTypeProperty(objValue, env, "TYPE_FLOAT_CAMERA", JsSessionType::TYPE_FLOAT_CAMERA);
    SetTypeProperty(objValue, env, "TYPE_DIALOG", JsSessionType::TYPE_DIALOG);
    SetTypeProperty(objValue, env, "TYPE_SCREENSHOT", JsSessionType::TYPE_SCREENSHOT);
    SetTypeProperty(objValue, env, "TYPE_TOAST", JsSessionType::TYPE_TOAST);
    SetTypeProperty(objValue, env, "TYPE_POINTER", JsSessionType::TYPE_POINTER);
    SetTypeProperty(objValue, env, "TYPE_LAUNCHER_RECENT", JsSessionType::TYPE_LAUNCHER_RECENT);
    SetTypeProperty(objValue, env, "TYPE_SCENE_BOARD", JsSessionType::TYPE_SCENE_BOARD);
    SetTypeProperty(objValue, env, "TYPE_DRAGGING_EFFECT", JsSessionType::TYPE_DRAGGING_EFFECT);
    SetTypeProperty(objValue, env, "TYPE_INPUT_METHOD_STATUS_BAR", JsSessionType::TYPE_INPUT_METHOD_STATUS_BAR);
    SetTypeProperty(objValue, env, "TYPE_GLOBAL_SEARCH", JsSessionType::TYPE_GLOBAL_SEARCH);
    SetTypeProperty(objValue, env, "TYPE_NEGATIVE_SCREEN", JsSessionType::TYPE_NEGATIVE_SCREEN);
    SetTypeProperty(objValue, env, "TYPE_VOICE_INTERACTION", JsSessionType::TYPE_VOICE_INTERACTION);
    SetTypeProperty(objValue, env, "TYPE_SYSTEM_TOAST", JsSessionType::TYPE_SYSTEM_TOAST);
    SetTypeProperty(objValue, env, "TYPE_SYSTEM_FLOAT", JsSessionType::TYPE_SYSTEM_FLOAT);
    SetTypeProperty(objValue, env, "TYPE_THEME_EDITOR", JsSessionType::TYPE_THEME_EDITOR);
    SetTypeProperty(objValue, env, "TYPE_PIP", JsSessionType::TYPE_PIP);
    SetTypeProperty(objValue, env, "TYPE_NAVIGATION_INDICATOR", JsSessionType::TYPE_NAVIGATION_INDICATOR);
    SetTypeProperty(objValue, env, "TYPE_HANDWRITE", JsSessionType::TYPE_HANDWRITE);
    SetTypeProperty(objValue, env, "TYPE_KEYBOARD_PANEL", JsSessionType::TYPE_KEYBOARD_PANEL);
    SetTypeProperty(objValue, env, "TYPE_DIVIDER", JsSessionType::TYPE_DIVIDER);
    SetTypeProperty(objValue, env, "TYPE_TRANSPARENT_VIEW", JsSessionType::TYPE_TRANSPARENT_VIEW);
    SetTypeProperty(objValue, env, "TYPE_WALLET_SWIPE_CARD", JsSessionType::TYPE_WALLET_SWIPE_CARD);
    SetTypeProperty(objValue, env, "TYPE_SCREEN_CONTROL", JsSessionType::TYPE_SCREEN_CONTROL);
    SetTypeProperty(objValue, env, "TYPE_FLOAT_NAVIGATION", JsSessionType::TYPE_FLOAT_NAVIGATION);
    SetTypeProperty(objValue, env, "TYPE_MUTISCREEN_COLLABORATION", JsSessionType::TYPE_MUTISCREEN_COLLABORATION);
    SetTypeProperty(objValue, env, "TYPE_DYNAMIC", JsSessionType::TYPE_DYNAMIC);
    SetTypeProperty(objValue, env, "TYPE_MAGNIFICATION", JsSessionType::TYPE_MAGNIFICATION);
    SetTypeProperty(objValue, env, "TYPE_MAGNIFICATION_MENU", JsSessionType::TYPE_MAGNIFICATION_MENU);
    SetTypeProperty(objValue, env, "TYPE_SELECTION", JsSessionType::TYPE_SELECTION);
    SetTypeProperty(objValue, env, "TYPE_FLOATING_BALL", JsSessionType::TYPE_FLOATING_BALL);
    return objValue;
}

napi_value SceneTypeInit(napi_env env)
{
    WLOGFD("in");

    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "DEFAULT",
        CreateJsValue(env, static_cast<int32_t>(SceneType::DEFAULT)));
    napi_set_named_property(env, objValue, "WINDOW_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::WINDOW_SCENE)));
    napi_set_named_property(env, objValue, "SYSTEM_WINDOW_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::SYSTEM_WINDOW_SCENE)));
    napi_set_named_property(env, objValue, "TRANSFORM_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::TRANSFORM_SCENE)));
    napi_set_named_property(env, objValue, "PANEL_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::PANEL_SCENE)));
    napi_set_named_property(env, objValue, "INPUT_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::INPUT_SCENE)));
    return objValue;
}

napi_value CreateRotationChangeType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "WINDOW_WILL_ROTATE",
        CreateJsValue(env, static_cast<uint32_t>(RotationChangeType::WINDOW_WILL_ROTATE)));
    napi_set_named_property(env, objValue, "WINDOW_DID_ROTATE",
        CreateJsValue(env, static_cast<uint32_t>(RotationChangeType::WINDOW_DID_ROTATE)));
    return objValue;
}

napi_value CreateRectType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "RELATIVE_TO_SCREEN",
        CreateJsValue(env, static_cast<uint32_t>(RectType::RELATIVE_TO_SCREEN)));
    napi_set_named_property(env, objValue, "RELATIVE_TO_PARENT_WINDOW",
        CreateJsValue(env, static_cast<uint32_t>(RectType::RELATIVE_TO_PARENT_WINDOW)));
    return objValue;
}

napi_value CreateSupportType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION",
        CreateJsValue(env, static_cast<uint32_t>(SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION)));
    napi_set_named_property(env, objValue, "ALLOW_KEYBOARD_DID_ANIMATION_NOTIFICATION",
        CreateJsValue(env, static_cast<uint32_t>(SupportFunctionType::ALLOW_KEYBOARD_DID_ANIMATION_NOTIFICATION)));
    return objValue;
}

napi_value CreateJsWindowAnchorInfo(napi_env env, const WindowAnchorInfo& windowAnchorInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "isAnchorEnabled",
        CreateJsValue(env, windowAnchorInfo.isAnchorEnabled_));
    napi_set_named_property(env, objValue, "windowAnchor",
        CreateJsValue(env, static_cast<uint32_t>(windowAnchorInfo.windowAnchor_)));
    napi_set_named_property(env, objValue, "offsetX",
        CreateJsValue(env, windowAnchorInfo.offsetX_));
    napi_set_named_property(env, objValue, "offsetY",
        CreateJsValue(env, windowAnchorInfo.offsetY_));
    return objValue;
}

napi_value CreateWindowAnchorType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "TOP_START",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::TOP_START)));
    napi_set_named_property(env, objValue, "TOP",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::TOP)));
    napi_set_named_property(env, objValue, "TOP_END",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::TOP_END)));
    napi_set_named_property(env, objValue, "START",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::START)));
    napi_set_named_property(env, objValue, "CENTER",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::CENTER)));
    napi_set_named_property(env, objValue, "END",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::END)));
    napi_set_named_property(env, objValue, "BOTTOM_START",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::BOTTOM_START)));
    napi_set_named_property(env, objValue, "BOTTOM",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::BOTTOM)));
    napi_set_named_property(env, objValue, "BOTTOM_END",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnchor::BOTTOM_END)));
    return objValue;
}

napi_value CreatePixelUnitType(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "PX",
        CreateJsValue(env, static_cast<uint32_t>(PixelUnit::PX)));
    napi_set_named_property(env, objValue, "VP",
        CreateJsValue(env, static_cast<uint32_t>(PixelUnit::VP)));
    return objValue;
}

napi_value CreateWaterfallResidentState(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "UNCHANGED",
        CreateJsValue(env, static_cast<uint32_t>(WaterfallResidentState::UNCHANGED)));
    napi_set_named_property(env, objValue, "OPEN",
        CreateJsValue(env, static_cast<uint32_t>(WaterfallResidentState::OPEN)));
    napi_set_named_property(env, objValue, "CLOSE",
        CreateJsValue(env, static_cast<uint32_t>(WaterfallResidentState::CLOSE)));
    return objValue;
}

napi_value CreateCompatibleStyleMode(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "INVALID_VALUE",
        CreateJsValue(env, static_cast<uint32_t>(CompatibleStyleMode::INVALID_VALUE)));
    napi_set_named_property(env, objValue, "LANDSCAPE_DEFAULT",
        CreateJsValue(env, static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_DEFAULT)));
    napi_set_named_property(env, objValue, "LANDSCAPE_18_9",
        CreateJsValue(env, static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_18_9)));
    napi_set_named_property(env, objValue, "LANDSCAPE_1_1",
        CreateJsValue(env, static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_1_1)));
    napi_set_named_property(env, objValue, "LANDSCAPE_2_3",
        CreateJsValue(env, static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_2_3)));
    napi_set_named_property(env, objValue, "LANDSCAPE_SPLIT",
        CreateJsValue(env, static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_SPLIT)));
    return objValue;
}
 
MainThreadScheduler::MainThreadScheduler(napi_env env)
    : env_(env)
{
    GetMainEventHandler();
    envChecker_ = std::make_shared<int>(0);
}

inline void MainThreadScheduler::GetMainEventHandler()
{
    auto runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    handler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
}

void MainThreadScheduler::PostMainThreadTask(Task&& localTask, std::string traceInfo, int64_t delayTime)
{
    auto task = [env = env_, localTask = std::move(localTask), traceInfo,
                 envChecker = std::weak_ptr<int>(envChecker_)] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SCBCb:%s", traceInfo.c_str());
        if (envChecker.expired()) {
            TLOGNE(WmsLogTag::WMS_MAIN, "post task expired because of invalid scheduler");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_status status = napi_open_handle_scope(env, &scope);
        if (status != napi_ok || scope == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "open handle scope failed.");
            return;
        }
        localTask();
        napi_close_handle_scope(env, scope);
    };
    if (handler_ && handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        return task();
    } else if (handler_ && !handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        handler_->PostTask(std::move(task), "wms:" + traceInfo, delayTime,
            OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        napi_send_event(env_, task, napi_eprio_immediate, traceInfo.c_str());
    }
}

void MainThreadScheduler::RemoveMainThreadTaskByName(const std::string taskName)
{
    if (handler_ && !handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        handler_->RemoveTask("wms:" + taskName);
    }
}

bool convertAnimConfigFromJs(napi_env env, napi_value jsObject, SceneAnimationConfig& config)
{
    napi_value jsDelay = nullptr;
    napi_get_named_property(env, jsObject, "delay", &jsDelay);
    napi_value jsDuration = nullptr;
    napi_get_named_property(env, jsObject, "duration", &jsDuration);
    napi_value jsAnimationCurve = nullptr;
    napi_get_named_property(env, jsObject, "curve", &jsAnimationCurve);
    napi_value jsParams = nullptr;
    napi_get_named_property(env, jsObject, "param", &jsParams);
    int32_t delay = 0;
    int32_t duration = 0;
    WindowAnimationCurve curve = WindowAnimationCurve::LINEAR;
    if (GetType(env, jsDelay) != napi_undefined && !ConvertFromJsValue(env, jsDelay, delay)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to delay");
        return false;
    }
    config.animationDelay_ = static_cast<uint32_t>(delay > 0 ? delay : 0);
    if (GetType(env, jsDuration) != napi_undefined && !ConvertFromJsValue(env, jsDuration, duration)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to duration");
        return false;
    }
    config.animationDuration_ = static_cast<uint32_t>(duration > 0 ? duration : 0);
    if (GetType(env, jsAnimationCurve) != napi_undefined && !ConvertFromJsValue(env, jsAnimationCurve, curve)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to curve");
        return false;
    }
    config.animationCurve_ = static_cast<WindowAnimationCurve>(curve);
    if (jsParams != nullptr && GetType(env, jsParams) != napi_undefined) {
        bool isArray = false;
        if (napi_is_array(env, jsParams, &isArray) != napi_ok || !isArray) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Params is not array");
            return false;
        }
        for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
            napi_value element = nullptr;
            double paramValue = 0.0;
            if (napi_get_element(env, jsParams, i, &element) == napi_ok &&
                napi_get_value_double(env, element, &paramValue) == napi_ok && paramValue >= 0) {
                config.animationParam_[i] = static_cast<float>(paramValue);
            } else {
                config.animationParam_[i] = static_cast<float>(paramValue);
            }
        }
    }
    return true;
}
} // namespace OHOS::Rosen