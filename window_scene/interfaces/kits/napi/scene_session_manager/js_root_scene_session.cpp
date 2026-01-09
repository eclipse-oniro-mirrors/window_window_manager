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
const std::string BATCH_PENDING_SCENE_ACTIVE_CB = "batchPendingSceneSessionsActivation";
const std::unordered_map<std::string, RootListenerFuncType> ListenerFuncMap {
    {PENDING_SCENE_CB,                      RootListenerFuncType::PENDING_SCENE_CB},
    {BATCH_PENDING_SCENE_ACTIVE_CB,         RootListenerFuncType::BATCH_PENDING_SCENE_ACTIVE_CB},
};
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
        WLOGFE("Object is null!");
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        WLOGFE("Invalid argument");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto iterFunctionType = ListenerFuncMap.find(cbType);
    if (iterFunctionType == ListenerFuncMap.end()) {
        TLOGE(WmsLogTag::WMS_MAIN, "callback type is not supported, type=%{public}s", cbType.c_str());
        return NapiGetUndefined(env);
    }
    RootListenerFuncType rootlistenerFuncType = iterFunctionType->second;
    if (IsCallbackRegistered(env, cbType, value)) {
        return NapiGetUndefined(env);
    }
    if (rootSceneSession_ == nullptr) {
        WLOGFE("Root session is nullptr");
        napi_throw(env, CreateJsError(
            env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "Root scene session is null!"));
        return NapiGetUndefined(env);
    }
    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsRootSceneSession set jsCbMap[%s]", cbType.c_str());
        std::unique_lock<std::shared_mutex> lock(jsCbMapMutex_);
        jsCbMap_[cbType] = callbackRef;
    }
    ProcessRegisterCallback(rootlistenerFuncType);
    WLOGFD("End, type=%{public}s", cbType.c_str());
    return NapiGetUndefined(env);
}

void JsRootSceneSession::ProcessRegisterCallback(RootListenerFuncType rootlistenerFuncType)
{
    switch (static_cast<uint32_t>(rootlistenerFuncType)) {
        case static_cast<uint32_t>(RootListenerFuncType::PENDING_SCENE_CB):
            ProcessPendingSceneSessionActivationRegister();
            break;
        case static_cast<uint32_t>(RootListenerFuncType::BATCH_PENDING_SCENE_ACTIVE_CB):
            ProcessBatchPendingSceneSessionsActivationRegister();
            break;
        default:
            break;
    }
}
 
void JsRootSceneSession::ProcessPendingSceneSessionActivationRegister()
{
    rootSceneSession_->SetPendingSessionActivationEventListener([this](SessionInfo& info) {
        this->PendingSessionActivation(info);
    });
}
 
void JsRootSceneSession::ProcessBatchPendingSceneSessionsActivationRegister()
{
    rootSceneSession_->SetBatchPendingSessionsActivationEventListener([this]
        (const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs) {
        this->BatchPendingSessionsActivation(sessionInfos, configs);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

napi_value JsRootSceneSession::OnLoadContent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string contentUrl;
    napi_value context = argv[1];
    napi_value storage = argc < 3 ? nullptr : argv[2];
    if (!ConvertFromJsValue(env, argv[0], contentUrl)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to content url");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get context object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    void* pointerResult = nullptr;
    napi_unwrap(env, context, &pointerResult);
    auto contextNativePointer = static_cast<std::weak_ptr<Context>*>(pointerResult);
    if (contextNativePointer == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get context pointer from js object");
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
    napi_value lastParam = nullptr, result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [rootSceneSession = rootSceneSession_, contentUrl, contextWeakPtr, contentStorage, env,
        task = napiAsyncTask] {
        if (rootSceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "rootSceneSession is nullptr");
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
            return;
        }
        auto contextLockPtr = contextWeakPtr.lock().get();
        if (contextLockPtr == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "context lock ptr is nullptr");
            return;
        }
        napi_value nativeStorage = contentStorage ? contentStorage->GetNapiValue() : nullptr;
        rootSceneSession->LoadContent(contentUrl, env, nativeStorage, contextLockPtr);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnLoadContent") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

bool JsRootSceneSession::IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsRootSceneSession::IsCallbackRegistered[%s]", type.c_str());
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGFI("Method %{public}s has not been registered", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_.begin(); iter != jsCbMap_.end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->second->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("Method %{public}s has already been registered", type.c_str());
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
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env_ref, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSessionInfo is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                sessionInfo->persistentId_, LifeCycleTaskType::START);
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        napi_value callResult = nullptr;
        napi_call_function(env_ref, NapiGetUndefined(env_ref),
            jsCallBack->GetNapiValue(), ArraySize(argv), argv, &callResult);
        SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
            sessionInfo->persistentId_, LifeCycleTaskType::START);
        ProcessPendingSessionActivationResult(env_ref, callResult, sessionInfo);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionActivationInner");
}

napi_value JsRootSceneSession::CreateSessionInfosNapiValue(
    napi_env env, const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, sessionInfos.size(), &arrayValue);
 
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
 
    int32_t index = 0;
    for (const auto& sessionInfo : sessionInfos) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "failed to create napi object");
            return NapiGetUndefined(env);
        }
        napi_set_element(env, arrayValue, index++, CreateJsSessionInfo(env, *sessionInfo));
    }
    return arrayValue;
}

