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

#include "scene_input_manager.h"

#include <hitrace_meter.h>
#include "perform_reporter.h"
#include "scene_session_dirty_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/session_change_recorder.h"

namespace OHOS {
namespace Rosen {

namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneInputManager" };
const std::string SCENE_INPUT_MANAGER_THREAD = "SceneInputManager";
const std::string FLUSH_DISPLAY_INFO_THREAD = "OS_FlushDisplayInfoThread";

constexpr int MAX_WINDOWINFO_NUM = 15;
constexpr int DEFALUT_DISPLAYID = 0;
constexpr int EMPTY_FOCUS_WINDOW_ID = -1;
constexpr int INVALID_PERSISTENT_ID = 0;
constexpr int DEFAULT_SCREEN_POS = 0;
constexpr int DEFAULT_SCREEN_SCALE = 100;
constexpr int DEFAULT_EXPAND_HEIGHT = 0;
constexpr float DIRECTION90 = 90.0F;

bool IsEqualUiExtentionWindowInfo(const std::vector<MMI::WindowInfo>& a, const std::vector<MMI::WindowInfo>& b);
constexpr unsigned int TRANSFORM_DATA_LEN = 9;

bool operator!=(const MMI::Rect& a, const MMI::Rect& b)
{
    if (a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height) {
        return true;
    }
    return false;
}

bool operator==(const MMI::ScreenInfo& a, const MMI::ScreenInfo& b)
{
    if (a.id != b.id || a.uniqueId != b.uniqueId || a.screenType != b.screenType ||
        a.width != b.width || a.height != b.height ||
        a.physicalWidth != b.physicalWidth || a.physicalHeight != b.physicalHeight ||
        static_cast<int32_t>(a.tpDirection) != static_cast<int32_t>(b.tpDirection) ||
        a.dpi != b.dpi || a.ppi != b.ppi) {
        return false;
    }
    if (a.rotation != b.rotation) {
        return false;
    }
    return true;
}

bool operator==(const MMI::DisplayInfo& a, const MMI::DisplayInfo& b)
{
    if (a.id != b.id || a.x != b.x || a.y != b.y || a.width != b.width ||
        a.height != b.height || a.dpi != b.dpi || a.name != b.name ||
        static_cast<int32_t>(a.direction) != static_cast<int32_t>(b.direction) ||
        static_cast<int32_t>(a.displayDirection) != static_cast<int32_t>(b.displayDirection) ||
        static_cast<int32_t>(a.displayMode) != static_cast<int32_t>(b.displayMode) ||
        a.transform != b.transform || a.scalePercent != b.scalePercent || a.expandHeight != b.expandHeight ||
        a.isCurrentOffScreenRendering != b.isCurrentOffScreenRendering || a.displaySourceMode != b.displaySourceMode ||
        a.oneHandX != b.oneHandX || a.oneHandY != b.oneHandY || a.screenArea.id != b.screenArea.id ||
        a.screenArea.area != b.screenArea.area || a.rsId != b.rsId) {
        return false;
    }
    if (a.offsetX != b.offsetX || a.offsetY != b.offsetY || a.pointerActiveWidth != b.pointerActiveWidth ||
        a.pointerActiveHeight != b.pointerActiveHeight) {
        return false;
    }
    if (a.deviceRotation != b.deviceRotation || a.rotationCorrection != b.rotationCorrection) {
        return false;
    }
    return true;
}

bool operator!=(const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size()) {
        return true;
    }
    int sizeOfA = static_cast<int>(a.size());
    for (int index = 0; index < sizeOfA; index++) {
        if (a[index] != b[index]) {
            return true;
        }
    }
    return false;
}

bool IsEqualWindowInfo(const MMI::WindowInfo& a, const MMI::WindowInfo& b)
{
    if (a.id != b.id || a.pid != b.pid || a.uid != b.uid || a.agentWindowId != b.agentWindowId || a.flags != b.flags ||
        a.displayId != b.displayId || a.zOrder != b.zOrder || a.agentPid != b.agentPid) {
        return false;
    }

    if (a.windowInputType != b.windowInputType || a.privacyMode != b.privacyMode ||
        a.windowType != b.windowType || a.pixelMap != b.pixelMap) {
        return false;
    }
    return true;
}

bool operator==(const MMI::WindowInfo& a, const MMI::WindowInfo& b)
{
    if (!IsEqualWindowInfo(a, b)) {
        return false;
    }

    if (a.area != b.area || a.defaultHotAreas.size() != b.defaultHotAreas.size() ||
        a.pointerHotAreas.size() != b.pointerHotAreas.size() ||
        a.pointerChangeAreas.size() != b.pointerChangeAreas.size() || a.transform.size() != b.transform.size()) {
        return false;
    }

    int sizeOfDefaultHotAreas = static_cast<int>(a.defaultHotAreas.size());
    for (int index = 0; index < sizeOfDefaultHotAreas; index++) {
        if (a.defaultHotAreas[index] != b.defaultHotAreas[index]) {
            return false;
        }
    }
    int sizeOfPointerHotAreas = static_cast<int>(a.pointerHotAreas.size());
    for (int index = 0; index < sizeOfPointerHotAreas; index++) {
        if (a.pointerHotAreas[index] != b.pointerHotAreas[index]) {
            return false;
        }
    }
    int sizeOfPointerChangeAreas = static_cast<int>(a.pointerChangeAreas.size());
    for (int index = 0; index < sizeOfPointerChangeAreas; index++) {
        if (a.pointerChangeAreas[index] != b.pointerChangeAreas[index]) {
            return false;
        }
    }

    if (a.transform != b.transform) {
        return false;
    }
    if (!IsEqualUiExtentionWindowInfo(a.uiExtentionWindowInfo, b.uiExtentionWindowInfo)) {
        return false;
    }
    return true;
}

bool operator!=(const MMI::WindowInfo& a, const MMI::WindowInfo& b)
{
    if (a == b) {
        return false;
    }
    return true;
}

bool IsEqualUiExtentionWindowInfo(const std::vector<MMI::WindowInfo>& a, const std::vector<MMI::WindowInfo>& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    int size = static_cast<int>(a.size());
    for (int i = 0; i < size; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

std::string DumpTransformInDisplayInfo(const std::vector<float>& transform)
{
    std::stringstream stream("[");
    for (float transformItem : transform) {
        stream << transformItem << ",";
    }
    stream << "]";
    return stream.str();
}

std::string DumpDisplayInfo(const MMI::DisplayInfo& info)
{
    std::ostringstream infoStream("DisplayInfo: ");
    infoStream << " id: " << info.id << " x: " << info.x << " y: " << info.y
               << " width: " << info.width << " height: " << info.height << " dpi: " << info.dpi
               << " name: " << info.name
               << " direction: " << static_cast<int>(info.direction)
               << " direction: " << static_cast<int>(info.displayDirection)
               << " displayMode: " << static_cast<int>(info.displayMode)
               << " transform: " << DumpTransformInDisplayInfo(info.transform);
    std::string infoStr = infoStream.str();
    return infoStr;
}
} //namespace


WM_IMPLEMENT_SINGLE_INSTANCE(SceneInputManager)

void SceneInputManager::Init()
{
    sceneSessionDirty_ = std::make_shared<SceneSessionDirtyManager>();
    eventLoop_ = AppExecFwk::EventRunner::Create(FLUSH_DISPLAY_INFO_THREAD);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
    SceneSession::RegisterGetConstrainedModalExtWindowInfo(
        [](const sptr<SceneSession>& sceneSession) -> std::optional<ExtensionWindowEventInfo> {
            return SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(sceneSession);
        });
}

void SceneInputManager::RegisterFlushWindowInfoCallback(FlushWindowInfoCallback&& callback)
{
    sceneSessionDirty_->RegisterFlushWindowInfoCallback(std::move(callback));
}

void SceneInputManager::ResetSessionDirty()
{
    sceneSessionDirty_->ResetSessionDirty();
}

auto SceneInputManager::GetFullWindowInfoList() ->
    std::pair<std::vector<MMI::WindowInfo>, std::vector<std::shared_ptr<Media::PixelMap>>>
{
    return sceneSessionDirty_->GetFullWindowInfoList();
}

void SceneInputManager::UpdateHotAreas(const sptr<SceneSession>& sceneSession,
    std::vector<MMI::Rect>& touchHotAreas, std::vector<MMI::Rect>& pointerHotAreas) const
{
    sceneSessionDirty_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
}

std::vector<MMI::ScreenInfo> SceneInputManager::ConstructScreenInfos(
    std::map<ScreenId, ScreenProperty>& screensProperties)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    if (screensProperties.empty()) {
        TLOGE(WmsLogTag::WMS_EVENT, "screensProperties is empty");
        return screenInfos;
    }
    for (auto& [screenId, screenProperty] : screensProperties) {
        MMI::ScreenInfo screenInfo = {
            .id = screenId,
            .uniqueId = "default" + std::to_string(screenProperty.GetRsId()),
            .screenType = static_cast<MMI::ScreenType>(screenProperty.GetScreenType()),
            .width = screenProperty.GetScreenRealWidth(),
            .height = screenProperty.GetScreenRealHeight(),
            .physicalWidth = screenProperty.GetPhyWidth(),
            .physicalHeight = screenProperty.GetPhyHeight(),
            .tpDirection = ConvertDegreeToMMIRotation(screenProperty.GetDefaultDeviceRotationOffset()),
            .dpi = screenProperty.GetScreenRealPPI(),
            .ppi = screenProperty.GetXDpi(),
            .rotation = ConvertToMMIRotation(screenProperty.GetRotation())
        };
        screenInfos.emplace_back(screenInfo);
    }
    return screenInfos;
}

void SceneInputManager::ConstructDisplayGroupInfos(std::map<ScreenId, ScreenProperty>& screensProperties,
    std::map<DisplayGroupId, MMI::DisplayGroupInfo>& displayGroupMap)
{
    if (screensProperties.empty()) {
        TLOGE(WmsLogTag::WMS_EVENT, "screensProperties is empty");
        return;
    }
    auto displayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    for (auto& [screenId, screenProperty] : screensProperties) {
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(screenId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "screenSession get failed, screenId: %{public}" PRIu64"", screenId);
            continue;
        }
        auto transform = Matrix3f::IDENTITY;
        Vector2f scale(screenProperty.GetScaleX(), screenProperty.GetScaleY());
        transform = transform.Scale(scale, screenProperty.GetPivotX() * screenProperty.GetBounds().rect_.GetWidth(),
            screenProperty.GetPivotY() * screenProperty.GetBounds().rect_.GetHeight()).Inverse();
        std::vector<float> transformData(transform.GetData(), transform.GetData() + TRANSFORM_DATA_LEN);
        int32_t screenOneHandX = DEFAULT_SCREEN_POS;
        int32_t screenOneHandY = DEFAULT_SCREEN_POS;
        int32_t scalePercent = DEFAULT_SCREEN_SCALE;
        int32_t expandHeight = DEFAULT_EXPAND_HEIGHT;
        const SingleHandScreenInfo& singleHandScreenInfo = SceneSessionManager::GetInstance().GetSingleHandScreenInfo();
        if (screenId == ScreenSessionManagerClient::GetInstance().GetDefaultScreenId() &&
            singleHandScreenInfo.mode != SingleHandMode::MIDDLE) {
            SingleHandTransform singleHandTransform = SceneSessionManager::GetInstance().GetNormalSingleHandTransform();
            WSRect originRect = SceneSessionManager::GetInstance().GetOriginRect();
            screenOneHandX = singleHandTransform.posX;
            screenOneHandY = singleHandTransform.posY;
            scalePercent = singleHandTransform.scaleX * DEFAULT_SCREEN_SCALE;
            expandHeight = screenProperty.GetBounds().rect_.GetHeight() - originRect.height_;
        }
        int32_t deviceRotation = static_cast<int32_t>(screenProperty.GetDeviceRotation());
        int32_t rotationCorrection = static_cast<int32_t>(screenSession->GetRotationCorrection(displayMode));
        MMI::DisplayInfo displayInfo = {
            .id = screenId,
            .x = screenProperty.GetX(),
            .y = screenProperty.GetY(),
            .width = screenProperty.GetValidWidth(),
            .height =screenProperty.GetValidHeight(),
            .dpi = screenProperty.GetDensity() *  DOT_PER_INCH,
            .name = "display" + std::to_string(screenId),
            .direction = ConvertDegreeToMMIRotation(screenProperty.GetPhysicalRotation()),
            .displayDirection = ConvertDegreeToMMIRotation(screenProperty.GetScreenComponentRotation()),
            .displayMode = static_cast<MMI::DisplayMode>(displayMode),
            .transform = transformData,
            .scalePercent = scalePercent,
            .expandHeight = expandHeight,
            .isCurrentOffScreenRendering = screenProperty.GetCurrentOffScreenRendering(),
            .displaySourceMode = static_cast<MMI::DisplaySourceMode>(screenSession->GetScreenCombination()),
            .oneHandX = screenOneHandX,
            .oneHandY = screenOneHandY,
            .screenArea = {
                .id = screenId,
                .area = {screenProperty.GetScreenAreaOffsetX(), screenProperty.GetScreenAreaOffsetY(),
                    screenProperty.GetScreenAreaWidth(), screenProperty.GetScreenAreaHeight()}
            },
            .rsId = screenProperty.GetRsId(),
            .offsetX = screenProperty.GetInputOffsetX(),
            .offsetY = screenProperty.GetInputOffsetY(),
            .pointerActiveWidth = screenProperty.GetPointerActiveWidth(),
            .pointerActiveHeight = screenProperty.GetPointerActiveHeight(),
            .deviceRotation = ConvertDegreeToMMIRotation(deviceRotation * DIRECTION90),
            .rotationCorrection = ConvertDegreeToMMIRotation(rotationCorrection * DIRECTION90)
        };
        DisplayGroupId displayGroupId = screenSession->GetDisplayGroupId();
        if (displayGroupMap.count(displayGroupId) == 0) {
            MMI::DisplayGroupInfo displayGroupInfo = {
                .id = displayGroupId,
                .name = "displayGroup" + std::to_string(displayGroupId),
                .type = displayGroupId == 0 ? MMI::GROUP_DEFAULT : MMI::GROUP_SPECIAL,
                .mainDisplayId = screenProperty.GetMainDisplayIdOfGroup(),
            };
            displayGroupMap[displayGroupId] = displayGroupInfo;
        }
        displayGroupMap[displayGroupId].displaysInfo.emplace_back(displayInfo);
    }
}

