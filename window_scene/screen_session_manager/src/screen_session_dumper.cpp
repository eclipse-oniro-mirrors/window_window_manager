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

#include "screen_session_dumper.h"

#include <csignal>
#include <fstream>
#include <transaction/rs_interfaces.h>

#include "unique_fd.h"
#include "screen_session_manager.h"
#include "session_permission.h"
#include "screen_rotation_property.h"
#include "screen_scene_config.h"
#include "screen_sensor_connector.h"
#include "parameters.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "fold_screen_controller/super_fold_sensor_manager.h"
#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_state_internel.h"
#include "window_helper.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "screen_sensor_mgr.h"
#include "fold_screen_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int LINE_WIDTH = 30;
constexpr int DUMPER_PARAM_INDEX_ONE = 1;
constexpr int DUMPER_PARAM_INDEX_TWO = 2;
constexpr int DUMPER_PARAM_INDEX_THREE = 3;
constexpr int MAX_DUMPER_PARAM_NUMBER = 10;
const std::string ARG_DUMP_HELP = "-h";
const std::string ARG_DUMP_ALL = "-a";
const std::string ARG_DUMP_FOLD_STATUS = "-f";
const std::string ARG_DUMP_LCD_STATUS = "-lcd";

constexpr int MOTION_SENSOR_PARAM_SIZE = 2;
const std::string STATUS_FOLD_HALF = "-z";
const std::string STATUS_EXPAND = "-y";
const std::string STATUS_EXPAND_WITH_SECOND_EXPAND = "-yy";
const std::string STATUS_FOLD = "-p";
const std::string ARG_SET_ROTATION_SENSOR = "-motion"; // rotation event inject
const std::string ARG_SET_ROTATION_LOCK = "-rotationlock";
const std::string ARG_PUBLISH_CAST_EVENT = "-publishcastevent";
const std::string ARG_FOLD_DISPLAY_FULL = "-f";
const std::string ARG_FOLD_DISPLAY_MAIN = "-m";
const std::string ARG_FOLD_DISPLAY_GLOBALL_FULL = "-g";
const std::string ARG_FOLD_DISPLAY_SUB = "-sub";
const std::string ARG_FOLD_DISPLAY_COOR = "-coor";
const std::vector<std::string> displayModeCommands = {"-f", "-m", "-sub", "-coor", "-g"};
const std::string ARG_LOCK_FOLD_DISPLAY_STATUS = "-l";
const std::string ARG_UNLOCK_FOLD_DISPLAY_STATUS = "-u";
const std::string ARG_FORCE_SET_FOLD_STATUS_AND_LOCK = "-ln";
const std::string ARG_RESTORE_PHYSICAL_FOLD_STATUS = "-u";
const std::string ARG_SET_ON_TENT_MODE = "-ontent";
const std::string ARG_SET_OFF_TENT_MODE = "-offtent";
const std::string ARG_SET_HOVER_STATUS = "-hoverstatus";
const std::string ARG_SET_SUPER_FOLD_STATUS = "-supertrans";
const std::string ARG_SET_POSTURE_HALL = "-posture";
const std::string ARG_SET_POSTURE_HALL_STATUS = "-registerhall"; // 关闭开合sensor报值
const std::string ARG_SET_SECONDARY_FOLD_STATUS = "-secondary";
const std::string ARG_CHANGE_OUTER_CMD = "outer";
const std::string ANGLE_STR = "angle";
const std::string HALL_STR = "hall";
const std::string ARG_SET_LANDSCAPE_LOCK = "-landscapelock";
const std::string ARG_SET_DURINGCALL_STATE = "-duringcallstate";
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
#ifdef FOLD_ABILITY_ENABLE
constexpr int SUPER_FOLD_STATUS_MAX = 2;
const char SECONDARY_DUMPER_VALUE_BOUNDARY[] = "mfg";
constexpr size_t SECONDARY_FOLD_STATUS_INDEX_M = 0;
constexpr size_t SECONDARY_FOLD_STATUS_INDEX_F = 1;
constexpr size_t SECONDARY_FOLD_STATUS_INDEX_G = 2;
constexpr size_t SECONDARY_FOLD_STATUS_COMMAND_NUM = 2;
constexpr uint16_t HALL_EXT_DATA_FLAG = 26;
#endif
}

static std::string GetProcessNameByPid(int32_t pid)
{
    std::string filePath = "/proc/" + std::to_string(pid) + "/comm";
    char tmpPath[PATH_MAX]  = { 0 };
    if (!realpath(filePath.c_str(), tmpPath)) {
        return "UNKNOWN";
    }
    std::ifstream infile(filePath);
    if (!infile.is_open()) {
        return "UNKNOWN";
    }
    std::string processName = "UNKNOWN";
    std::getline(infile, processName);
    infile.close();
    return processName;
}

ScreenSessionDumper::ScreenSessionDumper(int fd, const std::vector<std::u16string>& args)
    : fd_(fd)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> cv;
    std::string info;
    for (auto& u16str : args) {
        std::string arg = cv.to_bytes(u16str);
        params_.emplace_back(arg);
        info += arg;
    }
    TLOGI(WmsLogTag::DMS, "input args: [%{public}s]", info.c_str());
}

bool ScreenSessionDumper::IsNumber(std::string str)
{
    if (str.size() == 0) {
        return false;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(str.size()); i++) {
        if (str.at(i) < '0' || str.at(i) > '9') {
            return false;
        }
    }
    return true;
}

bool ScreenSessionDumper::IsConcurrentUser()
{
    return ScreenSceneConfig::IsConcurrentUser();
}

void ScreenSessionDumper::OutputDumpInfo()
{
    if (fd_ < 0) {
        TLOGE(WmsLogTag::DMS, "invalid fd: %{public}d", fd_);
        return;
    }

    static_cast<void>(signal(SIGPIPE, SIG_IGN));  // ignore SIGPIPE crash
    int ret = dprintf(fd_, "%s\n", dumpInfo_.c_str());
    if (ret < 0) {
        TLOGE(WmsLogTag::DMS, "dprintf error. ret: %{public}d", ret);
        return;
    }
    dumpInfo_.clear();
}


void ScreenSessionDumper::ExecuteDumpCmd()
{
    if (!(SessionPermission::IsSACalling() || SessionPermission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::DMS, "dump permission denied!");
        return;
    }
    if (params_.empty()) {
        TLOGE(WmsLogTag::DMS, "params is null");
        return;
    }
    if (params_[0] == ARG_DUMP_HELP) {
        ShowHelpInfo();
    } else if (params_[0] == ARG_DUMP_ALL) {
        ShowAllScreenInfo();
        ShowVisibleAreaDisplayInfo();
        ShowClientScreenInfo();
        if (IsConcurrentUser()) {
            ShowUserScreenRelation();
        }
        AppendSectionLine();
    } else if (params_[0] == ARG_DUMP_FOLD_STATUS) {
        DumpFoldStatus();
    } else if (params_[0] == ARG_DUMP_LCD_STATUS) {
        ShowCurrentLcdStatus(SCREEN_ID_FULL);
        ShowCurrentLcdStatus(SCREEN_ID_MAIN);
    }
    ExecuteInjectCmd();
    OutputDumpInfo();
}

