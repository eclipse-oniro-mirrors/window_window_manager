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

#include "screen_cache.h"
#include <hitrace_meter.h>
#include <surface_capture_future.h>
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "dms_xcollie.h"
#include "fold_screen_state_internel.h"
#include <parameters.h>
#include "sys_cap_util.h"
#include <ipc_skeleton.h>

namespace OHOS::Rosen {
namespace {
static const int32_t g_screenRotationOffSet = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
// 0: 横扫屏; 1: 竖扫屏. 默认为0
static const int32_t g_screenScanType = system::GetIntParameter<int32_t>("const.window.screen.scan_type", 0);
static const int32_t SCAN_TYPE_VERTICAL = 1;
static const int32_t ROTATION_90 = 1;
static const int32_t ROTATION_270 = 3;
const unsigned int XCOLLIE_TIMEOUT_5S = 5;
const static uint32_t MAX_INTERVAL_US = 1800000000; //30分钟
const int32_t MAP_SIZE = 300;
const int32_t NO_EXIST_UID_VERSION = -1;
const float FULL_STATUS_WIDTH = 2048;
const float GLOBAL_FULL_STATUS_WIDTH = 3184;
const float MAIN_STATUS_WIDTH = 1008;
const float FULL_STATUS_OFFSET_X = 1136;
const float SCREEN_HEIGHT = 2232;
constexpr uint32_t SECONDARY_ROTATION_90 = 1;
constexpr uint32_t SECONDARY_ROTATION_270 = 3;
constexpr uint32_t SECONDARY_ROTATION_MOD = 4;
ScreenCache<int32_t, int32_t> g_uidVersionMap(MAP_SIZE, NO_EXIST_UID_VERSION);
}

ScreenSession::ScreenSession(const ScreenSessionConfig& config, ScreenSessionReason reason)
    : name_(config.name), screenId_(config.screenId), rsId_(config.rsId), defaultScreenId_(config.defaultScreenId),
    property_(config.property), displayNode_(config.displayNode), innerName_(config.innerName)
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
            TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "create screen session for client. noting to do.");
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
            TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "screen session no need create displayNode.");
            isNeedCreateDisplayNode = false;
            break;
        }
        default : {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "invalid screen session config.");
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
            if (config.isMirrored) {
                EnableMirrorScreenRegion();
            }
        } else {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Failed to create displayNode, displayNode is null!");
        }
    }
    RSTransaction::FlushImplicitTransaction();
}

void ScreenSession::ReuseDisplayNode(const RSDisplayNodeConfig& config)
{
    if (displayNode_) {
        displayNode_->SetDisplayNodeMirrorConfig(config);
        RSTransaction::FlushImplicitTransaction();
    } else {
        CreateDisplayNode(config);
    }
}

ScreenSession::~ScreenSession()
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "~ScreenSession");
}

ScreenSession::ScreenSession(ScreenId screenId, ScreenId rsId, const std::string& name,
    const ScreenProperty& property, const std::shared_ptr<RSDisplayNode>& displayNode)
    : name_(name), screenId_(screenId), rsId_(rsId), property_(property), displayNode_(displayNode)
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Success to create screenSession in constructor_0, "
        "screenid is %{public}" PRIu64"", screenId_);
}

ScreenSession::ScreenSession(ScreenId screenId, const ScreenProperty& property, ScreenId defaultScreenId)
    : screenId_(screenId), defaultScreenId_(defaultScreenId), property_(property)
{
    Rosen::RSDisplayNodeConfig config = { .screenId = screenId_ };
    displayNode_ = Rosen::RSDisplayNode::Create(config);
    if (displayNode_) {
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Success to create displayNode in constructor_1, "
            "screenid is %{public}" PRIu64"", screenId_);
        displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
    } else {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Failed to create displayNode, displayNode is null!");
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
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Success to create displayNode in constructor_2, "
            "screenid is %{public}" PRIu64"", screenId_);
        displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
    } else {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Failed to create displayNode, displayNode is null!");
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
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Success to create displayNode in constructor_3, "
            "rs id is %{public}" PRIu64"", rsId_);
        displayNode_->SetFrame(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
        displayNode_->SetBounds(property_.GetBounds().rect_.left_, property_.GetBounds().rect_.top_,
            property_.GetBounds().rect_.width_, property_.GetBounds().rect_.height_);
    } else {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Failed to create displayNode, displayNode is null!");
    }
    RSTransaction::FlushImplicitTransaction();
}

void ScreenSession::SetDisplayNodeScreenId(ScreenId screenId)
{
    std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
    if (displayNode_ != nullptr) {
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetDisplayNodeScreenId %{public}" PRIu64"", screenId);
        displayNode_->SetScreenId(screenId);
    }
}

void ScreenSession::RegisterScreenChangeListener(IScreenChangeListener* screenChangeListener)
{
    if (screenChangeListener == nullptr) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Failed to register screen change listener, listener is null!");
        return;
    }
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (std::find(screenChangeListenerList_.begin(), screenChangeListenerList_.end(), screenChangeListener) !=
        screenChangeListenerList_.end()) {
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Repeat to register screen change listener!");
        return;
    }

    screenChangeListenerList_.emplace_back(screenChangeListener);
    if (screenState_ == ScreenState::CONNECTION) {
        screenChangeListener->OnConnect(screenId_);
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Success to call onconnect callback.");
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Success to register screen change listener.");
}

void ScreenSession::UnregisterScreenChangeListener(IScreenChangeListener* screenChangeListener)
{
    if (screenChangeListener == nullptr) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Failed to unregister screen change listener, listener is null!");
        return;
    }
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    screenChangeListenerList_.erase(
        std::remove_if(screenChangeListenerList_.begin(), screenChangeListenerList_.end(),
            [screenChangeListener](IScreenChangeListener* listener) { return screenChangeListener == listener; }),
        screenChangeListenerList_.end());
}

void ScreenSession::SetMirrorScreenRegion(ScreenId screenId, DMRect screenRegion)
{
    std::lock_guard<std::mutex> lock(mirrorScreenRegionMutex_);
    mirrorScreenRegion_ = std::make_pair(screenId, screenRegion);
}

std::pair<ScreenId, DMRect> ScreenSession::GetMirrorScreenRegion()
{
    std::lock_guard<std::mutex> lock(mirrorScreenRegionMutex_);
    return mirrorScreenRegion_;
}

