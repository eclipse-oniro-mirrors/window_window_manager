/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "float_view_controller.h"

#include <set>
#include <map>

#include "float_view_manager.h"
#include "float_window_manager.h"
#include "floating_ball_manager.h"
#include "picture_in_picture_manager.h"
#include "window_manager_hilog.h"
#include "singleton_container.h"
#include "window_adapter.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string FV_WINDOW_NAME = "float_view_window";
const std::set<FvWindowState> STATE_WITH_WINDOW = {
    FvWindowState::FV_STATE_STARTED,
    FvWindowState::FV_STATE_HIDDEN,
    FvWindowState::FV_STATE_IN_SIDEBAR,
    FvWindowState::FV_STATE_IN_FLOATING_BALL,
};

const std::map<FvWindowState, FloatViewState> STATE_TO_STATE = {
    {FvWindowState::FV_STATE_STARTED, FloatViewState::FV_STARTED},
    {FvWindowState::FV_STATE_HIDDEN, FloatViewState::FV_HIDDEN},
    {FvWindowState::FV_STATE_STOPPED, FloatViewState::FV_STOPPED},
    {FvWindowState::FV_STATE_IN_SIDEBAR, FloatViewState::FV_IN_SIDEBAR},
    {FvWindowState::FV_STATE_IN_FLOATING_BALL, FloatViewState::FV_IN_FLOATING_BALL},
};
}
FloatViewController::FloatViewController(const FvOption &option, napi_env env)
    : weakRef_(this), option_(option), env_(env), type_(APIType::NAPI)
{
    curState_ = FvWindowState::FV_STATE_UNDEFINED;
    id_ = FloatWindowManager::GetControllerId();
    TLOGI(WmsLogTag::WMS_SYSTEM, "FloatViewController created, id: %{public}s", id_.c_str());
}

FloatViewController::FloatViewController(const FvOption &option, ani_env* env)
    : weakRef_(this), option_(option), type_(APIType::ANI)
{
    if (env && (env->GetVM(&vm_) != ANI_OK || !vm_)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "FloatViewController for ani get vm failed");
    }
    curState_ = FvWindowState::FV_STATE_UNDEFINED;
    id_ = FloatWindowManager::GetControllerId();
    TLOGI(WmsLogTag::WMS_SYSTEM, "FloatViewController created, id: %{public}s", id_.c_str());
}

FloatViewController::~FloatViewController()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "FloatViewController release, id: %{public}s", id_.c_str());
}

FvWindowState FloatViewController::GetCurState()
{
    std::lock_guard<std::mutex> lock(controllerMutex_);
    return curState_;
}

void FloatViewController::ChangeState(const FvWindowState &newState)
{
    std::lock_guard<std::mutex> lock(controllerMutex_);
    TLOGI(WmsLogTag::WMS_SYSTEM, "change state from %{public}u to %{public}u, window: %{public}u, id: %{public}s",
        curState_, newState, (window_ == nullptr) ? INVALID_WINDOW_ID : window_->GetWindowId(), id_.c_str());
    curState_ = newState;
    auto it = STATE_TO_STATE.find(newState);
    if (it != STATE_TO_STATE.end()) {
        OnStateChange(it->second);
    }
}

bool FloatViewController::IsStateWithWindow(FvWindowState state)
{
    if (STATE_WITH_WINDOW.find(state) != STATE_WITH_WINDOW.end()) {
        return true;
    }
    return false;
}

void FloatViewController::SetBindState(bool isBind)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "FloatViewController SetBindState %{public}d, id: %{public}s", isBind, id_.c_str());
    std::lock_guard<std::mutex> lock(controllerMutex_);
    bindState_ = isBind;
}

bool FloatViewController::IsBind()
{
    std::lock_guard<std::mutex> lock(controllerMutex_);
    return bindState_;
}

void FloatViewController::SetBindWindowId(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "FloatViewController SetBindWindowId %{public}d, id: %{public}s", windowId,
        id_.c_str());
    std::lock_guard<std::mutex> lock(controllerMutex_);
    bindWindowId_ = windowId;
}