void ScreenSessionDumper::ShowCurrentLcdStatus(ScreenId screenId)
{
    std::ostringstream oss;
    PanelPowerStatus powerStatus = PanelPowerStatus::INVALID_PANEL_POWER_STATUS;
    if (!ScreenSessionManager::GetInstance().GetScreenLcdStatus(screenId, powerStatus)) {
        oss << std::left << std::setw(LINE_WIDTH) << "Get screen " << screenId << " status failed" << std::endl;
        dumpInfo_.append(oss.str());
        return;
    }
    std::string status = "";
    switch (powerStatus) {
        case PanelPowerStatus::PANEL_POWER_STATUS_ON: {
            status = "PANEL_POWER_STATUS_ON";
            break;
        }
        case PanelPowerStatus::PANEL_POWER_STATUS_OFF: {
            status = "PANEL_POWER_STATUS_OFF";
            break;
        }
        default: {
            status = "UNKNOWN";
            break;
        }
    }
    oss << std::left << std::setw(LINE_WIDTH) << "LCD " << screenId << " status: " << status << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::ExecuteInjectCmd()
{
    if (IsDeveloperModeCommand()) {
        return;
    }
    bool isDebugMode = system::GetBoolParameter("dms.hidumper.supportdebug", false);
    if (!isDebugMode) {
        TLOGI(WmsLogTag::DMS, "Can't use DMS hidumper inject methods.");
        dumpInfo_.append("dms.hidumper.supportdebug false\n");
        return;
    }
    if (params_.size() == DUMPER_PARAM_INDEX_THREE) {
        TLOGI(WmsLogTag::DMS, "dump params[0] = %{public}s ,params[1] = %{public}s ,para,s[2] = %{public}s",
            params_[0].c_str(), params_[DUMPER_PARAM_INDEX_ONE].c_str(), params_[DUMPER_PARAM_INDEX_TWO].c_str());
        ScreenSessionManager::GetInstance().MultiScreenModeChange(params_[0], params_[DUMPER_PARAM_INDEX_ONE],
            params_[DUMPER_PARAM_INDEX_TWO]);
        return;
    }
    if (params_[0].find(ARG_PUBLISH_CAST_EVENT) != std::string::npos) {
        MockSendCastPublishEvent(params_[0]);
        return;
    } else if (params_.size() == 1 && (params_[0] == ARG_LOCK_FOLD_DISPLAY_STATUS
                || params_[0] == ARG_UNLOCK_FOLD_DISPLAY_STATUS)) {
        int errCode = SetFoldStatusLocked();
        if (errCode != 0) {
            ShowIllegalArgsInfo();
        }
        return;
    } else if (params_[0].find(ARG_FORCE_SET_FOLD_STATUS_AND_LOCK) != std::string::npos) {
        int errCode = ForceSetFoldStatusAndLock(params_[0]);
        if (errCode != 0) {
            ShowIllegalArgsInfo();
        }
        return;
    } else if (params_[0] == ARG_RESTORE_PHYSICAL_FOLD_STATUS && params_.size() == 1) {
        int errCode = RestorePhysicalFoldStatus();
        if (errCode != 0) {
            ShowIllegalArgsInfo();
        }
        return;
    }
    ExecuteInjectCmd2();
}

bool ScreenSessionDumper::IsDeveloperModeCommand()
{
    bool isEnd = true;
    bool isDeveloperMode = system::GetBoolParameter("const.security.developermode.state", false);
    if (!isDeveloperMode) {
        return isEnd;
    }
    // print param
    TLOGI(WmsLogTag::DMS, "get calling uid(%{public}zu)", params_.size());
    if ((params_.size() < 1) || (params_.size() > MAX_DUMPER_PARAM_NUMBER)) {
        TLOGE(WmsLogTag::DMS, "params size exceeded limit");
        return isEnd;
    }
    for (const auto& param : params_) {
        TLOGI(WmsLogTag::DMS, "params_: [%{public}s]", param.c_str());
    }
    // deal displayMode and motion/foldstatus
    if (IsValidDisplayModeCommand(params_[0])) {
        // check and set display mode
        if (SetFoldDisplayMode() != 0) {
            ShowIllegalArgsInfo();
        }
    } else if (params_[0] == STATUS_FOLD_HALF || params_[0] == STATUS_EXPAND || params_[0] == STATUS_FOLD ||
        params_[0] == STATUS_EXPAND_WITH_SECOND_EXPAND) {
        ShowNotifyFoldStatusChangedInfo();
    } else if (params_[0].find(ARG_SET_ROTATION_SENSOR) != std::string::npos) {
        SetMotionSensorValue(params_[0]);
    } else if (params_[0].find(ARG_SET_ROTATION_LOCK) != std::string::npos) {
        SetRotationLockedValue(params_[0]);
    } else {
        isEnd = false;
    }
    return isEnd;
}

void ScreenSessionDumper::ExecuteInjectCmd2()
{
    if (params_[0].find(ARG_SET_ON_TENT_MODE) != std::string::npos ||
        params_[0].find(ARG_SET_OFF_TENT_MODE) != std::string::npos) {
        SetEnterOrExitTentMode(params_[0]);
    } else if (params_[0].find(ARG_SET_HOVER_STATUS) != std::string::npos) {
        SetHoverStatusChange(params_[0]);
    } else if (params_[0].find(ARG_SET_SUPER_FOLD_STATUS) != std::string::npos) {
        SetSuperFoldStatusChange(params_[0]);
    } else if (params_[0].find(ARG_SET_POSTURE_HALL) != std::string::npos) {
        SetHallAndPostureValue(params_[0]);
    } else if (params_[0].find(ARG_SET_POSTURE_HALL_STATUS) != std::string::npos) {
        SetHallAndPostureStatus(params_[0]);
    } else if (params_[0].find(ARG_SET_SECONDARY_FOLD_STATUS) != std::string::npos) {
        SetSecondaryStatusChange(params_[0]);
    } else if (params_[0].find(ARG_SET_LANDSCAPE_LOCK) != std::string::npos) {
        SetLandscapeLock(params_[0]);
    } else if (params_[0].find(ARG_SET_DURINGCALL_STATE) != std::string::npos) {
        SetDuringCallState(params_[0]);
    }
}

void ScreenSessionDumper::DumpEventTracker(EventTracker& tracker)
{
    std::ostringstream oss;
    auto recordInfos = tracker.GetRecordInfos();
    oss << "-------------- DMS KEY EVENTS LIST  --------------" << std::endl;
    for (const auto& info : recordInfos) {
        oss << std::left << "[" << tracker.formatTimestamp(info.timestamp).c_str()
            << "]: " << info.info.c_str() << std::endl;
    }
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpMultiUserInfo(std::vector<int32_t> oldScbPids, int32_t userId, int32_t ScbPid)
{
    std::ostringstream oss;
    oss << "-------------- DMS Multi User Info --------------" << std::endl;
    oss << std::left << "[oldScbPid:] ";
    for (auto oldScbPid : oldScbPids) {
        oss << oldScbPid  << " ";
    }
    oss << std::endl;
    oss << std::left << "[userId:] " << userId << std::endl;
    oss << std::left << "[ScbPid:] " << ScbPid << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpFreezedPidList(std::set<int32_t> pidList)
{
    std::ostringstream oss;
    oss << "-------------- DMS FREEZED PID LIST  --------------" << std::endl;
    for (auto pid : pidList) {
        oss << std::left << "[PID: " << pid  << "]: "
            << " [" << GetProcessNameByPid(pid) << "]"<< std::endl;
    }
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::ShowHelpInfo()
{
    dumpInfo_.append("Usage:\n")
        .append(" -h                             ")
        .append("|help text for the tool\n")
        .append(" -a                             ")
        .append("|dump all screen information in the system\n")
        .append(" -z                             ")
        .append("|switch to fold half status\n")
        .append(" -y                             ")
        .append("|switch to expand status\n")
        .append(" -yy                            ")
        .append("|switch to both first and second axes expand status\n")
        .append(" -p                             ")
        .append("|switch to fold status\n")
        .append(" -f                             ")
        .append("|switch to full display mode\n")
        .append(" -m                             ")
        .append("|switch to main display mode\n")
        .append(" -sub                           ")
        .append("|switch to sub display mode\n")
        .append(" -coor                          ")
        .append("|switch to coor display mode\n")
        .append(" -g                          ")
        .append("|switch to global full display mode\n")
        .append(" -rotationlock                  ")
        .append("|set rotation lock, 0 to unloick, 1 to lock, "\
            "eg. -rotationlock,0 \n")
        .append(" -motion                        ")
        .append("|set the sensor rotation angle clockwise, "\
            "0 means 0 degree, 1 means 90 degree, 2 means 180 degree, 3 means 270 degree, eg. -motion,1\n");
    if (!system::GetBoolParameter("dms.hidumper.supportdebug", false)) {
        return;
    }
    dumpInfo_.append(" -ontent        ")
        .append("|set up tent mode\n")
        .append(" -offtent        ")
        .append("|set exit tent mode\n")
        .append(" -publishcastevent        ")
        .append("|publish cast event\n")
        .append(" -registerhall        ")
        .append("|set hall register, 0 to unregister, 1 to register\n");
}

void ScreenSessionDumper::ShowAllScreenInfo()
{
    std::vector<ScreenId> screenIds = ScreenSessionManager::GetInstance().GetAllScreenIds();
    for (auto screenId : screenIds) {
        std::ostringstream oss;
        oss << "---------------- Screen ID: " << screenId << " ----------------" << std::endl;
        dumpInfo_.append(oss.str());
        DumpFoldStatus();
        DumpTentMode();
        DumpScreenSessionById(screenId);
        DumpRsInfoById(screenId);
        DumpCutoutInfoById(screenId);
        DumpScreenInfoById(screenId);
        DumpScreenPropertyById(screenId);
        DumpFoldCreaseRegion();
    }
}

void ScreenSessionDumper::ShowClientScreenInfo()
{
    std::string clientInfos = ScreenSessionManager::GetInstance().DumperClientScreenSessions();
    dumpInfo_.append(clientInfos);
}

void ScreenSessionDumper::ShowUserScreenRelation()
{
    std::ostringstream oss;
    dumpInfo_.append("------- ConcurrentUser-Screen Relation --------\n");
    std::vector<ScreenId> screenIds = ScreenSessionManager::GetInstance().GetAllScreenIds();
    for (auto screenId : screenIds) {
        DumpScreenUserRelation(screenId);
    }
}

void ScreenSessionDumper::AppendSectionLine()
{
    dumpInfo_.append("-----------------------------------------------\n");
}

void ScreenSessionDumper::ShowVisibleAreaDisplayInfo()
{
    std::vector<DisplayId> displayIds = ScreenSessionManager::GetInstance().GetAllDisplayIds();
    for (auto displayId : displayIds) {
        std::ostringstream oss;
        oss << "-------------- Display ID: " << displayId << " --------------" << std::endl;
        dumpInfo_.append(oss.str());
        DumpVisibleAreaDisplayInfoById(displayId);
    }
}

void ScreenSessionDumper::DumpFoldStatus()
{
    std::ostringstream oss;
    auto& manager = ScreenSessionManager::GetInstance();
    const FoldStatus physicalStatus = manager.GetPhysicalFoldStatus();
    std::string physicalStatusStr = ConvertFoldStatusToString(physicalStatus);
    const bool isLocked = manager.GetPhysicalFoldLockFlag();
    const FoldStatus displayStatus = manager.GetFoldStatus();
    std::string displayStatusStr = ConvertFoldStatusToString(displayStatus);
    oss << std::left << std::setw(LINE_WIDTH) << "PhysicalFoldStatus: " << physicalStatusStr << "\n"
        << std::left << std::setw(LINE_WIDTH) << "FoldLockStatus: " << (isLocked ? "LOCKED" : "UNLOCKED") << "\n"
        << std::left << std::setw(LINE_WIDTH) << "DisplayFoldStatus: " << displayStatusStr << "\n";
    dumpInfo_.append(oss.str());
}

std::string ScreenSessionDumper::ConvertFoldStatusToString(FoldStatus status)
{
    auto it = statusMap_.find(status);
    return it != statusMap_.end() ? it->second : "UNKNOWN";
}

void ScreenSessionDumper::DumpTentMode()
{
    std::ostringstream oss;
    bool isTentMode = ScreenSessionManager::GetInstance().GetTentMode();
    oss << std::left << std::setw(LINE_WIDTH) << "TentMode: "
        << (isTentMode ? "true" : "false") << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpFoldCreaseRegion()
{
    std::ostringstream oss;
    auto creaseRegion = ScreenSessionManager::GetInstance().GetCurrentFoldCreaseRegion();
    if (creaseRegion == nullptr) {
        TLOGE(WmsLogTag::DMS, "creaseRegion is nullptr.");
        return;
    }
    auto creaseRects = creaseRegion->GetCreaseRects();
    if (creaseRects.empty()) {
        TLOGE(WmsLogTag::DMS, "current crease region is null");
        return;
    }
    oss << std::left << std::setw(LINE_WIDTH) << "CurrentCreaseRects<X, Y, W, H>: "
        << creaseRects[0].posX_ << ", " << creaseRects[0].posY_ << ", "
        << creaseRects[0].width_ << ", " << creaseRects[0].height_ << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpScreenSessionById(ScreenId id)
{
    std::ostringstream oss;
    oss << "[SCREEN SESSION]" << std::endl;
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(id);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession nullptr. screen id: %{public}" PRIu64"", id);
        return;
    }
    oss << std::left << std::setw(LINE_WIDTH) << "Name: "
        << screenSession->GetName() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "DisplayId: "
        << screenSession->GetDisplayId() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "RSScreenId: "
        << screenSession->GetRSScreenId() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "isInternal: "
        <<(screenSession->GetIsInternal() ? "true" : "false") << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "isExtend: "
        << (screenSession->GetIsExtend() ? "true" : "false") << std::endl;
    if (screenSession->GetDisplayNode()) {
        oss << std::left << std::setw(LINE_WIDTH) << "NodeId: "
            << screenSession->GetDisplayNode()->GetId() << std::endl;
    } else {
        oss << std::left << std::setw(LINE_WIDTH) << "NodeId: " << "nullptr" << std::endl;
    }
    sptr<SupportedScreenModes> activeModes = screenSession->GetActiveScreenMode();
    if (activeModes != nullptr) {
        oss << std::left << std::setw(LINE_WIDTH) << "activeModes<id, W, H, RS>: "
            << activeModes->id_ << ", " << activeModes->width_ << ", "
            << activeModes->height_ << ", " << activeModes->refreshRate_ << std::endl;
    }
    oss << std::left << std::setw(LINE_WIDTH) << "SourceMode: "
        << static_cast<int32_t>(screenSession->GetSourceMode()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "ScreenCombination: "
        << static_cast<int32_t>(screenSession->GetScreenCombination()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Orientation: "
        << static_cast<int32_t>(screenSession->GetOrientation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Rotation: "
        << static_cast<int32_t>(screenSession->GetRotation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "ScreenRequestedOrientation: "
        << static_cast<int32_t>(screenSession->GetScreenRequestedOrientation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "isExtend: "
        << static_cast<int32_t>(screenSession->GetIsExtend()) << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpRsInfoById(ScreenId id)
{
    std::ostringstream oss;
    oss << "[RS INFO]" << std::endl;
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(id);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession nullptr. screen id: %{public}" PRIu64"", id);
        return;
    }
    ScreenId rsId = id;
    if (screenSession->rsId_ != SCREEN_ID_INVALID) {
        rsId = screenSession->rsId_;
    }
    auto state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance().GetScreenPowerStatus(rsId));
    oss << std::left << std::setw(LINE_WIDTH) << "ScreenPowerState: "
        << static_cast<int32_t>(state) << std::endl;
    std::vector<ScreenColorGamut> colorGamuts;
    DMError ret = screenSession->GetScreenSupportedColorGamuts(colorGamuts);
    if (ret == DMError::DM_OK && colorGamuts.size() > 0) {
        oss << std::left << std::setw(LINE_WIDTH) << "SupportedColorGamuts: ";
        for (uint32_t i = 0; i < colorGamuts.size() - 1; i++) {
            oss << static_cast<int32_t>(colorGamuts[i]) << ", ";
        }
        oss << static_cast<int32_t>(colorGamuts[colorGamuts.size() - 1]) << std::endl;
    }
    ScreenColorGamut colorGamut;
    ret = screenSession->GetScreenColorGamut(colorGamut);
    if (ret == DMError::DM_OK) {
        oss << std::left << std::setw(LINE_WIDTH) << "ScreenColorGamut: "
            << static_cast<int32_t>(colorGamut) << std::endl;
    }
    ScreenGamutMap gamutMap;
    ret = screenSession->GetScreenGamutMap(gamutMap);
    if (ret == DMError::DM_OK) {
        oss << std::left << std::setw(LINE_WIDTH) << "ScreenGamutMap: "
            << static_cast<int32_t>(gamutMap) << std::endl;
    }
    GraphicPixelFormat pixelFormat;
    ret = screenSession->GetPixelFormat(pixelFormat);
    if (ret == DMError::DM_OK) {
        oss << std::left << std::setw(LINE_WIDTH) << "GraphicPixelFormat: "
            << static_cast<int32_t>(pixelFormat) << std::endl;
    }
    dumpInfo_.append(oss.str());
    DumpRsInfoById01(screenSession); // 拆分函数，避免函数过长
}

void ScreenSessionDumper::DumpRsInfoById01(sptr<ScreenSession> screenSession)
{
    std::ostringstream oss;
    std::vector<ScreenHDRFormat> hdrFormats;
    DMError ret = screenSession->GetSupportedHDRFormats(hdrFormats);
    if (ret == DMError::DM_OK && hdrFormats.size() > 0) {
        oss << std::left << std::setw(LINE_WIDTH) << "SupportedScreenHDRFormat: ";
        for (uint32_t i = 0; i < hdrFormats.size() - 1; i++) {
            oss << static_cast<int32_t>(hdrFormats[i]) << ", ";
        }
        oss << static_cast<int32_t>(hdrFormats[hdrFormats.size() - 1]) << std::endl;
    }
    ScreenHDRFormat hdrFormat;
    ret = screenSession->GetScreenHDRFormat(hdrFormat);
    if (ret == DMError::DM_OK) {
        oss << std::left << std::setw(LINE_WIDTH) << "ScreenHDRFormat: "
            << static_cast<int32_t>(hdrFormat) << std::endl;
    }
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    ret = screenSession->GetSupportedColorSpaces(colorSpaces);
    if (ret == DMError::DM_OK && colorSpaces.size() > 0) {
        oss << std::left << std::setw(LINE_WIDTH) << "SupportedColorSpaces: ";
        for (uint32_t i = 0; i < colorSpaces.size() - 1; i++) {
            oss << static_cast<int32_t>(colorSpaces[i]) << ", ";
        }
        oss << static_cast<int32_t>(colorSpaces[colorSpaces.size() - 1]) << std::endl;
    }
    GraphicCM_ColorSpaceType colorSpace;
    ret = screenSession->GetScreenColorSpace(colorSpace);
    if (ret == DMError::DM_OK) {
        oss << std::left << std::setw(LINE_WIDTH) << "ScreenColorSpace: "
            << static_cast<int32_t>(colorSpace) << std::endl;
    }
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpCutoutInfoPrint(std::ostringstream& oss,
    const DMRect& areaRect, const std::string& label)
{
    oss << std::left << std::setw(LINE_WIDTH) << label
        << areaRect.posX_ << ", "
        << areaRect.posY_ << ", "
        << areaRect.width_ << ", "
        << areaRect.height_ << std::endl;
}

void ScreenSessionDumper::DumpCutoutInfoById(ScreenId id)
{
    std::ostringstream oss;
    oss << "[CUTOUT INFO]" << std::endl;
    sptr<CutoutInfo> cutoutInfo = ScreenSessionManager::GetInstance().GetCutoutInfo(id);
    if (cutoutInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "cutoutInfo nullptr. screen id: %{public}" PRIu64"", id);
        return;
    }
    DumpCutoutInfoPrint(oss, cutoutInfo->GetWaterfallDisplayAreaRects().left, "WaterFall_L<X,Y,W,H>: ");
    DumpCutoutInfoPrint(oss, cutoutInfo->GetWaterfallDisplayAreaRects().top, "WaterFall_T<X,Y,W,H>: ");
    DumpCutoutInfoPrint(oss, cutoutInfo->GetWaterfallDisplayAreaRects().right, "WaterFall_R<X,Y,W,H>: ");
    DumpCutoutInfoPrint(oss, cutoutInfo->GetWaterfallDisplayAreaRects().bottom, "WaterFall_B<X,Y,W,H>: ");

    std::vector<DMRect> boundingRects = cutoutInfo->GetBoundingRects();
    oss << std::left << std::setw(LINE_WIDTH) << "BoundingRects<X,Y,W,H>: ";
    for (auto rect : boundingRects) {
        oss << "[" << rect.posX_ << ", " << rect.posY_ << ", " << rect.width_ << ", " << rect.height_ << "] ";
    }
    oss << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpScreenInfoById(ScreenId id)
{
    std::ostringstream oss;
    oss << "[SCREEN INFO]" << std::endl;
    auto screenInfo = ScreenSessionManager::GetInstance().GetScreenInfoById(id);
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenInfo nullptr. screen id: %{public}" PRIu64"", id);
        return;
    }
    oss << std::left << std::setw(LINE_WIDTH) << "VirtualWidth: "
        << screenInfo->GetVirtualWidth() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "VirtualHeight: "
        << screenInfo->GetVirtualHeight() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "LastParentId: "
        << screenInfo->GetLastParentId() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "ParentId: "
        << screenInfo->GetParentId() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "IsScreenGroup: "
        << screenInfo->GetIsScreenGroup() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "VirtualPixelRatio: "
        << screenInfo->GetVirtualPixelRatio() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Rotation: "
        << static_cast<int32_t>(screenInfo->GetRotation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Orientation: "
        << static_cast<int32_t>(screenInfo->GetOrientation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "SourceMode: "
        << static_cast<int32_t>(screenInfo->GetSourceMode()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "ScreenType: "
        << static_cast<int32_t>(screenInfo->GetType()) << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpVisibleAreaDisplayInfoById(DisplayId id)
{
    std::ostringstream oss;
    oss << "[DISPLAY INFO]" << std::endl;
    auto displayInfo = ScreenSessionManager::GetInstance().GetVisibleAreaDisplayInfoById(id);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo nullptr. display id: %{public}" PRIu64"", id);
        return;
    }
    oss << std::left << std::setw(LINE_WIDTH) << "visibleAreaWidth: "
        << displayInfo->GetWidth() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "visibleAreaHeight: "
        << displayInfo->GetHeight() << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpScreenPropertyById(ScreenId id)
{
    std::ostringstream oss;
    oss << "[SCREEN PROPERTY]" << std::endl;
    ScreenProperty screenProperty = ScreenSessionManager::GetInstance().GetScreenProperty(id);

    oss << std::left << std::setw(LINE_WIDTH) << "Rotation: " << screenProperty.GetRotation() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Density: " << screenProperty.GetDensity() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "DensityInCurResolution: "
        << screenProperty.GetDensityInCurResolution() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "PhyWidth: " << screenProperty.GetPhyWidth() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "PhyHeight: " << screenProperty.GetPhyHeight() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "RefreshRate: " << screenProperty.GetRefreshRate() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "VirtualPixelRatio: "
        << screenProperty.GetVirtualPixelRatio() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "ScreenRotation: "
        << static_cast<int32_t>(screenProperty.GetRotation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Orientation: "
        <<  static_cast<int32_t>(screenProperty.GetOrientation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "DisplayOrientation: "
        << static_cast<int32_t>(screenProperty.GetDisplayOrientation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "GetScreenType: "
        << static_cast<int32_t>(screenProperty.GetScreenType()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "ReqOrientation: "
        << static_cast<int32_t>(screenProperty.GetScreenRequestedOrientation()) << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "DPI<X, Y>: " << screenProperty.GetXDpi()
        << ", " << screenProperty.GetYDpi() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Offset<X, Y>: " << screenProperty.GetOffsetX()
        << ", " << screenProperty.GetOffsetY() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "StartPosition<X, Y>: " << screenProperty.GetStartX()
        << ", " << screenProperty.GetStartY() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "Bounds<L,T,W,H>: "
        << screenProperty.GetBounds().rect_.GetLeft() << ", "
        << screenProperty.GetBounds().rect_.GetTop() << ", "
        << screenProperty.GetBounds().rect_.GetWidth() << ", "
        << screenProperty.GetBounds().rect_.GetHeight() << ", " << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "PhyBounds<L,T,W,H>: "
        << screenProperty.GetPhyBounds().rect_.GetLeft() << ", "
        << screenProperty.GetPhyBounds().rect_.GetTop() << ", "
        << screenProperty.GetPhyBounds().rect_.GetWidth() << ", "
        << screenProperty.GetPhyBounds().rect_.GetHeight() << ", " << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "AvailableArea<X,Y,W,H> "
        << screenProperty.GetAvailableArea().posX_ << ", "
        << screenProperty.GetAvailableArea().posY_ << ", "
        << screenProperty.GetAvailableArea().width_ << ", "
        << screenProperty.GetAvailableArea().height_ << ", " << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "DefaultDeviceRotationOffset "
        << screenProperty.GetDefaultDeviceRotationOffset() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "DisplayGroupId "
        << screenProperty.GetDisplayGroupId() << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "MainDisplayIdOfGroup "
        << screenProperty.GetMainDisplayIdOfGroup() << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpScreenUserRelation(ScreenId id)
{
    const auto& manager = ScreenSessionManager::GetInstance();
    const std::map<DisplayId, std::map<int32_t, ScreenSessionManager::UserInfo>> tempMap
        = manager.GetDisplayConcurrentUserMap();
    std::vector<int32_t> userVector;
    auto it = tempMap.find(id);
    if (it == tempMap.end()) {
        return;
    }
    const auto& userMap = it->second;
    for (const auto& [userId, UserInfo] : userMap) {
        if (!manager.CheckPidInDeathPidVector(UserInfo.pid)) {
            userVector.push_back(userId);
        }
    }
    if (userVector.empty()) {
        return;
    }
 
    std::ostringstream oss;
    oss << std::left << std::setw(LINE_WIDTH) << "DisplayId:" << id << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "User:";
    for (size_t i = 0; i < userVector.size(); i++) {
        if (i != 0) {
            oss << " ";
        }
        oss << userVector[i];
    }
    oss << std::endl;

    dumpInfo_.append(oss.str());
}

/**
 * hidumper inject methods
 */
void ScreenSessionDumper::ShowNotifyFoldStatusChangedInfo()
{
    TLOGI(WmsLogTag::DMS, "params_: [%{public}s]", params_[0].c_str());
    int errCode = ScreenSessionManager::GetInstance().NotifyFoldStatusChanged(params_[0]);
    if (errCode != 0) {
        ShowIllegalArgsInfo();
    } else {
        std::ostringstream oss;
        oss << "currentFoldStatus is: "
            << static_cast<uint32_t>(ScreenSessionManager::GetInstance().GetFoldStatus())
            << std::endl;
        dumpInfo_.append(oss.str());
    }
}

void ScreenSessionDumper::ShowIllegalArgsInfo()
{
    dumpInfo_.append("The arguments are illegal and you can enter '-h' for help.");
}

void ScreenSessionDumper::SetMotionSensorValue(std::string input)
{
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_ROTATION_SENSOR)) {
        std::string valueStr = input.substr(commaPos + 1, MOTION_SENSOR_PARAM_SIZE);
        if (valueStr.size() == 1 && !std::isdigit(valueStr[0])) {
            return;
        }
        if (valueStr.size() == MOTION_SENSOR_PARAM_SIZE && valueStr != "-1") {
            return;
        }
        int32_t value = std::stoi(valueStr);
        if (value <  static_cast<int32_t>(DeviceRotation::INVALID) ||
            value > static_cast<int32_t>(DeviceRotation::ROTATION_LANDSCAPE_INVERTED)) {
            TLOGE(WmsLogTag::DMS, "params is invalid: %{public}d", value);
            return;
        }
        ScreenRotationProperty::HandleSensorEventInput(static_cast<DeviceRotation>(value));
        TLOGI(WmsLogTag::DMS, "mock motion sensor: %{public}d", value);
    }
}

void ScreenSessionDumper::SetRotationLockedValue(std::string input)
{
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_ROTATION_LOCK)) {
        std::string valueStr = input.substr(commaPos + 1);
        if (valueStr.size() != 1) {
            return;
        }
        if (!std::isdigit(valueStr[0])) {
            return;
        }
        int32_t value = std::stoi(valueStr);
        ScreenSessionManager::GetInstance().SetScreenRotationLocked(static_cast<bool>(value));
        TLOGI(WmsLogTag::DMS, "mock rotation locked: %{public}d", value);
    }
}

void ScreenSessionDumper::MockSendCastPublishEvent(std::string input)
{
    std::ostringstream oss;
    oss << "-------------- DMS SEND CAST PUBLISH EVENT --------------" << std::endl;
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_PUBLISH_CAST_EVENT)) {
        std::string valueStr = input.substr(commaPos + 1);
        if (valueStr.size() != 1) {
            oss << std::left << "[error]: " << "the value is too long" << std::endl;
            dumpInfo_.append(oss.str());
            return;
        }
        if (!std::isdigit(valueStr[0])) {
            oss << std::left << "[error]: " << "value is not a number" << std::endl;
            dumpInfo_.append(oss.str());
            return;
        }
        int32_t value = std::stoi(valueStr);
        ScreenSessionManager::GetInstance().NotifyCastWhenScreenConnectChange(static_cast<bool>(value));
        oss << std::left << "[success]: " << "send cast publish event success" << std::endl;
    } else {
        oss << std::left << "[error]: " << "the command is invalid" << std::endl;
    }
    dumpInfo_.append(oss.str());
}

bool ScreenSessionDumper::IsValidDisplayModeCommand(std::string command)
{
    if (std::find(displayModeCommands.begin(), displayModeCommands.end(), command) != displayModeCommands.end()) {
        return true;
    }
    return false;
}

int32_t ScreenSessionDumper::SetFoldDisplayMode()
{
    std::string modeParam = params_[0];
    if (modeParam.empty()) {
        return -1;
    }
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    if (modeParam == ARG_FOLD_DISPLAY_FULL) {
        displayMode = FoldDisplayMode::FULL;
    } else if (modeParam == ARG_FOLD_DISPLAY_MAIN) {
        displayMode = FoldDisplayMode::MAIN;
    } else if (modeParam == ARG_FOLD_DISPLAY_SUB) {
        displayMode = FoldDisplayMode::SUB;
    } else if (modeParam == ARG_FOLD_DISPLAY_COOR) {
        displayMode = FoldDisplayMode::COORDINATION;
    } else if (modeParam == ARG_FOLD_DISPLAY_GLOBALL_FULL) {
        displayMode = FoldDisplayMode::GLOBAL_FULL;
    } else {
        TLOGW(WmsLogTag::DMS, "SetFoldDisplayMode mode not support");
        return -1;
    }
    ScreenSessionManager::GetInstance().SetFoldDisplayMode(displayMode);
    return 0;
}

int ScreenSessionDumper::SetFoldStatusLocked()
{
    std::string lockParam = params_[0];
    if (lockParam.empty()) {
        return -1;
    }
    bool lockDisplayStatus = false;
    if (lockParam == ARG_LOCK_FOLD_DISPLAY_STATUS) {
        lockDisplayStatus = true;
    } else if (lockParam == ARG_UNLOCK_FOLD_DISPLAY_STATUS) {
        lockDisplayStatus = false;
    } else {
        TLOGW(WmsLogTag::DMS, "SetFoldStatusLocked status not support");
        return -1;
    }
    ScreenSessionManager::GetInstance().SetFoldStatusLocked(lockDisplayStatus);
    return 0;
}

int ScreenSessionDumper::ForceSetFoldStatusAndLock(std::string& input)
{
    if (input.empty()) {
        return -1;
    }
    size_t commaPos = input.find(',');
    if (commaPos == std::string::npos) {
        TLOGW(WmsLogTag::DMS, "Input format is incorrect, expected '-ln,value'");
        return -1;
    }
    std::string prefix = input.substr(0, commaPos);
    std::string valueStr = input.substr(commaPos + 1);
    if (prefix != ARG_FORCE_SET_FOLD_STATUS_AND_LOCK) {
        TLOGW(WmsLogTag::DMS, "Invalid prefix, expect '-ln'");
        return -1;
    }

    std::unordered_map<std::string, FoldStatus> stringToEnmu = {
        {"1", FoldStatus::EXPAND},
        {"2", FoldStatus::FOLDED},
        {"3", FoldStatus::HALF_FOLD},
        {"11", FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND},
        {"21", FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED},
        {"12", FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND},
        {"22", FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED},
        {"13", FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND},
        {"23", FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED},
    };
    if (stringToEnmu.find(valueStr) == stringToEnmu.end()) {
        TLOGW(WmsLogTag::DMS, "Illeagle input: %{public}s fold status param!", valueStr.c_str());
        return -1;
    }
    DMError ret = ScreenSessionManager::GetInstance().ForceSetFoldStatusAndLock(stringToEnmu[valueStr]);
    if (ret != DMError::DM_OK) {
        return -1;
    }
    return 0;
}

int ScreenSessionDumper::RestorePhysicalFoldStatus()
{
    DMError ret = ScreenSessionManager::GetInstance().RestorePhysicalFoldStatus();
    if (ret != DMError::DM_OK) {
        return -1;
    }
    return 0;
}

void ScreenSessionDumper::SetEnterOrExitTentMode(std::string input)
{
#ifdef FOLD_ABILITY_ENABLE
    if (input == ARG_SET_ON_TENT_MODE) {
        ScreenSessionManager::GetInstance().OnTentModeChanged(true);
    } else if (input == ARG_SET_OFF_TENT_MODE) {
        ScreenSessionManager::GetInstance().OnTentModeChanged(false);
    }
#endif
}

void ScreenSessionDumper::SetHoverStatusChange(std::string input)
{
    size_t commaPos = input.find_last_of(',');
    auto screenSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_HOVER_STATUS)) {
        std::string valueStr = input.substr(commaPos + 1);
        if (valueStr.size() != 1) {
            dumpInfo_.append("[error]: the value is too long");
            return;
        }
        if (!std::isdigit(valueStr[0])) {
            dumpInfo_.append("[error]: value is not a number");
            return;
        }
        int32_t value = std::stoi(valueStr);
        if ((value < static_cast<int32_t>(DeviceHoverStatus::INVALID)) ||
            (value > static_cast<int32_t>(DeviceHoverStatus::CAMERA_STATUS_CANCEL))) {
            TLOGE(WmsLogTag::DMS, "params is invalid: %{public}d", value);
            return;
        }
        screenSession->HoverStatusChange(value);
        TLOGI(WmsLogTag::DMS, "SetHoverStatusChange: %{public}d", value);
    }
}

void ScreenSessionDumper::SetHallAndPostureValue(std::string input)
{
#ifdef FOLD_ABILITY_ENABLE
    std::string token;
    std::istringstream ss(input);
    std::vector<std::string> tokens;
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    if (tokens.size() != DUMPER_PARAM_INDEX_THREE && tokens[0] != ARG_SET_POSTURE_HALL) {
        TLOGE(WmsLogTag::DMS, "param error: %{public}s", input.c_str());
        return;
    }
    if (!IsNumber(tokens[DUMPER_PARAM_INDEX_ONE]) || !IsNumber(tokens[DUMPER_PARAM_INDEX_TWO])) {
        TLOGE(WmsLogTag::DMS, "param error: %{public}s", input.c_str());
        return;
    }
    int hallVal = stoi(tokens[DUMPER_PARAM_INDEX_ONE]);
    int postureVal = stoi(tokens[DUMPER_PARAM_INDEX_TWO]);
    DMS::ExtHallData hallData = {
        .flag = (1 << 1),
        .hall = hallVal,
    };
    PostureData postureData = {
        .angle = postureVal,
    };
    SensorEvent hallEvent = {
        .data = reinterpret_cast<uint8_t *>(&hallData),
        .dataLen = sizeof(DMS::ExtHallData),
    };
    SensorEvent postureEvent = {
        .data = reinterpret_cast<uint8_t *>(&postureData),
        .dataLen = sizeof(PostureData),
    };
    if (FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        DMS::ScreenSensorMgr::GetInstance().HandleHallData(&hallEvent);
        DMS::ScreenSensorMgr::GetInstance().HandlePostureData(&postureEvent);
    } else {
        FoldScreenSensorManager::GetInstance().HandleHallData(&hallEvent);
        FoldScreenSensorManager::GetInstance().HandlePostureData(&postureEvent);
    }
    
    TLOGI(WmsLogTag::DMS, "mock posture: %{public}d, hall: %{public}d ", postureVal, hallVal);
#endif
}

void ScreenSessionDumper::SetHallAndPostureStatus(std::string input)
{
#ifdef FOLD_ABILITY_ENABLE
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_POSTURE_HALL_STATUS)) {
        std::string valueStr = input.substr(commaPos + 1, DUMPER_PARAM_INDEX_ONE);
        if (valueStr.size() != DUMPER_PARAM_INDEX_ONE && !std::isdigit(valueStr[0])) {
            return;
        }
        int32_t value = std::stoi(valueStr);
        if (value) {
            if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
                SuperFoldSensorManager::GetInstance().RegisterPostureCallback();
                SuperFoldSensorManager::GetInstance().RegisterHallCallback();
            } else {
                DMS::ScreenSensorMgr::GetInstance().RegisterHallCallback();
                DMS::ScreenSensorMgr::GetInstance().RegisterPostureCallback();
                ScreenSensorConnector::SubscribeRotationSensor();
            }
        } else {
            if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
                SuperFoldSensorManager::GetInstance().UnregisterPostureCallback();
                SuperFoldSensorManager::GetInstance().UnregisterHallCallback();
            } else {
                DMS::ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
                DMS::ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
                ScreenSensorConnector::UnsubscribeRotationSensor();
            }
        }
        TLOGI(WmsLogTag::DMS, "hall and posture register status: %{public}d", value);
    }
#endif
}

void ScreenSessionDumper::SetSuperFoldStatusChange(std::string input)
{
#ifdef FOLD_ABILITY_ENABLE
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_SUPER_FOLD_STATUS)) {
        std::string valueStr = input.substr(commaPos + 1, SUPER_FOLD_STATUS_MAX);
        if (valueStr.empty()) {
            return;
        }
        if (valueStr.size() == 1 && !std::isdigit(valueStr[0])) {
            return;
        }
        if (valueStr.size() == SUPER_FOLD_STATUS_MAX && !std::isdigit(valueStr[0])
            && !std::isdigit(valueStr[1])) {
            return;
        }
        int32_t value = std::stoi(valueStr);
        if (value <=  static_cast<int32_t>(SuperFoldStatusChangeEvents::UNDEFINED) ||
            value >= static_cast<int32_t>(SuperFoldStatusChangeEvents::INVALID)) {
            TLOGE(WmsLogTag::DMS, "params is invalid: %{public}d", value);
            return;
        }
        SuperFoldStateManager::GetInstance().
            HandleSuperFoldStatusChange(static_cast<SuperFoldStatusChangeEvents>(value));
        TLOGI(WmsLogTag::DMS, "state: %{public}d, event: %{public}d",
            SuperFoldStateManager::GetInstance().GetCurrentStatus(), value);
    }
#endif
}

void ScreenSessionDumper::SetSecondaryStatusChange(const std::string &input)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        TLOGD(WmsLogTag::DMS, "not secondary device");
        return;
    }
    TLOGI(WmsLogTag::DMS, "secondary input: %{public}s", input.c_str());
    size_t commaPos = input.find(',');
    if (!((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_SECONDARY_FOLD_STATUS))) {
        return;
    }
    std::string valueStr = input.substr(commaPos + 1, 1);
    if (valueStr.empty()) {
        return;
    }
    char ch = valueStr[0];
    TLOGI(WmsLogTag::DMS, "value: %{public}c", ch);
    const char *end = SECONDARY_DUMPER_VALUE_BOUNDARY + sizeof(SECONDARY_DUMPER_VALUE_BOUNDARY); // mfg
    const char *result = std::find(SECONDARY_DUMPER_VALUE_BOUNDARY, end, ch);
    if (result == end) {
        if (ch == 'p') { // sensor
            TriggerSecondarySensor(input.substr(commaPos + 1, input.size() - commaPos - 1));
        } else if (ch == 'z') { // foldstatus
            TriggerSecondaryFoldStatus(input.substr(commaPos + 1, input.size() - commaPos - 1));
        } else {
            TLOGE(WmsLogTag::DMS, "command not supported.");
        }
        return;
    }

    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    if (ch == SECONDARY_DUMPER_VALUE_BOUNDARY[SECONDARY_FOLD_STATUS_INDEX_M]) {
        displayMode = FoldDisplayMode::FULL;
    } else if (ch == SECONDARY_DUMPER_VALUE_BOUNDARY[SECONDARY_FOLD_STATUS_INDEX_F]) {
        displayMode = FoldDisplayMode::MAIN;
    } else if (ch == SECONDARY_DUMPER_VALUE_BOUNDARY[SECONDARY_FOLD_STATUS_INDEX_G]) {
        displayMode = FoldDisplayMode::GLOBAL_FULL;
    } else {
        TLOGW(WmsLogTag::DMS, "SetFoldDisplayMode mode is not supported.");
        return;
    }
    ScreenSessionManager::GetInstance().SetFoldDisplayMode(displayMode);
#endif
}

void ScreenSessionDumper::SetLandscapeLock(std::string input)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "not super fold device");
        return;
    }
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_LANDSCAPE_LOCK)) {
        std::string valueStr = input.substr(commaPos + 1, DUMPER_PARAM_INDEX_ONE);
        if (valueStr.size() != DUMPER_PARAM_INDEX_ONE || !std::isdigit(valueStr[0])) {
            return;
        }
        int32_t value = std::stoi(valueStr);
        if (value) {
            ScreenSessionManager::GetInstance().SetExpandAndHorizontalLocked(true);
        } else {
            ScreenSessionManager::GetInstance().SetExpandAndHorizontalLocked(false);
        }
        TLOGI(WmsLogTag::DMS, "landscape lock: %{public}d", value);
    }