std::unordered_map<DisplayId, int32_t> SceneInputManager::GetFocusedSessionMap() const
{
    std::unordered_map<DisplayId, int32_t> focusInfoMap;
    auto focusInfoMapArray = SceneSessionManager::GetInstance().GetAllFocusedSessionList();
    for (const auto& item : focusInfoMapArray) {
        int32_t focusedSessionId = item.second;
        auto focusedSession = SceneSessionManager::GetInstance().GetSceneSession(focusedSessionId);
        if (focusedSession == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "focusedSession is null, id: %{public}d", focusedSessionId);
            focusInfoMap[item.first] = INVALID_PERSISTENT_ID;
            continue;
        }
        if (auto modalUIExtensionEventInfo = focusedSession->GetLastModalUIExtensionEventInfo()) {
            focusedSessionId = modalUIExtensionEventInfo.value().persistentId;
        }
        focusInfoMap[item.first] = focusedSessionId;
    }
    return focusInfoMap;
}

void SceneInputManager::FlushFullInfoToMMI(const std::vector<MMI::ScreenInfo>& screenInfos,
    std::map<DisplayGroupId, MMI::DisplayGroupInfo>& displayGroupMap,
    const std::vector<MMI::WindowInfo>& windowInfoList, bool isOverBatchSize)
{
    auto focusInfoMap = GetFocusedSessionMap();
    std::unordered_map<DisplayGroupId, std::vector<MMI::WindowInfo>> windowInfoMap;
    for (const auto& windowInfo : windowInfoList) {
        windowInfoMap[windowInfo.groupId].emplace_back(windowInfo);
    }
    std::vector<MMI::DisplayGroupInfo> displayGroupInfos;
    for (auto& [displayGroupId, displayGroup] : displayGroupMap) {
        if (!isOverBatchSize && !windowInfoMap[displayGroupId].empty()) {
            windowInfoMap[displayGroupId].back().action = MMI::WINDOW_UPDATE_ACTION::ADD_END;
        }
        displayGroup.windowsInfo = windowInfoMap[displayGroupId];
        displayGroup.focusWindowId = focusInfoMap[displayGroupId];
        displayGroupInfos.emplace_back(displayGroup);
    }
    MMI::UserScreenInfo userScreenInfo = {
        .userId = currentUserId_,
        .userState = MMI::USER_ACTIVE,
        .screens = screenInfos,
        .displayGroups = displayGroupInfos
    };
    MMI::InputManager::GetInstance()->UpdateDisplayInfo(userScreenInfo);

    for (auto groupInfo : displayGroupInfos) {
        TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] - displayGroupId: %{public}d", groupInfo.id);
        for (const auto& displayInfo : groupInfo.displaysInfo) {
            TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] - %{public}s", DumpDisplayInfo(displayInfo).c_str());
        }
        std::string windowInfoListDump = "windowinfo  ";
        for (const auto& windowInfo : groupInfo.windowsInfo) {
            windowInfoListDump.append(DumpWindowInfo(windowInfo).append("  ||  "));
        }
        TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] - %{public}s", windowInfoListDump.c_str());
    }
}

