/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "window_impl.h"

#include <unordered_set>

#include "dm_common.h"
#include "window_manager_hilog.h"
#include "window_helper.h"
#include "window_option.h"
#include "viewport_config.h"
#include "singleton_container.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowImpl"};
constexpr uint32_t API_VERSION_MOD = 1000;
constexpr int32_t API_VERSION_18 = 18;
}
std::map<std::string, std::pair<uint32_t, sptr<Window>>> WindowImpl::windowMap_;
std::map<uint32_t, std::vector<sptr<WindowImpl>>> WindowImpl::subWindowMap_;
std::map<uint32_t, std::vector<sptr<IWindowSystemBarEnableListener>>> WindowImpl::systemBarEnableListeners_;
std::map<uint32_t, std::vector<sptr<IIgnoreViewSafeAreaListener>>> WindowImpl::ignoreSafeAreaListeners_;
std::map<uint32_t, std::vector<sptr<IAvoidAreaChangedListener>>> WindowImpl::avoidAreaChangeListeners_;
std::mutex WindowImpl::globalMutex_;
static int constructorCnt = 0;
static int deConstructorCnt = 0;
WindowImpl::WindowImpl(const sptr<WindowOption>& option)
{
    if (option != nullptr) {
        name_ = option->GetWindowName();
    } else {
        name_ = "main_window";
    }
    WLOGFI("WindowImpl constructorCnt: %{public}d",
        ++constructorCnt);
}

WindowImpl::~WindowImpl()
{
    WLOGFI("windowName: %{public}s, windowId: %{public}d, deConstructorCnt: %{public}d",
        GetWindowName().c_str(), GetWindowId(), ++deConstructorCnt);
    Destroy();
}

void WindowImpl::CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback)
{
    WLOGFI("CreateSurfaceNode");
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = name;
    rsSurfaceNodeConfig.additionalData = reinterpret_cast<void*>(callback);
    surfaceNode_ = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode_ != nullptr) {
        vsyncStation_ = std::make_shared<VsyncStation>(surfaceNode_->GetId());
    }
}

void WindowImpl::SetContentInfoCallback(const ContentInfoCallback& callback)
{
    contentInfoCallback_ = callback;
}

sptr<Window> WindowImpl::Find(const std::string& name)
{
    return nullptr;
}

const std::shared_ptr<AbilityRuntime::Context> WindowImpl::GetContext() const
{
    return context_;
}

sptr<Window> WindowImpl::FindWindowById(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(globalMutex_);
    if (windowMap_.empty()) {
        WLOGFE("Please create mainWindow First!");
        return nullptr;
    }
    for (auto iter = windowMap_.begin(); iter != windowMap_.end(); iter++) {
        if (windowId == iter->second.first) {
            WLOGI("FindWindow id: %{public}u", windowId);
            return iter->second.second;
        }
    }
    WLOGFE("Cannot find Window!");
    return nullptr;
}

sptr<Window> WindowImpl::GetTopWindowWithId(uint32_t mainWinId)
{
    return FindWindowById(mainWinId);
}

sptr<Window> WindowImpl::GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    return nullptr;
}

std::vector<sptr<Window>> WindowImpl::GetSubWindow(uint32_t parentId)
{
    return std::vector<sptr<Window>>();
}

void WindowImpl::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    std::unordered_set<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowCtxSet(
        ignoreWindowContexts.begin(), ignoreWindowContexts.end());
    std::lock_guard<std::mutex> lock(globalMutex_);
    for (const auto& winPair : windowMap_) {
        auto window = winPair.second.second;
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
            continue;
        }
        auto context = window->GetContext();
        if (context == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "context is null, winId: %{public}u", window->GetWindowId());
            continue;
        }
        if (ignoreWindowCtxSet.count(context) == 0) {
            window->UpdateConfiguration(configuration);
        }
    }
}

void WindowImpl::UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "uiContent is null, winId: %{public}d", GetWindowId());
        return;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "winId: %{public}d", GetWindowId());
    uiContent_->UpdateConfigurationSyncForAll(configuration);
}

void WindowImpl::UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    std::lock_guard<std::mutex> lock(globalMutex_);
    for (const auto& winPair : windowMap_) {
        if (auto window = winPair.second.second) {
            window->UpdateConfigurationSync(configuration);
        }
    }
}