#endif
}

void ScreenSessionDumper::SetDuringCallState(std::string input)
{
#ifdef FOLD_ABILITY_ENABLE
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_DURINGCALL_STATE)) {
        std::string valueStr = input.substr(commaPos + 1);
        if (valueStr.size() != 1) {
            dumpInfo_.append("[error]: the value is too long");
            return;
        }
        if (!std::isdigit(valueStr[0])) {
            dumpInfo_.append("[error]: value is not a number");
            return;
        }
        if ((valueStr[0] != '0') && (valueStr[0] != '1')) {
            TLOGE(WmsLogTag::DMS, "param is invalid: %{public}s", valueStr.c_str());
            return;
        }
        bool value = static_cast<bool>(std::stoi(valueStr));
        ScreenSessionManager::GetInstance().SetDuringCallState(value);
        TLOGI(WmsLogTag::DMS, "SetDuringCallState: %{public}d", value);
    }
#endif
}

#ifdef FOLD_ABILITY_ENABLE
bool ScreenSessionDumper::IsAllCharDigit(const std::string &firstPostureStr)
{
    for (size_t i = 0; i < firstPostureStr.size(); i++) {
        if (!std::isdigit(firstPostureStr[i])) {
            TLOGW(WmsLogTag::DMS, "%{public}s is not number", firstPostureStr.c_str());
            return false;
        }
    }
    return true;
}

