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

#include "scene_session_dirty_manager.h"

#include <cinttypes>
#include <memory>
#include <sstream>
#include <parameter.h>
#include <parameters.h>
#include "screen_session_manager/include/screen_session_manager_client.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_helper.h"
#include "wm_common_inner.h"
#include "transaction/rs_uiextension_data.h"

namespace OHOS::Rosen {
namespace {
constexpr float DIRECTION0 = 0 ;
constexpr float DIRECTION90 = 90 ;
constexpr float DIRECTION180 = 180 ;
constexpr float DIRECTION270 = 270 ;
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionDirtyManager"};
constexpr unsigned int POINTER_CHANGE_AREA_COUNT = 8;
constexpr int POINTER_CHANGE_AREA_SIXTEEN = 16;
constexpr int POINTER_CHANGE_AREA_DEFAULT = 0;
constexpr int POINTER_CHANGE_AREA_FIVE = 5;
constexpr unsigned int TRANSFORM_DATA_LEN = 9;
constexpr int UPDATE_TASK_DURATION = 10;
const std::string UPDATE_WINDOW_INFO_TASK = "UpdateWindowInfoTask";
static int32_t g_screenRotationOffset = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
constexpr float ZORDER_UIEXTENSION_INDEX = 0.1;
} //namespace

static bool operator==(const MMI::Rect left, const MMI::Rect right)
{
    return ((left.x == right.x) && (left.y == right.y) && (left.width == right.width) && (left.height == right.height));
}

MMI::Direction ConvertDegreeToMMIRotation(float degree, MMI::DisplayMode displayMode)
{
    MMI::Direction rotation = MMI::DIRECTION0;
    if (NearEqual(degree, DIRECTION0)) {
        rotation = MMI::DIRECTION0;
    }
    if (NearEqual(degree, DIRECTION90)) {
        rotation = MMI::DIRECTION90;
    }
    if (NearEqual(degree, DIRECTION180)) {
        rotation = MMI::DIRECTION180;
    }
    if (NearEqual(degree, DIRECTION270)) {
        rotation = MMI::DIRECTION270;
    }
    if (displayMode == MMI::DisplayMode::FULL && g_screenRotationOffset != 0) {
        switch (rotation) {
            case MMI::DIRECTION0:
                rotation = MMI::DIRECTION90;
                break;
            case MMI::DIRECTION90:
                rotation = MMI::DIRECTION180;
                break;
            case MMI::DIRECTION180:
                rotation = MMI::DIRECTION270;
                break;
            case MMI::DIRECTION270:
                rotation = MMI::DIRECTION0;
                break;
            default:
                rotation = MMI::DIRECTION0;
                break;
        }
    }
    return rotation;
}

bool CmpMMIWindowInfo(const MMI::WindowInfo& a, const MMI::WindowInfo& b)
{
    return a.defaultHotAreas.size() > b.defaultHotAreas.size();
}

void SceneSessionDirtyManager::CalNotRotateTramform(const sptr<SceneSession> sceneSession, Matrix3f& tranform,
    bool useUIExtension) const
{
    if (sceneSession == nullptr || sceneSession->GetSessionProperty() == nullptr) {
        WLOGFE("SceneSession or SessionProperty is nullptr");
        return;
    }
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto displayMode = Rosen::ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    std::map<ScreenId, ScreenProperty> screensProperties =
        Rosen::ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    if (screensProperties.find(displayId) == screensProperties.end()) {
        return;
    }
    auto screenProperty = screensProperties[displayId];
    auto screenSession = Rosen::ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
    MMI::Direction displayRotation = ConvertDegreeToMMIRotation(screenProperty.GetRotation(),
        static_cast<MMI::DisplayMode>(displayMode));
    float width = screenProperty.GetBounds().rect_.GetWidth();
    float height = screenProperty.GetBounds().rect_.GetHeight();
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f offset = sceneSession->GetPosition(useUIExtension);
    Vector2f translate = offset;
    float rotate = 0.0f;
    switch (displayRotation) {
        case MMI::DIRECTION0: {
            break;
        }
        case MMI::DIRECTION270: {
            translate.x_ = offset.y_;
            translate.y_ = height - offset.x_;
            rotate = -M_PI_2;
            break;
        }
        case MMI::DIRECTION180:
            translate.x_ = width - offset.x_;
            translate.y_ = height - offset.y_;
            rotate = M_PI;
            break;
        case MMI::DIRECTION90: {
            translate.x_ = width - offset.y_;
            translate.y_ = offset.x_;
            rotate = M_PI_2;
            break;
        }
        default:
            break;
    }
    tranform = tranform.Translate(translate).Rotate(rotate).Scale(scale, sceneSession->GetPivotX(),
        sceneSession->GetPivotY());
    tranform = tranform.Inverse();
}

void SceneSessionDirtyManager::CalTramform(const sptr<SceneSession> sceneSession, Matrix3f& tranform,
    bool useUIExtension) const
{
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        return;
    }
    tranform = Matrix3f::IDENTITY;
    bool isRotate = sceneSession->GetSessionInfo().isRotable_;
    auto displayMode = Rosen::ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    if (isRotate || !sceneSession->GetSessionInfo().isSystem_ ||
        static_cast<MMI::DisplayMode>(displayMode) == MMI::DisplayMode::FULL) {
        Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
        Vector2f translate = sceneSession->GetPosition(useUIExtension);
        tranform = tranform.Translate(translate);
        tranform = tranform.Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY());
        tranform = tranform.Inverse();
        return;
    }
    CalNotRotateTramform(sceneSession, tranform, useUIExtension);
}


