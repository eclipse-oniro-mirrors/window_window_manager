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

#include <cmath>
#include <parameters.h>
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_helper.h"
#include "fold_screen_state_internel.h"

namespace OHOS::Rosen {
namespace {
constexpr float DIRECTION0 = 0;
constexpr float DIRECTION90 = 90;
constexpr float DIRECTION180 = 180;
constexpr float DIRECTION270 = 270;
constexpr float DIRECTION360 = 360;
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionDirtyManager"};
constexpr unsigned int POINTER_CHANGE_AREA_COUNT = 8;
constexpr int POINTER_CHANGE_AREA_SIXTEEN = 16;
constexpr int POINTER_CHANGE_AREA_DEFAULT = 0;
constexpr int POINTER_CHANGE_AREA_FIVE = 5;
constexpr unsigned int TRANSFORM_DATA_LEN = 9;
constexpr int UPDATE_TASK_DURATION = 10;
constexpr uint32_t MMI_FLAG_BIT_LOCK_CURSOR_NOT_FOLLOW_MOVEMENT = 0x08;
constexpr uint32_t MMI_FLAG_BIT_LOCK_CURSOR_FOLLOW_MOVEMENT = 0x10;
const std::string UPDATE_WINDOW_INFO_TASK = "UpdateWindowInfoTask";
static int32_t g_screenRotationOffset = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
constexpr float ZORDER_UIEXTENSION_INDEX = 0.1;
constexpr int WINDOW_NAME_TYPE_UNKNOWN = 0;
constexpr int WINDOW_NAME_TYPE_THUMBNAIL = 1;
constexpr int WINDOW_NAME_TYPE_VOICEINPUT = 2;
const std::string SCREENSHOT_WINDOW_NAME_PREFIX = "ScreenShotWindow";
const std::string PREVIEW_WINDOW_NAME_PREFIX = "PreviewWindow";
const std::string VOICEINPUT_WINDOW_NAME_PREFIX = "__VoiceHardwareInput";
const std::string SCREEN_LOCK_WINDOW = "scbScreenLock";
constexpr int32_t CURSOR_DRAG_COUNT_MAX = 1;
} // namespace

static bool operator==(const MMI::Rect left, const MMI::Rect right)
{
    return ((left.x == right.x) && (left.y == right.y) && (left.width == right.width) && (left.height == right.height));
}

struct InputRectHash {
    std::size_t operator()(const MMI::Rect& r) const
    {
        std::size_t h1 = std::hash<int32_t>{}(r.x);
        std::size_t h2 = std::hash<int32_t>{}(r.y);
        std::size_t h3 = std::hash<int32_t>{}(r.width);
        std::size_t h4 = std::hash<int32_t>{}(r.height);
        return ((h1 * 31 + h2) * 31 + h3) * 31 + h4;
    }
};

struct InputRectEqual {
    bool operator()(const MMI::Rect& left, const MMI::Rect& right) const
    {
        return left.x == right.x && left.y == right.y &&
               left.width == right.width && left.height == right.height;
    }
};

MMI::Direction ConvertDegreeToMMIRotation(float degree)
{
    MMI::Direction rotation = MMI::DIRECTION0;
    if (NearEqual(degree, DIRECTION0)) {
        rotation = MMI::DIRECTION0;
    } else if (NearEqual(degree, DIRECTION90)) {
        rotation = MMI::DIRECTION90;
    } else if (NearEqual(degree, DIRECTION180)) {
        rotation = MMI::DIRECTION180;
    } else if (NearEqual(degree, DIRECTION270)) {
        rotation = MMI::DIRECTION270;
    }
    return rotation;
}

MMI::Rotation ConvertToMMIRotation(float degree)
{
    MMI::Rotation rotation = MMI::Rotation::ROTATION_0;
    if (NearEqual(degree, DIRECTION0)) {
        rotation = MMI::Rotation::ROTATION_0;
    } else if (NearEqual(degree, DIRECTION90)) {
        rotation = MMI::Rotation::ROTATION_90;
    } else if (NearEqual(degree, DIRECTION180)) {
        rotation = MMI::Rotation::ROTATION_180;
    } else if (NearEqual(degree, DIRECTION270)) {
        rotation = MMI::Rotation::ROTATION_270;
    }
    return rotation;
}

bool CmpMMIWindowInfo(const MMI::WindowInfo& a, const MMI::WindowInfo& b)
{
    return a.defaultHotAreas.size() > b.defaultHotAreas.size();
}

Vector2f CalRotationToTranslate(const MMI::Direction& displayRotation, float width, float height,
    const Vector2f& offset, float& rotate)
{
    Vector2f translate = offset;
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
    return translate;
}

Matrix3f GetTransformFromWindowInfo(const MMI::WindowInfo& hostWindowInfo)
{
    const std::vector<float>& hostTransform = hostWindowInfo.transform;
    if (hostTransform.size() != TRANSFORM_DATA_LEN) {
        TLOGE(WmsLogTag::WMS_EVENT, "transform data len invalid, id: %{public}d", hostWindowInfo.id);
        return Matrix3f();
    }
    return Matrix3f(hostTransform[0], hostTransform[1], hostTransform[2],  // 0,1,2: matrix index
                    hostTransform[3], hostTransform[4], hostTransform[5],  // 3,4,5: matrix index
                    hostTransform[6], hostTransform[7], hostTransform[8]); // 6,7,8: matrix index
}

void SceneSessionDirtyManager::CalNotRotateTransform(const sptr<SceneSession>& sceneSession, Matrix3f& transform,
    bool useUIExtension) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sessionProperty is nullptr");
        return;
    }
    auto displayId = sessionProperty->GetDisplayId();
    std::map<ScreenId, ScreenProperty> screensProperties =
        Rosen::ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    if (screensProperties.find(displayId) == screensProperties.end()) {
        return;
    }
    auto screenProperty = screensProperties[displayId];
    MMI::Direction displayRotation = ConvertDegreeToMMIRotation(screenProperty.GetPhysicalRotation());
    float width = screenProperty.GetBounds().rect_.GetWidth();
    float height = screenProperty.GetBounds().rect_.GetHeight();
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f offset = sceneSession->GetSessionGlobalPosition(useUIExtension);
    float rotate = 0.0f;
    Vector2f translate = CalRotationToTranslate(displayRotation, width, height, offset, rotate);
    transform = transform.Translate(translate).Rotate(rotate)
                         .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse();
}

inline float PositiveFmod(float x, float y)
{
    float result = std::fmod(x, y);
    return result < 0 ? (result + y) : result;
}

void SceneSessionDirtyManager::CalSpecialNotRotateTransform(const sptr<SceneSession>& sceneSession,
    ScreenProperty& screenProperty, Matrix3f& transform, bool useUIExtension) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    auto physicalRotation = screenProperty.GetPhysicalRotation();
    auto componentRotation = screenProperty.GetScreenComponentRotation();
    auto currentRotation = sceneSession->GetCurrentRotation();
    auto rotation = physicalRotation - componentRotation - currentRotation;
    MMI::Direction displayRotation =
        ConvertDegreeToMMIRotation(PositiveFmod(rotation, DIRECTION360));
    TLOGD(WmsLogTag::WMS_EVENT, "wid:%{public}d, physicalRotation:%{public}f, componentRotation:%{public}f,"
        " currentRotation:%{public}d, rotation:%{public}f, displayRotation:%{public}d", sceneSession->GetWindowId(),
        physicalRotation, componentRotation, currentRotation, rotation, static_cast<int32_t>(displayRotation));
    float width = screenProperty.GetBounds().rect_.GetWidth();
    float height = screenProperty.GetBounds().rect_.GetHeight();
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f offset = sceneSession->GetSessionGlobalPosition(useUIExtension);
    float rotate = 0.0f;
    Vector2f translate = CalRotationToTranslate(displayRotation, width, height, offset, rotate);
    transform = transform.Translate(translate).Rotate(rotate)
                         .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse();
}