bool ScreenSessionDumper::GetPostureAndHall(std::vector<std::string> strVec,
    std::vector<float> &postures, std::vector<uint16_t> &halls)
{
    for (std::string str : strVec) {
        size_t index = str.find(":");
        if (index == std::string::npos) {
            return false;
        }
        std::string key = str.substr(0, index);
        std::string value = str.substr(index + 1, str.size() - index - 1);
        index = value.find(",");
        if (index == std::string::npos) {
            return false;
        }
        std::vector<std::string> posturesStrList = WindowHelper::Split(value, ",");
        std::string firstPostureStr;
        std::string secondPostureStr;
        std::string thirdPostureStr;
        if (posturesStrList.size() == DUMPER_PARAM_INDEX_THREE) {
            firstPostureStr = posturesStrList[0];
            secondPostureStr = posturesStrList[1];
            thirdPostureStr = posturesStrList[DUMPER_PARAM_INDEX_TWO];
            if (!WindowHelper::IsFloatingNumber(firstPostureStr) ||
                !WindowHelper::IsFloatingNumber(secondPostureStr)) {
                TLOGW(WmsLogTag::DMS, "posture should be a float number");
                return false;
            }
            if (thirdPostureStr != "0" && thirdPostureStr != "1") {
                TLOGW(WmsLogTag::DMS, "third posture is not 0 or 1");
                return false;
            }
            postures.emplace_back(std::stof(firstPostureStr));
            postures.emplace_back(std::stof(secondPostureStr));
            postures.emplace_back(std::stof(thirdPostureStr));
        } else if (posturesStrList.size() == DUMPER_PARAM_INDEX_TWO) {
            firstPostureStr = posturesStrList[0];
            secondPostureStr = posturesStrList[1];
            if (!IsAllCharDigit(firstPostureStr) || !IsAllCharDigit(secondPostureStr)) {
                return false;
            }
            halls.emplace_back(static_cast<uint16_t>(std::stoi(firstPostureStr)));
            halls.emplace_back(static_cast<uint16_t>(std::stoi(secondPostureStr)));
        } else {
            TLOGW(WmsLogTag::DMS, "sensor command error");
            return false;
        }
    }
    return true;
}