std::shared_ptr<RSSurfaceNode> WindowImpl::GetSurfaceNode() const
{
    return surfaceNode_;
}

Rect WindowImpl::GetRect() const
{
    return Rect{0, 0, 0, 0};
}

Rect WindowImpl::GetRequestRect() const
{
    return Rect{0, 0, 0, 0};
}

WindowType WindowImpl::GetType() const
{
    return WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
}

WindowMode WindowImpl::GetWindowMode() const
{
    return windowMode_;
}

float WindowImpl::GetAlpha() const
{
    return 0.0;
}

WindowState WindowImpl::GetWindowState() const
{
    return state_;
}

WMError WindowImpl::SetFocusable(bool isFocusable)
{
    return WMError::WM_OK;
}

bool WindowImpl::GetFocusable() const
{
    return false;
}

WMError WindowImpl::SetTouchable(bool isTouchable)
{
    return WMError::WM_OK;
}

bool WindowImpl::GetTouchable() const
{
    return false;
}

const std::string& WindowImpl::GetWindowName() const
{
    return name_;
}

uint32_t WindowImpl::GetWindowId() const
{
    return windowId_;
}

uint64_t WindowImpl::GetDisplayId() const
{
    return DISPLAY_ID_INVALID;
}

uint32_t WindowImpl::GetWindowFlags() const
{
    return 0;
}

uint32_t WindowImpl::GetRequestWindowModeSupportType() const
{
    return 0;
}

bool WindowImpl::IsMainHandlerAvailable() const
{
    return true;
}

SystemBarProperty WindowImpl::GetSystemBarPropertyByType(WindowType type) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sysBarPropMap_.find(type);
    if (it == sysBarPropMap_.end()) {
        return SystemBarProperty(false, 0x0, 0x0);
    }
    return it->second;
}

WMError WindowImpl::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto avoidAreaPtr = avoidAreaMap_[type];
    if (avoidAreaPtr == nullptr) {
        return WMError::WM_OK;
    }

    avoidArea = *avoidAreaPtr;
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowType(WindowType type)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowMode(WindowMode mode)
{
    windowMode_ = mode;
    return WMError::WM_OK;
}

WMError WindowImpl::SetAlpha(float alpha)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetTransform(const Transform& trans)
{
    return WMError::WM_OK;
}

const Transform& WindowImpl::GetTransform() const
{
    return transform_;
}

WMError WindowImpl::AddWindowFlag(WindowFlag flag)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RemoveWindowFlag(WindowFlag flag)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowFlags(uint32_t flags)
{
    return WMError::WM_OK;
}

void WindowImpl::OnNewWant(const AAFwk::Want& want)
{
    return;
}

WMError WindowImpl::SetUIContentByName(
    const std::string& contentInfo, napi_env env, napi_value storage, AppExecFwk::Ability* ability)
{
    TLOGD(WmsLogTag::WMS_LIFE, "contentInfo: %{public}s", contentInfo.c_str());
    if (uiContent_) {
        uiContent_->Destroy();
    }
    std::unique_ptr<Ace::UIContent> uiContent;
    if (ability != nullptr) {
        uiContent = Ace::UIContent::Create(ability);
    } else {
        uiContent = Ace::UIContent::Create(context_.get(), reinterpret_cast<NativeEngine*>(env));
    }
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "fail to SetUIContentByName");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent->InitializeByName(this, contentInfo, storage);
    uiContent_ = std::move(uiContent);
    NotifySetIgnoreSafeArea(isIgnoreSafeArea_);
    UpdateViewportConfig();
    if (contentInfoCallback_) {
        contentInfoCallback_(contentInfo);
    }
    return WMError::WM_OK;
}