void SceneSessionDirtyManager::UpdateDefaultHotAreas(sptr<SceneSession> sceneSession,
    std::vector<MMI::Rect>& touchHotAreas,
    std::vector<MMI::Rect>& pointerHotAreas) const
{
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        return;
    }
    WSRect windowRect = sceneSession->GetSessionRect();
    uint32_t touchOffset = 0;
    uint32_t pointerOffset = 0;
    if ((sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) ||
        (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_PIP)) {
        float vpr = 1.5f; // 1.5: default vp
        if (sceneSession->GetSessionProperty() != nullptr) {
            auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
            auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
            if (screenSession != nullptr) {
                vpr = screenSession->GetScreenProperty().GetDensity();
            }
        }
        WLOGFD("[WMSEvent] UpdateDefaultHotAreas, vpr: %{public}f", vpr);
        touchOffset = static_cast<uint32_t>(HOTZONE_TOUCH * vpr);
        pointerOffset = static_cast<uint32_t>(HOTZONE_POINTER * vpr);
    }

    MMI::Rect touchRect = {
        .x = -touchOffset,
        .y = -touchOffset,
        .width = windowRect.width_ + static_cast<int32_t>(touchOffset * 2),  // 2 : double touchOffset
        .height = windowRect.height_ + static_cast<int32_t>(touchOffset * 2) // 2 : double touchOffset
    };

    MMI::Rect pointerRect = {
        .x = -pointerOffset,
        .y = -pointerOffset,
        .width = windowRect.width_ + static_cast<int32_t>(pointerOffset * 2),  // 2 : double pointerOffset
        .height = windowRect.height_ + static_cast<int32_t>(pointerOffset * 2) // 2 : double pointerOffset
    };

    touchHotAreas.emplace_back(touchRect);
    pointerHotAreas.emplace_back(pointerRect);
}

void SceneSessionDirtyManager::UpdateHotAreas(sptr<SceneSession> sceneSession, std::vector<MMI::Rect>& touchHotAreas,
    std::vector<MMI::Rect>& pointerHotAreas) const
{
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        return;
    }

    WSRect windowRect = sceneSession->GetSessionRect();
    const std::vector<Rect>& hotAreas = sceneSession->GetTouchHotAreas();
    for (auto area : hotAreas) {
        MMI::Rect rect;
        rect.x = area.posX_;
        rect.y = area.posY_;
        rect.width = static_cast<int32_t>(area.width_);
        rect.height = static_cast<int32_t>(area.height_);
        auto iter = std::find_if(touchHotAreas.begin(), touchHotAreas.end(),
            [&rect](const MMI::Rect& var) { return rect == var; });
        if (iter != touchHotAreas.end()) {
            continue;
        }
        touchHotAreas.emplace_back(rect);
        pointerHotAreas.emplace_back(rect);
        if (touchHotAreas.size() == static_cast<uint32_t>(MMI::WindowInfo::MAX_HOTAREA_COUNT)) {
            auto sessionid = sceneSession->GetWindowId();
            WLOGFE("id = %{public}d hotAreas size > %{public}d", sessionid, static_cast<int>(hotAreas.size()));
            break;
        }
    }

    if (touchHotAreas.empty()) {
        return UpdateDefaultHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    }
}

