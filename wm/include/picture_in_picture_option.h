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
#include "js_runtime_utils.h"
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

    void SetContext(void* contextPtr);
    void SetNavigationId(const std::string& navigationId);
    void SetPipTemplate(uint32_t templateType);
    void SetDefaultWindowSizeType(uint32_t defaultWindowSizeType);
    void SetContentSize(uint32_t width, uint32_t height);
    void SetPiPControlStatus(PiPControlType controlType, PiPControlStatus status);
    void SetPiPControlEnabled(PiPControlType controlType, PiPControlStatus enabled);
    void SetXComponentController(std::shared_ptr<XComponentController> xComponentController);
    void SetCornerAdsorptionEnabled(bool cornerAdsorptionEnabled);
    void RegisterPipContentListenerWithType(const std::string&, std::shared_ptr<NativeReference> updateNodeCallbackRef);
    void UnRegisterPipContentListenerWithType(const std::string&);
    void SetControlGroup(std::vector<std::uint32_t> controlGroup);
    void* GetContext() const;
    std::string GetNavigationId() const;
    uint32_t GetPipTemplate();
    uint32_t GetDefaultWindowSizeType() const;
    std::vector<std::uint32_t> GetControlGroup();
    std::vector<PiPControlStatusInfo> GetControlStatus();
    std::vector<PiPControlEnableInfo> GetControlEnable();
    void GetContentSize(uint32_t& width, uint32_t& height);
    std::shared_ptr<XComponentController> GetXComponentController();
    bool GetCornerAdsorptionEnabled();
    std::shared_ptr<NativeReference> GetPipContentCallbackRef(const std::string&);
    void SetNodeControllerRef(std::shared_ptr<NativeReference> ref);
    std::shared_ptr<NativeReference> GetNodeControllerRef() const;
    void SetTypeNodeRef(std::shared_ptr<NativeReference> ref);
    std::shared_ptr<NativeReference> GetTypeNodeRef() const;
    void SetStorageRef(std::shared_ptr<NativeReference> ref);
    std::shared_ptr<NativeReference> GetStorageRef() const;
    void SetTypeNodeEnabled(bool enable);
    bool IsTypeNodeEnabled() const;
    uint32_t GetPipPriority(uint32_t pipTemplateType) const;
    void GetPiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo);
    void SetHandleId(const int32_t handleId);
    int32_t GetHandleId() const;

protected:
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
    bool useTypeNode_ = false;
    int32_t handleId_ = -1;
    bool cornerAdsorptionEnabled_ = true;

private:
    std::map<std::string, std::shared_ptr<NativeReference>> pipContentlistenerMap_;
    std::shared_ptr<NativeReference> customNodeController_ = nullptr;
    std::shared_ptr<NativeReference> typeNode_ = nullptr;
    std::shared_ptr<NativeReference> storage_ = nullptr;
};
}
}
#endif // OHOS_PIP_OPTION_H
