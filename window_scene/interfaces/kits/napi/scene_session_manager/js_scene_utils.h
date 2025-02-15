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
};

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
};

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
napi_value CreateJsSessionInfo(napi_env env, const SessionInfo& sessionInfo);
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
napi_value CreateJsSessionPiPControlType(napi_env env);
napi_value CreateJsSessionPiPControlStatus(napi_env env);
napi_value CreateJsSessionGravity(napi_env env);
napi_value CreateJsSessionDragResizeType(napi_env env);
void CreatePiPSizeChangeReason(napi_env env, napi_value objValue);
template<typename T>
napi_value CreateJsSessionRect(napi_env env, const T& rect);
napi_value CreateJsSessionEventParam(napi_env env, const SessionEventParam& param);
napi_value SubWindowModalTypeInit(napi_env env);
napi_value CreateJsSystemBarPropertyArrayObject(
    napi_env env, const std::unordered_map<WindowType, SystemBarProperty>& propertyMap);
napi_value CreateJsKeyboardLayoutParams(napi_env env, const KeyboardLayoutParams& params);
napi_value SessionTypeInit(napi_env env);
napi_value SceneTypeInit(napi_env env);
napi_value KeyboardGravityInit(napi_env env);
napi_value KeyboardViewModeInit(napi_env env);
napi_value NapiGetUndefined(napi_env env);
napi_valuetype GetType(napi_env env, napi_value value);
bool NapiIsCallable(napi_env env, napi_value value);
bool ConvertRectInfoFromJs(napi_env env, napi_value jsObject, WSRect& rect);
bool ConvertHookInfoFromJs(napi_env env, napi_value jsObject, HookInfo& hookInfo);
bool ConvertRotateAnimationConfigFromJs(napi_env env, napi_value value, RotateAnimationConfig& config);
bool ConvertDragResizeTypeFromJs(napi_env env, napi_value value, DragResizeType& dragResizeType);
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

private:
    void GetMainEventHandler();
    napi_env env_;
    std::shared_ptr<int> envChecker_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_;
};
} // namespace OHOS::Rosen
#endif // OHOS_WINDOW_SCENE_JS_SCENE_UTILS_H