void SceneSessionDirtyManager::CalTransform(const sptr<SceneSession>& sceneSession, Matrix3f& transform,
    const SingleHandData& singleHandData, bool useUIExtension) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    transform = Matrix3f::IDENTITY;
    bool isRotate = sceneSession->GetSessionInfo().isRotable_;
    auto displayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    if (singleHandData.mode != SingleHandMode::MIDDLE) {
        transform = transform.Scale({singleHandData.scaleX, singleHandData.scaleY},
                                    singleHandData.pivotX, singleHandData.pivotY);
    }
 
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sessionProperty is nullptr");
        return;
    }
    auto displayId = sessionProperty->GetDisplayId();
    std::map<ScreenId, ScreenProperty> screensProperties =
        Rosen::ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    if (screensProperties.find(displayId) == screensProperties.end()) {
        TLOGE(WmsLogTag::WMS_EVENT, "screensProperties find displayId failed");
        return;
    }
    auto screenProperty = screensProperties[displayId];
    auto isScreenLockWindow = sceneSession->GetSessionInfo().bundleName_.find(SCREEN_LOCK_WINDOW) != std::string::npos;
    bool isRotateWindow = !NearEqual(PositiveFmod(screenProperty.GetPhysicalRotation() -
        screenProperty.GetScreenComponentRotation(), DIRECTION360), DIRECTION360);
    bool isSystem = sceneSession->GetSessionInfo().isSystem_;
    bool displayModeIsFull = static_cast<MMI::DisplayMode>(displayMode) == MMI::DisplayMode::FULL;
    bool displayModeIsGlobalFull = displayMode == FoldDisplayMode::GLOBAL_FULL;
    bool displayModeIsMain = static_cast<MMI::DisplayMode>(displayMode) == MMI::DisplayMode::MAIN;
    bool displayModeIsCoordination = static_cast<MMI::DisplayMode>(displayMode) == MMI::DisplayMode::COORDINATION;
    bool foldScreenStateInternel = FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() ||
        FoldScreenStateInternel::IsSecondaryDisplayFoldDevice();
    TLOGD(WmsLogTag::WMS_EVENT, "wid:%{public}d, isRotate:%{public}d, isSystem:%{public}d,"
        " displayMode:%{public}d, foldScreenStateInternel:%{public}d, isRotateWindow:%{public}d,"
        " isScreenLockWindow:%{public}d", sceneSession->GetWindowId(), isRotate, isSystem,
        displayMode, foldScreenStateInternel, isRotateWindow, isScreenLockWindow);

    if (isRotate || !isSystem || displayModeIsFull || displayModeIsGlobalFull ||
        (displayModeIsMain && foldScreenStateInternel) || displayModeIsCoordination) {
        if (isScreenLockWindow && isRotateWindow) {
            CalSpecialNotRotateTransform(sceneSession, screenProperty, transform, useUIExtension);
            return;
        }
        Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
        Vector2f translate = sceneSession->GetSessionGlobalPosition(useUIExtension);
        if (useUIExtension && UpdateModalExtensionInCompatStatus(sceneSession, transform)) {
            TLOGD(WmsLogTag::WMS_EVENT, "sceneSession is compat mode");
            return;
        }
        transform = transform.Translate(translate)
                             .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse();
        return;
    }
    if (isScreenLockWindow && isRotateWindow) {
        CalSpecialNotRotateTransform(sceneSession, screenProperty, transform, useUIExtension);
        return;
    }
    CalNotRotateTransform(sceneSession, transform, useUIExtension);
}

bool SceneSessionDirtyManager::UpdateModalExtensionInCompatStatus(const sptr<SceneSession>& sceneSession,
    Matrix3f& transform) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return false;
    }
    if (!sceneSession->IsInCompatScaleStatus()) {
        TLOGD(WmsLogTag::WMS_EVENT, "sceneSession not is compat scale status");
        return false;
    }
    auto modalUIExtensionEventInfo = sceneSession->GetLastModalUIExtensionEventInfo();
    if (!modalUIExtensionEventInfo) {
        TLOGE(WmsLogTag::WMS_EVENT, "modalUIExtensionEventInfo is nullptr");
        return false;
    }
    const WSRect& rect = sceneSession->GetSessionGlobalRect();
    float heightDiff = rect.height_ - modalUIExtensionEventInfo.value().windowRect.height_;
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f translate(rect.posX_, rect.posY_ + heightDiff);
    transform = transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse();
    if (!sceneSession->GetSessionProperty()->IsAdaptToImmersive()) {
        Vector2f translateOffset(0, heightDiff * (1 - sceneSession->GetScaleY()));
        transform = transform.Translate(translateOffset);
    }
    return true;
}

void SceneSessionDirtyManager::UpdateDefaultHotAreas(sptr<SceneSession> sceneSession,
    std::vector<MMI::Rect>& touchHotAreas,
    std::vector<MMI::Rect>& pointerHotAreas) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    WSRect windowRect = sceneSession->GetSessionGlobalRectInMultiScreen();
    uint32_t touchOffset = 0;
    uint32_t pointerOffset = 0;
    bool isMidScene = sceneSession->GetIsMidScene();
    bool isAppPipWindow = sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_PIP;
    bool isAppMainWindow = sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    const auto& singleHandData = GetSingleHandData(sceneSession);
    sptr<WindowSessionProperty> windowSessionProperty = sceneSession->GetSessionProperty();
    bool isSystemOrSubWindow = (WindowHelper::IsSystemWindow(sceneSession->GetWindowType()) ||
        WindowHelper::IsSubWindow(sceneSession->GetWindowType()));
    bool isDragAccessibleWindow = windowSessionProperty->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
        sceneSession->IsDragAccessible();
    bool isSingleHandAffectedWindow = singleHandData.mode != SingleHandMode::MIDDLE &&
        windowSessionProperty->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN;
    bool isWindowSplit = windowSessionProperty->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
        windowSessionProperty->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY;
    if ((isAppPipWindow || isAppMainWindow || (isSystemOrSubWindow && isDragAccessibleWindow)) &&
        !isMidScene && !isSingleHandAffectedWindow && !isWindowSplit) {
        float vpr = 1.5f; // 1.5: default vp
        auto sessionProperty = sceneSession->GetSessionProperty();
        if (sessionProperty != nullptr) {
            auto displayId = sessionProperty->GetDisplayId();
            auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
            if (screenSession != nullptr) {
                vpr = screenSession->GetScreenProperty().GetDensity();
            }
        }
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

static void UpdateKeyboardHotAreasInner(const sptr<SceneSession>& sceneSession, std::vector<Rect>& hotAreas)
{
    sptr<SceneSession> session = (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_KEYBOARD_PANEL) ?
        sceneSession->GetKeyboardSession() : sceneSession;
    auto sessionProperty = session->GetSessionProperty();
    KeyboardTouchHotAreas keyboardTouchHotAreas = sessionProperty->GetKeyboardTouchHotAreas();
    auto displayId = (keyboardTouchHotAreas.displayId_ == DISPLAY_ID_INVALID) ?
                     sessionProperty->GetDisplayId() :
                     keyboardTouchHotAreas.displayId_;
    std::map<ScreenId, ScreenProperty> screensProperties =
        ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    if (screensProperties.find(displayId) == screensProperties.end()) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Set k-hotAreas failed: %{public}" PRIu64, displayId);
        return;
    }
    const auto& screenProperty = screensProperties[displayId];
    auto displayRect = screenProperty.GetBounds().rect_;
    int32_t displayWidth = displayRect.GetWidth();
    int32_t displayHeight = displayRect.GetHeight();
    bool isLandscape = displayWidth > displayHeight;
    if (displayWidth == displayHeight) {
        DisplayOrientation orientation = screenProperty.GetDisplayOrientation();
        if (orientation == DisplayOrientation::UNKNOWN) {
            TLOGW(WmsLogTag::WMS_KEYBOARD, "Display orientation is UNKNOWN");
        }
        isLandscape = (orientation == DisplayOrientation::LANDSCAPE ||
            orientation == DisplayOrientation::LANDSCAPE_INVERTED);
    }
    auto display = DisplayManager::GetInstance().GetDisplayById(displayId);
    std::string dispName = (display != nullptr) ? display->GetName() : "UNKNOWN";
    if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        if (keyboardTouchHotAreas.isKeyboardEmpty()) {
            return;
        }
        hotAreas = isLandscape ? keyboardTouchHotAreas.landscapeKeyboardHotAreas_ :
            keyboardTouchHotAreas.portraitKeyboardHotAreas_;
    } else if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_KEYBOARD_PANEL) {
        if (keyboardTouchHotAreas.isPanelEmpty()) {
            return;
        }
        hotAreas = isLandscape ? keyboardTouchHotAreas.landscapePanelHotAreas_ :
            keyboardTouchHotAreas.portraitPanelHotAreas_;
    }
}

