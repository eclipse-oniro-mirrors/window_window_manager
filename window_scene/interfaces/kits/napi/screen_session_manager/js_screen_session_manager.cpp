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

#include "js_screen_session_manager.h"

#include <hitrace_meter.h>
#include <js_runtime_utils.h>

#include "display_manager.h"
#include "interfaces/include/ws_common.h"
#include "js_screen_session.h"
#include "js_screen_utils.h"
#include "js_device_screen_config.h"
#include "pixel_map_napi.h"
#include "window_manager_hilog.h"

#ifdef POWER_MANAGER_ENABLE
#include "shutdown/shutdown_client.h"
#include "shutdown/itakeover_shutdown_callback.h"
#endif

namespace OHOS::Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr int32_t INVALID_ID = -1;

namespace {
const std::string ON_SCREEN_CONNECTION_CHANGE_CALLBACK = "screenConnectChange";
const std::string ON_TENT_MODE_CHANGE_CALLBACK = "tentModeChange";
} // namespace

JsScreenSessionManager::JsScreenSessionManager(napi_env env) : env_(env)
{
    TLOGI(WmsLogTag::DMS, "Create instance");
}

napi_value JsScreenSessionManager::Init(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::DMS, "Init.");
    if (env == nullptr || exportObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to init, env or exportObj is null!");
        return nullptr;
    }
    sptr<JsScreenSessionManager> jsScreenSessionManager = new (std::nothrow) JsScreenSessionManager(env);
    if (jsScreenSessionManager == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create, jsScreenSessionManager is null");
        return nullptr;
    }
    jsScreenSessionManager->IncStrongRef(nullptr); // Avoid being released after the function ends
    napi_wrap(env, exportObj, jsScreenSessionManager.GetRefPtr(), JsScreenSessionManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "ScreenConnectChangeType",
        JsScreenUtils::CreateJsScreenConnectChangeType(env));
    napi_set_named_property(env, exportObj, "ScreenPropertyChangeReason",
        JsScreenUtils::CreateJsScreenPropertyChangeReason(env));
    napi_set_named_property(env, exportObj, "FoldStatus",
        JsScreenUtils::CreateJsFoldStatus(env));
    napi_set_named_property(env, exportObj, "ScreenPropertyChangeType",
        JsScreenUtils::CreateJsScreenPropertyChangeType(env));
    napi_set_named_property(env, exportObj, "SuperFoldStatus",
        JsScreenUtils::CreateJsSuperFoldStatus(env));
    napi_set_named_property(env, exportObj, "FoldDisplayMode",
        JsScreenUtils::CreateJsFoldDisplayMode(env));
    napi_set_named_property(env, exportObj, "ExtendScreenConnectStatus",
        JsScreenUtils::CreateJsExtendScreenConnectStatus(env));
    napi_set_named_property(env, exportObj, "ScreenModeChangeEvent",
        JsScreenUtils::CreateJsScreenModeChangeEvent(env));

    const char* moduleName = "JsScreenSessionManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsScreenSessionManager::RegisterCallback);
    BindNativeFunction(env, exportObj, "updateScreenRotationProperty", moduleName,
        JsScreenSessionManager::UpdateScreenRotationProperty);
    BindNativeFunction(env, exportObj, "updateServerScreenProperty", moduleName,
        JsScreenSessionManager::UpdateServerScreenProperty);
    BindNativeFunction(env, exportObj, "getCurvedScreenCompressionArea", moduleName,
        JsScreenSessionManager::GetCurvedCompressionArea);
    BindNativeFunction(env, exportObj, "registerShutdownCallback", moduleName,
        JsScreenSessionManager::RegisterShutdownCallback);
    BindNativeFunction(env, exportObj, "unRegisterShutdownCallback", moduleName,
        JsScreenSessionManager::UnRegisterShutdownCallback);
    BindNativeFunction(env, exportObj, "getPhyScreenProperty", moduleName,
        JsScreenSessionManager::GetPhyScreenProperty);
    BindNativeFunction(env, exportObj, "notifyScreenLockEvent", moduleName,
        JsScreenSessionManager::NotifyScreenLockEvent);
    BindNativeFunction(env, exportObj, "updateAvailableArea", moduleName,
        JsScreenSessionManager::UpdateAvailableArea);
    BindNativeFunction(env, exportObj, "updateSuperFoldAvailableArea", moduleName,
        JsScreenSessionManager::UpdateSuperFoldAvailableArea);
    BindNativeFunction(env, exportObj, "extraDestroyScreen", moduleName,
        JsScreenSessionManager::ExtraDestroyScreen);
    BindNativeFunction(env, exportObj, "updateSuperFoldExpandAvailableArea", moduleName,
        JsScreenSessionManager::UpdateSuperFoldExpandAvailableArea);
    BindNativeFunction(env, exportObj, "setScreenOffDelayTime", moduleName,
        JsScreenSessionManager::SetScreenOffDelayTime);
    BindNativeFunction(env, exportObj, "notifyFoldToExpandCompletion", moduleName,
        JsScreenSessionManager::NotifyFoldToExpandCompletion);
    BindNativeFunction(env, exportObj, "notifyScreenConnectCompletion", moduleName,
        JsScreenSessionManager::NotifyScreenConnectCompletion);
    BindNativeFunction(env, exportObj, "notifyAodOpCompletion", moduleName,
        JsScreenSessionManager::NotifyAodOpCompletion);
    BindNativeFunction(env, exportObj, "recordEventFromScb", moduleName,
        JsScreenSessionManager::RecordEventFromScb);
    BindNativeFunction(env, exportObj, "getFoldStatus", moduleName, JsScreenSessionManager::GetFoldStatus);
    BindNativeFunction(env, exportObj, "getSuperFoldStatus", moduleName, JsScreenSessionManager::GetSuperFoldStatus);
    BindNativeFunction(env, exportObj, "getSuperRotation", moduleName, JsScreenSessionManager::GetSuperRotation);
    BindNativeFunction(env, exportObj, "setLandscapeLockStatus", moduleName,
        JsScreenSessionManager::SetLandscapeLockStatus);
    BindNativeFunction(env, exportObj, "setForceCloseHdr", moduleName,
        JsScreenSessionManager::SetForceCloseHdr);
    BindNativeFunction(env, exportObj, "getScreenSnapshot", moduleName,
        JsScreenSessionManager::GetScreenSnapshot);
    BindNativeFunction(env, exportObj, "getScreenSnapshotSync", moduleName,
        JsScreenSessionManager::GetScreenSnapshot);
    BindNativeFunction(env, exportObj, "getDeviceScreenConfig", moduleName,
        JsScreenSessionManager::GetDeviceScreenConfig);
    BindNativeFunction(env, exportObj, "setCameraStatus", moduleName, JsScreenSessionManager::SetCameraStatus);
    BindNativeFunction(env, exportObj, "setScreenOnDelayTime", moduleName,
        JsScreenSessionManager::SetScreenOnDelayTime);
    BindNativeFunction(env, exportObj, "getExtendScreenConnectStatus", moduleName,
        JsScreenSessionManager::GetExtendScreenConnectStatus);
    BindNativeFunction(env, exportObj, "setDefaultMultiScreenModeWhenSwitchUser", moduleName,
        JsScreenSessionManager::SetDefaultMultiScreenModeWhenSwitchUser);
    BindNativeFunction(env, exportObj, "notifyExtendScreenCreateFinish", moduleName,
        JsScreenSessionManager::NotifyExtendScreenCreateFinish);
    BindNativeFunction(env, exportObj, "notifyExtendScreenDestroyFinish", moduleName,
        JsScreenSessionManager::NotifyExtendScreenDestroyFinish);
    BindNativeFunction(env, exportObj, "notifyScreenMaskAppear", moduleName,
        JsScreenSessionManager::NotifyScreenMaskAppear);
    BindNativeFunction(env, exportObj, "setPrimaryDisplaySystemDpi", moduleName,
        JsScreenSessionManager::SetPrimaryDisplaySystemDpi);
    BindNativeFunction(env, exportObj, "getPrimaryDisplaySystemDpi", moduleName,
        JsScreenSessionManager::GetPrimaryDisplaySystemDpi);
    BindNativeFunction(env, exportObj, "getFoldDisplayMode", moduleName,
        JsScreenSessionManager::GetFoldDisplayMode);
    BindNativeFunction(env, exportObj, "freezeScreen", moduleName, JsScreenSessionManager::FreezeScreen);
    BindNativeFunction(env, exportObj, "getScreenSnapshotWithAllWindows", moduleName,
        JsScreenSessionManager::GetScreenSnapshotWithAllWindows);
    BindNativeFunction(env, exportObj, "notifySwitchUserAnimationFinish", moduleName,
        JsScreenSessionManager::NotifySwitchUserAnimationFinish);
    BindNativeFunction(env, exportObj, "registerSwitchUserAnimationNotification", moduleName,
        JsScreenSessionManager::RegisterSwitchUserAnimationNotification);
    return NapiGetUndefined(env);
}

