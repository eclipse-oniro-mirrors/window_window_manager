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

#include <hitrace_meter.h>
#include <hisysevent.h>
#include "screen_session_manager/include/screen_session_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "fold_screen_controller/super_fold_sensor_manager.h"
#include "window_manager_hilog.h"
#include "fold_screen_state_internel.h"
#include "fold_screen_controller/super_fold_state_manager.h"

namespace OHOS {

namespace Rosen {

WM_IMPLEMENT_SINGLE_INSTANCE(SuperFoldStateManager)

namespace {
const std::string g_FoldScreenRect = system::GetParameter("const.window.foldscreen.config_rect", "");
constexpr uint32_t DEFAULT_FOLD_REGION_HEIGHT = 82; // default height of pivot area
const int32_t PARAM_NUMBER_MIN = 10;
const int32_t HEIGHT_HALF = 2;
#ifdef TP_FEATURE_ENABLE
const int32_t TP_TYPE = 12;
const char* KEYBOARD_ON_CONFIG = "version:3+main";
const char* KEYBOARD_OFF_CONFIG = "version:3+whole";
#endif
static bool isKeyboardOn_ = false;
static bool isSystemKeyboardOn_ = false;
constexpr int32_t FOLD_CREASE_RECT_SIZE = 4; //numbers of parameter on the current device is 4
const std::string g_LiveCreaseRegion = system::GetParameter("const.display.foldscreen.crease_region", "");
const std::string FOLD_CREASE_DELIMITER = ",;";
constexpr ScreenId SCREEN_ID_FULL = 0;
const int32_t SCREEN_WIDTH_INDEX = 2;
const int32_t CREASE_WIDTH_INDEX = 3;
constexpr OHOS::Rect FULL_SCREEN_RECORD_RECT = {0, 0, 0, 0};
constexpr OHOS::Rect HALF_FOLD_B_SCREEN_RECORD_RECT = {0, 0, DISPLAY_A_WIDTH, DISPLAY_B_HEIGHT};
}

void SuperFoldStateManager::DoAngleChangeFolded(SuperFoldStatusChangeEvents event)
{
    TLOGI(WmsLogTag::DMS, "enter %{public}d", event);
}

void SuperFoldStateManager::DoAngleChangeHalfFolded(SuperFoldStatusChangeEvents event)
{
    TLOGI(WmsLogTag::DMS, "enter %{public}d", event);
    ScreenSessionManager::GetInstance().RecoveryResolutionEffect();
}

void SuperFoldStateManager::DoAngleChangeExpanded(SuperFoldStatusChangeEvents event)
{
    TLOGI(WmsLogTag::DMS, "enter %{public}d", event);
}

void SuperFoldStateManager::DoKeyboardOn(SuperFoldStatusChangeEvents event)
{
    TLOGI(WmsLogTag::DMS, "enter %{public}d", event);
    isKeyboardOn_ = true;
    if (!ChangeScreenState(true)) {
        TLOGI(WmsLogTag::DMS, "change to half screen fail!");
    }
}

void SuperFoldStateManager::DoKeyboardOff(SuperFoldStatusChangeEvents event)
{
    TLOGI(WmsLogTag::DMS, "enter %{public}d", event);
    isKeyboardOn_ = false;
    if (!ChangeScreenState(false)) {
        TLOGI(WmsLogTag::DMS, "recover from half screen fail!");
    }
}

void SuperFoldStateManager::DoFoldedToHalfFolded(SuperFoldStatusChangeEvents event)
{
    TLOGI(WmsLogTag::DMS, "enter %{public}d", event);
}

void SuperFoldStateManager::InitSuperFoldStateManagerMap()
{
    AddStateManagerMap(SuperFoldStatus::HALF_FOLDED,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED,
        SuperFoldStatus::EXPANDED,
        &SuperFoldStateManager::DoAngleChangeExpanded);
    
    AddStateManagerMap(SuperFoldStatus::FOLDED,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED,
        SuperFoldStatus::HALF_FOLDED,
        &SuperFoldStateManager::DoFoldedToHalfFolded);

    AddStateManagerMap(SuperFoldStatus::FOLDED,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED,
        SuperFoldStatus::EXPANDED,
        &SuperFoldStateManager::DoAngleChangeExpanded);

    AddStateManagerMap(SuperFoldStatus::EXPANDED,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED,
        SuperFoldStatus::HALF_FOLDED,
        &SuperFoldStateManager::DoAngleChangeHalfFolded);

    AddStateManagerMap(SuperFoldStatus::HALF_FOLDED,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_FOLDED,
        SuperFoldStatus::FOLDED,
        &SuperFoldStateManager::DoAngleChangeFolded);

    AddStateManagerMap(SuperFoldStatus::HALF_FOLDED,
        SuperFoldStatusChangeEvents::KEYBOARD_ON,
        SuperFoldStatus::KEYBOARD,
        &SuperFoldStateManager::DoKeyboardOn);

    AddStateManagerMap(SuperFoldStatus::KEYBOARD,
        SuperFoldStatusChangeEvents::KEYBOARD_OFF,
        SuperFoldStatus::HALF_FOLDED,
        &SuperFoldStateManager::DoKeyboardOff);

    // 开机状态自检
    AddStateManagerMap(SuperFoldStatus::UNKNOWN,
        SuperFoldStatusChangeEvents::KEYBOARD_ON,
        SuperFoldStatus::KEYBOARD,
        [&](SuperFoldStatusChangeEvents events) {});

    AddStateManagerMap(SuperFoldStatus::UNKNOWN,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED,
        SuperFoldStatus::HALF_FOLDED,
        [&](SuperFoldStatusChangeEvents events) {});

    AddStateManagerMap(SuperFoldStatus::UNKNOWN,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED,
        SuperFoldStatus::EXPANDED,
        [&](SuperFoldStatusChangeEvents events) {});

    AddStateManagerMap(SuperFoldStatus::UNKNOWN,
        SuperFoldStatusChangeEvents::ANGLE_CHANGE_FOLDED,
        SuperFoldStatus::FOLDED,
        [&](SuperFoldStatusChangeEvents events) {});
}

bool SuperFoldStateManager::isParamsValid(std::vector<std::string>& params)
{
    for (auto &param : params) {
        if (param.size() == 0) {
            return false;
        }
        for (int32_t i = 0; i < static_cast<int32_t>(param.size()); ++i) {
            if (param.at(i) < '0' || param.at(i) > '9') {
                return false;
            }
        }
    }
    return true;
}

void SuperFoldStateManager::InitSuperFoldCreaseRegionParams()
{
    std::vector<std::string> foldRect = FoldScreenStateInternel::StringSplit(g_FoldScreenRect, ',');

    if (foldRect.empty() || foldRect.size() <= PARAM_NUMBER_MIN) {
        // ccm numbers of parameter > 10
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return;
    }

    if (!isParamsValid(foldRect)) {
        TLOGE(WmsLogTag::DMS, "params is invalid");
        return;
    }

    ScreenId screenIdFull = 0;
    int32_t superFoldCreaseRegionPosX = 0;
    int32_t superFoldCreaseRegionPosY = std::stoi(foldRect[10]); // ccm PosY
    int32_t superFoldCreaseRegionPosWidth = std::stoi(foldRect[4]); // ccm PosWidth
    int32_t superFoldCreaseRegionPosHeight = std::stoi(foldRect[9]); // ccm PosHeight

    std::vector<DMRect> rect = {
        {
            superFoldCreaseRegionPosX, superFoldCreaseRegionPosY,
            superFoldCreaseRegionPosWidth, superFoldCreaseRegionPosHeight
        }
    };
    currentSuperFoldCreaseRegion_ = new FoldCreaseRegion(screenIdFull, rect);
}

FoldCreaseRegion SuperFoldStateManager::GetFoldCreaseRegion(bool isVertical, bool isNeedReverse) const
{
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(g_LiveCreaseRegion,
        FOLD_CREASE_DELIMITER);
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return FoldCreaseRegion(0, {});
    }

