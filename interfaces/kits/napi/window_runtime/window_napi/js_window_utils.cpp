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

#include "js_window_utils.h"
#include <iomanip>
#include <regex>
#include <sstream>
#include "accesstoken_kit.h"
#include "bundle_constants.h"
#include "ipc_skeleton.h"
#include "window_manager_hilog.h"
#include "js_window.h"
#include "wm_common.h"
#include "pixel_map.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsUtils"};
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t FOUR_PARAMS_SIZE = 4;
constexpr size_t ARG_COUNT_ZERO = 0;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr size_t ARG_COUNT_THREE = 3;
constexpr int32_t MAX_TOUCHABLE_AREAS = 10;
constexpr uint32_t CURVE_TYPE_SPRING = 1;
constexpr uint32_t ANIMATION_FOUR_PARAMS_SIZE = 4;
const std::string RESOLVED_CALLBACK = "resolvedCallback";
const std::string REJECTED_CALLBACK = "rejectedCallback";
const std::string INTERPOLATINGSPRING  = "interpolatingSpring";
constexpr std::array<DefaultSpecificZIndex, 2> DefaultSpecificZIndexList = {
    DefaultSpecificZIndex::MUTISCREEN_COLLABORATION,
    DefaultSpecificZIndex::SUPER_PRIVACY_ANIMATION
};
}

const std::map<WindowType, ApiWindowType> NATIVE_JS_TO_WINDOW_TYPE_MAP {
    { WindowType::WINDOW_TYPE_APP_SUB_WINDOW,           ApiWindowType::TYPE_APP                      },
    { WindowType::WINDOW_TYPE_DIALOG,                   ApiWindowType::TYPE_DIALOG                   },
    { WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,      ApiWindowType::TYPE_SYSTEM_ALERT             },
    { WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT,       ApiWindowType::TYPE_INPUT_METHOD             },
    { WindowType::WINDOW_TYPE_STATUS_BAR,               ApiWindowType::TYPE_STATUS_BAR               },
    { WindowType::WINDOW_TYPE_PANEL,                    ApiWindowType::TYPE_PANEL                    },
    { WindowType::WINDOW_TYPE_KEYGUARD,                 ApiWindowType::TYPE_KEYGUARD                 },
    { WindowType::WINDOW_TYPE_VOLUME_OVERLAY,           ApiWindowType::TYPE_VOLUME_OVERLAY           },
    { WindowType::WINDOW_TYPE_NAVIGATION_BAR,           ApiWindowType::TYPE_NAVIGATION_BAR           },
    { WindowType::WINDOW_TYPE_FLOAT,                    ApiWindowType::TYPE_FLOAT                    },
    { WindowType::WINDOW_TYPE_FLOAT_CAMERA,             ApiWindowType::TYPE_FLOAT_CAMERA             },
    { WindowType::WINDOW_TYPE_WALLPAPER,                ApiWindowType::TYPE_WALLPAPER                },
    { WindowType::WINDOW_TYPE_DESKTOP,                  ApiWindowType::TYPE_DESKTOP                  },
    { WindowType::WINDOW_TYPE_LAUNCHER_RECENT,          ApiWindowType::TYPE_LAUNCHER_RECENT          },
    { WindowType::WINDOW_TYPE_LAUNCHER_DOCK,            ApiWindowType::TYPE_LAUNCHER_DOCK            },
    { WindowType::WINDOW_TYPE_VOICE_INTERACTION,        ApiWindowType::TYPE_VOICE_INTERACTION        },
    { WindowType::WINDOW_TYPE_POINTER,                  ApiWindowType::TYPE_POINTER                  },
    { WindowType::WINDOW_TYPE_SCREENSHOT,               ApiWindowType::TYPE_SCREENSHOT               },
    { WindowType::WINDOW_TYPE_SYSTEM_TOAST,             ApiWindowType::TYPE_SYSTEM_TOAST             },
    { WindowType::WINDOW_TYPE_DOCK_SLICE,               ApiWindowType::TYPE_DIVIDER                  },
    { WindowType::WINDOW_TYPE_GLOBAL_SEARCH,            ApiWindowType::TYPE_GLOBAL_SEARCH            },
    { WindowType::WINDOW_TYPE_HANDWRITE,                ApiWindowType::TYPE_HANDWRITE                },
    { WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD,        ApiWindowType::TYPE_WALLET_SWIPE_CARD        },
    { WindowType::WINDOW_TYPE_SCREEN_CONTROL,           ApiWindowType::TYPE_SCREEN_CONTROL           },
    { WindowType::WINDOW_TYPE_FLOAT_NAVIGATION,         ApiWindowType::TYPE_FLOAT_NAVIGATION         },
    { WindowType::WINDOW_TYPE_DYNAMIC,                  ApiWindowType::TYPE_DYNAMIC                  },
    { WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION, ApiWindowType::TYPE_MUTISCREEN_COLLABORATION },
    { WindowType::WINDOW_TYPE_FB,                       ApiWindowType::TYPE_FB                       },
};

const std::map<ApiWindowType, WindowType> JS_TO_NATIVE_WINDOW_TYPE_MAP {
    { ApiWindowType::TYPE_APP,                      WindowType::WINDOW_TYPE_APP_SUB_WINDOW           },
    { ApiWindowType::TYPE_DIALOG,                   WindowType::WINDOW_TYPE_DIALOG                   },
    { ApiWindowType::TYPE_SYSTEM_ALERT,             WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW      },
    { ApiWindowType::TYPE_INPUT_METHOD,             WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT       },
    { ApiWindowType::TYPE_STATUS_BAR,               WindowType::WINDOW_TYPE_STATUS_BAR               },
    { ApiWindowType::TYPE_PANEL,                    WindowType::WINDOW_TYPE_PANEL                    },
    { ApiWindowType::TYPE_KEYGUARD,                 WindowType::WINDOW_TYPE_KEYGUARD                 },
    { ApiWindowType::TYPE_VOLUME_OVERLAY,           WindowType::WINDOW_TYPE_VOLUME_OVERLAY           },
    { ApiWindowType::TYPE_NAVIGATION_BAR,           WindowType::WINDOW_TYPE_NAVIGATION_BAR           },
    { ApiWindowType::TYPE_FLOAT,                    WindowType::WINDOW_TYPE_FLOAT                    },
    { ApiWindowType::TYPE_FLOAT_CAMERA,             WindowType::WINDOW_TYPE_FLOAT_CAMERA             },
    { ApiWindowType::TYPE_WALLPAPER,                WindowType::WINDOW_TYPE_WALLPAPER                },
    { ApiWindowType::TYPE_DESKTOP,                  WindowType::WINDOW_TYPE_DESKTOP                  },
    { ApiWindowType::TYPE_LAUNCHER_RECENT,          WindowType::WINDOW_TYPE_LAUNCHER_RECENT          },
    { ApiWindowType::TYPE_LAUNCHER_DOCK,            WindowType::WINDOW_TYPE_LAUNCHER_DOCK            },
    { ApiWindowType::TYPE_VOICE_INTERACTION,        WindowType::WINDOW_TYPE_VOICE_INTERACTION        },
    { ApiWindowType::TYPE_POINTER,                  WindowType::WINDOW_TYPE_POINTER                  },
    { ApiWindowType::TYPE_SCREENSHOT,               WindowType::WINDOW_TYPE_SCREENSHOT               },
    { ApiWindowType::TYPE_SYSTEM_TOAST,             WindowType::WINDOW_TYPE_SYSTEM_TOAST             },
    { ApiWindowType::TYPE_DIVIDER,                  WindowType::WINDOW_TYPE_DOCK_SLICE               },
    { ApiWindowType::TYPE_GLOBAL_SEARCH,            WindowType::WINDOW_TYPE_GLOBAL_SEARCH            },
    { ApiWindowType::TYPE_HANDWRITE,                WindowType::WINDOW_TYPE_HANDWRITE                },
    { ApiWindowType::TYPE_WALLET_SWIPE_CARD,        WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD        },
    { ApiWindowType::TYPE_SCREEN_CONTROL,           WindowType::WINDOW_TYPE_SCREEN_CONTROL           },
    { ApiWindowType::TYPE_FLOAT_NAVIGATION,         WindowType::WINDOW_TYPE_FLOAT_NAVIGATION         },
    { ApiWindowType::TYPE_DYNAMIC,                  WindowType::WINDOW_TYPE_DYNAMIC                  },
    { ApiWindowType::TYPE_MUTISCREEN_COLLABORATION, WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION },
    { ApiWindowType::TYPE_FB,                       WindowType::WINDOW_TYPE_FB                       },
};

const std::map<WindowMode, ApiWindowMode> NATIVE_TO_JS_WINDOW_MODE_MAP {
    { WindowMode::WINDOW_MODE_UNDEFINED,       ApiWindowMode::UNDEFINED  },
    { WindowMode::WINDOW_MODE_FULLSCREEN,      ApiWindowMode::FULLSCREEN },
    { WindowMode::WINDOW_MODE_SPLIT_PRIMARY,   ApiWindowMode::PRIMARY    },
    { WindowMode::WINDOW_MODE_SPLIT_SECONDARY, ApiWindowMode::SECONDARY  },
    { WindowMode::WINDOW_MODE_FLOATING,        ApiWindowMode::FLOATING   },
};

const std::map<ApiWindowMode, WindowMode> JS_TO_NATIVE_WINDOW_MODE_MAP {
    {ApiWindowMode::UNDEFINED,  WindowMode::WINDOW_MODE_UNDEFINED       },
    {ApiWindowMode::FULLSCREEN, WindowMode::WINDOW_MODE_FULLSCREEN      },
    {ApiWindowMode::PRIMARY,    WindowMode::WINDOW_MODE_SPLIT_PRIMARY   },
    {ApiWindowMode::SECONDARY,  WindowMode::WINDOW_MODE_SPLIT_SECONDARY },
    {ApiWindowMode::FLOATING,   WindowMode::WINDOW_MODE_FLOATING        },
};