JsScreenSessionManager::~JsScreenSessionManager()
{
    TLOGI(WmsLogTag::DMS, "Destroy instance");
}

void JsScreenSessionManager::Finalizer([[maybe_unused]]napi_env env, void* data, [[maybe_unused]]void* hint)
{
    if (data == nullptr) {
        TLOGE(WmsLogTag::DMS, "data is nullptr");
        return;
    }
    TLOGI(WmsLogTag::DMS, "[NAPI]Finalizer. jsScreenSessionManager refcount before DecStrongRef: %{public}d",
        static_cast<JsScreenSessionManager*>(data)->GetSptrRefCount());
    // Expected to release the jsScreenSessionManager object here
    static_cast<JsScreenSessionManager*>(data)->DecStrongRef(data);
}

napi_value JsScreenSessionManager::RegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]RegisterCallback");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsScreenSessionManager::UpdateScreenRotationProperty(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]UpdateScreenRotationProperty");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateScreenRotationProperty(env, info) : nullptr;
}

napi_value JsScreenSessionManager::UpdateServerScreenProperty(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]UpdateServerScreenProperty");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateServerScreenProperty(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetCurvedCompressionArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetCurvedCompressionArea");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetCurvedCompressionArea(env, info) : nullptr;
}

napi_value JsScreenSessionManager::RegisterShutdownCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]RegisterShutdownCallback");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnRegisterShutdownCallback(env, info) : nullptr;
}