    ScreenId screenIdFull = 0;
    std::vector<DMRect> foldCreaseRect;
    GetFoldCreaseRect(isVertical, isNeedReverse, foldRect, foldCreaseRect);
    return FoldCreaseRegion(screenIdFull, foldCreaseRect);
}

void SuperFoldStateManager::GetFoldCreaseRect(bool isVertical, bool isNeedReverse,
    const std::vector<int32_t>& foldRect, std::vector<DMRect>& foldCreaseRect) const
{
    int32_t liveCreaseRegionPosX; // live Crease Region PosX
    int32_t liveCreaseRegionPosY; // live Crease Region PosY
    uint32_t liveCreaseRegionPosWidth; // live Crease Region PosWidth
    uint32_t liveCreaseRegionPosHeight; // live Crease Region PosHeight
    int32_t curLeft = foldRect[0];
    int32_t curTop = foldRect[1];
    uint32_t screenWidth = static_cast<uint32_t>(foldRect[SCREEN_WIDTH_INDEX]);
    uint32_t creaseWidth = static_cast<uint32_t>(foldRect[CREASE_WIDTH_INDEX]);
    if (isNeedReverse) {
        auto screenProperty = ScreenSessionManager::GetInstance().GetPhyScreenProperty(SCREEN_ID_FULL);
        curTop = screenProperty.GetPhyBounds().rect_.GetHeight() - curTop - creaseWidth;
    }
    if (isVertical) {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is vertical");
        liveCreaseRegionPosX = curTop;
        liveCreaseRegionPosY = curLeft;
        liveCreaseRegionPosWidth = creaseWidth;
        liveCreaseRegionPosHeight = screenWidth;
    } else {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is horizontal");
        liveCreaseRegionPosX = curLeft;
        liveCreaseRegionPosY = curTop;
        liveCreaseRegionPosWidth = screenWidth;
        liveCreaseRegionPosHeight = creaseWidth;
    }
    foldCreaseRect = {
        {
            liveCreaseRegionPosX, liveCreaseRegionPosY,
            liveCreaseRegionPosWidth, liveCreaseRegionPosHeight
        }
    };
    return;
}

SuperFoldStateManager::SuperFoldStateManager()
{
    InitSuperFoldStateManagerMap();
    InitSuperFoldCreaseRegionParams();
    ScreenSessionManager::GetInstance().SetPropertyChangedCallback(
        std::bind(&SuperFoldStateManager::HandleSuperFoldDisplayCallback, this,
            std::placeholders::_1, std::placeholders::_2));
}

SuperFoldStateManager::~SuperFoldStateManager()
{
}

void SuperFoldStateManager::AddStateManagerMap(SuperFoldStatus curState,
    SuperFoldStatusChangeEvents event,
    SuperFoldStatus nextState,
    std::function<void (SuperFoldStatusChangeEvents)> action)
{
    stateManagerMap_[{curState, event}] = {nextState, action};
}

void SuperFoldStateManager::TransferState(SuperFoldStatus nextState)
{
    TLOGI(WmsLogTag::DMS, "TransferState from %{public}d to %{public}d", curState_.load(), nextState);
    curState_ .store(nextState);
}

FoldStatus SuperFoldStateManager::MatchSuperFoldStatusToFoldStatus(SuperFoldStatus superFoldStatus)
{
    switch (superFoldStatus) {
        case SuperFoldStatus::EXPANDED:
            return FoldStatus::EXPAND;
        case SuperFoldStatus::HALF_FOLDED:
            return FoldStatus::HALF_FOLD;
        case SuperFoldStatus::FOLDED:
            return FoldStatus::FOLDED;
        case SuperFoldStatus::KEYBOARD:
            return FoldStatus::HALF_FOLD;
        default:
            return FoldStatus::UNKNOWN;
    }
}

void SuperFoldStateManager::ReportNotifySuperFoldStatusChange(int32_t currentStatus, int32_t nextStatus,
    float postureAngle)
{
    TLOGI(WmsLogTag::DMS, "currentStatus: %{public}d, nextStatus: %{public}d, postureAngle: %{public}f",
        currentStatus, nextStatus, postureAngle);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "NOTIFY_FOLD_STATE_CHANGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "CURRENT_FOLD_STATUS", currentStatus,
        "NEXT_FOLD_STATUS", nextStatus,
        "SENSOR_POSTURE", postureAngle);
    if (ret != 0) {
        TLOGI(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void SuperFoldStateManager::DriveStateMachineToExpand()
{
    std::unique_lock<std::mutex> lock(superStatusMutex_);
    SuperFoldStatus curFoldState = curState_.load();
    lock.unlock();
    if (curFoldState == SuperFoldStatus::KEYBOARD) {
        HandleSuperFoldStatusChange(SuperFoldStatusChangeEvents::KEYBOARD_OFF);
        HandleSuperFoldStatusChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    } else {
        HandleSuperFoldStatusChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    }
}

void SuperFoldStateManager::HandleSuperFoldStatusChange(SuperFoldStatusChangeEvents event)
{
    std::unique_lock<std::mutex> lock(superStatusMutex_);
    SuperFoldStatus curState = curState_.load();
    SuperFoldStatus nextState = SuperFoldStatus::UNKNOWN;
    bool isTransfer = false;
    std::function<void (SuperFoldStatusChangeEvents)> action;

    auto item = stateManagerMap_.find({curState, event});
    if (item != stateManagerMap_.end()) {
        nextState = item->second.nextState;
        action = item->second.action;
        isTransfer = true;
    }

    float curAngle = SuperFoldSensorManager::GetInstance().GetCurAngle();
    TLOGD(WmsLogTag::DMS, "curAngle: %{public}f", curAngle);
    if (isTransfer && action) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:HandleSuperFoldStatusChange");
        action(event);
        TransferState(nextState);
        if (ScreenSessionManager::GetInstance().IsCaptured()) {
            ModifyMirrorScreenVisibleRect(curState, nextState);
        }
        lock.unlock();
        ReportNotifySuperFoldStatusChange(static_cast<int32_t>(curState), static_cast<int32_t>(nextState), curAngle);
        HandleDisplayNotify(event);
        // notify
        auto screenSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screen session is null!");
            return;
        }
        ScreenId screenId = screenSession->GetScreenId();
        ScreenSessionManager::GetInstance().OnSuperFoldStatusChange(screenId, curState_.load());
        ScreenSessionManager::GetInstance().NotifyFoldStatusChanged(
            MatchSuperFoldStatusToFoldStatus(curState_.load()));
    }
}

void SuperFoldStateManager::ModifyMirrorScreenVisibleRect(SuperFoldStatus preState, SuperFoldStatus curState)
{
    TLOGI(WmsLogTag::DMS, "begin");
    OHOS::Rect rsRect;
    if (curState == SuperFoldStatus::EXPANDED ||
        (preState == SuperFoldStatus::EXPANDED && curState == SuperFoldStatus::HALF_FOLDED)) {
        rsRect = FULL_SCREEN_RECORD_RECT;
        TLOGI(WmsLogTag::DMS, "full screen record");
    } else if ((preState == SuperFoldStatus::HALF_FOLDED && curState == SuperFoldStatus::KEYBOARD) ||
        (preState == SuperFoldStatus::KEYBOARD && curState == SuperFoldStatus::HALF_FOLDED)) {
        rsRect = HALF_FOLD_B_SCREEN_RECORD_RECT;
        TLOGI(WmsLogTag::DMS, "B half record");
    } else {
        TLOGE(WmsLogTag::DMS, "not update record rect");
        return;
    }
    std::vector<DisplayId> displayIds;
    ModifyMirrorScreenVisibleRectInner(rsRect, displayIds);
    ScreenSessionManager::GetInstance().NotifyRecordingDisplayChanged(displayIds);
}

void SuperFoldStateManager::ModifyMirrorScreenVisibleRect(bool isTpKeyboardOn)
{
    // modify rect caused by tp keyboard without foldstatus change
    OHOS::Rect rsRect;
    SuperFoldStatus curStatus = GetCurrentStatus();
    if (isTpKeyboardOn || (!isTpKeyboardOn && curStatus == SuperFoldStatus::HALF_FOLDED)) {
        TLOGI(WmsLogTag::DMS, "B half record");
        rsRect = HALF_FOLD_B_SCREEN_RECORD_RECT;
    } else {
        TLOGI(WmsLogTag::DMS, "caused by foldstatus change, return");
        return;
    }
    std::vector<DisplayId> displayIds;
    ModifyMirrorScreenVisibleRectInner(rsRect, displayIds);
    ScreenSessionManager::GetInstance().NotifyRecordingDisplayChanged(displayIds);
}

void SuperFoldStateManager::ModifyMirrorScreenVisibleRectInner(const OHOS::Rect& rsRect,
    std::vector<DisplayId>& displayIds)
{
    std::map<ScreenId, OHOS::Rect> mirrorScreenVisibleRectMap;
    {
        std::unique_lock<std::mutex> lock(mirrorScreenIdsMutex_);
        mirrorScreenVisibleRectMap = mirrorScreenVisibleRectMap_;
    }
    for (auto& [screenId, curRect]: mirrorScreenVisibleRectMap) {
        ScreenId rsId = SCREEN_ID_INVALID;
        ScreenSessionManager::GetInstance().ConvertScreenIdToRsScreenId(screenId, rsId);
        TLOGI(WmsLogTag::DMS, "handle mirror ScreenId: %{public}" PRIu64 ", rsId:  %{public}" PRIu64, screenId, rsId);
        displayIds = CalculateReCordingDisplayIds(rsRect);
        RSInterfaces::GetInstance().SetMirrorScreenVisibleRect(rsId, rsRect);
        curRect = rsRect;
    }
}

std::vector<DisplayId> SuperFoldStateManager::CalculateReCordingDisplayIds(const OHOS::Rect& nextRect)
{
    std::vector<DisplayId> displayIds = {MAIN_SCREEN_ID_DEFAULT};
    if (nextRect == FULL_SCREEN_RECORD_RECT && curState_ == SuperFoldStatus::HALF_FOLDED) {
        displayIds.emplace_back(DISPLAY_ID_FAKE);
    }
    return displayIds;
}

void SuperFoldStateManager::AddMirrorVirtualScreenIds(const std::vector<ScreenId>& screenIds, const DMRect& rect)
{
    std::unique_lock<std::mutex> lock(mirrorScreenIdsMutex_);
    OHOS::Rect rsRect{
        .x = rect.posX_,
        .y = rect.posY_,
        .w = rect.width_,
        .h = rect.height_,
    };
    for (const auto& screenId : screenIds) {
        auto it = mirrorScreenVisibleRectMap_.find(screenId);
        if (it == mirrorScreenVisibleRectMap_.end()) {
            mirrorScreenVisibleRectMap_[screenId] = rsRect;
        }
    }
}

void SuperFoldStateManager::ClearMirrorVirtualScreenIds(const std::vector<ScreenId>& screenIds)
{
    std::unique_lock<std::mutex> lock(mirrorScreenIdsMutex_);
    for (const auto& screenId : screenIds) {
        auto it = mirrorScreenVisibleRectMap_.find(screenId);
        if (it != mirrorScreenVisibleRectMap_.end()) {
            mirrorScreenVisibleRectMap_.erase(it);
        }
    }
}

SuperFoldStatus SuperFoldStateManager::GetCurrentStatus()
{
    return curState_.load();
}

void SuperFoldStateManager::SetCurrentStatus(SuperFoldStatus curState)
{
    std::unique_lock<std::mutex> lock(superStatusMutex_);
    curState_.store(curState);
}

sptr<FoldCreaseRegion> SuperFoldStateManager::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion()");
    return currentSuperFoldCreaseRegion_;
}

