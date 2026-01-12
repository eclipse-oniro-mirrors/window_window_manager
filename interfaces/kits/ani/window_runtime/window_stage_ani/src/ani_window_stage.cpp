/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_window_stage.h"

#include <memory>

#include "ani.h"
#include "ani_window.h"
#include "ani_window_manager.h"
#include "ani_window_utils.h"
#include "permission.h"
#include "pixel_map.h"
#include "pixel_map_taihe_ani.h"
#include "window_manager_hilog.h"
#include "window_scene.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_window_stage.h"
#include "ani_extension_window_config.h"

using OHOS::Rosen::WindowScene;

namespace OHOS {
namespace Rosen {
namespace {
/* used for free, ani has no destructor right now, only free when aniObj freed */
static std::map<ani_object, AniWindowStage*> g_localObjs;
const uint32_t MIN_RESOURCE_ID = 0x1000000;
const uint32_t MAX_RESOURCE_ID = 0xffffffff;
} // namespace

AniWindowStage::AniWindowStage(const std::shared_ptr<Rosen::WindowScene>& windowScene)
    : windowScene_(windowScene), registerManager_(std::make_unique<AniWindowRegisterManager>())
{
}
AniWindowStage::~AniWindowStage()
{
    TLOGE(WmsLogTag::DEFAULT, "[ANI] Ani WindowStage died");
}

ani_object AniWindowStage::NativeTransferStatic(ani_env* aniEnv, ani_class cls, ani_object input)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    void *unwrapResult = nullptr;
    if (!arkts_esvalue_unwrap(aniEnv, input, &unwrapResult)) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] fail to unwrap input");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (unwrapResult == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] unwrapResult is nullptr");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    JsWindowStage* jsWindowStage = static_cast<JsWindowStage*>(unwrapResult);
    if (jsWindowStage == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] jsWindowStage is nullptr");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::shared_ptr<WindowScene> windowScene = jsWindowStage->GetWindowScene().lock();
    return CreateAniWindowStage(aniEnv, windowScene);
}

ani_object AniWindowStage::NativeTransferDynamic(ani_env* aniEnv, ani_class cls, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindowStage is nullptr");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_env napiEnv {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] napi scope open fail");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value jsWindowStage = CreateJsWindowStage(napiEnv, aniWindowStage->GetWindowScene().lock());
    hybridgref ref {};
    if (!hybridgref_create_from_napi(napiEnv, jsWindowStage, &ref)) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create hybridgref fail");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    ani_object result {};
    if (!hybridgref_get_esvalue(aniEnv, ref, &result)) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] get esvalue fail");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (!hybridgref_delete_from_napi(napiEnv, ref)) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] delete hybridgref fail");
    }
    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] napi close scope fail");
        return AniWindowUtils::AniThrowError(aniEnv, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    return result;
}

void AniWindowStage::LoadContent(ani_env* env, ani_object obj, ani_long nativeObj, ani_string path,
    ani_object storage)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnLoadContent(env, path, storage, false);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::LoadContentByName(ani_env* env, ani_object obj, ani_long nativeObj, ani_string path,
    ani_object storage)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnLoadContent(env, path, storage, true);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::OnLoadContent(ani_env* env, ani_string path, ani_object storage, bool isLoadByName)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string contentPath;
    AniWindowUtils::GetStdString(env, path, contentPath);
    WmErrorCode ret;
    if (isLoadByName) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->AniSetUIContentByName(contentPath, env, storage));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->AniSetUIContent(contentPath, env, storage));
    }
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI] Window [%{public}u, %{public}s] load content end, ret=%{public}d",
        mainWindow->GetWindowId(), mainWindow->GetWindowName().c_str(), ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret, "Window load content failed");
    }
}

ani_ref AniWindowStage::GetMainWindow(ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] Get main window");
    std::shared_ptr<WindowScene> weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] WindowScene_ is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    sptr<Window> windowScene = weakScene->GetMainWindow();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Get main window failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] Get main window [%{public}u, %{public}s]",
        windowScene->GetWindowId(), windowScene->GetWindowName().c_str());

    return CreateAniWindowObject(env, windowScene);
}