void ScreenSession::EnableMirrorScreenRegion()
{
    const auto& mirrorScreenRegionPair = GetMirrorScreenRegion();
    const auto& rect = mirrorScreenRegionPair.second;
    ScreenId screenId = INVALID_SCREEN_ID;
    if (isPhysicalMirrorSwitch_) {
        screenId = screenId_;
    } else {
        screenId = rsId_;
    }
    auto ret = RSInterfaces::GetInstance().SetMirrorScreenVisibleRect(screenId,
        { rect.posX_, rect.posY_, rect.width_, rect.height_ });
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Fail! rsId %{public}" PRIu64", ret:%{public}d," PRIu64
        ", x:%{public}d y:%{public}d w:%{public}u h:%{public}u", screenId, ret,
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "Success! rsId %{public}" PRIu64", ret:%{public}d," PRIu64
        ", x:%{public}d y:%{public}d w:%{public}u h:%{public}u", screenId, ret,
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    }
}

sptr<DisplayInfo> ScreenSession::ConvertToDisplayInfo()
{
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    if (displayInfo == nullptr) {
        return displayInfo;
    }
    RRect bounds = property_.GetBounds();
    RRect phyBounds = property_.GetPhyBounds();
    displayInfo->name_ = name_;
    displayInfo->SetWidth(bounds.rect_.GetWidth());
    if (isBScreenHalf_) {
        DMRect creaseRect = property_.GetCreaseRect();
        if (creaseRect.posY_ > 0) {
            displayInfo->SetHeight(creaseRect.posY_);
        } else {
            displayInfo->SetHeight(bounds.rect_.GetHeight() / HALF_SCREEN_PARAM);
        }
    } else {
        displayInfo->SetHeight(bounds.rect_.GetHeight());
    }
    displayInfo->SetPhysicalWidth(phyBounds.rect_.GetWidth());
    displayInfo->SetPhysicalHeight(phyBounds.rect_.GetHeight());
    displayInfo->SetScreenId(screenId_);
    displayInfo->SetDisplayId(screenId_);
    displayInfo->SetRefreshRate(property_.GetRefreshRate());
    displayInfo->SetVirtualPixelRatio(property_.GetVirtualPixelRatio());
    displayInfo->SetDensityInCurResolution(property_.GetDensityInCurResolution());
    displayInfo->SetDefaultVirtualPixelRatio(property_.GetDefaultDensity());
    displayInfo->SetXDpi(property_.GetXDpi());
    displayInfo->SetYDpi(property_.GetYDpi());
    displayInfo->SetDpi(property_.GetVirtualPixelRatio() * DOT_PER_INCH);
    int32_t apiVersion = GetApiVersion();
    if (apiVersion >= 14 || apiVersion == 0) { // 14 is API version
        displayInfo->SetRotation(property_.GetDeviceRotation());
        displayInfo->SetDisplayOrientation(property_.GetDeviceOrientation());
    } else {
        displayInfo->SetRotation(property_.GetScreenRotation());
        displayInfo->SetDisplayOrientation(property_.GetDisplayOrientation());
    }
    displayInfo->SetOrientation(property_.GetOrientation());
    displayInfo->SetOffsetX(property_.GetOffsetX());
    displayInfo->SetOffsetY(property_.GetOffsetY());
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
    displayInfo->SetScreenShape(property_.GetScreenShape());
    displayInfo->SetOriginRotation(property_.GetScreenRotation());
    return displayInfo;
}

sptr<DisplayInfo> ScreenSession::ConvertToRealDisplayInfo()
{
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    if (displayInfo == nullptr) {
        return displayInfo;
    }
    RRect bounds = property_.GetBounds();
    RRect phyBounds = property_.GetPhyBounds();
    displayInfo->name_ = name_;
    displayInfo->SetWidth(bounds.rect_.GetWidth());
    displayInfo->SetHeight(bounds.rect_.GetHeight());
    displayInfo->SetPhysicalWidth(phyBounds.rect_.GetWidth());
    displayInfo->SetPhysicalHeight(phyBounds.rect_.GetHeight());
    displayInfo->SetScreenId(screenId_);
    displayInfo->SetDisplayId(screenId_);
    displayInfo->SetRefreshRate(property_.GetRefreshRate());
    displayInfo->SetVirtualPixelRatio(property_.GetVirtualPixelRatio());
    displayInfo->SetDensityInCurResolution(property_.GetDensityInCurResolution());
    displayInfo->SetDefaultVirtualPixelRatio(property_.GetDefaultDensity());
    displayInfo->SetXDpi(property_.GetXDpi());
    displayInfo->SetYDpi(property_.GetYDpi());
    displayInfo->SetDpi(property_.GetVirtualPixelRatio() * DOT_PER_INCH);
    int32_t apiVersion = GetApiVersion();
    if (apiVersion >= 14 || apiVersion == 0) { // 14 is API version
        displayInfo->SetRotation(property_.GetDeviceRotation());
        displayInfo->SetDisplayOrientation(property_.GetDeviceOrientation());
    } else {
        displayInfo->SetRotation(property_.GetScreenRotation());
        displayInfo->SetDisplayOrientation(property_.GetDisplayOrientation());
    }
    displayInfo->SetOrientation(property_.GetOrientation());
    displayInfo->SetOffsetX(property_.GetOffsetX());
    displayInfo->SetOffsetY(property_.GetOffsetY());
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
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SCB: fail! rsId %{public}" PRIu64", ret:%{public}d",
            rsId_, ret);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SCB: ok! rsId %{public}" PRIu64", size %{public}u",
        rsId_, static_cast<uint32_t>(colorGamuts.size()));

    return DMError::DM_OK;
}

void ScreenSession::SetIsExtend(bool isExtend)
{
    isExtended_ = isExtend;
}

bool ScreenSession::GetIsExtend() const
{
    return isExtended_;
}

void ScreenSession::SetIsInternal(bool isInternal)
{
    isInternal_ = isInternal;
}

bool ScreenSession::GetIsInternal() const
{
    return isInternal_;
}

void ScreenSession::SetIsCurrentInUse(bool isInUse)
{
    isInUse_ = isInUse;
}

bool ScreenSession::GetIsCurrentInUse() const
{
    return isInUse_;
}

void ScreenSession::SetIsFakeInUse(bool isFakeInUse)
{
    isFakeInUse_ = isFakeInUse;
}

bool ScreenSession::GetIsFakeInUse() const
{
    return isFakeInUse_;
}

void ScreenSession::SetIsRealScreen(bool isReal)
{
    isReal_ = isReal;
}

bool ScreenSession::GetIsRealScreen()
{
    return isReal_;
}

void ScreenSession::SetIsPcUse(bool isPcUse)
{
    isPcUse_ = isPcUse;
}

bool ScreenSession::GetIsPcUse()
{
    return isPcUse_;
}

