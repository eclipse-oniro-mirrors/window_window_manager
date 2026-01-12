/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "ani_extension_window.h"

#include "ani.h"
#include "ani_extension_window_register_manager.h"
#include "ani_window_utils.h"
#include "ani_window_listener.h"
#include "extension_window_impl.h"
#include "window_manager_hilog.h"
#include "permission.h"
#include "ani_window.h"

namespace OHOS {
namespace Rosen {
namespace {
static std::map<ani_ref, AniExtensionWindow*> localObjs;
constexpr const char* ETS_UIEXTENSION_HOST_CLASS_DESCRIPTOR =
    "@ohos.uiExtensionHost.uiExtensionHost.UIExtensionHostInternal";
constexpr const char* ETS_UIEXTENSION_CLASS_DESCRIPTOR =
    "@ohos.arkui.uiExtension.uiExtension.UIExtensionInternal";
}
AniExtensionWindow::AniExtensionWindow(
    const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow, int32_t hostWindowId)
    :extensionWindow_(extensionWindow),
    hostWindowId_(hostWindowId),
    extensionRegisterManager_(std::make_unique<AniExtensionWindowRegisterManager>())
{
    TLOGI(WmsLogTag::WMS_UIEXT, "hostWindowId: %{public}d", hostWindowId_);
}

AniExtensionWindow::AniExtensionWindow(
    const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow, sptr<AAFwk::SessionInfo> sessionInfo)
    :extensionWindow_(extensionWindow),
    hostWindowId_(-1),
    sessionInfo_(sessionInfo),
    extensionRegisterManager_(std::make_unique<AniExtensionWindowRegisterManager>())
{
}

void AniExtensionWindow::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindow* extensionWindow = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (extensionWindow != nullptr) {
        auto obj = localObjs.find(reinterpret_cast<ani_ref>(extensionWindow->GetAniRef()));
        if (obj != localObjs.end()) {
            delete obj->second;
            localObjs.erase(obj);
        }
        env->GlobalReference_Delete(extensionWindow->GetAniRef());
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] extensionWindow is nullptr");
    }
}

ani_object AniExtensionWindow::CreateAniExtensionWindow(ani_env* env, sptr<Rosen::Window> window, int32_t hostWindowId,
    bool isHost)
{
    if (env == nullptr || window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "env or window is nullptr");
        return nullptr;
    }

    ani_status ret {};
    ani_class cls = nullptr;
    const char* clsName = isHost ? ETS_UIEXTENSION_HOST_CLASS_DESCRIPTOR : ETS_UIEXTENSION_CLASS_DESCRIPTOR;
    if ((ret = env->FindClass(clsName, &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return cls;
    }

    std::shared_ptr<ExtensionWindow> extensionWindow = std::make_shared<ExtensionWindowImpl>(window);
    std::unique_ptr<AniExtensionWindow> aniExtensionWindow =
        std::make_unique<AniExtensionWindow>(extensionWindow, hostWindowId);

    ani_field contextField;
    if ((ret = env->Class_FindField(cls, "nativeObj", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find field failed, ret: %{public}u", ret);
        return nullptr;
    }

    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find ctor method failed, ret: %{public}u", ret);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]New object failed, ret: %{public}u", ret);
        return nullptr;
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find method failed, ret: %{public}u", ret);
        return nullptr;
    }
    env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(aniExtensionWindow.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(obj, &ref) == ANI_OK) {
        aniExtensionWindow->SetAniRef(ref);
        localObjs.insert(std::pair(ref, aniExtensionWindow.release()));
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] create global ref fail");
    }
    return obj;
}

WMError AniExtensionWindow::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return extensionWindow_->GetAvoidAreaByType(type, avoidArea);
}

WmErrorCode AniExtensionWindow::OnSetWaterMarkFlag(ani_env* env, ani_boolean enable)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError ret = extensionWindow_->SetWaterMarkFlag(enable);
    WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    if (errorCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errorCode);
        return errorCode;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]finished, enable: %{public}u", enable);
    return WmErrorCode::WM_OK;
}