std::map<int32_t, sptr<SceneSession>> SceneSessionDirtyManager::GetDialogSessionMap(
    const std::map<int32_t, sptr<SceneSession>>& sessionMap) const
{
    std::map<int32_t, sptr<SceneSession>> dialogMap;

    for (const auto& elem: sessionMap) {
        const auto& session = elem.second;
        if (session == nullptr || session->GetForceHideState() != ForceHideState::NOT_HIDDEN) {
            continue;
        }
        bool isModalSubWindow = false;
        const auto& property = session->GetSessionProperty();
        if (property != nullptr) {
            isModalSubWindow = WindowHelper::IsModalSubWindow(property->GetWindowType(), property->GetWindowFlags());
        }
        if (isModalSubWindow || session->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
            const auto& parentSession = session->GetParentSession();
            if (parentSession == nullptr) {
                continue;
            }
            bool isTopmostModalSubWindow = false;
            if (property != nullptr && property->IsTopmost()) {
                isTopmostModalSubWindow = true;
            }
            auto iter = dialogMap.find(parentSession->GetPersistentId());
            if (iter != dialogMap.end() && iter->second != nullptr && iter->second->GetSessionProperty() &&
                iter->second->GetSessionProperty()->IsTopmost() && isTopmostModalSubWindow == false) {
                continue;
            }
            dialogMap[parentSession->GetPersistentId()] = session;
            WLOGFI("Add dialog session, id: %{public}d, parentId: %{public}d",
                session->GetPersistentId(), parentSession->GetPersistentId());
        }
    }
    return dialogMap;
}

bool SceneSessionDirtyManager::IsFilterSession(const sptr<SceneSession>& sceneSession) const
{
    if (sceneSession == nullptr) {
        return true;
    }

    if (sceneSession->IsSystemInput()) {
        return false;
    } else if (sceneSession->IsSystemSession() && sceneSession->IsVisible() && sceneSession->IsSystemActive()) {
        return false;
    }
    if (!Rosen::SceneSessionManager::GetInstance().IsSessionVisible(sceneSession)) {
        return true;
    }
    return false;
}

void SceneSessionDirtyManager::NotifyWindowInfoChange(const sptr<SceneSession>& sceneSession,
    const WindowUpdateType& type, const bool startMoving)
{
    if (sceneSession == nullptr) {
        WLOGFW("sceneSession is null");
        return;
    }

    if (type == WindowUpdateType::WINDOW_UPDATE_ADDED || type == WindowUpdateType::WINDOW_UPDATE_REMOVED||
        type == WindowUpdateType::WINDOW_UPDATE_ACTIVE) {
            WLOGFD("[EventDispatch] wid = %{public}d, winType = %{public}d",
                sceneSession->GetWindowId(), static_cast<int>(type));
    }
    sessionDirty_.store(true);
    if (!hasPostTask_.load()) {
        hasPostTask_.store(true);
        auto task = [this]() {
            hasPostTask_.store(false);
            if (!sessionDirty_.load() || flushWindowInfoCallback_ == nullptr) {
                return;
            }
            flushWindowInfoCallback_();
        };
        SceneSessionManager::GetInstance().PostFlushWindowInfoTask(task,
            UPDATE_WINDOW_INFO_TASK, UPDATE_TASK_DURATION);
    }
}

void SceneSessionDirtyManager::UpdateModalExtensionWindowInfo(const sptr<SceneSession> sceneSession,
    MMI::WindowInfo& windowInfo)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    auto extensionInfo = sceneSession->GetLastModalUIExtensionEventInfo();
    windowInfo.agentWindowId = extensionInfo.persistentId;
    windowInfo.pid = extensionInfo.pid;
    std::vector<MMI::Rect> touchHotAreas;
    WSRect windowRect = sceneSession->GetSessionRect();
    MMI::Rect touchRect = {
        .x = 0,
        .y = 0,
        .width = windowRect.width_,
        .height = windowRect.height_
    };
    touchHotAreas.emplace_back(touchRect);
    windowInfo.defaultHotAreas = touchHotAreas;
    windowInfo.pointerHotAreas = touchHotAreas;
}