FoldCreaseRegion SuperFoldStateManager::GetLiveCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "enter");
    SuperFoldStatus curFoldState = ScreenSessionManager::GetInstance().GetSuperFoldStatus();
    if (curFoldState == SuperFoldStatus::UNKNOWN || curFoldState == SuperFoldStatus::FOLDED) {
        return FoldCreaseRegion(0, {});
    }
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return FoldCreaseRegion(0, {});
    }
    DisplayOrientation displayOrientation = screenSession->GetScreenProperty().GetDisplayOrientation();
    switch (displayOrientation) {
        case DisplayOrientation::PORTRAIT: {
            liveCreaseRegion_ = GetFoldCreaseRegion(false, false);
            break;
        }
        case DisplayOrientation::PORTRAIT_INVERTED: {
            liveCreaseRegion_ = GetFoldCreaseRegion(false, true);
            break;
        }
        case DisplayOrientation::LANDSCAPE: {
            liveCreaseRegion_ = GetFoldCreaseRegion(true, true);
            break;
        }
        case DisplayOrientation::LANDSCAPE_INVERTED: {
            liveCreaseRegion_ = GetFoldCreaseRegion(true, false);
            break;
        }
        default: {
            TLOGE(WmsLogTag::DMS, "displayOrientation is invalid");
        }
    }
    return liveCreaseRegion_;
}