napi_value JsScreenSessionManager::UnRegisterShutdownCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]UnRegisterShutdownCallback");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnUnRegisterShutdownCallback(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetPhyScreenProperty(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetPhyScreenProperty");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetPhyScreenProperty(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyScreenLockEvent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]NotifyScreenLockEvent");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyScreenLockEvent(env, info) : nullptr;
}

napi_value JsScreenSessionManager::UpdateAvailableArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]UpdateAvailableArea");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateAvailableArea(env, info) : nullptr;
}

napi_value JsScreenSessionManager::UpdateSuperFoldAvailableArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]UpdateSuperFoldAvailableArea");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateSuperFoldAvailableArea(env, info) : nullptr;
}

napi_value JsScreenSessionManager::ExtraDestroyScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]ExtraDestroyScreen");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnExtraDestroyScreen(env, info) : nullptr;
}

napi_value JsScreenSessionManager::UpdateSuperFoldExpandAvailableArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]UpdateSuperFoldExpandAvailableArea");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateSuperFoldExpandAvailableArea(env, info) : nullptr;
}

napi_value JsScreenSessionManager::SetScreenOffDelayTime(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]SetScreenOffDelayTime");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetScreenOffDelayTime(env, info) : nullptr;
}

napi_value JsScreenSessionManager::SetScreenOnDelayTime(napi_env env, napi_callback_info info)
{
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetScreenOnDelayTime(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyFoldToExpandCompletion(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]NotifyFoldToExpandCompletion");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyFoldToExpandCompletion(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyScreenConnectCompletion(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]NotifyFoldToExpandCompletion");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyScreenConnectCompletion(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyAodOpCompletion(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]NotifyAodOpCompletion");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyAodOpCompletion(env, info) : nullptr;
}

napi_value JsScreenSessionManager::RecordEventFromScb(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]RecordEventFromScb");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnRecordEventFromScb(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetFoldStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetFoldStatus");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetFoldStatus(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetSuperFoldStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetSuperFoldStatus");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetSuperFoldStatus(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetSuperRotation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetSuperRotation");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetSuperRotation(env, info) : nullptr;
}

napi_value JsScreenSessionManager::SetLandscapeLockStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]SetLandscapeLockStatus");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetLandscapeLockStatus(env, info) : nullptr;
}

napi_value JsScreenSessionManager::SetForceCloseHdr(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI] begin");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetForceCloseHdr(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetScreenSnapshot(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetScreenSnapshot");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetScreenSnapshot(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetDeviceScreenConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetDeviceScreenConfig");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetDeviceScreenConfig(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetExtendScreenConnectStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]Get extend screen status");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetExtendScreenConnectStatus(env, info) : nullptr;
}

napi_value JsScreenSessionManager::SetDefaultMultiScreenModeWhenSwitchUser(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]Set to default mode.");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetDefaultMultiScreenModeWhenSwitchUser(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyExtendScreenCreateFinish(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]Notify create finish.");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyExtendScreenCreateFinish(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyExtendScreenDestroyFinish(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]Notify destroy finish.");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyExtendScreenDestroyFinish(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifyScreenMaskAppear(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]Notify mask appear.");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyScreenMaskAppear(env, info) : nullptr;
}

napi_value JsScreenSessionManager::SetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI].");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetPrimaryDisplaySystemDpi(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI].");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetPrimaryDisplaySystemDpi(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI].");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetFoldDisplayMode(env, info) : nullptr;
}

napi_value JsScreenSessionManager::FreezeScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]FreezeScreen");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnFreezeScreen(env, info) : nullptr;
}

napi_value JsScreenSessionManager::GetScreenSnapshotWithAllWindows(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]GetScreenSnapshotWithAllWindows");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetScreenSnapshotWithAllWindows(env, info) : nullptr;
}

napi_value JsScreenSessionManager::NotifySwitchUserAnimationFinish(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]NotifyScreenSwitchUserAnimateFinish");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifySwitchUserAnimationFinish(env, info) : nullptr;
}

napi_value JsScreenSessionManager::RegisterSwitchUserAnimationNotification(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]RegisterSwitchUserAnimationNotification");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnRegisterSwitchUserAnimationNotification(env, info) : nullptr;
}