void SceneSessionDirtyManager::AddModalExtensionWindowInfo(std::vector<MMI::WindowInfo>& windowInfoList,
    MMI::WindowInfo windowInfo, const sptr<SceneSession> sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }

    auto extensionInfo = sceneSession->GetLastModalUIExtensionEventInfo();
    windowInfo.id = extensionInfo.persistentId;
    if (extensionInfo.windowRect.width_ != 0 || extensionInfo.windowRect.height_ != 0) {
        MMI::Rect windowRect = {
            .x = extensionInfo.windowRect.posX_,
            .y = extensionInfo.windowRect.posY_,
            .width = extensionInfo.windowRect.width_,
            .height = extensionInfo.windowRect.height_
        };
        windowInfo.area =  windowRect;
        std::vector<MMI::Rect> touchHotAreas;
        MMI::Rect touchRect = {
            .x = 0,
            .y = 0,
            .width = extensionInfo.windowRect.width_,
            .height = extensionInfo.windowRect.height_
        };
        touchHotAreas.emplace_back(touchRect);
        windowInfo.defaultHotAreas = touchHotAreas;
        windowInfo.pointerHotAreas = touchHotAreas;
        Matrix3f tranform;
        CalTramform(sceneSession, tranform, true);
        std::vector<float> transformData(tranform.GetData(), tranform.GetData() + TRANSFORM_DATA_LEN);
        windowInfo.transform = transformData;
    }

    windowInfo.agentWindowId = extensionInfo.persistentId;
    windowInfo.pid = extensionInfo.pid;
    std::vector<int32_t> pointerChangeAreas(POINTER_CHANGE_AREA_COUNT, 0);
    windowInfo.pointerChangeAreas = pointerChangeAreas;
    windowInfo.zOrder = windowInfo.zOrder + ZORDER_UIEXTENSION_INDEX;

    windowInfoList.emplace_back(windowInfo);
}

std::vector<MMI::WindowInfo> SceneSessionDirtyManager::GetFullWindowInfoList()
{
    std::vector<MMI::WindowInfo> windowInfoList;
    const auto sceneSessionMap = Rosen::SceneSessionManager::GetInstance().GetSceneSessionMap();
    // all input event should trans to dialog window if dialog exists
    const auto dialogMap = GetDialogSessionMap(sceneSessionMap);
    uint32_t maxHotAreasNum = 0;
    for (const auto& sceneSessionValuePair : sceneSessionMap) {
        const auto& sceneSessionValue = sceneSessionValuePair.second;
        if (sceneSessionValue == nullptr) {
            continue;
        }
        WLOGFD("[EventDispatch] FullSceneSessionInfoUpdate windowName = %{public}s bundleName = %{public}s"
            " windowId = %{public}d activeStatus = %{public}d", sceneSessionValue->GetWindowName().c_str(),
            sceneSessionValue->GetSessionInfo().bundleName_.c_str(), sceneSessionValue->GetWindowId(),
            sceneSessionValue->GetForegroundInteractiveStatus());
        auto windowInfo = GetWindowInfo(sceneSessionValue, WindowAction::WINDOW_ADD);
        auto iter = (sceneSessionValue->GetParentPersistentId() == INVALID_SESSION_ID) ?
            dialogMap.find(sceneSessionValue->GetPersistentId()) :
            dialogMap.find(sceneSessionValue->GetParentPersistentId());
        if (iter != dialogMap.end() && iter->second != nullptr &&
            sceneSessionValue->GetPersistentId() != iter->second->GetPersistentId()) {
            windowInfo.agentWindowId = static_cast<int32_t>(iter->second->GetPersistentId());
            windowInfo.pid = static_cast<int32_t>(iter->second->GetCallingPid());
        } else if (sceneSessionValue->HasModalUIExtension()) {
            if (sceneSessionValue->GetSessionProperty()->IsDecorEnable()) {
                AddModalExtensionWindowInfo(windowInfoList, windowInfo, sceneSessionValue);
            } else {
                UpdateModalExtensionWindowInfo(sceneSessionValue, windowInfo);
            }
        }
        windowInfoList.emplace_back(windowInfo);
        if (windowInfo.defaultHotAreas.size() > maxHotAreasNum) {
            maxHotAreasNum = windowInfo.defaultHotAreas.size();
        }
    }
    if (maxHotAreasNum > MMI::WindowInfo::DEFAULT_HOTAREA_COUNT) {
        std::sort(windowInfoList.begin(), windowInfoList.end(), CmpMMIWindowInfo);
    }
    return windowInfoList;
}

