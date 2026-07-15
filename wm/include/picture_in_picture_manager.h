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

#include <map>
#include <mutex>
#include <vector>

#include "picture_in_picture_controller.h"
#include "picture_in_picture_option_ani.h"
#include "window.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
class PictureInPictureManager {
    friend class PictureInPictureManagerTest;
public:
    PictureInPictureManager();
    ~PictureInPictureManager();
    static bool ShouldAbortPipStart();
    static bool IsSupportPiP();
    static void PutPipControllerInfo(uint32_t windowId, sptr<PictureInPictureControllerBase> pipController);
    static void RemovePipControllerInfo(uint32_t windowId);
    static sptr<PictureInPictureControllerBase> GetPipControllerInfo(uint32_t windowId);

    static bool HasActiveController();
    static bool IsActiveController(wptr<PictureInPictureControllerBase> pipController);
    static void AttachAutoStartController(int32_t handleId, wptr<PictureInPictureControllerBase> pipController);
    static void DetachAutoStartController(int32_t handleId, wptr<PictureInPictureControllerBase> pipController);
    static void EvictOldestControllerIfNeeded(const sptr<PictureInPictureControllerBase>& controller);

    static bool IsAttachedToSameWindow(uint32_t windowId);
    static sptr<Window> GetCurrentWindowByMainWindowId(uint32_t windowId);
    static sptr<Window> GetSameGroupWindowByMainWindowId(uint32_t windowId, PiPTemplateType templateType);
    static sptr<Window> GetCurrentWindow();

    static void DoPreRestore(uint32_t windowId);
    static void DoRestore(uint32_t windowId);
    static void DoClose(uint32_t windowId, bool destroyWindow, bool byPriority);
    static void DoCloseWithReason(uint32_t windowId, bool destroyWindow, bool byPriority, PiPStateChangeReason reason);
    static void DoActionCloseByRequest(uint32_t windowId);
    static void DoActionCloseByPanel(uint32_t windowId);
    static void DoActionCloseByDumpster(uint32_t windowId);
    static void DoActionClose(uint32_t windowId);
    static void DoPrepareSource(uint32_t windowId);
    static void DoLocateSource(uint32_t windowId);
    static void DoActionEvent(uint32_t windowId, const std::string& actionName, int32_t status);
    static void DoControlEvent(uint32_t windowId, PiPControlType controlType, PiPControlStatus status);
    static void PipSizeChange(uint32_t windowId, double width, double height, double scale);
    static void DoActiveStatusChangeEvent(uint32_t windowId, bool status);
    static void AutoStartPipWindow(uint32_t windowId);
    static void DoDestroy(uint32_t windowId);
    static bool GetPipEnabled();
    static void SetMultiPipConfig(const PiPMultiConfig& config);
    static const PiPMultiConfig& GetMultiPipConfig();
    static bool IsTemplateTypeSupported(PiPTemplateType type);
    static bool IsPipGroupLimitReached(PiPTemplateType type);
    static bool IsAutoStartControllerMapEmpty();
    static std::shared_ptr<NativeReference> innerCallbackRef_;

private:
    static std::map<int32_t, wptr<PictureInPictureControllerBase>> autoStartControllerMap_;
    static std::map<uint32_t, std::vector<wptr<PictureInPictureControllerBase>>> mainWindowToAutoStartControllersMap_;
    static std::map<uint32_t, sptr<PictureInPictureControllerBase>> windowToControllerMap_;
    static std::mutex controllerMapMutex_;
    static std::mutex AutoStartControllerMapMutex_;
    static void UpdatePipGroupCountLocked(const sptr<PictureInPictureControllerBase>& controller, bool increase);
    static bool IsControllerStateActive(PiPWindowState state);

    static PiPMultiConfig pipMultiConfig_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_PICTURE_IN_PICTURE_MANAGER_H