WMError WindowImpl::NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    WLOGFD("NapiSetUIContent: %{public}s", contentInfo.c_str());
    if (uiContent_) {
        uiContent_->Destroy();
    }
    std::unique_ptr<Ace::UIContent> uiContent;
    if (ability != nullptr) {
        uiContent = Ace::UIContent::Create(ability);
    } else {
        uiContent = Ace::UIContent::Create(context_.get(), reinterpret_cast<NativeEngine*>(env));
    }
    if (uiContent == nullptr) {
        WLOGFE("fail to NapiSetUIContent");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (type != BackupAndRestoreType::NONE) {
        uiContent->Restore(this, contentInfo, storage, type == BackupAndRestoreType::CONTINUATION ?
            Ace::ContentInfoType::CONTINUATION : Ace::ContentInfoType::APP_RECOVERY);
    } else {
        uiContent->Initialize(this, contentInfo, storage);
    }
    uiContent_ = std::move(uiContent);
    if (uiContent_ == nullptr) {
        WLOGFE("uiContent_ is NULL");
        return WMError::WM_ERROR_NULLPTR;
    }
    NotifySetIgnoreSafeArea(isIgnoreSafeArea_);
    UpdateViewportConfig();
    if (contentInfoCallback_) {
        contentInfoCallback_(contentInfo);
    }
    return WMError::WM_OK;
}


Ace::UIContent* WindowImpl::GetUIContent() const
{
    WLOGFD("WindowImpl::GetUIContent");
    return uiContent_.get();
}

std::string WindowImpl::GetContentInfo(BackupAndRestoreType type)
{
    return "";
}

bool WindowImpl::IsSupportWideGamut()
{
    return true;
}

void WindowImpl::SetColorSpace(ColorSpace colorSpace)
{
    return;
}

ColorSpace WindowImpl::GetColorSpace()
{
    return ColorSpace::COLOR_SPACE_DEFAULT;
}

std::shared_ptr<Media::PixelMap> WindowImpl::Snapshot()
{
    return nullptr;
}

WMError WindowImpl::SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap)
{
    return WMError::WM_OK;
}

void WindowImpl::DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    return;
}

WMError WindowImpl::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    return SetSpecificBarProperty(type, property);
}

WMError WindowImpl::UpdateSystemBarPropertyForPage(WindowType type,
    const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag)
{
    return SetSpecificBarProperty(type, systemBarProperty);
}

WMError WindowImpl::SetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
{
    WLOGI("Window %{public}u type %{public}u enable:%{public}u, bgColor:%{public}x, Color:%{public}x",
        GetWindowId(), static_cast<uint32_t>(type), property.enable_,
        property.backgroundColor_, property.contentColor_);

    if (GetSystemBarPropertyByType(type) == property) {
        return WMError::WM_OK;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sysBarPropMap_[type] = property;
    }
    NotifySystemBarChange(type, property);
    UpdateViewportConfig();
    return WMError::WM_OK;
}

WMError WindowImpl::UpdateSystemBarProperty(bool status)
{
    bool enable = !status;
    SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    if (statusProperty.enable_ != enable) {
        statusProperty.enable_ = enable;
        SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusProperty);
    }

    SystemBarProperty naviProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    if (naviProperty.enable_ != enable) {
        naviProperty.enable_ = enable;
        SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, naviProperty);
    }

    SystemBarProperty naviIndicatorProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
    if (naviIndicatorProperty.enable_ != enable) {
        naviIndicatorProperty.enable_ = enable;
        SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, naviIndicatorProperty);
    }

    return WMError::WM_OK;
}

WMError WindowImpl::SetStatusBarColorForNavigation(const std::optional<uint32_t> color)
{
    return WMError::WM_OK;
}

WMError WindowImpl::GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties)
{
    return WMError::WM_OK;
}

void WindowImpl::UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
    SystemBarProperty& property)
{
    property.enable_ = systemBarEnable;
    property.enableAnimation_ = systemBarEnableAnimation;
    // isolate on api 18
    if (GetApiTargetVersion() >= API_VERSION_18) {
        property.settingFlag_ |= SystemBarSettingFlag::ENABLE_SETTING;
    }
}

WMError WindowImpl::SetLayoutFullScreen(bool status)
{
    isIgnoreSafeArea_ = status;
    NotifySetIgnoreSafeArea(status);
    UpdateViewportConfig();
    return WMError::WM_OK;
}

WMError WindowImpl::SetFullScreen(bool status)
{
    WLOGI("status: %{public}d", status);
    WMError ret = UpdateSystemBarProperty(status);
    if (ret != WMError::WM_OK) {
        WLOGFE("UpdateSystemBarProperty errCode:%{public}d", static_cast<int32_t>(ret));
    }
    ret = SetLayoutFullScreen(status);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetLayoutFullScreen errCode:%{public}d", static_cast<int32_t>(ret));
    }
    return WMError::WM_OK;
}

