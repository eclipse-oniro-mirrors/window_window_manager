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

#ifndef OHOS_ROSEN_SCREEN_SESSION_DUMPER_H
#define OHOS_ROSEN_SCREEN_SESSION_DUMPER_H

#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <set>
#include <refbase.h>

#include "event_tracker.h"
#include "dm_common.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "screen_rotation_property.h"

namespace OHOS {
namespace Rosen {

class ScreenSessionDumper : public RefBase {
public:
    ScreenSessionDumper(int fd, const std::vector<std::u16string>& args);
    ~ScreenSessionDumper() = default;

    bool IsNumber(std::string str);
    static bool IsConcurrentUser();
    void ExecuteDumpCmd();
    void DumpEventTracker(EventTracker& tracker);
    void DumpFreezedPidList(std::set<int32_t> pidList);
    void DumpMultiUserInfo(std::vector<int32_t> oldScbPids, int32_t userId, int32_t ScbPid);

private:
    void ShowHelpInfo();
    void ShowAllScreenInfo();
    void ShowClientScreenInfo();
    void ShowUserScreenRelation();
    void AppendSectionLine();
    void ShowVisibleAreaDisplayInfo();
    void DumpFoldStatus();
    void DumpForceFoldLockStatus();
    std::string ConvertFoldStatusToString(FoldStatus foldStatus);
    void DumpTentMode();
    void DumpFoldCreaseRegion();
    void OutputDumpInfo();
    void DumpScreenSessionById(ScreenId id);
    void DumpRsInfoById(ScreenId id);
    void DumpRsInfoById01(sptr<ScreenSession> screenSession);
    void DumpCutoutInfoPrint(std::ostringstream& oss,
        const OHOS::Rosen::DMRect& areaRect, const std::string& label);
    void DumpCutoutInfoById(ScreenId id);
    void DumpScreenInfoById(ScreenId id);
    void DumpVisibleAreaDisplayInfoById(DisplayId id);
    void DumpScreenPropertyById(ScreenId id);
    void DumpScreenUserRelation(ScreenId id);
    void ExecuteInjectCmd();
    void ExecuteInjectCmd2();

    void ShowNotifyFoldStatusChangedInfo();
    void ShowIllegalArgsInfo();
    void SetMotionSensorValue(std::string input);
    void SetRotationLockedValue(std::string input);
    void SetEnterOrExitTentMode(std::string input);
    void SetHoverStatusChange(std::string input);
    void SetSuperFoldStatusChange(std::string input);
    void MockSendCastPublishEvent(std::string input);
    bool IsValidDisplayModeCommand(std::string command);
    int32_t SetFoldDisplayMode();
    int SetFoldStatusLocked();
    int ForceSetFoldStatusAndLock(std::string& input);
    int RestorePhysicalFoldStatus();
    void SetHallAndPostureValue(std::string input);
    void SetHallAndPostureStatus(std::string input);
    void SetSecondaryStatusChange(const std::string &input);
    void SetLandscapeLock(std::string input);
    bool IsAllCharDigit(const std::string &firstPostureStr);
    bool GetPostureAndHall(std::vector<std::string> strVec,
        std::vector<float> &postures, std::vector<uint16_t> &halls);
    void TriggerSecondarySensor(const std::string &valueStr);
    void TriggerSecondaryFoldStatus(const std::string &valueStr);
    void SetDuringCallState(std::string input);
    bool IsDeveloperModeCommand();
    void ShowCurrentLcdStatus(ScreenId screenId);
private:
    const std::map<FoldStatus, std::string> statusMap_ = {
        {FoldStatus::EXPAND, "EXPAND"},
        {FoldStatus::FOLDED, "FOLDED"},
        {FoldStatus::HALF_FOLD, "HALF_FOLD"},
        {FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND, "FOLD_STATE_EXPAND_WITH_SECOND_EXPAND"},
        {FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED, "FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED"},
        {FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND, "FOLD_STATE_FOLDED_WITH_SECOND_EXPAND"},
        {FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED, "FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED"},
        {FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND, "FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND"},
        {FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED, "FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED"}
    };
    int fd_;
    std::vector<std::string> params_;
    std::string dumpInfo_;
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SCREEN_SESSION_DUMPER_H