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

#include "../../interfaces/include/ws_common.h"

namespace OHOS::Rosen {

const std::map<WSError, WSErrorCode> WS_JS_TO_ERROR_CODE_MAP {
    { WSError::WS_OK,                    WSErrorCode::WS_OK },
    { WSError::WS_DO_NOTHING,            WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
    { WSError::WS_ERROR_INVALID_SESSION, WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
    { WSError::WS_ERROR_INVALID_PARAM, WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
    { WSError::WS_ERROR_IPC_FAILED,      WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY },
    { WSError::WS_ERROR_NULLPTR,         WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
    { WSError::WS_ERROR_EDM_CONTROLLED,  WSErrorCode::WS_ERROR_EDM_CONTROLLED },
    { WSError::WS_ERROR_INVALID_WINDOW,  WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
};
}