void SceneSessionDirtyManager::UpdatePointerAreas(sptr<SceneSession> sceneSession,
    std::vector<int32_t>& pointerChangeAreas) const
{
    bool dragEnabled = sceneSession->GetSessionProperty()->GetDragEnabled();
    if (dragEnabled) {
        float vpr = 1.5f; // 1.5: default vp
        if (sceneSession->GetSessionProperty() != nullptr) {
            auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
            auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
            if (screenSession != nullptr) {
                vpr = screenSession->GetScreenProperty().GetDensity();
            }
        }
        int32_t pointerAreaFivePx = static_cast<int32_t>(POINTER_CHANGE_AREA_FIVE * vpr);
        int32_t pointerAreaSixteenPx = static_cast<int32_t>(POINTER_CHANGE_AREA_SIXTEEN * vpr);

        if (sceneSession->GetSessionInfo().isSetPointerAreas_) {
            pointerChangeAreas = {POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
                POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx, pointerAreaSixteenPx,
                pointerAreaFivePx, pointerAreaSixteenPx, pointerAreaFivePx};
            return;
        }
        auto limits = sceneSession->GetSessionProperty()->GetWindowLimits();
        if (limits.minWidth_ == limits.maxWidth_ && limits.minHeight_ != limits.maxHeight_) {
            pointerChangeAreas = {POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx,
                POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
                pointerAreaFivePx, POINTER_CHANGE_AREA_DEFAULT,  POINTER_CHANGE_AREA_DEFAULT};
        } else if (limits.minWidth_ != limits.maxWidth_ && limits.minHeight_ == limits.maxHeight_) {
            pointerChangeAreas = {POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
                POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx, POINTER_CHANGE_AREA_DEFAULT,
                POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx};
        } else if (limits.minWidth_ != limits.maxWidth_ && limits.minHeight_ != limits.maxHeight_) {
            pointerChangeAreas = {pointerAreaSixteenPx, pointerAreaFivePx,
                pointerAreaSixteenPx, pointerAreaFivePx, pointerAreaSixteenPx,
                pointerAreaFivePx, pointerAreaSixteenPx, pointerAreaFivePx};
        }
    } else {
        WLOGFD("UpdatePointerAreas sceneSession is: %{public}d dragEnabled is false", sceneSession->GetPersistentId());
    }
}

void SceneSessionDirtyManager::UpdatePrivacyMode(const sptr<SceneSession> sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    windowInfo.privacyMode = MMI::SecureFlag::DEFAULT_MODE;
    sptr<WindowSessionProperty> windowSessionProperty = sceneSession->GetSessionProperty();
    if (windowSessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowSessionProperty is nullptr");
        return;
    }
    if (windowSessionProperty->GetPrivacyMode() || windowSessionProperty->GetSystemPrivacyMode()) {
        windowInfo.privacyMode = MMI::SecureFlag::PRIVACY_MODE;
    }
}

void SceneSessionDirtyManager::UpdateWindowFlags(DisplayId displayId, const sptr<SceneSession> sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    windowInfo.flags = 0;
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    if (screenSession != nullptr) {
        if (!screenSession->IsTouchEnabled()) {
            windowInfo.flags = MMI::WindowInfo::FLAG_BIT_UNTOUCHABLE;
        } else {
            windowInfo.flags = (!sceneSession->GetSystemTouchable() || !sceneSession->GetForegroundInteractiveStatus());
        }
    }
}

MMI::WindowInfo SceneSessionDirtyManager::GetWindowInfo(const sptr<SceneSession>& sceneSession,
    const SceneSessionDirtyManager::WindowAction& action) const
{
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        return {};
    }
    sptr<WindowSessionProperty> windowSessionProperty = sceneSession->GetSessionProperty();
    if (windowSessionProperty == nullptr) {
        WLOGFE("SceneSession property is nullptr");
        return {};
    }
    Matrix3f tranform;
    WSRect windowRect = sceneSession->GetSessionRect();
    auto pid = sceneSession->GetCallingPid();
    auto uid = sceneSession->GetCallingUid();
    auto windowId = sceneSession->GetWindowId();
    auto displayId = windowSessionProperty->GetDisplayId();
    CalTramform(sceneSession, tranform);
    std::vector<float> transformData(tranform.GetData(), tranform.GetData() + TRANSFORM_DATA_LEN);

    auto agentWindowId = sceneSession->GetWindowId();
    auto zOrder = sceneSession->GetZOrder();
    std::vector<int32_t> pointerChangeAreas(POINTER_CHANGE_AREA_COUNT, 0);
    auto windowMode = windowSessionProperty->GetWindowMode();
    auto maxMode = windowSessionProperty->GetMaximizeMode();
    WindowType windowType = windowSessionProperty->GetWindowType();
    bool isMainWindow = Rosen::WindowHelper::IsMainWindow(windowType);
    bool isDecorDialog = Rosen::WindowHelper::IsDialogWindow(windowType) && windowSessionProperty->IsDecorEnable();
    if ((windowMode == Rosen::WindowMode::WINDOW_MODE_FLOATING &&
        (isMainWindow || isDecorDialog) &&
        maxMode != Rosen::MaximizeMode::MODE_AVOID_SYSTEM_BAR) || (sceneSession->GetSessionInfo().isSetPointerAreas_)) {
            UpdatePointerAreas(sceneSession, pointerChangeAreas);
    }
    std::vector<MMI::Rect> touchHotAreas;
    std::vector<MMI::Rect> pointerHotAreas;
    UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    auto pixelMap = windowSessionProperty->GetWindowMask().get();
    MMI::WindowInfo windowInfo = {
        .id = windowId,
        .pid = sceneSession->IsStartMoving() ? static_cast<int32_t>(getpid()) : pid,
        .uid = uid,
        .area = { windowRect.posX_, windowRect.posY_, windowRect.width_, windowRect.height_ },
        .defaultHotAreas = touchHotAreas,
        .pointerHotAreas = pointerHotAreas,
        .agentWindowId = agentWindowId,
        .displayId = displayId,
        .action = static_cast<MMI::WINDOW_UPDATE_ACTION>(action),
        .pointerChangeAreas = pointerChangeAreas,
        .zOrder = zOrder,
        .transform = transformData,
        .pixelMap = pixelMap,
        .windowInputType = static_cast<MMI::WindowInputType>(sceneSession->GetSessionInfo().windowInputType_),
        .windowType = static_cast<int32_t>(windowType),
    };
    UpdateWindowFlags(displayId, sceneSession, windowInfo);
    if (windowSessionProperty != nullptr && (windowSessionProperty->GetWindowFlags() &
        static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING))) {
        windowInfo.flags |= MMI::WindowInfo::FLAG_BIT_HANDWRITING;
    }
    UpdatePrivacyMode(sceneSession, windowInfo);
    windowInfo.uiExtentionWindowInfo = GetSecSurfaceWindowinfoList(sceneSession, windowInfo, tranform);
    return windowInfo;
}