void JsScreenSessionManager::OnScreenConnected(const sptr<ScreenSession>& screenSession)
{
    if (screenConnectionCallback_ == nullptr || screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]screenConnectionCallback is nullptr");
        return;
    }
    TLOGD(WmsLogTag::DMS, "[NAPI]OnScreenConnected");
    std::shared_ptr<NativeReference> callback_ = screenConnectionCallback_;
    bool isRotationLocked = screenSession->GetUniqueRotationLock();
    int32_t rotation = screenSession->GetUniqueRotation();
    std::map<int32_t, int32_t> uniqueRotationOrientationMap = screenSession->GetUniqueRotationOrientationMap();
    TLOGD(WmsLogTag::DMS, "JsScreenSessionManager converting to JS,"
          "isUniqueRotationLocked: %{public}d, uniqueRotation: %{public}d,"
          "uniqueRotationOrientationMap: %{public}s",
          isRotationLocked, rotation, MapToString(uniqueRotationOrientationMap).c_str());
    auto asyncTask = [this, callback_, screenSession, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSessionManager::OnScreenConnected");
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Object is null!");
            return;
        }
        napi_value jsScreenSession = JsScreenSession::Create(env_, screenSession);
        napi_ref ref = nullptr;
        napi_create_reference(env_, jsScreenSession, 1, &ref);
        jsScreenSessionMap_[screenSession->GetSessionId()] = ref;
        TLOGNI(WmsLogTag::DMS, "screen connection, screenId: %{public}" PRIu64 " sessionId:%{public}" PRIu64,
            screenSession->GetScreenId(), screenSession->GetSessionId());
        napi_set_named_property(env, objValue, "screenSession", jsScreenSession);
        napi_set_named_property(env, objValue, "screenConnectChangeType", CreateJsValue(env, 0));

        napi_value argv[] = { objValue };
        napi_value method = callback_->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Failed to get method callback from object!");
            return;
        }
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_vip, "OnScreenConnected");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnScreenConnected: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnScreenConnected: env is nullptr");
    }
}

void JsScreenSessionManager::OnTentModeChange(const TentMode tentMode)
{
    std::vector<std::shared_ptr<NativeReference>> tentModeChangeCallback;
    {
        std::shared_lock<std::shared_mutex> lock(tentModeChangeCallbackMutex_);
        tentModeChangeCallback = tentModeChangeCallback_;
    }
    if (tentModeChangeCallback.empty()) {
        TLOGE(WmsLogTag::DMS, "[NAPI]screenConnectionCallback is nullptr");
        return;
    }
    TLOGD(WmsLogTag::DMS, "[NAPI]begin");
    for (auto& callback : tentModeChangeCallback) {
        TLOGD(WmsLogTag::DMS, "native call, change tent mode to");
        auto asyncTask = [this, callback, tentMode, env = env_]() {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSessionManager::OnTentModeChange");
            napi_value tentModeNapiVal = CreateJsValue(env, static_cast<std::uint32_t>(tentMode));
            napi_value argv[] = { tentModeNapiVal };
            napi_value method = callback->GetNapiValue();
            if (method == nullptr) {
                TLOGNE(WmsLogTag::DMS, "Failed to get method callback from object!");
                return;
            }
            napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
        };
        if (env_ != nullptr) {
            napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_vip, "OnTentModeChange");
            if (ret != napi_status::napi_ok) {
                TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
            }
        } else {
            TLOGE(WmsLogTag::DMS, "env is nullptr");
        }
    }
}

napi_value JsScreenSessionManager::SetCameraStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]SetCameraStatus");
    JsScreenSessionManager* me = CheckParamsAndGetThis<JsScreenSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetCameraStatus(env, info) : nullptr;
}

void JsScreenSessionManager::OnScreenDisconnected(const sptr<ScreenSession>& screenSession)
{
    if (screenConnectionCallback_ == nullptr || screenSession == nullptr) {
        return;
    }
    TLOGD(WmsLogTag::DMS, "[NAPI]OnScreenDisconnected");
    std::shared_ptr<NativeReference> callback_ = screenConnectionCallback_;
    auto asyncTask = [this, callback_, screenSession, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSessionManager::OnScreenDisconnected");
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Object is null!");
            return;
        }
        napi_ref jsScreenSessionRef = jsScreenSessionMap_[screenSession->GetSessionId()];
        if (jsScreenSessionRef == nullptr) {
            TLOGE(WmsLogTag::DMS, "js screen session ref invalid");
            return;
        }
        napi_value jsScreenSession;
        napi_get_reference_value(env, jsScreenSessionRef, &jsScreenSession);
        napi_delete_reference(env, jsScreenSessionRef);
        jsScreenSessionMap_.erase(screenSession->GetScreenId());
        TLOGW(WmsLogTag::DMS, "screen disconnection, screenId: %{public}" PRIu64 "sessionId:%{public}" PRIu64,
            screenSession->GetScreenId(), screenSession->GetSessionId());
        napi_set_named_property(env, objValue, "screenSession", jsScreenSession);
        napi_set_named_property(env, objValue, "screenConnectChangeType", CreateJsValue(env, 1));

        napi_value argv[] = { objValue };
        napi_value method = callback_->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Failed to get method callback from object!");
            return;
        }
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenDisconnected");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnScreenDisconnected: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnScreenDisconnected: env is nullptr");
    }
}

