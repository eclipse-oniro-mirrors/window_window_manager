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

#ifndef OHOS_PIP_OPTION_H
#define OHOS_PIP_OPTION_H
#include <refbase.h>
#include <string>
#include "napi/native_api.h"
#include "wm_common.h"
#include "xcomponent_controller.h"

namespace OHOS {
namespace Rosen {
using namespace Ace;
class PipOption : virtual public RefBase {
public:
    explicit PipOption();
    virtual ~PipOption() = default;
    void ClearNapiRefs(napi_env env);
    void SetContext(void* contextPtr);
    void SetNavigationId(const std::string& navigationId);
    void SetPipTemplate(uint32_t templateType);
    void SetDefaultWindowSizeType(uint32_t defaultWindowSizeType);
    void SetContentSize(uint32_t width, uint32_t height);
    void SetPiPControlStatus(PiPControlType controlType, PiPControlStatus status);
    void SetPiPControlEnabled(PiPControlType controlType, PiPControlStatus enabled);
    void SetXComponentController(std::shared_ptr<XComponentController> xComponentController);
    void SetControlGroup(std::vector<std::uint32_t> controlGroup);
    void SetCornerAdsorptionEnabled(bool cornerAdsorptionEnabled);
    void* GetContext() const;
    std::string GetNavigationId() const;
    uint32_t GetPipTemplate();
    uint32_t GetDefaultWindowSizeType() const;
    std::vector<std::uint32_t> GetControlGroup();
    std::vector<PiPControlStatusInfo> GetControlStatus();
    std::vector<PiPControlEnableInfo> GetControlEnable();
    void GetContentSize(uint32_t& width, uint32_t& height);
    std::shared_ptr<XComponentController> GetXComponentController();
    bool GetCornerAdsorptionEnabled() const;
    void SetNodeControllerRef(napi_ref ref);
    napi_ref GetNodeControllerRef() const;
    void SetTypeNodeRef(napi_ref ref);
    napi_ref GetTypeNodeRef() const;
    void SetStorageRef(napi_ref ref);
    napi_ref GetStorageRef() const;
    void SetTypeNodeEnabled(bool enable);
    bool IsTypeNodeEnabled() const;
    uint32_t GetPipPriority(uint32_t pipTemplateType) const;
    void GetPiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo);
    void SetHandleId(const int32_t handleId);
    int32_t GetHandleId() const;

private:
    void* contextPtr_ = nullptr;
    uint32_t templateType_  = 0;
    std::string navigationId_ = "";
    uint32_t contentWidth_ = 0;
    uint32_t contentHeight_ = 0;
    uint32_t defaultWindowSizeType_ = 0;
    std::vector<PiPControlStatusInfo> pipControlStatusInfoList_;
    std::vector<PiPControlEnableInfo> pipControlEnableInfoList_;
    std::vector<std::uint32_t> controlGroup_;
    std::shared_ptr<XComponentController> xComponentController_ = nullptr;
    napi_ref customNodeController_ = nullptr;
    napi_ref typeNode_ = nullptr;
    bool useTypeNode_ = false;
    bool cornerAdsorptionEnabled_ = true;
    napi_ref storage_ = nullptr;
    int32_t handleId_ = -1;
};
}
}
#endif //OHOS_PIP_OPTION_H
