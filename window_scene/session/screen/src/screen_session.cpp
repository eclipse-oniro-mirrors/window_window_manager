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

#include "session/screen/include/screen_session.h"
#include <hisysevent.h>

#include <hitrace_meter.h>
#include <surface_capture_future.h>
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "dms_xcollie.h"
#include "fold_screen_state_internel.h"
#include <parameters.h>

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "ScreenSession" };
static const int32_t g_screenRotationOffSet = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
static const int32_t ROTATION_90 = 1;
static const int32_t ROTATION_270 = 3;
const unsigned int XCOLLIE_TIMEOUT_5S = 5;
}

ScreenSession::ScreenSession(const ScreenSessionConfig& config, ScreenSessionReason reason)
    : name_(config.name), screenId_(config.screenId), rsId_(config.rsId), defaultScreenId_(config.defaultScreenId),
    property_(config.property), displayNode_(config.displayNode)
{
    TLOGI(WmsLogTag::DMS,
        "[DPNODE]Create Session, reason: %{public}d, screenId: %{public}" PRIu64", rsId: %{public}" PRIu64"",
        reason, screenId_, rsId_);
    TLOGI(WmsLogTag::DMS,
        "[DPNODE]Config name: %{public}s, defaultId: %{public}" PRIu64", mirrorNodeId: %{public}" PRIu64"",
        name_.c_str(), defaultScreenId_, config.mirrorNodeId);
    Rosen::RSDisplayNodeConfig rsConfig;
    bool isNeedCreateDisplayNode = true;
    switch (reason) {
        case ScreenSessionReason::CREATE_SESSION_FOR_CLIENT: {
            TLOGI(WmsLogTag::DMS, "create screen session for client. noting to do.");
            return;
        }
        case ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL: {
            // create virtual screen should use rsid
            rsConfig.screenId = rsId_;
            break;
        }
        case ScreenSessionReason::CREATE_SESSION_FOR_MIRROR: {
            rsConfig.screenId = screenId_;
            rsConfig.isMirrored = true;
            rsConfig.mirrorNodeId = config.mirrorNodeId;
            rsConfig.isSync = true;
            break;
        }
        case ScreenSessionReason::CREATE_SESSION_FOR_REAL: {
            rsConfig.screenId = screenId_;
            break;
        }
        case ScreenSessionReason::CREATE_SESSION_WITHOUT_DISPLAY_NODE: {
            TLOGI(WmsLogTag::DMS, "screen session no need create displayNode.");
            isNeedCreateDisplayNode = false;
            break;
        }
        default : {
            TLOGE(WmsLogTag::DMS, "INVALID invalid screen session config.");
            break;
        }
    }
    if (isNeedCreateDisplayNode) {
        CreateDisplayNode(rsConfig);
    }
}

void ScreenSession::CreateDisplayNode(const Rosen::RSDisplayNodeConfig& config)
{
    TLOGI(WmsLogTag::DMS,
        "[DPNODE]config screenId: %{public}" PRIu64", mirrorNodeId: %{public}" PRIu64", isMirrored: %{public}d",
        config.screenId, config.mirrorNodeId, static_cast<int32_t>(config.isMirrored));
    {
        std::unique_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        displayNode_ = Rosen::RSDisplayNode::Create(config);
        if (displayNode_) {
            displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
                property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
            displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
                property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        } else {
            TLOGE(WmsLogTag::DMS, "Failed to create displayNode, displayNode is null!");
        }
    }
    RSTransaction::FlushImplicitTransaction();
}

ScreenSession::ScreenSession(ScreenId screenId, ScreenId rsId, const std::string& name,
    const ScreenProperty& property, const std::shared_ptr<RSDisplayNode>& displayNode)
    : name_(name), screenId_(screenId), rsId_(rsId), property_(property), displayNode_(displayNode)
{
    WLOGFI("Success to create screenSession in constructor_0, screenid is %{public}" PRIu64"", screenId_);
}

ScreenSession::ScreenSession(ScreenId screenId, const ScreenProperty& property, ScreenId defaultScreenId)
    : screenId_(screenId), defaultScreenId_(defaultScreenId), property_(property)
{
    Rosen::RSDisplayNodeConfig config = { .screenId = screenId_ };
    displayNode_ = Rosen::RSDisplayNode::Create(config);
    if (displayNode_) {
        WLOGI("Success to create displayNode in constructor_1, screenid is %{public}" PRIu64"", screenId_);
        displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
    } else {
        WLOGFE("Failed to create displayNode, displayNode is null!");
    }
    RSTransaction::FlushImplicitTransaction();
}

ScreenSession::ScreenSession(ScreenId screenId, const ScreenProperty& property,
    NodeId nodeId, ScreenId defaultScreenId)
    : screenId_(screenId), defaultScreenId_(defaultScreenId), property_(property)
{
    rsId_ = screenId;
    Rosen::RSDisplayNodeConfig config = { .screenId = screenId_, .isMirrored = true, .mirrorNodeId = nodeId,
        .isSync = true};
    displayNode_ = Rosen::RSDisplayNode::Create(config);
    if (displayNode_) {
        WLOGI("Success to create displayNode in constructor_2, screenid is %{public}" PRIu64"", screenId_);
        displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
    } else {
        WLOGFE("Failed to create displayNode, displayNode is null!");
    }
    RSTransaction::FlushImplicitTransaction();
}

ScreenSession::ScreenSession(const std::string& name, ScreenId smsId, ScreenId rsId, ScreenId defaultScreenId)
    : name_(name), screenId_(smsId), rsId_(rsId), defaultScreenId_(defaultScreenId)
{
    (void)rsId_;
    // 虚拟屏的screen id和rs id不一致，displayNode的创建应使用rs id
    Rosen::RSDisplayNodeConfig config = { .screenId = rsId_ };
    displayNode_ = Rosen::RSDisplayNode::Create(config);
    if (displayNode_) {
        WLOGI("Success to create displayNode in constructor_3, rs id is %{public}" PRIu64"", rsId_);
        displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
    } else {
        WLOGFE("Failed to create displayNode, displayNode is null!");
    }
    RSTransaction::FlushImplicitTransaction();
}

void ScreenSession::SetMirrorScreenType(MirrorScreenType mirrorType)
{
    mirrorScreenType_ = mirrorType;
}