void ScreenSession::SetValidHeight(uint32_t validHeight)
{
    property_.SetValidHeight(validHeight);
}
 
void ScreenSession::SetValidWidth(uint32_t validWidth)
{
    property_.SetValidWidth(validWidth);
}
 
int32_t ScreenSession::GetValidHeight() const
{
    return property_.GetValidHeight();
}
 
int32_t ScreenSession::GetValidWidth() const
{
    return property_.GetValidWidth();
}

void ScreenSession::SetPointerActiveWidth(uint32_t pointerActiveWidth)
{
    property_.SetPointerActiveWidth(pointerActiveWidth);
}

uint32_t ScreenSession::GetPointerActiveWidth()
{
    return property_.GetPointerActiveWidth();
}

void ScreenSession::SetPointerActiveHeight(uint32_t pointerActiveHeight)
{
    property_.SetPointerActiveHeight(pointerActiveHeight);
}

uint32_t ScreenSession::GetPointerActiveHeight()
{
    return property_.GetPointerActiveHeight();
}

void ScreenSession::SetIsBScreenHalf(bool isBScreenHalf)
{
    isBScreenHalf_ = isBScreenHalf;
}

bool ScreenSession::GetIsBScreenHalf() const
{
    return isBScreenHalf_;
}

void ScreenSession::SetFakeScreenSession(sptr<ScreenSession> fakeScreenSession)
{
    fakeScreenSession_ = fakeScreenSession;
}

sptr<ScreenSession> ScreenSession::GetFakeScreenSession() const
{
    return fakeScreenSession_;
}

std::string ScreenSession::GetName()
{
    return name_;
}

void ScreenSession::SetName(std::string name)
{
    name_ = name;
}

std::string ScreenSession::GetInnerName()
{
    return innerName_;
}

void ScreenSession::SetInnerName(std::string innerName)
{
    innerName_ = innerName;
}

void ScreenSession::SetMirrorScreenType(MirrorScreenType mirrorType)
{
    mirrorScreenType_ = mirrorType;
}

MirrorScreenType ScreenSession::GetMirrorScreenType()
{
    return mirrorScreenType_;
}

ScreenId ScreenSession::GetScreenId()
{
    return screenId_;
}

ScreenId ScreenSession::GetRSScreenId()
{
    return rsId_;
}

ScreenShape ScreenSession::GetScreenShape() const
{
    return property_.GetScreenShape();
}

ScreenProperty ScreenSession::GetScreenProperty() const
{
    return property_;
}

void ScreenSession::SetSerialNumber(std::string serialNumber)
{
    serialNumber_ = serialNumber;
}

std::string ScreenSession::GetSerialNumber() const
{
    return serialNumber_;
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
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "set device default rotation offset: %{public}d", defaultRotationOffset);
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

ScreenProperty ScreenSession::UpdatePropertyByFoldControl(const ScreenProperty& updatedProperty,
    FoldDisplayMode foldDisplayMode)
{
    property_.SetDpiPhyBounds(updatedProperty.GetPhyWidth(), updatedProperty.GetPhyHeight());
    property_.SetPhyBounds(updatedProperty.GetPhyBounds());
    property_.SetBounds(updatedProperty.GetBounds());
    OptimizeSecondaryDisplayMode(updatedProperty.GetBounds(), foldDisplayMode);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        DisplayOrientation deviceOrientation =
            CalcDeviceOrientation(property_.GetScreenRotation(), foldDisplayMode, true);
        property_.SetDisplayOrientation(deviceOrientation);
        property_.SetDeviceOrientation(deviceOrientation);
    }
    UpdateTouchBoundsAndOffset();
    return property_;
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

void ScreenSession::UpdatePropertyByFakeBounds(uint32_t width, uint32_t height)
{
    auto screenFakeBounds = property_.GetFakeBounds();
    screenFakeBounds.rect_.width_ = width;
    screenFakeBounds.rect_.height_ = height;
    property_.SetFakeBounds(screenFakeBounds);
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
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "displayNode_ is released.");
}

void ScreenSession::Connect()
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    screenState_ = ScreenState::CONNECTION;
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnConnect(screenId_);
    }
}

void ScreenSession::Disconnect()
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    screenState_ = ScreenState::DISCONNECTION;
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnDisconnect(screenId_);
    }
}

void ScreenSession::PropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    property_ = newProperty;
    if (reason == ScreenPropertyChangeReason::VIRTUAL_PIXEL_RATIO_CHANGE) {
        return;
    }
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnPropertyChange(newProperty, reason, screenId_);
    }
}

void ScreenSession::PowerStatusChange(DisplayPowerEvent event, EventStatus status, PowerStateChangeReason reason)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
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
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (sensorRotation >= 0.0f) {
        currentValidSensorRotation_ = sensorRotation;
    }
    currentSensorRotation_ = sensorRotation;
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnSensorRotationChange(sensorRotation, screenId_);
    }
}

float ScreenSession::GetValidSensorRotation()
{
    return currentValidSensorRotation_;
}

void ScreenSession::HandleHoverStatusChange(int32_t hoverStatus, bool needRotate)
{
    HoverStatusChange(hoverStatus, needRotate);
}

void ScreenSession::HoverStatusChange(int32_t hoverStatus, bool needRotate)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnHoverStatusChange(hoverStatus, needRotate, screenId_);
    }
}

void ScreenSession::HandleCameraBackSelfieChange(bool isCameraBackSelfie)
{
    CameraBackSelfieChange(isCameraBackSelfie);
}

void ScreenSession::CameraBackSelfieChange(bool isCameraBackSelfie)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnCameraBackSelfieChange(isCameraBackSelfie, screenId_);
    }
}

void ScreenSession::ScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnScreenExtendChange(mainScreenId, extendScreenId);
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
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
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

void ScreenSession::UpdateTouchBoundsAndOffset()
{
    property_.SetPhysicalTouchBounds();
    property_.SetInputOffsetY();
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        property_.SetValidHeight(property_.GetBounds().rect_.GetHeight());
        property_.SetValidWidth(property_.GetBounds().rect_.GetWidth());
    }
}

