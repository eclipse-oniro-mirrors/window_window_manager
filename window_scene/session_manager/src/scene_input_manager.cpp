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
#include "scene_session_dirty_manager.h"
#include "screen_session_manager/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"

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

bool IsEqualUiExtentionWindowInfo(const std::vector<MMI::WindowInfo>& a, const std::vector<MMI::WindowInfo>& b);
constexpr unsigned int TRANSFORM_DATA_LEN = 9;

bool operator!=(const MMI::Rect& a, const MMI::Rect& b)
{
    if (a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height) {
        return true;
    }
    return false;
}

bool operator==(const MMI::DisplayInfo& a, const MMI::DisplayInfo& b)
{
    if (a.id != b.id || a.x != b.x || a.y != b.y || a.width != b.width ||
        a.height != b.height || a.dpi != b.dpi || a.name != b.name || a.uniq != b.uniq ||
        static_cast<int32_t>(a.direction) != static_cast<int32_t>(b.direction) ||
        static_cast<int32_t>(a.displayDirection) != static_cast<int32_t>(b.displayDirection) ||
        static_cast<int32_t>(a.displayMode) != static_cast<int32_t>(b.displayMode) ||
        a.transform != b.transform) {
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
        a.displayId != b.displayId || a.zOrder != b.zOrder) {
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
    std::string infoStr =  "DisplayInfo: ";
    infoStr = infoStr + " id: " + std::to_string(info.id) + " x: " + std::to_string(info.x) +
        "y: " + std::to_string(info.y) + " width: " + std::to_string(info.width) +
        "height: " + std::to_string(info.height) + " dpi: " + std::to_string(info.dpi) + " name:" + info.name +
        " uniq: " + info.uniq + " displayMode: " + std::to_string(static_cast<int>(info.displayMode)) +
        " direction: " + std::to_string(static_cast<int>(info.direction)) +
        " transform: " + DumpTransformInDisplayInfo(info.transform);
    return infoStr;
}
} //namespace


WM_IMPLEMENT_SINGLE_INSTANCE(SceneInputManager)

void SceneInputManager::Init()
{
    sceneSessionDirty_ = std::make_shared<SceneSessionDirtyManager>();
    eventLoop_ = AppExecFwk::EventRunner::Create(FLUSH_DISPLAY_INFO_THREAD);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
    auto callback = [this]() {
        FlushDisplayInfoToMMI();
    };
    sceneSessionDirty_->RegisterFlushWindowInfoCallback(callback);
    SceneSession::RegisterGetConstrainedModalExtWindowInfo(
        [](const sptr<SceneSession>& sceneSession) -> std::optional<ExtensionWindowEventInfo> {
            return SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(sceneSession);
        });
}

void SceneInputManager::ConstructDisplayInfos(std::vector<MMI::DisplayInfo>& displayInfos)
{
    std::map<ScreenId, ScreenProperty> screensProperties =
        Rosen::ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    auto displayMode = Rosen::ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    for (auto& iter: screensProperties) {
        auto screenId = iter.first;
        auto& screenProperty = iter.second;
        auto screenSession = Rosen::ScreenSessionManagerClient::GetInstance().GetScreenSessionById(screenId);
        MMI::Direction displayRotation;
        if (screenSession && screenSession->GetDisplayNode()) {
            displayRotation = ConvertDegreeToMMIRotation(
                screenSession->GetDisplayNode()->GetStagingProperties().GetRotation(),
                static_cast<MMI::DisplayMode>(displayMode));
        } else {
            displayRotation = ConvertDegreeToMMIRotation(screenProperty.GetRotation(),
                static_cast<MMI::DisplayMode>(displayMode));
        }
        auto screenWidth = screenProperty.GetBounds().rect_.GetWidth();
        auto screenHeight = screenProperty.GetBounds().rect_.GetHeight();
        auto transform = Matrix3f::IDENTITY;
        Vector2f scale(screenProperty.GetScaleX(), screenProperty.GetScaleY());
        transform = transform.Scale(scale, screenProperty.GetPivotX() * screenWidth,
            screenProperty.GetPivotY() * screenHeight);
        transform = transform.Inverse();
        std::vector<float> transformData(transform.GetData(), transform.GetData() + TRANSFORM_DATA_LEN);
        MMI::DisplayInfo displayInfo = {
            .id = screenId,
            .x = screenProperty.GetOffsetX(),
            .y = screenProperty.GetOffsetY(),
            .width = screenWidth,
            .height = screenHeight,
            .dpi = screenProperty.GetDensity() *  DOT_PER_INCH,
            .name = "display" + std::to_string(screenId),
            .uniq = "default" + std::to_string(screenId),
            .direction = ConvertDegreeToMMIRotation(screenProperty.GetRotation(),
                static_cast<MMI::DisplayMode>(displayMode)),
            .displayDirection = displayRotation,
            .displayMode = static_cast<MMI::DisplayMode>(displayMode),
            .transform = transformData,
            .oneHandX = SceneSessionManager::GetInstance().GetNormalSingleHandTransform().posX,
            .oneHandY = SceneSessionManager::GetInstance().GetNormalSingleHandTransform().posY
        };
        displayInfos.emplace_back(displayInfo);
    }
}

