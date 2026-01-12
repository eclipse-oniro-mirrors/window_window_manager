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

#ifndef OHOS_WINDOW_SCENE_JS_SCREEN_SESSION_MANAGER_H
#define OHOS_WINDOW_SCENE_JS_SCREEN_SESSION_MANAGER_H

#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>

#include "screen_session_manager_client.h"
#include "js_screen_utils.h"

#ifdef POWER_MANAGER_ENABLE
#include "shutdown/takeover_shutdown_callback_stub.h"
#endif

namespace OHOS::Rosen {
class JsScreenSessionManager final : public IScreenConnectionListener, public ITentModeListener,
    public PowerMgr::TakeOverShutdownCallbackStub {
public:
    explicit JsScreenSessionManager(napi_env env);
    ~JsScreenSessionManager();

    static napi_value Init(napi_env env, napi_value exportObj);
    static void Finalizer(napi_env env, void* data, void* hint);

    void OnScreenConnected(const sptr<ScreenSession>& screenSession) override;
    void OnScreenDisconnected(const sptr<ScreenSession>& screenSession) override;
    void OnTentModeChange(const TentMode tentMode) override;
    bool OnTakeOverShutdown(const PowerMgr::TakeOverInfo& info) override;

private:
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value UpdateScreenRotationProperty(napi_env env, napi_callback_info info);
    static napi_value UpdateServerScreenProperty(napi_env env, napi_callback_info info);
    static napi_value GetCurvedCompressionArea(napi_env env, napi_callback_info info);
    static napi_value RegisterShutdownCallback(napi_env env, napi_callback_info info);
    static napi_value UnRegisterShutdownCallback(napi_env env, napi_callback_info info);
    static napi_value GetPhyScreenProperty(napi_env env, napi_callback_info info);
    static napi_value NotifyScreenLockEvent(napi_env env, napi_callback_info info);
    static napi_value UpdateAvailableArea(napi_env env, napi_callback_info info);
    static napi_value ExtraDestroyScreen(napi_env env, napi_callback_info info);
    static napi_value UpdateSuperFoldAvailableArea(napi_env env, napi_callback_info info);
    static napi_value UpdateSuperFoldExpandAvailableArea(napi_env env, napi_callback_info info);
    static napi_value SetScreenOffDelayTime(napi_env env, napi_callback_info info);
    static napi_value SetScreenOnDelayTime(napi_env env, napi_callback_info info);
    static napi_value SetForceCloseHdr(napi_env env, napi_callback_info info);
    static napi_value NotifyFoldToExpandCompletion(napi_env env, napi_callback_info info);
    static napi_value NotifyScreenConnectCompletion(napi_env env, napi_callback_info info);
    static napi_value NotifyAodOpCompletion(napi_env env, napi_callback_info info);
    static napi_value RecordEventFromScb(napi_env env, napi_callback_info info);
    static napi_value SetCameraStatus(napi_env env, napi_callback_info info);
    static napi_value GetFoldStatus(napi_env env, napi_callback_info info);
    static napi_value GetSuperFoldStatus(napi_env env, napi_callback_info info);
    static napi_value GetSuperRotation(napi_env env, napi_callback_info info);
    static napi_value SetLandscapeLockStatus(napi_env env, napi_callback_info info);
    static napi_value GetScreenSnapshot(napi_env env, napi_callback_info info);
    static napi_value GetDeviceScreenConfig(napi_env env, napi_callback_info info);
    static napi_value GetExtendScreenConnectStatus(napi_env env, napi_callback_info info);
    static napi_value SetDefaultMultiScreenModeWhenSwitchUser(napi_env env, napi_callback_info info);
    static napi_value NotifyExtendScreenCreateFinish(napi_env env, napi_callback_info info);
    static napi_value NotifyExtendScreenDestroyFinish(napi_env env, napi_callback_info info);
    static napi_value NotifyScreenMaskAppear(napi_env env, napi_callback_info info);
    static napi_value SetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info);
    static napi_value GetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info);
    static napi_value GetFoldDisplayMode(napi_env env, napi_callback_info info);
    static napi_value FreezeScreen(napi_env env, napi_callback_info info);
    static napi_value GetScreenSnapshotWithAllWindows(napi_env env, napi_callback_info info);
    static napi_value NotifySwitchUserAnimationFinish(napi_env env, napi_callback_info info);
    static napi_value RegisterSwitchUserAnimationNotification(napi_env env, napi_callback_info info);

    napi_value OnRegisterCallback(napi_env env, const napi_callback_info info);
    void RegisterScreenConnectionCallback(napi_env env, const std::string& callbackType, napi_ref& callbackRef);
    void RegisterTentModeCallback(napi_env env, const std::string& callbackType, napi_ref& callbackRef);
    napi_value OnUpdateScreenRotationProperty(napi_env env, const napi_callback_info info);
    napi_value OnUpdateServerScreenProperty(napi_env env, const napi_callback_info info);
    napi_value OnGetCurvedCompressionArea(napi_env env, const napi_callback_info info);
    napi_value OnRegisterShutdownCallback(napi_env env, const napi_callback_info info);
    napi_value OnUnRegisterShutdownCallback(napi_env env, const napi_callback_info info);
    napi_value OnGetPhyScreenProperty(napi_env env, const napi_callback_info info);
    napi_value OnNotifyScreenLockEvent(napi_env env, const napi_callback_info info);
    napi_value OnUpdateAvailableArea(napi_env env, const napi_callback_info info);
    napi_value OnExtraDestroyScreen(napi_env env, const napi_callback_info info);
    napi_value OnUpdateSuperFoldAvailableArea(napi_env env, const napi_callback_info info);
    napi_value OnUpdateSuperFoldExpandAvailableArea(napi_env env, const napi_callback_info info);
    napi_value OnSetScreenOffDelayTime(napi_env env, const napi_callback_info info);
    napi_value OnSetScreenOnDelayTime(napi_env env, const napi_callback_info info);
    napi_value OnSetCameraStatus(napi_env env, napi_callback_info info);
    napi_value OnSetForceCloseHdr(napi_env env, const napi_callback_info info);
    napi_value OnNotifyFoldToExpandCompletion(napi_env env, const napi_callback_info info);
    napi_value OnNotifyScreenConnectCompletion(napi_env env, const napi_callback_info info);
    napi_value OnNotifyAodOpCompletion(napi_env env, const napi_callback_info info);
    napi_value OnRecordEventFromScb(napi_env env, const napi_callback_info info);
    napi_value OnGetFoldStatus(napi_env env, const napi_callback_info info);
    napi_value OnGetSuperFoldStatus(napi_env env, const napi_callback_info info);
    napi_value OnGetSuperRotation(napi_env env, const napi_callback_info info);
    napi_value OnSetLandscapeLockStatus(napi_env env, const napi_callback_info info);
    napi_value OnGetScreenSnapshot(napi_env env, const napi_callback_info info);
    napi_value OnGetDeviceScreenConfig(napi_env env, const napi_callback_info info);
    napi_value OnGetExtendScreenConnectStatus(napi_env env, napi_callback_info info);
    napi_value OnSetDefaultMultiScreenModeWhenSwitchUser(napi_env env, napi_callback_info info);
    napi_value OnNotifyExtendScreenCreateFinish(napi_env env, const napi_callback_info info);
    napi_value OnNotifyExtendScreenDestroyFinish(napi_env env, napi_callback_info info);
    napi_value OnNotifyScreenMaskAppear(napi_env env, napi_callback_info info);
    napi_value OnSetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info);
    napi_value OnGetPrimaryDisplaySystemDpi(napi_env env, napi_callback_info info);
    napi_value OnGetFoldDisplayMode(napi_env env, napi_callback_info info);
    napi_value OnFreezeScreen(napi_env env, napi_callback_info info);
    napi_value OnGetScreenSnapshotWithAllWindows(napi_env env, napi_callback_info info);
    napi_value OnNotifySwitchUserAnimationFinish(napi_env env, napi_callback_info info);

    napi_value OnRegisterSwitchUserAnimationNotification(napi_env env, napi_callback_info info);
    bool ObtainCallBackInfo(napi_env env, const napi_callback_info info,
        std::string& callbackType, napi_ref& callbackRef);

    std::shared_ptr<NativeReference> screenConnectionCallback_;
    std::vector<std::shared_ptr<NativeReference>> tentModeChangeCallback_;
    std::shared_ptr<NativeReference> shutdownCallback_;
    napi_env env_;
    std::map<uint64_t, napi_ref> jsScreenSessionMap_;
    std::shared_mutex tentModeChangeCallbackMutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCREEN_SESSION_MANAGER_H
