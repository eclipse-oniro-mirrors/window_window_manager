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
 
#ifndef RENDER_SERVICE_CLIENT_CORE_TRANSACTION_RS_RENDRE_INTERFACE_H
#define RENDER_SERVICE_CLIENT_CORE_TRANSACTION_RS_RENDRE_INTERFACE_H
 
#include "screen_manager/screen_types.h"
#include "rs_interfaces.h"
#include "ui/rs_display_node.h"
#include "rs_render_service_client.h"
#include "common/rs_common_def.h"
 
namespace OHOS {
namespace Rosen {
class RSRenderInterface {
    RSRenderInterface();
    ~RSRenderInterface() noexcept;
public:
    int32_t SetLogicalCameraRotationCorrection(ScreenId id, ScreenRotation logicalCorrection)
    {
        return 0;
    }
 
    int32_t GetBrightnessInfo(ScreenId screenId, BrightnessInfo& brightnessInfo)
    {
        return 0;
    }
 
    bool TakeSurfaceCapture(std::shared_ptr<RSSurfaceNode> node, std::shared_ptr<SurfaceCaptureCallback> callback,
        RSSurfaceCaptureConfig captureConfig = {})
    {
        return false;
    }
 
    bool TakeSurfaceCapture(std::shared_ptr<RSDisplayNode> node, std::shared_ptr<SurfaceCaptureCallback> callback,
        RSSurfaceCaptureConfig captureConfig = {})
    {
        return false;
    }
 
    int32_t GetScreenHDRStatus(ScreenId id, HdrStatus& hdrStatus)
    {
        return 0;
    }
 
    bool FreezeScreen(std::shared_ptr<RSDisplayNode> node, bool isFreeze, bool needSync = false)
    {
        return false;
    }
 
    bool TakeSurfaceCaptureWithAllWindows(std::shared_ptr<RSDisplayNode> node,
        std::shared_ptr<SurfaceCaptureCallback> callback, RSSurfaceCaptureConfig captureConfig,
        bool checkDrmAndSurfaceLock)
    {
        return false;
    }
};
}  // namespace Rosen
}  // namespace OHOS
#endif