void SceneInputManager::FlushFullInfoToMMI(const std::vector<MMI::DisplayInfo>& displayInfos,
    const std::vector<MMI::WindowInfo>& windowInfoList)
{
    int mainScreenWidth = 0;
    int mainScreenHeight = 0;
    if (!displayInfos.empty()) {
        mainScreenWidth = displayInfos[0].width;
        mainScreenHeight = displayInfos[0].height;
    }
    if (sceneSessionDirty_ == nullptr) {
        WLOGFE("scene session dirty is null");
        return;
    }

    MMI::DisplayGroupInfo displayGroupInfo = {
        .width = mainScreenWidth,
        .height = mainScreenHeight,
        .focusWindowId = focusedSessionId_,
        .currentUserId = currentUserId_,
        .windowsInfo = windowInfoList,
        .displaysInfo = displayInfos};
    for (const auto& displayInfo : displayGroupInfo.displaysInfo) {
        TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] - %{public}s", DumpDisplayInfo(displayInfo).c_str());
    }
    std::string windowInfoListDump = "windowinfo  ";
    for (const auto& windowInfo : displayGroupInfo.windowsInfo) {
        windowInfoListDump.append(DumpWindowInfo(windowInfo).append("  ||  "));
    }
    TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] - %{public}s", windowInfoListDump.c_str());
    MMI::InputManager::GetInstance()->UpdateDisplayInfo(displayGroupInfo);
}

void SceneInputManager::FlushEmptyInfoToMMI()
{
    auto task = [this]() {
        std::vector<MMI::DisplayInfo> displayInfos;
        ConstructDisplayInfos(displayInfos);
        int mainScreenWidth = 0;
        int mainScreenHeight = 0;
        if (!displayInfos.empty()) {
            mainScreenWidth = displayInfos[0].width;
            mainScreenHeight = displayInfos[0].height;
        }
        MMI::DisplayGroupInfo displayGroupInfo = {
            .width = mainScreenWidth,
            .height = mainScreenHeight,
            .focusWindowId = EMPTY_FOCUS_WINDOW_ID,
            .currentUserId = currentUserId_,
            .displaysInfo = displayInfos
        };
        TLOGI(WmsLogTag::WMS_EVENT, "currUserId:%{public}d width:%{public}d height:%{public}d",
            currentUserId_, mainScreenWidth, mainScreenHeight);
        MMI::InputManager::GetInstance()->UpdateDisplayInfo(displayGroupInfo);
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

bool SceneInputManager::CheckNeedUpdate(const std::vector<MMI::DisplayInfo>& displayInfos,
    const std::vector<MMI::WindowInfo>& windowInfoList)
{
    int32_t focusId = SceneSessionManager::GetInstance().GetFocusedSessionId();
    if (focusId != lastFocusId_) {
        lastFocusId_ = focusId;
        lastDisplayInfos_ = displayInfos;
        lastWindowInfoList_ = windowInfoList;
        return true;
    }

    if (displayInfos.size() != lastDisplayInfos_.size() || windowInfoList.size() != lastWindowInfoList_.size()) {
        lastDisplayInfos_ = displayInfos;
        lastWindowInfoList_ = windowInfoList;
        return true;
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
    TLOGI(WmsLogTag::WMS_EVENT, "HostId:%{public}d surfaceId:%{public}" PRIu64
        " uiExtentionWindowInfo:%{public}d",
        windowInfo.id, surfaceId, static_cast<int>(windowInfo.uiExtentionWindowInfo.size()));
    for (auto uiExWindowinfo : windowInfo.uiExtentionWindowInfo) {
        auto str = DumpWindowInfo(uiExWindowinfo);
        str = "sec:" + std::to_string(uiExWindowinfo.privacyUIFlag) + " " + str;
        TLOGI(WmsLogTag::WMS_EVENT, "uiExWindowinfo:%{public}s", str.c_str());
    }
}

void SceneInputManager::PrintWindowInfo(const std::vector<MMI::WindowInfo>& windowInfoList)
{
    int windowListSize = static_cast<int>(windowInfoList.size());
    std::string idList;
    static std::string lastIdList;
    static uint32_t windowEventID = 0;
    if (windowEventID == UINT32_MAX) {
        windowEventID = 0;
    }
    focusedSessionId_ = SceneSessionManager::GetInstance().GetFocusedSessionId();
    std::unordered_map<int32_t, MMI::Rect> currWindowDefaultHotArea;
    static std::unordered_map<int32_t, MMI::Rect> lastWindowDefaultHotArea;
    for (auto& e : windowInfoList) {
        idList += std::to_string(e.id) + "|" + std::to_string(e.flags) + "|" +
            std::to_string(static_cast<int32_t>(e.zOrder)) + "|" +
            std::to_string(e.pid) + "|" +
            std::to_string(e.defaultHotAreas.size());

        if (e.defaultHotAreas.size() > 0) {
            auto iter = lastWindowDefaultHotArea.find(e.id);
            if (iter == lastWindowDefaultHotArea.end() || iter->second != e.defaultHotAreas[0]) {
                idList += "|" + std::to_string(e.defaultHotAreas[0].x) + "|" +
                    std::to_string(e.defaultHotAreas[0].y) + "|" +
                    std::to_string(e.defaultHotAreas[0].width) + "|" +
                    std::to_string(e.defaultHotAreas[0].height);
            }
            currWindowDefaultHotArea.insert({e.id, e.defaultHotAreas[0]});
        }
        idList += ",";
        if ((focusedSessionId_ == e.id) && (e.id == e.agentWindowId)) {
            UpdateFocusedSessionId(focusedSessionId_);
        }
        if (e.uiExtentionWindowInfo.size() > 0) {
            DumpUIExtentionWindowInfo(e);
        }
    }
    lastWindowDefaultHotArea = currWindowDefaultHotArea;
    SingleHandTransform transform = SceneSessionManager::GetInstance().GetNormalSingleHandTransform();
    idList += std::to_string(focusedSessionId_) + "|" +
        std::to_string(transform.posX) + "|" + std::to_string(transform.posY) + "|" +
        std::to_string(transform.scaleX) + "|" + std::to_string(transform.scaleY);
    if (lastIdList != idList) {
        windowEventID++;
        TLOGI(WmsLogTag::WMS_EVENT, "eid:%{public}d,size:%{public}d,idList:%{public}s",
            windowEventID, windowListSize, idList.c_str());
        lastIdList = idList;
    }
}

void SceneInputManager::SetUserBackground(bool userBackground)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "userBackground = %{public}d", userBackground);
    isUserBackground_.store(userBackground);
}

void SceneInputManager::SetCurrentUserId(int32_t userId)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "Current userId = %{public}d", userId);
    currentUserId_ = userId;
    MMI::InputManager::GetInstance()->SetCurrentUser(userId);
}