WMError WindowImpl::Create(uint32_t parentId, const std::shared_ptr<AbilityRuntime::Context>& context)
{
    WLOGFI("[Client] Window [name:%{public}s] Create", name_.c_str());
    context_ = context;
    sptr<Window> self(this);
    static std::atomic<uint32_t> tempWindowId = 0;
    uint32_t windowId = tempWindowId++; // for test
    windowId_ = windowId;
    std::lock_guard<std::mutex> lock(globalMutex_);
    windowMap_.insert(std::make_pair(name_, std::pair<uint32_t, sptr<Window>>(windowId, self)));
    return WMError::WM_OK;
}

WMError WindowImpl::BindDialogTarget(sptr<IRemoteObject> targetToken)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetDialogBackGestureEnabled(bool isEnabled)
{
    return WMError::WM_OK;
}

WMError WindowImpl::Destroy()
{
    if (uiContent_) {
        uiContent_->Destroy();
    }
    std::lock_guard<std::mutex> lock(globalMutex_);
    windowMap_.erase(GetWindowName());
    return WMError::WM_OK;
}

WMError WindowImpl::UpdateSurfaceNodeAfterCustomAnimation(bool isAdd)
{
    return WMError::WM_OK;
}

void WindowImpl::SetShowWithOptions(bool showWithOptions)
{
    showWithOptions_ = showWithOptions;
}

bool WindowImpl::IsShowWithOptions() const
{
    return showWithOptions_;
}

WMError WindowImpl::Show(uint32_t reason, bool withAnimation, bool withFocus)
{
    return Show(reason, withAnimation, withFocus, false);
}

WMError WindowImpl::Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach)
{
    if (IsShowWithOptions()) {
        SetShowWithOptions(false);
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return WMError::WM_OK;
}

WMError WindowImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    return Hide(reason, withAnimation, isFromInnerkits, false);
}

WMError WindowImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach)
{
    return WMError::WM_OK;
}

WMError WindowImpl::MoveTo(int32_t x, int32_t y, bool isMoveToGlobal, MoveConfiguration moveConfiguration)
{
    return WMError::WM_OK;
}

WMError WindowImpl::Resize(uint32_t width, uint32_t height)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowGravity(WindowGravity gravity, uint32_t percent)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetKeepScreenOn(bool keepScreenOn)
{
    return WMError::WM_OK;
}

bool WindowImpl::IsKeepScreenOn() const
{
    return false;
}

WMError WindowImpl::SetTurnScreenOn(bool turnScreenOn)
{
    return WMError::WM_OK;
}

bool WindowImpl::IsTurnScreenOn() const
{
    return false;
}

WMError WindowImpl::SetBackgroundColor(const std::string& color)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetTransparent(bool isTransparent)
{
    return WMError::WM_OK;
}

bool WindowImpl::IsTransparent() const
{
    return true;
}

WMError WindowImpl::SetBrightness(float brightness)
{
    return WMError::WM_OK;
}

float WindowImpl::GetBrightness() const
{
    return 0.0;
}

WMError WindowImpl::ChangeCallingWindowId(uint32_t windowId)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetPrivacyMode(bool isPrivacyMode)
{
    return WMError::WM_OK;
}

bool WindowImpl::IsPrivacyMode() const
{
    return false;
}

void WindowImpl::SetSystemPrivacyMode(bool isSystemPrivacyMode)
{
}

WMError WindowImpl::SetSnapshotSkip(bool isSkip)
{
    return WMError::WM_OK;
}

WMError WindowImpl::DisableAppWindowDecor()
{
    return WMError::WM_OK;
}

WMError WindowImpl::Maximize()
{
    return WMError::WM_OK;
}

WMError WindowImpl::Minimize()
{
    return WMError::WM_OK;
}

WMError WindowImpl::Recover()
{
    return WMError::WM_OK;
}

WMError WindowImpl::Close()
{
    return WMError::WM_OK;
}

void WindowImpl::StartMove()
{
    return;
}

WMError WindowImpl::RequestFocus() const
{
    return WMError::WM_OK;
}

bool WindowImpl::IsFocused() const
{
    return true;
}

void WindowImpl::SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer)
{
    return;
}

WMError WindowImpl::RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::mutex> lock(globalMutex_);
    WMError ret = RegisterListener(avoidAreaChangeListeners_[GetWindowId()], listener);
    return ret;
}

