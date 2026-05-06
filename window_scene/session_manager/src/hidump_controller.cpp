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

#include "session_manager/include/hidump_controller.h"
#include "session_helper.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
constexpr int STRING_MAX_WIDTH = 21;

WM_IMPLEMENT_SINGLE_INSTANCE(HidumpController)

void HidumpController::GetAllSessionDumpDetailedInfo(std::ostringstream& oss,
    const std::vector<sptr<SceneSession>>& allSession, const std::vector<sptr<SceneSession>>& backgroundSession)
{
    oss << std::endl
        << "----------------------------------SessionDetailedInfo"
        << "-----------------------------------" << std::endl;
    DumpSceneSessionParamList(oss);
    uint32_t count = 0;
    for (const auto& session : allSession) {
        if (session == nullptr) {
            continue;
        }
        if (count >= static_cast<uint32_t>(allSession.size() - backgroundSession.size())) {
            break;
        }
        DumpSceneSessionParam(oss, session);
        count++;
    }
}

void HidumpController::DumpSceneSessionParamList(std::ostringstream& oss)
{
    DumpSessionParamList(oss);
    DumpLayoutRectParamList(oss);
    DumpLayoutParamList(oss);
    DumpAbilityParamList(oss);
    DumpKeyboardParamList(oss);
    DumpSysconfigParamList(oss);
    DumpLifeParamList(oss);
    DumpDisplayParamList(oss);
    DumpFocusParamList(oss);
    DumpInputParamList(oss);
    DumpLakeParamList(oss);
    DumpCOMParamList(oss);
    DumpVisibleParamList(oss);
}