const std::map<ApiOrientation, Orientation> JS_TO_NATIVE_ORIENTATION_MAP {
    {ApiOrientation::UNSPECIFIED,                           Orientation::UNSPECIFIED                        },
    {ApiOrientation::PORTRAIT,                              Orientation::VERTICAL                           },
    {ApiOrientation::LANDSCAPE,                             Orientation::HORIZONTAL                         },
    {ApiOrientation::PORTRAIT_INVERTED,                     Orientation::REVERSE_VERTICAL                   },
    {ApiOrientation::LANDSCAPE_INVERTED,                    Orientation::REVERSE_HORIZONTAL                 },
    {ApiOrientation::AUTO_ROTATION,                         Orientation::SENSOR                             },
    {ApiOrientation::AUTO_ROTATION_PORTRAIT,                Orientation::SENSOR_VERTICAL                    },
    {ApiOrientation::AUTO_ROTATION_LANDSCAPE,               Orientation::SENSOR_HORIZONTAL                  },
    {ApiOrientation::AUTO_ROTATION_RESTRICTED,              Orientation::AUTO_ROTATION_RESTRICTED           },
    {ApiOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED,     Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED  },
    {ApiOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED,    Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED },
    {ApiOrientation::LOCKED,                                Orientation::LOCKED                             },
    {ApiOrientation::AUTO_ROTATION_UNSPECIFIED,             Orientation::AUTO_ROTATION_UNSPECIFIED          },
    {ApiOrientation::USER_ROTATION_PORTRAIT,                Orientation::USER_ROTATION_PORTRAIT             },
    {ApiOrientation::USER_ROTATION_LANDSCAPE,               Orientation::USER_ROTATION_LANDSCAPE            },
    {ApiOrientation::USER_ROTATION_PORTRAIT_INVERTED,       Orientation::USER_ROTATION_PORTRAIT_INVERTED    },
    {ApiOrientation::USER_ROTATION_LANDSCAPE_INVERTED,      Orientation::USER_ROTATION_LANDSCAPE_INVERTED   },
    {ApiOrientation::FOLLOW_DESKTOP,                        Orientation::FOLLOW_DESKTOP                     },
};

const std::map<Orientation, ApiOrientation> NATIVE_TO_JS_ORIENTATION_MAP {
    {Orientation::UNSPECIFIED,                           ApiOrientation::UNSPECIFIED                        },
    {Orientation::VERTICAL,                              ApiOrientation::PORTRAIT                           },
    {Orientation::HORIZONTAL,                            ApiOrientation::LANDSCAPE                          },
    {Orientation::REVERSE_VERTICAL,                      ApiOrientation::PORTRAIT_INVERTED                  },
    {Orientation::REVERSE_HORIZONTAL,                    ApiOrientation::LANDSCAPE_INVERTED                 },
    {Orientation::SENSOR,                                ApiOrientation::AUTO_ROTATION                      },
    {Orientation::SENSOR_VERTICAL,                       ApiOrientation::AUTO_ROTATION_PORTRAIT             },
    {Orientation::SENSOR_HORIZONTAL,                     ApiOrientation::AUTO_ROTATION_LANDSCAPE            },
    {Orientation::AUTO_ROTATION_RESTRICTED,              ApiOrientation::AUTO_ROTATION_RESTRICTED           },
    {Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED,     ApiOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED  },
    {Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED,    ApiOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED },
    {Orientation::LOCKED,                                ApiOrientation::LOCKED                             },
    {Orientation::FOLLOW_RECENT,                         ApiOrientation::UNSPECIFIED                        },
    {Orientation::AUTO_ROTATION_UNSPECIFIED,             ApiOrientation::AUTO_ROTATION_UNSPECIFIED          },
    {Orientation::USER_ROTATION_PORTRAIT,                ApiOrientation::USER_ROTATION_PORTRAIT             },
    {Orientation::USER_ROTATION_LANDSCAPE,               ApiOrientation::USER_ROTATION_LANDSCAPE            },
    {Orientation::USER_ROTATION_PORTRAIT_INVERTED,       ApiOrientation::USER_ROTATION_PORTRAIT_INVERTED    },
    {Orientation::USER_ROTATION_LANDSCAPE_INVERTED,      ApiOrientation::USER_ROTATION_LANDSCAPE_INVERTED   },
    {Orientation::FOLLOW_DESKTOP,                        ApiOrientation::FOLLOW_DESKTOP                     },
};

const std::map<WindowSizeChangeReason, RectChangeReason> JS_SIZE_CHANGE_REASON {
    { WindowSizeChangeReason::UNDEFINED,                       RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::MAXIMIZE,                        RectChangeReason::MAXIMIZE   },
    { WindowSizeChangeReason::RECOVER,                         RectChangeReason::RECOVER    },
    { WindowSizeChangeReason::ROTATION,                        RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::DRAG,                            RectChangeReason::DRAG       },
    { WindowSizeChangeReason::DRAG_START,                      RectChangeReason::DRAG_START },
    { WindowSizeChangeReason::DRAG_MOVE,                       RectChangeReason::MOVE       },
    { WindowSizeChangeReason::DRAG_END,                        RectChangeReason::DRAG_END   },
    { WindowSizeChangeReason::RESIZE,                          RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::MOVE,                            RectChangeReason::MOVE       },
    { WindowSizeChangeReason::HIDE,                            RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::TRANSFORM,                       RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::CUSTOM_ANIMATION_SHOW,           RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::FULL_TO_SPLIT,                   RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SPLIT_TO_FULL,                   RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::FULL_TO_FLOATING,                RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::FLOATING_TO_FULL,                RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::MAXIMIZE_TO_SPLIT,               RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SPLIT_TO_MAXIMIZE,               RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::PAGE_ROTATION,                   RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SPLIT_DRAG_START,                RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SPLIT_DRAG,                      RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SPLIT_DRAG_END,                  RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::RESIZE_BY_LIMIT,                 RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::MAXIMIZE_IN_IMPLICT,             RectChangeReason::MAXIMIZE   },
    { WindowSizeChangeReason::RECOVER_IN_IMPLICIT,             RectChangeReason::RECOVER    },
    { WindowSizeChangeReason::SCREEN_RELATIVE_POSITION_CHANGE, RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SNAPSHOT_ROTATION,               RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::LS_STATE_CHANGE,                 RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::END,                             RectChangeReason::UNDEFINED  },
};

napi_value WindowTypeInit(napi_env env)
{
    WLOGFD("WindowTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "TYPE_APP", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_APP)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_ALERT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SYSTEM_ALERT)));
    napi_set_named_property(env, objValue, "TYPE_INPUT_METHOD", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_INPUT_METHOD)));
    napi_set_named_property(env, objValue, "TYPE_STATUS_BAR", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_STATUS_BAR)));
    napi_set_named_property(env, objValue, "TYPE_PANEL", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_PANEL)));
    napi_set_named_property(env, objValue, "TYPE_KEYGUARD", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_KEYGUARD)));
    napi_set_named_property(env, objValue, "TYPE_VOLUME_OVERLAY", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_VOLUME_OVERLAY)));
    napi_set_named_property(env, objValue, "TYPE_NAVIGATION_BAR", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_NAVIGATION_BAR)));
    napi_set_named_property(env, objValue, "TYPE_FLOAT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_FLOAT)));
    napi_set_named_property(env, objValue, "TYPE_FLOAT_CAMERA", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_FLOAT_CAMERA)));
    napi_set_named_property(env, objValue, "TYPE_WALLPAPER", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_WALLPAPER)));
    napi_set_named_property(env, objValue, "TYPE_DESKTOP", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DESKTOP)));
    napi_set_named_property(env, objValue, "TYPE_LAUNCHER_RECENT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_RECENT)));
    napi_set_named_property(env, objValue, "TYPE_LAUNCHER_DOCK", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_DOCK)));
    napi_set_named_property(env, objValue, "TYPE_VOICE_INTERACTION", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_VOICE_INTERACTION)));
    napi_set_named_property(env, objValue, "TYPE_DIALOG", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DIALOG)));
    napi_set_named_property(env, objValue, "TYPE_POINTER", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_POINTER)));
    napi_set_named_property(env, objValue, "TYPE_SCREENSHOT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SCREENSHOT)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_TOAST", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SYSTEM_TOAST)));
    napi_set_named_property(env, objValue, "TYPE_DIVIDER", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DIVIDER)));
    napi_set_named_property(env, objValue, "TYPE_GLOBAL_SEARCH", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_GLOBAL_SEARCH)));
    napi_set_named_property(env, objValue, "TYPE_HANDWRITE", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_HANDWRITE)));
    napi_set_named_property(env, objValue, "TYPE_WALLET_SWIPE_CARD", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_WALLET_SWIPE_CARD)));
    napi_set_named_property(env, objValue, "TYPE_SCREEN_CONTROL", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SCREEN_CONTROL)));
    napi_set_named_property(env, objValue, "TYPE_FLOAT_NAVIGATION", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_FLOAT_NAVIGATION)));
    napi_set_named_property(env, objValue, "TYPE_DYNAMIC", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DYNAMIC)));
    napi_set_named_property(env, objValue, "TYPE_MUTISCREEN_COLLABORATION", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_MUTISCREEN_COLLABORATION)));

    return objValue;
}

napi_value AvoidAreaTypeInit(napi_env env)
{
    WLOGFD("AvoidAreaTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "TYPE_SYSTEM", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_SYSTEM)));
    napi_set_named_property(env, objValue, "TYPE_CUTOUT", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_CUTOUT)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_GESTURE", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_SYSTEM_GESTURE)));
    napi_set_named_property(env, objValue, "TYPE_KEYBOARD",
        CreateJsValue(env, static_cast<int32_t>(AvoidAreaType::TYPE_KEYBOARD)));
    napi_set_named_property(env, objValue, "TYPE_NAVIGATION_INDICATOR",
        CreateJsValue(env, static_cast<int32_t>(AvoidAreaType::TYPE_NAVIGATION_INDICATOR)));
    return objValue;
}

napi_value WindowModeInit(napi_env env)
{
    WLOGFD("WindowModeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::UNDEFINED)));
    napi_set_named_property(env, objValue, "FULLSCREEN", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::FULLSCREEN)));
    napi_set_named_property(env, objValue, "PRIMARY", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::PRIMARY)));
    napi_set_named_property(env, objValue, "SECONDARY", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::SECONDARY)));
    napi_set_named_property(env, objValue, "FLOATING", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::FLOATING)));
    return objValue;
}

napi_value GlobalWindowModeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "FULLSCREEN", CreateJsValue(env,
        static_cast<int32_t>(GlobalWindowMode::FULLSCREEN)));
    napi_set_named_property(env, objValue, "SPLIT", CreateJsValue(env,
        static_cast<int32_t>(GlobalWindowMode::SPLIT)));
    napi_set_named_property(env, objValue, "FLOAT", CreateJsValue(env,
        static_cast<int32_t>(GlobalWindowMode::FLOAT)));
    napi_set_named_property(env, objValue, "PIP", CreateJsValue(env,
        static_cast<int32_t>(GlobalWindowMode::PIP)));
    return objValue;
}