void SceneInputManager::UpdateDisplayAndWindowInfo(const std::vector<MMI::DisplayInfo>& displayInfos,
    std::vector<MMI::WindowInfo> windowInfoList)
{
    if (windowInfoList.size() == 0) {
        return;
    }
    int32_t windowBatchSize = MAX_WINDOWINFO_NUM;
    if (windowInfoList[0].defaultHotAreas.size() > MMI::WindowInfo::DEFAULT_HOTAREA_COUNT) {
        windowBatchSize = MMI::InputManager::GetInstance()->GetWinSyncBatchSize(
            static_cast<int32_t>(windowInfoList[0].defaultHotAreas.size()),
            static_cast<int32_t>(displayInfos.size()));
    }
    windowInfoList.back().action = MMI::WINDOW_UPDATE_ACTION::ADD_END;
    int32_t windowListSize = static_cast<int32_t>(windowInfoList.size());
    if (windowListSize <= windowBatchSize) {
        FlushFullInfoToMMI(displayInfos, windowInfoList);
        return;
    }
    auto iterBegin = windowInfoList.begin();
    auto iterEnd = windowInfoList.end();
    auto iterNext = std::next(iterBegin, windowBatchSize);
    FlushFullInfoToMMI(displayInfos, std::vector<MMI::WindowInfo>(iterBegin, iterNext));
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

void SceneInputManager::FlushDisplayInfoToMMI(const bool forceFlush)
{
    auto task = [this, forceFlush]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "FlushDisplayInfoToMMI");
        if (isUserBackground_.load()) {
            TLOGD(WmsLogTag::WMS_MULTI_USER, "User in background, no need to flush display info");
            return;
        }
        if (sceneSessionDirty_ == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "sceneSessionDirty_ is nullptr");
            return;
        }
        sceneSessionDirty_->ResetSessionDirty();
        std::vector<MMI::DisplayInfo> displayInfos;
        ConstructDisplayInfos(displayInfos);
        auto [windowInfoList, pixelMapList] = sceneSessionDirty_->GetFullWindowInfoList();
        if (!forceFlush && !CheckNeedUpdate(displayInfos, windowInfoList)) {
            return;
        }
        PrintWindowInfo(windowInfoList);
        if (windowInfoList.size() == 0) {
            FlushFullInfoToMMI(displayInfos, windowInfoList);
            return;
        }
        UpdateDisplayAndWindowInfo(displayInfos, std::move(windowInfoList));
    };
    if (eventHandler_) {
        eventHandler_->PostTask(task);
    }
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
}
} // namespace OHOS::Rosen