WMError WindowImpl::UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::mutex> lock(globalMutex_);
    WMError ret = UnregisterListener(avoidAreaChangeListeners_[GetWindowId()], listener);
    return ret;
}

WMError WindowImpl::RegisterDragListener(const sptr<IWindowDragListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterDragListener(const sptr<IWindowDragListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener)
{
    return WMError::WM_OK;
}

void WindowImpl::RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func)
{
    return;
}

WMError WindowImpl::RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterScreenshotListener(const sptr<IScreenshotListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
{
    return WMError::WM_OK;
}

void WindowImpl::RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener)
{
    return;
}

void WindowImpl::UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener)
{
    return;
}

WMError WindowImpl::RegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener)
{
    WLOGFI("Register");
    std::lock_guard<std::mutex> lock(globalMutex_);
    WMError ret = RegisterListener(systemBarEnableListeners_[GetWindowId()], listener);
    return ret;
}

WMError WindowImpl::UnRegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener)
{
    WLOGFI("UnRegister");
    std::lock_guard<std::mutex> lock(globalMutex_);
    WMError ret = UnregisterListener(systemBarEnableListeners_[GetWindowId()], listener);
    return ret;
}

WMError WindowImpl::RegisterIgnoreViewSafeAreaListener(const sptr<IIgnoreViewSafeAreaListener>& listener)
{
    WLOGFI("Register");
    std::lock_guard<std::mutex> lock(globalMutex_);
    WMError ret = RegisterListener(ignoreSafeAreaListeners_[GetWindowId()], listener);
    return ret;
}

WMError WindowImpl::UnRegisterIgnoreViewSafeAreaListener(const sptr<IIgnoreViewSafeAreaListener>& listener)
{
    WLOGFI("UnRegister");
    std::lock_guard<std::mutex> lock(globalMutex_);
    WMError ret = UnregisterListener(ignoreSafeAreaListeners_[GetWindowId()], listener);
    return ret;
}

template<typename T>
WMError WindowImpl::RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        WLOGFE("Listener already registered");
        return WMError::WM_OK;
    }
    holder.emplace_back(listener);
    return WMError::WM_OK;
}

template<typename T>
WMError WindowImpl::UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    holder.erase(std::remove_if(holder.begin(), holder.end(),
        [listener](sptr<T> registeredListener) {
            return registeredListener == listener;
        }), holder.end());
    return WMError::WM_OK;
}

void WindowImpl::SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler)
{
    return;
}

void WindowImpl::SetRequestWindowModeSupportType(uint32_t windowModeSupportType)
{
    return;
}

void WindowImpl::ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (uiContent_ == nullptr) {
        WLOGFE("ConsumeKeyEvent to uiContent failed,uiContent_ is null");
        return;
    }
    uiContent_->ProcessKeyEvent(keyEvent);
}

void WindowImpl::ConsumeBackEvent()
{
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "ConsumeBackEvent to uiContent failed, uiContent_ is null");
        return;
    }
    auto isConsumed = uiContent_->ProcessBackPressed();
    TLOGI(WmsLogTag::WMS_EVENT, "ConsumeBackEvent to uiContent, %{public}d", isConsumed);
}

bool WindowImpl::IsDialogSessionBackGestureEnabled()
{
    return false;
}

void WindowImpl::ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (uiContent_ == nullptr) {
        WLOGFE("ConsumePointerEvent to uiContent failed,uiContent_ is null");
        return;
    }
    (void)uiContent_->ProcessPointerEvent(pointerEvent);
}


void WindowImpl::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    if (vsyncStation_ == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, vsyncStation is null");
        return;
    }
    vsyncStation_->RequestVsync(vsyncCallback);
}

int64_t WindowImpl::GetVSyncPeriod()
{
    return 0;
}

void WindowImpl::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (uiContent_ != nullptr) {
        WLOGFD("notify ace winId:%{public}u", GetWindowId());
        uiContent_->UpdateConfiguration(configuration);
    }
}

void WindowImpl::UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null, winId: %{public}u", GetWindowId());
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace winId: %{public}u", GetWindowId());
    uiContent_->UpdateConfiguration(configuration, resourceManager);
}