napi_value WindowOcclusionStateInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "NO_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION)));
    napi_set_named_property(env, objValue, "PARTIAL_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION)));
    napi_set_named_property(env, objValue, "FULL_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION)));
    return objValue;
}

napi_value ScreenshotEventTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "SYSTEM_SCREENSHOT", CreateJsValue(env,
        static_cast<int32_t>(ScreenshotEventType::SYSTEM_SCREENSHOT)));
    napi_set_named_property(env, objValue, "SYSTEM_SCREENSHOT_ABORT", CreateJsValue(env,
        static_cast<int32_t>(ScreenshotEventType::SYSTEM_SCREENSHOT_ABORT)));
    napi_set_named_property(env, objValue, "SCROLL_SHOT_START", CreateJsValue(env,
        static_cast<int32_t>(ScreenshotEventType::SCROLL_SHOT_START)));
    napi_set_named_property(env, objValue, "SCROLL_SHOT_END", CreateJsValue(env,
        static_cast<int32_t>(ScreenshotEventType::SCROLL_SHOT_END)));
    napi_set_named_property(env, objValue, "SCROLL_SHOT_ABORT", CreateJsValue(env,
        static_cast<int32_t>(ScreenshotEventType::SCROLL_SHOT_ABORT)));
    return objValue;
}

napi_value ColorSpaceInit(napi_env env)
{
    WLOGFD("ColorSpaceInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "DEFAULT", CreateJsValue(env,
        static_cast<int32_t>(ColorSpace::COLOR_SPACE_DEFAULT)));
    napi_set_named_property(env, objValue, "WIDE_GAMUT", CreateJsValue(env,
        static_cast<int32_t>(ColorSpace::COLOR_SPACE_WIDE_GAMUT)));
    return objValue;
}

napi_value OrientationInit(napi_env env)
{
    WLOGFD("OrientationInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "PORTRAIT", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::PORTRAIT)));
    napi_set_named_property(env, objValue, "LANDSCAPE", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::LANDSCAPE)));
    napi_set_named_property(env, objValue, "PORTRAIT_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::PORTRAIT_INVERTED)));
    napi_set_named_property(env, objValue, "LANDSCAPE_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::LANDSCAPE_INVERTED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_PORTRAIT", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_PORTRAIT)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_LANDSCAPE", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_LANDSCAPE)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_RESTRICTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_RESTRICTED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_PORTRAIT_RESTRICTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_LANDSCAPE_RESTRICTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED)));
    napi_set_named_property(env, objValue, "LOCKED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::LOCKED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_UNSPECIFIED)));
    napi_set_named_property(env, objValue, "USER_ROTATION_PORTRAIT", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_PORTRAIT)));
    napi_set_named_property(env, objValue, "USER_ROTATION_LANDSCAPE", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_LANDSCAPE)));
    napi_set_named_property(env, objValue, "USER_ROTATION_PORTRAIT_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_PORTRAIT_INVERTED)));
    napi_set_named_property(env, objValue, "USER_ROTATION_LANDSCAPE_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_LANDSCAPE_INVERTED)));
    napi_set_named_property(env, objValue, "FOLLOW_DESKTOP", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::FOLLOW_DESKTOP)));
    return objValue;
}

napi_value WindowStageEventTypeInit(napi_env env)
{
    WLOGFD("WindowStageEventTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "SHOWN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::FOREGROUND)));
    napi_set_named_property(env, objValue, "ACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::ACTIVE)));
    napi_set_named_property(env, objValue, "INACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::INACTIVE)));
    napi_set_named_property(env, objValue, "HIDDEN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::BACKGROUND)));
    napi_set_named_property(env, objValue, "RESUMED", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::RESUMED)));
    napi_set_named_property(env, objValue, "PAUSED", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::PAUSED)));
    return objValue;
}

napi_value WindowStageLifecycleEventTypeInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "SHOWN", CreateJsValue(env,
        static_cast<int32_t>(WindowStageLifeCycleEventType::FOREGROUND)));
    napi_set_named_property(env, objValue, "RESUMED", CreateJsValue(env,
        static_cast<int32_t>(WindowStageLifeCycleEventType::RESUMED)));
    napi_set_named_property(env, objValue, "PAUSED", CreateJsValue(env,
        static_cast<int32_t>(WindowStageLifeCycleEventType::PAUSED)));
    napi_set_named_property(env, objValue, "HIDDEN", CreateJsValue(env,
        static_cast<int32_t>(WindowStageLifeCycleEventType::BACKGROUND)));
    return objValue;
}

napi_value WindowAnchorInit(napi_env env)
{
    WLOGFD("WindowAnchorInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "TOP_START", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::TOP_START)));
    napi_set_named_property(env, objValue, "TOP", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::TOP)));
    napi_set_named_property(env, objValue, "TOP_END", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::TOP_END)));
    napi_set_named_property(env, objValue, "START", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::START)));
    napi_set_named_property(env, objValue, "CENTER", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::CENTER)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::END)));
    napi_set_named_property(env, objValue, "BOTTOM_START", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::BOTTOM_START)));
    napi_set_named_property(env, objValue, "BOTTOM", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::BOTTOM)));
    napi_set_named_property(env, objValue, "BOTTOM_END", CreateJsValue(env,
        static_cast<int32_t>(WindowAnchor::BOTTOM_END)));
    return objValue;
}

napi_value PixelUnitInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "PX", CreateJsValue(env,
        static_cast<int32_t>(PixelUnit::PX)));
    napi_set_named_property(env, objValue, "VP", CreateJsValue(env,
        static_cast<int32_t>(PixelUnit::VP)));
    return objValue;
}

napi_value WindowEventTypeInit(napi_env env)
{
    WLOGFD("WindowEventTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WINDOW_SHOWN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::FOREGROUND)));
    napi_set_named_property(env, objValue, "WINDOW_ACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::ACTIVE)));
    napi_set_named_property(env, objValue, "WINDOW_INACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::INACTIVE)));
    napi_set_named_property(env, objValue, "WINDOW_HIDDEN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::BACKGROUND)));
    napi_set_named_property(env, objValue, "WINDOW_DESTROYED", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::DESTROYED)));
    return objValue;
}

napi_value WindowLayoutModeInit(napi_env env)
{
    WLOGFD("WindowLayoutModeInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WINDOW_LAYOUT_MODE_CASCADE", CreateJsValue(env,
        static_cast<int32_t>(WindowLayoutMode::CASCADE)));
    napi_set_named_property(env, objValue, "WINDOW_LAYOUT_MODE_TILE", CreateJsValue(env,
        static_cast<int32_t>(WindowLayoutMode::TILE)));
    return objValue;
}

napi_value BlurStyleInit(napi_env env)
{
    WLOGD("BlurStyleInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "OFF", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_OFF)));
    napi_set_named_property(env, objValue, "THIN", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_THIN)));
    napi_set_named_property(env, objValue, "REGULAR", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_REGULAR)));
    napi_set_named_property(env, objValue, "THICK", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_THICK)));
    return objValue;
}

napi_value MaximizePresentationInit(napi_env env)
{
    WLOGD("MaximizePresentationInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "FOLLOW_APP_IMMERSIVE_SETTING", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING)));
    napi_set_named_property(env, objValue, "EXIT_IMMERSIVE", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::EXIT_IMMERSIVE)));
    napi_set_named_property(env, objValue, "ENTER_IMMERSIVE", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::ENTER_IMMERSIVE)));
    napi_set_named_property(env, objValue, "ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER)));
    return objValue;
}

napi_value WindowErrorInit(napi_env env)
{
    WLOGFD("WindowErrorInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WM_DO_NOTHING", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_DO_NOTHING)));
    napi_set_named_property(env, objValue, "WM_ERROR_NO_MEM", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_NO_MEM)));
    napi_set_named_property(env, objValue, "WM_ERROR_DESTROYED_OBJECT", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_DESTROYED_OBJECT)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_WINDOW", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_OPERATION", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_OPERATION)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_PERMISSION", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION)));
    napi_set_named_property(env, objValue, "WM_ERROR_NO_REMOTE_ANIMATION", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_NO_REMOTE_ANIMATION)));
    napi_set_named_property(env, objValue, "WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_DEVICE_NOT_SUPPORT)));
    napi_set_named_property(env, objValue, "WM_ERROR_NULLPTR", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_NULLPTR)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_TYPE", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_TYPE)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_PARAM", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, objValue, "WM_ERROR_SAMGR", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_SAMGR)));
    napi_set_named_property(env, objValue, "WM_ERROR_IPC_FAILED", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_IPC_FAILED)));
    napi_set_named_property(env, objValue, "WM_ERROR_START_ABILITY_FAILED", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_START_ABILITY_FAILED)));
    return objValue;
}

napi_value WindowErrorCodeInit(napi_env env)
{
    WLOGFD("WindowErrorCodeInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "WM_ERROR_NO_PERMISSION", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_NO_PERMISSION)));
    napi_set_named_property(env, objValue, "WM_ERROR_NOT_SYSTEM_APP", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_PARAM", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, objValue, "WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    napi_set_named_property(env, objValue, "WM_ERROR_REPEAT_OPERATION", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_REPEAT_OPERATION)));
    napi_set_named_property(env, objValue, "WM_ERROR_STATE_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_SYSTEM_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_CALLING", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING)));
    napi_set_named_property(env, objValue, "WM_ERROR_STAGE_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_CONTEXT_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_START_ABILITY_FAILED", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_START_ABILITY_FAILED)));
    return objValue;
}

napi_value WindowStatusTypeInit(napi_env env)
{
    WLOGFD("WindowStatusTypeInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_UNDEFINED)));
    napi_set_named_property(env, objValue, "FULL_SCREEN", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_FULLSCREEN)));
    napi_set_named_property(env, objValue, "MAXIMIZE", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_MAXIMIZE)));
    napi_set_named_property(env, objValue, "MINIMIZE", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_MINIMIZE)));
    napi_set_named_property(env, objValue, "FLOATING", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_FLOATING)));
    napi_set_named_property(env, objValue, "SPLIT_SCREEN", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_SPLITSCREEN)));
    return objValue;
}

napi_value RectChangeReasonInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "RectChangeReasonInit called");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "UNDEFINED",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::UNDEFINED)));
    napi_set_named_property(env, objValue, "MAXIMIZE",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::MAXIMIZE)));
    napi_set_named_property(env, objValue, "RECOVER",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::RECOVER)));
    napi_set_named_property(env, objValue, "MOVE",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::MOVE)));
    napi_set_named_property(env, objValue, "DRAG",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::DRAG)));
    napi_set_named_property(env, objValue, "DRAG_START",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::DRAG_START)));
    napi_set_named_property(env, objValue, "DRAG_END",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::DRAG_END)));
    return objValue;
}

napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value CreateJsWindowAnimationConfigObject(napi_env env, const KeyboardAnimationCurve& curve)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_status status = napi_invalid_arg;
    if (curve.curveType_ == INTERPOLATINGSPRING) {
        status = napi_set_named_property(env, objValue, "curve", CreateJsValue(env, CURVE_TYPE_SPRING));
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Fail to set curve to napi property");
            return NapiGetUndefined(env);
        }
    }
    status = napi_set_named_property(env, objValue, "duration", CreateJsValue(env, curve.duration_));
    if (status != napi_ok) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Fail to set duration to napi property");
        return NapiGetUndefined(env);
    }
    auto paramSize = curve.curveParams_.size();
    if (paramSize == ANIMATION_FOUR_PARAMS_SIZE) {
        napi_value jsArray = nullptr;
        status = napi_create_array_with_length(env, paramSize, &jsArray);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Fail to create jsArray");
            return NapiGetUndefined(env);
        }
        for (size_t i = 0; i < paramSize; i++) {
            napi_value param = nullptr;
            status = napi_create_double(env, static_cast<double>(curve.curveParams_[i]), &param);
            if (status != napi_ok) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "Fail to create jsDoubleValue");
                return NapiGetUndefined(env);
            }
            status = napi_set_element(env, jsArray, i, param);
            if (status != napi_ok) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "Fail to set element to jsArray");
                return NapiGetUndefined(env);
            }
        }
        status = napi_set_named_property(env, objValue, "param", jsArray);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Fail to set param to napi property");
            return NapiGetUndefined(env);
        }
    }
    return objValue;
}

napi_value CreateJsWindowPropertiesObject(napi_env env, const WindowPropertyInfo& windowPropertyInfo)
{
    WLOGD("CreateJsWindowPropertiesObject");
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_value windowRectObj = GetRectAndConvertToJsValue(env, windowPropertyInfo.windowRect);
    if (windowRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "GetWindowRect failed!");
    }
    napi_set_named_property(env, objValue, "windowRect", windowRectObj);

    napi_value drawableRectObj = GetRectAndConvertToJsValue(env, windowPropertyInfo.drawableRect);
    if (drawableRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "GetDrawableRect failed!");
    }
    napi_set_named_property(env, objValue, "drawableRect", drawableRectObj);

    napi_value globalDisplayRectObj = GetRectAndConvertToJsValue(env, windowPropertyInfo.globalDisplayRect);
    if (globalDisplayRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "GetGlobalDisplayRect failed!");
    }
    napi_set_named_property(env, objValue, "globalDisplayRect", globalDisplayRectObj);

    WindowType type = windowPropertyInfo.type;
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(type) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(type)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, type));
    }
    napi_set_named_property(env, objValue, "isLayoutFullScreen",
                            CreateJsValue(env, windowPropertyInfo.isLayoutFullScreen));
    napi_set_named_property(env, objValue, "isFullScreen", CreateJsValue(env, windowPropertyInfo.isFullScreen));
    napi_set_named_property(env, objValue, "touchable", CreateJsValue(env, windowPropertyInfo.isTouchable));
    napi_set_named_property(env, objValue, "focusable", CreateJsValue(env, windowPropertyInfo.isFocusable));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, windowPropertyInfo.name));
    napi_set_named_property(env, objValue, "isPrivacyMode", CreateJsValue(env, windowPropertyInfo.isPrivacyMode));
    napi_set_named_property(env, objValue, "isKeepScreenOn", CreateJsValue(env, windowPropertyInfo.isKeepScreenOn));
    napi_set_named_property(env, objValue, "brightness", CreateJsValue(env, windowPropertyInfo.brightness));
    napi_set_named_property(env, objValue, "isTransparent", CreateJsValue(env, windowPropertyInfo.isTransparent));
    napi_set_named_property(env, objValue, "isRoundCorner", CreateJsValue(env, false)); // empty method
    napi_set_named_property(env, objValue, "dimBehindValue", CreateJsValue(env, 0));
    napi_set_named_property(env, objValue, "id", CreateJsValue(env, windowPropertyInfo.id));
    napi_set_named_property(env, objValue, "displayId", CreateJsValue(env,
        static_cast<int64_t>(windowPropertyInfo.displayId)));
    return objValue;
}

static std::string GetHexColor(uint32_t color)
{
    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = RGBA_LENGTH - static_cast<int>(temp.length());
    std::string tmpColor(count, '0');
    tmpColor += temp;
    std::string finalColor("#");
    finalColor += tmpColor;
    return finalColor;
}

napi_value CreateJsSystemBarPropertiesObject(napi_env env, sptr<Window>& window)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    SystemBarProperty status = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty navi = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    napi_set_named_property(env, objValue, "statusBarColor",
        CreateJsValue(env, GetHexColor(status.backgroundColor_)));
    napi_set_named_property(env, objValue, "statusBarContentColor",
        CreateJsValue(env, GetHexColor(status.contentColor_)));
    napi_set_named_property(env, objValue, "isStatusBarLightIcon",
        CreateJsValue(env, status.contentColor_ == SYSTEM_COLOR_WHITE));
    napi_set_named_property(env, objValue, "navigationBarColor",
        CreateJsValue(env, GetHexColor(navi.backgroundColor_)));
    napi_set_named_property(env, objValue, "navigationBarContentColor",
        CreateJsValue(env, GetHexColor(navi.contentColor_)));
    napi_set_named_property(env, objValue, "isNavigationBarLightIcon",
        CreateJsValue(env, navi.contentColor_ == SYSTEM_COLOR_WHITE));
    napi_set_named_property(env, objValue, "enableStatusBarAnimation",
                            CreateJsValue(env, status.enableAnimation_));
    napi_set_named_property(env, objValue, "enableNavigationBarAnimation",
                            CreateJsValue(env, navi.enableAnimation_));
    return objValue;
}

static napi_value CreateJsSystemBarRegionTintObject(napi_env env, const SystemBarRegionTint& tint)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(tint.type_) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(tint.type_)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, tint.type_));
    }
    napi_set_named_property(env, objValue, "isEnable", CreateJsValue(env, tint.prop_.enable_));
    std::string bkgColor = GetHexColor(tint.prop_.backgroundColor_);
    napi_set_named_property(env, objValue, "backgroundColor", CreateJsValue(env, bkgColor));
    std::string contentColor = GetHexColor(tint.prop_.contentColor_);
    napi_set_named_property(env, objValue, "contentColor", CreateJsValue(env, contentColor));
    Rect rect = tint.region_;
    napi_set_named_property(env, objValue, "region", GetRectAndConvertToJsValue(env, rect));

    WLOGFD("Type %{public}u [%{public}u %{public}s %{public}s]",
        tint.type_, tint.prop_.enable_, bkgColor.c_str(), contentColor.c_str());
    WLOGFD("Region [%{public}d %{public}d %{public}u %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    return objValue;
}

napi_value CreateJsWindowLayoutInfoArrayObject(napi_env env, const std::vector<sptr<WindowLayoutInfo>>& infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "arrayValue is null");
        return nullptr;
    }
    for (size_t i = 0; i < infos.size(); i++) {
        napi_set_element(env, arrayValue, i, CreateJsWindowLayoutInfoObject(env, infos[i]));
    }
    return arrayValue;
}

napi_value CreateJsPixelMapArrayObject(napi_env env, const std::vector<std::shared_ptr<Media::PixelMap>>& pixelMaps)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, pixelMaps.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "arrayValue is null");
        return nullptr;
    }
    for (size_t i = 0; i < pixelMaps.size(); i++) {
        if (!pixelMaps[i]) {
            TLOGW(WmsLogTag::WMS_LIFE, "pixelMaps index: %{public}d is null", static_cast<int32_t>(i));
            napi_set_element(env, arrayValue, i, NapiGetUndefined(env));
        } else {
            napi_set_element(env, arrayValue, i, CreateJsPixelMapObject(env, pixelMaps[i]));
        }
    }
    return arrayValue;
}
 
napi_value CreateJsMainWindowInfoArrayObject(napi_env env, const std::vector<sptr<MainWindowInfo>>& infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "arrayValue is null");
        return nullptr;
    }
    for (size_t i = 0; i < infos.size(); i++) {
        napi_set_element(env, arrayValue, i, CreateJsMainWindowInfoObject(env, infos[i]));
    }
    return arrayValue;
}

napi_value CreateJsWindowInfoArrayObject(napi_env env, const std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("Failed to convert windowVisibilityInfo to jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < infos.size(); i++) {
        auto info = infos[i];
        auto windowType = info->GetWindowType();
        if (windowType >= WindowType::APP_MAIN_WINDOW_BASE && windowType < WindowType::APP_MAIN_WINDOW_END) {
            napi_set_element(env, arrayValue, index++, CreateJsWindowInfoObject(env, info));
        }
    }
    return arrayValue;
}

napi_value CreateJsDecorButtonStyleObj(napi_env env, DecorButtonStyle decorButtonStyle)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "colorMode", CreateJsValue(env, decorButtonStyle.colorMode));
    napi_set_named_property(env, objValue, "buttonBackgroundSize",
        CreateJsValue(env, decorButtonStyle.buttonBackgroundSize));
    napi_set_named_property(env, objValue, "spacingBetweenButtons",
        CreateJsValue(env, decorButtonStyle.spacingBetweenButtons));
    napi_set_named_property(env, objValue, "closeButtonRightMargin",
        CreateJsValue(env, decorButtonStyle.closeButtonRightMargin));
    napi_set_named_property(env, objValue, "buttonIconSize",
        CreateJsValue(env, decorButtonStyle.buttonIconSize));
    napi_set_named_property(env, objValue, "buttonBackgroundCornerRadius",
        CreateJsValue(env, decorButtonStyle.buttonBackgroundCornerRadius));
    return objValue;
}

bool ConvertDecorButtonStyleFromJs(napi_env env, napi_value jsObject, DecorButtonStyle& style)
{
    int32_t colorMode;
    bool emptyParam = true;
    if (ParseJsValue(jsObject, env, "colorMode", colorMode)) {
        style.colorMode = colorMode;
        emptyParam = false;
    }
    uint32_t buttonBackgroundSize;
    if (ParseJsValue(jsObject, env, "buttonBackgroundSize", buttonBackgroundSize)) {
        style.buttonBackgroundSize = buttonBackgroundSize;
        emptyParam = false;
    }
    uint32_t spacingBetweenButtons;
    if (ParseJsValue(jsObject, env, "spacingBetweenButtons", spacingBetweenButtons)) {
        style.spacingBetweenButtons = spacingBetweenButtons;
        emptyParam = false;
    }
    uint32_t closeButtonRightMargin;
    if (ParseJsValue(jsObject, env, "closeButtonRightMargin", closeButtonRightMargin)) {
        style.closeButtonRightMargin = closeButtonRightMargin;
        emptyParam = false;
    }
    uint32_t buttonIconSize;
    if (ParseJsValue(jsObject, env, "buttonIconSize", buttonIconSize)) {
        style.buttonIconSize = buttonIconSize;
        emptyParam = false;
    }
    uint32_t buttonBackgroundCornerRadius;
    if (ParseJsValue(jsObject, env, "buttonBackgroundCornerRadius", buttonBackgroundCornerRadius)) {
        style.buttonBackgroundCornerRadius = buttonBackgroundCornerRadius;
        emptyParam = false;
    }
    return !emptyParam;
}

