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

#ifndef OHOS_PICTURE_IN_PICTURE_MANAGER_H
#define OHOS_PICTURE_IN_PICTURE_MANAGER_H

#include "picture_in_picture_controller.h"
#include "window.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
class PictureInPictureManager {
public:
    PictureInPictureManager();
    ~PictureInPictureManager();
    static bool ShouldAbortPipStart();
    static bool IsSupportPiP();
    static void PutPipControllerInfo(int32_t windowId, sptr<PictureInPictureControllerBase> pipController);
    static void RemovePipControllerInfo(int32_t windowId);
    static void AttachActivePipController(sptr<PictureInPictureControllerBase> pipController);
    static void DetachActivePipController(sptr<PictureInPictureControllerBase> pipController);
    static sptr<PictureInPictureControllerBase> GetPipControllerInfo(int32_t windowId);

    static bool HasActiveController();
    static bool IsActiveController(wptr<PictureInPictureControllerBase> pipController);
    static void SetActiveController(sptr<PictureInPictureControllerBase> pipController);
    static void RemoveActiveController(wptr<PictureInPictureControllerBase> pipController);
    static void AttachAutoStartController(int32_t handleId, wptr<PictureInPictureController> pipController);
    static void DetachAutoStartController(int32_t handleId, wptr<PictureInPictureController> pipController);
    static bool IsAttachedToSameWindow(uint32_t windowId);
    static sptr<Window> GetCurrentWindow();

    static void DoPreRestore();
    static void DoRestore();
    static void DoClose(bool destroyWindow, bool needAnim);
    static void DoCloseWithReason(bool destroyWindow, bool needAnim, PiPStateChangeReason reason);
    static void DoActionCloseByRequest();
    static void DoActionCloseByPanel();
    static void DoActionCloseByDumpster();
    static void DoActionClose();
    static void DoPrepareSource();
    static void DoLocateSource();
    static void DoActionEvent(const std::string& actionName, int32_t status);
    static void DoControlEvent(PiPControlType controlType, PiPControlStatus status);
    static void PipSizeChange(double width, double height, double scale);
    static void DoActiveStatusChangeEvent(bool status);
    static void AutoStartPipWindow();
    static void DoDestroy();
    static bool GetPipEnabled();
    static std::shared_ptr<NativeReference> innerCallbackRef_;

private:
    static sptr<PictureInPictureControllerBase> GetActiveController() { return activeController_; }

    // controller in use
    static sptr<PictureInPictureControllerBase> activeController_;
    static wptr<PictureInPictureController> autoStartController_;
    // controllers enable auto start
    static std::map<int32_t, wptr<PictureInPictureController>> autoStartControllerMap_;

    static std::map<int32_t, sptr<PictureInPictureControllerBase>> windowToControllerMap_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_PICTURE_IN_PICTURE_MANAGER_H