void SceneInputManager::FlushEmptyInfoToMMI()
{
    auto task = [this]() {
        std::map<ScreenId, ScreenProperty> screensProperties =
            ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
        std::vector<MMI::ScreenInfo> screenInfos = ConstructScreenInfos(screensProperties);
        std::map<DisplayGroupId, MMI::DisplayGroupInfo> displayGroupMap;
        ConstructDisplayGroupInfos(screensProperties, displayGroupMap);
        std::vector<MMI::DisplayGroupInfo> displayGroupInfos;
        for (auto& [displayGroupId, displayGroup] : displayGroupMap) {
            displayGroupInfos.emplace_back(displayGroup);
        }
        MMI::UserScreenInfo userScreenInfo = {
            .userId = currentUserId_,
            .userState = MMI::USER_INACTIVE,
            .screens = screenInfos,
            .displayGroups = displayGroupInfos
        };
        TLOGNI(WmsLogTag::WMS_EVENT, "userId:%{public}d", currentUserId_);
        MMI::InputManager::GetInstance()->UpdateDisplayInfo(userScreenInfo);
    };
    if (eventHandler_) {
        eventHandler_->PostTask(task);
    }
}

void SceneInputManager::NotifyWindowInfoChange(const sptr<SceneSession>& sceneSession, const WindowUpdateType& type)
{
    if (sceneSessionDirty_) {
        sceneSessionDirty_->NotifyWindowInfoChange(sceneSession, type);
    }
}