napi_value CreateJsWindowLayoutInfoObject(napi_env env, const sptr<WindowLayoutInfo>& info)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "windowRect", GetRectAndConvertToJsValue(env, info->rect));
    return objValue;
}

napi_value CreateJsMainWindowInfoObject(napi_env env, const sptr<MainWindowInfo>& info)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
 
    napi_set_named_property(env, objValue, "displayId", CreateJsValue(env, static_cast<uint32_t>(info->displayId_)));
    napi_set_named_property(env, objValue, "windowId", CreateJsValue(env, info->persistentId_));
    napi_set_named_property(env, objValue, "showing", CreateJsValue(env, info->showing_));
    napi_set_named_property(env, objValue, "label", CreateJsValue(env, info->label_));
    return objValue;
}
 
napi_value CreateJsPixelMapObject(napi_env env, const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    if (!pixelMap) {
        TLOGE(WmsLogTag::WMS_LIFE, "pixelMap is null");
        return nullptr;
    }
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    
    objValue = Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
    return objValue;
}

napi_value CreateJsWindowInfoObject(napi_env env, const sptr<WindowVisibilityInfo>& info)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "rect", GetRectAndConvertToJsValue(env, info->GetRect()));
    napi_set_named_property(env, objValue, "globalDisplayRect",
        GetRectAndConvertToJsValue(env, info->GetGlobalDisplayRect()));
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, info->GetBundleName()));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info->GetAbilityName()));
    napi_set_named_property(env, objValue, "windowId", CreateJsValue(env, info->GetWindowId()));
    napi_set_named_property(env, objValue, "windowStatusType",
        CreateJsValue(env, static_cast<int32_t>(info->GetWindowStatus())));
    napi_set_named_property(env, objValue, "isFocused", CreateJsValue(env, info->IsFocused()));
    return objValue;
}

napi_value CreateJsSystemBarRegionTintArrayObject(napi_env env, const SystemBarRegionTints& tints)
{
    if (tints.empty()) {
        WLOGFE("Empty tints");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_array_with_length(env, tints.size(), &objValue);
    napi_value array = objValue;
    if (array == nullptr) {
        WLOGFE("Failed to convert SystemBarProperties to jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < tints.size(); i++) {
        napi_set_element(env, array, index++, CreateJsSystemBarRegionTintObject(env, tints[i]));
    }
    return objValue;
}

napi_value CreateRotationChangeInfoObject(napi_env env, const RotationChangeInfo& info)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, static_cast<uint32_t>(info.type_)));
    napi_set_named_property(env, objValue, "orientation", CreateJsValue(env, info.orientation_));
    napi_set_named_property(env, objValue, "displayId", CreateJsValue(env, static_cast<uint32_t>(info.displayId_)));
    napi_set_named_property(env, objValue, "displayRect", GetRectAndConvertToJsValue(env, info.displayRect_));
    return objValue;
}

bool GetSystemBarStatus(napi_env env, napi_callback_info info,
    std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarpropertyFlags)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    uint32_t size = INDEX_ZERO;
    napi_value nativeArray = nullptr;
    if (argc > ARG_COUNT_ZERO && GetType(env, argv[INDEX_ZERO]) != napi_function) {
        nativeArray = argv[INDEX_ZERO];
        if (nativeArray == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to SystemBarArray");
            return false;
        }
        napi_get_array_length(env, nativeArray, &size);
    }
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = false;
    for (uint32_t i = 0; i < size; i++) {
        std::string name;
        napi_value getElementValue = nullptr;
        napi_get_element(env, nativeArray, i, &getElementValue);
        if (!ConvertFromJsValue(env, getElementValue, name)) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to SystemBarName");
            return false;
        }
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

napi_value GetStatusBarPropertyObject(napi_env env, sptr<Window>& window)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    SystemBarProperty statusBarProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    napi_set_named_property(env, objValue, "contentColor",
        CreateJsValue(env, GetHexColor(statusBarProperty.contentColor_)));
    return objValue;
}

static bool CheckTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }
    return valueType == expectType;
}

bool ParseColorMetrics(napi_env env, napi_value value, uint32_t& colorValue)
{
    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        return false;
    }
    napi_value jsToNumeric = nullptr;
    napi_get_named_property(env, value, "toNumeric", &jsToNumeric);
    if (!CheckTypeForNapiValue(env, jsToNumeric, napi_function)) {
        return false;
    }
    napi_value jsColor;
    if (napi_call_function(env, value, jsToNumeric, 0, nullptr, &jsColor) != napi_ok) {
        return false;
    }
    if (!CheckTypeForNapiValue(env, jsColor, napi_number)) {
        return false;
    }
    return napi_get_value_uint32(env, jsColor, &colorValue) == napi_ok;
}

bool GetWindowBackgroundColorFromJs(napi_env env, napi_value value, std::string& colorStr)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        return ConvertFromJsValue(env, value, colorStr);
    }
    uint32_t colorValue = 0;
    if (ParseColorMetrics(env, value, colorValue)) {
        colorStr = GetHexColor(colorValue);
        return true;
    }
    return false;
}

bool ParseAndCheckRect(napi_env env, napi_value jsObject,
    const Rect& windowRect, Rect& touchableRect)
{
    int32_t data = 0;
    if (ParseJsValue(jsObject, env, "left", data)) {
        touchableRect.posX_ = data;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:legt");
        return false;
    }
    if (ParseJsValue(jsObject, env, "top", data)) {
        touchableRect.posY_ = data;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:top");
        return false;
    }
    uint32_t udata = 0;
    if (ParseJsValue(jsObject, env, "width", udata)) {
        touchableRect.width_ = udata;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:width");
        return false;
    }
    if (ParseJsValue(jsObject, env, "height", udata)) {
        touchableRect.height_ = udata;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:height");
        return false;
    }
    if ((touchableRect.posX_ < 0) || (touchableRect.posY_ < 0) ||
        (touchableRect.posX_ > static_cast<int32_t>(windowRect.width_)) ||
        (touchableRect.posY_ > static_cast<int32_t>(windowRect.height_)) ||
        (touchableRect.width_ > (windowRect.width_ - static_cast<uint32_t>(touchableRect.posX_))) ||
        (touchableRect.height_ > (windowRect.height_ - static_cast<uint32_t>(touchableRect.posY_)))) {
        TLOGE(WmsLogTag::WMS_EVENT, "Outside the window area, "
            "touchRect:[%{public}d %{public}d %{public}u %{public}u], "
            "windowRect:[%{public}d %{public}d %{public}u %{public}u]",
            touchableRect.posX_, touchableRect.posY_, touchableRect.width_, touchableRect.height_,
            windowRect.posX_, windowRect.posY_, windowRect.width_, windowRect.height_);
        return false;
    }
    return true;
}

WmErrorCode ParseTouchableAreas(napi_env env, napi_callback_info info,
    const Rect& windowRect, std::vector<Rect>& touchableAreas)
{
    WmErrorCode errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_EVENT, "Argc is invalid:%{public}zu", argc);
        return errCode;
    }
    if (GetType(env, argv[0]) != napi_object) {
        TLOGE(WmsLogTag::WMS_EVENT, "GetType error");
        return errCode;
    }
    napi_value nativeArray = argv[0];
    if (nativeArray == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter");
        return errCode;
    }
    uint32_t size = 0;
    napi_get_array_length(env, nativeArray, &size);
    if (size > MAX_TOUCHABLE_AREAS) {
        TLOGE(WmsLogTag::WMS_EVENT, "Exceeded maximum limit");
        return errCode;
    }
    errCode = WmErrorCode::WM_OK;
    for (uint32_t i = 0; i < size; i++) {
        napi_value getElementValue = nullptr;
        napi_get_element(env, nativeArray, i, &getElementValue);
        if (getElementValue == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "Failed to get element");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            break;
        }
        Rect touchableArea;
        if (ParseAndCheckRect(env, getElementValue, windowRect, touchableArea)) {
            touchableAreas.emplace_back(touchableArea);
        } else {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            break;
        }
    }
    return errCode;
}

bool GetSpecificBarStatus(napi_env env, napi_callback_info info, bool& systemBarEnable, bool& systemBarEnableAnimation)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_TWO || !ConvertFromJsValue(env, argv[INDEX_ONE], systemBarEnable)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu or Failed to convert enable parameter to bool", argc);
        return false;
    }
    if (argc >= ARG_COUNT_THREE && !ConvertFromJsValue(env, argv[INDEX_TWO], systemBarEnableAnimation)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert enableAnimation parameter to bool");
        return false;
    }
    return true;
}

static uint32_t GetColorFromJs(napi_env env, napi_value jsObject,
    const char* name, uint32_t defaultColor, bool& flag)
{
    napi_value jsColor = nullptr;
    napi_get_named_property(env, jsObject, name, &jsColor);
    if (GetType(env, jsColor) != napi_undefined) {
        std::string colorStr;
        if (!ConvertFromJsValue(env, jsColor, colorStr)) {
            WLOGFE("Failed to convert parameter to color");
            return defaultColor;
        }
        std::regex pattern("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{8})$");
        if (!std::regex_match(colorStr, pattern)) {
            WLOGFD("Invalid color input");
            return defaultColor;
        }
        std::string color = colorStr.substr(1);
        if (color.length() == RGB_LENGTH) {
            color = "FF" + color; // ARGB
        }
        flag = true;
        std::stringstream ss;
        uint32_t hexColor;
        ss << std::hex << color;
        ss >> hexColor;
        TLOGD(WmsLogTag::DEFAULT, "Origin %{public}s, process %{public}s, final %{public}x",
            colorStr.c_str(), color.c_str(), hexColor);
        return hexColor;
    }
    return defaultColor;
}

bool SetWindowStatusBarContentColor(napi_env env, napi_value jsObject,
    std::unordered_map<WindowType, SystemBarProperty>& properties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto statusProperty = properties[WindowType::WINDOW_TYPE_STATUS_BAR];
    napi_value jsStatusContentColor = nullptr;
    napi_get_named_property(env, jsObject, "statusBarContentColor", &jsStatusContentColor);
    napi_value jsStatusIcon = nullptr;
    napi_get_named_property(env, jsObject, "isStatusBarLightIcon", &jsStatusIcon);
    if (GetType(env, jsStatusContentColor) != napi_undefined) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ =  GetColorFromJs(env,
            jsObject, "statusBarContentColor", statusProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag);
    } else if (GetType(env, jsStatusIcon) != napi_undefined) {
        bool isStatusBarLightIcon;
        if (!ConvertFromJsValue(env, jsStatusIcon, isStatusBarLightIcon)) {
            TLOGE(WmsLogTag::DEFAULT, "Convert status icon value failed");
            return false;
        }
        if (isStatusBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag = true;
    }
    return true;
}