void ScreenSession::UpdateToInputManager(RRect bounds, int rotation, int deviceRotation,
    FoldDisplayMode foldDisplayMode, bool isChanged)
{
    OptimizeSecondaryDisplayMode(bounds, foldDisplayMode);
    bool needUpdateToInputManager = false;
    if (foldDisplayMode == FoldDisplayMode::FULL &&
        property_.GetBounds() == bounds && property_.GetRotation() != static_cast<float>(rotation)) {
        needUpdateToInputManager = true;
    }
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation displayOrientation = CalcDisplayOrientation(targetRotation, foldDisplayMode, isChanged);
    property_.SetBounds(bounds);
    property_.SetRotation(static_cast<float>(rotation));
    property_.UpdateScreenRotation(targetRotation);
    property_.SetDisplayOrientation(displayOrientation);
    UpdateTouchBoundsAndOffset();
    Rotation targetDeviceRotation = ConvertIntToRotation(deviceRotation);
    DisplayOrientation deviceOrientation = CalcDeviceOrientation(targetDeviceRotation, foldDisplayMode, isChanged);
    property_.UpdateDeviceRotation(targetDeviceRotation);
    property_.SetDeviceOrientation(deviceOrientation);
    if (needUpdateToInputManager && updateToInputManagerCallback_ != nullptr
        && g_screenRotationOffSet == ROTATION_270) {
        // fold phone need fix 90 degree by remainder 360 degree
        int foldRotation = (rotation + 90) % 360;
        updateToInputManagerCallback_(static_cast<float>(foldRotation));
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "updateToInputManagerCallback_:%{public}d", foldRotation);
    }
}

void ScreenSession::SetPhysicalRotation(int rotation)
{
    property_.SetPhysicalRotation(static_cast<float>(rotation));
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "physicalrotation :%{public}f", property_.GetPhysicalRotation());
}

void ScreenSession::SetScreenComponentRotation(int rotation)
{
    property_.SetScreenComponentRotation(static_cast<float>(rotation));
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "screenComponentRotation :%{public}f ",
        property_.GetScreenComponentRotation());
}

void ScreenSession::OptimizeSecondaryDisplayMode(const RRect &bounds, FoldDisplayMode &foldDisplayMode)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        return;
    }
    if (IsWidthHeightMatch(bounds.rect_.GetWidth(), bounds.rect_.GetHeight(),
        MAIN_STATUS_WIDTH, SCREEN_HEIGHT)) {
        foldDisplayMode = FoldDisplayMode::MAIN;
    } else if (IsWidthHeightMatch(bounds.rect_.GetWidth(), bounds.rect_.GetHeight(),
        FULL_STATUS_WIDTH, SCREEN_HEIGHT)) {
        foldDisplayMode = FoldDisplayMode::FULL;
    } else if (IsWidthHeightMatch(bounds.rect_.GetWidth(), bounds.rect_.GetHeight(),
        GLOBAL_FULL_STATUS_WIDTH, SCREEN_HEIGHT)) {
        foldDisplayMode = FoldDisplayMode::GLOBAL_FULL;
    }
}

void ScreenSession::UpdatePropertyAfterRotation(RRect bounds, int rotation,
    FoldDisplayMode foldDisplayMode, bool isChanged)
{
    OptimizeSecondaryDisplayMode(bounds, foldDisplayMode);
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation displayOrientation = CalcDisplayOrientation(targetRotation, foldDisplayMode, isChanged);
    property_.SetBounds(bounds);
    property_.SetRotation(static_cast<float>(rotation));
    property_.UpdateScreenRotation(targetRotation);
    property_.SetDisplayOrientation(displayOrientation);
    if (!isBScreenHalf_ || property_.GetIsFakeInUse()) {
        property_.SetValidHeight(bounds.rect_.GetHeight());
        property_.SetValidWidth(bounds.rect_.GetWidth());
    }
    UpdateTouchBoundsAndOffset();
    {
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        if (!displayNode_) {
            TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "update failed since null display node with \
                rotation:%{public}d displayOrientation:%{public}u", rotation, displayOrientation);
            return;
        }
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->Begin();
        {
            std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
            displayNode_->SetScreenRotation(static_cast<uint32_t>(property_.GetDeviceRotation()));
        }
        transactionProxy->Commit();
    } else {
        {
            std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
            displayNode_->SetScreenRotation(static_cast<uint32_t>(property_.GetDeviceRotation()));
        }
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "bounds:[%{public}f %{public}f %{public}f %{public}f],rotation:%{public}d,\
        displayOrientation:%{public}u, foldDisplayMode:%{public}u",
        property_.GetBounds().rect_.GetLeft(), property_.GetBounds().rect_.GetTop(),
        property_.GetBounds().rect_.GetWidth(), property_.GetBounds().rect_.GetHeight(),
        rotation, displayOrientation, foldDisplayMode);
    ReportNotifyModeChange(displayOrientation);
}

void ScreenSession::UpdatePropertyOnly(RRect bounds, int rotation, FoldDisplayMode foldDisplayMode, bool isChanged)
{
    OptimizeSecondaryDisplayMode(bounds, foldDisplayMode);
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation displayOrientation = CalcDisplayOrientation(targetRotation, foldDisplayMode, isChanged);
    property_.SetBounds(bounds);
    property_.SetRotation(static_cast<float>(rotation));
    property_.UpdateScreenRotation(targetRotation);
    property_.SetDisplayOrientation(displayOrientation);
    if (!isBScreenHalf_ || property_.GetIsFakeInUse()) {
        property_.SetValidHeight(bounds.rect_.GetHeight());
        property_.SetValidWidth(bounds.rect_.GetWidth());
    }
    UpdateTouchBoundsAndOffset();
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "bounds:[%{public}f %{public}f %{public}f %{public}f],\
        rotation:%{public}d, displayOrientation:%{public}u",
        property_.GetBounds().rect_.GetLeft(), property_.GetBounds().rect_.GetTop(),
        property_.GetBounds().rect_.GetWidth(), property_.GetBounds().rect_.GetHeight(),
        rotation, displayOrientation);
}

void ScreenSession::UpdateRotationOrientation(int rotation, FoldDisplayMode foldDisplayMode, bool isChanged)
{
    OptimizeSecondaryDisplayMode(property_.GetBounds(), foldDisplayMode);
    Rotation targetRotation = ConvertIntToRotation(rotation);
    DisplayOrientation deviceOrientation = CalcDeviceOrientation(targetRotation, foldDisplayMode, isChanged);
    property_.UpdateDeviceRotation(targetRotation);
    property_.SetDeviceOrientation(deviceOrientation);
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "rotation:%{public}d, orientation:%{public}u", rotation, deviceOrientation);
}

void ScreenSession::UpdatePropertyByFakeInUse(bool isFakeInUse)
{
    property_.SetIsFakeInUse(isFakeInUse);
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
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "ReportNotifyModeChange Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void ScreenSession::UpdateRotationAfterBoot(bool foldToExpand)
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "foldToExpand: %{public}d, Rotation: %{public}f",
        static_cast<int32_t>(foldToExpand), currentSensorRotation_);
    if (foldToExpand) {
        SensorRotationChange(currentSensorRotation_);
    }
}