MirrorScreenType ScreenSession::GetMirrorScreenType()
{
    return mirrorScreenType_;
}

void ScreenSession::SetDisplayNodeScreenId(ScreenId screenId)
{
    std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
    if (displayNode_ != nullptr) {
        WLOGFI("SetDisplayNodeScreenId %{public}" PRIu64"", screenId);
        displayNode_->SetScreenId(screenId);
    }
}

void ScreenSession::RegisterScreenChangeListener(IScreenChangeListener* screenChangeListener)
{
    if (screenChangeListener == nullptr) {
        WLOGFE("Failed to register screen change listener, listener is null!");
        return;
    }

    if (std::find(screenChangeListenerList_.begin(), screenChangeListenerList_.end(), screenChangeListener) !=
        screenChangeListenerList_.end()) {
        WLOGFI("Repeat to register screen change listener!");
        return;
    }

    screenChangeListenerList_.emplace_back(screenChangeListener);
    if (screenState_ == ScreenState::CONNECTION) {
        screenChangeListener->OnConnect(screenId_);
        WLOGFI("Success to call onconnect callback.");
    }
    WLOGFI("Success to register screen change listener.");
}

void ScreenSession::UnregisterScreenChangeListener(IScreenChangeListener* screenChangeListener)
{
    if (screenChangeListener == nullptr) {
        WLOGFE("Failed to unregister screen change listener, listener is null!");
        return;
    }

    screenChangeListenerList_.erase(
        std::remove_if(screenChangeListenerList_.begin(), screenChangeListenerList_.end(),
            [screenChangeListener](IScreenChangeListener* listener) { return screenChangeListener == listener; }),
        screenChangeListenerList_.end());
}

sptr<DisplayInfo> ScreenSession::ConvertToDisplayInfo()
{
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    if (displayInfo == nullptr) {
        return displayInfo;
    }
    displayInfo->name_ = name_;
    displayInfo->SetWidth(property_.GetBounds().rect_.GetWidth());
    displayInfo->SetHeight(property_.GetBounds().rect_.GetHeight());
    displayInfo->SetPhysicalWidth(property_.GetPhyBounds().rect_.GetWidth());
    displayInfo->SetPhysicalHeight(property_.GetPhyBounds().rect_.GetHeight());
    displayInfo->SetScreenId(screenId_);
    displayInfo->SetDisplayId(screenId_);
    displayInfo->SetRefreshRate(property_.GetRefreshRate());
    displayInfo->SetVirtualPixelRatio(property_.GetVirtualPixelRatio());
    displayInfo->SetDensityInCurResolution(property_.GetDensityInCurResolution());
    displayInfo->SetDefaultVirtualPixelRatio(property_.GetDefaultDensity());
    displayInfo->SetXDpi(property_.GetXDpi());
    displayInfo->SetYDpi(property_.GetYDpi());
    displayInfo->SetDpi(property_.GetVirtualPixelRatio() * DOT_PER_INCH);
    displayInfo->SetRotation(property_.GetScreenRotation());
    displayInfo->SetOrientation(property_.GetOrientation());
    displayInfo->SetOffsetX(property_.GetOffsetX());
    displayInfo->SetOffsetY(property_.GetOffsetY());
    displayInfo->SetDisplayOrientation(property_.GetDisplayOrientation());
    displayInfo->SetHdrFormats(hdrFormats_);
    displayInfo->SetColorSpaces(colorSpaces_);
    displayInfo->SetDisplayState(property_.GetDisplayState());
    displayInfo->SetDefaultDeviceRotationOffset(property_.GetDefaultDeviceRotationOffset());
    displayInfo->SetAvailableWidth(property_.GetAvailableArea().width_);
    displayInfo->SetAvailableHeight(property_.GetAvailableArea().height_);
    displayInfo->SetScaleX(property_.GetScaleX());
    displayInfo->SetScaleY(property_.GetScaleY());
    displayInfo->SetPivotX(property_.GetPivotX());
    displayInfo->SetPivotY(property_.GetPivotY());
    displayInfo->SetTranslateX(property_.GetTranslateX());
    displayInfo->SetTranslateY(property_.GetTranslateY());
    return displayInfo;
}

DMError ScreenSession::GetScreenSupportedColorGamuts(std::vector<ScreenColorGamut>& colorGamuts)
{
    auto ret = RSInterfaces::GetInstance().GetScreenSupportedColorGamuts(rsId_, colorGamuts);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SCB: ScreenSession::GetScreenSupportedColorGamuts fail! rsId %{public}" PRIu64", ret:%{public}d",
            rsId_, ret);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SCB: ScreenSession::GetScreenSupportedColorGamuts ok! rsId %{public}" PRIu64", size %{public}u",
        rsId_, static_cast<uint32_t>(colorGamuts.size()));

    return DMError::DM_OK;
}

std::string ScreenSession::GetName()
{
    return name_;
}

void ScreenSession::SetName(std::string name)
{
    name_ = name;
}

ScreenId ScreenSession::GetScreenId()
{
    return screenId_;
}

ScreenId ScreenSession::GetRSScreenId()
{
    return rsId_;
}

ScreenProperty ScreenSession::GetScreenProperty() const
{
    return property_;
}

void ScreenSession::SetScreenScale(float scaleX, float scaleY, float pivotX, float pivotY, float translateX,
                                   float translateY)
{
    property_.SetScaleX(scaleX);
    property_.SetScaleY(scaleY);
    property_.SetPivotX(pivotX);
    property_.SetPivotY(pivotY);
    property_.SetTranslateX(translateX);
    property_.SetTranslateY(translateY);
}

void ScreenSession::SetDefaultDeviceRotationOffset(uint32_t defaultRotationOffset)
{
    WLOGFI("set device default rotation offset: %{public}u", defaultRotationOffset);
    property_.SetDefaultDeviceRotationOffset(defaultRotationOffset);
}

void ScreenSession::UpdatePropertyByActiveMode()
{
    sptr<SupportedScreenModes> mode = GetActiveScreenMode();
    if (mode != nullptr) {
        auto screeBounds = property_.GetBounds();
        screeBounds.rect_.width_ = mode->width_;
        screeBounds.rect_.height_ = mode->height_;
        property_.SetBounds(screeBounds);
    }
}