void WindowImpl::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    if (!avoidArea) {
        WLOGFE("invalid avoidArea");
        return;
    }

    WLOGFI("type:%{public}d, top:{%{public}d,%{public}d,%{public}d,%{public}d}, "
        "left:{%{public}d,%{public}d,%{public}d,%{public}d}, right:{%{public}d,%{public}d,%{public}d,%{public}d}, "
        "bottom:{%{public}d,%{public}d,%{public}d,%{public}d}",
        type, avoidArea->topRect_.posX_, avoidArea->topRect_.posY_, avoidArea->topRect_.width_,
        avoidArea->topRect_.height_, avoidArea->leftRect_.posX_, avoidArea->leftRect_.posY_,
        avoidArea->leftRect_.width_, avoidArea->leftRect_.height_, avoidArea->rightRect_.posX_,
        avoidArea->rightRect_.posY_, avoidArea->rightRect_.width_, avoidArea->rightRect_.height_,
        avoidArea->bottomRect_.posX_, avoidArea->bottomRect_.posY_, avoidArea->bottomRect_.width_,
        avoidArea->bottomRect_.height_);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        avoidAreaMap_[type] = avoidArea;
    }
    NotifyAvoidAreaChange(avoidArea, type);
}

void WindowImpl::NotifySystemBarChange(WindowType type, const SystemBarProperty& property)
{
    auto systemBarEnableListeners = GetListeners<IWindowSystemBarEnableListener>();
    for (auto& listener : systemBarEnableListeners) {
        if (listener != nullptr) {
            WLOGFD("type=%{public}u", type);
            listener->OnSetSpecificBarProperty(type, property);
        }
    }
}

void WindowImpl::NotifySetIgnoreSafeArea(bool value)
{
    auto ignoreSafeAreaListeners = GetListeners<IIgnoreViewSafeAreaListener>();
    for (auto& listener : ignoreSafeAreaListeners) {
        if (listener != nullptr) {
            WLOGFD("value=%{public}d", value);
            listener->SetIgnoreViewSafeArea(value);
        }
    }
}

void WindowImpl::NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    auto avoidAreaChangeListeners = GetListeners<IAvoidAreaChangedListener>();
    for (auto& listener : avoidAreaChangeListeners) {
        if (listener != nullptr) {
            WLOGFD("type=%{public}u", type);
            listener->OnAvoidAreaChanged(*avoidArea, type);
        }
    }
}

void WindowImpl::NotifyTouchDialogTarget(int32_t posX, int32_t posY)
{
    return;
}

void WindowImpl::SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel)
{
    needRemoveWindowInputChannel_ = needRemoveWindowInputChannel;
}

bool WindowImpl::IsLayoutFullScreen() const
{
    return isIgnoreSafeArea_;
}

bool WindowImpl::IsFullScreen() const
{
    auto statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto naviProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    return (IsLayoutFullScreen() && !statusProperty.enable_ && !naviProperty.enable_);
}

void WindowImpl::NotifyPreferredOrientationChange(Orientation orientation)
{
}

void WindowImpl::SetUserRequestedOrientation(Orientation orientation)
{
}

void WindowImpl::SetRequestedOrientation(Orientation orientation)
{
}

Orientation WindowImpl::GetRequestedOrientation()
{
    return Orientation::UNSPECIFIED;
}

WMError WindowImpl::SetTouchHotAreas(const std::vector<Rect>& rects)
{
    return WMError::WM_OK;
}
void WindowImpl::GetRequestedTouchHotAreas(std::vector<Rect>& rects) const
{
}