nlohmann::ordered_json SuperFoldStateManager::GetFoldCreaseRegionJson()
{
    if (superFoldCreaseRegionItems_.size() == 0) {
        GetAllCreaseRegion();
    }
    nlohmann::ordered_json ret = nlohmann::ordered_json::array();
    for (const auto& foldCreaseRegionItem : superFoldCreaseRegionItems_) {
        nlohmann::ordered_json capabilityInfo;
        capabilityInfo["superFoldStatus"] =
            std::to_string(static_cast<int32_t>(foldCreaseRegionItem.superFoldStatus_));
        capabilityInfo["displayOrientation"] =
            std::to_string(static_cast<int32_t>(foldCreaseRegionItem.orientation_));
        capabilityInfo["creaseRects"]["displayId"] =
            std::to_string(static_cast<int32_t>(foldCreaseRegionItem.region_.GetDisplayId()));
        auto creaseRects = foldCreaseRegionItem.region_.GetCreaseRects();
        capabilityInfo["creaseRects"]["rects"] = nlohmann::ordered_json::array();
        for (const auto& creaseRect : creaseRects) {
            capabilityInfo["creaseRects"]["rects"].push_back({
                {"posX", creaseRect.posX_},
                {"posY", creaseRect.posY_},
                {"width", creaseRect.width_},
                {"height", creaseRect.height_}
            });
        }
        ret.push_back(capabilityInfo);
    }
    return ret;
}