napi_value JsRootSceneSession::CreatePendingInfosNapiValue(
    napi_env env, const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, sessionInfos.size(), &arrayValue);
 
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }

    if (configs.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "configs is empty");
        return CreateSessionInfosNapiValue(env, sessionInfos);
    }

    if (sessionInfos.size() != configs.size()) {
        TLOGE(WmsLogTag::WMS_LIFE,
            "The caller Param is illegal parameters.sessionInfo: %{public}zu configs: %{public}zu",
            sessionInfos.size(), configs.size());
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_LIFE, "sessionInfos:%{public}zu", sessionInfos.size());
    int32_t index = 0;
    for (size_t i = 0; i < sessionInfos.size(); i++) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "failed to create napi object");
            return NapiGetUndefined(env);
        }
        napi_set_element(env, arrayValue, index++, CreateJsSessionInfo(env, *(sessionInfos[i]), configs[i]));
    }
    return arrayValue;
}
 
void JsRootSceneSession::BatchPendingSessionsActivationInner(
    const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)
{
    const char* const where = __func__;
    auto task = [jsCallBack = GetJSCallback(BATCH_PENDING_SCENE_ACTIVE_CB), sessionInfos, configs, env = env_, where] {
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSessionInfos = CreatePendingInfosNapiValue(env, sessionInfos, configs);
        if (jsSessionInfos == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s target session info is nullptr", where);
            return;
        }
        napi_value argv[] = {jsSessionInfos};
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s task success", where);
        napi_call_function(env, NapiGetUndefined(env),
            jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "BatchPendingSessionsActivationInner");
}

static int32_t GetRealCallerSessionId(const sptr<SceneSession>& sceneSession)
{
    int32_t realCallerSessionId = SceneSessionManager::GetInstance().GetFocusedSessionId();
    if (realCallerSessionId == sceneSession->GetPersistentId()) {
        TLOGI(WmsLogTag::WMS_LIFE, "Caller is self, switch to self caller.");
        realCallerSessionId = sceneSession->GetSessionInfo().callerPersistentId_;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Caller session: %{public}d.", realCallerSessionId);
    return realCallerSessionId;
}

void JsRootSceneSession::SetSceneSessionForPrelaunch(const SessionInfo& info, const sptr<SceneSession>& sceneSession)
{
    if (info.isPrelaunch_) {
        sceneSession->SetPrelaunch();
        sceneSession->EditSessionInfo().frameNum_ = info.frameNum_;
    }
}

void JsRootSceneSession::PendingSessionActivation(SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d,bundleName:%{public}s,moduleName:%{public}s,abilityName:%{public}s,"
        "appIndex:%{public}d,reuse:%{public}d,requestId:%{public}d,specifiedFlag:%{public}s,prelaunch:%{public}d,"
        "frameNum:%{public}d",
        info.persistentId_, info.bundleName_.c_str(), info.moduleName_.c_str(), info.abilityName_.c_str(),
        info.appIndex_, info.reuse, info.requestId, info.specifiedFlag_.c_str(), info.isPrelaunch_, info.frameNum_);
    sptr<SceneSession> sceneSession = GenSceneSession(info);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        return;
    }

    SetSceneSessionForPrelaunch(info, sceneSession);
    if (info.want != nullptr) {
        bool isNeedBackToOther = info.want->GetBoolParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, false);
        TLOGI(WmsLogTag::WMS_LIFE, "session: %{public}d isNeedBackToOther: %{public}d",
            sceneSession->GetPersistentId(), isNeedBackToOther);
        if (isNeedBackToOther) {
            info.callerPersistentId_ = GetRealCallerSessionId(sceneSession);
            VerifyCallerToken(info);
        } else {
            info.callerPersistentId_ = INVALID_SESSION_ID;
        }

        auto focusedOnShow = info.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WINDOW_FOCUSED, true);
        sceneSession->SetFocusedOnShow(focusedOnShow);

        std::string continueSessionId = info.want->GetStringParam(Rosen::PARAM_KEY::PARAM_DMS_CONTINUE_SESSION_ID_KEY);
        if (!continueSessionId.empty()) {
            info.continueSessionId_ = continueSessionId;
            TLOGI(WmsLogTag::WMS_LIFE, "continueSessionId from ability manager: %{public}s",
                continueSessionId.c_str());
        }

        // app continue report for distributed scheduled service
        if (info.want->GetIntParam(Rosen::PARAM_KEY::PARAM_DMS_PERSISTENT_ID_KEY, 0) > 0) {
            TLOGI(WmsLogTag::WMS_LIFE, "Continue app with persistentId: %{public}d", info.persistentId_);
            SingletonContainer::Get<DmsReporter>().ReportContinueApp(true, static_cast<int32_t>(WSError::WS_OK));
        }
    } else {
        sceneSession->SetFocusedOnShow(true);
    }

    sceneSession->SetSessionInfo(info);
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    SceneSessionManager::GetInstance().AddRequestTaskInfo(sceneSession, info.requestId);
    auto task = [this, sessionInfo]() {
        PendingSessionActivationInner(sessionInfo);
    };
    sceneSession->PostLifeCycleTask(task, "PendingSessionActivation", LifeCycleTaskType::START);
    if (info.fullScreenStart_) {
        sceneSession->NotifySessionFullScreen(true);
    }
}