void ScreenSessionDumper::TriggerSecondarySensor(const std::string &valueStr)
{
    TLOGI(WmsLogTag::DMS, "%{public}s", valueStr.c_str());
    std::vector<std::string> strVec = WindowHelper::Split(valueStr, "/");
    std::vector<float> postures;
    std::vector<uint16_t> halls;
    if (!GetPostureAndHall(strVec, postures, halls)) {
        TLOGW(WmsLogTag::DMS, "GetPostureAndHall failed");
        return;
    }
    DMS::ExtHallData hallData = {
        .flag = HALL_EXT_DATA_FLAG,
        .hall = halls[0],
        .hallAb = halls[1],
    };
    DMS::PostureDataSecondary postureData = {
        .postureBc = postures[0],
        .postureAb = postures[1],
        .postureAbAnti = postures[DUMPER_PARAM_INDEX_TWO],
    };
    SensorEvent hallEvent = {
        .data = reinterpret_cast<uint8_t *>(&hallData),
        .dataLen = sizeof(DMS::ExtHallData),
    };
    SensorEvent postureEvent = {
        .data = reinterpret_cast<uint8_t *>(&postureData),
        .dataLen = sizeof(DMS::PostureDataSecondary),
    };
    TLOGI(WmsLogTag::DMS, "mock secondary sensor: %{public}s, %{public}s",
        FoldScreenStateInternel::TransVec2Str(postures, ANGLE_STR).c_str(),
        FoldScreenStateInternel::TransVec2Str(halls, HALL_STR).c_str());
    SecondaryFoldSensorManager::GetInstance().HandleHallDataExt(&hallEvent);
    SecondaryFoldSensorManager::GetInstance().HandlePostureData(&postureEvent);
}