void SuperFoldStateManager::GetAllCreaseRegion()
{
    SuperFoldCreaseRegionItem FCreaseItem{DisplayOrientation::LANDSCAPE, SuperFoldStatus::FOLDED,
        FoldCreaseRegion(0, {})};
    SuperFoldCreaseRegionItem HPorCreaseItem{DisplayOrientation::PORTRAIT, SuperFoldStatus::HALF_FOLDED,
        GetFoldCreaseRegion(false, false)};
    SuperFoldCreaseRegionItem HLandCreaseItem{DisplayOrientation::LANDSCAPE, SuperFoldStatus::HALF_FOLDED,
        GetFoldCreaseRegion(true, false)};
    SuperFoldCreaseRegionItem EPorCreaseItem{DisplayOrientation::PORTRAIT, SuperFoldStatus::EXPANDED,
        GetFoldCreaseRegion(false, false)};
    SuperFoldCreaseRegionItem ELandCreaseItem{DisplayOrientation::LANDSCAPE, SuperFoldStatus::EXPANDED,
        GetFoldCreaseRegion(true, true)};
    SuperFoldCreaseRegionItem EPorInvRCreaseItem{DisplayOrientation::PORTRAIT_INVERTED, SuperFoldStatus::EXPANDED,
        GetFoldCreaseRegion(false, true)};
    SuperFoldCreaseRegionItem ELandInvRCreaseItem{DisplayOrientation::LANDSCAPE_INVERTED, SuperFoldStatus::EXPANDED,
        GetFoldCreaseRegion(true, false)};
    SuperFoldCreaseRegionItem KPorCreaseItem{DisplayOrientation::PORTRAIT, SuperFoldStatus::KEYBOARD,
        GetFoldCreaseRegion(false, false)};
    SuperFoldCreaseRegionItem KLandCreaseItem{DisplayOrientation::LANDSCAPE, SuperFoldStatus::KEYBOARD,
        GetFoldCreaseRegion(true, false)};
    superFoldCreaseRegionItems_.push_back(FCreaseItem);
    superFoldCreaseRegionItems_.push_back(HPorCreaseItem);
    superFoldCreaseRegionItems_.push_back(HLandCreaseItem);
    superFoldCreaseRegionItems_.push_back(EPorCreaseItem);
    superFoldCreaseRegionItems_.push_back(ELandCreaseItem);
    superFoldCreaseRegionItems_.push_back(EPorInvRCreaseItem);
    superFoldCreaseRegionItems_.push_back(ELandInvRCreaseItem);
    superFoldCreaseRegionItems_.push_back(KPorCreaseItem);
    superFoldCreaseRegionItems_.push_back(KLandCreaseItem);
}

void SuperFoldStateManager::HandleDisplayNotify(SuperFoldStatusChangeEvents changeEvent)
{
    TLOGI(WmsLogTag::DMS, "changeEvent: %{public}d", static_cast<uint32_t>(changeEvent));
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    if (screenSession->GetFakeScreenSession() == nullptr) {
        TLOGE(WmsLogTag::DMS, "fake screen session is null");
        return;
    }

    if (!ScreenSessionManager::GetInstance().GetClientProxy()) {
        HandleSuperFoldDisplayInServer(screenSession, changeEvent);
    } else {
        ScreenProperty property = screenSession->GetScreenProperty();
        // changeEvent only forward to client by property, not save in session property
        property.SetSuperFoldStatusChangeEvent(changeEvent);
        property.SetFoldStatus(ScreenSessionManager::GetInstance().GetSuperFoldStatus());
        property.SetCurrentValidHeight(GetCurrentValidHeight(screenSession));
        property.SetIsKeyboardOn(isKeyboardOn_);

        TLOGI(WmsLogTag::DMS,
              "GetIsFakeInUse: %{public}d, GetCurrentValidHeight: %{public}d  isKeyboardOn: %{public}d",
              property.GetIsFakeInUse(),
              property.GetCurrentValidHeight(),
              isKeyboardOn_);
        UpdateScreenHalfState(screenSession, changeEvent);
        screenSession->NotifyFoldPropertyChange(
            property, ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE, FoldDisplayMode::UNKNOWN);
    }
}

void SuperFoldStateManager::UpdateScreenHalfState(sptr<ScreenSession>& screenSession,
    SuperFoldStatusChangeEvents changeEvent)
{
    switch (changeEvent) {
        case SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED:
            screenSession->SetIsBScreenHalf(true);
            break;
        case SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED:
            screenSession->SetIsBScreenHalf(false);
            break;
        case SuperFoldStatusChangeEvents::KEYBOARD_ON:
            screenSession->SetIsBScreenHalf(true);
            break;
        case SuperFoldStatusChangeEvents::KEYBOARD_OFF:
            screenSession->SetIsBScreenHalf(true);
            break;
        case SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON: {
            SuperFoldStatus curFoldState = curState_.load();
            if (!isKeyboardOn_ && curFoldState == SuperFoldStatus::HALF_FOLDED) {
                screenSession->SetIsFakeInUse(false);
                screenSession->SetIsBScreenHalf(true);
            }
            break;
            }
        case SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF: {
            SuperFoldStatus curFoldStateOff = curState_.load();
            if (!isKeyboardOn_ && curFoldStateOff == SuperFoldStatus::HALF_FOLDED) {
                screenSession->SetIsFakeInUse(true);
                screenSession->SetIsBScreenHalf(true);
            }
            break;
        }
        default:
            break;
    }
}

void SuperFoldStateManager::HandleSuperFoldDisplayInServer(sptr<ScreenSession>& screenSession,
    SuperFoldStatusChangeEvents changeEvent)
{
    switch (changeEvent) {
        case SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED: {
            HandleExtendToHalfFoldDisplayNotifyInServer(screenSession);
            TLOGI(WmsLogTag::DMS, "handle extend change to half fold");
            break;
        }
        case SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED: {
            HandleHalfFoldToExtendDisplayNotifyInServer(screenSession);
            TLOGI(WmsLogTag::DMS, "handle half fold change to extend");
            break;
        }
        case SuperFoldStatusChangeEvents::KEYBOARD_ON: {
            HandleKeyboardOnDisplayNotifyInServer(screenSession);
            TLOGI(WmsLogTag::DMS, "handle keyboard on");
            break;
        }
        case SuperFoldStatusChangeEvents::KEYBOARD_OFF: {
            HandleKeyboardOffDisplayNotifyInServer(screenSession);
            TLOGI(WmsLogTag::DMS, "handle keyboard off");
            break;
        }
        case SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON: {
            HandleSystemKeyboardStatusDisplayNotifyInServer(screenSession, true);
            TLOGI(WmsLogTag::DMS, "handle system keyboard on");
            break;
        }
        case SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF: {
            HandleSystemKeyboardStatusDisplayNotifyInServer(screenSession, false);
            TLOGI(WmsLogTag::DMS, "handle system keyboard off");
            break;
        }
        default:
            TLOGE(WmsLogTag::DMS, "nothing to handle changeEvent:%{public}d", changeEvent);
            break;
    }
}