void SceneSessionDirtyManager::RegisterFlushWindowInfoCallback(const FlushWindowInfoCallback &&callback)
{
    flushWindowInfoCallback_ = std::move(callback);
}

void SceneSessionDirtyManager::ResetSessionDirty()
{
    sessionDirty_.store(false);
}

std::string DumpRect(const std::vector<MMI::Rect>& rects)
{
    std::string rectStr = "";
    for (const auto& rect : rects) {
        rectStr = rectStr + " hot : [ " + std::to_string(rect.x) +" , " + std::to_string(rect.y) +
        " , " + std::to_string(rect.width) + " , " + std::to_string(rect.height) + "]";
    }
    return rectStr;
}

std::string DumpWindowInfo(const MMI::WindowInfo& info)
{
    std::string infoStr = "windowInfo:";
    infoStr = infoStr + "windowId: " + std::to_string(info.id) + " pid : " + std::to_string(info.pid) +
        " uid: " + std::to_string(info.uid) + " area: [ " + std::to_string(info.area.x) + " , " +
        std::to_string(info.area.y) +  " , " + std::to_string(info.area.width) + " , " +
        std::to_string(info.area.height) + "] agentWindowId:" + std::to_string(info.agentWindowId) + " flags:" +
        std::to_string(info.flags)  +" displayId: " + std::to_string(info.displayId) +
        " action: " + std::to_string(static_cast<int>(info.action)) + " zOrder: " + std::to_string(info.zOrder);
    return infoStr + DumpRect(info.defaultHotAreas);
}

std::string DumpSecRectInfo(const SecRectInfo & secRectInfo)
{
    std::string infoStr = " area: [ " + std::to_string(secRectInfo.relativeCoords.GetLeft()) + " , " +
        std::to_string(secRectInfo.relativeCoords.GetTop()) +  " , " +
        std::to_string(secRectInfo.relativeCoords.GetWidth()) + " , " +
        std::to_string(secRectInfo.relativeCoords.GetHeight()) + "]" +
        " scaleX:" + std::to_string(secRectInfo.scale[0]) + " scaleY:" + std::to_string(secRectInfo.scale[1]) +
        " anchorX:" + std::to_string(secRectInfo.anchor[0]) + " anchorY:" + std::to_string(secRectInfo.anchor[1]);
    return infoStr;
}

std::string DumpSecSurfaceInfo(const SecSurfaceInfo& secSurfaceInfo)
{
    std::string infoStr = "hostPid:" + std::to_string(secSurfaceInfo.hostPid) +
        " uiExtensionPid:" + std::to_string(secSurfaceInfo.uiExtensionPid) +
        " hostNodeId:" + std::to_string(secSurfaceInfo.hostNodeId) +
        " uiExtensionNodeId:" + std::to_string(secSurfaceInfo.uiExtensionNodeId);
    return infoStr;
}

