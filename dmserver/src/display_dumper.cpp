/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "display_dumper.h"

#include <cinttypes>
#include <csignal>
#include <iomanip>
#include <map>
#include <sstream>
#include <string_ex.h>
#include <unique_fd.h>

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {

constexpr int SCREEN_NAME_MAX_LENGTH = 20;
const std::string ARG_DUMP_HELP = "-h";
const std::string ARG_DUMP_ALL = "-a";
const std::string ARG_DUMP_SCREEN = "-s";
const std::string ARG_DUMP_DISPLAY = "-d";
// set the output width of screen
constexpr int W_SCREEN_NAME = 21;
constexpr int W_SCREEN_TYPE = 9;
constexpr int W_GROUP_TYPE = 8;
constexpr int W_DMS_ID = 6;
constexpr int W_RS_ID = 21;
constexpr int W_ACTIVE_IDX = 10;
constexpr int W_VIR_PIXEL_RATIO = 4;
constexpr int W_SCREEN_ROTATION = 9;
constexpr int W_ORIENTATION = 12;
constexpr int W_REQUESTED_ORIENTATION = 19;
constexpr int W_NODE_ID = 21;
constexpr int W_MIRROR_TYPE = 11;
constexpr int W_MIRROR_NODE_ID = 13;
// set the output width of display
constexpr int W_DISPLAY_ID = 10;
constexpr int W_ABSTR_SCREEN_ID = 9;
constexpr int W_REFRESH_RATE = 12;
constexpr int W_DISPLAY_ROTATION = 9;
constexpr int W_DISPLAY_ORIENTATION = 18;
constexpr int W_DISPLAY_FREEZE_FLAG = 11;
constexpr int W_DISPLAY_OFFSET_X = 5;
constexpr int W_DISPLAY_OFFSET_Y = 5;
constexpr int W_DISPLAY_WIDTH = 5;
constexpr int W_DISPLAY_HEITHT = 5;
constexpr int DECIMAL_NUM = 10;
}

DisplayDumper::DisplayDumper(const sptr<AbstractDisplayController>& abstractDisplayController,
    const sptr<AbstractScreenController>& abstractScreenController, std::recursive_mutex& mutex)
    : abstractDisplayController_(abstractDisplayController), abstractScreenController_(abstractScreenController),
    mutex_(mutex)
{
}

DMError DisplayDumper::Dump(int fd, const std::vector<std::u16string>& args) const
{
    TLOGI(WmsLogTag::DMS, "Dump begin fd: %{public}d", fd);
    if (fd < 0) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    (void) signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE crash
    UniqueFd ufd = UniqueFd(fd); // auto close
    fd = ufd.Get();
    std::vector<std::string> params;
    for (auto& arg : args) {
        params.emplace_back(Str16ToStr8(arg));
    }

    std::string dumpInfo;
    if (params.empty()) {
        ShowHelpInfo(dumpInfo);
    } else if (params.size() == 1 && params[0] == ARG_DUMP_HELP) { // 1: params num
        ShowHelpInfo(dumpInfo);
    } else {
        DMError errCode = DumpInfo(params, dumpInfo);
        if (errCode != DMError::DM_OK) {
            ShowIllegalArgsInfo(dumpInfo, errCode);
        }
    }
    int ret = dprintf(fd, "%s\n", dumpInfo.c_str());
    if (ret < 0) {
        TLOGE(WmsLogTag::DMS, "dprintf error");
        return DMError::DM_ERROR_UNKNOWN;
    }
    TLOGI(WmsLogTag::DMS, "Dump end");
    return DMError::DM_OK;
}

void DisplayDumper::ShowHelpInfo(std::string& dumpInfo) const
{
    dumpInfo.append("Usage:\n")
        .append(" -h                          ")
        .append("|help text for the tool\n")
        .append(" -s -a                       ")
        .append("|dump all screen information in the system\n")
        .append(" -d -a                       ")
        .append("|dump all display information in the system\n")
        .append(" -s {screen id}              ")
        .append("|dump specified screen information\n")
        .append(" -d {display id}             ")
        .append("|dump specified display information\n");
}

