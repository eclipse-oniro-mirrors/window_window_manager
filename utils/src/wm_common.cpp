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

#include "wm_common.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
const std::map<WMError, WmErrorCode> WM_JS_TO_ERROR_CODE_MAP {
    {WMError::WM_OK,                                   WmErrorCode::WM_OK                             },
    {WMError::WM_DO_NOTHING,                           WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_DESTROYED_OBJECT,               WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_DEVICE_NOT_SUPPORT,             WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT       },
    {WMError::WM_ERROR_INVALID_OPERATION,              WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_INVALID_PARAM,                  WmErrorCode::WM_ERROR_INVALID_PARAM            },
    {WMError::WM_ERROR_INVALID_PERMISSION,             WmErrorCode::WM_ERROR_NO_PERMISSION            },
    {WMError::WM_ERROR_NOT_SYSTEM_APP,                 WmErrorCode::WM_ERROR_NOT_SYSTEM_APP           },
    {WMError::WM_ERROR_INVALID_TYPE,                   WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_INVALID_WINDOW,                 WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE,    WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_IPC_FAILED,                     WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_NO_MEM,                         WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_NO_REMOTE_ANIMATION,            WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_INVALID_DISPLAY,                WmErrorCode::WM_ERROR_INVALID_DISPLAY          },
    {WMError::WM_ERROR_INVALID_PARENT,                 WmErrorCode::WM_ERROR_INVALID_PARENT           },
    {WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS,       WmErrorCode::WM_ERROR_INVALID_OP_IN_CUR_STATUS },
    {WMError::WM_ERROR_REPEAT_OPERATION,               WmErrorCode::WM_ERROR_REPEAT_OPERATION         },
    {WMError::WM_ERROR_NULLPTR,                        WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_SAMGR,                          WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_START_ABILITY_FAILED,           WmErrorCode::WM_ERROR_START_ABILITY_FAILED     },
    {WMError::WM_ERROR_PIP_DESTROY_FAILED,             WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED       },
    {WMError::WM_ERROR_PIP_STATE_ABNORMALLY,           WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY     },
    {WMError::WM_ERROR_PIP_CREATE_FAILED,              WmErrorCode::WM_ERROR_PIP_CREATE_FAILED        },
    {WMError::WM_ERROR_PIP_INTERNAL_ERROR,             WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR       },
    {WMError::WM_ERROR_PIP_REPEAT_OPERATION,           WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION     },
    {WMError::WM_ERROR_INVALID_CALLING,                WmErrorCode::WM_ERROR_INVALID_CALLING          },
    {WMError::WM_ERROR_INVALID_SESSION,                WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_SYSTEM_ABNORMALLY,              WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_TIMEOUT,                        WmErrorCode::WM_ERROR_TIMEOUT                  },
    {WMError::WM_ERROR_ILLEGAL_PARAM,                  WmErrorCode::WM_ERROR_ILLEGAL_PARAM            },
    {WMError::WM_ERROR_FB_PARAM_INVALID,               WmErrorCode::WM_ERROR_FB_PARAM_INVALID         },
    {WMError::WM_ERROR_FB_CREATE_FAILED,               WmErrorCode::WM_ERROR_FB_CREATE_FAILED         },
    {WMError::WM_ERROR_FB_REPEAT_CONTROLLER,           WmErrorCode::WM_ERROR_FB_REPEAT_CONTROLLER     },
    {WMError::WM_ERROR_FB_REPEAT_OPERATION,            WmErrorCode::WM_ERROR_FB_REPEAT_OPERATION      },
    {WMError::WM_ERROR_FB_INTERNAL_ERROR,              WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR        },
    {WMError::WM_ERROR_FB_STATE_ABNORMALLY,            WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY      },
    {WMError::WM_ERROR_FB_INVALID_STATE,               WmErrorCode::WM_ERROR_FB_INVALID_STATE         },
    {WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED,  WmErrorCode::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED  },
    {WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED, WmErrorCode::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED  },
    {WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,  WmErrorCode::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED  },
    {WMError::WM_ERROR_UI_EFFECT_ERROR,                WmErrorCode::WM_ERROR_UI_EFFECT_ERROR          },
    {WMError::WM_ERROR_INVALID_WINDOW_TYPE,            WmErrorCode::WM_ERROR_INVALID_WINDOW_TYPE      },
    {WMError::WM_ERROR_FV_REPEAT_OPERATION,            WmErrorCode::WM_ERROR_FV_REPEAT_OPERATION  },
    {WMError::WM_ERROR_FV_INVALID_STATE,               WmErrorCode::WM_ERROR_FV_INVALID_STATE  },
    {WMError::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED,  WmErrorCode::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED  },
    {WMError::WM_ERROR_FV_START_FAILED,                WmErrorCode::WM_ERROR_FV_START_FAILED          },
    {WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS,     WmErrorCode::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS      },
};

WmErrorCode ConvertErrorToCode(WMError error, WmErrorCode defaultCode)
{
    auto iter = WM_JS_TO_ERROR_CODE_MAP.find(error);
    if (iter != WM_JS_TO_ERROR_CODE_MAP.end()) {
        return iter->second;
    }
    TLOGE(WmsLogTag::DEFAULT,
        "No mapping found for WMError: %{public}d, fallback to default WmErrorCode: %{public}d",
        static_cast<int32_t>(error), static_cast<int32_t>(defaultCode));
    return defaultCode;
}
}