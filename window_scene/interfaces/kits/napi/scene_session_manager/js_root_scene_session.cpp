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

#include "js_root_scene_session.h"
#include "session_manager/include/scene_session_manager.h"

#include "context.h"
#include <js_runtime_utils.h>
#include "window_manager_hilog.h"

#include "js_scene_utils.h"
#include "singleton_container.h"
#include "dms_reporter.h"
#include "common/include/session_permission.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsRootSceneSession" };
const std::string PENDING_SCENE_CB = "pendingSceneSessionActivation";
} // namespace

JsRootSceneSession::JsRootSceneSession(napi_env env, const sptr<RootSceneSession>& rootSceneSession)
    : env_(env), rootSceneSession_(rootSceneSession)
{
    taskScheduler_ = std::make_shared<MainThreadScheduler>(env);
}

napi_value JsRootSceneSession::Create(napi_env env, const sptr<RootSceneSession>& rootSceneSession)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("[NAPI]Object is null!");
        return NapiGetUndefined(env);
    }

    auto jsRootSceneSession = std::make_unique<JsRootSceneSession>(env, rootSceneSession);
    napi_wrap(env, objValue, jsRootSceneSession.release(), JsRootSceneSession::Finalizer, nullptr, nullptr);

    const char* moduleName = "JsRootSceneSession";
    BindNativeFunction(env, objValue, "loadContent", moduleName, JsRootSceneSession::LoadContent);
    BindNativeFunction(env, objValue, "on", moduleName, JsRootSceneSession::RegisterCallback);
    return objValue;
}

void JsRootSceneSession::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGD("Finalizer.");
    std::unique_ptr<JsRootSceneSession>(static_cast<JsRootSceneSession*>(data));
}

napi_value JsRootSceneSession::RegisterCallback(napi_env env, napi_callback_info info)
{
    WLOGD("RegisterCallback.");
    JsRootSceneSession* me = CheckParamsAndGetThis<JsRootSceneSession>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsRootSceneSession::LoadContent(napi_env env, napi_callback_info info)
{
    WLOGD("LoadContent.");
    JsRootSceneSession* me = CheckParamsAndGetThis<JsRootSceneSession>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info) : nullptr;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_value JsRootSceneSession::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("[NAPI]Failed to convert parameter to callbackType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        WLOGFE("[NAPI]Invalid argument");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (IsCallbackRegistered(env, cbType, value)) {
        return NapiGetUndefined(env);
    }
    if (rootSceneSession_ == nullptr) {
        WLOGFE("[NAPI]root session is nullptr");
        napi_throw(env, CreateJsError(
            env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "Root scene session is null!"));
        return NapiGetUndefined(env);
    }

    NotifyPendingSessionActivationFunc func = [this](SessionInfo& info) {
        this->PendingSessionActivation(info);
    };
    rootSceneSession_->SetPendingSessionActivationEventListener(func);
    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsRootSceneSession set jsCbMap[%s]", cbType.c_str());
        std::unique_lock<std::shared_mutex> lock(jsCbMapMutex_);
        jsCbMap_[cbType] = callbackRef;
    }
    WLOGFD("[NAPI]Register end, type = %{public}s", cbType.c_str());
    return NapiGetUndefined(env);
}