ani_object AniWindowStage::GetSubWindow(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage == nullptr || aniWindowStage->GetMainWindow(env) == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] aniWindowStage is nullptr!");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return aniWindowStage->OnGetSubWindow(env);
}

ani_object AniWindowStage::OnGetSubWindow(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[ANI]");
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] Window scene is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::vector<sptr<Window>> subWindowVec = windowScene->GetSubWindow();
    TLOGI(WmsLogTag::WMS_LIFE, "Get sub windows, size = %{public}zu", subWindowVec.size());

    std::vector<ani_ref> windows(subWindowVec.size());
    for (size_t i = 0; i < subWindowVec.size(); i++) {
        windows[i] = CreateAniWindowObject(env, subWindowVec[i]);
    }
    return AniWindowUtils::CreateAniWindowsArray(env, windows);
}

ani_object AniWindowStage::CreateSubWindowWithOptions(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string name, ani_object options)
{
    TLOGI(WmsLogTag::WMS_SUB, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        return aniWindowStage->OnCreateSubWindowWithOptions(env, name, options);
    } else {
        TLOGE(WmsLogTag::WMS_SUB, "[ANI] aniWindowStage is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }
}

ani_object AniWindowStage::OnCreateSubWindowWithOptions(ani_env* env, ani_string name, ani_object options)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "WindowScene is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::string windowName;
    ani_status window_name = AniWindowUtils::GetStdString(env, name, windowName);
    if (window_name != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to windowName");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    sptr<WindowOption> windowOption = sptr<WindowOption>::MakeSptr();
    if (!AniWindowUtils::ParseSubWindowOption(env, options, windowOption)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to options");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if ((windowOption->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !windowScene->GetMainWindow()->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        return AniWindowUtils::CreateAniUndefined(env);
    }

    if (windowOption->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subWindow has topmost, but no system permission");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    windowOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowOption->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowOption->SetOnlySupportSceneBoard(true);
    auto window = windowScene->CreateWindow(windowName, windowOption);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "create window failed");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "get window failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_SUB, "Create sub window %{public}s end", windowName.c_str());
    return static_cast<ani_object>(CreateAniWindowObject(env, window));
}

void DropWindowStageByAni(ani_object aniObj)
{
    auto obj = g_localObjs.find(reinterpret_cast<ani_object>(aniObj));
    if (obj != g_localObjs.end()) {
        delete obj->second;
        g_localObjs.erase(obj);
    }
}

AniWindowStage* GetWindowStageFromAni(void* aniObj)
{
    auto obj = g_localObjs.find(reinterpret_cast<ani_object>(aniObj));
    if (obj == g_localObjs.end()) {
        return nullptr;
    }
    return obj->second;
}

ani_object CreateAniWindowStage(ani_env* env, std::shared_ptr<Rosen::WindowScene>& windowScene)
__attribute__((no_sanitize("cfi")))
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return nullptr;
    }
    TLOGD(WmsLogTag::DEFAULT, "[ANI] create wstage");

    ani_status ret;
    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowStageInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env %{public}u", ret);
        return cls;
    }

    std::unique_ptr<AniWindowStage> windowStage = std::make_unique<AniWindowStage>(windowScene);

    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] get ctor fail %{public}u", ret);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] obj new fail %{public}u", ret);
        return nullptr;
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] call setNativeObj fail %{public}u", ret);
        return nullptr;
    }
    env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(windowStage.get()));
    g_localObjs.insert(std::pair(obj, windowStage.release()));
    return obj;
}

AniWindowStage* GetWindowStageFromEnv(ani_env* env, ani_class cls, ani_object obj)
{
    ani_field nativeObjName {};
    ani_status ret;
    if ((ret = env->Class_FindField(cls, "nativeObj", &nativeObjName)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] obj fetch field %{public}u", ret);
        return nullptr;
    }
    ani_long nativeObj {};
    if ((ret = env->Object_GetField_Long(obj, nativeObjName, &nativeObj)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] obj fetch long %{public}u", ret);
        return nullptr;
    }
    return reinterpret_cast<AniWindowStage*>(nativeObj);
}

