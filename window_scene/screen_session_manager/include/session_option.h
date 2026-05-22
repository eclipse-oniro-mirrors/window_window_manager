/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SESSION_OPTION_H
#define OHOS_ROSEN_SESSION_OPTION_H

#include <map>
#include <string>
#include <unordered_map>

#include "dm_common.h"
#include "iremote_object.h"

namespace OHOS::Rosen {

struct SessionOption {
    ScreenId rsId_;
    std::string name_;
    bool isExtend_;
    std::string innerName_;
    ScreenId screenId_;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap_;
    bool supportsFocus_ {true};
    bool isRotationLocked_;
    int32_t rotation_;
    std::map<int32_t, int32_t> rotationOrientationMap_;
    bool isBooting_ { false };
    sptr<IRemoteObject> connectToRenderToken_ = nullptr;
};

}
#endif // OHOS_ROSEN_SESSION_OPTION_H