WmErrorCode AniExtensionWindow::OnHidePrivacyContentForHost(ani_env* env, ani_boolean shouldHide)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError ret = extensionWindow_->HidePrivacyContentForHost(shouldHide);
    WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    if (errorCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errorCode);
        return errorCode;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]finished, shouldHide: %{public}u", shouldHide);
    return WmErrorCode::WM_OK;
}

bool AniExtensionWindow::IsExtensionWindowValid()
{
    if (extensionWindow_ == nullptr) {
        return false;
    }
    sptr<Rosen::Window> window = extensionWindow_->GetWindow();
    if (window == nullptr) {
        return false;
    }
    return true;
}

WmErrorCode AniExtensionWindow::UnregisterListener(ani_env* env, ani_string type, ani_object fn)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI] type:%{public}s", cbType.c_str());
    sptr<Rosen::Window> window = extensionWindow_->GetWindow();
    return extensionRegisterManager_->UnregisterListener(window, cbType, CaseType::CASE_WINDOW, env, fn);
}

WmErrorCode AniExtensionWindow::RegisterListener(ani_env* env, ani_string type,  ani_object fn)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI] type:%{public}s", cbType.c_str());
    sptr<Rosen::Window> window = extensionWindow_->GetWindow();
    return extensionRegisterManager_->RegisterListener(window, cbType, CaseType::CASE_WINDOW, env, fn);
}

WmErrorCode AniExtensionWindow::GetExtensionWindowRect(Rect& rect)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    rect = extensionWindow_->GetWindow()->GetRect();
    return WmErrorCode::WM_OK;
}

WmErrorCode AniExtensionWindow::OnHideNonSecureWindows(ani_env* env, ani_boolean shouldHide)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError ret = extensionWindow_->HideNonSecureWindows(shouldHide);
    WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    if (errorCode != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, errorCode);
        return errorCode;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]finished, shouldHide: %{public}u", shouldHide);
    return WmErrorCode::WM_OK;
}

ani_object AniExtensionWindow::OnCreateSubWindowWithOptions(ani_env* env, ani_string name, ani_object subWindowOptions,
    ani_boolean followCreatorLifecycle)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::string windowName;
    ani_status status = AniWindowUtils::GetStdString(env, name, windowName);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Failed to convert parameter to windowName");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    sptr<WindowOption> option = new WindowOption();
    if (!AniWindowUtils::ParseSubWindowOptions(env, subWindowOptions, option)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get invalid options param");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if ((option->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !extensionWindow_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI]device not support");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    }
    if (option->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    option->SetParentId(hostWindowId_);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    option->SetOnlySupportSceneBoard(true);
    option->SetIsUIExtFirstSubWindow(true);
    option->SetFollowCreatorLifecycle(static_cast<bool>(followCreatorLifecycle));
    auto window = Window::Create(windowName, option, extensionWindow_->GetWindow()->GetContext());
    if (window == nullptr) {
        TLOGI(WmsLogTag::WMS_UIEXT, "create sub window failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!window->IsTopmost()) {
        extensionWindow_->GetWindow()->NotifyModalUIExtensionMayBeCovered(false);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "%{public}s end", windowName.c_str());
    return AniWindow::CreateAniWindow(env, window);
}

void AniExtensionWindow::OnOccupyEvents(ani_env* env, ani_int eventFlags)
{
    auto ret = extensionWindow_->OccupyEvents(eventFlags);
    WmErrorCode errorCode = AniWindowUtils::ToErrorCode(ret);
    if (errorCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]OnOccupyEvents failed, code: %{public}d", errorCode);
        AniWindowUtils::AniThrowError(env, ret);
    }
}

WmErrorCode AniExtensionWindow::OnRegisterRectChangeCallback(ani_env* env, ani_int reason, ani_object fn)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto windowImpl = extensionWindow_->GetWindow();
    if (!windowImpl->IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Device is not PC");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (static_cast<uint32_t>(reason) != static_cast<uint32_t>(
        AniExtensionWindowListener::ComponentRectChangeReason::HOST_WINDOW_RECT_CHANGE)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Unsupported rect change reasons");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    return extensionRegisterManager_->RegisterListener(
        windowImpl, WINDOW_RECT_CHANGE_CB, CaseType::CASE_WINDOW, env, fn);
}