void ScreenSession::UpdatePropertyByFoldControl(const ScreenProperty& updatedProperty)
{
    property_.SetDpiPhyBounds(updatedProperty.GetPhyWidth(), updatedProperty.GetPhyHeight());
    property_.SetBounds(updatedProperty.GetBounds());
    property_.SetPhyBounds(updatedProperty.GetPhyBounds());
}

void ScreenSession::UpdateDisplayState(DisplayState displayState)
{
    property_.SetDisplayState(displayState);
}

void ScreenSession::UpdateRefreshRate(uint32_t refreshRate)
{
    property_.SetRefreshRate(refreshRate);
}

uint32_t ScreenSession::GetRefreshRate()
{
    return property_.GetRefreshRate();
}

void ScreenSession::UpdatePropertyByResolution(uint32_t width, uint32_t height)
{
    auto screenBounds = property_.GetBounds();
    screenBounds.rect_.width_ = width;
    screenBounds.rect_.height_ = height;
    property_.SetBounds(screenBounds);
}

std::shared_ptr<RSDisplayNode> ScreenSession::GetDisplayNode() const
{
    std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
    return displayNode_;
}

void ScreenSession::ReleaseDisplayNode()
{
    std::unique_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
    displayNode_ = nullptr;
    WLOGFI("displayNode_ is released.");
}

void ScreenSession::Connect()
{
    screenState_ = ScreenState::CONNECTION;
    if (screenChangeListenerList_.empty()) {
        WLOGFE("screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        listener->OnConnect(screenId_);
    }
}

void ScreenSession::Disconnect()
{
    screenState_ = ScreenState::DISCONNECTION;
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            continue;
        }
        listener->OnDisconnect(screenId_);
    }
}

void ScreenSession::PropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason)
{
    property_ = newProperty;
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            continue;
        }
        listener->OnPropertyChange(newProperty, reason, screenId_);
    }
}

void ScreenSession::PowerStatusChange(DisplayPowerEvent event, EventStatus status, PowerStateChangeReason reason)
{
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            continue;
        }
        listener->OnPowerStatusChange(event, status, reason);
    }
}

float ScreenSession::ConvertRotationToFloat(Rotation sensorRotation)
{
    float rotation = 0.f;
    switch (sensorRotation) {
        case Rotation::ROTATION_90:
            rotation = 90.f; // degree 90
            break;
        case Rotation::ROTATION_180:
            rotation = 180.f; // degree 180
            break;
        case Rotation::ROTATION_270:
            rotation = 270.f; // degree 270
            break;
        default:
            rotation = 0.f;
            break;
    }
    return rotation;
}

void ScreenSession::HandleSensorRotation(float sensorRotation)
{
    SensorRotationChange(sensorRotation);
}

void ScreenSession::SensorRotationChange(Rotation sensorRotation)
{
    float rotation = ConvertRotationToFloat(sensorRotation);
    SensorRotationChange(rotation);
}

void ScreenSession::SensorRotationChange(float sensorRotation)
{
    if (sensorRotation >= 0.0f) {
        currentSensorRotation_ = sensorRotation;
    }
    for (auto& listener : screenChangeListenerList_) {
        listener->OnSensorRotationChange(sensorRotation, screenId_);
    }
}

void ScreenSession::ScreenOrientationChange(Orientation orientation, FoldDisplayMode foldDisplayMode)
{
    Rotation rotationAfter = CalcRotation(orientation, foldDisplayMode);
    float screenRotation = ConvertRotationToFloat(rotationAfter);
    ScreenOrientationChange(screenRotation);
}

void ScreenSession::ScreenOrientationChange(float orientation)
{
    for (auto& listener : screenChangeListenerList_) {
        listener->OnScreenOrientationChange(orientation, screenId_);
    }
}

Rotation ScreenSession::ConvertIntToRotation(int rotation)
{
    Rotation targetRotation = Rotation::ROTATION_0;
    switch (rotation) {
        case 90: // Rotation 90 degree
            targetRotation = Rotation::ROTATION_90;
            break;
        case 180: // Rotation 180 degree
            targetRotation = Rotation::ROTATION_180;
            break;
        case 270: // Rotation 270 degree
            targetRotation = Rotation::ROTATION_270;
            break;
        default:
            targetRotation = Rotation::ROTATION_0;
            break;
    }
    return targetRotation;
}

void ScreenSession::SetUpdateToInputManagerCallback(std::function<void(float)> updateToInputManagerCallback)
{
    updateToInputManagerCallback_ = updateToInputManagerCallback;
}

void ScreenSession::SetUpdateScreenPivotCallback(std::function<void(float, float)>&& updateScreenPivotCallback)
{
    updateScreenPivotCallback_ = std::move(updateScreenPivotCallback);
}

VirtualScreenFlag ScreenSession::GetVirtualScreenFlag()
{
    return screenFlag_;
}

void ScreenSession::SetVirtualScreenFlag(VirtualScreenFlag screenFlag)
{
    screenFlag_ = screenFlag;
}

void ScreenSession::UpdateToInputManager(RRect bounds, int rotation, FoldDisplayMode foldDisplayMode)
{
    bool needUpdateToInputManager = false;
    if (foldDisplayMode == FoldDisplayMode::FULL && g_screenRotationOffSet == ROTATION_270 &&
        property_.GetBounds() == bounds && property_.GetRotation() != static_cast<float>(rotation)) {
        needUpdateToInputManager = true;
    }
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation displayOrientation = CalcDisplayOrientation(targetRotation, foldDisplayMode);
    property_.SetBounds(bounds);
    property_.SetRotation(static_cast<float>(rotation));
    property_.UpdateScreenRotation(targetRotation);
    property_.SetDisplayOrientation(displayOrientation);
    if (needUpdateToInputManager && updateToInputManagerCallback_ != nullptr) {
        // fold phone need fix 90 degree by remainder 360 degree
        int foldRotation = (rotation + 90) % 360;
        updateToInputManagerCallback_(static_cast<float>(foldRotation));
        WLOGFI("updateToInputManagerCallback_:%{public}d", foldRotation);
    }
}

