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

#include "dm_common.h"

namespace OHOS::Rosen {
namespace {
const std::string DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING =
    "[wired casting]not support coordination when wired casting.";
const std::string DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_WIRELESS_CASTING =
    "[wireless casting]not support coordination when wireless casting.";
const std::string DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_RECORDING =
    "[recording]not support coordination when recording.";
const std::string DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_TENTMODE =
    "[tentmode]not support coordination when tentmode.";
}

const std::map<DMError, DmErrorCode> DM_JS_TO_ERROR_CODE_MAP {
    {DMError::DM_OK,                                             DmErrorCode::DM_OK                              },
    {DMError::DM_ERROR_INVALID_PERMISSION,                       DmErrorCode::DM_ERROR_NO_PERMISSION             },
    {DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED,                    DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_IPC_FAILED,                               DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_REMOTE_CREATE_FAILED,                     DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_NULLPTR,                                  DmErrorCode::DM_ERROR_INVALID_SCREEN            },
    {DMError::DM_ERROR_INVALID_PARAM,                            DmErrorCode::DM_ERROR_INVALID_PARAM             },
    {DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED,             DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_DEATH_RECIPIENT,                          DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_INVALID_MODE_ID,                          DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_WRITE_DATA_FAILED,                        DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_RENDER_SERVICE_FAILED,                    DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_UNREGISTER_AGENT_FAILED,                  DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_INVALID_CALLING,                          DmErrorCode::DM_ERROR_INVALID_CALLING           },
    {DMError::DM_ERROR_NOT_SYSTEM_APP,                           DmErrorCode::DM_ERROR_NOT_SYSTEM_APP            },
    {DMError::DM_ERROR_UNKNOWN,                                  DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_DEVICE_NOT_SUPPORT,                       DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT        },
    {DMError::DM_ERROR_ILLEGAL_PARAM,                            DmErrorCode::DM_ERROR_ILLEGAL_PARAM             },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING,      DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRLESS_CASTING,    DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_RECORDING,          DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_TENTMODE,           DmErrorCode::DM_ERROR_SYSTEM_INNORMAL           },
};

const std::map<DMError, std::string> DM_ERROR_JS_TO_ERROR_MESSAGE_MAP {
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING,      DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING   },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRLESS_CASTING,    DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_WIRELESS_CASTING },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_RECORDING,          DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_RECORDING       },
    {DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_TENTMODE,           DM_ERROR_MSG_NOT_SUPPORT_COOR_WHEN_TENTMODE        },
};

const std::map<DisplayOrientation, WindowOrientation> DISPLAY_TO_WINDOW_MAP{
    { DisplayOrientation::PORTRAIT, WindowOrientation::PORTRAIT },
    { DisplayOrientation::LANDSCAPE, WindowOrientation::LANDSCAPE_INVERTED },
    { DisplayOrientation::PORTRAIT_INVERTED, WindowOrientation::PORTRAIT_INVERTED },
    { DisplayOrientation::LANDSCAPE_INVERTED, WindowOrientation::LANDSCAPE },
};

const std::map<WindowOrientation, DisplayOrientation> WINDOW_TO_DISPLAY_MAP{
    { WindowOrientation::PORTRAIT, DisplayOrientation::PORTRAIT },
    { WindowOrientation::LANDSCAPE, DisplayOrientation::LANDSCAPE_INVERTED },
    { WindowOrientation::PORTRAIT_INVERTED, DisplayOrientation::PORTRAIT_INVERTED },
    { WindowOrientation::LANDSCAPE_INVERTED, DisplayOrientation::LANDSCAPE },
};
}