WMError WindowImpl::SetAPPWindowLabel(const std::string& label)
{
    if (uiContent_ == nullptr) {
        WLOGFE("uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent_->SetAppWindowTitle(label);
    return WMError::WM_OK;
}

WMError WindowImpl::SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon)
{
    if (icon == nullptr) {
        WLOGFE("window icon is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (uiContent_ == nullptr) {
        WLOGFE("uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent_->SetAppWindowIcon(icon);
    return WMError::WM_OK;
}

WMError WindowImpl::SetCornerRadius(float cornerRadius)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowRadius(float radius)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowColor(std::string color)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowOffsetX(float offsetX)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowOffsetY(float offsetY)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetBlur(float radius)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetBackdropBlur(float radius)
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetBackdropBlurStyle(WindowBlurStyle blurStyle)
{
    return WMError::WM_OK;
}

WMError WindowImpl::NotifyMemoryLevel(int32_t level)
{
    return WMError::WM_OK;
}

bool WindowImpl::IsAllowHaveSystemSubWindow()
{
    return true;
}

/** @note @window.hierarchy */
WMError WindowImpl::RaiseToAppTop()
{
    return WMError::WM_OK;
}

WMError WindowImpl::SetAspectRatio(float ratio)
{
    return WMError::WM_OK;
}

WMError WindowImpl::ResetAspectRatio()
{
    return WMError::WM_OK;
}

KeyboardAnimationConfig WindowImpl::GetKeyboardAnimationConfig()
{
    return keyboardAnimationConfig_;
}

void WindowImpl::SetNeedDefaultAnimation(bool needDefaultAnimation)
{
    return;
}

void WindowImpl::SetViewportConfig(const Ace::ViewportConfig& config)
{
    bool isUpdate = false;
    if (width_ != config.Width()) {
        width_ = config.Width();
        isUpdate = true;
    }
    if (height_ != config.Height()) {
        height_ = config.Height();
        isUpdate = true;
    }
    if (abs(density_ - config.Density()) >= 1e-6) {
        density_ = config.Density();
        isUpdate = true;
    }
    if (orientation_ != config.Orientation()) {
        orientation_ = config.Orientation();
        isUpdate = true;
    }
    if (isUpdate) {
        UpdateViewportConfig();
    }
}

void WindowImpl::UpdateViewportConfig()
{
    Ace::ViewportConfig config;
    config.SetSize(width_, height_);
    config.SetPosition(0, 0);
    config.SetDensity(density_);
    config.SetOrientation(orientation_);
    if (uiContent_ != nullptr) {
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::UNDEFINED);
    }
}

void WindowImpl::SetOrientation(Orientation orientation)
{
    WLOGFD("SetOrientation : orientation=%{public}d", static_cast<int32_t>(orientation));
    if (orientation_ == static_cast<int32_t>(orientation)) {
        return;
    }
    orientation_ = static_cast<int32_t>(orientation);
    UpdateViewportConfig();
}

void WindowImpl::SetSize(int32_t width, int32_t height)
{
    WLOGFD("SetSize : width=%{public}d, height=%{public}d", width, height);
    if (width_ == width && height_ == height) {
        return;
    }
    width_ = width;
    height_ = height;
    UpdateViewportConfig();
}

void WindowImpl::SetDensity(float density)
{
    WLOGFD("SetDensity : density=%{public}f", density);
    if (abs(density_ - density) <= 0.000001) { // 0.000001: near zero.
        return;
    }
    density_ = density;
    UpdateViewportConfig();
}

WMError WindowImpl::SetResizeByDragEnabled(bool dragEnabled)
{
    return WMError::WM_OK;
}

/** @note @window.hierarchy */
WMError WindowImpl::SetRaiseByClickEnabled(bool raiseEnabled)
{
    return WMError::WM_OK;
}

/** @note @window.hierarchy */
WMError WindowImpl::RaiseAboveTarget(int32_t subWindowId)
{
    return WMError::WM_OK;
}

WMError WindowImpl::HideNonSystemFloatingWindows(bool shouldHide)
{
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener)
{
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

WMError WindowImpl::UnregisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener)
{
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

WmErrorCode WindowImpl::KeepKeyboardOnFocus(bool keepKeyboardFlag)
{
    return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
}

WMError WindowImpl::SetSingleFrameComposerEnabled(bool enable)
{
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

WMError WindowImpl::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    return WMError::WM_OK;
}

void WindowImpl::SetUiDvsyncSwitch(bool dvsyncSwitch)
{
}

void WindowImpl::SetTouchEvent(int32_t touchType)
{
}

WMError WindowImpl::SetImmersiveModeEnabledState(bool enable)
{
    return WMError::WM_OK;
}

bool WindowImpl::GetImmersiveModeEnabledState() const
{
    return true;
}

uint32_t WindowImpl::GetApiTargetVersion() const
{
    uint32_t version = 0;
    if ((context_ != nullptr) && (context_->GetApplicationInfo() != nullptr)) {
        version = context_->GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD;
    }
    return version;
}
} // namespace Rosen
} // namespace OHOS