void SuperFoldStateManager::HandleSuperFoldDisplayCallback(sptr<ScreenSession>& screenSession,
    SuperFoldStatusChangeEvents changeEvent)
{
    switch (changeEvent) {
        case SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED: {
            HandleExtendToHalfFoldDisplayNotify(screenSession);
            TLOGI(WmsLogTag::DMS, "handle extend to half folded callback");
            break;
        }
        case SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED: {
            HandleHalfFoldToExtendDisplayNotify(screenSession);
            TLOGI(WmsLogTag::DMS, "handle half folded to extend callback");
            break;
        }
        case SuperFoldStatusChangeEvents::KEYBOARD_ON: {
            HandleKeyboardOnDisplayNotify(screenSession);
            TLOGI(WmsLogTag::DMS, "handle keyboard on callback");
            break;
        }
        case SuperFoldStatusChangeEvents::KEYBOARD_OFF: {
            HandleKeyboardOffDisplayNotify(screenSession);
            TLOGI(WmsLogTag::DMS, "handle keyboard off callback");
            break;
        }
        case SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON: {
            HandleSystemKeyboardStatusDisplayNotify(screenSession, true);
            TLOGI(WmsLogTag::DMS, "handle system keyboard on callback");
            break;
        }
        case SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF: {
            HandleSystemKeyboardStatusDisplayNotify(screenSession, false);
            TLOGI(WmsLogTag::DMS, "handle system keyboard off callback");
            break;
        }
        default:
            TLOGE(WmsLogTag::DMS, "nothing to handle");
            return;
    }
}

void SuperFoldStateManager::HandleExtendToHalfFoldDisplayNotifyInServer(sptr<ScreenSession>& screenSession)
{
    screenSession->UpdatePropertyByFakeInUse(true);
    screenSession->SetIsBScreenHalf(true);
    ScreenSessionManager::GetInstance().NotifyDisplayChanged(
        screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::SUPER_FOLD_RESOLUTION_CHANGED);
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    ScreenSessionManager::GetInstance().NotifyDisplayCreate(
        fakeScreenSession->ConvertToDisplayInfo());
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE);
    RefreshExternalRegion();
    ScreenSessionManager::GetInstance().GetStaticAndDynamicSession();
}

void SuperFoldStateManager::HandleHalfFoldToExtendDisplayNotifyInServer(sptr<ScreenSession>& screenSession)
{
    screenSession->UpdatePropertyByFakeInUse(false);
    screenSession->SetIsBScreenHalf(false);
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get fake display failed");
        return;
    }
    DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
    ScreenSessionManager::GetInstance().NotifyDisplayDestroy(fakeDisplayId);
    ScreenSessionManager::GetInstance().NotifyDisplayChanged(
        screenSession->ConvertToDisplayInfo(),
        DisplayChangeEvent::SUPER_FOLD_RESOLUTION_CHANGED);
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE);
    RefreshExternalRegion();
    ScreenSessionManager::GetInstance().GetStaticAndDynamicSession();
}

void SuperFoldStateManager::HandleKeyboardOnDisplayNotifyInServer(sptr<ScreenSession>& screenSession)
{
    auto screenBounds = screenSession->GetScreenProperty().GetBounds();
    bool currFakeInUse = screenSession->GetScreenProperty().GetIsFakeInUse();
    screenSession->UpdatePropertyByFakeInUse(false);
    screenSession->SetIsBScreenHalf(true);
    int32_t validheight = GetCurrentValidHeight(screenSession);
    if (screenBounds.rect_.GetWidth() < screenBounds.rect_.GetHeight()) {
        screenSession->SetValidHeight(validheight);
        screenSession->SetValidWidth(screenBounds.rect_.GetWidth());
    } else {
        screenSession->SetValidWidth(screenBounds.rect_.GetHeight());
        screenSession->SetValidHeight(validheight);
    }
    screenSession->SetScreenAreaHeight(DISPLAY_B_HEIGHT);
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get fake display failed");
        return;
    }
    DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
    bool isDestroy = screenSession->GetIsDestroyDisplay();
    if (isDestroy) {
        ScreenSessionManager::GetInstance().NotifyDisplayDestroy(fakeDisplayId);
    }
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE);
    ScreenSessionManager::GetInstance().UpdateValidArea(
        screenSession->GetScreenId(),
        screenSession->GetValidWidth(),
        screenSession->GetValidHeight());
}

void SuperFoldStateManager::HandleKeyboardOffDisplayNotifyInServer(sptr<ScreenSession>& screenSession)
{
    auto screenBounds = screenSession->GetScreenProperty().GetBounds();
    screenSession->UpdatePropertyByFakeInUse(true);
    screenSession->SetIsBScreenHalf(true);
    screenSession->SetValidWidth(screenBounds.rect_.GetWidth());
    screenSession->SetValidHeight(screenBounds.rect_.GetHeight());
    screenSession->SetScreenAreaHeight(DISPLAY_A_HEIGHT);
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    ScreenSessionManager::GetInstance().NotifyDisplayCreate(
        fakeScreenSession->ConvertToDisplayInfo());
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE);
    ScreenSessionManager::GetInstance().UpdateValidArea(
        screenSession->GetScreenId(),
        screenSession->GetValidWidth(),
        screenSession->GetValidHeight());
}