void ScreenSessionDumper::TriggerSecondaryFoldStatus(const std::string &valueStr)
{
    std::vector<std::string> strVec = WindowHelper::Split(valueStr, "=");
    if (strVec.size() != SECONDARY_FOLD_STATUS_COMMAND_NUM) {
        TLOGW(WmsLogTag::DMS, "secondary foldstatus command miss '=' or has extra '='.");
        return;
    }
    const std::string &foldStatusStr = strVec[1];
    for (size_t i = 0; i < foldStatusStr.size(); i++) {
        if (!std::isdigit(foldStatusStr[i])) {
            TLOGW(WmsLogTag::DMS, "secondary foldstatus command contains characters that are not numbers.");
            return;
        }
    }
    uint32_t foldStatus = static_cast<uint32_t>(std::stoi(foldStatusStr));
    switch (foldStatus) {
        case static_cast<uint32_t>(FoldStatus::EXPAND) :
        case static_cast<uint32_t>(FoldStatus::FOLDED) :
        case static_cast<uint32_t>(FoldStatus::HALF_FOLD) :
        case static_cast<uint32_t>(FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND) :
        case static_cast<uint32_t>(FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED) :
        case static_cast<uint32_t>(FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND) :
        case static_cast<uint32_t>(FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED) :
        case static_cast<uint32_t>(FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND) :
        case static_cast<uint32_t>(FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED) : {
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "secondary foldstatus is out of range.");
            return;
        }
    }
    TLOGI(WmsLogTag::DMS, "change fold status, %{public}s", foldStatusStr.c_str());
    ScreenSessionManager::GetInstance().TriggerFoldStatusChange(static_cast<FoldStatus>(foldStatus));
}
#endif
} // Rosen
} // OHOS