bool SetWindowNavigationBarContentColor(napi_env env, napi_value jsObject,
    std::unordered_map<WindowType, SystemBarProperty>& properties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto navProperty = properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR];
    napi_value jsNavigationContentColor = nullptr;
    napi_get_named_property(env, jsObject, "navigationBarContentColor", &jsNavigationContentColor);
    napi_value jsNavigationIcon = nullptr;
    napi_get_named_property(env, jsObject, "isNavigationBarLightIcon", &jsNavigationIcon);
    if (GetType(env, jsNavigationContentColor) != napi_undefined) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = GetColorFromJs(env,
            jsObject, "navigationBarContentColor", navProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag);
    } else if (GetType(env, jsNavigationIcon) != napi_undefined) {
        bool isNavigationBarLightIcon;
        if (!ConvertFromJsValue(env, jsNavigationIcon, isNavigationBarLightIcon)) {
            TLOGE(WmsLogTag::DEFAULT, "Convert navigation icon value failed");
            return false;
        }
        if (isNavigationBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag = true;
    }
    return true;
}

bool GetSystemBarPropertiesFromJs(napi_env env, napi_value jsObject,
    std::unordered_map<WindowType, SystemBarProperty>& properties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_ =
        GetColorFromJs(env, jsObject, "statusBarColor",
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColorFlag);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_ =
        GetColorFromJs(env, jsObject, "navigationBarColor",
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColorFlag);

    if (!SetWindowStatusBarContentColor(env, jsObject, properties, propertyFlags) ||
        !SetWindowNavigationBarContentColor(env, jsObject, properties, propertyFlags)) {
        return false;
    }
    bool enableStatusBarAnimation = false;
    if (ParseJsValue(jsObject, env, "enableStatusBarAnimation", enableStatusBarAnimation)) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ = enableStatusBarAnimation;
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimationFlag = true;
    }
    bool enableNavigationBarAnimation = false;
    if (ParseJsValue(jsObject, env, "enableNavigationBarAnimation", enableNavigationBarAnimation)) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ = enableNavigationBarAnimation;
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimationFlag = true;
    }
    return true;
}

void ConvertJSSystemBarStyleToSystemBarProperties(napi_env env, napi_value jsObject,
    std::map<WindowType, SystemBarProperty>& properties, std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = GetColorFromJs(env,
        jsObject, "statusBarContentColor",
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_,
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag);
}

napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea,
    AvoidAreaType type, bool useActualVisibility)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    if (useActualVisibility) {
        napi_set_named_property(env, objValue, "visible", CreateJsValue(env, !avoidArea.isEmptyAvoidArea()));
    } else {
        napi_set_named_property(env, objValue, "visible", CreateJsValue(env, type != AvoidAreaType::TYPE_CUTOUT));
    }
    napi_set_named_property(env, objValue, "leftRect", GetRectAndConvertToJsValue(env, avoidArea.leftRect_));
    napi_set_named_property(env, objValue, "topRect", GetRectAndConvertToJsValue(env, avoidArea.topRect_));
    napi_set_named_property(env, objValue, "rightRect", GetRectAndConvertToJsValue(env, avoidArea.rightRect_));
    napi_set_named_property(env, objValue, "bottomRect", GetRectAndConvertToJsValue(env, avoidArea.bottomRect_));
    return objValue;
}

napi_value GetWindowLimitsAndConvertToJsValue(napi_env env, const WindowLimits& windowLimits)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "maxWidth", CreateJsValue(env, windowLimits.maxWidth_));
    napi_set_named_property(env, objValue, "maxHeight", CreateJsValue(env, windowLimits.maxHeight_));
    napi_set_named_property(env, objValue, "minWidth", CreateJsValue(env, windowLimits.minWidth_));
    napi_set_named_property(env, objValue, "minHeight", CreateJsValue(env, windowLimits.minHeight_));
    napi_set_named_property(env, objValue, "pixelUnit", CreateJsValue(env, windowLimits.pixelUnit_));
    return objValue;
}

napi_value ConvertTitleButtonAreaToJsValue(napi_env env, const TitleButtonRect& titleButtonRect)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "right", CreateJsValue(env, titleButtonRect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, titleButtonRect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, titleButtonRect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, titleButtonRect.height_));
    return objValue;
}

napi_value ConvertFrameMetricsToJsValue(napi_env env, const FrameMetrics& metrics)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "firstDrawFrame", CreateJsValue(env, metrics.firstDrawFrame_));
    napi_set_named_property(env, objValue, "inputHandlingDuration",
        CreateJsValue(env, static_cast<int64_t>(metrics.inputHandlingDuration_)));
    napi_set_named_property(env, objValue, "layoutMeasureDuration",
        CreateJsValue(env, static_cast<int64_t>(metrics.layoutMeasureDuration_)));
    napi_set_named_property(env, objValue, "vsyncTimestamp",
        CreateJsValue(env, static_cast<int64_t>(metrics.vsyncTimestamp_)));
    return objValue;
}

napi_value ConvertWindowDensityInfoToJsValue(napi_env env, const WindowDensityInfo& windowDensityInfo)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(
        env, objValue, "systemDensity", CreateJsValue(env, static_cast<double>(windowDensityInfo.systemDensity)));
    napi_set_named_property(
        env, objValue, "defaultDensity", CreateJsValue(env, static_cast<double>(windowDensityInfo.defaultDensity)));
    napi_set_named_property(
        env, objValue, "customDensity", CreateJsValue(env, static_cast<double>(windowDensityInfo.customDensity)));
    return objValue;
}

bool CheckCallingPermission(std::string permission)
{
    WLOGD("Permission: %{public}s", permission.c_str());
    if (!permission.empty() &&
        Security::AccessToken::AccessTokenKit::VerifyAccessToken(IPCSkeleton::GetCallingTokenID(), permission)
        != AppExecFwk::Constants::PERMISSION_GRANTED) {
        WLOGFE("Permission %{public}s is not granted", permission.c_str());
        return false;
    }
    return true;
}

bool ParseSystemWindowTypeForApiWindowType(int32_t apiWindowType, WindowType& windowType)
{
    if (JS_TO_NATIVE_WINDOW_TYPE_MAP.count(static_cast<ApiWindowType>(apiWindowType)) != 0) {
        windowType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(apiWindowType));
        if (WindowHelper::IsSystemWindow(windowType)) {
            return true;
        }
    }
    return false;
}

bool GetAPI7Ability(napi_env env, AppExecFwk::Ability* &ability)
{
    napi_value global;
    if (napi_get_global(env, &global) != napi_ok) {
        WLOGI("Get global failed");
        return false;
    }
    napi_value jsAbility;
    napi_status status = napi_get_named_property(env, global, "ability", &jsAbility);
    if (status != napi_ok || jsAbility == nullptr) {
        WLOGI("Get ability property failed");
        return false;
    }

    if (napi_get_value_external(env, jsAbility, reinterpret_cast<void **>(&ability)) != napi_ok) {
        WLOGI("Get ability external failed");
        return false;
    }
    if (ability == nullptr) {
        return false;
    } else {
        WLOGI("Get ability");
    }
    return true;
}
bool GetWindowMaskFromJsValue(napi_env env, napi_value jsObject, std::vector<std::vector<uint32_t>>& windowMask)
{
    if (jsObject == nullptr) {
        WLOGFE("Failed to convert parameter to window mask");
        return false;
    }
    uint32_t size = 0;
    napi_get_array_length(env, jsObject, &size);
    for (uint32_t i = 0; i < size; i++) {
        std::vector<uint32_t> elementArray;
        napi_value getElementValue = nullptr;
        napi_get_element(env, jsObject, i, &getElementValue);
        if (!ConvertNativeValueToVector(env, getElementValue, elementArray)) {
            WLOGFE("Failed to convert parameter to window mask");
            return false;
        }
        windowMask.emplace_back(elementArray);
    }
    return true;
}

bool GetWindowIdFromJsValue(napi_env env, napi_value jsObject, std::vector<int32_t>& windowIds)
{
    if (jsObject == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to window id, jsObject is nullptr");
        return false;
    }
    uint32_t size = 0;
    if (GetType(env, jsObject) != napi_object || napi_get_array_length(env, jsObject, &size) == napi_invalid_arg) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to window id, invalid arg");
        return false;
    }
    if (size == 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "invalid windowIds size");
        return false;
    }
    for (uint32_t i = 0; i < size; i++) {
        int32_t elementArray;
        napi_value getElementValue = nullptr;
        napi_get_element(env, jsObject, i, &getElementValue);
        if (!ConvertFromJsValue(env, getElementValue, elementArray)) {
            TLOGE(WmsLogTag::WMS_LIFE, "jsObject is null");
            return false;
        }
        windowIds.emplace_back(elementArray);
    }
    return true;
}

bool GetMoveConfigurationFromJsValue(napi_env env, napi_value jsObject, MoveConfiguration& moveConfiguration)
{
    if (jsObject == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "jsObject is null");
        return false;
    }
    napi_value jsConfig = nullptr;
    napi_get_named_property(env, jsObject, "displayId", &jsConfig);
    if (GetType(env, jsConfig) != napi_undefined) {
        int64_t displayId = DISPLAY_ID_INVALID;
        if (!ConvertFromJsValue(env, jsConfig, displayId)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to displayId");
            return false;
        }
        moveConfiguration.displayId = static_cast<DisplayId>(displayId);
    }
    napi_get_named_property(env, jsObject, "rectAnimationConfig", &jsConfig);
    if (GetType(env, jsConfig) != napi_undefined) {
        RectAnimationConfig rectAnimationConfig;
        if (!ParseRectAnimationConfig(env, jsConfig, rectAnimationConfig)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to rectAnimationConfig");
            return false;
        }
        moveConfiguration.rectAnimationConfig = rectAnimationConfig;
    }
    return true;
}

