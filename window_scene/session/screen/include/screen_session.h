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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCREEN_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_SCREEN_SESSION_H

#include <mutex>
#include <vector>

#include <refbase.h>
#include <screen_manager/screen_types.h>
#include <shared_mutex>
#include <ui/rs_display_node.h>
#include <ui/rs_ui_director.h>

#include "screen_property.h"
#include "dm_common.h"
#include "display_info.h"
#include "screen.h"
#include "screen_info.h"
#include "screen_group.h"
#include "screen_group_info.h"
#include "event_handler.h"
#include "screen_session_manager/include/screen_rotation_property.h"
#include "screen_manager/rs_screen_mode_info.h"

namespace OHOS::Rosen {
using SetScreenSceneDpiFunc = std::function<void(float density)>;
using DestroyScreenSceneFunc = std::function<void()>;

class IScreenChangeListener {
public:
    virtual ~IScreenChangeListener() = default;
    virtual void OnConnect(ScreenId screenId) {}
    virtual void OnDisconnect(ScreenId screenId) {}
    virtual void OnPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
        ScreenId screenId) {}
    virtual void OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& newProperty,
            ScreenPropertyChangeReason reason, FoldDisplayMode mode) {}
    virtual void OnPowerStatusChange(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason) {}
    virtual void OnSensorRotationChange(float sensorRotation, ScreenId screenId, bool isSwitchUser) {}
    virtual void OnScreenOrientationChange(float screenOrientation, ScreenId screenId) {}
    virtual void OnScreenRotationLockedChange(bool isLocked, ScreenId screenId) {}
    virtual void OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId) {}
    virtual void OnHoverStatusChange(int32_t hoverStatus, bool needRotate, ScreenId extendScreenId) {}
    virtual void OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName) {}
    virtual void OnCameraBackSelfieChange(bool isCameraBackSelfie, ScreenId screenId) {}
    virtual void OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus) {}
    virtual void OnSecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion) {}
    virtual void OnExtendScreenConnectStatusChange(ScreenId screenId,
        ExtendScreenConnectStatus extendScreenConnectStatus) {}
    virtual void OnBeforeScreenPropertyChange(FoldStatus foldStatus) {}
    virtual void OnScreenModeChange(ScreenModeChangeEvent screenModeChangeEvent) {}
};

enum class MirrorScreenType : int32_t {
    PHYSICAL_MIRROR = 0,
    VIRTUAL_MIRROR = 1,
};

enum class ScreenState : int32_t {
    INIT,
    CONNECTION,
    DISCONNECTION,
};

struct ScreenSessionConfig {
    ScreenId screenId {0};
    ScreenId rsId {0};
    ScreenId defaultScreenId {0};
    ScreenId mirrorNodeId {0};
    std::string name = "UNKNOWN";
    std::string innerName = "UNKNOWN";
    ScreenProperty property;
    std::shared_ptr<RSDisplayNode> displayNode;
};

enum class ScreenSessionReason : int32_t {
    CREATE_SESSION_FOR_CLIENT,
    CREATE_SESSION_FOR_VIRTUAL,
    CREATE_SESSION_FOR_MIRROR,
    CREATE_SESSION_FOR_REAL,
    CREATE_SESSION_WITHOUT_DISPLAY_NODE,
    INVALID,
};

class ScreenSession : public RefBase {
public:
    ScreenSession() = default;
    ScreenSession(const ScreenSessionConfig& config, ScreenSessionReason reason);
    ScreenSession(ScreenId screenId, ScreenId rsId, const std::string& name,
        const ScreenProperty& property, const std::shared_ptr<RSDisplayNode>& displayNode);
    ScreenSession(ScreenId screenId, const ScreenProperty& property, ScreenId defaultScreenId);
    ScreenSession(ScreenId screenId, const ScreenProperty& property, NodeId nodeId, ScreenId defaultScreenId);
    ScreenSession(const std::string& name, ScreenId smsId, ScreenId rsId, ScreenId defaultScreenId);
    virtual ~ScreenSession();

    void CreateDisplayNode(const Rosen::RSDisplayNodeConfig& config);
    void ReuseDisplayNode(const RSDisplayNodeConfig& config);
    void SetDisplayNodeScreenId(ScreenId screenId);
    void RegisterScreenChangeListener(IScreenChangeListener* screenChangeListener);
    void UnregisterScreenChangeListener(IScreenChangeListener* screenChangeListener);
    void UpdateScbScreenPropertyToServer(const ScreenProperty& screenProperty);