void SceneInputManager::NotifyMMIWindowPidChange(const sptr<SceneSession>& sceneSession, const bool startMoving)
{
    if (sceneSessionDirty_) {
        sceneSessionDirty_->NotifyWindowInfoChange(sceneSession,
            WindowUpdateType::WINDOW_UPDATE_PROPERTY, startMoving);
        if (sceneSession == nullptr) {
            return;
        }
        sceneSession->SetIsStartMoving(startMoving);
    }
}

void SceneInputManager::NotifyWindowInfoChangeFromSession(const sptr<SceneSession>& sceneSesion)
{
    if (sceneSessionDirty_) {
        sceneSessionDirty_->NotifyWindowInfoChange(sceneSesion, WindowUpdateType::WINDOW_UPDATE_PROPERTY);
    } else {
        TLOGD(WmsLogTag::WMS_EVENT, "sceneSessionDirty is nullptr");
    }
}

void SceneInputManager::FlushChangeInfoToMMI(const std::map<uint64_t, std::vector<MMI::WindowInfo>>& screenId2Windows)
{
    for (auto& iter : screenId2Windows) {
        auto displayId = iter.first;
        auto& windowInfos = iter.second;
        std::string windowInfoListDump = "windowinfo  ";
        for (auto& windowInfo : windowInfos) {
            windowInfoListDump.append(DumpWindowInfo(windowInfo).append("  ||  "));
        }
        TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] --- %{public}s", windowInfoListDump.c_str());
        MMI::WindowGroupInfo windowGroup = {focusedSessionId_, displayId, windowInfos};
        MMI::InputManager::GetInstance()->UpdateWindowInfo(windowGroup);
    }
}

