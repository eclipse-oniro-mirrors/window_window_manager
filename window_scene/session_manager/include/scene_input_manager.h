/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_SESSION_MANAGER_SCENE_INPUT_MANAGER_H
#define OHOS_SESSION_MANAGER_SCENE_INPUT_MANAGER_H

#include <string>
#include <vector>

#include <ui_content.h>
#include "input_manager.h"
#include "session/host/include/scene_session.h"
#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
class SceneSessionDirtyManager;
struct SecSurfaceInfo;
class SceneInputManager : public std::enable_shared_from_this<SceneInputManager> {
WM_DECLARE_SINGLE_INSTANCE_BASE(SceneInputManager)
public:
    void FlushDisplayInfoToMMI(const bool forceFlush = false);
    void FlushEmptyInfoToMMI();
    void NotifyWindowInfoChange(const sptr<SceneSession>& scenenSession, const WindowUpdateType& type);
    void NotifyWindowInfoChangeFromSession(const sptr<SceneSession>& sceneSession);
    void NotifyMMIWindowPidChange(const sptr<SceneSession>& sceneSession, const bool startMoving);
    void SetUserBackground(bool userBackground);
    bool IsUserBackground();
    void SetCurrentUserId(int32_t userId);
    void UpdateSecSurfaceInfo(const std::map<uint64_t, std::vector<SecSurfaceInfo>>& secSurfaceInfoMap);

protected:
    SceneInputManager() = default;
    virtual ~SceneInputManager() = default;

private:
    void Init();
    void UpdateFocusedSessionId(int32_t focusedSessionId);
    void FlushFullInfoToMMI(const std::vector<MMI::DisplayInfo>& displayInfos,
        const std::vector<MMI::WindowInfo>& windowInfoList);
    void FlushChangeInfoToMMI(const std::map<uint64_t, std::vector<MMI::WindowInfo>>& screenId2Windows);
    void ConstructDisplayInfos(std::vector<MMI::DisplayInfo>& displayInfos);
    bool CheckNeedUpdate(const std::vector<MMI::DisplayInfo>& displayInfos,
        const std::vector<MMI::WindowInfo>& windowInfoList);
    void PrintWindowInfo(const std::vector<MMI::WindowInfo>& windowInfoList);
    void UpdateDisplayAndWindowInfo(const std::vector<MMI::DisplayInfo>& displayInfos,
        std::vector<MMI::WindowInfo> windowInfoList);
    std::shared_ptr<SceneSessionDirtyManager> sceneSessionDirty_;
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::vector<MMI::DisplayInfo> lastDisplayInfos_;
    std::vector<MMI::WindowInfo> lastWindowInfoList_;
    int32_t lastFocusId_ { -1 };
    int32_t currentUserId_ { -1 };
    int32_t focusedSessionId_ { -1 };
    std::atomic<bool> isUserBackground_ = false;
};
}//Rosen
}//OHOS
#endif //OHOS_SESSION_MANAGER_SCENE_INPUT_MANAGER_H