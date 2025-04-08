/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "js_runtime_utils.h"
#include "picture_in_picture_option.h"

namespace OHOS {
namespace Rosen {
PipOption::PipOption()
{
}

void PipOption::ClearNapiRefs(napi_env env)
{
    if (customNodeController_) {
        napi_delete_reference(env, customNodeController_);
        customNodeController_ = nullptr;
    }
    if (typeNode_) {
        napi_delete_reference(env, typeNode_);
        typeNode_ = nullptr;
    }
    if (storage_) {
        napi_delete_reference(env, storage_);
        storage_ = nullptr;
    }
}

void PipOption::SetContext(void* contextPtr)
{
    contextPtr_ = contextPtr;
}

void PipOption::SetNavigationId(const std::string& navigationId)
{
    navigationId_ = navigationId;
}

void PipOption::SetPipTemplate(uint32_t templateType)
{
    templateType_ = templateType;
}

void PipOption::SetPiPControlStatus(PiPControlType controlType, PiPControlStatus status)
{
    for (auto& controlStatusInfo : pipControlStatusInfoList_) {
        if (controlType == controlStatusInfo.controlType) {
            controlStatusInfo.status = status;
            return;
        }
    }
    PiPControlStatusInfo newPiPControlStatusInfo {controlType, status};
    pipControlStatusInfoList_.push_back(newPiPControlStatusInfo);
}

void PipOption::SetPiPControlEnabled(PiPControlType controlType, PiPControlStatus enabled)
{
    for (auto& controlEnableInfo : pipControlEnableInfoList_) {
        if (controlType == controlEnableInfo.controlType) {
            controlEnableInfo.enabled = enabled;
            return;
        }
    }
    PiPControlEnableInfo newPiPControlEnableInfo {controlType, enabled};
    pipControlEnableInfoList_.push_back(newPiPControlEnableInfo);
}

void PipOption::SetContentSize(uint32_t width, uint32_t height)
{
    contentWidth_ = width;
    contentHeight_ = height;
}

void PipOption::SetControlGroup(std::vector<std::uint32_t> controlGroup)
{
    controlGroup_ = controlGroup;
}

void PipOption::SetNodeControllerRef(napi_ref ref)
{
    customNodeController_ = ref;
}

void PipOption::SetStorageRef(napi_ref ref)
{
    storage_ = ref;
}
 
napi_ref PipOption::GetNodeControllerRef() const
{
    return customNodeController_;
}

napi_ref PipOption::GetStorageRef() const
{
    return storage_;
}
 
void PipOption::SetTypeNodeRef(napi_ref ref)
{
    typeNode_ = ref;
}

napi_ref PipOption::GetTypeNodeRef() const
{
    return typeNode_;
}

void PipOption::RegisterPipContentListenerWithType(const std::string& type,
    std::shared_ptr<NativeReference> updateNodeCallbackRef)
{
    pipContentlistenerMap_[type] = updateNodeCallbackRef;
}

void PipOption::UnRegisterPipContentListenerWithType(const std::string& type)
{
    pipContentlistenerMap_.erase(type);
}

std::shared_ptr<NativeReference> PipOption::GetPipContentCallbackRef(const std::string& type)
{
    auto iter = pipContentlistenerMap_.find(type);
    if (iter == pipContentlistenerMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

void* PipOption::GetContext() const
{
    return contextPtr_;
}

std::string PipOption::GetNavigationId() const
{
    return navigationId_;
}

uint32_t PipOption::GetPipTemplate()
{
    return templateType_;
}

void PipOption::GetContentSize(uint32_t& width, uint32_t& height)
{
    width = contentWidth_;
    height = contentHeight_;
}

std::vector<std::uint32_t> PipOption::GetControlGroup()
{
    return controlGroup_;
}

std::vector<PiPControlStatusInfo> PipOption::GetControlStatus()
{
    return pipControlStatusInfoList_;
}

std::vector<PiPControlEnableInfo> PipOption::GetControlEnable()
{
    return pipControlEnableInfoList_;
}

void PipOption::SetXComponentController(std::shared_ptr<XComponentController> xComponentController)
{
    xComponentController_ = xComponentController;
}

std::shared_ptr<XComponentController> PipOption::GetXComponentController()
{
    return xComponentController_;
}

void PipOption::SetTypeNodeEnabled(bool enable)
{
    useTypeNode_ = enable;
}

bool PipOption::IsTypeNodeEnabled() const
{
    return useTypeNode_;
}
} // namespace Rosen
} // namespace OHOS