void ScreenSession::UpdatePropertyAfterRotation(RRect bounds, int rotation, FoldDisplayMode foldDisplayMode)
{
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation displayOrientation = CalcDisplayOrientation(targetRotation, foldDisplayMode);
    property_.SetBounds(bounds);
    property_.SetRotation(static_cast<float>(rotation));
    property_.UpdateScreenRotation(targetRotation);
    property_.SetDisplayOrientation(displayOrientation);
    {
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        if (!displayNode_) {
            WLOGFI("update failed since null display node with rotation:%{public}d displayOrientation:%{public}u",
                rotation, displayOrientation);
            return;
        }
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->Begin();
        {
            std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
            displayNode_->SetScreenRotation(static_cast<uint32_t>(targetRotation));
        }
        transactionProxy->Commit();
    } else {
        {
            std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
            displayNode_->SetScreenRotation(static_cast<uint32_t>(targetRotation));
        }
    }
    WLOGFI("bounds:[%{public}f %{public}f %{public}f %{public}f],rotation:%{public}d,displayOrientation:%{public}u",
        property_.GetBounds().rect_.GetLeft(), property_.GetBounds().rect_.GetTop(),
        property_.GetBounds().rect_.GetWidth(), property_.GetBounds().rect_.GetHeight(),
        rotation, displayOrientation);
    ReportNotifyModeChange(displayOrientation);
}

void ScreenSession::UpdatePropertyOnly(RRect bounds, int rotation, FoldDisplayMode foldDisplayMode)
{
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation displayOrientation = CalcDisplayOrientation(targetRotation, foldDisplayMode);
    property_.SetBounds(bounds);
    property_.SetRotation(static_cast<float>(rotation));
    property_.UpdateScreenRotation(targetRotation);
    property_.SetDisplayOrientation(displayOrientation);
    WLOGFI("bounds:[%{public}f %{public}f %{public}f %{public}f],rotation:%{public}d,displayOrientation:%{public}u",
        property_.GetBounds().rect_.GetLeft(), property_.GetBounds().rect_.GetTop(),
        property_.GetBounds().rect_.GetWidth(), property_.GetBounds().rect_.GetHeight(),
        rotation, displayOrientation);
}

void ScreenSession::ReportNotifyModeChange(DisplayOrientation displayOrientation)
{
    int32_t vhMode = 1;
    if (displayOrientation == DisplayOrientation::PORTRAIT_INVERTED ||
        displayOrientation == DisplayOrientation::PORTRAIT) {
        vhMode = 0;
    }
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "VH_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "MODE", vhMode);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "ReportNotifyModeChange Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void ScreenSession::UpdateRotationAfterBoot(bool foldToExpand)
{
    TLOGI(WmsLogTag::DMS, "foldToExpand: %{public}d, Rotation: %{public}f",
        static_cast<int32_t>(foldToExpand), currentSensorRotation_);
    if (foldToExpand) {
        SensorRotationChange(currentSensorRotation_);
    }
}

sptr<SupportedScreenModes> ScreenSession::GetActiveScreenMode() const
{
    if (activeIdx_ < 0 || activeIdx_ >= static_cast<int32_t>(modes_.size())) {
        WLOGW("SCB: ScreenSession::GetActiveScreenMode active mode index is wrong: %{public}d", activeIdx_);
        return nullptr;
    }
    return modes_[activeIdx_];
}

Orientation ScreenSession::GetOrientation() const
{
    return property_.GetOrientation();
}

void ScreenSession::SetOrientation(Orientation orientation)
{
    property_.SetOrientation(orientation);
}

Rotation ScreenSession::GetRotation() const
{
    return property_.GetScreenRotation();
}

void ScreenSession::SetRotation(Rotation rotation)
{
    property_.SetScreenRotation(rotation);
}

void ScreenSession::SetRotationAndScreenRotationOnly(Rotation rotation)
{
    property_.SetRotationAndScreenRotationOnly(rotation);
}

void ScreenSession::SetScreenRequestedOrientation(Orientation orientation)
{
    property_.SetScreenRequestedOrientation(orientation);
}

void ScreenSession::SetScreenRotationLocked(bool isLocked)
{
    isScreenLocked_ = isLocked;
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            continue;
        }
        listener->OnScreenRotationLockedChange(isLocked, screenId_);
    }
}

void ScreenSession::SetScreenRotationLockedFromJs(bool isLocked)
{
    isScreenLocked_ = isLocked;
}

bool ScreenSession::IsScreenRotationLocked()
{
    return isScreenLocked_;
}

void ScreenSession::SetTouchEnabledFromJs(bool isTouchEnabled)
{
    TLOGI(WmsLogTag::WMS_EVENT, "isTouchEnabled:%{public}u", static_cast<uint32_t>(isTouchEnabled));
    touchEnabled_.store(isTouchEnabled);
}

bool ScreenSession::IsTouchEnabled()
{
    return touchEnabled_.load();
}

Orientation ScreenSession::GetScreenRequestedOrientation() const
{
    return property_.GetScreenRequestedOrientation();
}

void ScreenSession::SetVirtualPixelRatio(float virtualPixelRatio)
{
    property_.SetVirtualPixelRatio(virtualPixelRatio);
}

void ScreenSession::SetScreenSceneDpiChangeListener(const SetScreenSceneDpiFunc& func)
{
    SetScreenSceneDpiCallback_ = func;
    TLOGI(WmsLogTag::DMS, "SetScreenSceneDpiChangeListener");
}

void ScreenSession::SetScreenSceneDpi(float density)
{
    if (SetScreenSceneDpiCallback_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "SetScreenSceneDpiCallback_ is nullptr");
        return;
    }
    SetScreenSceneDpiCallback_(density);
}

void ScreenSession::SetScreenSceneDestroyListener(const DestroyScreenSceneFunc& func)
{
    destroyScreenSceneCallback_ = func;
    TLOGI(WmsLogTag::DMS, "SetScreenSceneDestroyListener");
}

void ScreenSession::DestroyScreenScene()
{
    if (destroyScreenSceneCallback_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "destroyScreenSceneCallback_ is nullptr");
        return;
    }
    destroyScreenSceneCallback_();
}

void ScreenSession::SetDensityInCurResolution(float densityInCurResolution)
{
    property_.SetDensityInCurResolution(densityInCurResolution);
}

