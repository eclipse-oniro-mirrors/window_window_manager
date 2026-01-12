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
#include "session/screen/include/screen_property.h"
#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
class SceneSessionDirtyManager;
struct SecSurfaceInfo;
class SceneInputManager : public std::enable_shared_from_this<SceneInputManager> {
WM_DECLARE_SINGLE_INSTANCE_BASE(SceneInputManager)
public:
    void Init();
    void FlushDisplayInfoToMMI(std::vector<MMI::WindowInfo>&& windowInfoList,
        std::vector<std::shared_ptr<Media::PixelMap>>&& pixelMapList, const bool forceFlush = false);
    void NotifyWindowInfoChange(const sptr<SceneSession>& scenenSession, const WindowUpdateType& type);
    void NotifyWindowInfoChangeFromSession(const sptr<SceneSession>& sceneSession);
    void NotifyMMIWindowPidChange(const sptr<SceneSession>& sceneSession, const bool startMoving);
    void UpdateSecSurfaceInfo(const std::map<uint64_t, std::vector<SecSurfaceInfo>>& secSurfaceInfoMap);
    void UpdateConstrainedModalUIExtInfo(const std::map<uint64_t,
        std::vector<SecSurfaceInfo>>& constrainedModalUIExtInfoMap);
    std::optional<ExtensionWindowEventInfo> GetConstrainedModalExtWindowInfo(const sptr<SceneSession>& sceneSession);
    using FlushWindowInfoCallback = std::function<void()>;
    void RegisterFlushWindowInfoCallback(FlushWindowInfoCallback&& callback);
    void ResetSessionDirty();
    std::pair<std::vector<MMI::WindowInfo>, std::vector<std::shared_ptr<Media::PixelMap>>>
        GetFullWindowInfoList();
    void UpdateHotAreas(const sptr<SceneSession>& sceneSession, std::vector<MMI::Rect>& touchHotAreas,
        std::vector<MMI::Rect>& pointerHotAreas) const;

    /*
     * Multi User
     */
    void FlushEmptyInfoToMMI();
    void SetUserBackground(bool userBackground);
    void SetCurrentUserId(int32_t userId);

protected:
    SceneInputManager() = default;
    virtual ~SceneInputManager() = default;

private:
    void UpdateFocusedSessionId(int32_t focusedSessionId);
    void FlushFullInfoToMMI(const std::vector<MMI::ScreenInfo>& screenInfos,
        std::map<DisplayGroupId, MMI::DisplayGroupInfo>& displayGroupMap,
        const std::vector<MMI::WindowInfo>& windowInfoList, bool isOverBatchSize = false);
    void FlushChangeInfoToMMI(const std::map<uint64_t, std::vector<MMI::WindowInfo>>& screenId2Windows);
    std::vector<MMI::ScreenInfo> ConstructScreenInfos(std::map<ScreenId, ScreenProperty>& screensProperties);
    void ConstructDisplayGroupInfos(std::map<ScreenId, ScreenProperty>& screensProperties,
        std::map<DisplayGroupId, MMI::DisplayGroupInfo>& displayGroupMap);
    bool CheckNeedUpdate(const std::vector<MMI::ScreenInfo>& screenInfos,
        const std::vector<MMI::DisplayInfo>& displayInfos, const std::vector<MMI::WindowInfo>& windowInfoList);
    void PrintScreenInfo(const std::vector<MMI::ScreenInfo>& screenInfos);
    void PrintDisplayInfo(const std::vector<MMI::DisplayInfo>& displayInfos);
    void PrintWindowInfo(const std::vector<MMI::WindowInfo>& windowInfoList);
    void UpdateDisplayAndWindowInfo(const std::vector<MMI::ScreenInfo>& screenInfos,
        std::map<DisplayGroupId, MMI::DisplayGroupInfo>& displayGroupMap,
        std::vector<MMI::WindowInfo> windowInfoList);
    void ConstructDumpDisplayInfo(const MMI::DisplayInfo& displayInfo,
        std::ostringstream& dumpDisplayListStream);
    void ConstructDumpWindowInfo(const MMI::WindowInfo& windowInfo,
        std::ostringstream& dumpWindowListStream);
    std::unordered_map<DisplayId, int32_t> GetFocusedSessionMap() const;
    std::shared_ptr<SceneSessionDirtyManager> sceneSessionDirty_;
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::vector<MMI::ScreenInfo> lastScreenInfos_;
    std::vector<MMI::DisplayInfo> lastDisplayInfos_;
    std::vector<MMI::WindowInfo> lastWindowInfoList_;
    int32_t lastFocusId_ { -1 };
    int32_t focusedSessionId_ { -1 };

    /*
     * Multi User
     */
    int32_t currentUserId_ = INVALID_USER_ID;
    std::atomic<bool> isUserBackground_ { false };
};
}//Rosen
}//OHOS
#endif //OHOS_SESSION_MANAGER_SCENE_INPUT_MANAGER_H