bool SceneInputManager::CheckNeedUpdate(const std::vector<MMI::ScreenInfo>& screenInfos,
    const std::vector<MMI::DisplayInfo>& displayInfos, const std::vector<MMI::WindowInfo>& windowInfoList)
{
    int32_t focusId = SceneSessionManager::GetInstance().GetFocusedSessionId();
    if (focusId != lastFocusId_) {
        lastFocusId_ = focusId;
        lastScreenInfos_ = screenInfos;
        lastDisplayInfos_ = displayInfos;
        lastWindowInfoList_ = windowInfoList;
        return true;
    }

    if (screenInfos.size() != lastScreenInfos_.size() || displayInfos.size() != lastDisplayInfos_.size() ||
        windowInfoList.size() != lastWindowInfoList_.size()) {
        lastScreenInfos_ = screenInfos;
        lastDisplayInfos_ = displayInfos;
        lastWindowInfoList_ = windowInfoList;
        return true;
    }

    int sizeOfScreenInfos = static_cast<int>(screenInfos.size());
    for (int index = 0; index < sizeOfScreenInfos; index++) {
        if (!(screenInfos[index] == lastScreenInfos_[index])) {
            lastScreenInfos_ = screenInfos;
            lastDisplayInfos_ = displayInfos;
            lastWindowInfoList_ = windowInfoList;
            return true;
        }
    }

    int sizeOfDisplayInfos = static_cast<int>(displayInfos.size());
    for (int index = 0; index < sizeOfDisplayInfos; index++) {
        if (!(displayInfos[index] == lastDisplayInfos_[index])) {
            lastDisplayInfos_ = displayInfos;
            lastWindowInfoList_ = windowInfoList;
            return true;
        }
    }

    int sizeOfWindowInfoList = static_cast<int>(windowInfoList.size());
    for (int index = 0; index < sizeOfWindowInfoList; index++) {
        if (!(windowInfoList[index] == lastWindowInfoList_[index])) {
            lastWindowInfoList_ = windowInfoList;
            return true;
        }
    }
    return false;
}

void SceneInputManager::UpdateFocusedSessionId(int32_t focusedSessionId)
{
    auto focusedSceneSession = SceneSessionManager::GetInstance().GetSceneSession(focusedSessionId);
    if (focusedSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "focusedSceneSession is null");
        return;
    }
    if (auto modalUIExtensionEventInfo = focusedSceneSession->GetLastModalUIExtensionEventInfo()) {
        focusedSessionId_ = modalUIExtensionEventInfo.value().persistentId;
    }
}

void DumpUIExtentionWindowInfo(const MMI::WindowInfo& windowInfo)
{
    auto sceneSession = SceneSessionManager::GetInstance().GetSceneSession(windowInfo.id);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is null");
        return;
    }
    auto surfaceNode = sceneSession->GetSurfaceNode();
    if (surfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "surfaceNode is null");
        return;
    }
    auto surfaceId = surfaceNode->GetId();
    std::ostringstream dumpUecWindowInfo;
    dumpUecWindowInfo << "DumpUecWindowInfo:" << windowInfo.id << ";"<< surfaceId;
    dumpUecWindowInfo << ";" << windowInfo.uiExtentionWindowInfo.size();
    for (const auto& uiExtWindowInfo : windowInfo.uiExtentionWindowInfo) {
        auto str = DumpWindowInfo(uiExtWindowInfo);
        str = "sec:" + std::to_string(uiExtWindowInfo.privacyUIFlag) + " " + str;
        dumpUecWindowInfo << ";{" << str << "}";
    }
    TLOGND(WmsLogTag::WMS_EVENT, "DumpUecWindowInfo: wid;surfaceId;uecInfoSize"
        "{wInfo:wId|pid|agentPid|uid|[x,y,width,height]|agentWindowId|flags|displayId|action|zOrder"
        ",hot:[x,y,width,height]}");
    TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s", dumpUecWindowInfo.str().c_str());
}

void SceneInputManager::PrintWindowInfo(const std::vector<MMI::WindowInfo>& windowInfoList)
{
    int windowListSize = static_cast<int>(windowInfoList.size());
    std::ostringstream idListStream;
    std::ostringstream dumpWindowListStream;
    static std::string lastIdList;
    static uint32_t windowEventID = 0;
    if (windowEventID == UINT32_MAX) {
        windowEventID = 0;
    }
    focusedSessionId_ = SceneSessionManager::GetInstance().GetFocusedSessionId();
    std::unordered_map<int32_t, MMI::Rect> currWindowDefaultHotArea;
    static std::unordered_map<int32_t, MMI::Rect> lastWindowDefaultHotArea;
    MMI::WindowInfo focusedWindow;
    for (auto& e : windowInfoList) {
        idListStream << e.id << "|" << e.displayId << "|" << e.flags << "|" << e.zOrder << "|"
                     << e.pid << "|" << e.agentPid << "|" << e.defaultHotAreas.size();

        if (e.defaultHotAreas.size() > 0) {
            auto iter = lastWindowDefaultHotArea.find(e.id);
            if (iter == lastWindowDefaultHotArea.end() || iter->second != e.defaultHotAreas[0]) {
                idListStream << "|" << e.defaultHotAreas[0].x << "," << e.defaultHotAreas[0].y
                             << "," << e.defaultHotAreas[0].width << "," << e.defaultHotAreas[0].height;
            }
            currWindowDefaultHotArea.insert({e.id, e.defaultHotAreas[0]});
        }
        idListStream << ";";
        if ((focusedSessionId_ == e.id) && (e.id == e.agentWindowId)) {
            focusedWindow = e;
            UpdateFocusedSessionId(focusedSessionId_);
        }
        if (e.uiExtentionWindowInfo.size() > 0) {
            DumpUIExtentionWindowInfo(e);
        }
        ConstructDumpWindowInfo(e, dumpWindowListStream);
    }
    lastWindowDefaultHotArea = currWindowDefaultHotArea;
    SingleHandTransform transform = SceneSessionManager::GetInstance().GetNormalSingleHandTransform();
    idListStream << focusedSessionId_ << "|" << transform.posX << "," << transform.posY
        << "|" << transform.scaleX << "," << transform.scaleY;
    if (focusedWindow.defaultHotAreas.size() > 0) {
        idListStream << "|" << focusedWindow.defaultHotAreas[0].x << "," << focusedWindow.defaultHotAreas[0].y << ","
                     << focusedWindow.defaultHotAreas[0].width << "," << focusedWindow.defaultHotAreas[0].height;
    }
    std::string idList = idListStream.str();
    if (lastIdList != idList) {
        windowEventID++;
        TLOGNI(WmsLogTag::WMS_EVENT, "LogWinInfo: eid:%{public}d,size:%{public}d,idList:%{public}s",
            windowEventID, windowListSize, idList.c_str());
        lastIdList = idList;
    }
    std::string dumpWindowList = dumpWindowListStream.str();
    SceneSessionChangeInfo changeInfo {
        .changeInfo_ = "WindowInfos: " + dumpWindowList,
        .logTag_ = WmsLogTag::WMS_EVENT,
    };
    SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::EVENT_RECORD, changeInfo);
}