void SceneSessionDirtyManager::UpdateHotAreas(const sptr<SceneSession>& sceneSession,
    std::vector<MMI::Rect>& touchHotAreas, std::vector<MMI::Rect>& pointerHotAreas) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    std::unordered_set<MMI::Rect, InputRectHash, InputRectEqual> hotAreaHashSet;
    std::vector<Rect> hotAreas = sceneSession->GetTouchHotAreas();
    if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
        sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_KEYBOARD_PANEL) {
        UpdateKeyboardHotAreasInner(sceneSession, hotAreas);
    }
    for (const auto& area : hotAreas) {
        MMI::Rect rect;
        rect.x = area.posX_;
        rect.y = area.posY_;
        rect.width = static_cast<int32_t>(area.width_);
        rect.height = static_cast<int32_t>(area.height_);
        if (hotAreaHashSet.count(rect)) {
            continue;
        }
        hotAreaHashSet.insert(rect);
        touchHotAreas.emplace_back(rect);
        pointerHotAreas.emplace_back(rect);
        if (touchHotAreas.size() == static_cast<uint32_t>(MMI::WindowInfo::MAX_HOTAREA_COUNT)) {
            auto sessionId = sceneSession->GetWindowId();
            TLOGE(WmsLogTag::WMS_EVENT, "id=%{public}d hotAreas size > %{public}d",
                sessionId, static_cast<int>(hotAreas.size()));
            break;
        }
    }
    if (touchHotAreas.empty()) {
        return UpdateDefaultHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    }
}

static void AddDialogSessionMapItem(const sptr<SceneSession>& session,
    std::map<int32_t, sptr<SceneSession>>& dialogMap)
{
    const auto& mainSession = session->GetMainSession();
    if (mainSession == nullptr) {
        return;
    }
    bool isTopmostModalWindow = false;
    const auto& property = session->GetSessionProperty();
    if (property != nullptr && property->IsTopmost()) {
        isTopmostModalWindow = true;
    }
    if (auto iter = dialogMap.find(mainSession->GetPersistentId());
        iter != dialogMap.end() && iter->second != nullptr) {
        auto& targetSession = iter->second;
        if (session->IsApplicationModal() == targetSession->IsApplicationModal()) {
            if (targetSession->GetSessionProperty() &&
                targetSession->GetSessionProperty()->IsTopmost() &&
                !isTopmostModalWindow) {
                return;
            }
            if (targetSession->GetZOrder() > session->GetZOrder()) {
                return;
            }
        } else if (!session->IsApplicationModal() && targetSession->IsApplicationModal()) {
            return;
        }
    }
    dialogMap[mainSession->GetPersistentId()] = session;
    TLOGD(WmsLogTag::WMS_DIALOG, "Add dialog session, id: %{public}d, mainSessionId: %{public}d",
        session->GetPersistentId(), mainSession->GetPersistentId());
}

static void UpdateCallingPidMapItem(const sptr<SceneSession>& session,
    std::unordered_map<int32_t, sptr<SceneSession>>& callingPidMap)
{
    auto sessionPid = session->GetCallingPid();
    auto parentSession = session->GetParentSession();
    while (parentSession) {
        auto parentSessionPid = parentSession->GetCallingPid();
        if (sessionPid != parentSessionPid) {
            callingPidMap[parentSessionPid] = session;
        }
        parentSession = parentSession->GetParentSession();
    }
}

static void AddCallingPidMapItem(const sptr<SceneSession>& session,
    std::unordered_map<int32_t, sptr<SceneSession>>& callingPidMap)
{
    auto sessionCallingPid = session->GetCallingPid();
    auto iter = callingPidMap.find(sessionCallingPid);
    if (iter == callingPidMap.end()) {
        if (!session->IsApplicationModal()) {
            return;
        }
        callingPidMap.emplace(std::make_pair(sessionCallingPid, session));
        UpdateCallingPidMapItem(session, callingPidMap);
        TLOGD(WmsLogTag::WMS_DIALOG,
            "Add callingPid session, sessionCallingPid: %{public}d, sessionId: %{public}d",
            sessionCallingPid, session->GetPersistentId());
    } else {
        if (iter->second->GetZOrder() < session->GetZOrder()) {
            callingPidMap[sessionCallingPid] = session;
            UpdateCallingPidMapItem(session, callingPidMap);
            TLOGD(WmsLogTag::WMS_DIALOG,
                "Update callingPid session, sessionCallingPid: %{public}d, sessionId: %{public}d",
                sessionCallingPid, session->GetPersistentId());
        }
    }
}

static void UpdateDialogSessionMap(
    const std::map<int32_t, sptr<SceneSession>>& sessionMap,
    const std::unordered_map<int32_t, sptr<SceneSession>>& callingPidMap,
    std::map<int32_t, sptr<SceneSession>>& dialogMap)
{
    for (const auto& [_, session] : sessionMap) {
        if (session == nullptr || session->GetForceHideState() != ForceHideState::NOT_HIDDEN) {
            continue;
        }
        auto iter = callingPidMap.find(session->GetCallingPid());
        if (iter != callingPidMap.end()) {
            dialogMap[session->GetPersistentId()] = iter->second;
            TLOGD(WmsLogTag::WMS_DIALOG,
                "Update dialog session, sessionId: %{public}d, callingPidSessionId: %{public}d",
                session->GetPersistentId(), iter->second->GetPersistentId());
        }
    }
}