void AniWindowStage::DisableWindowDecor(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnDisableWindowDecor(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowStage is nullptr");
    }
}
void AniWindowStage::OnDisableWindowDecor(ani_env* env)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->DisableAppWindowDecor());
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindowStage::SetWindowRectAutoSave(ani_env* env, ani_class cls, ani_long nativeObj,
    ani_boolean enabled, bool isSaveBySpecifiedFlag)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnSetWindowRectAutoSave(env, enabled, isSaveBySpecifiedFlag);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::OnSetWindowRectAutoSave(ani_env* env, ani_boolean enabled, bool isSaveBySpecifiedFlag)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->SetWindowRectAutoSave(enabled, isSaveBySpecifiedFlag));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] OnSetWindowRectAutoSave end!");
}

ani_boolean AniWindowStage::IsWindowRectAutoSave(ani_env* env, ani_class cls, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        return aniWindowStage->OnIsWindowRectAutoSave(env);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowStage is nullptr");
        return false;
    }
}

ani_boolean AniWindowStage::OnIsWindowRectAutoSave(ani_env* env)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    bool enabled = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->IsWindowRectAutoSave(enabled));
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return false;
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] OnIsWindowRectAutoSave end!");
    return ani_boolean(enabled);
}
    
void AniWindowStage::SetShowOnLockScreen(ani_env* env, ani_class cls, ani_long nativeObj, ani_boolean showOnLockScreen)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnSetShowOnLockScreen(env, showOnLockScreen);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindowStage is nullptr");
    }
}
void AniWindowStage::OnSetShowOnLockScreen(ani_env* env, ani_boolean showOnLockScreen)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI]set show on lock screen permission denied!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][SetShowOnLockScreen]msg: permission denied");
        return;
    }
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][SetShowOnLockScreen]msg: invalid windowScene");
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][SetShowOnLockScreen]msg: invalid mainWindow");
        return;
    }
    if (showOnLockScreen) {
        mainWindow->AddWindowFlag(OHOS::Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    } else {
        mainWindow->RemoveWindowFlag(OHOS::Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }
    TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] OnSetShowOnLockScreen end!");
}

void AniWindowStage::SetWindowModal(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isModal)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnSetWindowModal(env, isModal);
    } else {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::OnSetWindowModal(ani_env* env, ani_boolean isModal)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[ANI]");
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowModal]msg: windowScene is nullptr");
        return;
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowModal]msg: windowScene is nullptr");
        return;
    }
    if (!window->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "device not support");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][setWindowModal]msg: device not support");
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowModal(isModal));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "failed, ret is %{public}d", ret);
        AniWindowUtils::AniThrowError(env, ret,
            "window][setWindowModal]msg: Set main window modal failed");
        return;
    }
    TLOGI(WmsLogTag::WMS_HIERARCHY, "id:%{public}u, name:%{public}s, isModal:%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), isModal);
}