    sptr<DisplayInfo> ConvertToDisplayInfo();
    sptr<DisplayInfo> ConvertToRealDisplayInfo();
    sptr<ScreenInfo> ConvertToScreenInfo() const;
    sptr<SupportedScreenModes> GetActiveScreenMode() const;
    ScreenSourceMode GetSourceMode() const;
    void SetScreenCombination(ScreenCombination combination);
    ScreenCombination GetScreenCombination() const;

    void SetBounds(RRect screenBounds);
    void SetHorizontalRotation();
    Orientation GetOrientation() const;
    void SetOrientation(Orientation orientation);
    Rotation GetRotation() const;
    void SetRotation(Rotation rotation);
    void SetRotationAndScreenRotationOnly(Rotation rotation);
    void SetOrientationMatchRotation(Rotation rotation, FoldDisplayMode displayMode);
    void SetScreenRequestedOrientation(Orientation orientation);
    Orientation GetScreenRequestedOrientation() const;
    void SetUpdateToInputManagerCallback(std::function<void(float)> updateToInputManagerCallback);
    void SetUpdateScreenPivotCallback(std::function<void(float, float)>&& updateScreenPivotCallback);

    void SetVirtualPixelRatio(float virtualPixelRatio);
    void SetScreenSceneDpiChangeListener(const SetScreenSceneDpiFunc& func);
    void SetScreenSceneDpi(float density);
    void SetDensityInCurResolution(float densityInCurResolution);
    float GetDensityInCurResolution();
    void SetDefaultDensity(float defaultDensity);
    void UpdateVirtualPixelRatio(const RRect& bounds);
    void SetScreenType(ScreenType type);

    void SetScreenSceneDestroyListener(const DestroyScreenSceneFunc& func);
    void DestroyScreenScene();

    void SetScreenScale(float scaleX, float scaleY, float pivotX, float pivotY, float translateX, float translateY);

    std::string GetName();
    std::string GetInnerName();
    ScreenId GetScreenId();
    ScreenId GetRSScreenId();
    ScreenProperty GetScreenProperty() const;
    void SetFakeScreenSession(sptr<ScreenSession> screenSession);
    sptr<ScreenSession> GetFakeScreenSession() const;
    void UpdatePropertyByActiveMode();
    void UpdatePropertyByScreenMode(RSScreenModeInfo screenMode);
    void UpdatePropertyByActiveModeChange();
    std::shared_ptr<RSDisplayNode> GetDisplayNode() const;
    void ReleaseDisplayNode();

    Rotation CalcRotation(Orientation orientation, FoldDisplayMode foldDisplayMode);
    DisplayOrientation CalcOrientationToDisplayOrientation(Orientation orientation);
    Rotation CalcRotationByDeviceOrientation(DisplayOrientation displayRotation,
        FoldDisplayMode foldDisplayMode, const RRect& boundsInRotationZero);
    Rotation CalcRotationSystemInner(Orientation orientation, FoldDisplayMode foldDisplayMode) const;
    DisplayOrientation CalcDisplayOrientation(Rotation rotation, FoldDisplayMode foldDisplayMode);
    DisplayOrientation CalcDeviceOrientation(Rotation rotation, FoldDisplayMode foldDisplayMode);
    DisplayOrientation CalcDeviceOrientationWithBounds(Rotation rotation,
        FoldDisplayMode foldDisplayMode, const RRect& bounds);
    RRect CalcBoundsInRotationZero(FoldDisplayMode foldDisplayMode);
    RRect CalcBoundsByRotation(Rotation rotation);
    DisplayOrientation GetTargetOrientationWithBounds(
        DisplayOrientation displayRotation, const RRect& boundsInRotationZero, uint32_t rotationOffset);
    void FillScreenInfo(sptr<ScreenInfo> info) const;
    void SetDisplayNodeSecurity();
    void InitRSDisplayNode(RSDisplayNodeConfig& config, Point& startPoint, bool isExtend = false,
        float positionX = 0, float positionY = 0);
    void ConvertBScreenHeight(uint32_t& height);