bool ParseRectAnimationConfig(napi_env env, napi_value jsObject, RectAnimationConfig& rectAnimationConfig)
{
    uint32_t duration = 0;
    if (ParseJsValue(jsObject, env, "duration", duration)) {
        if (duration <= 0) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "RectAnimationConfig duration invalid");
            return false;
        }
        rectAnimationConfig.duration = duration;
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to rectAnimationConfig duration");
        return false;
    }
    double coordinate = 0.0;
    if (ParseJsValue(jsObject, env, "x1", coordinate)) {
        rectAnimationConfig.x1 = static_cast<float>(std::clamp(coordinate, 0.0, 1.0));
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to rectAnimationConfig x1");
        return false;
    }
    if (ParseJsValue(jsObject, env, "y1", coordinate)) {
        rectAnimationConfig.y1 = static_cast<float>(coordinate);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to rectAnimationConfig y1");
        return false;
    }
    if (ParseJsValue(jsObject, env, "x2", coordinate)) {
        rectAnimationConfig.x2 = static_cast<float>(std::clamp(coordinate, 0.0, 1.0));
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to rectAnimationConfig x2");
        return false;
    }
    if (ParseJsValue(jsObject, env, "y2", coordinate)) {
        rectAnimationConfig.y2 = static_cast<float>(coordinate);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to rectAnimationConfig y2");
        return false;
    }
    return true;
}

bool ConvertRectFromJsValue(napi_env env, napi_value jsObject, Rect& displayRect)
{
    if (!ParseJsValue(jsObject, env, "left", displayRect.posX_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert parameter to posX_");
        return false;
    }

    if (!ParseJsValue(jsObject, env, "top", displayRect.posY_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert parameter to posY_");
        return false;
    }

    if (!ParseJsValue(jsObject, env, "width", displayRect.width_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert parameter to width_");
        return false;
    }

    if (!ParseJsValue(jsObject, env, "height", displayRect.height_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert parameter to height_");
        return false;
    }
    return true;
}

bool GetRotationResultFromJs(napi_env env, napi_value jsObject, RotationChangeResult& rotationChangeResult)
{
    napi_value jsRectType = nullptr;
    napi_value jsWindowRect = nullptr;
    napi_get_named_property(env, jsObject, "rectType", &jsRectType);
    napi_get_named_property(env, jsObject, "windowRect", &jsWindowRect);
    if (GetType(env, jsRectType) != napi_undefined) {
        uint32_t rectType;
        if (!ConvertFromJsValue(env, jsRectType, rectType)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert parameter to rectType");
            return false;
        }
        rotationChangeResult.rectType_ = static_cast<RectType>(rectType);
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to get object rectType");
        return false;
    }
    if (GetType(env, jsWindowRect) != napi_undefined) {
        Rect windowRect;
        if (!ConvertRectFromJsValue(env, jsWindowRect, windowRect)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert parameter to windowRect");
            return false;
        }
        rotationChangeResult.windowRect_ = windowRect;
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to get object windowRect");
        return false;
    }
    return true;
}

napi_value ExtensionWindowAttributeInit(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to create object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "SYSTEM_WINDOW",
        CreateJsValue(env, static_cast<int32_t>(ExtensionWindowAttribute::SYSTEM_WINDOW)));
    napi_set_named_property(env, objValue, "SUB_WINDOW",
        CreateJsValue(env, static_cast<int32_t>(ExtensionWindowAttribute::SUB_WINDOW)));
    return objValue;
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTask(napi_env env, napi_value lastParam,
    std::unique_ptr<NapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback>&& complete, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &nativeDeferred, result));
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}

std::unique_ptr<NapiAsyncTask> CreateEmptyAsyncTask(napi_env env,
    napi_value lastParam, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred,
            std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef,
            std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    }
}

napi_value ModalityTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WINDOW_MODALITY",
        CreateJsValue(env, ApiModalityType::WINDOW_MODALITY));
    napi_set_named_property(env, objValue, "APPLICATION_MODALITY",
        CreateJsValue(env, ApiModalityType::APPLICATION_MODALITY));
    return objValue;
}

napi_value RotationChangeTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "WINDOW_WILL_ROTATE",
        CreateJsValue(env, static_cast<uint32_t>(RotationChangeType::WINDOW_WILL_ROTATE)));
    napi_set_named_property(env, objValue, "WINDOW_DID_ROTATE",
        CreateJsValue(env, static_cast<uint32_t>(RotationChangeType::WINDOW_DID_ROTATE)));
    return objValue;
}

napi_value RotationInfoTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "WINDOW_ORIENTATION",
        CreateJsValue(env, static_cast<uint32_t>(RotationInfoType::WINDOW_ORIENTATION)));
    napi_set_named_property(env, objValue, "DISPLAY_ORIENTATION",
        CreateJsValue(env, static_cast<uint32_t>(RotationInfoType::DISPLAY_ORIENTATION)));
    napi_set_named_property(env, objValue, "DISPLAY_ROTATION",
        CreateJsValue(env, static_cast<uint32_t>(RotationInfoType::DISPLAY_ROTATION)));
    return objValue;
}

napi_value RectTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "RELATIVE_TO_SCREEN",
        CreateJsValue(env, static_cast<uint32_t>(RectType::RELATIVE_TO_SCREEN)));
    napi_set_named_property(env, objValue, "RELATIVE_TO_PARENT_WINDOW",
        CreateJsValue(env, static_cast<uint32_t>(RectType::RELATIVE_TO_PARENT_WINDOW)));
    return objValue;
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
    napi_set_named_property(env, objValue, "CUBIC_BEZIER",
        CreateJsValue(env, static_cast<uint32_t>(WindowAnimationCurve::CUBIC_BEZIER)));
    return objValue;
}

static bool ParseModalityParam(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption)
{
    bool isModal = false;
    if (ParseJsValue(jsObject, env, "isModal", isModal)) {
        if (isModal) {
            windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
        }
    }
    bool isTopmost = false;
    if (ParseJsValue(jsObject, env, "isTopmost", isTopmost)) {
        if (!isModal && isTopmost) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support topmost");
            return false;
        }
        windowOption->SetWindowTopmost(isTopmost);
    }
    ApiModalityType apiModalityType;
    if (ParseJsValue(jsObject, env, "modalityType", apiModalityType)) {
        if (!isModal) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support modalityType");
            return false;
        }
        using T = std::underlying_type_t<ApiModalityType>;
        T modalityType = static_cast<T>(apiModalityType);
        if (modalityType >= static_cast<T>(ApiModalityType::BEGIN) &&
            modalityType <= static_cast<T>(ApiModalityType::END)) {
            auto type = JS_TO_NATIVE_MODALITY_TYPE_MAP.at(apiModalityType);
            if (type == ModalityType::APPLICATION_MODALITY) {
                windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
            }
        } else {
            TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to modalityType");
            return false;
        }
    }
    TLOGI(WmsLogTag::WMS_SUB, "isModal: %{public}d, isTopmost: %{public}d, WindowFlag: %{public}d",
        isModal, isTopmost, windowOption->GetWindowFlags());
    return true;
}

static bool ParseRectParam(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption)
{
    napi_value windowRect = nullptr;
    napi_get_named_property(env, jsObject, "windowRect", &windowRect);
    if (windowRect == nullptr || GetType(env, windowRect) != napi_object) {
        return true;
    }
    int32_t width = 0;
    if (!ParseJsValue(windowRect, env, "width", width)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to width");
        return false;
    }
    int32_t height = 0;
    if (!ParseJsValue(windowRect, env, "height", height)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to height");
        return false;
    }
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_SUB, "width or height should greater than 0!");
        return false;
    }
    int32_t left = 0;
    if (!ParseJsValue(windowRect, env, "left", left)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to left");
        return false;
    }
    int32_t top = 0;
    if (!ParseJsValue(windowRect, env, "top", top)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to top");
        return false;
    }
    Rect rect = { left, top, static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    windowOption->SetWindowRect(rect);
    TLOGI(WmsLogTag::WMS_SUB, "windowRect: %{public}s", rect.ToString().c_str());
    return true;
}

static bool ParseZLevelParam(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption)
{
    int32_t zLevel = 0;
    bool isModal = false;
    if (ParseJsValue(jsObject, env, "zLevel", zLevel)) {
        if (zLevel < MINIMUM_Z_LEVEL || zLevel > MAXIMUM_Z_LEVEL) {
            TLOGE(WmsLogTag::WMS_SUB, "zLevel value %{public}d exceeds valid range [-10000, 10000]!", zLevel);
            return false;
        }
        if (ParseJsValue(jsObject, env, "isModal", isModal)) {
            if (isModal) {
                TLOGE(WmsLogTag::WMS_SUB, "modal window not support custom zLevel");
                return false;
            }
        }
        windowOption->SetSubWindowZLevel(zLevel);
    }
    TLOGI(WmsLogTag::WMS_SUB, "zLevel: %{public}d", zLevel);
    return true;
}

bool ParseSubWindowOptions(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption)
{
    if (jsObject == nullptr || windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "jsObject or windowOption is null");
        return false;
    }
    std::string title;
    if (!ParseJsValue(jsObject, env, "title", title)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to title");
        return false;
    }
    bool decorEnabled = false;
    if (!ParseJsValue(jsObject, env, "decorEnabled", decorEnabled)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to decorEnabled");
        return false;
    }

    bool maximizeSupported = false;
    if (!ParseJsValue(jsObject, env, "maximizeSupported", maximizeSupported)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to maximizeSupported");
    }

    bool outlineEnabled = false;
    if (!ParseJsValue(jsObject, env, "outlineEnabled", outlineEnabled)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to outlineEnabled");
    }

    windowOption->SetSubWindowTitle(title);
    windowOption->SetSubWindowDecorEnable(decorEnabled);
    windowOption->SetSubWindowMaximizeSupported(maximizeSupported);
    windowOption->SetSubWindowOutlineEnabled(outlineEnabled);
    if (!ParseRectParam(env, jsObject, windowOption)) {
        return false;
    }
    if (!ParseModalityParam(env, jsObject, windowOption)) {
        return false;
    }
    return ParseZLevelParam(env, jsObject, windowOption);
}

