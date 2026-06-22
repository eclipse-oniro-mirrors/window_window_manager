/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_PICTURE_IN_PICTURE_OPTION_ANI_H
#define OHOS_PICTURE_IN_PICTURE_OPTION_ANI_H

#include "ani.h"

#include "picture_in_picture_option.h"

namespace OHOS {
namespace Rosen {
using namespace Ace;
class PipOptionAni : public PipOption {
public:
    explicit PipOptionAni();
    virtual ~PipOptionAni() = default;

    // new method with new ani type
    void ClearAniRefs(ani_env* env);
    
    // old methods with new ani type
    void SetNodeControllerRef(ani_ref ref);
    ani_ref GetNodeControllerRef() const;
    void SetTypeNodeRef(ani_ref ref);
    ani_ref GetTypeNodeRef() const;
    void SetStorageRef(ani_ref ref);
    ani_ref GetStorageRef() const;
    void RegisterPipContentListenerWithType(const std::string&, ani_ref updateNodeCallbackRef);
    void UnRegisterPipContentListenerWithType(const std::string&);
    ani_ref GetANIPipContentCallbackRef(const std::string& type);

private:
    std::map<std::string, ani_ref> pipContentlistenerMap_;
    ani_ref customNodeController_ = nullptr;
    ani_ref typeNode_ = nullptr;
    ani_ref storage_ = nullptr;
};
}  // Rosen
}  // OHOS
#endif // OHOS_PICTURE_IN_PICTURE_OPTION_ANI_H