void SuperFoldStateManager::HandleSystemKeyboardStatusDisplayNotifyInServer(
    sptr<ScreenSession>& screenSession, bool isTpKeyboardOn)
{
    SuperFoldStatus curFoldState = curState_.load();
    TLOGD(WmsLogTag::DMS, "curFoldState: %{public}u", curFoldState);
    if (!isKeyboardOn_ && curFoldState == SuperFoldStatus::HALF_FOLDED) {
        screenSession->UpdatePropertyByFakeInUse(!isTpKeyboardOn);
        screenSession->SetIsFakeInUse(!isTpKeyboardOn);
        screenSession->SetIsBScreenHalf(true);
    }
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get fake display failed");
        return;
    }
    if (isTpKeyboardOn) {
        DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
        ScreenSessionManager::GetInstance().NotifyDisplayDestroy(fakeDisplayId);
        auto screenBounds = screenSession->GetScreenProperty().GetBounds();
        int32_t validheight = GetCurrentValidHeight(screenSession);
        if (screenBounds.rect_.GetWidth() < screenBounds.rect_.GetHeight()) {
            screenSession->SetPointerActiveWidth(static_cast<uint32_t>(screenBounds.rect_.GetWidth()));
            screenSession->SetPointerActiveHeight(static_cast<uint32_t>(validheight));
        } else {
            screenSession->SetPointerActiveWidth(static_cast<uint32_t>(screenBounds.rect_.GetHeight()));
            screenSession->SetPointerActiveHeight(static_cast<uint32_t>(validheight));
        }
        TLOGD(WmsLogTag::DMS, " vh: %{public}d, paw: %{public}u, pah: %{public}u", validheight,
            screenSession->GetPointerActiveWidth(), screenSession->GetPointerActiveHeight());
    } else {
        if (curFoldState == SuperFoldStatus::HALF_FOLDED) {
            ScreenSessionManager::GetInstance().NotifyDisplayCreate(fakeDisplayInfo);
        }
        screenSession->SetPointerActiveWidth(0);
        screenSession->SetPointerActiveHeight(0);
        TLOGD(WmsLogTag::DMS, "paw: %{public}u, pah: %{public}u",
            screenSession->GetPointerActiveWidth(), screenSession->GetPointerActiveHeight());
    }
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE);
}

void SuperFoldStateManager::HandleExtendToHalfFoldDisplayNotify(sptr<ScreenSession>& screenSession)
{
    TLOGI(WmsLogTag::DMS, "SuperFoldStateManager HandleExtendToHalfFoldDisplayNotify");
    ScreenSessionManager::GetInstance().NotifyDisplayChanged(
        screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::SUPER_FOLD_RESOLUTION_CHANGED);
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    ScreenSessionManager::GetInstance().NotifyDisplayCreate(
        fakeScreenSession->ConvertToDisplayInfo());
    RefreshExternalRegion();
    ScreenSessionManager::GetInstance().GetStaticAndDynamicSession();
}

uint32_t SuperFoldStateManager::GetFoldCreaseHeight() const
{
    if (currentSuperFoldCreaseRegion_ == nullptr) {
        return DEFAULT_FOLD_REGION_HEIGHT;
    }
    std::vector<DMRect> creaseRects = currentSuperFoldCreaseRegion_->GetCreaseRects();
    if (!creaseRects.empty()) {
        return creaseRects[0].height_;
    }
    return DEFAULT_FOLD_REGION_HEIGHT;
}

DMError SuperFoldStateManager::RefreshMirrorRegionInner(
    sptr<ScreenSession>& mainScreenSession, sptr<ScreenSession>& secondarySession)
{
    if (mainScreenSession == nullptr || secondarySession == nullptr) {
        TLOGE(WmsLogTag::DMS, "mainScreenSession or secondarySession is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto mainScreenProperty = mainScreenSession->GetScreenProperty();
    auto screenProperty = secondarySession->GetScreenProperty();
    std::shared_ptr<RSDisplayNode> displayNode = mainScreenSession->GetDisplayNode();
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null.");
        return DMError::DM_ERROR_NULLPTR;
    }
    DMRect mirrorRegion = DMRect::NONE();
    if (GetCurrentStatus() != SuperFoldStatus::EXPANDED) {
        mirrorRegion.posX_ = 0;
        mirrorRegion.posY_ = 0;
        mirrorRegion.width_ = mainScreenProperty.GetScreenRealWidth();
        mirrorRegion.height_ = (mainScreenProperty.GetScreenRealHeight() - GetFoldCreaseHeight()) / HEIGHT_HALF;
        if (mirrorRegion.width_ == 0 || mirrorRegion.height_ == 0) {
            TLOGE(WmsLogTag::DMS, "mirrorRegion.width_:%{public}d mirrorRegion.height_:%{public}d",
                mirrorRegion.width_, mirrorRegion.height_);
            return DMError::DM_ERROR_INVALID_PARAM;
        }
    } else {
        mirrorRegion.width_ = mainScreenProperty.GetScreenRealWidth();
        mirrorRegion.height_ = mainScreenProperty.GetScreenRealHeight();
    }
    mainScreenSession->UpdateMirrorWidth(mirrorRegion.width_);
    mainScreenSession->UpdateMirrorHeight(mainScreenProperty.GetScreenRealHeight());
    secondarySession->UpdateMirrorWidth(mirrorRegion.width_);
    secondarySession->UpdateMirrorHeight(mirrorRegion.height_);
    secondarySession->SetMirrorScreenRegion(secondarySession->GetScreenId(), mirrorRegion);
    secondarySession->SetIsPhysicalMirrorSwitch(true);
    secondarySession->EnableMirrorScreenRegion();
    RSDisplayNodeConfig config = { secondarySession->rsId_, true, displayNode->GetId() };
    secondarySession->ReuseDisplayNode(config);
    return DMError::DM_OK;
}

DMError SuperFoldStateManager::RefreshExternalRegion()
{
    if (!ScreenSessionManager::GetInstance().GetIsPhysicalExtendScreenConnected()) {
        TLOGW(WmsLogTag::DMS, "extend screen not connect");
        return DMError::DM_OK;
    }
    sptr<ScreenSession> mainScreenSession = ScreenSessionManager::GetInstance().GetScreenSessionByRsId(SCREEN_ID_FULL);
    if (mainScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenSession null");
        return DMError::DM_ERROR_NULLPTR;
    }
    std::vector<ScreenId> screenIds = ScreenSessionManager::GetInstance().GetAllScreenIds();
    for (auto screenId : screenIds) {
        sptr<ScreenSession> secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
        if (secondarySession == nullptr) {
            TLOGE(WmsLogTag::DMS, "get mainScreenSession failed secondarySession null");
            continue;
        }
        if (secondarySession->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
            secondarySession->GetIsExtend()) {
            if (secondarySession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
                RefreshMirrorRegionInner(mainScreenSession, secondarySession);
            }
        }
    }
    return DMError::DM_OK;
}

void SuperFoldStateManager::HandleHalfFoldToExtendDisplayNotify(sptr<ScreenSession>& screenSession)
{
    TLOGI(WmsLogTag::DMS, "SuperFoldStateManager HandleHalfFoldToExtendDisplayNotify");
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get fake display failed");
        return;
    }
    DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
    ScreenSessionManager::GetInstance().NotifyDisplayDestroy(fakeDisplayId);
    ScreenSessionManager::GetInstance().NotifyDisplayChanged(
        screenSession->ConvertToDisplayInfo(),
        DisplayChangeEvent::SUPER_FOLD_RESOLUTION_CHANGED);
    RefreshExternalRegion();
    ScreenSessionManager::GetInstance().GetStaticAndDynamicSession();
}