void SceneInputManager::PrintScreenInfo(const std::vector<MMI::ScreenInfo>& screenInfos)
{
    int screenListSize = static_cast<int>(screenInfos.size());
    std::ostringstream screenListStream;
    static std::string lastScreenList = "";
    for (auto& screenInfo : screenInfos) {
        screenListStream << screenInfo.id << "|" << screenInfo.uniqueId << "|"
                         << static_cast<uint32_t>(screenInfo.screenType) << "|"
                         << screenInfo.width << "|" << screenInfo.height << "|"
                         << screenInfo.physicalWidth << "|" << screenInfo.physicalHeight << "|"
                         << static_cast<int32_t>(screenInfo.tpDirection) << "|"
                         << screenInfo.dpi << "|" << screenInfo.ppi << ",";
    }

    std::string screenList = screenListStream.str();
    if (lastScreenList != screenList) {
        TLOGI(WmsLogTag::WMS_EVENT, "num:%{public}d,list:%{public}s", screenListSize, screenList.c_str());
        lastScreenList = screenList;
    }
}

void SceneInputManager::PrintDisplayInfo(const std::vector<MMI::DisplayInfo>& displayInfos)
{
    int displayListSize = static_cast<int>(displayInfos.size());
    std::ostringstream displayListStream;
    std::ostringstream dumpDisplayListStream;
    static std::string lastDisplayList = "";
    for (auto& displayInfo : displayInfos) {
        displayListStream << displayInfo.id << "|" << displayInfo.x << "|" << displayInfo.y << "|"
                          << displayInfo.width << "|" << displayInfo.height << "|"
                          << displayInfo.dpi << "|"
                          << static_cast<int32_t>(displayInfo.direction) << "|"
                          << static_cast<int32_t>(displayInfo.displayDirection) << "|"
                          << static_cast<int32_t>(displayInfo.displayMode) << "|"
                          << displayInfo.scalePercent << "|" << displayInfo.expandHeight << "|"
                          << displayInfo.isCurrentOffScreenRendering << "|"
                          << static_cast<uint32_t>(displayInfo.displaySourceMode) << "|" << displayInfo.oneHandX << "|"
                          << displayInfo.oneHandY << "|" << displayInfo.screenArea.area.x << "|"
                          << displayInfo.screenArea.area.y << "|" << displayInfo.screenArea.area.width << "|"
                          << displayInfo.screenArea.area.height << "|" << displayInfo.rsId << ",";
        ConstructDumpDisplayInfo(displayInfo, dumpDisplayListStream);
    }

    std::string displayList = displayListStream.str();
    if (lastDisplayList != displayList) {
        TLOGI(WmsLogTag::WMS_EVENT, "num:%{public}d,list:%{public}s", displayListSize, displayList.c_str());
        lastDisplayList = displayList;
    }
    std::string dumpDisplayList = dumpDisplayListStream.str();
    SceneSessionChangeInfo changeInfo {
        .changeInfo_ = "DisplayInfos: " + dumpDisplayList,
        .logTag_ = WmsLogTag::WMS_EVENT,
    };
    SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::EVENT_RECORD, changeInfo);
}

void SceneInputManager::SetUserBackground(bool userBackground)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "userBackground=%{public}d", userBackground);
    isUserBackground_.store(userBackground);
}

void SceneInputManager::SetCurrentUserId(int32_t userId)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "Current userId=%{public}d", userId);
    currentUserId_ = userId;
    MMI::InputManager::GetInstance()->SetCurrentUser(userId);
}