void ScreenSession::SetScreenType(ScreenType type)
{
    property_.SetScreenType(type);
}

Rotation ScreenSession::CalcRotation(Orientation orientation, FoldDisplayMode foldDisplayMode) const
{
    sptr<SupportedScreenModes> info = GetActiveScreenMode();
    if (info == nullptr) {
        return Rotation::ROTATION_0;
    }
    // vertical: phone(Plugin screen); horizontal: pad & external screen
    bool isVerticalScreen = info->width_ < info->height_;
    if (foldDisplayMode != FoldDisplayMode::UNKNOWN &&
        (g_screenRotationOffSet == ROTATION_90 || g_screenRotationOffSet == ROTATION_270)) {
        isVerticalScreen = info->width_ > info->height_;
    }
    switch (orientation) {
        case Orientation::UNSPECIFIED: {
            return Rotation::ROTATION_0;
        }
        case Orientation::VERTICAL: {
            return isVerticalScreen ? Rotation::ROTATION_0 : Rotation::ROTATION_90;
        }
        case Orientation::HORIZONTAL: {
            return isVerticalScreen ? Rotation::ROTATION_90 : Rotation::ROTATION_0;
        }
        case Orientation::REVERSE_VERTICAL: {
            return isVerticalScreen ? Rotation::ROTATION_180 : Rotation::ROTATION_270;
        }
        case Orientation::REVERSE_HORIZONTAL: {
            return isVerticalScreen ? Rotation::ROTATION_270 : Rotation::ROTATION_180;
        }
        default: {
            WLOGE("unknown orientation %{public}u", orientation);
            return Rotation::ROTATION_0;
        }
    }
}

DisplayOrientation ScreenSession::CalcDisplayOrientation(Rotation rotation, FoldDisplayMode foldDisplayMode) const
{
    // vertical: phone(Plugin screen); horizontal: pad & external screen
    bool isVerticalScreen = property_.GetPhyWidth() < property_.GetPhyHeight();
    if (foldDisplayMode != FoldDisplayMode::UNKNOWN
        && (g_screenRotationOffSet == ROTATION_90 || g_screenRotationOffSet == ROTATION_270)) {
        WLOGD("foldDisplay is verticalScreen when width is greater than height");
        isVerticalScreen = property_.GetPhyWidth() > property_.GetPhyHeight();
    }
    switch (rotation) {
        case Rotation::ROTATION_0: {
            return isVerticalScreen ? DisplayOrientation::PORTRAIT : DisplayOrientation::LANDSCAPE;
        }
        case Rotation::ROTATION_90: {
            return isVerticalScreen ? DisplayOrientation::LANDSCAPE : DisplayOrientation::PORTRAIT;
        }
        case Rotation::ROTATION_180: {
            return isVerticalScreen ? DisplayOrientation::PORTRAIT_INVERTED : DisplayOrientation::LANDSCAPE_INVERTED;
        }
        case Rotation::ROTATION_270: {
            return isVerticalScreen ? DisplayOrientation::LANDSCAPE_INVERTED : DisplayOrientation::PORTRAIT_INVERTED;
        }
        default: {
            WLOGE("unknown rotation %{public}u", rotation);
            return DisplayOrientation::UNKNOWN;
        }
    }
}

ScreenSourceMode ScreenSession::GetSourceMode() const
{
    if (screenId_ == defaultScreenId_) {
        return ScreenSourceMode::SCREEN_MAIN;
    }
    ScreenCombination combination = GetScreenCombination();
    switch (combination) {
        case ScreenCombination::SCREEN_MIRROR: {
            return ScreenSourceMode::SCREEN_MIRROR;
        }
        case ScreenCombination::SCREEN_EXPAND: {
            return ScreenSourceMode::SCREEN_EXTEND;
        }
        case ScreenCombination::SCREEN_ALONE: {
            return ScreenSourceMode::SCREEN_ALONE;
        }
        case ScreenCombination::SCREEN_UNIQUE: {
            return ScreenSourceMode::SCREEN_UNIQUE;
        }
        default: {
            return ScreenSourceMode::SCREEN_ALONE;
        }
    }
}

void ScreenSession::SetScreenCombination(ScreenCombination combination)
{
    combination_ = combination;
}

ScreenCombination ScreenSession::GetScreenCombination() const
{
    return combination_;
}

void ScreenSession::FillScreenInfo(sptr<ScreenInfo> info) const
{
    if (info == nullptr) {
        WLOGE("FillScreenInfo failed! info is nullptr");
        return;
    }
    info->SetScreenId(screenId_);
    info->SetName(name_);
    uint32_t width = 0;
    uint32_t height = 0;
    sptr<SupportedScreenModes> screenSessionModes = GetActiveScreenMode();
    if (screenSessionModes != nullptr) {
        height = screenSessionModes->height_;
        width = screenSessionModes->width_;
    }
    float virtualPixelRatio = property_.GetVirtualPixelRatio();
    // "< 1e-set6" means virtualPixelRatio is 0.
    if (fabsf(virtualPixelRatio) < 1e-6) {
        virtualPixelRatio = 1.0f;
    }
    ScreenSourceMode sourceMode = GetSourceMode();
    info->SetVirtualPixelRatio(property_.GetVirtualPixelRatio());
    info->SetVirtualHeight(height / virtualPixelRatio);
    info->SetVirtualWidth(width / virtualPixelRatio);
    info->SetRotation(property_.GetScreenRotation());
    info->SetOrientation(static_cast<Orientation>(property_.GetDisplayOrientation()));
    info->SetSourceMode(sourceMode);
    info->SetType(property_.GetScreenType());
    info->SetModeId(activeIdx_);

    info->lastParent_ = lastGroupSmsId_;
    info->parent_ = groupSmsId_;
    info->isScreenGroup_ = isScreenGroup_;
    info->modes_ = modes_;
}

sptr<ScreenInfo> ScreenSession::ConvertToScreenInfo() const
{
    sptr<ScreenInfo> info = new(std::nothrow) ScreenInfo();
    if (info == nullptr) {
        return nullptr;
    }
    FillScreenInfo(info);
    return info;
}