    DMError GetScreenSupportedColorGamuts(std::vector<ScreenColorGamut>& colorGamuts);
    DMError GetScreenColorGamut(ScreenColorGamut& colorGamut);
    DMError SetScreenColorGamut(int32_t colorGamutIdx);
    DMError GetScreenGamutMap(ScreenGamutMap& gamutMap);
    DMError SetScreenGamutMap(ScreenGamutMap gamutMap);
    DMError SetScreenColorTransform();

    DMError GetPixelFormat(GraphicPixelFormat& pixelFormat);
    DMError SetPixelFormat(GraphicPixelFormat pixelFormat);
    DMError GetSupportedHDRFormats(std::vector<ScreenHDRFormat>& hdrFormats);
    DMError GetScreenHDRFormat(ScreenHDRFormat& hdrFormat);
    DMError SetScreenHDRFormat(int32_t modeIdx);
    DMError GetSupportedColorSpaces(std::vector<GraphicCM_ColorSpaceType>& colorSpaces);
    DMError GetScreenColorSpace(GraphicCM_ColorSpaceType& colorSpace);
    DMError SetScreenColorSpace(GraphicCM_ColorSpaceType colorSpace);

    void HandleSensorRotation(float sensorRotation);
    void HandleHoverStatusChange(int32_t hoverStatus, bool needRotate = true);
    void HandleCameraBackSelfieChange(bool isCameraBackSelfie);
    float ConvertRotationToFloat(Rotation sensorRotation);

    void SetDisplayBoundary(const RectF& rect, const uint32_t& offsetY);
    void SetExtendProperty(RRect bounds, bool isCurrentOffScreenRendering);
    void SetScreenRotationLocked(bool isLocked);
    void SetScreenRotationLockedFromJs(bool isLocked);
    bool IsScreenRotationLocked();
    void SetTouchEnabledFromJs(bool isTouchEnabled);
    bool IsTouchEnabled();
    void SetIsPhysicalMirrorSwitch(bool isPhysicalMirrorSwitch);
    bool GetIsPhysicalMirrorSwitch();
    void UpdateTouchBoundsAndOffset(FoldDisplayMode foldDisplayMode);
    void UpdateToInputManager(RRect bounds, int rotation, int deviceRotation, FoldDisplayMode foldDisplayMode);
    void UpdatePropertyAfterRotation(RRect bounds, int rotation, FoldDisplayMode foldDisplayMode);
    void UpdatePropertyOnly(RRect bounds, int rotation, FoldDisplayMode foldDisplayMode);
    void UpdateRotationOrientation(int rotation, FoldDisplayMode foldDisplayMode, const RRect& bounds);
    void UpdatePropertyByFakeInUse(bool isFakeInUse);
    ScreenProperty UpdatePropertyByFoldControl(const ScreenProperty& updatedProperty,
        FoldDisplayMode foldDisplayMode = FoldDisplayMode::UNKNOWN, bool firstSCBConnect = false);
    void UpdateDisplayState(DisplayState displayState);
    void UpdateRefreshRate(uint32_t refreshRate);
    uint32_t GetRefreshRate();
    void UpdatePropertyByResolution(uint32_t width, uint32_t height);
    void UpdatePropertyByResolution(const DMRect& rect);
    void UpdatePropertyByFakeBounds(uint32_t width, uint32_t height);
    void SetName(std::string name);
    void SetInnerName(std::string innerName);
    void Resize(uint32_t width, uint32_t height, bool isFreshBoundsSync = true);
    void SetFrameGravity(Gravity gravity);

    void SetHdrFormats(std::vector<uint32_t>&& hdrFormats);
    std::vector<uint32_t> GetHdrFormats();
    void SetColorSpaces(std::vector<uint32_t>&& colorSpaces);
    std::vector<uint32_t> GetColorSpaces();
    void SetSupportedRefreshRate(std::vector<uint32_t>&& supportedRefreshRate);
    std::vector<uint32_t> GetSupportedRefreshRate() const;
    void SetForceCloseHdr(bool isForceCloseHdr);
    void SetBorderingAreaPercent(uint32_t borderingAreaPercent);
    uint32_t GetBorderingAreaPercent() const;

    VirtualScreenFlag GetVirtualScreenFlag();
    void SetVirtualScreenFlag(VirtualScreenFlag screenFlag);
    void SetSecurity(bool isSecurity);