void SuperFoldStateManager::HandleKeyboardOnDisplayNotify(sptr<ScreenSession>& screenSession)
{
    TLOGI(WmsLogTag::DMS, "SuperFoldStateManager HandleKeyboardOnDisplayNotify");
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get fake display failed");
        return;
    }
    DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
    bool isDestroy = screenSession->GetIsDestroyDisplay();
    if (isDestroy) {
        ScreenSessionManager::GetInstance().NotifyDisplayDestroy(fakeDisplayId);
    }
}

void SuperFoldStateManager::HandleKeyboardOffDisplayNotify(sptr<ScreenSession>& screenSession)
{
    TLOGI(WmsLogTag::DMS, "SuperFoldStateManager HandleKeyboardOffDisplayNotify");
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    ScreenSessionManager::GetInstance().NotifyDisplayCreate(
        fakeScreenSession->ConvertToDisplayInfo());
}

void SuperFoldStateManager::SetSystemKeyboardStatus(bool isTpKeyboardOn)
{
    SuperFoldStatusChangeEvents events = isTpKeyboardOn ? SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON
        : SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF;
    isSystemKeyboardOn_ = isTpKeyboardOn;
    HandleDisplayNotify(events);
}

bool SuperFoldStateManager::GetSystemKeyboardStatus()
{
    return isSystemKeyboardOn_;
}

void SuperFoldStateManager::HandleSystemKeyboardStatusDisplayNotify(
    sptr<ScreenSession>& screenSession, bool isTpKeyboardOn)
{
    SuperFoldStatus curFoldState = curState_.load();
    TLOGD(WmsLogTag::DMS, "curFoldState: %{public}u", curFoldState);
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get fake display failed");
        return;
    }
    if (isTpKeyboardOn) {
        DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
        ScreenSessionManager::GetInstance().NotifyDisplayDestroy(fakeDisplayId);
        TLOGD(WmsLogTag::DMS, "paw: %{public}u, pah: %{public}u",
            screenSession->GetPointerActiveWidth(), screenSession->GetPointerActiveHeight());
    } else {
        if (curFoldState == SuperFoldStatus::HALF_FOLDED) {
            ScreenSessionManager::GetInstance().NotifyDisplayCreate(fakeDisplayInfo);
        }
        TLOGD(WmsLogTag::DMS, "paw: %{public}u, pah: %{public}u",
            screenSession->GetPointerActiveWidth(), screenSession->GetPointerActiveHeight());
    }
    if (ScreenSessionManager::GetInstance().IsCaptured()) {
        ModifyMirrorScreenVisibleRect(isTpKeyboardOn);
    }
}

int32_t SuperFoldStateManager::GetCurrentValidHeight(sptr<ScreenSession> screenSession)
{
    if (currentSuperFoldCreaseRegion_ != nullptr) {
        std::vector<DMRect> creaseRects = currentSuperFoldCreaseRegion_->GetCreaseRects();
        if (!creaseRects.empty()) {
            TLOGI(WmsLogTag::DMS, "validheight: %{public}d", creaseRects[0].posY_);
            return creaseRects[0].posY_;
        }
    }
    TLOGE(WmsLogTag::DMS, "Get CreaseRects failed, validheight is half of height");
    auto screenBounds = screenSession->GetScreenProperty().GetBounds();
    if (screenBounds.rect_.GetWidth() < screenBounds.rect_.GetHeight()) {
        return screenBounds.rect_.GetHeight() / HEIGHT_HALF;
    } else {
        return screenBounds.rect_.GetWidth() / HEIGHT_HALF;
    }
}

bool SuperFoldStateManager::ChangeScreenState(bool toHalf)
{
    ScreenSessionManager::GetInstance().NotifyScreenMagneticStateChanged(toHalf);
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().
        GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session is null!");
        return false;
    }
    auto screenProperty = ScreenSessionManager::GetInstance().
        GetPhyScreenProperty(screenSession->GetScreenId());
    auto screenWidth = screenProperty.GetPhyBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetPhyBounds().rect_.GetHeight();
    if (toHalf) {
        screenWidth = screenProperty.GetPhyBounds().rect_.GetWidth();
        screenHeight = screenProperty.GetPhyBounds().rect_.GetHeight() / HEIGHT_HALF;
    }
    OHOS::Rect rectCur{
        .x = 0,
        .y = 0,
        .w = static_cast<int>(screenWidth),
        .h = static_cast<int>(screenHeight),
    };
    // SCREEN_ID_FULL = 0
    auto response = RSInterfaces::GetInstance().SetScreenActiveRect(0, rectCur);
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE,
        toHalf ? KEYBOARD_ON_CONFIG : KEYBOARD_OFF_CONFIG, TpFeatureConfigType::AFT_TP_FEATURE);
#endif
    TLOGI(WmsLogTag::DMS, "rect [%{public}f , %{public}f], rs response is %{public}ld",
        screenWidth, screenHeight, static_cast<long>(response));
    return true;
}

bool SuperFoldStateManager::GetKeyboardState()
{
    TLOGI(WmsLogTag::DMS, "GetKeyboardState isKeyboardOn_ : %{public}d", isKeyboardOn_);
    return isKeyboardOn_;
}
} // Rosen
} // OHOS