void AniWindowStage::SetImageForRecent(ani_env* env, ani_class cls, ani_long nativeObj, ani_object imageResourceId,
    ani_int value)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnSetImageForRecent(env, imageResourceId, value);
    } else {
        TLOGE(WmsLogTag::WMS_PATTERN, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::OnSetImageForRecent(ani_env* env, ani_object imageResource, ani_int value)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    ani_class longClass {};
    env->FindClass("std.core.Long", &longClass);
    ani_class pixelMapClass {};
    env->FindClass("@ohos.multimedia.image.image.PixelMap", &pixelMapClass);
    ani_boolean isLong = ANI_FALSE;
    env->Object_InstanceOf(imageResource, longClass, &isLong);
    ani_boolean isPixelMap = ANI_FALSE;
    env->Object_InstanceOf(imageResource, pixelMapClass, &isPixelMap);
 
    ani_long imageResourceId;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    if (isLong) {
        env->Object_CallMethodByName_Long(imageResource, "toLong", ":l", &imageResourceId);
        if (imageResourceId < MIN_RESOURCE_ID || imageResourceId > MAX_RESOURCE_ID) {
            TLOGE(WmsLogTag::WMS_PATTERN, "imageResourceId invalid: %{public}d", static_cast<int32_t>(imageResourceId));
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return;
        }
    } else if (isPixelMap) {
        pixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, imageResource);
        if (pixelMap == nullptr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Get pixelMap error");
            AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return;
        }
    } else {
        TLOGE(WmsLogTag::WMS_PATTERN, "imageResource invalid");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return;
    }
    if (value < static_cast<int32_t>(ImageFit::FILL) || value > static_cast<int32_t>(ImageFit::MATRIX)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "imageFit invalid: %{public}d", value);
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
        return;
    }
    Ark_ImageFit arkImageFit = static_cast<Ark_ImageFit>(value);
    ImageFit imageFit = ImageFit::FILL;
    ConvertImageFit(imageFit, arkImageFit);
    TLOGI(WmsLogTag::WMS_PATTERN, "value: %{public}d, imageFit: %{public}d", value, imageFit);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (pixelMap) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->SetImageForRecentPixelMap(pixelMap, imageFit));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->SetImageForRecent(imageResourceId, imageFit));
    }
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindowStage::ConvertImageFit(ImageFit& dst, const Ark_ImageFit& src)
{
    switch (src) {
        case Ark_ImageFit::ARK_IMAGE_FIT_CONTAIN: dst = ImageFit::CONTAIN; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_COVER: dst = ImageFit::COVER; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_AUTO: dst = ImageFit::FITWIDTH; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_FILL: dst = ImageFit::FILL; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_SCALE_DOWN: dst = ImageFit::SCALE_DOWN; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_NONE: dst = ImageFit::NONE; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_TOP_START: dst = ImageFit::TOP_LEFT; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_TOP: dst = ImageFit::TOP; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_TOP_END: dst = ImageFit::TOP_END; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_START: dst = ImageFit::START; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_CENTER: dst = ImageFit::CENTER; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_END: dst = ImageFit::END; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_BOTTOM_START: dst = ImageFit::BOTTOM_START; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_BOTTOM: dst = ImageFit::BOTTOM; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_BOTTOM_END: dst = ImageFit::CONTAIN; break;
        case Ark_ImageFit::ARK_IMAGE_FIT_MATRIX: dst = ImageFit::MATRIX; break;
        default: TLOGE(WmsLogTag::DEFAULT, "imageFit: %{public}d", src);
    }
}

void AniWindowStage::RemoveImageForRecent(ani_env* env, ani_class cls, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnRemoveImageForRecent(env);
    } else {
        TLOGE(WmsLogTag::WMS_PATTERN, "[ANI] aniWindowStage is nullptr");
    }
}
 
void AniWindowStage::OnRemoveImageForRecent(ani_env* env)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(mainWindow->RemoveImageForRecent());
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindowStage::SetCustomDensity(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_double density, ani_boolean applyToSubWindow)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnSetCustomDensity(env, density, applyToSubWindow);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindowStage is nullptr");
        return;
    }
}

void AniWindowStage::OnSetCustomDensity(ani_env* env, ani_double density, ani_boolean applyToSubWindow)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        window->SetCustomDensity(static_cast<float>(density), applyToSubWindow));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u,%{public}s] set density=%{public}f, result=%{public}d",
        window->GetWindowId(), window->GetWindowName().c_str(), density, ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindowStage::SetDefaultDensityEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnSetDefaultDensityEnabled(env, enabled);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] aniWindowStage is nullptr");
        return;
    }
}

void AniWindowStage::OnSetDefaultDensityEnabled(ani_env* env, ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetDefaultDensityEnabled(enabled));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u,%{public}s] enabled=%{public}u ret=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), enabled, ret);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}

void AniWindowStage::RemoveStartingWindow(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "[ANI] start");
    AniWindowStage* windowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (windowStage == nullptr || windowStage->GetWindowScene().lock() == nullptr) {
        TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "[ANI] windowStage is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    windowStage->OnRemoveStartingWindow(env);
}