DMError ScreenSession::GetScreenColorGamut(ScreenColorGamut& colorGamut)
{
    auto ret = RSInterfaces::GetInstance().GetScreenColorGamut(rsId_, colorGamut);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("GetScreenColorGamut fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("GetScreenColorGamut ok! rsId %{public}" PRIu64", colorGamut %{public}u",
        rsId_, static_cast<uint32_t>(colorGamut));
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenColorGamut(int32_t colorGamutIdx)
{
    std::vector<ScreenColorGamut> colorGamuts;
    DMError res = GetScreenSupportedColorGamuts(colorGamuts);
    if (res != DMError::DM_OK) {
        WLOGE("SetScreenColorGamut fail! rsId %{public}" PRIu64"", rsId_);
        return res;
    }
    if (colorGamutIdx < 0 || colorGamutIdx >= static_cast<int32_t>(colorGamuts.size())) {
        WLOGE("SetScreenColorGamut fail! rsId %{public}" PRIu64" colorGamutIdx %{public}d invalid.",
            rsId_, colorGamutIdx);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenColorGamut(rsId_, colorGamutIdx);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SetScreenColorGamut fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SetScreenColorGamut ok! rsId %{public}" PRIu64", colorGamutIdx %{public}u",
        rsId_, colorGamutIdx);
    return DMError::DM_OK;
}

DMError ScreenSession::GetScreenGamutMap(ScreenGamutMap& gamutMap)
{
    auto ret = RSInterfaces::GetInstance().GetScreenGamutMap(rsId_, gamutMap);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("GetScreenGamutMap fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("GetScreenGamutMap ok! rsId %{public}" PRIu64", gamutMap %{public}u",
        rsId_, static_cast<uint32_t>(gamutMap));
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenGamutMap(ScreenGamutMap gamutMap)
{
    if (gamutMap > GAMUT_MAP_HDR_EXTENSION) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenGamutMap(rsId_, gamutMap);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SetScreenGamutMap fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SetScreenGamutMap ok! rsId %{public}" PRIu64", gamutMap %{public}u",
        rsId_, static_cast<uint32_t>(gamutMap));
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenColorTransform()
{
    WLOGI("SetScreenColorTransform ok! rsId %{public}" PRIu64"", rsId_);
    return DMError::DM_OK;
}

DMError ScreenSession::GetPixelFormat(GraphicPixelFormat& pixelFormat)
{
    auto ret = RSInterfaces::GetInstance().GetPixelFormat(rsId_, pixelFormat);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("GetPixelFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("GetPixelFormat ok! rsId %{public}" PRIu64 ", pixelFormat %{public}u",
        rsId_, static_cast<uint32_t>(pixelFormat));
    return DMError::DM_OK;
}

DMError ScreenSession::SetPixelFormat(GraphicPixelFormat pixelFormat)
{
    if (pixelFormat > GRAPHIC_PIXEL_FMT_VENDER_MASK) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetPixelFormat(rsId_, pixelFormat);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SetPixelFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SetPixelFormat ok! rsId %{public}" PRIu64 ", gamutMap %{public}u",
        rsId_, static_cast<uint32_t>(pixelFormat));
    return DMError::DM_OK;
}

DMError ScreenSession::GetSupportedHDRFormats(std::vector<ScreenHDRFormat>& hdrFormats)
{
    auto ret = RSInterfaces::GetInstance().GetScreenSupportedHDRFormats(rsId_, hdrFormats);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SCB: ScreenSession::GetSupportedHDRFormats fail! rsId %{public}" PRIu64 ", ret:%{public}d",
            rsId_, ret);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SCB: ScreenSession::GetSupportedHDRFormats ok! rsId %{public}" PRIu64 ", size %{public}u",
        rsId_, static_cast<uint32_t>(hdrFormats.size()));

    return DMError::DM_OK;
}

DMError ScreenSession::GetScreenHDRFormat(ScreenHDRFormat& hdrFormat)
{
    auto ret = RSInterfaces::GetInstance().GetScreenHDRFormat(rsId_, hdrFormat);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("GetScreenHDRFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("GetScreenHDRFormat ok! rsId %{public}" PRIu64 ", colorSpace %{public}u",
        rsId_, static_cast<uint32_t>(hdrFormat));
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenHDRFormat(int32_t modeIdx)
{
    std::vector<ScreenHDRFormat> hdrFormats;
    DMError res = GetSupportedHDRFormats(hdrFormats);
    if (res != DMError::DM_OK) {
        WLOGE("SetScreenHDRFormat fail! rsId %{public}" PRIu64, rsId_);
        return res;
    }
    if (modeIdx < 0 || modeIdx >= static_cast<int32_t>(hdrFormats.size())) {
        WLOGE("SetScreenHDRFormat fail! rsId %{public}" PRIu64 " modeIdx %{public}d invalid.",
            rsId_, modeIdx);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenHDRFormat(rsId_, modeIdx);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SetScreenHDRFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SetScreenHDRFormat ok! rsId %{public}" PRIu64 ", modeIdx %{public}u",
        rsId_, modeIdx);
    return DMError::DM_OK;
}

DMError ScreenSession::GetSupportedColorSpaces(std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
    auto ret = RSInterfaces::GetInstance().GetScreenSupportedColorSpaces(rsId_, colorSpaces);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SCB: ScreenSession::GetSupportedColorSpaces fail! rsId %{public}" PRIu64 ", ret:%{public}d",
            rsId_, ret);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SCB: ScreenSession::GetSupportedColorSpaces ok! rsId %{public}" PRIu64 ", size %{public}u",
        rsId_, static_cast<uint32_t>(colorSpaces.size()));
    return DMError::DM_OK;
}