void FloatViewController::SetShowWhenCreate(bool showWhenCreate)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "FloatViewController SetShowWhenCreate %{public}d, id: %{public}s", showWhenCreate,
        id_.c_str());
    std::lock_guard<std::mutex> lock(controllerMutex_);
    option_.SetShowWhenCreate(showWhenCreate);
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SetShowWhenCreate failed, window is null, id: %{public}s", id_.c_str());
        return;
    }
    // for restore when sceneboard dead
    window_->UpdateFloatShowWhenCreate(showWhenCreate);
}

WMError FloatViewController::StartFloatView()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "StartFloatView called, bindState_ %{public}d, id: %{public}s", bindState_,
        id_.c_str());
    if (IsBind()) {
        return FloatWindowManager::StartBindFloatView(weakRef_);
    }
    return StartFloatViewSingle();
}

WMError FloatViewController::StartFloatViewSingle(bool showWhenCreate)
{
    auto errorCode = PrepareStartFloatView(showWhenCreate);
    if (errorCode != WMError::WM_OK) {
        return errorCode;
    }
    errorCode = StartFloatViewInner();
    if (errorCode != WMError::WM_OK) {
        curState_ = FvWindowState::FV_STATE_ERROR;
        OnStateChange(FloatViewState::FV_ERROR);
        FloatViewManager::RemoveActiveController(weakRef_);
    }
    return errorCode;
}

WMError FloatViewController::PrepareStartFloatView(bool showWhenCreate)
{
    std::lock_guard<std::mutex> lock(controllerMutex_);
    if (FloatViewManager::HasActiveController() && !FloatViewManager::IsActiveController(weakRef_)) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "StartFloatView abort");
        return WMError::WM_ERROR_FV_START_FAILED;
    }

    if (curState_ == FvWindowState::FV_STATE_STARTING || IsStateWithWindow(curState_)) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "fvWindow state is: %{public}u, id: %{public}u, mainWindow: %{public}u",
            curState_, (window_ == nullptr) ? INVALID_WINDOW_ID : window_->GetWindowId(), mainWindowId_);
        return WMError::WM_ERROR_FV_REPEAT_OPERATION;
    }
    if (curState_ == FvWindowState::FV_STATE_STOPPING) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "fvWindow state is: %{public}u, id: %{public}u, mainWindow: %{public}u",
            curState_, (window_ == nullptr) ? INVALID_WINDOW_ID : window_->GetWindowId(), mainWindowId_);
        return WMError::WM_ERROR_FV_INVALID_STATE;
    }
    curState_ = FvWindowState::FV_STATE_STARTING;
    FloatViewManager::SetActiveController(weakRef_);
    option_.SetShowWhenCreate(showWhenCreate);
    return WMError::WM_OK;
}

WMError FloatViewController::StartFloatViewInner()
{
    {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        WMError errCode = CreateFloatViewWindow();
        if (errCode != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Create fv window failed, err: %{public}u", errCode);
            return errCode;
        }
        errCode = SetFloatViewContext();
        if (errCode != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Set fv window content failed, err: %{public}u", errCode);
            (void)window_->Destroy();
            window_ = nullptr;
            return errCode;
        }
    }
    auto errCode = window_->Show(0, false);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Show fv window failed, err: %{public}u", errCode);
        (void)window_->Destroy();
        window_ = nullptr;
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return WMError::WM_OK;
}

WMError FloatViewController::CreateFloatViewWindow()
{
    auto contextPtr = option_.GetContext();
    if (contextPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Create fv failed, invalid fvOption or mainWindow");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!Window::IsAnyWindowMatchState(WindowState::STATE_SHOWN)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Create fv failed, no shown window");
        return WMError::WM_ERROR_FV_START_FAILED;
    }
    auto windowOption = sptr<WindowOption>::MakeSptr();
    windowOption->SetWindowName(FV_WINDOW_NAME);
    windowOption->SetWindowType(WindowType::WINDOW_TYPE_FV);
    windowOption->SetWindowMode(WindowMode::WINDOW_MODE_FV);
    if (option_.IsRectValid()) {
        windowOption->SetWindowRect(option_.GetRect());
    }
    FloatViewTemplateInfo fvTemplateInfo;
    option_.GetFvTemplateInfo(fvTemplateInfo);
    fvTemplateInfo.isBind_ = bindState_;
    fvTemplateInfo.bindWindowId_ = bindWindowId_;
    fvTemplateInfo.id_ = id_;
    WMError errCode = WMError::WM_OK;
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (context == nullptr) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "Context is invalid when create window");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    sptr<Window> window = FloatWindowManager::CreateFvWindow(windowOption, fvTemplateInfo, context->lock(), errCode,
        weakRef_);
    if (window == nullptr || errCode != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "Window create failed, reason: %{public}d", errCode);
        return errCode == WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS ? errCode : WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    window_ = window;
    return WMError::WM_OK;
}