std::map<int32_t, sptr<SceneSession>> SceneSessionDirtyManager::GetDialogSessionMap(
    const std::map<int32_t, sptr<SceneSession>>& sessionMap) const
{
    std::map<int32_t, sptr<SceneSession>> dialogMap;
    std::unordered_map<int32_t, sptr<SceneSession>> callingPidMap;
    bool hasModalApplication = false;
    for (const auto& [_, session] : sessionMap) {
        if (session == nullptr || session->GetForceHideState() != ForceHideState::NOT_HIDDEN) {
            continue;
        }
        if (!session->IsModal() && !session->IsDialogWindow()) {
            continue;
        }
        AddDialogSessionMapItem(session, dialogMap);
        AddCallingPidMapItem(session, callingPidMap);
        if (session->IsApplicationModal()) {
            hasModalApplication = true;
        }
    }
    if (hasModalApplication) {
        UpdateDialogSessionMap(sessionMap, callingPidMap, dialogMap);
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
    if (!SceneSessionManager::GetInstance().IsSessionVisible(sceneSession)) {
        return true;
    }
    return false;
}

void SceneSessionDirtyManager::NotifyWindowInfoChange(const sptr<SceneSession>& sceneSession,
    const WindowUpdateType& type, const bool startMoving)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is null");
        return;
    }

    if (type == WindowUpdateType::WINDOW_UPDATE_ADDED || type == WindowUpdateType::WINDOW_UPDATE_REMOVED||
        type == WindowUpdateType::WINDOW_UPDATE_ACTIVE) {
        TLOGD(WmsLogTag::WMS_EVENT, "[EventDispatch] wid=%{public}d, winType=%{public}d",
            sceneSession->GetWindowId(), static_cast<int>(type));
    }
    ResetFlushWindowInfoTask();
}

void SceneSessionDirtyManager::ResetFlushWindowInfoTask()
{
    sessionDirty_.store(true);
    bool hasPostTask = false;
    if (hasPostTask_.compare_exchange_strong(hasPostTask, true)) {
        auto task = [this]() {
            hasPostTask_.store(false);
            if (!sessionDirty_.load() || flushWindowInfoCallback_ == nullptr) {
                return;
            }
            flushWindowInfoCallback_();
        };
        TLOGD(WmsLogTag::WMS_EVENT, "in");
        SceneSessionManager::GetInstance().PostFlushWindowInfoTask(task,
            UPDATE_WINDOW_INFO_TASK, UPDATE_TASK_DURATION);
    }
}

bool SceneSessionDirtyManager::GetLastConstrainedModalUIExtInfo(const sptr<SceneSession>& sceneSession,
    SecSurfaceInfo& constrainedModalUIExtInfo)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return false;
    }
    auto surfaceNodeId = sceneSession->GetSurfaceNodeId();
    if (!surfaceNodeId) {
        TLOGD(WmsLogTag::WMS_EVENT, "surfaceNodeId not found");
        return false;
    }
    {
        std::shared_lock<std::shared_mutex> lock(constrainedModalUIExtInfoMutex_);
        auto iter = constrainedModalUIExtInfoMap_.find(*surfaceNodeId);
        if (iter == constrainedModalUIExtInfoMap_.end()) {
            return false;
        }
        if (!iter->second.empty()) {
            constrainedModalUIExtInfo = iter->second.back();
            return true;
        }
    }
    return false;
}

void SceneSessionDirtyManager::AddModalExtensionWindowInfo(std::vector<MMI::WindowInfo>& windowInfoList,
    MMI::WindowInfo windowInfo, const sptr<SceneSession>& sceneSession,
    const ExtensionWindowEventInfo& extensionInfo)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }

    windowInfo.id = extensionInfo.persistentId;
    if (extensionInfo.windowRect.width_ != 0 || extensionInfo.windowRect.height_ != 0) {
        auto singleHandData = GetSingleHandData(sceneSession);
        MMI::Rect windowRect = {
            .x = ceil(singleHandData.scaleX * extensionInfo.windowRect.posX_ +
                 singleHandData.singleHandX),
            .y = ceil(singleHandData.scaleY * extensionInfo.windowRect.posY_ +
                 singleHandData.singleHandY),
            .width = extensionInfo.windowRect.width_,
            .height = extensionInfo.windowRect.height_
        };
        windowInfo.area = windowRect;
        std::vector<MMI::Rect> touchHotAreas;
        MMI::Rect touchRect = {
            .x = 0,
            .y = 0,
            .width = extensionInfo.windowRect.width_,
            .height = extensionInfo.windowRect.height_
        };
        touchHotAreas.emplace_back(touchRect);
        windowInfo.defaultHotAreas = touchHotAreas;
        windowInfo.pointerHotAreas = std::move(touchHotAreas);
        Matrix3f transform;
        CalTransform(sceneSession, transform, singleHandData, true);
        std::vector<float> transformData(transform.GetData(), transform.GetData() + TRANSFORM_DATA_LEN);
        windowInfo.transform = std::move(transformData);
    }

    windowInfo.agentWindowId = extensionInfo.persistentId;
    windowInfo.pid = extensionInfo.pid;
    windowInfo.agentPid = extensionInfo.pid;
    std::vector<int32_t> pointerChangeAreas(POINTER_CHANGE_AREA_COUNT, 0);
    windowInfo.pointerChangeAreas = std::move(pointerChangeAreas);
    windowInfo.zOrder = windowInfo.zOrder + ZORDER_UIEXTENSION_INDEX;

    windowInfoList.emplace_back(windowInfo);
}

void SceneSessionDirtyManager::GetModalUIExtensionInfo(std::vector<MMI::WindowInfo>& windowInfoList,
    const sptr<SceneSession>& sceneSession, const MMI::WindowInfo& hostWindowInfo)
{
    auto modalUIExtensionEventInfo = sceneSession->GetLastModalUIExtensionEventInfo();
    if (!modalUIExtensionEventInfo) {
        return;
    }
    if (modalUIExtensionEventInfo->isConstrainedModal) {  // constrained UIExt
        SecSurfaceInfo constrainedModalUIExtInfo;
        if (!GetLastConstrainedModalUIExtInfo(sceneSession, constrainedModalUIExtInfo)) {
            TLOGE(WmsLogTag::WMS_EVENT, "cannot find last constrained Modal UIExtInfo");
            return;
        }
        MMI::WindowInfo windowInfo = GetSecComponentWindowInfo(constrainedModalUIExtInfo,
            hostWindowInfo, sceneSession, GetTransformFromWindowInfo(hostWindowInfo));
        std::vector<int32_t> pointerChangeAreas(POINTER_CHANGE_AREA_COUNT, 0);
        windowInfo.pointerChangeAreas = std::move(pointerChangeAreas);
        windowInfo.zOrder = hostWindowInfo.zOrder + ZORDER_UIEXTENSION_INDEX;
        windowInfo.privacyUIFlag = false;
        TLOGD(WmsLogTag::WMS_EVENT, "constrained Modal UIExt id: %{public}d", windowInfo.id);
        windowInfoList.emplace_back(windowInfo);
    } else {  // normal UIExt
        AddModalExtensionWindowInfo(windowInfoList, hostWindowInfo, sceneSession, *modalUIExtensionEventInfo);
    }
}

