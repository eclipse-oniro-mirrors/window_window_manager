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
#include "screen_sensor_connector.h"
#include "parameters.h"
#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_state_internel.h"
#include "window_helper.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int LINE_WIDTH = 30;
constexpr int DUMPER_PARAM_INDEX_ONE = 1;
constexpr int DUMPER_PARAM_INDEX_TWO = 2;
constexpr int DUMPER_PARAM_INDEX_THREE = 3;
const std::string ARG_DUMP_HELP = "-h";
const std::string ARG_DUMP_ALL = "-a";
const std::string ARG_DUMP_FOLD_STATUS = "-f";

constexpr int MOTION_SENSOR_PARAM_SIZE = 2;
const std::string STATUS_FOLD_HALF = "-z";
const std::string STATUS_EXPAND = "-y";
const std::string STATUS_FOLD = "-p";
const std::string ARG_SET_ROTATION_SENSOR = "-motion"; // rotation event inject
const std::string ARG_SET_ROTATION_LOCK = "-rotationlock";
const std::string ARG_FOLD_DISPLAY_FULL = "-f";
const std::string ARG_FOLD_DISPLAY_MAIN = "-m";
const std::string ARG_FOLD_DISPLAY_GLOBALL_FULL = "-g";
const std::string ARG_FOLD_DISPLAY_SUB = "-sub";
const std::string ARG_FOLD_DISPLAY_COOR = "-coor";
const std::vector<std::string> displayModeCommands = {"-f", "-m", "-sub", "-coor", "-g"};
const std::string ARG_LOCK_FOLD_DISPLAY_STATUS = "-l";
const std::string ARG_UNLOCK_FOLD_DISPLAY_STATUS = "-u";
const std::string ARG_SET_ON_TENT_MODE = "-ontent";
const std::string ARG_SET_OFF_TENT_MODE = "-offtent";
const std::string ARG_SET_HOVER_STATUS = "-hoverstatus";
const std::string ARG_SET_POSTURE_HALL = "-posture";
const std::string ARG_SET_POSTURE_HALL_STATUS = "-registerhall"; // 关闭开合sensor报值
const std::string ARG_CHANGE_OUTER_CMD = "outer";
const std::string ARG_SET_SECONDARY_FOLD_STATUS = "-secondary";
const char SECONDARY_DUMPER_VALUE_BOUNDARY[] = "mfg";
constexpr size_t SECONDARY_FOLD_STATUS_INDEX_M = 0;
constexpr size_t SECONDARY_FOLD_STATUS_INDEX_F = 1;
constexpr size_t SECONDARY_FOLD_STATUS_INDEX_G = 2;
constexpr size_t SECONDARY_FOLD_STATUS_COMMAND_NUM = 2;
constexpr uint16_t HALL_EXT_DATA_FLAG = 26;
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
    for (auto& u16str: args) {
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

void ScreenSessionDumper::OutputDumpInfo()
{
    if (fd_ < 0) {
        TLOGE(WmsLogTag::DMS, "invalid fd: %{public}d", fd_);
        return;
    }

    static_cast<void>(signal(SIGPIPE, SIG_IGN)); // ignore SIGPIPE crash
    int ret = dprintf(fd_, "%s\n", dumpInfo_.c_str());
    if (ret < 0) {
        TLOGE(WmsLogTag::DMS, "dprintf error. ret: %{public}d", ret);
        return;
    }
    dumpInfo_.clear();
}


void ScreenSessionDumper::ExcuteDumpCmd()
{
    if (params_.empty() || params_[0] == ARG_DUMP_HELP) { //help command
        ShowHelpInfo();
    }

    if (!(SessionPermission::IsSACalling() || SessionPermission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::DMS, "dump permission denied!");
        return;
    }
    if (!params_.empty() && params_[0] == ARG_DUMP_ALL) { // dump all info command
        ShowAllScreenInfo();
    } else if (!params_.empty() && params_[0] == ARG_DUMP_FOLD_STATUS) { // dump fold status command
        DumpFoldStatus();
    }
    ExcuteInjectCmd();
    OutputDumpInfo();
}

void ScreenSessionDumper::ExcuteInjectCmd()
{
    bool isDeveloperMode = system::GetBoolParameter("const.security.developermode.state", false);
    if (isDeveloperMode) {
        if (params_.size() == 1 && IsValidDisplayModeCommand(params_[0])) {
            int errCode = SetFoldDisplayMode();
            if (errCode != 0) {
                ShowIllegalArgsInfo();
            }
            return;
        }
    }

    bool isDebugMode = system::GetBoolParameter("dms.hidumper.supportdebug", false);
    if (!isDebugMode) {
        TLOGI(WmsLogTag::DMS, "Can't use DMS hidumper inject methods.");
        dumpInfo_.append("dms.hidumper.supportdebug false\n");
        return;
    }
    if (params_[0] == STATUS_FOLD_HALF || params_[0] == STATUS_EXPAND || params_[0] == STATUS_FOLD) {
        ShowNotifyFoldStatusChangedInfo();
        return;
    } else if (params_[0].find(ARG_SET_ROTATION_SENSOR) != std::string::npos) {
        SetMotionSensorvalue(params_[0]);
        return;
    } else if (params_[0].find(ARG_SET_ROTATION_LOCK) != std::string::npos) {
        SetRotationLockedvalue(params_[0]);
        return;
    } else if (params_.size() == 1 && (params_[0] == ARG_LOCK_FOLD_DISPLAY_STATUS
                || params_[0] == ARG_UNLOCK_FOLD_DISPLAY_STATUS)) {
        int errCode = SetFoldStatusLocked();
        if (errCode != 0) {
            ShowIllegalArgsInfo();
        }
        return;
    }
    ExcuteInjectCmd2();
}

void ScreenSessionDumper::ExcuteInjectCmd2()
{
    if (params_[0].find(ARG_SET_ON_TENT_MODE) != std::string::npos ||
        params_[0].find(ARG_SET_OFF_TENT_MODE) != std::string::npos) {
        SetEnterOrExitTentMode(params_[0]);
    } else if (params_[0].find(ARG_SET_HOVER_STATUS) != std::string::npos) {
        SetHoverStatusChange(params_[0]);
    } else if (params_[0].find(ARG_SET_POSTURE_HALL) != std::string::npos) {
        SetHallAndPostureValue(params_[0]);
    } else if (params_[0].find(ARG_SET_POSTURE_HALL_STATUS) != std::string::npos) {
        SetHallAndPostureStatus(params_[0]);
    } else if (params_[0].find(ARG_SET_SECONDARY_FOLD_STATUS) != std::string::npos) {
        SetSecondaryStatusChange(params_[0]);
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
        .append(" -p                             ")
        .append("|switch to fold status\n")
        .append(" -f                             ")
        .append("|get to fold status\n");
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
    }
}

void ScreenSessionDumper::DumpFoldStatus()
{
    std::ostringstream oss;
    auto foldStatus = ScreenSessionManager::GetInstance().GetFoldStatus();
    std::string status = "";
    switch (foldStatus) {
        case FoldStatus::EXPAND: {
            status = "EXPAND";
            break;
        }
        case FoldStatus::FOLDED: {
            status = "FOLDED";
            break;
        }
        case FoldStatus::HALF_FOLD: {
            status = "HALF_FOLD";
            break;
        }
        default: {
            status = "UNKNOWN";
            break;
        }
    }
    oss << std::left << std::setw(LINE_WIDTH) << "FoldStatus: "
        << status << std::endl;
    dumpInfo_.append(oss.str());
}

void ScreenSessionDumper::DumpTentMode()
{
    std::ostringstream oss;
    bool isTentMode = ScreenSessionManager::GetInstance().GetTentMode();
    std::string status = "";
    if (isTentMode) {
        status = "TRUE";
    } else {
        status = "FALSE";
    }
    oss << std::left << std::setw(LINE_WIDTH) << "TentMode: "
        << status << std::endl;
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
    oss << std::left << std::setw(LINE_WIDTH) << "RSScreenId: "
        << screenSession->GetRSScreenId() << std::endl;
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

void ScreenSessionDumper::DumpCutoutInfoById(ScreenId id)
{
    std::ostringstream oss;
    oss << "[CUTOUT INFO]" << std::endl;
    sptr<CutoutInfo> cutoutInfo = ScreenSessionManager::GetInstance().GetCutoutInfo(id);
    if (cutoutInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "cutoutInfo nullptr. screen id: %{public}" PRIu64"", id);
        return;
    }
    oss << std::left << std::setw(LINE_WIDTH) << "WaterFall_L<X,Y,W,H>: "
        << cutoutInfo->GetWaterfallDisplayAreaRects().left.posX_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().left.posY_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().left.width_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().left.height_ << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "WaterFall_T<X,Y,W,H>: "
        << cutoutInfo->GetWaterfallDisplayAreaRects().top.posX_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().top.posY_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().top.width_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().top.height_ << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "WaterFall_R<X,Y,W,H>: "
        << cutoutInfo->GetWaterfallDisplayAreaRects().right.posX_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().right.posY_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().right.width_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().right.height_ << std::endl;
    oss << std::left << std::setw(LINE_WIDTH) << "WaterFall_B<X,Y,W,H>: "
        << cutoutInfo->GetWaterfallDisplayAreaRects().bottom.posX_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().bottom.posY_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().bottom.width_ << ", "
        << cutoutInfo->GetWaterfallDisplayAreaRects().bottom.height_ << std::endl;

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
    dumpInfo_.append(oss.str());
}

/*
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

void ScreenSessionDumper::SetMotionSensorvalue(std::string input)
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
        int32_t value = static_cast<uint32_t>(std::stoi(valueStr));
        if (value <  static_cast<int32_t>(DeviceRotation::INVALID) ||
            value > static_cast<int32_t>(DeviceRotation::ROTATION_LANDSCAPE_INVERTED)) {
            TLOGE(WmsLogTag::DMS, "params is invalid: %{public}d", value);
            return;
        }
        ScreenRotationProperty::HandleSensorEventInput(static_cast<DeviceRotation>(value));
        TLOGI(WmsLogTag::DMS, "mock motion sensor: %{public}d", value);
    }
}

void ScreenSessionDumper::SetRotationLockedvalue(std::string input)
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

bool ScreenSessionDumper::IsValidDisplayModeCommand(std::string command)
{
    if (std::find(displayModeCommands.begin(), displayModeCommands.end(), command) != displayModeCommands.end()) {
        return true;
    }
    return false;
}

int ScreenSessionDumper::SetFoldDisplayMode()
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

void ScreenSessionDumper::SetEnterOrExitTentMode(std::string input)
{
    if (input == ARG_SET_ON_TENT_MODE) {
        ScreenSessionManager::GetInstance().OnTentModeChanged(true);
    } else if (input == ARG_SET_OFF_TENT_MODE) {
        ScreenSessionManager::GetInstance().OnTentModeChanged(false);
    }
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
    FoldScreenSensorManager::ExtHallData hallData = {
        .flag = (1 << 1),
        .hall = hallVal,
    };
    PostureData postureData = {
        .angle = postureVal,
    };
    SensorEvent hallEvent = {
        .data = reinterpret_cast<uint8_t *>(&hallData),
        .dataLen = sizeof(FoldScreenSensorManager::ExtHallData),
    };
    SensorEvent postureEvent = {
        .data = reinterpret_cast<uint8_t *>(&postureData),
        .dataLen = sizeof(PostureData),
    };
    FoldScreenSensorManager::GetInstance().HandleHallData(&hallEvent);
    FoldScreenSensorManager::GetInstance().HandlePostureData(&postureEvent);
    TLOGI(WmsLogTag::DMS, "mock posture: %{public}d, hall: %{public}d ", postureVal, hallVal);
}

void ScreenSessionDumper::SetHallAndPostureStatus(std::string input)
{
    size_t commaPos = input.find_last_of(',');
    if ((commaPos != std::string::npos) && (input.substr(0, commaPos) == ARG_SET_POSTURE_HALL_STATUS)) {
        std::string valueStr = input.substr(commaPos + 1, DUMPER_PARAM_INDEX_ONE);
        if (valueStr.size() != DUMPER_PARAM_INDEX_ONE && !std::isdigit(valueStr[0])) {
            return;
        }
        int32_t value = std::stoi(valueStr);
        if (value) {
            FoldScreenSensorManager::GetInstance().RegisterHallCallback();
            FoldScreenSensorManager::GetInstance().RegisterPostureCallback();
            ScreenSensorConnector::SubscribeRotationSensor();
        } else {
            FoldScreenSensorManager::GetInstance().UnRegisterHallCallback();
            FoldScreenSensorManager::GetInstance().UnRegisterPostureCallback();
            ScreenSensorConnector::UnsubscribeRotationSensor();
        }
        TLOGI(WmsLogTag::DMS, "hall and posture register status: %{public}d", value);
    }
}

void ScreenSessionDumper::SetSecondaryStatusChange(const std::string &input)
{
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
}

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
    FoldScreenSensorManager::ExtHallData hallData = {
        .flag = HALL_EXT_DATA_FLAG,
        .hall = halls[0],
        .hallAb = halls[1],
    };
    FoldScreenSensorManager::PostureDataSecondary postureData = {
        .postureBc = postures[0],
        .postureAb = postures[1],
        .postureAbAnti = postures[DUMPER_PARAM_INDEX_TWO],
    };
    SensorEvent hallEvent = {
        .data = reinterpret_cast<uint8_t *>(&hallData),
        .dataLen = sizeof(FoldScreenSensorManager::ExtHallData),
    };
    SensorEvent postureEvent = {
        .data = reinterpret_cast<uint8_t *>(&postureData),
        .dataLen = sizeof(FoldScreenSensorManager::PostureDataSecondary),
    };
    TLOGI(WmsLogTag::DMS, "mock secondary sensor: %{public}s, %{public}s",
        FoldScreenStateInternel::TransVec2Str(postures, "angle").c_str(),
        FoldScreenStateInternel::TransVec2Str(halls, "hall").c_str());
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
} // Rosen
} // OHOS