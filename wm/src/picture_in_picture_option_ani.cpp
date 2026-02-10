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

#include "picture_in_picture_option_ani.h"

#include "js_runtime_utils.h"

#include "ani.h"

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
PipOptionAni::PipOptionAni()
{
}

void PipOptionAni::ClearAniRefs(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (customNodeController_) {
        env->GlobalReference_Delete(customNodeController_);
        customNodeController_ = nullptr;
    }
    if (typeNode_) {
        env->GlobalReference_Delete(typeNode_);
        typeNode_ = nullptr;
    }
    if (storage_) {
        env->GlobalReference_Delete(storage_);
        storage_ = nullptr;
    }
}

void PipOptionAni::SetNodeControllerRef(ani_ref ref)
{
    if (ref) {
        TLOGI(WmsLogTag::WMS_PIP, "SetNodeControllerRef");
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "NodeControllerRef is null, not set");
    }
    customNodeController_ = ref;
}

ani_ref PipOptionAni::GetNodeControllerRef() const
{
    return customNodeController_;
}

void PipOptionAni::SetTypeNodeRef(ani_ref ref)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    typeNode_ = ref;
}

ani_ref PipOptionAni::GetTypeNodeRef() const
{
    return typeNode_;
}

void PipOptionAni::SetStorageRef(ani_ref ref)
{
    if (ref) {
        TLOGI(WmsLogTag::WMS_PIP, "SetStorageRef");
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "SetStorageRef is null");
    }
    storage_ = ref;
}

ani_ref PipOptionAni::GetStorageRef() const
{
    return storage_;
}

void PipOptionAni::RegisterPipContentListenerWithType(const std::string& type,
    ani_ref updateNodeCallbackRef)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    pipContentlistenerMap_[type] = updateNodeCallbackRef;
}

void PipOptionAni::UnRegisterPipContentListenerWithType(const std::string& type)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    pipContentlistenerMap_.erase(type);
}

ani_ref PipOptionAni::GetANIPipContentCallbackRef(const std::string& type)
{
    auto iter = pipContentlistenerMap_.find(type);
    if (iter == pipContentlistenerMap_.end()) {
        return nullptr;
    }
    return iter->second;
}
} // namespace Rosen
} // namespace OHOS