    VirtualScreenType GetVirtualScreenType();
    void SetVirtualScreenType(VirtualScreenType screenType);

    std::string name_ { "UNKNOWN" };
    ScreenId screenId_ {};
    ScreenId rsId_ {};
    ScreenId defaultScreenId_ = SCREEN_ID_INVALID;
    ScreenId phyScreenId_ = SCREEN_ID_INVALID;

    void SetIsExtend(bool isExtend);
    bool GetIsExtend() const;
    void SetIsInternal(bool isInternal);
    void SetIsRealScreen(bool isReal);
    bool GetIsRealScreen();
    void SetIsPcUse(bool isPcUse);
    bool GetIsPcUse();
    void SetIsFakeSession(bool isFakeSession);
    bool GetIsInternal() const;
    void SetIsCurrentInUse(bool isInUse);
    bool GetIsCurrentInUse() const;
    void SetIsFakeInUse(bool isFakeInUse);
    bool GetIsFakeInUse() const;
    void SetIsBScreenHalf(bool isBScreenHalf);
    bool GetIsBScreenHalf() const;
    void SetIsExtendVirtual(bool isExtendVirtual);
    bool GetIsExtendVirtual() const;
    void SetSerialNumber(std::string serialNumber);
    std::string GetSerialNumber() const;
    ScreenShape GetScreenShape() const;
    void SetValidHeight(uint32_t validHeight);
    void SetValidWidth(uint32_t validWidth);
    uint32_t GetValidHeight() const;
    uint32_t GetValidWidth() const;
    float GetVirtualPixelRatio() const;
    void SetRealHeight(uint32_t realHeight) { property_.SetScreenRealHeight(realHeight); }
    void SetRealWidth(uint32_t realWidth) { property_.SetScreenRealWidth(realWidth); }

    void SetPointerActiveWidth(uint32_t pointerActiveWidth);
    uint32_t GetPointerActiveWidth();
    void SetPointerActiveHeight(uint32_t pointerActiveHeight);
    uint32_t GetPointerActiveHeight();
    float GetSensorRotation() const;
    DisplaySourceMode GetDisplaySourceMode() const;
    void SetXYPosition(int32_t x, int32_t y);

    bool isPrimary_ { false };
    bool isInternal_ { false };
    bool isExtended_ { false };
    bool isInUse_ { false };
    bool isReal_ { false };
    bool isPcUse_ { false };
    bool isFakeSession_ { false };

    NodeId nodeId_ {};

    int32_t activeIdx_ { 0 };
    uint32_t borderingAreaPercent_ { 0 };
    std::vector<sptr<SupportedScreenModes>> modes_ = {};

    bool isScreenGroup_ { false };
    ScreenId groupSmsId_ { SCREEN_ID_INVALID };
    ScreenId lastGroupSmsId_ { SCREEN_ID_INVALID };
    std::atomic<bool> isScreenLocked_ = true;

