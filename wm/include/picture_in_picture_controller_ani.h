/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#ifndef OHOS_PICTURE_IN_PICTURE_CONTROLLER_ANI_H
#define OHOS_PICTURE_IN_PICTURE_CONTROLLER_ANI_H

#include "picture_in_picture_controller_base.h"

namespace OHOS {
namespace Rosen {

using namespace Ace;
class PictureInPictureControllerAni : public PictureInPictureControllerBase {
public:
    PictureInPictureControllerAni(sptr<PipOptionAni> pipOption,
                                  sptr<Window> mainWindow,
                                  uint32_t mainWindowId,
                                  ani_env* env);
    ~PictureInPictureControllerAni();

    WMError StartPictureInPicture(StartPipType startType) override;
    void SetAutoStartEnabled(bool enable) override;
    void IsAutoStartEnabled(bool& enable) const override;
    void UpdateContentSize(int32_t width, int32_t height) override;
    void UpdateContentNodeRef(ani_ref nodeRef) override;
    void PrepareSource() override;
    void RestorePictureInPictureWindow() override;
    WMError RegisterPipContentListenerWithType(const std::string&,
        ani_ref updateNodeCallbackRef) override;
    WMError UnRegisterPipContentListenerWithType(const std::string&) override;
    ani_ref GetANIPipContentCallbackRef(const std::string&) override;
    WMError SetXComponentController(std::shared_ptr<XComponentController> xComponentController) override;
    ani_ref GetANICustomNodeController() override;
    ani_ref GetANITypeNode() const override;
    bool IsTypeNodeEnabled() const override;
    bool IsPullPiPAndHandleNavigation();
    std::string GetPiPNavigationId() const override;

protected:
    sptr<PipOptionAni> pipOption_ = nullptr;
    ani_vm* vm_ = nullptr;
    
    ani_env* GetEnv() const;
    WMError CreatePictureInPictureWindow(StartPipType startType) override;
    void UpdateWinRectByComponent() override;
    void UpdatePiPSourceRect() const override;
    void SetUIContent() const override;
    void ResetExtController() override;
    void NotifyNodeUpdate(ani_ref nodeRef) override;
    void NotifyStateChangeInner(PiPState state) override;
    void NotifyStateChangeInner(ani_env* env, PiPState state) override;

    wptr<PictureInPictureControllerAni> weakRef_ = nullptr;
    std::shared_ptr<XComponentController> mainWindowXComponentController_ = nullptr;
    int32_t firstHandleId_ = -1;

private:
    void DeletePIPMode();
    virtual NavigationController* GetNavigationController(const std::string& navId);
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_PICTURE_IN_PICTURE_CONTROLLER_ANI_H