napi_value JsRootSceneSession::OnLoadContent(napi_env env, napi_callback_info info)
{
    WLOGD("[NAPI]OnLoadContent");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string contentUrl;
    napi_value context = argv[1];
    napi_value storage = argc < 3 ? nullptr : argv[2];
    if (!ConvertFromJsValue(env, argv[0], contentUrl)) {
        WLOGFE("[NAPI]Failed to convert parameter to content url");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    if (context == nullptr) {
        WLOGFE("[NAPI]Failed to get context object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    void* pointerResult = nullptr;
    napi_unwrap(env, context, &pointerResult);
    auto contextNativePointer = static_cast<std::weak_ptr<Context>*>(pointerResult);
    if (contextNativePointer == nullptr) {
        WLOGFE("[NAPI]Failed to get context pointer from js object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    auto contextWeakPtr = *contextNativePointer;
    SceneSessionManager::GetInstance().SetRootSceneContext(contextWeakPtr);

    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env, storage, 1, &ref);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }

    NapiAsyncTask::CompleteCallback complete = [rootSceneSession = rootSceneSession_,
        contentUrl, contextWeakPtr, contentStorage](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (rootSceneSession == nullptr) {
            WLOGFE("[NAPI]rootSceneSession is nullptr");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
            return;
        }
        napi_value nativeStorage = contentStorage ? contentStorage->GetNapiValue() : nullptr;
        rootSceneSession->LoadContent(contentUrl, env, nativeStorage, contextWeakPtr.lock().get());
    };
    napi_value lastParam = nullptr, result = nullptr;
    NapiAsyncTask::Schedule("JsRootSceneSession::OnLoadContent", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsRootSceneSession::IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsRootSceneSession::IsCallbackRegistered[%s]", type.c_str());
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGFI("[NAPI]Method %{public}s has not been registered", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_.begin(); iter != jsCbMap_.end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->second->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("[NAPI]Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

std::shared_ptr<NativeReference> JsRootSceneSession::GetJSCallback(const std::string& functionName) const
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsRootSceneSession::GetJSCallback[%s]", functionName.c_str());
    std::shared_ptr<NativeReference> jsCallBack = nullptr;
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    auto iter = jsCbMap_.find(functionName);
    if (iter == jsCbMap_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "%{public}s callback not found!", functionName.c_str());
    } else {
        jsCallBack = iter->second;
    }
    return jsCallBack;
}

void JsRootSceneSession::PendingSessionActivationInner(std::shared_ptr<SessionInfo> sessionInfo)
{
    napi_env& env_ref = env_;
    auto task = [sessionInfo, jsCallBack = GetJSCallback(PENDING_SCENE_CB), env_ref]() {
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env_ref, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsSessionInfo is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        TLOGI(WmsLogTag::WMS_LIFE, "pend active success, id:%{public}d",
            sessionInfo->persistentId_);
        napi_call_function(env_ref, NapiGetUndefined(env_ref),
            jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionActivationInner");
}

static int32_t GetRealCallerSessionId(const sptr<SceneSession>& sceneSession)
{
    int32_t realCallerSessionId = SceneSessionManager::GetInstance().GetFocusedSessionId();
    if (realCallerSessionId == sceneSession->GetPersistentId()) {
        TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]caller is self, switch to self caller.");
        realCallerSessionId = sceneSession->GetSessionInfo().callerPersistentId_;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]caller session: %{public}d.", realCallerSessionId);
    return realCallerSessionId;
}

void JsRootSceneSession::PendingSessionActivation(SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]bundleName %{public}s, moduleName %{public}s, abilityName %{public}s, "
        "appIndex %{public}d, reuse %{public}d", info.bundleName_.c_str(), info.moduleName_.c_str(),
        info.abilityName_.c_str(), info.appIndex_, info.reuse);
    sptr<SceneSession> sceneSession = GenSceneSession(info);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        return;
    }

    if (info.want != nullptr) {
        bool isNeedBackToOther = info.want->GetBoolParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, false);
        TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]session: %{public}d isNeedBackToOther: %{public}d",
            sceneSession->GetPersistentId(), isNeedBackToOther);
        if (isNeedBackToOther) {
            info.callerPersistentId_ = GetRealCallerSessionId(sceneSession);
            VerifyCallerToken(info);
        } else {
            info.callerPersistentId_ = 0;
        }

        auto focusedOnShow = info.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WINDOW_FOCUSED, true);
        sceneSession->SetFocusedOnShow(focusedOnShow);

        std::string continueSessionId = info.want->GetStringParam(Rosen::PARAM_KEY::PARAM_DMS_CONTINUE_SESSION_ID_KEY);
        if (!continueSessionId.empty()) {
            info.continueSessionId_ = continueSessionId;
            TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]continueSessionId from ability manager: %{public}s",
                continueSessionId.c_str());
        }

        // app continue report for distributed scheduled service
        if (info.want->GetIntParam(Rosen::PARAM_KEY::PARAM_DMS_PERSISTENT_ID_KEY, 0) > 0) {
            TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]continue app with persistentId: %{public}d", info.persistentId_);
            SingletonContainer::Get<DmsReporter>().ReportContinueApp(true, static_cast<int32_t>(WSError::WS_OK));
        }
    } else {
        sceneSession->SetFocusedOnShow(true);
    }

    sceneSession->SetSessionInfo(info);
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [this, sessionInfo]() {
        PendingSessionActivationInner(sessionInfo);
    };
    sceneSession->PostLifeCycleTask(task, "PendingSessionActivation", LifeCycleTaskType::START);
    if (info.fullScreenStart_) {
        sceneSession->NotifySessionFullScreen(true);
    }
}

void JsRootSceneSession::VerifyCallerToken(SessionInfo& info)
{
    auto callerSession = SceneSessionManager::GetInstance().GetSceneSession(info.callerPersistentId_);
    if (callerSession != nullptr) {
        TLOGI(WmsLogTag::WMS_SCB,
            "update isCalledRightlyByCallerId from :%{public}d to false", info.isCalledRightlyByCallerId_);
        info.isCalledRightlyByCallerId_ = false;
    }
}

sptr<SceneSession> JsRootSceneSession::GenSceneSession(SessionInfo& info)
{
    sptr<SceneSession> sceneSession;
    if (info.persistentId_ == 0) {
        auto result = SceneSessionManager::GetInstance().CheckIfReuseSession(info);
        if (result == BrokerStates::BROKER_NOT_START) {
            WLOGE("[NAPI] The BrokerStates is not opened");
            return nullptr;
        }

        if (info.reuse || info.isAtomicService_) {
            if (SceneSessionManager::GetInstance().CheckCollaboratorType(info.collaboratorType_)) {
                sceneSession = SceneSessionManager::GetInstance().FindSessionByAffinity(
                    info.sessionAffinity);
            } else {
                ComparedSessionInfo compareSessionInfo = { info.bundleName_, info.moduleName_, info.abilityName_,
                    info.appIndex_, info.windowType_, info.isAtomicService_ };
                sceneSession = SceneSessionManager::GetInstance().GetSceneSessionByName(compareSessionInfo);
            }
        }
        if (sceneSession == nullptr) {
            WLOGFI("GetSceneSessionByName return nullptr, RequestSceneSession");
            sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(info);
            if (sceneSession == nullptr) {
                WLOGFE("RequestSceneSession return nullptr");
                return sceneSession;
            }
        }
        info.persistentId_ = sceneSession->GetPersistentId();
        sceneSession->SetSessionInfoPersistentId(sceneSession->GetPersistentId());
        sceneSession->SetDefaultDisplayIdIfNeed();
    } else {
        sceneSession = SceneSessionManager::GetInstance().GetSceneSession(info.persistentId_);
        if (sceneSession == nullptr) {
            WLOGFE("GetSceneSession return nullptr");
            sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(info);
            if (sceneSession == nullptr) {
                WLOGFE("retry RequestSceneSession return nullptr");
                return sceneSession;
            }
            info.persistentId_ = sceneSession->GetPersistentId();
            sceneSession->SetSessionInfoPersistentId(sceneSession->GetPersistentId());
        }
    }
    return sceneSession;
}
} // namespace OHOS::Rosen