bool JsScreenSessionManager::OnTakeOverShutdown(const PowerMgr::TakeOverInfo& info)
{
    if (!shutdownCallback_) {
        return false;
    }
    TLOGD(WmsLogTag::DMS, "[NAPI]OnTakeOverShutdown");
    std::shared_ptr<NativeReference> callback_ = shutdownCallback_;
    auto asyncTask = [callback_, info, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSessionManager::OnTakeOverShutdown");
        napi_value argv[] = {CreateJsValue(env, info.reason_), CreateJsValue(env, info.intfParam_)};
        napi_value method = callback_->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Failed to get method callback from object!");
            return;
        }
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnTakeOverShutdown");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnTakeOverShutdown: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnTakeOverShutdown: env is nullptr");
    }
    return true;
}

napi_value JsScreenSessionManager::OnRegisterShutdownCallback(napi_env env, const napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "[NAPI]OnRegisterShutdownCallback");
    if (shutdownCallback_ != nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to register callback, callback exits");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_REPEAT_OPERATION)));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[0];
    if (!NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::DMS, "Failed to register callback, param is not callable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(result));
    shutdownCallback_ = callbackRef;
#ifdef POWER_MANAGER_ENABLE
    sptr<PowerMgr::ITakeOverShutdownCallback> callback(this);
    PowerMgr::ShutdownClient::GetInstance().RegisterShutdownCallback(callback, PowerMgr::ShutdownPriority::LOW);
#else
    TLOGD(WmsLogTag::DMS, "Can not find the sub system of PowerMgr");
#endif
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnUnRegisterShutdownCallback(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnUnRegisterShutdownCallback");
    if (shutdownCallback_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to unregister callback, callback is not exits");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_NOT_REGISTER_SYNC_CALLBACK)));
        return NapiGetUndefined(env);
    }
#ifdef POWER_MANAGER_ENABLE
    sptr<PowerMgr::ITakeOverShutdownCallback> callback(this);
    PowerMgr::ShutdownClient::GetInstance().UnRegisterShutdownCallback(callback);
#else
    TLOGD(WmsLogTag::DMS, "Can not find the sub system of PowerMgr");
#endif
    shutdownCallback_ = nullptr;
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnRegisterCallback(napi_env env, const napi_callback_info info)
{
    std::string callbackType;
    napi_ref callbackRef;
    if (!ObtainCallBackInfo(env, info, callbackType, callbackRef)) {
        TLOGE(WmsLogTag::DMS, "[NAPI] param check fail");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    if (callbackType == ON_SCREEN_CONNECTION_CHANGE_CALLBACK) {
        RegisterScreenConnectionCallback(env, callbackType, callbackRef);
    } else if (callbackType == ON_TENT_MODE_CHANGE_CALLBACK) {
        RegisterTentModeCallback(env, callbackType, callbackRef);
    } else {
        TLOGE(WmsLogTag::DMS, "Unsupported callback type: %{public}s.", callbackType.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
    }
    return NapiGetUndefined(env);
}

void JsScreenSessionManager::RegisterScreenConnectionCallback(napi_env env,
    const std::string& callbackType, napi_ref& callback)
{
    TLOGI(WmsLogTag::DMS, "[NAPI] begin");
    if (screenConnectionCallback_ != nullptr) {
        return;
    }
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(callback));
    screenConnectionCallback_ = callbackRef;
    ScreenSessionManagerClient::GetInstance().RegisterScreenConnectionListener(this);
}

void JsScreenSessionManager::RegisterTentModeCallback(napi_env env,
    const std::string& callbackType, napi_ref& callback)
{
    TLOGI(WmsLogTag::DMS, "[NAPI] begin");
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(callback));
    {
        std::unique_lock<std::shared_mutex> lock(tentModeChangeCallbackMutex_);
        tentModeChangeCallback_.emplace_back(callbackRef);
    }
    ScreenSessionManagerClient::GetInstance().RegisterTentModeChangeListener(this);
}

bool JsScreenSessionManager::ObtainCallBackInfo(napi_env env, const napi_callback_info info,
    std::string& callbackType, napi_ref& callbackRef)
{
    TLOGI(WmsLogTag::DMS, "[NAPI] begin");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: params num
        TLOGE(WmsLogTag::DMS, "Argc is invalid: %{public}zu", argc);
        return false;
    }
    if (!ConvertFromJsValue(env, argv[0], callbackType)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to callback type.");
        return false;
    }
    napi_value value = argv[1];
    if (!NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::DMS, "Failed to register callback, callback is not callable!");
        return false;
    }

    napi_create_reference(env, value, 1, &callbackRef);
    return true;
}

napi_value JsScreenSessionManager::OnUpdateScreenRotationProperty(napi_env env,
    const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnUpdateScreenRotationProperty");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) { // at least 3: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    RRect bounds;
    napi_value nativeObj = argv[1];
    if (nativeObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert object to RRect bounds");
        return NapiGetUndefined(env);
    } else if (!ConvertRRectFromJs(env, nativeObj, bounds)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to get bounds from js object");
        return NapiGetUndefined(env);
    }
    ScreenDirectionInfo directionInfo;
    napi_value nativeObject = argv[2];
    if (nativeObject == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to ScreenDirectionInfo,the param is null");
        return NapiGetUndefined(env);
    }
    if (!ConvertScreenDirectionInfoFromJs(env, nativeObject, directionInfo)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to ScreenDirectionInfo");
        return NapiGetUndefined(env);
    }
    ScreenPropertyChangeType type = ScreenPropertyChangeType::UNSPECIFIED;
    if (argc > ARGC_THREE) {
        if (!ConvertFromJsValue(env, argv[ARGC_THREE], type) || type < ScreenPropertyChangeType::UNSPECIFIED ||
            type >= ScreenPropertyChangeType::UNDEFINED) {
            TLOGE(WmsLogTag::DMS, "[NAPI]screenPropertyChangeType is invalid");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }
    ScreenSessionManagerClient::GetInstance().UpdateScreenRotationProperty(screenId, bounds, directionInfo,
        type);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnUpdateServerScreenProperty(napi_env env,
    const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnUpdateServerScreenProperty");
    size_t argc = 3;
    napi_value argv[3] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 3) { // 3: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId = INVALID_ID;
    if (!ConvertFromJsValue(env, argv[0], screenId) || screenId < 0) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t rotation;
    if (!ConvertFromJsValue(env, argv[1], rotation)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to rotation");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    RRect bounds;
    napi_value nativeObj = argv[2];
    if (nativeObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to get bounds from js object");
        return NapiGetUndefined(env);
    } else if (!ConvertRRectFromJs(env, nativeObj, bounds)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to bounds");
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().OnScreenPropertyChanged(screenId, static_cast<float>(rotation), bounds);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyScreenLockEvent(napi_env env,
    const napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "[NAPI]OnNotifyScreenLockEvent");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t event;
    if (!ConvertFromJsValue(env, argv[0], event)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to display event");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    DisplayManager::GetInstance().NotifyDisplayEvent(static_cast<DisplayEvent>(event));
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnGetCurvedCompressionArea(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetCurvedCompressionArea");
    napi_value result = nullptr;
    napi_create_uint32(env, ScreenSessionManagerClient::GetInstance().GetCurvedCompressionArea(), &result);
    return result;
}

napi_value JsScreenSessionManager::OnGetPhyScreenProperty(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetPhyScreenProperty");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenProperty screenProperty = ScreenSessionManagerClient::GetInstance().GetPhyScreenProperty(screenId);
    return JsScreenUtils::CreateJsScreenProperty(env, screenProperty);
}

napi_value JsScreenSessionManager::OnUpdateAvailableArea(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnUpdateAvailableArea");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    DMRect area;
    napi_value nativeObj = argv[1];
    if (nativeObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect,the param is null");
        return NapiGetUndefined(env);
    } else if (!ConvertDMRectFromJs(env, nativeObj, area)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect");
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().UpdateAvailableArea(screenId, area);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnUpdateSuperFoldAvailableArea(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnUpdateSuperFoldAvailableArea");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    DMRect bArea;
    napi_value bNativeObj = argv[1];
    if (bNativeObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect,the param is null");
        return NapiGetUndefined(env);
    } else if (!ConvertDMRectFromJs(env, bNativeObj, bArea)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect");
        return NapiGetUndefined(env);
    }
    DMRect cArea;
    napi_value cNativeObj = argv[2];
    if (cNativeObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect,the param is null");
        return NapiGetUndefined(env);
    } else if (!ConvertDMRectFromJs(env, cNativeObj, cArea)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect");
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().UpdateSuperFoldAvailableArea(screenId, bArea, cArea);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnExtraDestroyScreen(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnExtraDestroyScreen");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().ExtraDestroyScreen(static_cast<ScreenId>(screenId));
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnUpdateSuperFoldExpandAvailableArea(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnUpdateSuperFoldExpandAvailableArea");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    DMRect area;
    napi_value nativeObj = argv[1];
    if (nativeObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect,the param is null");
        return NapiGetUndefined(env);
    } else if (!ConvertDMRectFromJs(env, nativeObj, area)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to DMRect");
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().UpdateSuperFoldExpandAvailableArea(screenId, area);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnSetScreenOffDelayTime(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnSetScreenOffDelayTime");
    size_t argc = 1;
    napi_handle_scope scope = nullptr;
    auto status = napi_open_handle_scope(env, &scope);
    if ((status != napi_ok) || (scope == nullptr)) {
        TLOGW(WmsLogTag::DMS, "[NAPI]napi_open_handle_scope fail");
        return NapiGetUndefined(env);
    }
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    int32_t delay;
    if (!ConvertFromJsValue(env, argv[0], delay)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to delay");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().SetScreenOffDelayTime(delay);
    napi_close_handle_scope(env, scope);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnSetScreenOnDelayTime(napi_env env, const napi_callback_info info)
{
    size_t argc = 1;
    napi_handle_scope scope = nullptr;
    auto status = napi_open_handle_scope(env, &scope);
    if ((status != napi_ok) || (scope == nullptr)) {
        TLOGW(WmsLogTag::DMS, "[NAPI]napi_open_handle_scope fail");
        return NapiGetUndefined(env);
    }
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    int32_t delay;
    if (!ConvertFromJsValue(env, argv[0], delay)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to delay");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().SetScreenOnDelayTime(delay);
    napi_close_handle_scope(env, scope);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyFoldToExpandCompletion(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnNotifyFoldToExpandCompletion");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool foldToExpand;
    if (!ConvertFromJsValue(env, argv[0], foldToExpand)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to foldToExpand");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().NotifyFoldToExpandCompletion(foldToExpand);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyScreenConnectCompletion(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnNotifyScreenConnectCompletion");
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId) || (screenId < 0)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().NotifyScreenConnectCompletion(static_cast<uint64_t>(screenId));
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyAodOpCompletion(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]Enter");
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    AodOP op;
    int32_t result;
    if (!ConvertFromJsValue(env, argv[0], op) || (op >= AodOP::AOD_OP_MAX)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to aod operation");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (!ConvertFromJsValue(env, argv[1], result)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to aod operation result");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().NotifyAodOpCompletion(op, result);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnRecordEventFromScb(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnRecordEventFromScb");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string description;
    if (!ConvertFromJsValue(env, argv[0], description)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to description");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool needRecordEvent;
    if (!ConvertFromJsValue(env, argv[1], needRecordEvent)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to needRecordEvent");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().RecordEventFromScb(description, needRecordEvent);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnGetFoldStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetFoldStatus");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 1) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    FoldStatus status = ScreenSessionManagerClient::GetInstance().GetFoldStatus();
    TLOGI(WmsLogTag::DMS, "[NAPI]" PRIu64", getFoldStatus = %{public}u", status);
    return CreateJsValue(env, status);
}

napi_value JsScreenSessionManager::OnGetSuperFoldStatus(napi_env env, napi_callback_info info)
{
    SuperFoldStatus status = ScreenSessionManagerClient::GetInstance().GetSuperFoldStatus();
    TLOGI(WmsLogTag::DMS, "[NAPI]" PRIu64", getSuperFoldStatus = %{public}u", status);
    return CreateJsValue(env, status);
}

napi_value JsScreenSessionManager::OnGetSuperRotation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetSuperRotation");
    float rotation = ScreenSessionManagerClient::GetInstance().GetSuperRotation();
    TLOGI(WmsLogTag::DMS, "[NAPI] getSuperRotation = %{public}f", rotation);
    return CreateJsValue(env, rotation);
}

napi_value JsScreenSessionManager::OnSetLandscapeLockStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnSetLandscapeLockStatus");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isLocked;
    if (!ConvertFromJsValue(env, argv[0], isLocked)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to isLocked");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().SetLandscapeLockStatus(isLocked);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnSetForceCloseHdr(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isForceCloseHdr;
    if (!ConvertFromJsValue(env, argv[1], isForceCloseHdr)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to isForceCloseHdr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().SetForceCloseHdr(screenId, isForceCloseHdr);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnGetScreenSnapshot(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetScreenSnapshot");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::array<double, ARGC_TWO> scaleParam;
    for (uint8_t i = 0; i < ARGC_TWO; i++) {
        if (!ConvertFromJsValue(env, argv[i + 1], scaleParam[i])) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to scale[%d]", i + 1);
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
        scaleParam[i] = (scaleParam[i] > 0.0 && scaleParam[i] < 1.0) ? scaleParam[i] : 1.0;
    }
    napi_value nativeData = nullptr;
    auto pixelMap = ScreenSessionManagerClient::GetInstance().GetScreenSnapshot(screenId,
        static_cast<float>(scaleParam[0]), static_cast<float>(scaleParam[1]));
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]pixelMap is nullptr");
        return nativeData;
    }
    nativeData = Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
    if (nativeData != nullptr) {
        TLOGD(WmsLogTag::DMS, "[NAPI]pixelmap W x H = %{public}d x %{public}d", pixelMap->GetWidth(),
            pixelMap->GetHeight());
    } else {
        TLOGE(WmsLogTag::DMS, "[NAPI]create native pixelmap failed");
    }
    return nativeData;
}

napi_value JsScreenSessionManager::OnGetDeviceScreenConfig(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetDeviceScreenConfig");
    DeviceScreenConfig deviceScreenConfig = ScreenSessionManagerClient::GetInstance().GetDeviceScreenConfig();
    napi_value jsDeviceScreenConfigObj = JsDeviceScreenConfig::CreateDeviceScreenConfig(env, deviceScreenConfig);
    if (jsDeviceScreenConfigObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]jsDeviceScreenConfigObj is nullptr");
        napi_throw(env, CreateJsError(env,
            static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
    }
    return jsDeviceScreenConfigObj;
}

napi_value JsScreenSessionManager::OnSetCameraStatus(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnSetCameraStatus");
    size_t argc = 2;
    napi_handle_scope scope = nullptr;
    auto status = napi_open_handle_scope(env, &scope);
    if ((status != napi_ok) || (scope == nullptr)) {
        TLOGW(WmsLogTag::DMS, "[NAPI]napi_open_handle_scope fail");
        return NapiGetUndefined(env);
    }
    napi_value argv[2] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    int32_t cameraStatus;
    int32_t cameraPosition;
    if (!ConvertFromJsValue(env, argv[0], cameraStatus)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to cameraStatus");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    if (!ConvertFromJsValue(env, argv[1], cameraPosition)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to cameraPosition");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        napi_close_handle_scope(env, scope);
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().SetCameraStatus(cameraStatus, cameraPosition);
    napi_close_handle_scope(env, scope);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnGetExtendScreenConnectStatus(napi_env env, napi_callback_info info)
{
    ExtendScreenConnectStatus status = ScreenSessionManagerClient::GetInstance().GetExtendScreenConnectStatus();
    TLOGI(WmsLogTag::DMS, "[NAPI] OnGetExtendScreenConnectStatus = %{public}u", status);
    return CreateJsValue(env, status);
}

napi_value JsScreenSessionManager::OnSetDefaultMultiScreenModeWhenSwitchUser(napi_env env,
    const napi_callback_info info)
{
    ScreenSessionManagerClient::GetInstance().SetDefaultMultiScreenModeWhenSwitchUser();
    TLOGI(WmsLogTag::DMS, "[NAPI] OnSetDefaultMultiScreenModeWhenSwitchUser");
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyExtendScreenCreateFinish(napi_env env, const napi_callback_info info)
{
    ScreenSessionManagerClient::GetInstance().NotifyExtendScreenCreateFinish();
    TLOGI(WmsLogTag::DMS, "[NAPI] OnNotifyExtendScreenCreateFinish");
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyExtendScreenDestroyFinish(napi_env env, const napi_callback_info info)
{
    ScreenSessionManagerClient::GetInstance().NotifyExtendScreenDestroyFinish();
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnNotifyScreenMaskAppear(napi_env env, const napi_callback_info info)
{
    ScreenSessionManagerClient::GetInstance().NotifyScreenMaskAppear();
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnSetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI].");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double dpi;
    if (!ConvertFromJsNumber(env, argv[0], dpi)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to cameraStatus");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    float floatDpi = static_cast<float>(dpi / BASELINE_DENSITY);
    ScreenSessionManagerClient::GetInstance().SetPrimaryDisplaySystemDpi(floatDpi);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnGetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI].");
    float primaryDisplaySystemDpi = DisplayManager::GetInstance().GetPrimaryDisplaySystemDpi();
    return CreateJsValue(env, primaryDisplaySystemDpi * BASELINE_DENSITY);
}

napi_value JsScreenSessionManager::OnGetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    FoldDisplayMode foldDisplayMode = DisplayManager::GetInstance().GetFoldDisplayMode();
    TLOGI(WmsLogTag::DMS, "[NAPI] foldDisplayMode:%{public}d", foldDisplayMode);
    return CreateJsValue(env, foldDisplayMode);
}

napi_value JsScreenSessionManager::OnFreezeScreen(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnFreezeScreen");
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId = INVALID_ID;
    if (!ConvertFromJsValue(env, argv[0], screenId) || screenId < 0) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isFreeze = false;
    if (!ConvertFromJsValue(env, argv[ARGC_ONE], isFreeze)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to isFreeze");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().FreezeScreen(static_cast<ScreenId>(screenId), isFreeze);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnGetScreenSnapshotWithAllWindows(napi_env env, const napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]OnGetScreenSnapshotWithAllWindows");
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_FOUR) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId = INVALID_ID;
    if (!ConvertFromJsValue(env, argv[0], screenId) || screenId < 0) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::array<double, ARGC_TWO> scaleParam;
    for (uint8_t i = 0; i < ARGC_TWO; i++) {
        if (!ConvertFromJsValue(env, argv[i + 1], scaleParam[i])) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to scale[%d]", i + 1);
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
        scaleParam[i] = MathHelper::Clamp(scaleParam[i], 0.0, 1.0);
    }
    bool isNeedCheckDrmAndSurfaceLock = false;
    if (!ConvertFromJsValue(env, argv[ARGC_THREE], isNeedCheckDrmAndSurfaceLock)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to isNeedCheckDrmAndSurfaceLock");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value nativeData = nullptr;
    auto pixelMap = ScreenSessionManagerClient::GetInstance().GetScreenSnapshotWithAllWindows(
        static_cast<ScreenId>(screenId), static_cast<float>(scaleParam[0]), static_cast<float>(scaleParam[1]),
        isNeedCheckDrmAndSurfaceLock);
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]pixelMap is nullptr");
        return nativeData;
    }
    nativeData = Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
    if (nativeData == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]create native pixelmap failed");
    }
    return nativeData;
}

napi_value JsScreenSessionManager::OnNotifySwitchUserAnimationFinish(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "[NAPI]notify switch user animate finish");
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string description;
    if (!ConvertFromJsValue(env, argv[0], description)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to description");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().NotifySwitchUserAnimationFinish(description);
    return NapiGetUndefined(env);
}

napi_value JsScreenSessionManager::OnRegisterSwitchUserAnimationNotification(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "[NAPI]register animate need notify");
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string description;
    if (!ConvertFromJsValue(env, argv[0], description)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to description");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ScreenSessionManagerClient::GetInstance().RegisterSwitchUserAnimationNotification(description);
    return NapiGetUndefined(env);
}
} // namespace OHOS::Rosen