auto SceneSessionDirtyManager::GetFullWindowInfoList() ->
std::pair<std::vector<MMI::WindowInfo>, std::vector<std::shared_ptr<Media::PixelMap>>>
{
    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    const auto sceneSessionMap = SceneSessionManager::GetInstance().GetSceneSessionMap();
    // all input event should trans to dialog window if dialog exists
    const auto dialogMap = GetDialogSessionMap(sceneSessionMap);
    uint32_t maxHotAreasNum = 0;
    for (const auto& sceneSessionValuePair : sceneSessionMap) {
        const auto& sceneSessionValue = sceneSessionValuePair.second;
        if (sceneSessionValue == nullptr) {
            continue;
        }
        TLOGD(WmsLogTag::WMS_EVENT,
            "[EventDispatch] windowName=%{public}s bundleName=%{public}s"
            " windowId=%{public}d activeStatus=%{public}d", sceneSessionValue->GetWindowName().c_str(),
            sceneSessionValue->GetSessionInfo().bundleName_.c_str(), sceneSessionValue->GetWindowId(),
            sceneSessionValue->GetForegroundInteractiveStatus());
        auto [windowInfo, pixelMap] = GetWindowInfo(sceneSessionValue, WindowAction::WINDOW_ADD);
        auto iter = (sceneSessionValue->GetMainSessionId() == INVALID_SESSION_ID) ?
            dialogMap.find(sceneSessionValue->GetPersistentId()) :
            dialogMap.find(sceneSessionValue->GetMainSessionId());
        if (iter != dialogMap.end() && iter->second != nullptr &&
            sceneSessionValue->GetPersistentId() != iter->second->GetPersistentId() &&
            iter->second->GetZOrder() > sceneSessionValue->GetZOrder()) {
            windowInfo.agentWindowId = static_cast<int32_t>(iter->second->GetPersistentId());
            windowInfo.pid = static_cast<int32_t>(iter->second->GetCallingPid());
            windowInfo.agentPid = static_cast<int32_t>(iter->second->GetCallingPid());
        } else {
            GetModalUIExtensionInfo(windowInfoList, sceneSessionValue, windowInfo);
        }
        TLOGD(WmsLogTag::WMS_EVENT, "windowId=%{public}d, agentWindowId=%{public}d, zOrder=%{public}f",
            windowInfo.id, windowInfo.agentWindowId, windowInfo.zOrder);
        windowInfoList.emplace_back(windowInfo);
        pixelMapList.emplace_back(pixelMap);
        // set the number of hot areas to the maximum number of hot areas when it exceeds the maximum number
        // to avoid exceeding socket buff limits
        if (windowInfo.defaultHotAreas.size() > maxHotAreasNum) {
            maxHotAreasNum = windowInfo.defaultHotAreas.size();
        }
    }
    if (maxHotAreasNum > MMI::WindowInfo::DEFAULT_HOTAREA_COUNT) {
        std::sort(windowInfoList.begin(), windowInfoList.end(), CmpMMIWindowInfo);
    }
    return {windowInfoList, pixelMapList};
}

void SceneSessionDirtyManager::UpdatePointerAreas(sptr<SceneSession> sceneSession,
    std::vector<int32_t>& pointerChangeAreas) const
{
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is null");
        return;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_EVENT, "sessionProperty is null");
        return;
    }
    bool isDragAccessible = sceneSession->IsDragAccessible();
    TLOGD(WmsLogTag::WMS_EVENT, "window %{public}s isDragAccessible: %{public}d", sceneSession->GetWindowName().c_str(),
        isDragAccessible);
    if (isDragAccessible) {
        float vpr = 1.5f; // 1.5: default vp
        auto displayId = sessionProperty->GetDisplayId();
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
        if (screenSession != nullptr) {
            vpr = screenSession->GetScreenProperty().GetDensity();
        }
        int32_t pointerAreaFivePx = static_cast<int32_t>(POINTER_CHANGE_AREA_FIVE * vpr);
        int32_t pointerAreaSixteenPx = static_cast<int32_t>(POINTER_CHANGE_AREA_SIXTEEN * vpr);
        if (sceneSession->GetSessionInfo().isSetPointerAreas_) {
            pointerChangeAreas = {POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
                POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx, pointerAreaSixteenPx,
                pointerAreaFivePx, pointerAreaSixteenPx, pointerAreaFivePx};
            return;
        }
        auto limits = sessionProperty->GetWindowLimits();
        TLOGD(WmsLogTag::WMS_EVENT, "%{public}s [minWidth,maxWidth,minHeight,maxHeight]: %{public}d,"
            " %{public}d, %{public}d, %{public}d", sceneSession->GetWindowName().c_str(), limits.minWidth_,
            limits.maxWidth_, limits.minHeight_, limits.maxHeight_);
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
        TLOGD(WmsLogTag::WMS_EVENT, "sceneSession is: %{public}d dragAccessible is false",
            sceneSession->GetPersistentId());
    }
}

void SceneSessionDirtyManager::UpdatePrivacyMode(const sptr<SceneSession>& sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return;
    }
    windowInfo.privacyMode = MMI::SecureFlag::DEFAULT_MODE;
    sptr<WindowSessionProperty> windowSessionProperty = sceneSession->GetSessionProperty();
    if (windowSessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowSessionProperty is nullptr");
        return;
    }
    if (windowSessionProperty->GetPrivacyMode() || windowSessionProperty->GetSystemPrivacyMode() ||
        sceneSession->GetCombinedExtWindowFlags().privacyModeFlag) {
        windowInfo.privacyMode = MMI::SecureFlag::PRIVACY_MODE;
    }
}

void SceneSessionDirtyManager::UpdateWindowFlags(DisplayId displayId, const sptr<SceneSession>& sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    windowInfo.flags = 0;
    bool isTouchable = sceneSession->GetWindowTouchableForMMI(displayId);
    if (!isTouchable) {
        windowInfo.flags |= MMI::WindowInfo::FLAG_BIT_UNTOUCHABLE;
    }
}

void SceneSessionDirtyManager::UpdateWindowFlagsForReceiveDragEventEnabled(const sptr<SceneSession>& sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is null");
        return;
    }
    if (sceneSession->GetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_RECEIVE_DRAG_EVENT) ||
        !sceneSession->GetSessionInfoReceiveDragEventEnabled()) {
        windowInfo.flags |= MMI::WindowInputPolicy::FLAG_DRAG_DISABLED;
    }
}

void SceneSessionDirtyManager::UpdateWindowFlagsForWindowSeparation(const sptr<SceneSession>& sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is null");
        return;
    }
    if (sceneSession->GetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_WINDOW_SEPARATION_TOUCH_ENABLED) ||
        !sceneSession->GetSessionInfoSeparationTouchEnabled()) {
        windowInfo.flags |= MMI::WindowInputPolicy::FLAG_FIRST_TOUCH_HIT;
    }
}

void SceneSessionDirtyManager::UpdateWindowFlagsForLockCursor(const sptr<SceneSession>& sceneSession,
    MMI::WindowInfo& windowInfo) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is null");
        return;
    }
    auto lastLockCursor = sceneSession->GetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR);
    if (!sceneSession->IsFocused()) {
        if (lastLockCursor) {
            sceneSession->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, false);
            TLOGW(WmsLogTag::WMS_EVENT, "LockCursor:Inconsistent focus ID:%{public}d", sceneSession->GetWindowId());
        }
        return;
    }
    if (!sceneSession->GetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR)) {
        return;
    }
    if (sceneSession->IsDragMoving() || sceneSession->IsDragZooming()) {
        sceneSession->SetSessionInfoCursorDragFlag(true);
        sceneSession->SetSessionInfoCursorDragCount(0);
        TLOGI(WmsLogTag::WMS_EVENT, "in moving or drag WId:%{public}d", sceneSession->GetWindowId());
        return;
    }
    if (sceneSession->GetSessionInfoCursorDragFlag()) {
        int32_t count = sceneSession->GetSessionInfoCursorDragCount();
        sceneSession->SetSessionInfoCursorDragCount(++count);
        windowInfo.agentPid = getpid();
        if (count > CURSOR_DRAG_COUNT_MAX) {
            sceneSession->SetSessionInfoCursorDragFlag(false);
            sceneSession->SetSessionInfoCursorDragCount(0);
        }
        TLOGI(WmsLogTag::WMS_EVENT, "cursorDragFlag_ delay 1 time, WId:%{public}d", sceneSession->GetWindowId());
        return;
    }
    if (sceneSession->GetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_CURSOR_FOLLOW_MOVEMENT)) {
        windowInfo.flags |= MMI_FLAG_BIT_LOCK_CURSOR_FOLLOW_MOVEMENT;
    } else {
        windowInfo.flags |= MMI_FLAG_BIT_LOCK_CURSOR_NOT_FOLLOW_MOVEMENT;
    }
}

