/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SUPER_FOLD_STATE_MANAGER_H
#define OHOS_ROSEN_SUPER_FOLD_STATE_MANAGER_H
  
#include <mutex>
#include <refbase.h>
#include <functional>
#include <map>
#include <utility>
#include <atomic>
#include "nlohmann/json.hpp"
#include "session/screen/include/screen_session.h"

#include "dm_common.h"
#include "wm_single_instance.h"
#include "transaction/rs_interfaces.h"
#include "fold_screen_info.h"

namespace OHOS {

namespace Rosen {
enum class ScreenDirectionType : uint32_t {
    LEFT,
    TOP,
    RIGHT,
    BOTTOM
};

class RSInterfaces;

struct SuperFoldCreaseRegionItem {
    DisplayOrientation orientation_;
    SuperFoldStatus superFoldStatus_;
    FoldCreaseRegion region_;
    SuperFoldCreaseRegionItem(DisplayOrientation orientation, SuperFoldStatus superFoldStatus, FoldCreaseRegion region)
        : orientation_(orientation), superFoldStatus_(superFoldStatus), region_(region) {}
};

class SuperFoldStateManager final {
    WM_DECLARE_SINGLE_INSTANCE_BASE(SuperFoldStateManager)
public:
    SuperFoldStateManager();
    ~SuperFoldStateManager();

    void AddStateManagerMap(SuperFoldStatus curState,
    SuperFoldStatusChangeEvents event,
    SuperFoldStatus nextState,
    std::function<void (SuperFoldStatusChangeEvents)> action);

    void TransferState(SuperFoldStatus nextState);

    void HandleSuperFoldStatusChange(SuperFoldStatusChangeEvents events);

    void DriveStateMachineToExpand();

    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();

    FoldCreaseRegion GetLiveCreaseRegion();

    SuperFoldStatus GetCurrentStatus();

    FoldStatus MatchSuperFoldStatusToFoldStatus(SuperFoldStatus superFoldStatus);

    void SetSystemKeyboardStatus(bool isTpKeyboardOn = false);
    bool GetSystemKeyboardStatus();

    bool GetKeyboardState();

    DMError RefreshExternalRegion();

    void GetAllCreaseRegion();

    nlohmann::ordered_json GetFoldCreaseRegionJson();

    void HandleSuperFoldDisplayCallback(sptr<ScreenSession>& screenSession, SuperFoldStatusChangeEvents changeEvent);

    void AddMirrorVirtualScreenIds(const std::vector<ScreenId>& screenIds, const DMRect& rect);
    void ClearMirrorVirtualScreenIds(const std::vector<ScreenId>& screenIds);

private:
    std::atomic<SuperFoldStatus> curState_ = SuperFoldStatus::UNKNOWN;
    sptr<FoldCreaseRegion> currentSuperFoldCreaseRegion_ = nullptr;
    FoldCreaseRegion liveCreaseRegion_ = FoldCreaseRegion(0, {});
    FoldCreaseRegion GetFoldCreaseRegion(bool isVertical, bool isNeedReverse) const;
    void GetFoldCreaseRect(bool isVertical, bool isNeedReverse, const std::vector<int32_t>& foldRect,
        std::vector<DMRect>& foldCreaseRect) const;
    std::mutex superStatusMutex_;
    std::vector<SuperFoldCreaseRegionItem> superFoldCreaseRegionItems_;
    struct Transition {
        SuperFoldStatus nextState;
        std::function<void (SuperFoldStatusChangeEvents)> action;
    };

    using transEvent = std::pair<SuperFoldStatus, SuperFoldStatusChangeEvents>;
    std::map<transEvent, Transition> stateManagerMap_;

    bool isParamsValid(std::vector<std::string>& params);

    void InitSuperFoldStateManagerMap();

    void InitSuperFoldCreaseRegionParams();

    static void DoAngleChangeFolded(SuperFoldStatusChangeEvents event);

    static void DoAngleChangeHalfFolded(SuperFoldStatusChangeEvents event);

    static void DoAngleChangeExpanded(SuperFoldStatusChangeEvents event);

    static void DoKeyboardOn(SuperFoldStatusChangeEvents event);

    static void DoKeyboardOff(SuperFoldStatusChangeEvents event);

    static void DoFoldedToHalfFolded(SuperFoldStatusChangeEvents event);

    void SetCurrentStatus(SuperFoldStatus curState);

    void HandleDisplayNotify(SuperFoldStatusChangeEvents changeEvent);
    void HandleExtendToHalfFoldDisplayNotify(sptr<ScreenSession>& screenSession);
    void HandleHalfFoldToExtendDisplayNotify(sptr<ScreenSession>& screenSession);
    void HandleKeyboardOnDisplayNotify(sptr<ScreenSession>& screenSession);
    void HandleKeyboardOffDisplayNotify(sptr<ScreenSession>& screenSession);
    void HandleSystemKeyboardStatusDisplayNotify(sptr<ScreenSession>& screenSession, bool isTpKeyboardOn = false);
    void ReportNotifySuperFoldStatusChange(int32_t currentStatus, int32_t nextStatus, float postureAngle);
    void UpdateScreenHalfState(sptr<ScreenSession>& screenSession, SuperFoldStatusChangeEvents changeEvent);

    void HandleExtendToHalfFoldDisplayNotifyInServer(sptr<ScreenSession>& screenSession);
    void HandleHalfFoldToExtendDisplayNotifyInServer(sptr<ScreenSession>& screenSession);
    void HandleKeyboardOnDisplayNotifyInServer(sptr<ScreenSession>& screenSession);
    void HandleKeyboardOffDisplayNotifyInServer(sptr<ScreenSession>& screenSession);
    void HandleSystemKeyboardStatusDisplayNotifyInServer(sptr<ScreenSession>& screenSession,
        bool isTpKeyboardOn = false);
    void HandleSuperFoldDisplayInServer(sptr<ScreenSession>& screenSession,
        SuperFoldStatusChangeEvents changeEvent);

    static bool ChangeScreenState(bool toHalf);
    int32_t GetCurrentValidHeight(sptr<ScreenSession> screenSession);
    // physical mode change
    void ModifyMirrorScreenVisibleRect(SuperFoldStatus preState, SuperFoldStatus curState);
    // virtual keyboard change
    void ModifyMirrorScreenVisibleRect(bool isTpKeyBoardOn);
    void ModifyMirrorScreenVisibleRectInner(const OHOS::Rect& rsRect, std::vector<DisplayId>& displayIds);
    std::vector<DisplayId> CalculateReCordingDisplayIds(const OHOS::Rect& nextRect);

    std::map<ScreenId, OHOS::Rect> mirrorScreenVisibleRectMap_;

    std::mutex mirrorScreenIdsMutex_;

    uint32_t GetFoldCreaseHeight() const;
    DMError RefreshMirrorRegionInner(sptr<ScreenSession>& mainScreenSession, sptr<ScreenSession>& secondarySession);
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SUPER_FOLD_STATE_MANAGER_H