void DisplayDumper::ShowIllegalArgsInfo(std::string& dumpInfo, DMError errCode) const
{
    switch (errCode) {
        case DMError::DM_ERROR_INVALID_PARAM:
            dumpInfo.append("The arguments are illegal and you can enter '-h' for help.");
            break;
        case DMError::DM_ERROR_NULLPTR:
            dumpInfo.append("The screen or display is invalid, ")
                .append("you can enter '-s -a' or '-d -a' to get valid screen or display id.");
            break;
        default:
            break;
    }
}

DMError DisplayDumper::DumpInfo(const std::vector<std::string>& args, std::string& dumpInfo) const
{
    if (args.size() != 2) { // 2: params num
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    
    if (args[0] == ARG_DUMP_SCREEN && args[1] == ARG_DUMP_ALL) {
        return DumpAllScreenInfo(dumpInfo);
    } else if (args[0] == ARG_DUMP_DISPLAY && args[1] == ARG_DUMP_ALL) {
        return DumpAllDisplayInfo(dumpInfo);
    } else if (args[0] == ARG_DUMP_SCREEN && IsValidDigitString(args[1])) {
        ScreenId screenId = strtoll(args[1].c_str(), nullptr, DECIMAL_NUM);
        return DumpSpecifiedScreenInfo(screenId, dumpInfo);
    } else if (args[0] == ARG_DUMP_DISPLAY && IsValidDigitString(args[1])) {
        DisplayId displayId = strtoll(args[1].c_str(), nullptr, DECIMAL_NUM);
        return DumpSpecifiedDisplayInfo(displayId, dumpInfo);
    } else {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
}

DMError DisplayDumper::DumpAllScreenInfo(std::string& dumpInfo) const
{
    std::map<ScreenId, sptr<AbstractScreenGroup>> screenGroups;
    std::vector<ScreenId> screenIds = abstractScreenController_->GetAllScreenIds();
    std::ostringstream oss;
    oss << "--------------------------------------Free Screen"
        << "--------------------------------------"
        << std::endl;
    oss << "ScreenName           Type     IsGroup DmsId RsId                 ActiveIdx VPR Rotation Orientation "
        << "RequestOrientation NodeId               IsMirrored MirrorNodeId"
        << std::endl;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (ScreenId screenId : screenIds) {
        auto screen = abstractScreenController_->GetAbstractScreen(screenId);
        if (screen == nullptr) {
            TLOGE(WmsLogTag::DMS, "screen is null");
            return DMError::DM_ERROR_NULLPTR;
        }
        if (SCREEN_ID_INVALID == screen->groupDmsId_ || screen->isScreenGroup_) {
            GetScreenInfo(screen, oss);
        }
        if (screen->isScreenGroup_) {
            auto screenGroup = abstractScreenController_->GetAbstractScreenGroup(screenId);
            screenGroups.insert(std::make_pair(screenId, screenGroup));
        }
    }
    oss << "total screen num: " << screenIds.size() << std::endl;
    dumpInfo.append(oss.str());
    for (auto it = screenGroups.begin(); it != screenGroups.end(); it++) {
        DMError ret = DumpScreenInfo(it->second, dumpInfo);
        if (ret != DMError::DM_OK) {
            return ret;
        }
    }
    return DMError::DM_OK;
}

DMError DisplayDumper::DumpScreenInfo(const sptr<AbstractScreenGroup>& screenGroup, std::string& dumpInfo) const
{
    if (screenGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenGroup is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    std::ostringstream oss;
    oss << "-------------------------------------ScreenGroup " << screenGroup->dmsId_
        << "-------------------------------------"
        << std::endl;
    oss << "ScreenName           Type     IsGroup DmsId RsId                 "
        << "ActiveIdx VPR Rotation Orientation "
        << "RequestOrientation NodeId               IsMirrored MirrorNodeId"
        << std::endl;
    auto childrenScreen = screenGroup->GetChildren();
    for (auto screen : childrenScreen) {
        GetScreenInfo(screen, oss);
    }
    dumpInfo.append(oss.str());
    return DMError::DM_OK;
}

DMError DisplayDumper::DumpSpecifiedScreenInfo(ScreenId screenId, std::string& dumpInfo) const
{
    auto screen = abstractScreenController_->GetAbstractScreen(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    const std::string& name = screen->GetScreenName();
    const std::string& screenName = name.size() <= SCREEN_NAME_MAX_LENGTH ?
        name : name.substr(0, SCREEN_NAME_MAX_LENGTH);
    std::string isGroup = screen->isScreenGroup_ ? "true" : "false";
    std::string screenType = TransferTypeToString(screen->type_);
    std::string isMirrored = screen->rSDisplayNodeConfig_.isMirrored ? "true" : "false";
    NodeId nodeId = (screen->rsDisplayNode_ == nullptr) ? SCREEN_ID_INVALID : screen->rsDisplayNode_->GetId();
    std::ostringstream oss;
    oss << "ScreenName: " << screenName << std::endl;
    oss << "Type: " << screenType << std::endl;
    oss << "IsGroup: " << isGroup << std::endl;
    oss << "DmsId: " << screen->dmsId_ << std::endl;
    oss << "RsId: " << screen->rsId_ << std::endl;
    oss << "GroupDmsId: " << screen->groupDmsId_ << std::endl;
    oss << "ActiveIdx: " << screen->activeIdx_ << std::endl;
    oss << "VPR: " << screen->virtualPixelRatio_ << std::endl;
    oss << "Rotation: " << static_cast<uint32_t>(screen->rotation_) << std::endl;
    oss << "Orientation: " << static_cast<uint32_t>(screen->orientation_) << std::endl;
    oss << "RequestOrientation: " << static_cast<uint32_t>(screen->screenRequestedOrientation_) << std::endl;
    oss << "NodeId: " << nodeId << std::endl;
    oss << "IsMirrored: " << isMirrored << std::endl;
    oss << "MirrorNodeId: " << screen->rSDisplayNodeConfig_.mirrorNodeId << std::endl;
    dumpInfo.append(oss.str());
    return DMError::DM_OK;
}

DMError DisplayDumper::DumpAllDisplayInfo(std::string& dumpInfo) const
{
    std::vector<DisplayId> displayIds = abstractDisplayController_->GetAllDisplayIds();
    std::ostringstream oss;
    oss << "--------------------------------------Display Info"
        << "--------------------------------------"
        << std::endl;
    oss << "DisplayId ScreenId RefreshRate VPR Rotation Orientation DisplayOrientation FreezeFlag [ x   y   w   h   ]"
        << std::endl;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (DisplayId displayId : displayIds) {
        auto display = abstractDisplayController_->GetAbstractDisplay(displayId);
        if (display == nullptr) {
            TLOGE(WmsLogTag::DMS, "display is null");
            return DMError::DM_ERROR_NULLPTR;
        }
        GetDisplayInfo(display, oss);
    }
    dumpInfo.append(oss.str());
    return DMError::DM_OK;
}

DMError DisplayDumper::DumpSpecifiedDisplayInfo(DisplayId displayId, std::string& dumpInfo) const
{
    auto display = abstractDisplayController_->GetAbstractDisplay(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "display is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    std::ostringstream oss;
    oss << "DisplayId: " << display->GetId() << std::endl;
    oss << "ScreenId: " << display->GetAbstractScreenId() << std::endl;
    oss << "RefreshRate: " << display->GetRefreshRate() << std::endl;
    oss << "VPR: " << display->GetVirtualPixelRatio() << std::endl;
    oss << "Rotation: " << static_cast<uint32_t>(display->GetRotation()) << std::endl;
    oss << "Orientation: " << static_cast<uint32_t>(display->GetOrientation()) << std::endl;
    oss << "DisplayOrientation: " << static_cast<uint32_t>(display->GetDisplayOrientation()) << std::endl;
    oss << "FreezeFlag: " << static_cast<uint32_t>(display->GetFreezeFlag()) << std::endl;
    oss << "DisplayRect: " << "[ "
        << display->GetOffsetX() << ", " << display->GetOffsetY() << ", "
        << display->GetWidth() << ", " << display->GetHeight() << " ]" << std::endl;
    dumpInfo.append(oss.str());
    return DMError::DM_OK;
}

bool DisplayDumper::IsValidDigitString(const std::string& idStr) const
{
    if (idStr.empty()) {
        return false;
    }
    for (char ch : idStr) {
        if ((ch >= '0' && ch <= '9')) {
            continue;
        }
        TLOGE(WmsLogTag::DMS, "invalid id");
        return false;
    }
    return true;
}

std::string DisplayDumper::TransferTypeToString(ScreenType type) const
{
    std::string screenType;
    switch (type) {
        case ScreenType::REAL:
            screenType = "REAL";
            break;
        case ScreenType::VIRTUAL:
            screenType = "VIRTUAL";
            break;
        default:
            screenType = "UNDEFINED";
            break;
    }
    return screenType;
}

void DisplayDumper::GetScreenInfo(const sptr<AbstractScreen>& screen, std::ostringstream& oss) const
{
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen is null");
        return;
    }

    const std::string& name = screen->GetScreenName();
    const std::string& screenName = name.size() <= SCREEN_NAME_MAX_LENGTH ?
        name : name.substr(0, SCREEN_NAME_MAX_LENGTH);
    std::string isGroup = screen->isScreenGroup_ ? "true" : "false";
    std::string screenType = TransferTypeToString(screen->type_);
    std::string isMirrored = screen->rSDisplayNodeConfig_.isMirrored ? "true" : "false";
    NodeId nodeId = (screen->rsDisplayNode_ == nullptr) ? SCREEN_ID_INVALID : screen->rsDisplayNode_->GetId();
    // std::setw is used to set the output width and different width values are set to keep the format aligned.
    oss << std::left << std::setw(W_SCREEN_NAME) << screenName
        << std::left << std::setw(W_SCREEN_TYPE) << screenType
        << std::left << std::setw(W_GROUP_TYPE) << isGroup
        << std::left << std::setw(W_DMS_ID) << screen->dmsId_
        << std::left << std::setw(W_RS_ID) << screen->rsId_
        << std::left << std::setw(W_ACTIVE_IDX) << screen->activeIdx_
        << std::left << std::setw(W_VIR_PIXEL_RATIO) << screen->virtualPixelRatio_
        << std::left << std::setw(W_SCREEN_ROTATION) << static_cast<uint32_t>(screen->rotation_)
        << std::left << std::setw(W_ORIENTATION) << static_cast<uint32_t>(screen->orientation_)
        << std::left << std::setw(W_REQUESTED_ORIENTATION) << static_cast<uint32_t>(screen->screenRequestedOrientation_)
        << std::left << std::setw(W_NODE_ID) << nodeId
        << std::left << std::setw(W_MIRROR_TYPE) << isMirrored
        << std::left << std::setw(W_MIRROR_NODE_ID) << screen->rSDisplayNodeConfig_.mirrorNodeId
        << std::endl;
}

void DisplayDumper::GetDisplayInfo(const sptr<AbstractDisplay>& display, std::ostringstream& oss) const
{
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "display is null");
        return;
    }
    // std::setw is used to set the output width and different width values are set to keep the format aligned.
    oss << std::left << std::setw(W_DISPLAY_ID) << display->GetId()
        << std::left << std::setw(W_ABSTR_SCREEN_ID) << display->GetAbstractScreenId()
        << std::left << std::setw(W_REFRESH_RATE) << display->GetRefreshRate()
        << std::left << std::setw(W_VIR_PIXEL_RATIO) << display->GetVirtualPixelRatio()
        << std::left << std::setw(W_DISPLAY_ROTATION) << static_cast<uint32_t>(display->GetRotation())
        << std::left << std::setw(W_ORIENTATION) << static_cast<uint32_t>(display->GetOrientation())
        << std::left << std::setw(W_DISPLAY_ORIENTATION) << static_cast<uint32_t>(display->GetDisplayOrientation())
        << std::left << std::setw(W_DISPLAY_FREEZE_FLAG) << static_cast<uint32_t>(display->GetFreezeFlag())
        << "[ "
        << std::left << std::setw(W_DISPLAY_OFFSET_X) << display->GetOffsetX()
        << std::left << std::setw(W_DISPLAY_OFFSET_Y) << display->GetOffsetY()
        << std::left << std::setw(W_DISPLAY_WIDTH) << display->GetWidth()
        << std::left << std::setw(W_DISPLAY_HEITHT) << display->GetHeight()
        << "]"
        << std::endl;
}
}
}