std::pair<MMI::WindowInfo, std::shared_ptr<Media::PixelMap>> SceneSessionDirtyManager::GetWindowInfo(
    const sptr<SceneSession>& sceneSession, const WindowAction& action) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return {};
    }
    sptr<WindowSessionProperty> windowSessionProperty = sceneSession->GetSessionProperty();
    if (windowSessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "GetSessionProperty is nullptr");
        return {};
    }
    Matrix3f transform;
    WSRect windowRect = sceneSession->GetSessionGlobalRectInMultiScreen();
    auto pid = sceneSession->GetCallingPid();
    auto uid = sceneSession->GetCallingUid();
    auto windowId = sceneSession->GetWindowId();
    auto displayId = windowSessionProperty->GetDisplayId();
    const auto& singleHandData = GetSingleHandData(sceneSession);
    CalTransform(sceneSession, transform, singleHandData);
    std::vector<float> transformData(transform.GetData(), transform.GetData() + TRANSFORM_DATA_LEN);

    auto agentWindowId = sceneSession->GetWindowId();
    auto zOrder = sceneSession->GetZOrder();
    std::vector<int32_t> pointerChangeAreas(POINTER_CHANGE_AREA_COUNT, 0);
    WindowType windowType = windowSessionProperty->GetWindowType();
    CheckIfUpdatePointAreas(windowType, sceneSession, windowSessionProperty, pointerChangeAreas);
    std::vector<MMI::Rect> touchHotAreas;
    std::vector<MMI::Rect> pointerHotAreas;
    UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    int windowNameType = WINDOW_NAME_TYPE_UNKNOWN;
    std::string windowName = sceneSession->GetWindowNameAllType();
    auto startsWith = [](const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() &&
            std::equal(prefix.begin(), prefix.end(), str.begin());
    };
    if (startsWith(windowName, SCREENSHOT_WINDOW_NAME_PREFIX) || startsWith(windowName, PREVIEW_WINDOW_NAME_PREFIX)) {
        windowNameType = WINDOW_NAME_TYPE_THUMBNAIL;
    } else if (startsWith(windowName, VOICEINPUT_WINDOW_NAME_PREFIX)) {
        windowNameType = WINDOW_NAME_TYPE_VOICEINPUT;
    }
    auto pixelMap = windowSessionProperty->GetWindowMask();
    MMI::WindowInfo windowInfo = {
        .id = windowId,
        .pid = pid,
        .uid = uid,
        .area = { ceil(singleHandData.scaleX * windowRect.posX_ + singleHandData.singleHandX),
                  ceil(singleHandData.scaleX * windowRect.posY_ + singleHandData.singleHandY),
                  windowRect.width_, windowRect.height_ },
        .defaultHotAreas = std::move(touchHotAreas),
        .pointerHotAreas = std::move(pointerHotAreas),
        .agentWindowId = agentWindowId,
        .action = static_cast<MMI::WINDOW_UPDATE_ACTION>(action),
        .displayId = displayId,
        .groupId = SceneSessionManager::GetInstance().GetDisplayGroupId(displayId),
        .zOrder = zOrder,
        .pointerChangeAreas = std::move(pointerChangeAreas),
        .transform = transformData,
        .pixelMap = pixelMap.get(),
        .windowInputType = static_cast<MMI::WindowInputType>(sceneSession->GetSessionInfo().windowInputType_),
        .windowType = static_cast<int32_t>(windowType),
        .isSkipSelfWhenShowOnVirtualScreen = sceneSession->GetSessionProperty()->GetSkipEventOnCastPlus(),
        .windowNameType = windowNameType,
        .agentPid = sceneSession->IsStartMoving() ? static_cast<int32_t>(getpid()) : pid,
    };
    UpdateWindowFlags(displayId, sceneSession, windowInfo);
    if (windowSessionProperty->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING)) {
        windowInfo.flags |= MMI::WindowInfo::FLAG_BIT_HANDWRITING;
    }
    auto expandInputFlag = sceneSession->GetSessionInfoExpandInputFlag();
    if (expandInputFlag & static_cast<uint32_t>(ExpandInputFlag::WINDOW_DISABLE_USER_ACTION)) {
        windowInfo.flags |= MMI::WindowInfo::FLAG_BIT_DISABLE_USER_ACTION;
    }
    UpdateWindowFlagsForReceiveDragEventEnabled(sceneSession, windowInfo);
    UpdateWindowFlagsForWindowSeparation(sceneSession, windowInfo);
    UpdateWindowFlagsForLockCursor(sceneSession, windowInfo);
    UpdatePrivacyMode(sceneSession, windowInfo);
    windowInfo.uiExtentionWindowInfo = GetSecSurfaceWindowinfoList(sceneSession, windowInfo, transform);
    return {windowInfo, pixelMap};
}

SingleHandData SceneSessionDirtyManager::GetSingleHandData(const sptr<SceneSession>& sceneSession) const
{
    SingleHandData singleHandData;
    auto sessionProperty = sceneSession->GetSessionProperty();
    auto displayId = sessionProperty->GetDisplayId();
    if (displayId != ScreenSessionManagerClient::GetInstance().GetDefaultScreenId() ||
        !sceneSession->SessionIsSingleHandMode()) {
        return singleHandData;
    }
    const std::map<ScreenId, ScreenProperty>& screensProperties =
        ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    const SingleHandTransform& transform = sceneSession->GetSingleHandTransform();
    const SingleHandScreenInfo& singleHandScreenInfo = SceneSessionManager::GetInstance().GetSingleHandScreenInfo();
    singleHandData.scaleX = transform.scaleX;
    singleHandData.scaleY = transform.scaleY;
    singleHandData.singleHandX = transform.posX;
    singleHandData.singleHandY = transform.posY;
    singleHandData.pivotX = singleHandScreenInfo.scalePivotX;
    singleHandData.pivotY = singleHandScreenInfo.scalePivotY;
    singleHandData.mode = singleHandScreenInfo.mode;
    return singleHandData;
}

void SceneSessionDirtyManager::CheckIfUpdatePointAreas(WindowType windowType, const sptr<SceneSession>& sceneSession,
    const sptr<WindowSessionProperty>& windowSessionProperty, std::vector<int32_t>& pointerChangeAreas) const
{
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isDecorEnabledDialog = WindowHelper::IsDialogWindow(windowType) && windowSessionProperty->IsDecorEnable();
    bool isDragAccessibleSystemWindowButNotDialog = WindowHelper::IsSystemWindow(windowType) &&
        !WindowHelper::IsDialogWindow(windowType) && sceneSession->IsDragAccessible();
    bool isDecorEnabledSubWindow = WindowHelper::IsSubWindow(windowType) && windowSessionProperty->IsDecorEnable();
    bool isDragAccessibleSubWindow = WindowHelper::IsSubWindow(windowType) && sceneSession->IsDragAccessible();
    bool isUpdatePointerAreasNeeded = isMainWindow || isDecorEnabledDialog || isDecorEnabledSubWindow ||
        isDragAccessibleSubWindow || isDragAccessibleSystemWindowButNotDialog;
    auto windowMode = windowSessionProperty->GetWindowMode();
    auto maximizeMode = windowSessionProperty->GetMaximizeMode();
    if ((windowMode == WindowMode::WINDOW_MODE_FLOATING && maximizeMode != MaximizeMode::MODE_AVOID_SYSTEM_BAR &&
         isUpdatePointerAreasNeeded) ||
        sceneSession->GetSessionInfo().isSetPointerAreas_) {
        UpdatePointerAreas(sceneSession, pointerChangeAreas);
    }
}