void SceneInputManager::UpdateDisplayAndWindowInfo(const std::vector<MMI::ScreenInfo>& screenInfos,
    std::map<DisplayGroupId, MMI::DisplayGroupInfo>& displayGroupMap,
    std::vector<MMI::WindowInfo> windowInfoList)
{
    if (windowInfoList.size() == 0) {
        FlushFullInfoToMMI(screenInfos, displayGroupMap, windowInfoList);
        return;
    }
    int32_t windowBatchSize = MAX_WINDOWINFO_NUM;
    if (windowInfoList[0].defaultHotAreas.size() > MMI::WindowInfo::DEFAULT_HOTAREA_COUNT) {
        windowBatchSize = MMI::InputManager::GetInstance()->GetWinSyncBatchSize(
            static_cast<int32_t>(windowInfoList[0].defaultHotAreas.size()),
            static_cast<int32_t>(displayGroupMap.size()));
    }
    int32_t windowListSize = static_cast<int32_t>(windowInfoList.size());
    if (windowListSize <= windowBatchSize) {
        FlushFullInfoToMMI(screenInfos, displayGroupMap, windowInfoList);
        return;
    }
    std::unordered_map<int32_t, std::vector<int32_t>> windowIndexMap;
    int32_t index = 0;
    for (const auto& windowInfo : windowInfoList) {
        windowIndexMap[windowInfo.groupId].emplace_back(index);
        index++;
    }
    for (auto& [displayGroupId, indexInfos] : windowIndexMap) {
        windowInfoList[indexInfos.back()].action = MMI::WINDOW_UPDATE_ACTION::ADD_END;
    }
    auto iterBegin = windowInfoList.begin();
    auto iterEnd = windowInfoList.end();
    auto iterNext = std::next(iterBegin, windowBatchSize);
    FlushFullInfoToMMI(screenInfos, displayGroupMap, std::vector<MMI::WindowInfo>(iterBegin, iterNext), true);
    while (iterNext != iterEnd) {
        auto iterNewBegin = iterNext;
        if (iterNewBegin->defaultHotAreas.size() <= MMI::WindowInfo::DEFAULT_HOTAREA_COUNT) {
            windowBatchSize = MAX_WINDOWINFO_NUM;
        }
        if (std::distance(iterNewBegin, iterEnd) <= windowBatchSize) {
            iterNext = iterEnd;
        } else {
            iterNext = std::next(iterNewBegin, windowBatchSize);
        }
        std::map<uint64_t, std::vector<MMI::WindowInfo>> screenToWindowInfoList;
        screenToWindowInfoList.emplace(DEFALUT_DISPLAYID, std::vector<MMI::WindowInfo>(iterNewBegin, iterNext));
        FlushChangeInfoToMMI(screenToWindowInfoList);
    }
}

void SceneInputManager::FlushDisplayInfoToMMI(std::vector<MMI::WindowInfo>&& windowInfoList,
                                              std::vector<std::shared_ptr<Media::PixelMap>>&& pixelMapList,
                                              const bool forceFlush)
{
    eventHandler_->PostTask([this, windowInfoList = std::move(windowInfoList),
                            pixelMapList = std::move(pixelMapList), forceFlush]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "FlushDisplayInfoToMMI");
        if (isUserBackground_.load()) {
            TLOGND(WmsLogTag::WMS_MULTI_USER, "User in background, no need to flush display info");
            return;
        }
        if (sceneSessionDirty_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "sceneSessionDirty_ is nullptr");
            return;
        }
        std::map<ScreenId, ScreenProperty> screensProperties =
            ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
        std::vector<MMI::ScreenInfo> screenInfos = ConstructScreenInfos(screensProperties);
        std::map<DisplayGroupId, MMI::DisplayGroupInfo> displayGroupMap;
        ConstructDisplayGroupInfos(screensProperties, displayGroupMap);
        if (displayGroupMap.empty()) {
            std::ostringstream oss;
            oss << "displayInfos flush to MMI is empty!";
            int32_t ret = WindowInfoReporter::GetInstance().ReportEventDispatchException(
                static_cast<int32_t>(WindowDFXHelperType::WINDOW_FLUSH_EMPTY_DISPLAY_INFO_TO_MMI_EXCEPTION),
                getpid(), oss.str()
            );
            if (ret != 0) {
                TLOGNI(WmsLogTag::WMS_EVENT, "ReportEventDispatchException message failed, ret: %{public}d", ret);
            }
            return;
        }
        std::vector<MMI::DisplayInfo> displayInfos;
        for (auto& [displayGroupId, displayGroup] : displayGroupMap) {
            for (auto& displayInfo : displayGroup.displaysInfo) {
                displayInfos.emplace_back(displayInfo);
            }
        }
        if (!forceFlush && !CheckNeedUpdate(screenInfos, displayInfos, windowInfoList)) {
            return;
        }
        PrintScreenInfo(screenInfos);
        PrintDisplayInfo(displayInfos);
        PrintWindowInfo(windowInfoList);
        UpdateDisplayAndWindowInfo(screenInfos, displayGroupMap, std::move(windowInfoList));
    });
}

void SceneInputManager::UpdateSecSurfaceInfo(const std::map<uint64_t, std::vector<SecSurfaceInfo>>& secSurfaceInfoMap)
{
    if (sceneSessionDirty_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSessionDirty_ is nullptr");
        return;
    }
    sceneSessionDirty_->UpdateSecSurfaceInfo(secSurfaceInfoMap);
}

void SceneInputManager::UpdateConstrainedModalUIExtInfo(
    const std::map<uint64_t, std::vector<SecSurfaceInfo>>& constrainedModalUIExtInfoMap)
{
    if (sceneSessionDirty_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSessionDirty_ is nullptr");
        return;
    }
    sceneSessionDirty_->UpdateConstrainedModalUIExtInfo(constrainedModalUIExtInfoMap);
}

std::optional<ExtensionWindowEventInfo> SceneInputManager::GetConstrainedModalExtWindowInfo(
    const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return std::nullopt;
    }
    if (sceneSessionDirty_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSessionDirty_ is nullptr");
        return std::nullopt;
    }
    SecSurfaceInfo constrainedModalUIExtInfo;
    if (!sceneSessionDirty_->GetLastConstrainedModalUIExtInfo(sceneSession, constrainedModalUIExtInfo)) {
        return std::nullopt;
    }
    auto persistentId = sceneSession->GetUIExtPersistentIdBySurfaceNodeId(constrainedModalUIExtInfo.uiExtensionNodeId);
    if (persistentId == INVALID_PERSISTENT_ID) {
        TLOGE(WmsLogTag::WMS_EVENT, "invalid persistentId");
        return std::nullopt;
    }
    return ExtensionWindowEventInfo {
        .persistentId = persistentId,
        .pid = constrainedModalUIExtInfo.uiExtensionPid,
        .isConstrainedModal = true };
}