void JsRootSceneSession::BatchPendingSessionsActivation(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)
{
    for (auto& info : sessionInfos) {
        if (info == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "sessionInfo is null");
            return;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "bundleName %{public}s, moduleName %{public}s, abilityName %{public}s, "
            "appIndex %{public}d, reuse %{public}d, requestId %{public}d, specifiedFlag %{public}s",
            info->bundleName_.c_str(), info->moduleName_.c_str(),
            info->abilityName_.c_str(), info->appIndex_, info->reuse, info->requestId, info->specifiedFlag_.c_str());
        if (info->want != nullptr) {
            std::string continueSessionId =
                info->want->GetStringParam(Rosen::PARAM_KEY::PARAM_DMS_CONTINUE_SESSION_ID_KEY);
            if (!continueSessionId.empty()) {
                info->continueSessionId_ = continueSessionId;
                TLOGI(WmsLogTag::WMS_LIFE, "continueSessionId from ability manager: %{public}s",
                      continueSessionId.c_str());
            }
 
            // app continue report for distributed scheduled service
            if (info->want->GetIntParam(Rosen::PARAM_KEY::PARAM_DMS_PERSISTENT_ID_KEY, 0) > 0) {
                TLOGI(WmsLogTag::WMS_LIFE, "Continue app with persistentId: %{public}d", info->persistentId_);
                SingletonContainer::Get<DmsReporter>().ReportContinueApp(true, static_cast<int32_t>(WSError::WS_OK));
            }
        }
    }
    BatchPendingSessionsActivationInner(sessionInfos, configs);
}

void JsRootSceneSession::VerifyCallerToken(SessionInfo& info)
{
    auto callerSession = SceneSessionManager::GetInstance().GetSceneSession(info.callerPersistentId_);
    if (callerSession != nullptr) {
        info.isCalledRightlyByCallerId_ = (info.callerToken_ == callerSession->GetAbilityToken()) &&
            SessionPermission::VerifyPermissionByBundleName(info.bundleName_,
                                                            "ohos.permission.CALLED_TRANSITION_ON_LOCK_SCREEN",
                                                            SceneSessionManager::GetInstance().GetCurrentUserId());
        TLOGD(WmsLogTag::WMS_SCB,
            "root isCalledRightlyByCallerId result is: %{public}d", info.isCalledRightlyByCallerId_);
    }
}

sptr<SceneSession> JsRootSceneSession::GenSceneSession(SessionInfo& info)
{
    sptr<SceneSession> sceneSession;
    if (info.persistentId_ == INVALID_SESSION_ID) {
        auto result = SceneSessionManager::GetInstance().CheckIfReuseSession(info);
        if (result == BrokerStates::BROKER_NOT_START) {
            WLOGE("The BrokerStates is not opened");
            return nullptr;
        }

        if (result == BrokerStates::BROKER_STARTED && info.collaboratorType_ == CollaboratorType::REDIRECT_TYPE) {
            TLOGW(WmsLogTag::WMS_LIFE, "redirect and not create session.");
            return nullptr;
        }

        if (info.reuse || info.isAtomicService_ || !info.specifiedFlag_.empty()) {
            if (SceneSessionManager::GetInstance().CheckCollaboratorType(info.collaboratorType_)) {
                sceneSession = SceneSessionManager::GetInstance().FindSessionByAffinity(
                    info.sessionAffinity);
            } else {
                SessionIdentityInfo identityInfo = { info.bundleName_, info.moduleName_, info.abilityName_,
                    info.appIndex_, info.appInstanceKey_, info.windowType_, info.isAtomicService_,
                    info.specifiedFlag_ };
                sceneSession = SceneSessionManager::GetInstance().GetSceneSessionByIdentityInfo(identityInfo);
            }
        }
        if (sceneSession == nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "SceneSession not exist, request a new one.");
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
        sceneSession->SetDefaultDisplayIdIfNeed();
    }
    return sceneSession;
}
} // namespace OHOS::Rosen
