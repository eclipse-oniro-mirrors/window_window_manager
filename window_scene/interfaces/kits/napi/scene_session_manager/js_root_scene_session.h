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

#ifndef OHOS_WINDOW_SCENE_JS_ROOT_SCENE_SESSION_H
#define OHOS_WINDOW_SCENE_JS_ROOT_SCENE_SESSION_H

#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>
#include "session/host/include/root_scene_session.h"
#include "js_scene_utils.h"

namespace OHOS::Rosen {
enum class RootListenerFuncType : uint32_t {
    PENDING_SCENE_CB,
    BATCH_PENDING_SCENE_ACTIVE_CB,
};

class JsRootSceneSession : public RefBase {
public:
    JsRootSceneSession(napi_env env, const sptr<RootSceneSession>& rootSceneSession);
    ~JsRootSceneSession() = default;

    static napi_value Create(napi_env env, const sptr<RootSceneSession>& rootSceneSession);
    static void Finalizer(napi_env env, void* data, void* hint);

    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value LoadContent(napi_env env, napi_callback_info info);

private:
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnLoadContent(napi_env env, napi_callback_info info);
    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    void PendingSessionActivation(SessionInfo& info);
    void PendingSessionActivationInner(std::shared_ptr<SessionInfo> sessionInfo);
    sptr<SceneSession> GenSceneSession(SessionInfo& info);
    std::shared_ptr<NativeReference> GetJSCallback(const std::string& functionName) const;
    void ProcessRegisterCallback(RootListenerFuncType rootlistenerFuncType);

    /*
     * Window Lifecycle
     */
    void ProcessPendingSceneSessionActivationRegister();
    void ProcessBatchPendingSceneSessionsActivationRegister();
    void BatchPendingSessionsActivation(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs);
    void BatchPendingSessionsActivationInner(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs);
    static napi_value CreateSessionInfosNapiValue(
        napi_env env, const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos);
    static napi_value CreatePendingInfosNapiValue(
        napi_env env, const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs);
    void SetSceneSessionForPrelaunch(const SessionInfo& info, const sptr<SceneSession>& sceneSession);
    napi_env env_;
    std::map<std::string, std::shared_ptr<NativeReference>> jsCbMap_;
    mutable std::shared_mutex jsCbMapMutex_;
    sptr<RootSceneSession> rootSceneSession_;
    std::shared_ptr<MainThreadScheduler> taskScheduler_;
    void VerifyCallerToken(SessionInfo& info);
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_ROOT_SCENE_SESSION_H