void SceneSessionDirtyManager::RegisterFlushWindowInfoCallback(FlushWindowInfoCallback&& callback)
{
    flushWindowInfoCallback_ = std::move(callback);
}

void SceneSessionDirtyManager::ResetSessionDirty()
{
    sessionDirty_.store(false);
}

std::string DumpRect(const std::vector<MMI::Rect>& rects)
{
    std::string rectStr = "hot:";
    for (const auto& rect : rects) {
        rectStr = rectStr + std::to_string(rect.x) +"," + std::to_string(rect.y) +
        "," + std::to_string(rect.width) + "," + std::to_string(rect.height) + "|";
    }
    return rectStr;
}

std::string DumpWindowInfo(const MMI::WindowInfo& info)
{
    std::string infoStr = "wInfo:";
    infoStr = infoStr + std::to_string(info.id) + "|" + std::to_string(info.pid) +
        "|" + std::to_string(info.agentPid) + "|" + std::to_string(info.uid) + "|" + std::to_string(info.area.x) +
        "," + std::to_string(info.area.y) + "," + std::to_string(info.area.width) + "," +
        std::to_string(info.area.height) + "|" + std::to_string(info.agentWindowId) + "|" +
        std::to_string(info.flags) + "|" + std::to_string(info.displayId) +
        "|" + std::to_string(static_cast<int>(info.action)) + "|" + std::to_string(info.zOrder) + ",";
    return infoStr + DumpRect(info.defaultHotAreas);
}

std::string DumpSecRectInfo(const SecRectInfo & secRectInfo)
{
    std::string infoStr = "[" + std::to_string(secRectInfo.relativeCoords.GetLeft()) + "," +
        std::to_string(secRectInfo.relativeCoords.GetTop()) +  "," +
        std::to_string(secRectInfo.relativeCoords.GetWidth()) + "," +
        std::to_string(secRectInfo.relativeCoords.GetHeight()) + "]" +
        "|" + std::to_string(static_cast<int>(secRectInfo.scale[0])) +
        "|" + std::to_string(static_cast<int>(secRectInfo.scale[1])) +
        "|" + std::to_string(static_cast<int>(secRectInfo.anchor[0])) +
        "|" + std::to_string(static_cast<int>(secRectInfo.anchor[1]));
    return infoStr;
}

std::string DumpSecSurfaceInfo(const SecSurfaceInfo& secSurfaceInfo)
{
    std::string infoStr = std::to_string(secSurfaceInfo.hostPid) +
        "|" + std::to_string(secSurfaceInfo.uiExtensionPid) +
        "|" + std::to_string(secSurfaceInfo.hostNodeId) +
        "|" + std::to_string(secSurfaceInfo.uiExtensionNodeId);
    return infoStr;
}

MMI::WindowInfo SceneSessionDirtyManager::MakeWindowInfoFormHostWindow(const MMI::WindowInfo& hostWindowinfo) const
{
    MMI::WindowInfo windowinfo;
    windowinfo.id = hostWindowinfo.id;
    windowinfo.pid = hostWindowinfo.pid;
    windowinfo.agentPid = hostWindowinfo.agentPid;
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

Matrix3f CoordinateSystemHostWindowToScreen(const Matrix3f hostTransform, const SecRectInfo& secRectInfo)
{
    Matrix3f transform = Matrix3f::IDENTITY;
    Vector2f translate(secRectInfo.relativeCoords.GetLeft(), secRectInfo.relativeCoords.GetTop());
    transform = transform.Translate(translate);
    Vector2f scale(secRectInfo.scale[0], secRectInfo.scale[1]);
    transform = transform.Scale(scale, secRectInfo.anchor[0], secRectInfo.anchor[1]);
    transform = hostTransform.Inverse() * transform;
    return transform;
}

MMI::Rect CalRectInScreen(const Matrix3f& transform, const SecRectInfo& secRectInfo)
{
    auto topLeft = transform * Vector3f(0, 0, 1.0);
    auto bottomRight = transform * Vector3f(secRectInfo.relativeCoords.GetWidth(),
        secRectInfo.relativeCoords.GetHeight(), 1.0);
    auto left = std::min(topLeft[0], bottomRight[0]);
    auto top = std::min(topLeft[1], bottomRight[1]);
    auto topLeftX = static_cast<int32_t>(topLeft[0]);
    auto topLeftY = static_cast<int32_t>(topLeft[1]);
    auto bottomRightX = static_cast<int32_t>(bottomRight[0]);
    auto bottomRightY = static_cast<int32_t>(bottomRight[1]);
    if ((topLeftX > 0 && bottomRightX < INT32_MIN + topLeftX) ||
        (topLeftX < 0 && bottomRightX > INT32_MAX + topLeftX)) {
        TLOGE(WmsLogTag::WMS_EVENT, "data overflows topLeftX:%{public}d bottomRightX:%{public}d",
            topLeftX, bottomRightX);
    }
    if ((topLeftY > 0 && bottomRightY < INT32_MIN + topLeftY) ||
        (topLeftY < 0 && bottomRightY > INT32_MAX + topLeftY)) {
        TLOGE(WmsLogTag::WMS_EVENT, "data overflows topLeftY:%{public}d bottomRightY:%{public}d",
            topLeftY, bottomRightY);
    }
    auto width = std::abs(topLeftX - bottomRightX);
    auto height = std::abs(topLeftY - bottomRightY);
    return MMI::Rect{ left, top, width, height};
}


MMI::WindowInfo SceneSessionDirtyManager::GetHostComponentWindowInfo(const SecSurfaceInfo& secSurfaceInfo,
    const MMI::WindowInfo& hostWindowinfo, const sptr<SceneSession>& sceneSession, const Matrix3f hostTransform) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return {};
    }
    MMI::WindowInfo windowinfo;
    const auto& secRectInfoList = secSurfaceInfo.upperNodes;
    if (secRectInfoList.size() > 0) {
        windowinfo = MakeWindowInfoFormHostWindow(hostWindowinfo);
    }
    for (const auto& secRectInfo : secRectInfoList) {
        windowinfo.pid = secSurfaceInfo.hostPid;
        windowinfo.agentPid = sceneSession->IsStartMoving() ? static_cast<int32_t>(getpid()) : windowinfo.pid;
        MMI::Rect hotArea = { secRectInfo.relativeCoords.GetLeft(), secRectInfo.relativeCoords.GetTop(),
            secRectInfo.relativeCoords.GetWidth(), secRectInfo.relativeCoords.GetHeight() };
        windowinfo.defaultHotAreas.emplace_back(hotArea);
        windowinfo.pointerHotAreas.emplace_back(hotArea);
    }
    return windowinfo;
}