WMError FloatViewController::SetFloatViewContext()
{
    if (window_ != nullptr && option_.IsUIPathValid()) {
        return SetUIContextInner();
    }
    return WMError::WM_OK;
}

void FloatViewController::WindowLifeCycleListener::AfterDestroyed()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "float view AfterDestroyed");
    FloatViewManager::DoActionClose("AppMainWindowStop");
}

WMError FloatViewController::StopFloatViewFromClient()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "StopFloatViewFromClient called, bindState_ %{public}d, id: %{public}s", bindState_,
        id_.c_str());
    if (IsBind()) {
        return FloatWindowManager::StopBindFloatView(weakRef_);
    }
    return StopFloatViewFromClientSingle();
}

WMError FloatViewController::StopFloatViewFromClientSingle()
{
    {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        TLOGI(WmsLogTag::WMS_SYSTEM, "StopFloatViewFromClient");
        if (curState_ == FvWindowState::FV_STATE_STOPPING ||
            curState_ == FvWindowState::FV_STATE_STOPPED) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Repeat stop request, curState: %{public}u", curState_);
            return WMError::WM_ERROR_FV_REPEAT_OPERATION;
        }
        if (!IsStateWithWindow(curState_)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "curState cannot stop: %{public}u", curState_);
            return WMError::WM_ERROR_FV_INVALID_STATE;
        }
        if (window_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when stop fv");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        stopFromClient_ = true;
        curState_ = FvWindowState::FV_STATE_STOPPING;
    }
    window_->NotifyPrepareCloseFloatView();
    return WMError::WM_OK;
}

WMError FloatViewController::StopFloatView(const std::string& reason)
{
    {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        TLOGI(WmsLogTag::WMS_SYSTEM, "StopFloatView in, id: %{public}s, reason: %{public}s", id_.c_str(),
            reason.c_str());
        if ((!stopFromClient_ && curState_ == FvWindowState::FV_STATE_STOPPING) ||
            curState_ == FvWindowState::FV_STATE_STOPPED) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Repeat stop request, curState: %{public}u", curState_);
            return WMError::WM_ERROR_FV_REPEAT_OPERATION;
        }
        if (window_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when stop fv");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        curState_ = FvWindowState::FV_STATE_STOPPING;
    }
    return DestroyFloatViewWindow(reason);
}

WMError FloatViewController::RestoreMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParams)
{
    {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (curState_ != FvWindowState::FV_STATE_STARTED) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "float view is not created, curState: %{public}u", curState_);
            return WMError::WM_ERROR_FV_INVALID_STATE;
        }
        if (window_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
    }
    return window_->RestoreFloatViewMainWindow(wantParams);
}

WMError FloatViewController::DestroyFloatViewWindow(const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "called, id: %{public}s", id_.c_str());
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when destroy fv");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WMError ret = FloatWindowManager::DestroyFloatWindow(window_);
    if (ret != WMError::WM_OK) {
        curState_ = FvWindowState::FV_STATE_ERROR;
        OnStateChange(FloatViewState::FV_ERROR);
        TLOGE(WmsLogTag::WMS_SYSTEM, "window destroy failed, err:%{public}u", ret);
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    curState_ = FvWindowState::FV_STATE_STOPPED;
    OnStateChange(FloatViewState::FV_STOPPED, reason);
    FloatViewManager::RemoveActiveController(weakRef_);
    window_ = nullptr;
    stopFromClient_ = false;
    bindWindowId_ = INVALID_WINDOW_ID;
    return WMError::WM_OK;
}

WMError FloatViewController::SetUIContext(const std::string &contextUrl,
    const std::shared_ptr<NativeReference>& contentStorage)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "napi SetUIContext called");
    if (type_ != APIType::NAPI) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi controller called by ani");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::lock_guard<std::mutex> lock(controllerMutex_);
    option_.SetUIPath(contextUrl);
    option_.SetStorage(contentStorage);
    return SetUIContextInner();
}