    void Connect();
    void Disconnect();
    void HandleKeyboardOnPropertyChange(ScreenProperty& screenProperty, int32_t height);
    void HandleKeyboardOffPropertyChange(ScreenProperty& screenProperty);
    void HandleSystemKeyboardOnPropertyChange(ScreenProperty& screenProperty,
        SuperFoldStatus currentStatus, bool isKeyboardOn, int32_t validHeight);
    void HandleSystemKeyboardOffPropertyChange(ScreenProperty& screenProperty,
        SuperFoldStatus currentStatus, bool isKeyboardOn);
    void HandleResolutionEffectPropertyChange(ScreenProperty& screenProperty, const ScreenProperty& eventPara);
    void ProcPropertyChange(ScreenProperty& screenProperty, const ScreenProperty& eventPara);
    void ProcPropertyChangedForSuperFold(ScreenProperty& screenProperty, const ScreenProperty& eventPara);
    void NotifyListenerPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason);
    void PropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason);
    void NotifyFoldPropertyChange(ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
        FoldDisplayMode displayMode);
    void UpdateSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents changeEvent);
    SuperFoldStatusChangeEvents GetSuperFoldStatusChangeEvent();
    void PowerStatusChange(DisplayPowerEvent event, EventStatus status, PowerStateChangeReason reason);
    // notify scb
    void SensorRotationChange(Rotation sensorRotation);
    void SensorRotationChange(float sensorRotation);
    void SensorRotationChange(float sensorRotation, bool isSwitchUser);
    float GetValidSensorRotation();
    void HoverStatusChange(int32_t hoverStatus, bool needRotate = true);
    void CameraBackSelfieChange(bool isCameraBackSelfie);
    void ScreenOrientationChange(Orientation orientation, FoldDisplayMode foldDisplayMode, bool isFromNapi);
    void ScreenOrientationChange(float orientation);
    void ScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId);
    DMRect GetAvailableArea();
    DMRect GetExpandAvailableArea();
    void SetAvailableArea(DMRect area);
    bool UpdateAvailableArea(DMRect area);
    bool UpdateExpandAvailableArea(DMRect area);
    void SetFoldScreen(bool isFold);
    void UpdateRotationAfterBoot(bool foldToExpand);
    void UpdateValidRotationToScb();
    std::shared_ptr<Media::PixelMap> GetScreenSnapshot(float scaleX, float scaleY);
    void SetDefaultDeviceRotationOffset(uint32_t defaultRotationOffset);

    void SetMirrorScreenType(MirrorScreenType mirrorType);
    MirrorScreenType GetMirrorScreenType();
    Rotation ConvertIntToRotation(int rotation);
    void SetPhysicalRotation(int rotation);
    void SetScreenComponentRotation(int rotation);
    void SetStartPosition(uint32_t startX, uint32_t startY);
    void SetMirrorScreenRegion(ScreenId screenId, DMRect screenRegion);
    std::pair<ScreenId, DMRect> GetMirrorScreenRegion();
    void ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName);
    void SuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus);
    void SecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion);
    void EnableMirrorScreenRegion();
    void ExtendScreenConnectStatusChange(ScreenId screenId, ExtendScreenConnectStatus extendScreenConnectStatus);
    void SetIsEnableRegionRotation(bool isEnableRegionRotation);
    bool GetIsEnableRegionRotation();
    void SetIsEnableCanvasRotation(bool isEnableCanvasRotation);
    bool GetIsEnableCanvasRotation();
    void UpdateDisplayNodeRotation(int rotation);
    void BeforeScreenPropertyChange(FoldStatus foldStatus);
    void ScreenModeChange(ScreenModeChangeEvent screenModeChangeEvent);
    void FreezeScreen(bool isFreeze);
    std::shared_ptr<Media::PixelMap> GetScreenSnapshotWithAllWindows(float scaleX, float scaleY,
        bool isNeedCheckDrmAndSurfaceLock);

    DisplayId GetDisplayId();

    std::vector<sptr<SupportedScreenModes>> GetScreenModes();
    void SetScreenModes(const std::vector<sptr<SupportedScreenModes>>& modes);

    int32_t GetActiveId();
    void SetActiveId(int32_t activeIdx);

    void SetScreenId(ScreenId screenId);
    void SetRSScreenId(ScreenId rsId);

    void SetDisplayNode(std::shared_ptr<RSDisplayNode> displayNode);
    void SetScreenOffScreenRendering();
    void SetScreenOffScreenRenderingInner();
    void SetScreenProperty(ScreenProperty property);

    void SetScreenAvailableStatus(bool isScreenAvailable);
    bool IsScreenAvailable() const;

    void SetIsAvailableAreaNeedNotify(bool isAvailableAreaNeedNotify);
    bool GetIsAvailableAreaNeedNotify() const;
    uint64_t GetSessionId() const;

    void SetRotationCorrectionMap(std::unordered_map<FoldDisplayMode, int32_t>& rotationCorrectionMap);
    std::unordered_map<FoldDisplayMode, int32_t> GetRotationCorrectionMap();
    Rotation GetRotationCorrection(FoldDisplayMode foldDisplayMode);
    void SetCurrentRotationCorrection(Rotation currentRotationCorrection);
    Rotation GetCurrentRotationCorrection() const;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> GetRSUIDirector() const;
    std::shared_ptr<RSUIContext> GetRSUIContext() const;

    /*
     * Display Group Info
     */
    void SetDisplayGroupId(DisplayGroupId displayGroupId);
    DisplayGroupId GetDisplayGroupId() const;
    void SetMainDisplayIdOfGroup(ScreenId screenId);
    ScreenId GetMainDisplayIdOfGroup() const;
    void SetScreenAreaOffsetX(uint32_t screenAreaOffsetX);
    uint32_t GetScreenAreaOffsetX() const;
    void SetScreenAreaOffsetY(uint32_t screenAreaOffsetY);
    uint32_t GetScreenAreaOffsetY() const;
    void SetScreenAreaWidth(uint32_t screenAreaWidth);
    uint32_t GetScreenAreaWidth() const;
    void SetScreenAreaHeight(uint32_t screenAreaHeight);
    uint32_t GetScreenAreaHeight() const;

    void UpdateMirrorWidth(uint32_t mirrorWidth);
    void UpdateMirrorHeight(uint32_t mirrorHeight);
    void SetCurrentValidHeight(int32_t currentValidHeight);
    int32_t GetCurrentValidHeight() const;
    void SetIsDestroyDisplay(bool isPreFakeInUse);
    bool GetIsDestroyDisplay() const;
    void SetIsKeyboardOn(bool isKeyboardOn);
    bool GetIsKeyboardOn() const;
    void SetFloatRotation(float rotation);
    void ModifyScreenPropertyWithLock(float rotation, RRect bounds);
    ScreenId GetPhyScreenId();
    void SetPhyScreenId(ScreenId screenId);
    bool GetSupportsFocus() const;
    void SetSupportsFocus(bool focus);
    bool GetSupportsInput() const;
    void SetSupportsInput(bool input);
    const std::string& GetBundleName() const;
    void SetBundleName(const std::string& bundleName);

    bool GetUniqueRotationLock() const;
    void SetUniqueRotationLock(bool isRotationLocked);
    int32_t GetUniqueRotation() const;
    void SetUniqueRotation(int32_t rotation);
    const std::map<int32_t, int32_t> GetUniqueRotationOrientationMap() const;
    bool UpdateRotationOrientationMap(UniqueScreenRotationOptions& rotationOptions, int32_t rotation,
                                            int32_t orientation);
    void SetUniqueRotationOrientationMap(const std::map<int32_t, int32_t>& rotationOrientationMap);

    void SetVprScaleRatio(float vprScaleRatio);
    float GetVprScaleRatio() const;
    void AddRotationCorrection(Rotation& rotation, FoldDisplayMode displayMode);