MMI::WindowInfo SceneSessionDirtyManager::GetSecComponentWindowInfo(const SecSurfaceInfo& secSurfaceInfo,
    const MMI::WindowInfo& hostWindowinfo, const sptr<SceneSession>& sceneSession, const Matrix3f hostTransform) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return {};
    }
    const auto& secRectInfo = secSurfaceInfo.uiExtensionRectInfo;
    MMI::WindowInfo windowinfo = MakeWindowInfoFormHostWindow(hostWindowinfo);
    windowinfo.id = sceneSession->GetUIExtPersistentIdBySurfaceNodeId(secSurfaceInfo.uiExtensionNodeId);
    if (windowinfo.id == 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "GetUIExtPersistentId ERROR");
        return {};
    }
    windowinfo.agentWindowId = windowinfo.id;
    windowinfo.pid = secSurfaceInfo.uiExtensionPid;
    windowinfo.agentPid = secSurfaceInfo.uiExtensionPid;
    windowinfo.privacyUIFlag = true;
    auto transform = CoordinateSystemHostWindowToScreen(hostTransform, secRectInfo);
    windowinfo.area = CalRectInScreen(transform, secRectInfo);
    MMI::Rect hotArea = { 0, 0, secRectInfo.relativeCoords.GetWidth(), secRectInfo.relativeCoords.GetHeight() };
    windowinfo.defaultHotAreas.emplace_back(hotArea);
    windowinfo.pointerHotAreas.emplace_back(hotArea);
    // 屏幕坐标系到控件坐标系转换
    transform = transform.Inverse();
    std::vector<float> transformData(transform.GetData(), transform.GetData() + TRANSFORM_DATA_LEN);
    windowinfo.transform = transformData;
    return windowinfo;
}

bool operator==(const SecRectInfo& a, const SecRectInfo& b)
{
    return (a.relativeCoords == b.relativeCoords && a.scale == b.scale && a.anchor == b.anchor);
}

bool operator!=(const SecRectInfo& a, const SecRectInfo& b)
{
    return !(a == b);
}

bool operator==(const SecSurfaceInfo& a, const SecSurfaceInfo& b)
{
    return (a.uiExtensionRectInfo == b.uiExtensionRectInfo && a.hostPid == b.hostPid &&
        a.uiExtensionNodeId == b.uiExtensionNodeId && a.uiExtensionPid == b.uiExtensionPid &&
        a.hostNodeId == b.hostNodeId && a.upperNodes == b.upperNodes);
}

static void DumpUpperNodes(std::ostringstream& dumpSecSurfaceStream, const std::vector<SecRectInfo>& upperNodes)
{
    if (upperNodes.size() == 0) {
        return;
    }

    std::unordered_map<std::string, uint32_t> secRectInfoMap;
    std::vector<std::string> secRectInfoList;
    for (const auto& secRectInfo : upperNodes) {
        auto infoStr = DumpSecRectInfo(secRectInfo);
        if (secRectInfoMap.find(infoStr) != secRectInfoMap.end()) {
            secRectInfoMap[infoStr] += 1;
            continue;
        }
        secRectInfoMap.insert(std::pair<std::string, uint32_t>(infoStr, 1));
        secRectInfoList.push_back(infoStr);
    }
    for (auto& info : secRectInfoList) {
        auto it = secRectInfoMap.find(info);
        if (it == secRectInfoMap.end()) {
            continue;
        }

        dumpSecSurfaceStream << "," << info;
        if (it->second != 1) {
            dumpSecSurfaceStream << "|" << it->second;
        }
    }
}

void DumpSecSurfaceInfoMap(const std::map<uint64_t, std::vector<SecSurfaceInfo>>& secSurfaceInfoMap)
{
    std::ostringstream dumpSecSurfaceStream;
    dumpSecSurfaceStream << "DumpSecSurface map:" << secSurfaceInfoMap.size();
    bool isStart = false;
    for (auto& e : secSurfaceInfoMap) {
        if (isStart) {
            dumpSecSurfaceStream << ";{";
        } else {
            dumpSecSurfaceStream << "{";
        }
        auto hostNodeId = e.first;
        dumpSecSurfaceStream << "id:" << hostNodeId << ";list:" << e.second.size();
        for (const auto& secSurfaceInfo : e.second) {
            auto surfaceInfoStr = DumpSecSurfaceInfo(secSurfaceInfo);
            auto rectInfoStr = DumpSecRectInfo(secSurfaceInfo.uiExtensionRectInfo);
            dumpSecSurfaceStream << ";{" << surfaceInfoStr << ";" << rectInfoStr;
            DumpUpperNodes(dumpSecSurfaceStream, secSurfaceInfo.upperNodes);
            dumpSecSurfaceStream << "}";
        }
        isStart = true;
        dumpSecSurfaceStream << "}";
    }
    TLOGND(WmsLogTag::WMS_EVENT, "DumpSecSurface desc:map size{id;list;{secSurfaceInfo:"
        "hostPid|uiExtensionPid|hostNodeId|uiExtensionNodeId;secRectInfo:"
        "[x,y,width,height]|scalex|scaley|anchorx|anchory|[repet num];[upperNodes same as secRectInfo]}}");
    TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s", dumpSecSurfaceStream.str().c_str());
}

void SceneSessionDirtyManager::UpdateSecSurfaceInfo(const std::map<uint64_t,
    std::vector<SecSurfaceInfo>>& secSurfaceInfoMap)
{
    bool updateSecSurfaceInfoNeeded = false;
    {
        std::unique_lock<std::shared_mutex> lock(secSurfaceInfoMutex_);
        if (secSurfaceInfoMap_ != secSurfaceInfoMap) {
            secSurfaceInfoMap_ = secSurfaceInfoMap;
            updateSecSurfaceInfoNeeded = true;
        }
    }
    if (updateSecSurfaceInfoNeeded) {
        ResetFlushWindowInfoTask();
        DumpSecSurfaceInfoMap(secSurfaceInfoMap);
    }
}

void SceneSessionDirtyManager::UpdateConstrainedModalUIExtInfo(const std::map<uint64_t,
    std::vector<SecSurfaceInfo>>& constrainedModalUIExtInfoMap)
{
    bool updateConstrainedModalUIExtInfoNeeded = false;
    {
        std::unique_lock<std::shared_mutex> lock(constrainedModalUIExtInfoMutex_);
        if (constrainedModalUIExtInfoMap_ != constrainedModalUIExtInfoMap) {
            constrainedModalUIExtInfoMap_ = constrainedModalUIExtInfoMap;
            updateConstrainedModalUIExtInfoNeeded = true;
        }
    }
    if (updateConstrainedModalUIExtInfoNeeded) {
        ResetFlushWindowInfoTask();
        DumpSecSurfaceInfoMap(constrainedModalUIExtInfoMap);
    }
}

std::vector<MMI::WindowInfo> SceneSessionDirtyManager::GetSecSurfaceWindowinfoList(
    const sptr<SceneSession>& sceneSession, const MMI::WindowInfo& hostWindowinfo, const Matrix3f& hostTransform) const
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr");
        return {};
    }
    auto surfaceNodeId = sceneSession->GetSurfaceNodeId();
    if (!surfaceNodeId) {
        TLOGD(WmsLogTag::WMS_EVENT, "surfaceNodeId not found");
        return {};
    }
    std::vector<SecSurfaceInfo> secSurfaceInfoList;
    {
        std::shared_lock<std::shared_mutex> lock(secSurfaceInfoMutex_);
        auto iter = secSurfaceInfoMap_.find(*surfaceNodeId);
        if (iter == secSurfaceInfoMap_.end()) {
            return {};
        }
        secSurfaceInfoList = iter->second;
    }
    std::vector<MMI::WindowInfo> windowinfoList;
    int seczOrder = 0;
    MMI::WindowInfo windowinfo;
    for (const auto& secSurfaceInfo : secSurfaceInfoList) {
        windowinfo = GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, hostTransform);
        windowinfo.zOrder = seczOrder++;
        windowinfoList.emplace_back(windowinfo);
        windowinfo = GetHostComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, hostTransform);
        windowinfo.zOrder = seczOrder++;
        windowinfoList.emplace_back(windowinfo);
    }
    return windowinfoList;
}
} //namespace OHOS::Rosen