// LCOV_EXCL_START
void HidumpController::DumpSceneSessionParam(std::ostringstream& oss, sptr<SceneSession> session)
{
    std::string sName = session->GetWindowNameAllType();
    const std::string& windowName = sName.size() <= STRING_MAX_WIDTH ?
        sName : sName.substr(0, STRING_MAX_WIDTH);
    oss << "----------------------------------"
        << windowName << "|"
        << session->GetPersistentId()
        << "----------------------------------" << std::endl;
    sptr<WindowSessionProperty> property = session->GetSessionProperty();
    if (property == nullptr) {
        oss << "property is nullptr" << std::endl << std::endl;
        return;
    }
    DumpSessionParam(oss, session, property);
    DumpLayoutRectParam(oss, session, property);
    DumpLayoutParam(oss, session, property);
    DumpAbilityParam(oss, session, property);
    DumpKeyboardParam(oss, session, property);
    DumpSysconfigParam(oss, session);
    DumpLifeParam(oss, session);
    DumpDisplayParam(oss, session, property);
    DumpFocusParam(oss, session, property);
    DumpInputParam(oss, session, property);
    DumpLakeParam(oss, session);
    DumpCOMParam(oss, session);
    DumpVisibleParam(oss, session);
    oss << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpSessionParamList(std::ostringstream& oss)
{
    oss << "Session:"
        << std::endl
        << "callingPid callingUid isSystem reuse lockedState time type isSystemCalling topmost"
        << std::endl
        << "isPrivacyMode isSystemPrivacyMode parentId flag parentPersistentId mode "
        << "state windowModeSupportType animationFlag"
        << std::endl
        << "isFloatingAppType isNonSystemFloating forceHide isNeedUpdateMode "
        << "meedDefaultAnimationFlag shouldHideNonSecure forceHideState"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpSessionParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    oss << "Session:"
        << std::endl
        << session->GetCallingPid() << "|"
        << session->GetCallingUid() << "|"
        << session->IsSystemSession() << "|"
        << session->sessionInfo_.reuse << "|"
        << session->sessionInfo_.lockedState << "|"
        << session->sessionInfo_.time << "|"
        << static_cast<uint32_t>(property->GetWindowType()) << "|"
        << property->GetSystemCalling() << "|"
        << property->IsTopmost() << "|"
        << std::endl
        << property->GetPrivacyMode() << "|"
        << property->GetSystemPrivacyMode() << "|"
        << property->GetParentId() << "|"
        << property->GetWindowFlags() << "|"
        << property->GetParentPersistentId() << "|"
        << static_cast<uint32_t>(property->GetWindowMode()) << "|"
        << static_cast<uint32_t>(property->GetWindowState()) << "|"
        << property->GetWindowModeSupportType() << "|"
        << property->GetAnimationFlag() << "|"
        << std::endl
        << property->IsFloatingWindowAppType() << "|"
        << property->GetHideNonSystemFloatingWindows() << "|"
        << property->GetForceHide() << "|"
        << property->GetIsNeedUpdateWindowMode() << "|"
        << session->IsNeedDefaultAnimation() << "|"
        << session->shouldHideNonSecureWindows_.load() << "|"
        << static_cast<uint32_t>(session->GetForceHideState()) << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpLayoutRectParamList(std::ostringstream& oss)
{
    oss << "LayoutRect:"
        << std::endl
        << "bounds requestRect windowRect"
        << std::endl
        << "limits userLimits configLimitsVP"
        << std::endl
        << "trans"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpLayoutRectParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    constexpr int precision = 1;
    WSRectF bounds = session->GetBounds();
    Rect requestRect = property->GetRequestRect();
    Rect windowRect = property->GetWindowRect();
    WindowLimits limits = property->GetWindowLimits();
    WindowLimits userLimits = property->GetUserWindowLimits();
    WindowLimits configLimitsVP = property->GetConfigWindowLimitsVP();
    Transform trans = property->GetTransform();
    oss << "LayoutRect:" << std::endl
        << "[" << std::setprecision(precision) << bounds.posX_ << " "
        << std::setprecision(precision) << bounds.posY_ << " "
        << std::setprecision(precision) << bounds.width_ << " "
        << std::setprecision(precision) << bounds.height_ << "]|"
        << "[" << requestRect.posX_ << " " << requestRect.posY_ << " "
        << requestRect.width_ << " " << requestRect.height_ << "]|"
        << "[" << windowRect.posX_ << windowRect.posY_ << " "
        << windowRect.width_ << " " << windowRect.height_ << "]|"
        << std::endl
        << "[" << limits.maxWidth_ << " " << limits.maxHeight_ << " "
        << limits.minWidth_ << " " << limits.minHeight_ << " "
        << std::setprecision(precision) << limits.maxRatio_ << " "
        << std::setprecision(precision) << limits.minRatio_ << " "
        << std::setprecision(precision) << limits.vpRatio_ << "]|"
        << "[" << userLimits.maxWidth_ << " " << userLimits.maxHeight_ << " "
        << userLimits.minWidth_ << " " << userLimits.minHeight_ << "]|"
        << std::endl
        << "[" << configLimitsVP.maxWidth_ << " " << configLimitsVP.maxHeight_ << " "
        << configLimitsVP.minWidth_ << " " << configLimitsVP.minHeight_ << "]|"
        << std::endl
        << "[" << std::setprecision(precision) << trans.pivotX_ << " "
        << std::setprecision(precision) << trans.pivotY_ << " "
        << std::setprecision(precision) << trans.scaleX_ << " "
        << std::setprecision(precision) << trans.scaleY_ << " "
        << std::setprecision(precision) << trans.scaleZ_ << " "
        << std::setprecision(precision) << trans.rotationX_ << " "
        << std::setprecision(precision) << trans.rotationY_ << " "
        << std::setprecision(precision) << trans.rotationZ_ << " "
        << std::setprecision(precision) << trans.translateX_ << " "
        << std::setprecision(precision) << trans.translateY_ << " "
        << std::setprecision(precision) << trans.translateZ_ << "]|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpLayoutParamList(std::ostringstream& oss)
{
    oss << "Layout:"
        << std::endl
        << "rotation reason zOrder aspectRatio floatingScale isDirty dragEnabled raiseEnabled"
        << std::endl
        << "requestedOrientation maximizeMode lastVpr isDecorEnable isLayoutFullScreen "
        << "customDecorHeight isTempShowWhenLocked"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpLayoutParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    constexpr int precision = 1;
    oss << "Layout:"
        << std::endl
        << static_cast<uint32_t>(session->GetRotation()) << "|"
        << static_cast<uint32_t>(session->GetSizeChangeReason()) << "|"
        << session->GetZOrder() << "|"
        << std::setprecision(precision) << session->GetAspectRatio() << "|"
        << std::setprecision(precision) << session->GetFloatingScale() << "|"
        << session->IsDirtyWindow() << "|"
        << session->IsDragAccessible() << "|"
        << property->GetDragEnabled() << "|"
        << property->GetRaiseEnabled() << "|"
        << std::endl
        << static_cast<uint32_t>(property->GetRequestedOrientation()) << "|"
        << static_cast<uint32_t>(property->GetMaximizeMode()) << "|"
        << std::setprecision(precision) << property->GetLastLimitsVpr() << "|"
        << property->IsDecorEnable() << "|"
        << property->IsLayoutFullScreen() << "|"
        << session->GetCustomDecorHeight() << "|"
        << session->IsTemporarilyShowWhenLocked() << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpAbilityParamList(std::ostringstream& oss)
{
    oss << "Ability:"
        << std::endl
        << "callingBundleName bundleName moduleName abilityName"
        << std::endl
        << "errorCode errorReason callerPersistentId callerBundleName callerAbilityName clientIdentityToken"
        << std::endl
        << "appIndex resultCode requestCode callState callingTokenId "
        << "continueState uiAbilityId tokenState accessTokenId callingSessionId"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpAbilityParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    oss << "Ability:"
        << std::endl
        << session->callingBundleName_ << "|"
        << session->sessionInfo_.bundleName_ << "|"
        << session->sessionInfo_.moduleName_ << "|"
        << session->sessionInfo_.abilityName_ << "|"
        << std::endl
        << session->sessionInfo_.errorCode << "|"
        << session->sessionInfo_.errorReason << "|"
        << session->sessionInfo_.callerPersistentId_ << "|"
        << session->sessionInfo_.callerBundleName_ << "|"
        << session->sessionInfo_.callerAbilityName_ << "|"
        << session->GetClientIdentityToken() << "|"
        << std::endl
        << session->sessionInfo_.appIndex_ << "|"
        << session->sessionInfo_.resultCode << "|"
        << session->sessionInfo_.requestCode << "|"
        << session->sessionInfo_.callState_ << "|"
        << session->sessionInfo_.callingTokenId_ << "|"
        << static_cast<uint32_t>(session->sessionInfo_.continueState) << "|"
        << session->sessionInfo_.uiAbilityId_ << "|"
        << property->GetTokenState() << "|"
        << property->GetAccessTokenId() << "|"
        << property->GetCallingSessionId() << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpKeyboardParamList(std::ostringstream& oss)
{
    oss << "Keyboard:"
        << std::endl
        << "scbKeepKeyboardFlag isSystemInput sessionGravity gravitySizePercent "
        << "keepKeyboardFlag textFieldPositionY textFieldHeight"
        << std::endl
        << "keyboardLayoutParams"
        << std::endl
        << "lastSafeRect oriPosYBeforeRaisedByKeyboard_"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpKeyboardParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    constexpr int precision = 1;
    WSRect lastSafeRect = session->GetLastSafeRect();
    int32_t oriPosYBeforeRaisedByKeyboard = session->GetOriPosYBeforeRaisedByKeyboard();
    KeyboardLayoutParams keyboardLayoutParams = property->GetKeyboardLayoutParams();
    Rect LandscapeKeyboardRect = keyboardLayoutParams.LandscapeKeyboardRect_;
    Rect PortraitKeyboardRect = keyboardLayoutParams.PortraitKeyboardRect_;
    Rect LandscapePanelRect = keyboardLayoutParams.LandscapePanelRect_;
    Rect PortraitPanelRect = keyboardLayoutParams.PortraitPanelRect_;
    oss << "Keyboard:"
        << std::endl
        << session->GetSCBKeepKeyboardFlag() << "|"
        << session->IsSystemInput() << "|"
        << 0 << "|"
        << 0 << "|"
        << property->GetKeepKeyboardFlag() << "|"
        << std::setprecision(precision) << property->GetTextFieldPositionY() << "|"
        << std::setprecision(precision) << property->GetTextFieldHeight() << "|"
        << std::endl
        << "{" << static_cast<uint32_t>(keyboardLayoutParams.gravity_) << "|"
        << keyboardLayoutParams.landscapeAvoidHeight_ << "|"
        << keyboardLayoutParams.portraitAvoidHeight_ << "|"
        << "[" << LandscapeKeyboardRect.posX_ << " "
        << LandscapeKeyboardRect.posY_ << " "
        << LandscapeKeyboardRect.width_ << " "
        << LandscapeKeyboardRect.height_ << "]|"
        << "[" << PortraitKeyboardRect.posX_ << " "
        << PortraitKeyboardRect.posY_ << " "
        << PortraitKeyboardRect.width_ << " "
        << PortraitKeyboardRect.height_ << "]|"
        << "[" << LandscapePanelRect.posX_ << " "
        << LandscapePanelRect.posY_ << " "
        << LandscapePanelRect.width_ << " "
        << LandscapePanelRect.height_ << "]|"
        << "[" << PortraitPanelRect.posX_ << " "
        << PortraitPanelRect.posY_ << " "
        << PortraitPanelRect.width_ << " "
        << PortraitPanelRect.height_ << "]}"
        << std::endl
        << "[" << lastSafeRect.posX_ << " "
        << lastSafeRect.posY_ << " "
        << lastSafeRect.width_ << " "
        << lastSafeRect.height_ << "]|"
        << "[" << oriPosYBeforeRaisedByKeyboard << "]|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpSysconfigParamList(std::ostringstream& oss)
{
    oss << "Sysconfig:"
        << std::endl
        << "isSystemDecorEnable decorWindowModeSupportType isStretchable defaultWindowMode "
        << "keyboardAnimationConfig maxFloatingWindowSize windowUIType"
        << std::endl
        << "miniWidthOfMainWindow miniHeightOfMainWindow miniWidthOfSubWindow miniHeightOfSubWindow backgroundswitch "
        << "freeMultiWindowEnable freeMultiWindowSupport supportTypeFloatWindow freeMultiWindowConfig_"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpSysconfigParam(std::ostringstream& oss, sptr<SceneSession> session)
{
    SystemSessionConfig systemConfig = session->GetSystemConfig();
    FreeMultiWindowConfig freeMultiWindowConfig = systemConfig.freeMultiWindowConfig_;
    oss << "Sysconfig:"
        << std::endl
        << systemConfig.isSystemDecorEnable_ << "|"
        << systemConfig.decorWindowModeSupportType_ << "|"
        << systemConfig.isStretchable_ << "|"
        << static_cast<uint32_t>(systemConfig.defaultWindowMode_) << "|"
        << "[" << systemConfig.animationIn_.curveType_ << " "
        << systemConfig.animationIn_.duration_ << "]|"
        << "[" << systemConfig.animationOut_.curveType_ << " "
        << systemConfig.animationOut_.duration_ << "]|"
        << systemConfig.maxFloatingWindowSize_ << "|"
        << static_cast<uint8_t>(systemConfig.windowUIType_) << "|"
        << std::endl
        << systemConfig.miniWidthOfMainWindow_ << "|"
        << systemConfig.miniHeightOfMainWindow_ << "|"
        << systemConfig.miniWidthOfSubWindow_ << "|"
        << systemConfig.miniHeightOfSubWindow_ << "|"
        << systemConfig.miniWidthOfDialogWindow_ << "|"
        << systemConfig.miniHeightOfDialogWindow_ << "|"
        << systemConfig.backgroundswitch << "|"
        << systemConfig.freeMultiWindowEnable_ << "|"
        << systemConfig.freeMultiWindowSupport_ << "|"
        << systemConfig.supportTypeFloatWindow_ << "|"
        <<  "[" << freeMultiWindowConfig.isSystemDecorEnable_ << " "
        << freeMultiWindowConfig.decorWindowModeSupportType_ << " "
        << static_cast<uint32_t>(freeMultiWindowConfig.defaultWindowMode_) << " "
        << freeMultiWindowConfig.maxMainFloatingWindowNumber_ << " "
        << static_cast<uint32_t>(freeMultiWindowConfig.defaultDragResizeType_) << "]|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpLifeParamList(std::ostringstream& oss)
{
    oss << "Life: "
        << "state isActive isSystemActive isVisible uiNodeId showRecent bufferAvailable "
        << "foregroundInteractiveStatus isAttach isPersistentRecover"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpLifeParam(std::ostringstream& oss, sptr<SceneSession> session)
{
    oss << "Life: "
        << static_cast<uint32_t>(session->GetSessionState()) << "|"
        << session->IsActive() << "|"
        << session->IsSystemActive() << "|"
        << session->IsVisible() << "|"
        << session->GetUINodeId() << "|"
        << session->GetShowRecent() << "|"
        << session->GetBufferAvailable() << "|"
        << session->GetForegroundInteractiveStatus() << "|"
        << session->GetAttachState() << "|"
        << session->sessionInfo_.isPersistentRecover_ << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpDisplayParamList(std::ostringstream& oss)
{
    oss << "Display: "
        << "snapshotScale vpr screenId brightness displayId"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpDisplayParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    constexpr int precision = 1;
    oss << "Display: "
        << std::setprecision(precision) << session->snapshotScale_ << "|"
        << std::setprecision(precision) << session->vpr_ << "|"
        << session->sessionInfo_.screenId_ << "|"
        << std::setprecision(precision) << property->GetBrightness() << "|"
        << property->GetDisplayId() << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpFocusParamList(std::ostringstream& oss)
{
    oss << "Focus: "
        << "isFocused blockingFocus focusedOnShow focusable"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpFocusParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    oss << "Focus: "
        << session->IsFocused() << "|"
        << session->GetBlockingFocus() << "|"
        << session->IsFocusedOnShow() << "|"
        << property->GetFocusable() << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpInputParamList(std::ostringstream& oss)
{
    oss << "Input: "
        << "forceTouchable systemTouchable isSetPointerAreas touchable"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpInputParam(
    std::ostringstream& oss, sptr<SceneSession> session, sptr<WindowSessionProperty> property)
{
    oss << "Input: "
        << session->forceTouchable_ << "|"
        << session->systemTouchable_ << "|"
        << session->sessionInfo_.isSetPointerAreas_ << "|"
        << property->GetTouchable() << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpLakeParamList(std::ostringstream& oss)
{
    oss << "Lake: "
        << "sessionAffinity collaboratorType"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpLakeParam(std::ostringstream& oss, sptr<SceneSession> session)
{
    oss << "Lake: "
        << session->sessionInfo_.sessionAffinity << "|"
        << session->sessionInfo_.collaboratorType_ << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpCOMParamList(std::ostringstream& oss)
{
    oss << "COM: "
        << "isRSDrawing"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpCOMParam(std::ostringstream& oss, sptr<SceneSession> session)
{
    oss << "COM: "
        << session->GetDrawingContentState() << "|"
        << std::endl;
}
// LCOV_EXCL_STOP

void HidumpController::DumpVisibleParamList(std::ostringstream& oss)
{
    oss << "Visible: "
        << "isRSVisible visibilityState"
        << std::endl;
}

// LCOV_EXCL_START
void HidumpController::DumpVisibleParam(std::ostringstream& oss, sptr<SceneSession> session)
{
    oss << "Visible: "
        << session->GetRSVisible() << "|"
        << static_cast<uint32_t>(session->GetVisibilityState()) << "|"
        << std::endl;
}
// LCOV_EXCL_STOP
}
} // namespace OHOS::Rosen