WMError FloatViewController::SetUIContext(const std::string &contextUrl, const ani_object& contentStorage)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "ani SetUIContext called");
    if (type_ != APIType::ANI) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "ani controller called by napi");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::lock_guard<std::mutex> lock(controllerMutex_);
    option_.SetUIPath(contextUrl);
    option_.SetStorage(contentStorage);
    return SetUIContextInner();
}

WMError FloatViewController::SetUIContextInner()
{
    if (window_ == nullptr) {
        if (IsStateWithWindow(curState_)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when SetUIContext");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        return WMError::WM_OK;
    }
    if (type_ == APIType::NAPI) {
        napi_value storage = nullptr;
        auto contentStorage = option_.GetStorage();
        auto contentUrl = option_.GetUIPath();
        if (contentStorage != nullptr) {
            storage = contentStorage->GetNapiValue();
            TLOGI(WmsLogTag::WMS_SYSTEM, "Set UI Context with localStorage");
        }
        napi_env env = static_cast<napi_env>(env_);
        auto errCode = window_->NapiSetUIContent(contentUrl, env, storage, BackupAndRestoreType::NONE);
        if (errCode != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Set fv window content failed, err: %{public}u", errCode);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
    } else if (type_ == APIType::ANI) {
        auto contentUrl = option_.GetUIPath();
        ani_object storage = option_.GetAniStorage();
        ani_env* env = GetEnv();
        auto errCode = window_->AniSetUIContent(contentUrl, env, storage, BackupAndRestoreType::NONE);
        if (errCode != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Set fv window content failed, err: %{public}u", errCode);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
    }
    return WMError::WM_OK;
}

ani_env* FloatViewController::GetEnv() const
{
    if (!vm_) {
        return nullptr;
    }
    if (type_ != APIType::ANI) {
        return nullptr;
    }
    ani_env* env_ = nullptr;
    ani_status ret = vm_->GetEnv(ANI_VERSION_1, &env_);
    if (ret != ANI_OK || !env_) {
        TLOGE(WmsLogTag::WMS_PIP, "PictureInPictureControllerAni Get Env failed ret:%{public}u", ret);
    }
    return env_;
}

WMError FloatViewController::SetVisibilityInApp(bool visibleInApp)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "SetVisibilityInApp called");
    std::lock_guard<std::mutex> lock(controllerMutex_);
    option_.SetVisibilityInApp(visibleInApp);
    if (window_ == nullptr) {
        if (IsStateWithWindow(curState_)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when SetVisibilityInApp");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        TLOGI(WmsLogTag::WMS_SYSTEM, "SetVisibilityInApp when window not created, save info");
        return WMError::WM_OK;
    }
    FloatViewTemplateInfo fvTemplateInfo;
    option_.GetFvTemplateInfo(fvTemplateInfo);
    fvTemplateInfo.id_ = id_;
    auto errCode = window_->UpdateFloatView(fvTemplateInfo);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Update float view failed when set visibility in app, err: %{public}u", errCode);
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    return WMError::WM_OK;
}

WMError FloatViewController::SetWindowSize(const Rect &rect)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "SetWindowSize called");
    std::lock_guard<std::mutex> lock(controllerMutex_);
    option_.SetRect(rect);
    if (window_ == nullptr) {
        if (IsStateWithWindow(curState_)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when SetWindowSize");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        TLOGI(WmsLogTag::WMS_SYSTEM, "SetWindowSize when window not created, save info");
        return WMError::WM_OK;
    }
    FloatViewTemplateInfo fvTemplateInfo;
    option_.GetFvTemplateInfo(fvTemplateInfo);
    fvTemplateInfo.id_ = id_;
    auto errCode = window_->UpdateFloatView(fvTemplateInfo);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Update float view failed when set window size, err: %{public}u", errCode);
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    return WMError::WM_OK;
}

void FloatViewController::SyncWindowInfo(uint32_t windowId, const FloatViewWindowInfo& windowInfo,
    const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "SyncWindowInfo called, id: %{public}s, reason: %{public}s", id_.c_str(),
        reason.c_str());
    {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (window_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when SyncWindowInfo");
            return;
        }
        if (window_->GetWindowId() != windowId) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window id is not matched when SyncWindowInfo, "
            "windowId: %{public}u, infoWindowId: %{public}u", window_->GetWindowId(), windowId);
            return;
        }
        windowInfo_ = windowInfo;
        option_.SetRect(windowInfo_.windowRect_); // update option for UpdateFloatView
    }
    OnRectChange(windowInfo.windowRect_, windowInfo.scale_, reason);
}