WmErrorCode AniExtensionWindow::OnUnRegisterRectChangeCallback(ani_env* env, ani_object fn)
{
    if (!IsExtensionWindowValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]extension window is invalid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto windowImpl = extensionWindow_->GetWindow();
    if (!windowImpl->IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Device is not PC");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return extensionRegisterManager_->UnregisterListener(
        windowImpl, WINDOW_RECT_CHANGE_CB, CaseType::CASE_WINDOW, env, fn);
}

static ani_status ExtWindowSetRect(ani_env* env, ani_object obj, OHOS::Rosen::Rect& rect)
{
    ani_status ret {};
    if ((ret = env->Object_SetFieldByName_Int(obj, "<property>left", rect.posX_)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Set left field failed, ret: %{public}u", ret);
        return ret;
    }
    if ((ret = env->Object_SetFieldByName_Int(obj, "<property>top", rect.posY_)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Set top field failed, ret: %{public}u", ret);
        return ret;
    }
    if ((ret = env->Object_SetFieldByName_Int(obj, "<property>width", rect.width_)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Set width field failed, ret: %{public}u", ret);
        return ret;
    }
    if ((ret = env->Object_SetFieldByName_Int(obj, "<property>height", rect.height_)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Set height field failed, ret: %{public}u", ret);
        return ret;
    }
    return ANI_OK;
}

static ani_status ExtWindowSetRectMember(ani_env* env, ani_object obj, const char* member, OHOS::Rosen::Rect& rect)
{
    ani_status ret {};
    ani_ref rectRef {};
    if ((ret = env->Object_GetFieldByName_Ref(obj, member, &rectRef)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get field %{public}s failed, ret: %{public}u", member, ret);
        return ret;
    };
    return ExtWindowSetRect(env, static_cast<ani_object>(rectRef), rect);
}

static ani_int ExtWindowGetProperties(ani_env* env, ani_object obj, ani_long nativeObj, ani_object propertyRef)
{
    WmErrorCode retCode = WmErrorCode::WM_OK;
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        return static_cast<ani_int>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    Rect rect {};
    if ((retCode = aniExtWinPtr->GetExtensionWindowRect(rect)) != WmErrorCode::WM_OK) {
        return static_cast<ani_int>(retCode);
    };
    return static_cast<ani_int>(
        ExtWindowSetRectMember(env, propertyRef, "<property>uiExtensionHostWindowProxyRect", rect));
}

static ani_int ExtWindowGetWindowAvoidArea(ani_env* env, ani_object obj, ani_long win,
    ani_int areaType, ani_object area)
{
    bool hasAvoidAreaTypeErr = (areaType < static_cast<int32_t>(AvoidAreaType::TYPE_START) ||
        areaType >= static_cast<int32_t>(AvoidAreaType::TYPE_END));
    if (hasAvoidAreaTypeErr) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return static_cast<ani_int>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    AvoidArea avoidArea {};
    ani_status ret {};
    ret = env->Object_SetFieldByName_Boolean(area, "<property>visible", areaType !=
        static_cast<int32_t>(AvoidAreaType::TYPE_CUTOUT));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Set visible failed, ret: %{public}u", ret);
        return static_cast<ani_int>(ret);
    }
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(win);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        return static_cast<ani_int>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WMError retCode = aniExtWinPtr->GetAvoidAreaByType(static_cast<AvoidAreaType>(areaType), avoidArea);
    if (retCode != WMError::WM_OK) {
        return static_cast<ani_int>(AniWindowUtils::ToErrorCode(retCode));
    }
    ret = ExtWindowSetRectMember(env, area, "<property>leftRect", avoidArea.leftRect_);
    if (ret != ANI_OK) {
        return static_cast<ani_int>(ret);
    }
    ret = ExtWindowSetRectMember(env, area, "<property>rightRect", avoidArea.rightRect_);
    if (ret != ANI_OK) {
        return static_cast<ani_int>(ret);
    }
    ret = ExtWindowSetRectMember(env, area, "<property>topRect", avoidArea.topRect_);
    if (ret != ANI_OK) {
        return static_cast<ani_int>(ret);
    }
    ret = ExtWindowSetRectMember(env, area, "<property>bottomRect", avoidArea.bottomRect_);
    if (ret != ANI_OK) {
        return static_cast<ani_int>(ret);
    }
    return static_cast<ani_int>(WmErrorCode::WM_OK);
}

static ani_int ExtWindowSetWaterMarkFlag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return static_cast<ani_int>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return static_cast<ani_int>(aniExtWinPtr->OnSetWaterMarkFlag(env, enable));
}

static ani_int ExtWindowHidePrivacyContentForHost(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_boolean shouldHide)
{
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return static_cast<ani_int>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return static_cast<ani_int>(aniExtWinPtr->OnHidePrivacyContentForHost(env, shouldHide));
}

static void RegisterExtWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_object callback)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]");
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    WmErrorCode ret = aniExtWinPtr->RegisterListener(env, type, callback);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Register failed: %{public}d", static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
    }
}

