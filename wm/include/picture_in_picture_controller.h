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

#ifndef OHOS_PICTURE_IN_PICTURE_CONTROLLER_H
#define OHOS_PICTURE_IN_PICTURE_CONTROLLER_H

#include "picture_in_picture_controller_base.h"

namespace OHOS {
namespace Rosen {

using namespace Ace;
class PictureInPictureController : public PictureInPictureControllerBase {
public:
    PictureInPictureController(sptr<PipOption> pipOption, sptr<Window> mainWindow, uint32_t mainWindowId, napi_env env);
    ~PictureInPictureController();
    WMError StartPictureInPicture(StartPipType startType) override;
    void SetAutoStartEnabled(bool enable) override;
    void IsAutoStartEnabled(bool& enable) const override;
    void UpdateContentSize(int32_t width, int32_t height) override;
    void UpdateContentNodeRef(napi_ref nodeRef) override;
    void PrepareSource() override;
    void RestorePictureInPictureWindow() override;
    void NotifyNodeUpdate(napi_ref nodeRef) override;
    WMError SetXComponentController(std::shared_ptr<XComponentController> xComponentController) override;
    napi_ref GetCustomNodeController() override;
    napi_ref GetTypeNode() const override;
    bool IsTypeNodeEnabled() const override;
    bool IsPullPiPAndHandleNavigation();
    std::string GetPiPNavigationId();

protected:
    WMError CreatePictureInPictureWindow(StartPipType startType) override;
    void UpdateWinRectByComponent() override;
    void UpdatePiPSourceRect() const override;
    void SetUIContent() const override;
    void ResetExtController() override;
    wptr<PictureInPictureController> weakRef_ = nullptr;
    std::shared_ptr<XComponentController> mainWindowXComponentController_ = nullptr;
    int32_t firstHandleId_ = -1;

private:
    void DeletePIPMode();
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_PICTURE_IN_PICTURE_CONTROLLER_H