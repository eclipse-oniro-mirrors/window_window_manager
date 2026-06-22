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

#ifndef RENDER_SERVICE_CLIENT_CORE_UI_RS_DISPLAY_NODE_H
#define RENDER_SERVICE_CLIENT_CORE_UI_RS_DISPLAY_NODE_H

#include "ui/rs_node.h"
#include "screen_manager/screen_types.h"
#include "common/rs_common_def.h"

namespace OHOS {
namespace Rosen {

class RSDisplayNode : public RSNode {
public:
    using WeakPtr = std::weak_ptr<RSDisplayNode>;
    using SharedPtr = std::shared_ptr<RSDisplayNode>;

    static SharedPtr Create(const RSDisplayNodeConfig& displayNodeConfig,
                            std::shared_ptr<RSUIContext> rsUIContext = nullptr)
    {
        SharedPtr ret = std::make_shared<RSDisplayNode>();
        ret->displayNodeConfig_ = displayNodeConfig;
        return ret;
    }
    void AddDisplayNodeToTree() {}
    void RemoveDisplayNodeFromTree() {}
    void SetScreenId(uint64_t screenId) {}
    void SetSecurityDisplay(bool isSecurityDisplay) {}
    void SetScreenRotation(const uint32_t& rotation) {}
    void SetDisplayNodeMirrorConfig(const RSDisplayNodeConfig& displayNodeConfig)
    {
        displayNodeConfig_ = displayNodeConfig;
    }
    void SetForceCloseHdr(bool isForceCloseHdr) {}

    bool IsMirrorDisplay()
    {
        return true;
    }

    void SetBootAnimation(bool isBootAnimation) {}

    bool GetBootAnimation()
    {
        return false;
    }

    void ClearModifierByPid(pid_t pid) {}

    void SetVirtualScreenMuteStatus(bool virtualScreenMuteStatus) {}

    bool Marshalling(Parcel& parcel) const
    {
        return true;
    }

    RSDisplayNodeConfig displayNodeConfig_;
};
}  // namespace Rosen
}  // namespace OHOS

/** @} */
#endif  // RENDER_SERVICE_CLIENT_CORE_UI_RS_DISPLAY_NODE_H
