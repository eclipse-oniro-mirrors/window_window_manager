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

#ifndef OHOS_WINDOW_UTILS_H
#define OHOS_WINDOW_UTILS_H

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include "wm_common.h"

#define FFI_EXPORT __attribute__((visibility("default")))

struct SystemBarPropertyFlag {
    bool enableFlag;
    bool backgroundColorFlag;
    bool contentColorFlag;
    SystemBarPropertyFlag() : enableFlag(false),
        backgroundColorFlag(false), contentColorFlag(false) {}
};

constexpr int32_t RGB_LENGTH = 6;

extern "C" {
struct RetStruct {
    int32_t code;
    int64_t len;
    void* data;
};

struct CRect {
    int32_t posX;
    int32_t posY;
    uint32_t width;
    uint32_t height;
};

struct CWindowProperties {
    CRect windowRect;
    CRect drawableRect;
    uint32_t type;
    bool isFullScreen;
    bool isLayoutFullScreen;
    bool focusable;
    bool touchable;
    float brightness;
    bool isKeepScreenOn;
    bool isPrivacyMode;
    bool isRoundCorner;
    bool isTransparent;
    uint32_t id;
};

struct CSubWindowOptions {
    std::string title;
    bool decorEnabled;
    bool isModal;
};

enum class SpecificSystemBar {
    STATUS = 0,
    NAVIGATION = 1,
    NAVIGATION_INDICATOR = 2
};
}

struct WindowRotate {
    double x;
    double y;
    double z;
    double pivotX;
    double pivotY;
};

struct CJBarProperties {
    const char* statusBarColor;
    bool isStatusBarLightIcon;
    const char* statusBarContentColor;
    const char* navigationBarColor;
    bool isNavigationBarLightIcon;
    const char* navigationBarContentColor;
    bool enableStatusBarAnimation;
    bool enableNavigationBarAnimation;

    ~CJBarProperties()
    {
        if (statusBarColor != nullptr) {
            delete[] statusBarColor;
        }
        if (statusBarContentColor != nullptr) {
            delete[] statusBarContentColor;
        }
        if (navigationBarColor != nullptr) {
            delete[] navigationBarColor;
        }
        if (navigationBarContentColor != nullptr) {
            delete[] navigationBarContentColor;
        }
    }

    CJBarProperties(const CJBarProperties&) = delete;
    CJBarProperties& operator=(const CJBarProperties&) = delete;
};