private:
    bool IsVertical(Rotation rotation) const;
    Orientation CalcDisplayOrientationToOrientation(DisplayOrientation displayOrientation) const;
    std::vector<IScreenChangeListener*> GetScreenChangeListenerList() const;
    void UpdateScbScreenPropertyForSuperFlod(const ScreenProperty& screenProperty);

    ScreenProperty property_;
    mutable std::mutex propertyMutex_; // above guarded by clientProxyMutex_
    std::shared_ptr<RSDisplayNode> displayNode_;
    ScreenState screenState_ { ScreenState::INIT };
    std::vector<IScreenChangeListener*> screenChangeListenerList_;
    mutable std::mutex screenChangeListenerListMutex_;
    ScreenCombination combination_ { ScreenCombination::SCREEN_ALONE };
    mutable std::mutex combinationMutex_; // above guarded by clientProxyMutex_
    VirtualScreenFlag screenFlag_ { VirtualScreenFlag::DEFAULT };
    VirtualScreenType screenType_ { VirtualScreenType::UNKNOWN };
    bool isFakeInUse_ = false;  // is fakeScreenSession can be used
    bool isBScreenHalf_ = false;
    bool isPhysicalMirrorSwitch_ = false;
    bool isScreenAvailable_ = true;
    bool isExtendVirtual_ {false};
    mutable std::shared_mutex displayNodeMutex_;
    std::atomic<bool> touchEnabled_ { true };
    std::function<void(float)> updateToInputManagerCallback_ = nullptr;
    std::function<void(float, float)> updateScreenPivotCallback_ = nullptr;
    bool isFold_ = false;
    float currentSensorRotation_ { -1.0f };
    float currentValidSensorRotation_ { -1.0f };
    mutable std::shared_mutex hdrFormatsMutex_;
    std::vector<uint32_t> hdrFormats_;
    mutable std::shared_mutex colorSpacesMutex_;
    std::vector<uint32_t> colorSpaces_;
    mutable std::shared_mutex supportedRefreshRateMutex_;
    std::vector<uint32_t> supportedRefreshRate_;
    MirrorScreenType mirrorScreenType_ { MirrorScreenType::VIRTUAL_MIRROR };
    std::string serialNumber_;
    std::pair<ScreenId, DMRect> mirrorScreenRegion_ = std::make_pair(INVALID_SCREEN_ID, DMRect::NONE());
    SetScreenSceneDpiFunc setScreenSceneDpiCallback_ = nullptr;
    DestroyScreenSceneFunc destroyScreenSceneCallback_ = nullptr;
    void ReportNotifyModeChange(DisplayOrientation displayOrientation);
    sptr<ScreenSession> fakeScreenSession_ = nullptr;
    int32_t GetApiVersion();
    void SetScreenSnapshotRect(RSSurfaceCaptureConfig& config);
    bool IsWidthHeightMatch(float width, float height, float targetWidth, float targetHeight);
    std::mutex mirrorScreenRegionMutex_;
    std::string innerName_ {"UNKOWN"};
    bool isEnableRegionRotation_ = false;
    bool isEnableCanvasRotation_ = false;
    std::mutex isEnableRegionRotationMutex_;
    std::mutex isEnableCanvasRotationMutex_;
    std::shared_mutex availableAreaMutex_;
    bool isAvailableAreaNeedNotify_ = false;
    bool isSecurity_ = true;
    uint64_t sessionId_;
    bool lastCloseHdrStatus_ = false;
    mutable std::shared_mutex modesMutex_;
    float vprScaleRatio_ { 1.0f };

    void RemoveRotationCorrection(Rotation& rotation, FoldDisplayMode foldDisplayMode);
    Rotation GetTargetRotationWithBounds(Rotation rotation, const RRect& bounds, uint32_t rotationOffset);
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap_;
    std::shared_mutex rotationCorrectionMutex_;
    std::atomic<Rotation> currentRotationCorrection_ { Rotation::ROTATION_0 };

    /*
     * Create Unique Screen Locked Rotation Parameters
     */
    bool isUniqueRotationLocked_ { false };
    int32_t uniqueRotation_ { 0 };
    mutable std::shared_mutex rotationMapMutex_;
    std::map<int32_t, int32_t> uniqueRotationOrientationMap_;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> rsUIDirector_;

    inline static std::atomic<uint64_t> sessionIdGenerator_ { 0 };
    std::atomic<bool> supportsFocus_ { true };
    std::atomic<bool> supportsInput_ { true };
    std::string bundleName_ = "";
};

