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
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
constexpr uint32_t PIP_LOW_PRIORITY = 0;
constexpr uint32_t PIP_HIGH_PRIORITY = 1;
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

void PipOption::SetHandleId(const int32_t handleId)
{
    handleId_ = handleId;
}

void PipOption::SetPipTemplate(uint32_t templateType)
{
    templateType_ = templateType;
}

void PipOption::SetDefaultWindowSizeType(uint32_t defaultWindowSizeType)
{
    defaultWindowSizeType_ = defaultWindowSizeType;
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

// LCOV_EXCL_START
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

void PipOption::SetCornerAdsorptionEnabled(bool cornerAdsorptionEnabled)
{
    cornerAdsorptionEnabled_ = cornerAdsorptionEnabled;
}
 
bool PipOption::GetCornerAdsorptionEnabled() const
{
    return cornerAdsorptionEnabled_;
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

void* PipOption::GetContext() const
{
    return contextPtr_;
}
// LCOV_EXCL_STOP

std::string PipOption::GetNavigationId() const
{
    return navigationId_;
}

int32_t PipOption::GetHandleId() const
{
    return handleId_;
}

uint32_t PipOption::GetPipTemplate()
{
    return templateType_;
}

uint32_t PipOption::GetDefaultWindowSizeType() const
{
    return defaultWindowSizeType_;
}

void PipOption::GetContentSize(uint32_t& width, uint32_t& height)
{
    width = contentWidth_;
    height = contentHeight_;
}

// LCOV_EXCL_START
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
// LCOV_EXCL_STOP

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

uint32_t PipOption::GetPipPriority(uint32_t pipTemplateType) const
{
    if (pipTemplateType >= static_cast<uint32_t>(PiPTemplateType::END)) {
        TLOGE(WmsLogTag::WMS_PIP, "param invalid, pipTemplateType is %{public}d", pipTemplateType);
        return PIP_LOW_PRIORITY;
    }
    if (pipTemplateType == static_cast<uint32_t>(PiPTemplateType::VIDEO_PLAY) ||
        pipTemplateType == static_cast<uint32_t>(PiPTemplateType::VIDEO_LIVE)) {
        return PIP_LOW_PRIORITY;
    } else {
        return PIP_HIGH_PRIORITY;
    }
}

// LCOV_EXCL_START
void PipOption::GetPiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo)
{
    pipTemplateInfo.pipTemplateType = templateType_;
    pipTemplateInfo.controlGroup = controlGroup_;
    pipTemplateInfo.priority = GetPipPriority(templateType_);
    pipTemplateInfo.defaultWindowSizeType = defaultWindowSizeType_;
    pipTemplateInfo.pipControlStatusInfoList = pipControlStatusInfoList_;
    pipTemplateInfo.pipControlEnableInfoList = pipControlEnableInfoList_;
    pipTemplateInfo.cornerAdsorptionEnabled = cornerAdsorptionEnabled_;
}
// LCOV_EXCL_STOP
} // namespace Rosen
} // namespace OHOS