void ScreenSession::UpdateValidRotationToScb()
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Rotation: %{public}f", currentValidSensorRotation_);
    SensorRotationChange(currentValidSensorRotation_);
}

sptr<SupportedScreenModes> ScreenSession::GetActiveScreenMode() const
{
    if (activeIdx_ < 0 || activeIdx_ >= static_cast<int32_t>(modes_.size())) {
        TLOGW(WmsLogTag::DMS_SCREEN_SESSION, "SCB: active mode index is wrong: %{public}d", activeIdx_);
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
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    isScreenLocked_ = isLocked;
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
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
    TLOGI(WmsLogTag::WMS_EVENT, "%{public}u", isTouchEnabled);
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
    setScreenSceneDpiCallback_ = func;
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenSceneDpiChangeListener");
}

void ScreenSession::SetScreenSceneDpi(float density)
{
    if (setScreenSceneDpiCallback_ == nullptr) {
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "setScreenSceneDpiCallback_ is nullptr");
        return;
    }
    setScreenSceneDpiCallback_(density);
}

void ScreenSession::SetScreenSceneDestroyListener(const DestroyScreenSceneFunc& func)
{
    destroyScreenSceneCallback_  = func;
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenSceneDestroyListener");
}

void ScreenSession::DestroyScreenScene()
{
    if (destroyScreenSceneCallback_  == nullptr) {
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "destroyScreenSceneCallback_  is nullptr");
        return;
    }
    destroyScreenSceneCallback_();
}

void ScreenSession::SetDensityInCurResolution(float densityInCurResolution)
{
    property_.SetDensityInCurResolution(densityInCurResolution);
}

void ScreenSession::SetDefaultDensity(float defaultDensity)
{
    property_.SetDefaultDensity(defaultDensity);
}

void ScreenSession::UpdateVirtualPixelRatio(const RRect& bounds)
{
    property_.UpdateVirtualPixelRatio(bounds);
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
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "unknown orientation %{public}u", orientation);
            return Rotation::ROTATION_0;
        }
    }
}

DisplayOrientation ScreenSession::CalcDisplayOrientation(Rotation rotation,
    FoldDisplayMode foldDisplayMode, bool IsOrientationNeedChanged) const
{
    // vertical: phone(Plugin screen); horizontal: pad & external screen
    bool isVerticalScreen = property_.GetPhyWidth() < property_.GetPhyHeight();
    if (g_screenScanType == SCAN_TYPE_VERTICAL) {
        isVerticalScreen = false;
    }
    if (foldDisplayMode != FoldDisplayMode::UNKNOWN
        && (g_screenRotationOffSet == ROTATION_90 || g_screenRotationOffSet == ROTATION_270)) {
        TLOGD(WmsLogTag::DMS_SCREEN_SESSION, "foldDisplay is verticalScreen when width is greater than height");
        isVerticalScreen = property_.GetPhyWidth() > property_.GetPhyHeight();
    }
    if (IsOrientationNeedChanged) {
        if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
            uint32_t temp = (static_cast<uint32_t>(rotation) + SECONDARY_ROTATION_90) % SECONDARY_ROTATION_MOD;
            rotation = static_cast<Rotation>(temp);
        } else {
            uint32_t temp = (static_cast<uint32_t>(rotation) + SECONDARY_ROTATION_270) % SECONDARY_ROTATION_MOD;
            rotation = static_cast<Rotation>(temp);
        }
        isVerticalScreen = true;
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
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "unknown rotation %{public}u", rotation);
            return DisplayOrientation::UNKNOWN;
        }
    }
}

DisplayOrientation ScreenSession::CalcDeviceOrientation(Rotation rotation,
    FoldDisplayMode foldDisplayMode, bool IsOrientationNeedChanged) const
{
    if (IsOrientationNeedChanged) {
        if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
            uint32_t temp = (static_cast<uint32_t>(rotation) + SECONDARY_ROTATION_90) % SECONDARY_ROTATION_MOD;
            rotation = static_cast<Rotation>(temp);
        } else {
            uint32_t temp = (static_cast<uint32_t>(rotation) + SECONDARY_ROTATION_270) % SECONDARY_ROTATION_MOD;
            rotation = static_cast<Rotation>(temp);
        }
    }
    DisplayOrientation displayRotation = DisplayOrientation::UNKNOWN;
    switch (rotation) {
        case Rotation::ROTATION_0: {
            displayRotation = DisplayOrientation::PORTRAIT;
            break;
        }
        case Rotation::ROTATION_90: {
            displayRotation = DisplayOrientation::LANDSCAPE;
            break;
        }
        case Rotation::ROTATION_180: {
            displayRotation = DisplayOrientation::PORTRAIT_INVERTED;
            break;
        }
        case Rotation::ROTATION_270: {
            displayRotation = DisplayOrientation::LANDSCAPE_INVERTED;
            break;
        }
        default: {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "unknown rotation %{public}u", rotation);
        }
    }
    return displayRotation;
}

ScreenSourceMode ScreenSession::GetSourceMode() const
{
    if (!isPcUse_ && screenId_ == defaultScreenId_) {
        return ScreenSourceMode::SCREEN_MAIN;
    }
    ScreenCombination combination = GetScreenCombination();
    switch (combination) {
        case ScreenCombination::SCREEN_MAIN: {
            return ScreenSourceMode::SCREEN_MAIN;
        }
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
        case ScreenCombination::SCREEN_EXTEND: {
            return ScreenSourceMode::SCREEN_EXTEND;
        }
        default: {
            return ScreenSourceMode::SCREEN_ALONE;
        }
    }
}

void ScreenSession::SetScreenCombination(ScreenCombination combination)
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "screenId:%{public}" PRIu64", set combination:%{public}d", screenId_,
        static_cast<int32_t>(combination));
    combination_ = combination;
}

ScreenCombination ScreenSession::GetScreenCombination() const
{
    return combination_;
}

DisplaySourceMode ScreenSession::GetDisplaySourceMode() const
{
    if (!isPcUse_ && screenId_ == defaultScreenId_) {
        return DisplaySourceMode::MAIN;
    }
    ScreenCombination combination = GetScreenCombination();
    switch (combination) {
        case ScreenCombination::SCREEN_MAIN: {
            return DisplaySourceMode::MAIN;
        }
        case ScreenCombination::SCREEN_MIRROR: {
            return DisplaySourceMode::MIRROR;
        }
        case ScreenCombination::SCREEN_EXPAND: {
            return DisplaySourceMode::EXTEND;
        }
        case ScreenCombination::SCREEN_EXTEND: {
            return DisplaySourceMode::EXTEND;
        }
        case ScreenCombination::SCREEN_UNIQUE: {
            return DisplaySourceMode::ALONE;
        }
        case ScreenCombination::SCREEN_ALONE: {
            return DisplaySourceMode::NONE;
        }
        default: {
            return DisplaySourceMode::NONE;
        }
    }
}