DMError ScreenSession::GetScreenColorSpace(GraphicCM_ColorSpaceType& colorSpace)
{
    auto ret = RSInterfaces::GetInstance().GetScreenColorSpace(rsId_, colorSpace);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("GetScreenColorSpace fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("GetScreenColorSpace ok! rsId %{public}" PRIu64 ", colorSpace %{public}u",
        rsId_, static_cast<uint32_t>(colorSpace));
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenColorSpace(GraphicCM_ColorSpaceType colorSpace)
{
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    DMError res = GetSupportedColorSpaces(colorSpaces);
    if (res != DMError::DM_OK) {
        WLOGE("SetScreenColorSpace fail! rsId %{public}" PRIu64, rsId_);
        return res;
    }
    if (colorSpace < 0 || static_cast<int32_t>(colorSpace) >= static_cast<int32_t>(colorSpaces.size())) {
        WLOGE("SetScreenColorSpace fail! rsId %{public}" PRIu64 " colorSpace %{public}d invalid.",
            rsId_, colorSpace);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenColorSpace(rsId_, colorSpace);
    if (ret != StatusCode::SUCCESS) {
        WLOGE("SetScreenColorSpace fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    WLOGI("SetScreenColorSpace ok! rsId %{public}" PRIu64 ", colorSpace %{public}u",
        rsId_, colorSpace);
    return DMError::DM_OK;
}

bool ScreenSession::HasPrivateSessionForeground() const
{
    return hasPrivateWindowForeground_;
}

void ScreenSession::SetPrivateSessionForeground(bool hasPrivate)
{
    hasPrivateWindowForeground_ = hasPrivate;
}

void ScreenSession::InitRSDisplayNode(RSDisplayNodeConfig& config, Point& startPoint)
{
    std::unique_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
    if (displayNode_ != nullptr) {
        displayNode_->SetDisplayNodeMirrorConfig(config);
        if (screenId_ == 0 && isFold_) {
            WLOGFI("Return InitRSDisplayNode foldScreen0");
            return;
        }
    } else {
        std::shared_ptr<RSDisplayNode> rsDisplayNode = RSDisplayNode::Create(config);
        if (rsDisplayNode == nullptr) {
            WLOGE("fail to add child. create rsDisplayNode fail!");
            return;
        }
        displayNode_ = rsDisplayNode;
    }
    WLOGFI("SetDisplayOffset: posX:%{public}d, posY:%{public}d", startPoint.posX_, startPoint.posY_);
    displayNode_->SetDisplayOffset(startPoint.posX_, startPoint.posY_);
    uint32_t width = 0;
    uint32_t height = 0;
    sptr<SupportedScreenModes> abstractScreenModes = GetActiveScreenMode();
    if (abstractScreenModes != nullptr) {
        height = abstractScreenModes->height_;
        width = abstractScreenModes->width_;
    }
    RSScreenType screenType;
    DmsXcollie dmsXcollie("DMS:InitRSDisplayNode:GetScreenType", XCOLLIE_TIMEOUT_5S);
    auto ret = RSInterfaces::GetInstance().GetScreenType(rsId_, screenType);
    if (ret == StatusCode::SUCCESS && screenType == RSScreenType::VIRTUAL_TYPE_SCREEN) {
        displayNode_->SetSecurityDisplay(true);
        WLOGFI("virtualScreen SetSecurityDisplay success");
    }
    // If setDisplayOffset is not valid for SetFrame/SetBounds
    WLOGFI("InitRSDisplayNode screenId:%{public}" PRIu64" width:%{public}u height:%{public}u",
        screenId_, width, height);
    displayNode_->SetFrame(0, 0, width, height);
    displayNode_->SetBounds(0, 0, width, height);
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
    }
}

ScreenSessionGroup::ScreenSessionGroup(ScreenId screenId, ScreenId rsId,
    std::string name, ScreenCombination combination) : combination_(combination)
{
    name_ = name;
    screenId_ = screenId;
    rsId_ = rsId;
    SetScreenType(ScreenType::UNDEFINED);
    isScreenGroup_ = true;
}

ScreenSessionGroup::~ScreenSessionGroup()
{
    ReleaseDisplayNode();
    screenSessionMap_.clear();
}

bool ScreenSessionGroup::GetRSDisplayNodeConfig(sptr<ScreenSession>& screenSession, struct RSDisplayNodeConfig& config,
                                                sptr<ScreenSession> defaultScreenSession)
{
    if (screenSession == nullptr) {
        WLOGE("screenSession is nullptr.");
        return false;
    }
    config = { screenSession->rsId_ };
    switch (combination_) {
        case ScreenCombination::SCREEN_ALONE:
            [[fallthrough]];
        case ScreenCombination::SCREEN_EXPAND:
            break;
        case ScreenCombination::SCREEN_UNIQUE:
            break;
        case ScreenCombination::SCREEN_MIRROR: {
            if (GetChildCount() == 0 || mirrorScreenId_ == screenSession->screenId_) {
                WLOGI("SCREEN_MIRROR, config is not mirror");
                break;
            }
            if (defaultScreenSession == nullptr) {
                WLOGFE("defaultScreenSession is nullptr");
                break;
            }
            std::shared_ptr<RSDisplayNode> displayNode = defaultScreenSession->GetDisplayNode();
            if (displayNode == nullptr) {
                WLOGFE("displayNode is nullptr, cannot get DisplayNode");
                break;
            }
            NodeId nodeId = displayNode->GetId();
            WLOGI("mirrorScreenId_:%{public}" PRIu64", rsId_:%{public}" PRIu64", nodeId:%{public}" PRIu64"",
                mirrorScreenId_, screenSession->rsId_, nodeId);
            config = {screenSession->rsId_, true, nodeId, true};
            break;
        }
        default:
            WLOGE("fail to add child. invalid group combination:%{public}u", combination_);
            return false;
    }
    return true;
}

bool ScreenSessionGroup::AddChild(sptr<ScreenSession>& smsScreen, Point& startPoint,
                                  sptr<ScreenSession> defaultScreenSession)
{
    if (smsScreen == nullptr) {
        WLOGE("AddChild, smsScreen is nullptr.");
        return false;
    }
    ScreenId screenId = smsScreen->screenId_;
    auto iter = screenSessionMap_.find(screenId);
    if (iter != screenSessionMap_.end()) {
        WLOGE("AddChild, screenSessionMap_ has smsScreen:%{public}" PRIu64"", screenId);
        return false;
    }
    struct RSDisplayNodeConfig config;
    if (!GetRSDisplayNodeConfig(smsScreen, config, defaultScreenSession)) {
        return false;
    }
    smsScreen->InitRSDisplayNode(config, startPoint);
    smsScreen->lastGroupSmsId_ = smsScreen->groupSmsId_;
    smsScreen->groupSmsId_ = screenId_;
    screenSessionMap_.insert(std::make_pair(screenId, std::make_pair(smsScreen, startPoint)));
    return true;
}

bool ScreenSessionGroup::AddChildren(std::vector<sptr<ScreenSession>>& smsScreens, std::vector<Point>& startPoints)
{
    size_t size = smsScreens.size();
    if (size != startPoints.size()) {
        WLOGE("AddChildren, unequal size.");
        return false;
    }
    bool res = true;
    for (size_t i = 0; i < size; i++) {
        res = AddChild(smsScreens[i], startPoints[i], nullptr) && res;
    }
    return res;
}

bool ScreenSessionGroup::RemoveChild(sptr<ScreenSession>& smsScreen)
{
    if (smsScreen == nullptr) {
        WLOGE("RemoveChild, smsScreen is nullptr.");
        return false;
    }
    ScreenId screenId = smsScreen->screenId_;
    smsScreen->lastGroupSmsId_ = smsScreen->groupSmsId_;
    smsScreen->groupSmsId_ = SCREEN_ID_INVALID;
    std::shared_ptr<RSDisplayNode> displayNode = smsScreen->GetDisplayNode();
    if (displayNode != nullptr) {
        displayNode->SetDisplayOffset(0, 0);
        displayNode->RemoveFromTree();
        smsScreen->ReleaseDisplayNode();
    }
    displayNode = nullptr;
    // attention: make sure reference count 0
    RSTransaction::FlushImplicitTransaction();
    return screenSessionMap_.erase(screenId);
}

bool ScreenSessionGroup::HasChild(ScreenId childScreen) const
{
    return screenSessionMap_.find(childScreen) != screenSessionMap_.end();
}

std::vector<sptr<ScreenSession>> ScreenSessionGroup::GetChildren() const
{
    std::vector<sptr<ScreenSession>> res;
    for (auto iter = screenSessionMap_.begin(); iter != screenSessionMap_.end(); iter++) {
        res.push_back(iter->second.first);
    }
    return res;
}

std::vector<Point> ScreenSessionGroup::GetChildrenPosition() const
{
    std::vector<Point> res;
    for (auto iter = screenSessionMap_.begin(); iter != screenSessionMap_.end(); iter++) {
        res.push_back(iter->second.second);
    }
    return res;
}

Point ScreenSessionGroup::GetChildPosition(ScreenId screenId) const
{
    Point point;
    auto iter = screenSessionMap_.find(screenId);
    if (iter != screenSessionMap_.end()) {
        point = iter->second.second;
    }
    return point;
}

size_t ScreenSessionGroup::GetChildCount() const
{
    return screenSessionMap_.size();
}

ScreenCombination ScreenSessionGroup::GetScreenCombination() const
{
    return combination_;
}

sptr<ScreenGroupInfo> ScreenSessionGroup::ConvertToScreenGroupInfo() const
{
    sptr<ScreenGroupInfo> screenGroupInfo = new(std::nothrow) ScreenGroupInfo();
    if (screenGroupInfo == nullptr) {
        return nullptr;
    }
    FillScreenInfo(screenGroupInfo);
    screenGroupInfo->combination_ = combination_;
    for (auto iter = screenSessionMap_.begin(); iter != screenSessionMap_.end(); iter++) {
        screenGroupInfo->children_.push_back(iter->first);
    }
    auto positions = GetChildrenPosition();
    screenGroupInfo->position_.insert(screenGroupInfo->position_.end(), positions.begin(), positions.end());
    return screenGroupInfo;
}

void ScreenSession::SetDisplayBoundary(const RectF& rect, const uint32_t& offsetY)
{
    property_.SetOffsetY(offsetY);
    property_.SetBounds(RRect(rect, 0.0f, 0.0f));
}

void ScreenSession::Resize(uint32_t width, uint32_t height)
{
    sptr<SupportedScreenModes> screenMode = GetActiveScreenMode();
    if (screenMode != nullptr) {
        screenMode->width_ = width;
        screenMode->height_ = height;
        UpdatePropertyByActiveMode();
        {
            std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
            displayNode_->SetFrame(0, 0, static_cast<float>(width), static_cast<float>(height));
            displayNode_->SetBounds(0, 0, static_cast<float>(width), static_cast<float>(height));
        }
        RSTransaction::FlushImplicitTransaction();
    }
}

bool ScreenSession::UpdateAvailableArea(DMRect area)
{
    if (property_.GetAvailableArea() == area) {
        return false;
    }
    property_.SetAvailableArea(area);
    return true;
}

void ScreenSession::SetAvailableArea(DMRect area)
{
    property_.SetAvailableArea(area);
}

DMRect ScreenSession::GetAvailableArea()
{
    return property_.GetAvailableArea();
}

void ScreenSession::SetFoldScreen(bool isFold)
{
    WLOGFI("SetFoldScreen %{public}u", isFold);
    isFold_ = isFold;
}

void ScreenSession::SetHdrFormats(std::vector<uint32_t>&& hdrFormats)
{
    hdrFormats_ = std::move(hdrFormats);
}

void ScreenSession::SetColorSpaces(std::vector<uint32_t>&& colorSpaces)
{
    colorSpaces_ = std::move(colorSpaces);
}

std::shared_ptr<Media::PixelMap> ScreenSession::GetScreenSnapshot(float scaleX, float scaleY)
{
    {
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        if (displayNode_ == nullptr) {
            WLOGFE("get screen snapshot displayNode_ is null");
            return nullptr;
        }
    }

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ss:GetScreenSnapshot");
    auto callback = std::make_shared<SurfaceCaptureFuture>();
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleX,
        .scaleY = scaleY,
    };
    {
        DmsXcollie dmsXcollie("DMS:GetScreenSnapshot:TakeSurfaceCapture", XCOLLIE_TIMEOUT_5S);
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        bool ret = RSInterfaces::GetInstance().TakeSurfaceCapture(displayNode_, callback, config);
        if (!ret) {
            WLOGFE("get screen snapshot TakeSurfaceCapture failed");
            return nullptr;
        }
    }

    auto pixelMap = callback->GetResult(2000);
    if (pixelMap != nullptr) {
        WLOGFD("save pixelMap WxH = %{public}dx%{public}d", pixelMap->GetWidth(), pixelMap->GetHeight());
    } else {
        WLOGFE("failed to get pixelMap, return nullptr");
    }
    return pixelMap;
}
} // namespace OHOS::Rosen