static void UnregisterExtWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_object callback)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]");
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = aniExtWinPtr->UnregisterListener(env, type, callback);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Unregister failde: %{public}d", static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
    }
}

static ani_int ExtWindowHideNonSecureWindows(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_boolean shouldHide)
{
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return static_cast<ani_int>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return static_cast<ani_int>(aniExtWinPtr->OnHideNonSecureWindows(env, shouldHide));
}

static ani_object ExtWindowCreateSubWindowWithOptions(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string name, ani_object subWindowOptions, ani_boolean followCreatorLifecycle)
{
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return aniExtWinPtr->OnCreateSubWindowWithOptions(env, name, subWindowOptions, followCreatorLifecycle);
}

static void ExtWindowOccupyEvents(ani_env* env, ani_object obj, ani_long nativeObj, ani_int eventFlags)
{
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniExtWinPtr->OnOccupyEvents(env, eventFlags);
}

static void ExtWindowOnRectChange(ani_env* env, ani_object obj, ani_long nativeObj, ani_int reason, ani_object callback)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]");
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }

    WmErrorCode ret = aniExtWinPtr->OnRegisterRectChangeCallback(env, reason, callback);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Register failde: %{public}d", static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
    }
}

static void ExtWindowOffRectChange(ani_env* env, ani_object obj, ani_long nativeObj, ani_object callback)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]");
    AniExtensionWindow* aniExtWinPtr = reinterpret_cast<AniExtensionWindow*>(nativeObj);
    if (aniExtWinPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniExtWinPtr is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = aniExtWinPtr->OnUnRegisterRectChangeCallback(env, callback);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Unregister failde: %{public}d", static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, ret);
    }
}

ani_object createExtensionWindow(ani_env* env, ani_long win, ani_int hostId)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]Create extwindow with hostId: %{public}d", hostId);
    auto winPtr = reinterpret_cast<sptr<Rosen::Window>*>(win);
    if (winPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]winPtr is nullptr");
        return nullptr;
    }
    sptr<Rosen::Window> window = *winPtr;
    return AniExtensionWindow::CreateAniExtensionWindow(env, window, hostId);
}
}
} // namespace