void ScreenSession::FillScreenInfo(sptr<ScreenInfo> info) const
{
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "FillScreenInfo failed! info is nullptr");
        return;
    }
    info->SetScreenId(screenId_);
    info->SetName(name_);
    info->SetIsExtend(GetIsExtend());
    uint32_t width = 0;
    uint32_t height = 0;
    if (isPcUse_) {
        RRect bounds = property_.GetBounds();
        width = bounds.rect_.GetWidth();
        height = bounds.rect_.GetHeight();
    } else {
        sptr<SupportedScreenModes> screenSessionModes = GetActiveScreenMode();
        if (screenSessionModes != nullptr) {
            height = screenSessionModes->height_;
            width = screenSessionModes->width_;
        }
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
    info->SetSerialNumber(serialNumber_);

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
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    auto ret = RSInterfaces::GetInstance().GetScreenColorGamut(rsId_, colorGamut);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenColorGamut fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenColorGamut ok! rsId %{public}" PRIu64", colorGamut %{public}u",
        rsId_, static_cast<uint32_t>(colorGamut));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenColorGamut(int32_t colorGamutIdx)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    std::vector<ScreenColorGamut> colorGamuts;
    DMError res = GetScreenSupportedColorGamuts(colorGamuts);
    if (res != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorGamut fail! rsId %{public}" PRIu64"", rsId_);
        return res;
    }
    if (colorGamutIdx < 0 || colorGamutIdx >= static_cast<int32_t>(colorGamuts.size())) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorGamut fail! rsId %{public}" PRIu64" colorGamutIdx"
            " %{public}d invalid.", rsId_, colorGamutIdx);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenColorGamut(rsId_, colorGamutIdx);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorGamut fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorGamut ok! rsId %{public}" PRIu64", colorGamutIdx %{public}u",
        rsId_, colorGamutIdx);
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::GetScreenGamutMap(ScreenGamutMap& gamutMap)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    auto ret = RSInterfaces::GetInstance().GetScreenGamutMap(rsId_, gamutMap);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenGamutMap fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenGamutMap ok! rsId %{public}" PRIu64", gamutMap %{public}u",
        rsId_, static_cast<uint32_t>(gamutMap));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenGamutMap(ScreenGamutMap gamutMap)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    if (gamutMap > GAMUT_MAP_HDR_EXTENSION) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenGamutMap(rsId_, gamutMap);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenGamutMap fail! rsId %{public}" PRIu64"", rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenGamutMap ok! rsId %{public}" PRIu64", gamutMap %{public}u",
        rsId_, static_cast<uint32_t>(gamutMap));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenColorTransform()
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorTransform ok! rsId %{public}" PRIu64"", rsId_);
    return DMError::DM_OK;
}

DMError ScreenSession::GetPixelFormat(GraphicPixelFormat& pixelFormat)
{
    auto ret = RSInterfaces::GetInstance().GetPixelFormat(rsId_, pixelFormat);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "GetPixelFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "GetPixelFormat ok! rsId %{public}" PRIu64 ", pixelFormat %{public}u",
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
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetPixelFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetPixelFormat ok! rsId %{public}" PRIu64 ", gamutMap %{public}u",
        rsId_, static_cast<uint32_t>(pixelFormat));
    return DMError::DM_OK;
}

DMError ScreenSession::GetSupportedHDRFormats(std::vector<ScreenHDRFormat>& hdrFormats)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    auto ret = RSInterfaces::GetInstance().GetScreenSupportedHDRFormats(rsId_, hdrFormats);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SCB: fail! rsId %{public}" PRIu64 ", ret:%{public}d",
            rsId_, ret);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SCB: ok! rsId %{public}" PRIu64 ", size %{public}u",
        rsId_, static_cast<uint32_t>(hdrFormats.size()));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::GetScreenHDRFormat(ScreenHDRFormat& hdrFormat)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    auto ret = RSInterfaces::GetInstance().GetScreenHDRFormat(rsId_, hdrFormat);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenHDRFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenHDRFormat ok! rsId %{public}" PRIu64 ", colorSpace %{public}u",
        rsId_, static_cast<uint32_t>(hdrFormat));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenHDRFormat(int32_t modeIdx)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    std::vector<ScreenHDRFormat> hdrFormats;
    DMError res = GetSupportedHDRFormats(hdrFormats);
    if (res != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenHDRFormat fail! rsId %{public}" PRIu64, rsId_);
        return res;
    }
    if (modeIdx < 0 || modeIdx >= static_cast<int32_t>(hdrFormats.size())) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenHDRFormat fail! rsId %{public}" PRIu64 " "
            "modeIdx %{public}d invalid.", rsId_, modeIdx);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenHDRFormat(rsId_, modeIdx);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenHDRFormat fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenHDRFormat ok! rsId %{public}" PRIu64 ", modeIdx %{public}d",
        rsId_, modeIdx);
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::GetSupportedColorSpaces(std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    auto ret = RSInterfaces::GetInstance().GetScreenSupportedColorSpaces(rsId_, colorSpaces);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SCB: fail! rsId %{public}" PRIu64 ", ret:%{public}d",
            rsId_, ret);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SCB: ok! rsId %{public}" PRIu64 ", size %{public}u",
        rsId_, static_cast<uint32_t>(colorSpaces.size()));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::GetScreenColorSpace(GraphicCM_ColorSpaceType& colorSpace)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    auto ret = RSInterfaces::GetInstance().GetScreenColorSpace(rsId_, colorSpace);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenColorSpace fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "GetScreenColorSpace ok! rsId %{public}" PRIu64 ", colorSpace %{public}u",
        rsId_, static_cast<uint32_t>(colorSpace));
#endif
    return DMError::DM_OK;
}