WmErrorCode ParseShowWindowOptions(napi_env env, napi_value showWindowOptions, bool& focusOnShow)
{
    napi_value focusOnShowValue = nullptr;
    napi_get_named_property(env, showWindowOptions, "focusOnShow", &focusOnShowValue);
    if (focusOnShowValue != nullptr) {
        if (GetType(env, focusOnShowValue) == napi_undefined) {
            focusOnShow = true;
            return WmErrorCode::WM_OK;
        }
        if (GetType(env, focusOnShowValue) != napi_boolean || !ConvertFromJsValue(env, focusOnShowValue, focusOnShow)) {
            TLOGE(WmsLogTag::WMS_FOCUS, "failed to convert focusOnShow to boolean");
            return WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
        }
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode MappingWmErrorCodeSafely(WMError err)
{
    auto it = WM_JS_TO_ERROR_CODE_MAP.find(err);
    if (it != WM_JS_TO_ERROR_CODE_MAP.end()) {
        return it->second;
    }
    TLOGW(WmsLogTag::DEFAULT, "[NAPI] Unknown error: %{public}d", static_cast<int32_t>(err));
    return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
}

bool ParseKeyFramePolicy(napi_env env, napi_value jsObject, KeyFramePolicy& keyFramePolicy)
{
    if (jsObject == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "jsObject is null");
        return false;
    }
    bool enable = false;
    if (ParseJsValue(jsObject, env, "enable", enable)) {
        keyFramePolicy.dragResizeType_ = enable ? DragResizeType::RESIZE_KEY_FRAME :
            DragResizeType::RESIZE_TYPE_UNDEFINED;
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to enable");
        return false;
    }
    int32_t data = 0;
    if (ParseJsValueOrGetDefault(jsObject, env, "interval", data,
        static_cast<int32_t>(keyFramePolicy.interval_)) && data > 0) {
        keyFramePolicy.interval_ = static_cast<uint32_t>(data);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to interval");
        return false;
    }
    if (ParseJsValueOrGetDefault(jsObject, env, "distance", data,
        static_cast<int32_t>(keyFramePolicy.distance_)) && data >= 0) {
        keyFramePolicy.distance_ = static_cast<uint32_t>(data);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to distance");
        return false;
    }
    if (ParseJsValueOrGetDefault(jsObject, env, "animationDuration", data,
        static_cast<int32_t>(keyFramePolicy.animationDuration_)) && data >= 0) {
        keyFramePolicy.animationDuration_ = static_cast<uint32_t>(data);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to animationDuration");
        return false;
    }
    if (ParseJsValueOrGetDefault(jsObject, env, "animationDelay", data,
        static_cast<int32_t>(keyFramePolicy.animationDelay_)) && data >= 0) {
        keyFramePolicy.animationDelay_ = static_cast<uint32_t>(data);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to animationDelay");
        return false;
    }
    return true;
}

napi_value ConvertKeyFramePolicyToJsValue(napi_env env, const KeyFramePolicy& keyFramePolicy)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(
        env, objValue, "enable", CreateJsValue(env, keyFramePolicy.enabled()));
    napi_set_named_property(
        env, objValue, "interval", CreateJsValue(env, keyFramePolicy.interval_));
    napi_set_named_property(
        env, objValue, "distance", CreateJsValue(env, keyFramePolicy.distance_));
    napi_set_named_property(
        env, objValue, "animationDuration", CreateJsValue(env, keyFramePolicy.animationDuration_));
    napi_set_named_property(
        env, objValue, "animationDelay", CreateJsValue(env, keyFramePolicy.animationDelay_));
    return objValue;
}

bool CheckPromise(napi_env env, napi_value promiseObj)
{
    if (promiseObj == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "promiseObj is nullptr");
        return false;
    }
    bool isPromise = false;
    napi_is_promise(env, promiseObj, &isPromise);
    return isPromise;
}

napi_value ResolvedCallback(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    void* returnValue = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, &returnValue) != napi_ok) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to get returnValue");
        return nullptr;
    }
    auto* asyncCallback = static_cast<AsyncCallback*>(returnValue);
    if (asyncCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "asyncCallback is nullptr");
        return nullptr;
    }
    if (asyncCallback->resolvedCallback_ != nullptr) {
        asyncCallback->resolvedCallback_(env, argc, argv);
    } else {
        TLOGW(WmsLogTag::WMS_PC, "resolvedCallback is nullptr");
    }
    returnValue = nullptr;
    return nullptr;
}

napi_value RejectedCallback(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    void* returnValue = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, &returnValue) != napi_ok) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to get returnValue");
        return nullptr;
    }
    auto* asyncCallback = static_cast<AsyncCallback*>(returnValue);
    if (asyncCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "asyncCallback is nullptr");
        return nullptr;
    }
    if (asyncCallback->rejectedCallback_ != nullptr) {
        asyncCallback->rejectedCallback_(env, argc, argv);
    } else {
        TLOGW(WmsLogTag::WMS_PC, "rejectedCallback is nullptr");
    }
    returnValue = nullptr;
    return nullptr;
}

bool CallPromise(napi_env env, napi_value promiseObj, AsyncCallback* asyncCallback)
{
    if (!CheckPromise(env, promiseObj)) {
        TLOGE(WmsLogTag::WMS_PC, "promiseObj not is promise");
        return false;
    }

    napi_value promiseThen = nullptr;
    napi_get_named_property(env, promiseObj, "then", &promiseThen);
    if (promiseThen == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "promiseObj then property is nullptr");
        return false;
    }
    if (!NapiIsCallable(env, promiseThen)) {
        TLOGE(WmsLogTag::WMS_PC, "promiseThen not is callable");
        return false;
    }
    napi_value resolvedCallback = nullptr;
    napi_create_function(env, RESOLVED_CALLBACK.c_str(), RESOLVED_CALLBACK.size(),
        ResolvedCallback, asyncCallback, &resolvedCallback);
    napi_value thenArgv[] = { resolvedCallback };
    napi_call_function(env, promiseObj, promiseThen, ArraySize(thenArgv), thenArgv, nullptr);

    napi_value promiseCatch = nullptr;
    napi_get_named_property(env, promiseObj, "catch", &promiseCatch);
    if (promiseCatch == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "promiseObj catch property is nullptr");
        return false;
    }
    if (!NapiIsCallable(env, promiseCatch)) {
        TLOGE(WmsLogTag::WMS_PC, "promiseCatch not is callable");
        return false;
    }
    napi_value rejectedCallback = nullptr;
    napi_create_function(env, REJECTED_CALLBACK.c_str(), REJECTED_CALLBACK.size(),
        RejectedCallback, asyncCallback, &rejectedCallback);
    napi_value catchArgv[] = { rejectedCallback };
    napi_call_function(env, promiseObj, promiseCatch, ArraySize(catchArgv), catchArgv, nullptr);

    return true;
}

bool CheckZIndex(int32_t zIndex)
{
    DefaultSpecificZIndex zIndexEnum = static_cast<DefaultSpecificZIndex>(zIndex);
    return std::find(DefaultSpecificZIndexList.begin(), DefaultSpecificZIndexList.end(), zIndexEnum) !=
        DefaultSpecificZIndexList.end();
}

bool ParseZIndex(napi_env env, napi_value jsObject, WindowOption& option)
{
    if (!WindowHelper::IsDynamicWindow(option.GetWindowType())) {
        return true;
    }
    int32_t zIndex = 0;
    if (!ParseJsValue(jsObject, env, "zIndex", zIndex)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "parse zIndex failed");
        return true;
    }
    if (!CheckZIndex(zIndex)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "check zIndex failed, %{public}d", zIndex);
        return true;
    }
    option.SetZIndex(zIndex);
    return true;
}

napi_value BuildJsRectChangeOptions(napi_env env, const Rect& rect, RectChangeReason reason)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value jsRectChangeOptions = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, jsRectChangeOptions);
    napi_value jsRect = GetRectAndConvertToJsValue(env, rect);
    if (jsRect == nullptr || napi_set_named_property(env, jsRectChangeOptions, "rect", jsRect) != napi_ok) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to set rect");
        return nullptr;
    }
    napi_value jsReason = CreateJsValue(env, reason);
    if (jsReason == nullptr || napi_set_named_property(env, jsRectChangeOptions, "reason", jsReason) != napi_ok) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to set rectChangeReason");
        return nullptr;
    }
    return jsRectChangeOptions;
}

napi_value BuildJsPosition(napi_env env, const Position& position)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value jsPosition = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, jsPosition);
    napi_value jsX = CreateJsValue(env, position.x);
    if (jsX == nullptr || napi_set_named_property(env, jsPosition, "x", jsX) != napi_ok) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to set x pos");
        return nullptr;
    }
    napi_value jsY = CreateJsValue(env, position.y);
    if (jsY == nullptr || napi_set_named_property(env, jsPosition, "y", jsY) != napi_ok) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to set y pos");
        return nullptr;
    }
    return jsPosition;
}

WsNapiAsyncTask::WsNapiAsyncTask(napi_deferred deferred, std::unique_ptr<WsNapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<WsNapiAsyncTask::CompleteCallback>&& complete)
    : deferred_(deferred), execute_(std::move(execute)), complete_(std::move(complete))
{}
 
WsNapiAsyncTask::WsNapiAsyncTask(napi_ref callbackRef, std::unique_ptr<WsNapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<WsNapiAsyncTask::CompleteCallback>&& complete)
    : callbackRef_(callbackRef), execute_(std::move(execute)), complete_(std::move(complete))
{}
 
WsNapiAsyncTask::~WsNapiAsyncTask()
{
    if (work_ && env_) {
        napi_delete_async_work(env_, work_);
        work_ = nullptr;
    }
}
 
void WsNapiAsyncTask::Resolve(napi_env env, napi_value value)
{
    TLOGD(WmsLogTag::DEFAULT, "called");
    napi_status status = napi_ok;
    if (deferred_) {
        status = napi_resolve_deferred(env, deferred_, value);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi resolve deferred exception status:%{public}d", status);
        }
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            CreateJsError(env, 0),
            value,
        };
        napi_value func = nullptr;
        status = napi_get_reference_value(env, callbackRef_, &func);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi get reference value exception status:%{public}d", status);
        }
        status = napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi call function exception status:%{public}d", status);
        }
        status = napi_delete_reference(env, callbackRef_);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi delete reference exception status:%{public}d", status);
        }
        callbackRef_ = nullptr;
    }
}

void WsNapiAsyncTask::Reject(napi_env env, napi_value error)
{
    napi_status status = napi_ok;
    if (deferred_) {
        status = napi_reject_deferred(env, deferred_, error);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi get reference value exception status:%{public}d", status);
        }
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            CreateJsUndefined(env),
        };
        napi_value func = nullptr;
        status = napi_get_reference_value(env, callbackRef_, &func);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi get reference value exception status:%{public}d", status);
        }
        status = napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi call function exception status:%{public}d", status);
        }
        status = napi_delete_reference(env, callbackRef_);
        if (status != napi_ok) {
            TLOGE(WmsLogTag::WMS_LIFE, "napi delete reference exception status:%{public}d", status);
        }
        callbackRef_ = nullptr;
    }
}

std::unique_ptr<WsNapiAsyncTask> CreateEmptyWsNapiAsyncTask(napi_env env,
    napi_value lastParam, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<WsNapiAsyncTask>(nativeDeferred,
            std::unique_ptr<WsNapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<WsNapiAsyncTask::CompleteCallback>());
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<WsNapiAsyncTask>(callbackRef,
            std::unique_ptr<WsNapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<WsNapiAsyncTask::CompleteCallback>());
    }
}
} // namespace Rosen
} // namespace OHOS