MMI::WindowInfo SceneSessionDirtyManager::MakeWindowInfoFormHostWindow(const SecRectInfo& secRectInfo,
    const MMI::WindowInfo& hostWindowinfo) const
{
    MMI::WindowInfo windowinfo;
    windowinfo.id = hostWindowinfo.id;
    windowinfo.pid = hostWindowinfo.pid;
    windowinfo.uid = hostWindowinfo.uid;
    windowinfo.area = hostWindowinfo.area;
    windowinfo.agentWindowId = hostWindowinfo.agentWindowId;
    windowinfo.action = hostWindowinfo.action;
    windowinfo.displayId = hostWindowinfo.displayId;
    windowinfo.flags = hostWindowinfo.flags;
    windowinfo.privacyMode = hostWindowinfo.privacyMode;
    windowinfo.transform = hostWindowinfo.transform;
    return windowinfo;
}

Matrix3f CoordinateSystemHostWindowToScreen(const Matrix3f hostTranform, const SecRectInfo& secRectInfo)
{
    Matrix3f transform = Matrix3f::IDENTITY;
    Vector2f translate(secRectInfo.relativeCoords.GetLeft(), secRectInfo.relativeCoords.GetTop());
    transform = transform.Translate(translate);
    Vector2f scale(secRectInfo.scale[0], secRectInfo.scale[1]);
    transform = transform.Scale(scale, secRectInfo.anchor[0], secRectInfo.anchor[1]);
    transform = hostTranform.Inverse() * transform;
    return transform;
}

MMI::Rect CalRectInScreen(const Matrix3f& transform, const SecRectInfo& secRectInfo)
{
    auto value1 = transform * Vector3f(0, 0, 1.0);
    auto value2 = transform * Vector3f(secRectInfo.relativeCoords.GetWidth(),
        secRectInfo.relativeCoords.GetHeight(), 1.0);
    auto left = std::min(value1[0], value2[0]);
    auto top = std::min(value1[1], value2[1]);
    if (INT32_MIN + value2[0] > value1[0]) {
        TLOGE(WmsLogTag::WMS_EVENT, "data overflows value1:%{public}d value2:%{public}d",
            static_cast<int32_t>(value1[0]), static_cast<int32_t>(value1[1]));
            return {};
    }
    if (INT32_MAX + value2[0] < value1[0]) {
        TLOGE(WmsLogTag::WMS_EVENT, "data overflows value1:%{public}d value2:%{public}d",
            static_cast<int32_t>(value1[0]), static_cast<int32_t>(value1[1]));
            return {};
    }
    auto width = std::abs(value1[0] - value2[0]);
    auto height = std::abs(value1[1] - value2[1]);
    return MMI::Rect{ left, top, width, height};
}


MMI::WindowInfo SceneSessionDirtyManager::GetHostComponentWindowInfo(const SecSurfaceInfo& secSurfaceInfo,
    const MMI::WindowInfo& hostWindowinfo, const Matrix3f hostTranform) const
{
    MMI::WindowInfo windowinfo;
    const auto& secRectInfoList = secSurfaceInfo.upperNodes;
    if (secRectInfoList.size() > 0) {
        windowinfo = MakeWindowInfoFormHostWindow(secRectInfoList[0], hostWindowinfo);
    }
    for (const auto& secRectInfo : secRectInfoList) {
        auto infoStr = DumpSecRectInfo(secRectInfo);
        TLOGI(WmsLogTag::WMS_EVENT, "HostsecRectInfo:%{public}s", infoStr.c_str());
        windowinfo.pid = secSurfaceInfo.hostPid;
        MMI::Rect hotArea = { secRectInfo.relativeCoords.GetLeft(), secRectInfo.relativeCoords.GetTop(),
            secRectInfo.relativeCoords.GetWidth(), secRectInfo.relativeCoords.GetHeight() };
        windowinfo.defaultHotAreas.emplace_back(hotArea);
        windowinfo.pointerHotAreas.emplace_back(hotArea);
    }
    auto str = DumpWindowInfo(windowinfo);
    TLOGI(WmsLogTag::WMS_EVENT, "host:%{public}s", str.c_str());
    return windowinfo;
}

