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

#ifndef OHOS_WEB_PICTURE_IN_PICTURE_CONTROLLER_H
#define OHOS_WEB_PICTURE_IN_PICTURE_CONTROLLER_H

#include "picture_in_picture_controller_base.h"

namespace OHOS {
namespace Rosen {

using namespace Ace;

struct PiPConfig {
    uint32_t mainWindowId;
    uint32_t pipTemplateType;
    uint32_t width;
    uint32_t height;
    std::vector<uint32_t> controlGroup;
    napi_env env;
};

class WebPictureInPictureController : public PictureInPictureControllerBase {
public:
    explicit WebPictureInPictureController(const PiPConfig& config);
    ~WebPictureInPictureController() = default;
    WMError StartPictureInPicture(StartPipType startType) override;
    void UpdateContentSize(int32_t width, int32_t height) override;
    void SetPipInitialSurfaceRect(int32_t positionX, int32_t positionY, uint32_t width, uint32_t height) override;
    void RestorePictureInPictureWindow() override;
    WMError SetXComponentController(std::shared_ptr<XComponentController> xComponentController) override;
    uint8_t GetWebRequestId() override;
    WMError SetPipParentWindowId(uint32_t windowId) override;
    void SetAutoStartEnabled(bool enable) override;

protected:
    WMError CreatePictureInPictureWindow(StartPipType startType) override;
    void UpdateWinRectByComponent() override;
    void SetUIContent() const override;
    void UpdatePiPSourceRect() const override;

private:
    wptr<WebPictureInPictureController> weakRef_ = nullptr;
    uint8_t webRequestId_ = 0;
    int32_t initPositionX_ = 0;
    int32_t initPositionY_ = 0;
    uint32_t initWidth_ = 0;
    uint32_t initHeight_ = 0;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_PICTURE_IN_PICTURE_CONTROLLER_H