void SceneInputManager::ConstructDumpDisplayInfo(const MMI::DisplayInfo& displayInfo,
    std::ostringstream& dumpDisplayListStream)
{
    std::ostringstream transformStream;
    transformStream << "[";
    for (auto& it : displayInfo.transform) {
        transformStream << it << ", ";
    }
    transformStream << "]";
    dumpDisplayListStream << "id:" << displayInfo.id
                          << "|x:" << displayInfo.x
                          << "|y:" << displayInfo.y
                          << "|width:" << displayInfo.width
                          << "|height:" << displayInfo.height
                          << "|dpi:" << displayInfo.dpi
                          << "|name:" << displayInfo.name
                          << "|direction:" << static_cast<int32_t>(displayInfo.direction)
                          << "|displayDirection:" << static_cast<int32_t>(displayInfo.displayDirection)
                          << "|displayMode:" << static_cast<int32_t>(displayInfo.displayMode)
                          << "|transform:" << transformStream.str()
                          << "|screenOneHandX:" << displayInfo.oneHandX
                          << "|screenOneHandY:" << displayInfo.oneHandY
                          << "|scalePercent:" << displayInfo.scalePercent
                          << "|expandHeight:" << displayInfo.expandHeight
                          << "|isCurrentOffScreenRendering:" << displayInfo.isCurrentOffScreenRendering
                          << "|displaySourceMode:" << static_cast<int32_t>(displayInfo.displaySourceMode)
                          << "|screenArea.id:" << displayInfo.screenArea.id
                          << "|screenArea.area:[" << displayInfo.screenArea.area.x << ","
                          << displayInfo.screenArea.area.y << ","
                          << displayInfo.screenArea.area.width << ","
                          << displayInfo.screenArea.area.height << "]"
                          << "|rsId:" << displayInfo.rsId
                          << "|offsetX:" << displayInfo.offsetX
                          << "|offsetY:" << displayInfo.offsetY
                          << "|pointerActiveWidth:" << displayInfo.pointerActiveWidth
                          << "|pointerActiveHeight:" << displayInfo.pointerActiveHeight << ", ";
}

void SceneInputManager::ConstructDumpWindowInfo(const MMI::WindowInfo& windowInfo,
    std::ostringstream& dumpWindowListStream)
{
    std::ostringstream transformStream;
    transformStream << "[";
    for (auto& it : windowInfo.transform) {
        transformStream << it << ", ";
    }
    transformStream << "]";
    std::ostringstream defaultHotAreasStream;
    for (auto& defaultHotArea : windowInfo.defaultHotAreas) {
        std::ostringstream defaultHotAreaStream;
        defaultHotAreaStream << "[" << defaultHotArea.x << ", " <<defaultHotArea.y << ", "
                             << defaultHotArea.width << ", " <<defaultHotArea.height << "]";
        defaultHotAreasStream << defaultHotAreaStream.str() << " ";
    }
    std::ostringstream pointerHotAreasStream;
    for (auto& pointerHotArea : windowInfo.pointerHotAreas) {
        std::ostringstream pointerHotAreaStream;
        pointerHotAreaStream << "[" << pointerHotArea.x << ", " <<pointerHotArea.y << ", "
                             << pointerHotArea.width << ", " <<pointerHotArea.height << "]";
        pointerHotAreasStream << pointerHotAreaStream.str() << " ";
    }
    std::ostringstream pointerChangeAreasStream;
    pointerChangeAreasStream << "[";
    for (auto& pointerChangeArea : windowInfo.pointerChangeAreas) {
        pointerChangeAreasStream << pointerChangeArea << ", ";
    }
    pointerChangeAreasStream << "]";
    dumpWindowListStream << "id:" << windowInfo.id << "|pid:" << windowInfo.pid << "|uid:" << windowInfo.uid
                          << "|area:[" << windowInfo.area.x << ", " << windowInfo.area.y << ", "
                          << windowInfo.area.width << ", " << windowInfo.area.height << "]"
                          << "|defaultHotAreas:" << defaultHotAreasStream.str()
                          << "|pointerHotAreas:" << pointerHotAreasStream.str() << "|agentWindowId:"
                          << windowInfo.agentWindowId << "|action:" << static_cast<uint32_t>(windowInfo.action)
                          << "|displayId:" << windowInfo.displayId << "|zOrder:" << windowInfo.zOrder
                          << "|pointerChangeAreas:" << pointerChangeAreasStream.str() << "|transform:"
                          << transformStream.str() << "|windowInputType:"
                          << static_cast<int32_t>(windowInfo.windowInputType) << "|windowType:"
                          << windowInfo.windowType << "|isSkipSelfWhenShowOnVirtualScreen:"
                          << windowInfo.isSkipSelfWhenShowOnVirtualScreen << "|windowNameType:"
                          << windowInfo.windowNameType << "|groupId:" << windowInfo.groupId
                          << "|flags:" << windowInfo.flags << ", ";
}
}
} // namespace OHOS::Rosen