void FloatViewController::SyncLimits(uint32_t windowId, const FloatViewLimits& limits)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "SyncLimits called, id: %{public}s", id_.c_str());
    {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (window_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when SyncLimits");
            return;
        }
        if (window_->GetWindowId() != windowId) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "window id is not matched when SyncLimits, "
            "windowId: %{public}u, infoWindowId: %{public}u", window_->GetWindowId(), windowId);
            return;
        }
    }
    OnLimitsChange(limits);
}

FloatViewWindowInfo FloatViewController::GetWindowInfo() const
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "GetWindowInfo called, id: %{public}s", id_.c_str());
    return windowInfo_;
}

sptr<Window> FloatViewController::GetWindow() const
{
    return window_;
}

void FloatViewController::OnStateChange(const FloatViewState& state, std::string stopReason)
{
    std::vector<sptr<IFvStateChangeObserver>> listeners;
    {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        listeners.assign(stateChangeObservers_.begin(), stateChangeObservers_.end());
    }
    for (auto& listener : listeners) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "state change listener is nullptr");
            continue;
        }
        listener->OnStateChange(state, stopReason);
    }
}

void FloatViewController::OnRectChange(const Rect& window, double scale, const std::string& reason)
{
    std::vector<sptr<IFvRectChangeObserver>> listeners;
    {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        listeners.assign(rectChangeObservers_.begin(), rectChangeObservers_.end());
    }
    for (auto& listener : listeners) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "rectangle change listener is nullptr");
            continue;
        }
        listener->OnRectangleChange(window, scale, reason);
    }
}

void FloatViewController::OnLimitsChange(const FloatViewLimits& limits)
{
    std::vector<sptr<IFvLimitsChangeObserver>> listeners;
    {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        listeners.assign(limitsChangeObservers_.begin(), limitsChangeObservers_.end());
    }
    for (auto& listener : listeners) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "limits change listener is nullptr");
            continue;
        }
        listener->OnLimitsChange(limits);
    }
}

WMError FloatViewController::RegisterStateChangeListener(const sptr<IFvStateChangeObserver>& listener)
{
    return RegisterListener(stateChangeObservers_, listener);
}

WMError FloatViewController::UnregisterStateChangeListener(const sptr<IFvStateChangeObserver>& listener)
{
    return UnRegisterListener(stateChangeObservers_, listener);
}

WMError FloatViewController::RegisterRectChangeListener(const sptr<IFvRectChangeObserver>& listener)
{
    return RegisterListener(rectChangeObservers_, listener);
}

WMError FloatViewController::UnregisterRectChangeListener(const sptr<IFvRectChangeObserver>& listener)
{
    return UnRegisterListener(rectChangeObservers_, listener);
}

WMError FloatViewController::RegisterLimitsChangeListener(const sptr<IFvLimitsChangeObserver>& listener)
{
    return RegisterListener(limitsChangeObservers_, listener);
}

WMError FloatViewController::UnregisterLimitsChangeListener(const sptr<IFvLimitsChangeObserver>& listener)
{
    return UnRegisterListener(limitsChangeObservers_, listener);
}

template<typename T>
WMError FloatViewController::RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "listener is nullptr");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Listener already registered");
        return WMError::WM_ERROR_FV_REPEAT_OPERATION;
    }
    holder.emplace_back(listener);
    return WMError::WM_OK;
}

template<typename T>
WMError FloatViewController::UnRegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "listener could not be null");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    holder.erase(std::remove_if(holder.begin(), holder.end(),
        [listener](const sptr<T>& registeredListener) {
            return registeredListener == listener;
        }), holder.end());
    return WMError::WM_OK;
}
} // namespace Rosen
} // namespace OHOS