DMError ScreenSession::SetScreenColorSpace(GraphicCM_ColorSpaceType colorSpace)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    DMError res = GetSupportedColorSpaces(colorSpaces);
    if (res != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorSpace fail! rsId %{public}" PRIu64, rsId_);
        return res;
    }
    if (colorSpace < 0) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorSpace fail! rsId %{public}" PRIu64 " "
            "colorSpace %{public}d invalid.", rsId_, static_cast<int32_t>(colorSpace));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto ret = RSInterfaces::GetInstance().SetScreenColorSpace(rsId_, colorSpace);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorSpace fail! rsId %{public}" PRIu64, rsId_);
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetScreenColorSpace ok! rsId %{public}" PRIu64 ", colorSpace %{public}u",
        rsId_, static_cast<uint32_t>(colorSpace));
#endif
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

void ScreenSession::InitRSDisplayNode(RSDisplayNodeConfig& config, Point& startPoint, bool isExtend)
{
    std::unique_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
    if (displayNode_ != nullptr) {
        displayNode_->SetDisplayNodeMirrorConfig(config);
        if (screenId_ == 0 && isFold_) {
            TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Return InitRSDisplayNode foldScreen0");
            return;
        }
    } else {
        std::shared_ptr<RSDisplayNode> rsDisplayNode = RSDisplayNode::Create(config);
        if (rsDisplayNode == nullptr) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "fail to add child. create rsDisplayNode fail!");
            return;
        }
        displayNode_ = rsDisplayNode;
    }
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetDisplayOffset: posX:%{public}d, posY:%{public}d", startPoint.posX_,
        startPoint.posY_);
    displayNode_->SetDisplayOffset(startPoint.posX_, startPoint.posY_);
    uint32_t width = 0;
    uint32_t height = 0;
    if (isExtend) {
        width = property_.GetBounds().rect_.GetWidth();
        height = property_.GetBounds().rect_.GetHeight();
    } else {
        sptr<SupportedScreenModes> abstractScreenModes = GetActiveScreenMode();
        if (abstractScreenModes != nullptr) {
            height = abstractScreenModes->height_;
            width = abstractScreenModes->width_;
        }
    }
    RSScreenType screenType;
    DmsXcollie dmsXcollie("DMS:InitRSDisplayNode:GetScreenType", XCOLLIE_TIMEOUT_5S);
    auto ret = RSInterfaces::GetInstance().GetScreenType(rsId_, screenType);
    if (ret == StatusCode::SUCCESS && screenType == RSScreenType::VIRTUAL_TYPE_SCREEN) {
        displayNode_->SetSecurityDisplay(true);
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "virtualScreen SetSecurityDisplay success");
    }
    // If setDisplayOffset is not valid for SetFrame/SetBounds
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "InitRSDisplayNode screenId:%{public}" PRIu64" width:%{public}u "
        "height:%{public}u", screenId_, width, height);
    displayNode_->SetFrame(0, 0, static_cast<float>(width), static_cast<float>(height));
    displayNode_->SetBounds(0, 0, static_cast<float>(width), static_cast<float>(height));
    if (config.isMirrored) {
        EnableMirrorScreenRegion();
    }
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
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenSession is nullptr.");
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
                TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SCREEN_MIRROR, config is not mirror");
                break;
            }
            if (defaultScreenSession == nullptr) {
                TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "defaultScreenSession is nullptr");
                break;
            }
            std::shared_ptr<RSDisplayNode> displayNode = defaultScreenSession->GetDisplayNode();
            if (displayNode == nullptr) {
                TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "displayNode is nullptr, cannot get DisplayNode");
                break;
            }
            NodeId nodeId = displayNode->GetId();
            TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "mirrorScreenId_:%{public}" PRIu64", rsId_:%{public}" PRIu64", \
                nodeId:%{public}" PRIu64"", mirrorScreenId_, screenSession->rsId_, nodeId);
            config = {screenSession->rsId_, true, nodeId, true};
            break;
        }
        default:
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "fail to add child. invalid group combination:%{public}u",
                combination_);
            return false;
    }
    return true;
}

bool ScreenSessionGroup::AddChild(sptr<ScreenSession>& smsScreen, Point& startPoint,
                                  sptr<ScreenSession> defaultScreenSession, bool isExtend)
{
    if (smsScreen == nullptr) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "AddChild, smsScreen is nullptr.");
        return false;
    }
    ScreenId screenId = smsScreen->screenId_;
    auto iter = screenSessionMap_.find(screenId);
    if (iter != screenSessionMap_.end()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "AddChild, screenSessionMap_ has smsScreen:%{public}" PRIu64"", screenId);
        return false;
    }
    struct RSDisplayNodeConfig config;
    if (!GetRSDisplayNodeConfig(smsScreen, config, defaultScreenSession)) {
        return false;
    }
    smsScreen->InitRSDisplayNode(config, startPoint, isExtend);
    smsScreen->lastGroupSmsId_ = smsScreen->groupSmsId_;
    smsScreen->groupSmsId_ = screenId_;
    screenSessionMap_.insert(std::make_pair(screenId, std::make_pair(smsScreen, startPoint)));
    return true;
}

bool ScreenSessionGroup::AddChildren(std::vector<sptr<ScreenSession>>& smsScreens, std::vector<Point>& startPoints)
{
    size_t size = smsScreens.size();
    if (size != startPoints.size()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "AddChildren, unequal size.");
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
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "RemoveChild, smsScreen is nullptr.");
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
    Point point{};
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
    property_.SetOffsetY(static_cast<int32_t>(offsetY));
    property_.SetBounds(RRect(rect, 0.0f, 0.0f));
}

void ScreenSession::SetExtendProperty(RRect bounds, bool isCurrentOffScreenRendering)
{
    property_.SetBounds(bounds);
    property_.SetCurrentOffScreenRendering(isCurrentOffScreenRendering);
}

void ScreenSession::Resize(uint32_t width, uint32_t height, bool isFreshBoundsSync)
{
    if (isFreshBoundsSync) {
        sptr<SupportedScreenModes> screenMode = GetActiveScreenMode();
        if (screenMode != nullptr) {
            screenMode->width_ = width;
            screenMode->height_ = height;
            UpdatePropertyByActiveMode();
        }
    }
    {
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        if (displayNode_ == nullptr) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "displayNode_ is null, resize failed");
            return;
        }
        displayNode_->SetFrame(0, 0, static_cast<float>(width), static_cast<float>(height));
        displayNode_->SetBounds(0, 0, static_cast<float>(width), static_cast<float>(height));
    }
    RSTransaction::FlushImplicitTransaction();
}

void ScreenSession::SetFrameGravity(Gravity gravity)
{
    {
        std::unique_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        if (displayNode_ == nullptr) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "displayNode_ is null, setFrameGravity failed");
            return;
        }
        displayNode_->SetFrameGravity(gravity);
    }
    RSTransaction::FlushImplicitTransaction();
}