namespace OHOS {
namespace Rosen {
const int32_t WINDOW_SUCCESS = 0;

enum class LifeCycleEventType : uint32_t {
    FOREGROUND = 1,
    ACTIVE,
    INACTIVE,
    BACKGROUND,
    RESUMED,
    PAUSED,
    DESTROYED,
};

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
    TYPE_END
};

struct CjRectInfo {
    int32_t left;
    int32_t top;
    uint32_t width;
    uint32_t height;
};

struct SystemBarRegionTintInfo {
    uint32_t type;
    bool isEnable;
    uint32_t backgroundColor;
    uint32_t contentColor;
    CjRectInfo region;
};

enum class RectChangeReason : uint32_t {
    UNDEFINED = 0,
    MAXIMIZE,
    RECOVER,
    MOVE,
    DRAG,
    DRAG_START,
    DRAG_END,
};

const std::map<ApiWindowType, WindowType> CJ_TO_NATIVE_WINDOW_TYPE_MAP {
    { ApiWindowType::TYPE_APP,                 WindowType::WINDOW_TYPE_APP_SUB_WINDOW      },
    { ApiWindowType::TYPE_DIALOG,              WindowType::WINDOW_TYPE_DIALOG              },
    { ApiWindowType::TYPE_SYSTEM_ALERT,        WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW },
    { ApiWindowType::TYPE_INPUT_METHOD,        WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT  },
    { ApiWindowType::TYPE_STATUS_BAR,          WindowType::WINDOW_TYPE_STATUS_BAR          },
    { ApiWindowType::TYPE_PANEL,               WindowType::WINDOW_TYPE_PANEL               },
    { ApiWindowType::TYPE_KEYGUARD,            WindowType::WINDOW_TYPE_KEYGUARD            },
    { ApiWindowType::TYPE_VOLUME_OVERLAY,      WindowType::WINDOW_TYPE_VOLUME_OVERLAY      },
    { ApiWindowType::TYPE_NAVIGATION_BAR,      WindowType::WINDOW_TYPE_NAVIGATION_BAR      },
    { ApiWindowType::TYPE_FLOAT,               WindowType::WINDOW_TYPE_FLOAT               },
    { ApiWindowType::TYPE_FLOAT_CAMERA,        WindowType::WINDOW_TYPE_FLOAT_CAMERA        },
    { ApiWindowType::TYPE_WALLPAPER,           WindowType::WINDOW_TYPE_WALLPAPER           },
    { ApiWindowType::TYPE_DESKTOP,             WindowType::WINDOW_TYPE_DESKTOP             },
    { ApiWindowType::TYPE_LAUNCHER_RECENT,     WindowType::WINDOW_TYPE_LAUNCHER_RECENT     },
    { ApiWindowType::TYPE_LAUNCHER_DOCK,       WindowType::WINDOW_TYPE_LAUNCHER_DOCK       },
    { ApiWindowType::TYPE_VOICE_INTERACTION,   WindowType::WINDOW_TYPE_VOICE_INTERACTION   },
    { ApiWindowType::TYPE_POINTER,             WindowType::WINDOW_TYPE_POINTER             },
    { ApiWindowType::TYPE_SCREENSHOT,          WindowType::WINDOW_TYPE_SCREENSHOT          },
    { ApiWindowType::TYPE_SYSTEM_TOAST,        WindowType::WINDOW_TYPE_SYSTEM_TOAST        },
    { ApiWindowType::TYPE_DIVIDER,             WindowType::WINDOW_TYPE_DOCK_SLICE          },
    { ApiWindowType::TYPE_GLOBAL_SEARCH,       WindowType::WINDOW_TYPE_GLOBAL_SEARCH       },
};

enum class ApiWindowMode : uint32_t {
    UNDEFINED = 1,
    FULLSCREEN,
    PRIMARY,
    SECONDARY,
    FLOATING,
    MODE_END = FLOATING
};

const std::map<ApiWindowMode, WindowMode> CJ_TO_NATIVE_WINDOW_MODE_MAP {
    {ApiWindowMode::UNDEFINED,  WindowMode::WINDOW_MODE_UNDEFINED       },
    {ApiWindowMode::FULLSCREEN, WindowMode::WINDOW_MODE_FULLSCREEN      },
    {ApiWindowMode::PRIMARY,    WindowMode::WINDOW_MODE_SPLIT_PRIMARY   },
    {ApiWindowMode::SECONDARY,  WindowMode::WINDOW_MODE_SPLIT_SECONDARY },
    {ApiWindowMode::FLOATING,   WindowMode::WINDOW_MODE_FLOATING        },
};

const std::map<WindowType, ApiWindowType> CJ_TO_WINDOW_TYPE_MAP {
    { WindowType::WINDOW_TYPE_APP_SUB_WINDOW,      ApiWindowType::TYPE_APP               },
    { WindowType::WINDOW_TYPE_DIALOG,              ApiWindowType::TYPE_DIALOG            },
    { WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW, ApiWindowType::TYPE_SYSTEM_ALERT      },
    { WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT,  ApiWindowType::TYPE_INPUT_METHOD      },
    { WindowType::WINDOW_TYPE_STATUS_BAR,          ApiWindowType::TYPE_STATUS_BAR        },
    { WindowType::WINDOW_TYPE_PANEL,               ApiWindowType::TYPE_PANEL             },
    { WindowType::WINDOW_TYPE_KEYGUARD,            ApiWindowType::TYPE_KEYGUARD          },
    { WindowType::WINDOW_TYPE_VOLUME_OVERLAY,      ApiWindowType::TYPE_VOLUME_OVERLAY    },
    { WindowType::WINDOW_TYPE_NAVIGATION_BAR,      ApiWindowType::TYPE_NAVIGATION_BAR    },
    { WindowType::WINDOW_TYPE_FLOAT,               ApiWindowType::TYPE_FLOAT             },
    { WindowType::WINDOW_TYPE_FLOAT_CAMERA,        ApiWindowType::TYPE_FLOAT_CAMERA      },
    { WindowType::WINDOW_TYPE_WALLPAPER,           ApiWindowType::TYPE_WALLPAPER         },
    { WindowType::WINDOW_TYPE_DESKTOP,             ApiWindowType::TYPE_DESKTOP           },
    { WindowType::WINDOW_TYPE_LAUNCHER_RECENT,     ApiWindowType::TYPE_LAUNCHER_RECENT   },
    { WindowType::WINDOW_TYPE_LAUNCHER_DOCK,       ApiWindowType::TYPE_LAUNCHER_DOCK     },
    { WindowType::WINDOW_TYPE_VOICE_INTERACTION,   ApiWindowType::TYPE_VOICE_INTERACTION },
    { WindowType::WINDOW_TYPE_POINTER,             ApiWindowType::TYPE_POINTER           },
    { WindowType::WINDOW_TYPE_SCREENSHOT,          ApiWindowType::TYPE_SCREENSHOT        },
    { WindowType::WINDOW_TYPE_SYSTEM_TOAST,        ApiWindowType::TYPE_SYSTEM_TOAST      },
    { WindowType::WINDOW_TYPE_DOCK_SLICE,          ApiWindowType::TYPE_DIVIDER           },
    { WindowType::WINDOW_TYPE_GLOBAL_SEARCH,       ApiWindowType::TYPE_GLOBAL_SEARCH     },
};

const std::map<WindowSizeChangeReason, RectChangeReason> CJ_SIZE_CHANGE_REASON {
    { WindowSizeChangeReason::UNDEFINED,             RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::MAXIMIZE,              RectChangeReason::MAXIMIZE   },
    { WindowSizeChangeReason::RECOVER,               RectChangeReason::RECOVER    },
    { WindowSizeChangeReason::ROTATION,              RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::DRAG,                  RectChangeReason::DRAG       },
    { WindowSizeChangeReason::DRAG_START,            RectChangeReason::DRAG_START },
    { WindowSizeChangeReason::DRAG_END,              RectChangeReason::DRAG_END   },
    { WindowSizeChangeReason::RESIZE,                RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::MOVE,                  RectChangeReason::MOVE       },
    { WindowSizeChangeReason::HIDE,                  RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::TRANSFORM,             RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::CUSTOM_ANIMATION_SHOW, RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::FULL_TO_SPLIT,         RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::SPLIT_TO_FULL,         RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::FULL_TO_FLOATING,      RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::FLOATING_TO_FULL,      RectChangeReason::UNDEFINED  },
    { WindowSizeChangeReason::END,                   RectChangeReason::UNDEFINED  },
};
}
}

#endif