void AniWindowStage::OnRemoveStartingWindow(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "[ANI]");
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "[ANI] windowScene is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "[ANI] window is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->NotifyRemoveStartingWindow());
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "[ANI] Notify remove starting window failed");
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindowStage::SetSupportedWindowModes(
    ani_env* env, ani_object obj, ani_long nativeObj, ani_object aniSupportedWindowModes)
{
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindowStage is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindowStage->OnSetSupportedWindowModes(env, aniSupportedWindowModes);
}

void AniWindowStage::OnSetSupportedWindowModes(ani_env* env, ani_object aniSupportedWindowModes)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] windowScene is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] mainWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto supportedWindowModes =
        AniWindowUtils::ExtractEnumValues<AppExecFwk::SupportWindowMode>(env, aniSupportedWindowModes);
    WMError ret = window->SetSupportedWindowModes(supportedWindowModes);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed, windowId: %{public}u, ret: %{public}d",
              window->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, AniWindowUtils::ToErrorCode(ret));
        return;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Success, windowId: %{public}u", window->GetWindowId());
}

void AniWindowStage::SetSupportedWindowModesWithGrayOutMaximizeButton(
    ani_env* env, ani_object obj, ani_long nativeObj,
    ani_object supportedWindowModes, ani_boolean grayOutMaximizeButton)
{
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] aniWindowStage is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    aniWindowStage->OnSetSupportedWindowModesWithGrayOutMaximizeButton(
        env, supportedWindowModes, grayOutMaximizeButton);
}

void AniWindowStage::OnSetSupportedWindowModesWithGrayOutMaximizeButton(
    ani_env* env, ani_object aniSupportedWindowModes, ani_boolean grayOutMaximizeButton)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] windowScene is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] mainWindow is nullptr");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto supportedWindowModes =
        AniWindowUtils::ExtractEnumValues<AppExecFwk::SupportWindowMode>(env, aniSupportedWindowModes);
    WMError ret = window->SetSupportedWindowModes(supportedWindowModes, static_cast<bool>(grayOutMaximizeButton));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed, windowId: %{public}u, ret: %{public}d",
              window->GetWindowId(), static_cast<int32_t>(ret));
        AniWindowUtils::AniThrowError(env, AniWindowUtils::ToErrorCode(ret));
        return;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Success, windowId: %{public}u", window->GetWindowId());
}

ani_ref AniWindowStage::OnCreateSubWindow(ani_env* env, ani_string name)
{
    std::string windowName;
    ani_status ret = AniWindowUtils::GetStdString(env, name, windowName);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] invalid param of name");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI] Window scene is nullptr");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    sptr<Rosen::WindowOption> windowOption = new Rosen::WindowOption();
    windowOption->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    auto window = weakScene->CreateWindow(windowName, windowOption);
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Create window failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return CreateAniWindowObject(env, window);
}

void AniWindowStage::RegisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnRegisterWindowCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::OnRegisterWindowCallback(ani_env* env, ani_string type, ani_ref callback)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}s", cbType.c_str());
    WmErrorCode ret = registerManager_->RegisterListener(mainWindow, cbType, CaseType::CASE_STAGE, env, callback, 0);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
    }
}

void AniWindowStage::UnregisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
    ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* aniWindowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (aniWindowStage != nullptr) {
        aniWindowStage->OnUnregisterWindowCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] aniWindowStage is nullptr");
    }
}

void AniWindowStage::OnUnregisterWindowCallback(ani_env* env, ani_string type, ani_ref callback)
{
    auto windowScene = GetWindowScene().lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]windowScene is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto mainWindow = windowScene->GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] mainWindow is nullptr!");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    std::string cbType;
    AniWindowUtils::GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] type:%{public}s", cbType.c_str());
    WmErrorCode ret = registerManager_->UnregisterListener(mainWindow, cbType, CaseType::CASE_STAGE, env, callback);
    if (ret != WmErrorCode::WM_OK) {
        AniWindowUtils::AniThrowError(env, ret);
        return;
    }
}
}  // namespace Rosen
}  // namespace OHOS