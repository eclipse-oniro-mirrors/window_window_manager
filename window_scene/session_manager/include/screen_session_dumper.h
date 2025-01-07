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
    void ExcuteDumpCmd();
    void DumpEventTracker(EventTracker& tracker);
    void DumpFreezedPidList(std::set<int32_t> pidList);
    void DumpMultiUserInfo(std::vector<int32_t> oldScbPids, int32_t userId, int32_t ScbPid);

private:
    void ShowHelpInfo();
    void ShowAllScreenInfo();
    void DumpFoldStatus();
    void DumpTentMode();
    void OutputDumpInfo();
    void DumpScreenSessionById(ScreenId id);
    void DumpRsInfoById(ScreenId id);
    void DumpRsInfoById01(sptr<ScreenSession> screenSession);
    void DumpCutoutInfoById(ScreenId id);
    void DumpScreenInfoById(ScreenId id);
    void DumpScreenPropertyById(ScreenId id);
    void ExcuteInjectCmd();
    void ExcuteInjectCmd2();
/*
    hidumper 命令注入隔离
*/
    void ShowNotifyFoldStatusChangedInfo();
    void ShowIllegalArgsInfo();
    void SetMotionSensorvalue(std::string input);
    void SetRotationLockedvalue(std::string input);
    void SetEnterOrExitTentMode(std::string input);
    void SetHoverStatusChange(std::string input);
    bool IsValidDisplayModeCommand(std::string command);
    int SetFoldDisplayMode();
    int SetFoldStatusLocked();
    void SetHallAndPostureValue(std::string input);
    void SetHallAndPostureStatus(std::string input);
private:
    int fd_;
    std::vector<std::string> params_;
    std::string dumpInfo_;
/*
    依赖的外部定义
*/
private:
    typedef struct EXTHALLData {
        float flag = 0.0;
        float hall = 0.0;
    } ExtHallData;
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SCREEN_SESSION_DUMPER_H