bool ScreenSession::UpdateAvailableArea(DMRect area)
{
    if (property_.GetAvailableArea() == area) {
        return false;
    }
    property_.SetAvailableArea(area);
    return true;
}

bool ScreenSession::UpdateExpandAvailableArea(DMRect area)
{
    if (property_.GetExpandAvailableArea() == area) {
        return false;
    }
    property_.SetExpandAvailableArea(area);
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

DMRect ScreenSession::GetExpandAvailableArea()
{
    return property_.GetExpandAvailableArea();
}

void ScreenSession::SetFoldScreen(bool isFold)
{
    TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "SetFoldScreen %{public}u", isFold);
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

bool ScreenSession::IsWidthHeightMatch(float width, float height, float targetWidth, float targetHeight)
{
    return (width == targetWidth && height == targetHeight) || (width == targetHeight && height == targetWidth);
}

void ScreenSession::SetScreenSnapshotRect(RSSurfaceCaptureConfig& config)
{
    bool isChanged = false;
    auto width = property_.GetBounds().rect_.width_;
    auto height = property_.GetBounds().rect_.height_;
    Drawing::Rect snapshotRect = {0, 0, 0, 0};
    if (IsWidthHeightMatch(width, height, MAIN_STATUS_WIDTH, SCREEN_HEIGHT)) {
        snapshotRect = {0, 0, SCREEN_HEIGHT, MAIN_STATUS_WIDTH};
        config.mainScreenRect = snapshotRect;
        isChanged = true;
    } else if (IsWidthHeightMatch(width, height, FULL_STATUS_WIDTH, SCREEN_HEIGHT)) {
        snapshotRect = {0, FULL_STATUS_OFFSET_X, SCREEN_HEIGHT, GLOBAL_FULL_STATUS_WIDTH};
        config.mainScreenRect = snapshotRect;
        isChanged = true;
    } else if (IsWidthHeightMatch(width, height, GLOBAL_FULL_STATUS_WIDTH, SCREEN_HEIGHT)) {
        snapshotRect = {0, 0, SCREEN_HEIGHT, GLOBAL_FULL_STATUS_WIDTH};
        config.mainScreenRect = snapshotRect;
        isChanged = true;
    }
    if (isChanged) {
        TLOGI(WmsLogTag::DMS,
            "left: %{public}f, top: %{public}f, right: %{public}f, bottom: %{public}f",
            snapshotRect.left_, snapshotRect.top_, snapshotRect.right_, snapshotRect.bottom_);
    } else {
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "no need to set screen snapshot rect, use default rect");
    }
}

std::shared_ptr<Media::PixelMap> ScreenSession::GetScreenSnapshot(float scaleX, float scaleY)
{
    {
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        if (displayNode_ == nullptr) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "get screen snapshot displayNode_ is null");
            return nullptr;
        }
    }

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ss:GetScreenSnapshot");
    auto callback = std::make_shared<SurfaceCaptureFuture>();
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleX,
        .scaleY = scaleY,
        .useDma = true,
    };
    SetScreenSnapshotRect(config);
    {
        DmsXcollie dmsXcollie("DMS:GetScreenSnapshot:TakeSurfaceCapture", XCOLLIE_TIMEOUT_5S);
        std::shared_lock<std::shared_mutex> displayNodeLock(displayNodeMutex_);
        bool ret = RSInterfaces::GetInstance().TakeSurfaceCapture(displayNode_, callback, config);
        if (!ret) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "get screen snapshot TakeSurfaceCapture failed");
            return nullptr;
        }
    }

    auto pixelMap = callback->GetResult(2000); // 2000, default timeout
    if (pixelMap != nullptr) {
        TLOGD(WmsLogTag::DMS_SCREEN_SESSION, "save pixelMap WxH = %{public}dx%{public}d", pixelMap->GetWidth(),
            pixelMap->GetHeight());
    } else {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "failed to get pixelMap, return nullptr");
    }
    return pixelMap;
}

void ScreenSession::SetStartPosition(uint32_t startX, uint32_t startY)
{
    property_.SetStartPosition(startX, startY);
}

void ScreenSession::SetXYPosition(int32_t x, int32_t y)
{
    property_.SetXYPosition(x, y);
}

void ScreenSession::ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnScreenCaptureNotify(mainScreenId, uid, clientName);
    }
}

void ScreenSession::SuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnSuperFoldStatusChange(screenId, superFoldStatus);
    }
}

void ScreenSession::ExtendScreenConnectStatusChange(ScreenId screenId,
    ExtendScreenConnectStatus extendScreenConnectStatus)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnExtendScreenConnectStatusChange(screenId, extendScreenConnectStatus);
    }
}

void ScreenSession::SecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion)
{
    std::lock_guard<std::mutex> lock(screenChangeListenerListMutex_);
    if (screenChangeListenerList_.empty()) {
        TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListenerList is empty.");
        return;
    }
    for (auto& listener : screenChangeListenerList_) {
        if (!listener) {
            TLOGE(WmsLogTag::DMS_SCREEN_SESSION, "screenChangeListener is null.");
            continue;
        }
        listener->OnSecondaryReflexionChange(screenId, isSecondaryReflexion);
    }
}

void ScreenSession::SetIsPhysicalMirrorSwitch(bool isPhysicalMirrorSwitch)
{
    isPhysicalMirrorSwitch_ = isPhysicalMirrorSwitch;
}

bool ScreenSession::GetIsPhysicalMirrorSwitch()
{
    return isPhysicalMirrorSwitch_;
}

int32_t ScreenSession::GetApiVersion()
{
    static std::chrono::steady_clock::time_point lastRequestTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRequestTime).count();
    int32_t apiVersion = NO_EXIST_UID_VERSION;
    int32_t currentPid = IPCSkeleton::GetCallingPid();
    if (interval < MAX_INTERVAL_US) {
        apiVersion = g_uidVersionMap.Get(currentPid);
    }
    if (apiVersion == NO_EXIST_UID_VERSION) {
        apiVersion = static_cast<int32_t>(SysCapUtil::GetApiCompatibleVersion());
        TLOGI(WmsLogTag::DMS_SCREEN_SESSION, "Get version from IPC");
        g_uidVersionMap.Set(currentPid, apiVersion);
    }
    lastRequestTime = currentTime;
    return apiVersion;
}

void ScreenSession::SetShareProtect(bool needShareProtect)
{
    needShareProtect_ = needShareProtect;
}

bool ScreenSession::GetShareProtect()
{
    return needShareProtect_;
}

float ScreenSession::GetSensorRotation() const
{
    return currentSensorRotation_;
}
} // namespace OHOS::Rosen