class ScreenSessionGroup : public ScreenSession {
public:
    ScreenSessionGroup(ScreenId smsId, ScreenId rsId, std::string name, ScreenCombination combination);
    ScreenSessionGroup() = delete;
    WM_DISALLOW_COPY_AND_MOVE(ScreenSessionGroup);
    ~ScreenSessionGroup();

    bool AddChild(sptr<ScreenSession>& smsScreen, Point& startPoint);
    bool AddChild(sptr<ScreenSession>& smsScreen, Point& startPoint,
        sptr<ScreenSession> defaultScreenSession, bool isExtend = false,
        const RotationOption& rotationOption = {Rotation::ROTATION_0, false});
    bool AddChildren(std::vector<sptr<ScreenSession>>& smsScreens, std::vector<Point>& startPoints);
    bool RemoveChild(sptr<ScreenSession>& smsScreen);
    bool HasChild(ScreenId childScreen) const;
    std::vector<sptr<ScreenSession>> GetChildren() const;
    std::vector<Point> GetChildrenPosition() const;
    Point GetChildPosition(ScreenId screenId) const;
    size_t GetChildCount() const;
    sptr<ScreenGroupInfo> ConvertToScreenGroupInfo() const;
    ScreenCombination GetScreenCombination() const;

    ScreenCombination combination_ { ScreenCombination::SCREEN_ALONE };
    ScreenId mirrorScreenId_ { SCREEN_ID_INVALID };

private:
    bool GetRSDisplayNodeConfig(sptr<ScreenSession>& screenSession, struct RSDisplayNodeConfig& config,
        sptr<ScreenSession> defaultScreenSession);

    std::map<ScreenId, std::pair<sptr<ScreenSession>, Point>> screenSessionMap_;
    mutable std::shared_mutex screenSessionMapMutex_;
};

} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SCREEN_SESSION_H