extern "C" {
using namespace OHOS::Rosen;
std::array extensionWindowNativeMethods = {
    ani_native_function {"getProperties", "lC{std.core.Object}:i", reinterpret_cast<void *>(ExtWindowGetProperties)},
    ani_native_function {"getWindowAvoidArea", "liC{std.core.Object}:i",
        reinterpret_cast<void *>(ExtWindowGetWindowAvoidArea)},
    ani_native_function {"setWaterMarkFlag", "lz:i",
        reinterpret_cast<void *>(ExtWindowSetWaterMarkFlag)},
    ani_native_function {"hideNonSecureWindows", "lz:i", reinterpret_cast<void *>(ExtWindowHideNonSecureWindows)},
    ani_native_function {"createSubWindowWithOptions",
        "lC{std.core.String}C{@ohos.window.window.SubWindowOptions}z:@ohos.window.window.Window",
        reinterpret_cast<void *>(ExtWindowCreateSubWindowWithOptions)},
    ani_native_function {"occupyEvents", "li:", reinterpret_cast<void *>(ExtWindowOccupyEvents)},
    ani_native_function {"onSync", "lC{std.core.String}C{std.core.Object}:",
        reinterpret_cast<void *>(RegisterExtWindowCallback)},
    ani_native_function {"offSync", "lC{std.core.String}C{std.core.Object}:",
        reinterpret_cast<void *>(UnregisterExtWindowCallback)},
    ani_native_function {"onRectChange", "liC{std.core.Object}:", reinterpret_cast<void *>(ExtWindowOnRectChange)},
    ani_native_function {"offRectChange", "lC{std.core.Object}:", reinterpret_cast<void *>(ExtWindowOffRectChange)},
    };

std::array extensionWindowHostNativeMethods = {
    ani_native_function {"getProperties", "lC{std.core.Object}:i", reinterpret_cast<void *>(ExtWindowGetProperties)},
    ani_native_function {"getWindowAvoidArea", "liC{std.core.Object}:",
        reinterpret_cast<void *>(ExtWindowGetWindowAvoidArea)},
    ani_native_function {"setWaterMarkFlag", "lz:i",
        reinterpret_cast<void *>(ExtWindowSetWaterMarkFlag)},
    ani_native_function {"hidePrivacyContentForHost", "lz:i",
        reinterpret_cast<void *>(ExtWindowHidePrivacyContentForHost)},
    ani_native_function {"hideNonSecureWindows", "lz:i", reinterpret_cast<void *>(ExtWindowHideNonSecureWindows)},
    ani_native_function {"createSubWindowWithOptions",
        "lC{std.core.String}C{@ohos.window.window.SubWindowOptions}z:C{@ohos.window.window.Window}",
        reinterpret_cast<void *>(ExtWindowCreateSubWindowWithOptions)},
    ani_native_function {"onSync", "lC{std.core.String}C{std.core.Object}:",
        reinterpret_cast<void *>(RegisterExtWindowCallback)},
    ani_native_function {"offSync", "lC{std.core.String}C{std.core.Object}:",
        reinterpret_cast<void *>(UnregisterExtWindowCallback)},
    };

ANI_EXPORT ani_status ExtensionWindow_ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindow begin");
    ani_status ret {};
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get env failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass(ETS_UIEXTENSION_CLASS_DESCRIPTOR, &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    if ((ret = env->Class_BindNativeMethods(cls, extensionWindowNativeMethods.data(),
        extensionWindowNativeMethods.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Class bind native methods failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    ani_namespace ns;
    if ((ret = env->FindNamespace(ETS_UIEXTENSION_NAMESPACE_DESCRIPTOR, &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find namespace failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array functions = {
        ani_native_function {"destroyWindowProxy", nullptr, reinterpret_cast<void *>(AniExtensionWindow::Finalizer)},
    };
    if ((ret = env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Namespace bind native functions failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindow end");
    return ANI_OK;
}

ANI_EXPORT ani_status ExtensionWindowHost_ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowHost begin");
    ani_status ret {};
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get env failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass(ETS_UIEXTENSION_HOST_CLASS_DESCRIPTOR, &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    if ((ret = env->Class_BindNativeMethods(cls, extensionWindowHostNativeMethods.data(),
        extensionWindowHostNativeMethods.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Class bind native methods failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    ani_namespace ns;
    if ((ret = env->FindNamespace(ETS_UIEXTENSION_HOST_NAMESPACE_DESCRIPTOR, &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find namespace failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array functions = {
        ani_native_function {"createExtensionWindow", nullptr, reinterpret_cast<void *>(createExtensionWindow)},
        ani_native_function {"destroyWindowProxy", nullptr, reinterpret_cast<void *>(AniExtensionWindow::Finalizer)},
    };
    if ((ret = env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Namespace bind native functions failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowHost end");
    return ANI_OK;
}
}