MMI::WindowInfo SceneSessionDirtyManager::GetSecComponentWindowInfo(const SecSurfaceInfo& secSurfaceInfo,
    const MMI::WindowInfo& hostWindowinfo, const sptr<SceneSession>& sceneSession, const Matrix3f hostTranform) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return {};
    }
    MMI::WindowInfo windowinfo;
    const auto& secRectInfo = secSurfaceInfo.uiExtensionRectInfo;
    auto infoStr = DumpSecRectInfo(secRectInfo);
    TLOGI(WmsLogTag::WMS_EVENT, "secRectInfo:%{public}s", infoStr.c_str());
    windowinfo = MakeWindowInfoFormHostWindow(secRectInfo, hostWindowinfo);
    windowinfo.id = sceneSession->GetUIExtPersistentIdBySurfaceNodeId(secSurfaceInfo.uiExtensionNodeId);
    if (windowinfo.id == 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "GetUIExtPersistentId ERROR");
        return {};
    }
    windowinfo.agentWindowId = windowinfo.id;
    windowinfo.pid = secSurfaceInfo.uiExtensionPid;
    windowinfo.privacyUIFlag = true;
    auto transform = CoordinateSystemHostWindowToScreen(hostTranform, secRectInfo);
    windowinfo.area = CalRectInScreen(transform, secRectInfo);
    MMI::Rect hotArea = { 0, 0, secRectInfo.relativeCoords.GetWidth(), secRectInfo.relativeCoords.GetHeight() };
    windowinfo.defaultHotAreas.emplace_back(hotArea);
    windowinfo.pointerHotAreas.emplace_back(hotArea);
    // 屏幕坐标系到控件坐标系转换
    transform = transform.Inverse();
    std::vector<float> transformData(transform.GetData(), transform.GetData() + TRANSFORM_DATA_LEN);
    windowinfo.transform = transformData;
    auto str = DumpWindowInfo(windowinfo);
    TLOGI(WmsLogTag::WMS_EVENT, "sec:%{public}s", str.c_str());
    return windowinfo;
}

void SceneSessionDirtyManager::UpdateSecSurfaceInfo(const std::map<uint64_t,
    std::vector<SecSurfaceInfo>>& secSurfaceInfoMap)
{
    std::unique_lock<std::shared_mutex> lock(secSurfaceInfoMutex_);
    TLOGD(WmsLogTag::WMS_EVENT, "secSurfaceInfoMap size:%{public}d", int(secSurfaceInfoMap.size()));
    secSurfaceInfoMap_ = secSurfaceInfoMap;
}

std::vector<MMI::WindowInfo> SceneSessionDirtyManager::GetSecSurfaceWindowinfoList(
    const sptr<SceneSession>& sceneSession, const MMI::WindowInfo& hostWindowinfo, const Matrix3f hostTranform) const
{
    if (secSurfaceInfoMap_.size() == 0) {
        return {};
    }
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return {};
    }
    auto surfaceNode = sceneSession->GetSurfaceNode();
    if (surfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "surfaceNode is nullptr");
        return {};
    }
    std::vector<SecSurfaceInfo> secSurfaceInfoList;
    auto surfaceNodeId = surfaceNode->GetId();
    {
        std::shared_lock<std::shared_mutex> lock(secSurfaceInfoMutex_);
        auto iter = secSurfaceInfoMap_.find(surfaceNodeId);
        if (iter == secSurfaceInfoMap_.end()) {
            return {};
        }
        secSurfaceInfoList = iter->second;
    }
    TLOGI(WmsLogTag::WMS_EVENT, "hostWindowId:%{public}d secSurfaceInfoList:%{public}d secSurfaceInfoMap:%{public}d",
        hostWindowinfo.id, static_cast<int>(secSurfaceInfoList.size()), static_cast<int>(secSurfaceInfoMap_.size()));
    std::vector<MMI::WindowInfo> windowinfoList;
    int seczOrder = 0;
    MMI::WindowInfo windowinfo;
    for (const auto& secSurfaceInfo : secSurfaceInfoList) {
        auto infoStr = DumpSecSurfaceInfo(secSurfaceInfo);
        TLOGI(WmsLogTag::WMS_EVENT, "secSurfaceInfo:%{public}s", infoStr.c_str());
        windowinfo = GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, hostTranform);
        windowinfo.zOrder = seczOrder++;
        windowinfoList.emplace_back(windowinfo);
        windowinfo = GetHostComponentWindowInfo(secSurfaceInfo, hostWindowinfo, hostTranform);
        windowinfo.zOrder = seczOrder++;
        windowinfoList.emplace_back(windowinfo);
    }
    TLOGI(WmsLogTag::WMS_EVENT, "surfaceNodeId:%{public}" PRIu64" windowinfoList:%{public}d",
        surfaceNodeId, int(windowinfoList.size()));
    return windowinfoList;
}
} //namespace OHOS::Rosen
