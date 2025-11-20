/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <ability_manager_client.h>
#include <common/rs_common_def.h>
#include <filesystem>
#include <fstream>
#include <hisysevent.h>
#include <parameters.h>
#include <ipc_skeleton.h>
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_node.h>

#include "permission.h"
#include "color_parser.h"
#include "display_manager.h"
#include "display_info.h"
#include "ressched_report.h"
#include "rs_adapter.h"
#include "singleton_container.h"
#include "surface_capture_future.h"
#include "sys_cap_util.h"
#include "window_adapter.h"
#include "window_agent.h"
#include "window_helper.h"
#include "window_inspector.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "wm_math.h"
#include "perform_reporter.h"
#include "hitrace_meter.h"
#include <hisysevent.h>

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowImpl"};
const std::string PARAM_DUMP_HELP = "-h";
const uint32_t API_VERSION_MOD = 1000;
constexpr int32_t API_VERSION_18 = 18;

Ace::ContentInfoType GetAceContentInfoType(BackupAndRestoreType type)
{
    auto contentInfoType = Ace::ContentInfoType::NONE;
    switch (type) {
        case BackupAndRestoreType::CONTINUATION:
            contentInfoType = Ace::ContentInfoType::CONTINUATION;
            break;
        case BackupAndRestoreType::APP_RECOVERY:
            contentInfoType = Ace::ContentInfoType::APP_RECOVERY;
            break;
        case BackupAndRestoreType::RESOURCESCHEDULE_RECOVERY:
            contentInfoType = Ace::ContentInfoType::RESOURCESCHEDULE_RECOVERY;
            break;
        case BackupAndRestoreType::NONE:
            [[fallthrough]];
        default:
            break;
    }
    return contentInfoType;
}
}

WM_IMPLEMENT_SINGLE_INSTANCE(ResSchedReport);

const WindowImpl::ColorSpaceConvertMap WindowImpl::colorSpaceConvertMap[] = {
    { ColorSpace::COLOR_SPACE_DEFAULT, GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB },
    { ColorSpace::COLOR_SPACE_WIDE_GAMUT, GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3 },
};

std::map<std::string, std::pair<uint32_t, sptr<Window>>> WindowImpl::windowMap_;
std::map<uint32_t, std::vector<sptr<WindowImpl>>> WindowImpl::subWindowMap_;
std::map<uint32_t, std::vector<sptr<WindowImpl>>> WindowImpl::appFloatingWindowMap_;
std::map<uint32_t, std::vector<sptr<WindowImpl>>> WindowImpl::appDialogWindowMap_;
std::map<uint32_t, std::vector<sptr<IScreenshotListener>>> WindowImpl::screenshotListeners_;
std::map<uint32_t, std::vector<sptr<IScreenshotAppEventListener>>> WindowImpl::screenshotAppEventListeners_;
std::map<uint32_t, std::vector<sptr<ITouchOutsideListener>>> WindowImpl::touchOutsideListeners_;
std::map<uint32_t, std::vector<sptr<IDialogTargetTouchListener>>> WindowImpl::dialogTargetTouchListeners_;
std::map<uint32_t, std::vector<sptr<IWindowLifeCycle>>> WindowImpl::lifecycleListeners_;
std::map<uint32_t, std::vector<sptr<IWindowChangeListener>>> WindowImpl::windowChangeListeners_;
std::map<uint32_t, std::vector<sptr<IAvoidAreaChangedListener>>> WindowImpl::avoidAreaChangeListeners_;
std::map<uint32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> WindowImpl::occupiedAreaChangeListeners_;
std::map<uint32_t, sptr<IDialogDeathRecipientListener>> WindowImpl::dialogDeathRecipientListener_;
std::recursive_mutex WindowImpl::globalMutex_;
std::shared_mutex WindowImpl::windowMapMutex_;
int g_constructorCnt = 0;
int g_deConstructorCnt = 0;
WindowImpl::WindowImpl(const sptr<WindowOption>& option)
{
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    property_ = sptr<WindowProperty>::MakeSptr();
    InitWindowProperty(option);

    windowTag_ = option->GetWindowTag();
    isMainHandlerAvailable_ = option->GetMainHandlerAvailable();
    AdjustWindowAnimationFlag();
    UpdateDecorEnable();
    auto& sysBarPropMap = option->GetSystemBarProperty();
    for (auto it : sysBarPropMap) {
        property_->SetSystemBarProperty(it.first, it.second);
    }
    name_ = option->GetWindowName();

    RSAdapterUtil::InitRSUIDirector(rsUIDirector_, true, true);

    surfaceNode_ = CreateSurfaceNode(property_->GetWindowName(), option->GetWindowType());
    if (surfaceNode_ != nullptr) {
        vsyncStation_ = std::make_shared<VsyncStation>(surfaceNode_->GetId());
    }

    moveDragProperty_ = new (std::nothrow) MoveDragProperty();
    if (moveDragProperty_ == nullptr) {
        WLOGFE("MoveDragProperty is null");
    }
    WLOGFD("g_constructorCnt: %{public}d name: %{public}s",
        ++g_constructorCnt, property_->GetWindowName().c_str());
}

void WindowImpl::InitWindowProperty(const sptr<WindowOption>& option)
{
    if (option == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Init window property failed, option is nullptr.");
        return;
    }
    property_->SetWindowName(option->GetWindowName());
    property_->SetRequestRect(option->GetWindowRect());
    property_->SetWindowType(option->GetWindowType());
    if (WindowHelper::IsAppFloatingWindow(option->GetWindowType())) {
        property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    } else {
        property_->SetWindowMode(option->GetWindowMode());
    }
    property_->SetFullScreen(option->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN);
    property_->SetFocusable(option->GetFocusable());
    property_->SetTouchable(option->GetTouchable());
    property_->SetDisplayId(option->GetDisplayId());
    property_->ChangeCallingWindowId(option->GetCallingWindow());
    property_->SetWindowFlags(option->GetWindowFlags());
    property_->SetHitOffset(option->GetHitOffset());
    property_->SetRequestedOrientation(option->GetRequestedOrientation());
    property_->SetTurnScreenOn(option->IsTurnScreenOn());
    property_->SetKeepScreenOn(option->IsKeepScreenOn());
    property_->SetBrightness(option->GetBrightness());
}

RSSurfaceNode::SharedPtr WindowImpl::CreateSurfaceNode(std::string name, WindowType type)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = name;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    switch (type) {
        case WindowType::WINDOW_TYPE_BOOT_ANIMATION:
        case WindowType::WINDOW_TYPE_POINTER:
            rsSurfaceNodeType = RSSurfaceNodeType::SELF_DRAWING_WINDOW_NODE;
            break;
        case WindowType::WINDOW_TYPE_APP_MAIN_WINDOW:
            rsSurfaceNodeType = RSSurfaceNodeType::APP_WINDOW_NODE;
            break;
        case WindowType::WINDOW_TYPE_PIP:
            rsSurfaceNodeType = RSSurfaceNodeType::APP_WINDOW_NODE;
            break;
        default:
            rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
            break;
    }

    if (windowSystemConfig_.IsPhoneWindow() && WindowHelper::IsWindowFollowParent(type)) {
        rsSurfaceNodeType = RSSurfaceNodeType::ABILITY_COMPONENT_NODE;
    }
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType, true, false, GetRSUIContext());
    RSAdapterUtil::SetSkipCheckInMultiInstance(surfaceNode, true);
    TLOGD(WmsLogTag::WMS_SCB,
          "Create RSSurfaceNode: %{public}s, name: %{public}s",
          RSAdapterUtil::RSNodeToStr(surfaceNode).c_str(), name.c_str());
    return surfaceNode;
}

WindowImpl::~WindowImpl()
{
    WLOGI("windowName: %{public}s, windowId: %{public}d, g_deConstructorCnt: %{public}d, surfaceNode:%{public}d",
        GetWindowName().c_str(), GetWindowId(), ++g_deConstructorCnt, static_cast<uint32_t>(surfaceNode_.use_count()));
    Destroy(true, false);
}

sptr<Window> WindowImpl::Find(const std::string& name)
{
    std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
    auto iter = windowMap_.find(name);
    if (iter == windowMap_.end()) {
        return nullptr;
    }
    return iter->second.second;
}

const std::shared_ptr<AbilityRuntime::Context> WindowImpl::GetContext() const
{
    return context_;
}

sptr<Window> WindowImpl::FindWindowById(uint32_t WinId)
{
    std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
    if (windowMap_.empty()) {
        WLOGFE("Please create mainWindow First!");
        return nullptr;
    }
    for (auto iter = windowMap_.begin(); iter != windowMap_.end(); iter++) {
        if (WinId == iter->second.first) {
            WLOGI("FindWindow id: %{public}u", WinId);
            return iter->second.second;
        }
    }
    WLOGFE("Cannot find Window!");
    return nullptr;
}

sptr<Window> WindowImpl::GetTopWindowWithId(uint32_t mainWinId)
{
    uint32_t topWinId = INVALID_WINDOW_ID;
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetTopWindowId(mainWinId, topWinId);
    if (ret != WMError::WM_OK) {
        WLOGFE("GetTopWindowId failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    return FindWindowById(topWinId);
}

sptr<Window> WindowImpl::GetWindowWithId(uint32_t WinId)
{
    return FindWindowById(WinId);
}

sptr<Window> WindowImpl::GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    uint32_t mainWinId = INVALID_WINDOW_ID;
    {
        std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
        if (windowMap_.empty()) {
            WLOGFE("Please create mainWindow First!");
            return nullptr;
        }
        for (auto iter = windowMap_.begin(); iter != windowMap_.end(); iter++) {
            auto win = iter->second.second;
            if (context.get() == win->GetContext().get() && WindowHelper::IsMainWindow(win->GetType())) {
                mainWinId = win->GetWindowId();
                WLOGI("GetTopWindow Find MainWinId:%{public}u.", mainWinId);
                break;
            }
        }
    }
    WLOGI("GetTopWindowfinal winId:%{public}u!", mainWinId);
    if (mainWinId == INVALID_WINDOW_ID) {
        WLOGFE("Cannot find topWindow!");
        return nullptr;
    }
    uint32_t topWinId = INVALID_WINDOW_ID;
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetTopWindowId(mainWinId, topWinId);
    if (ret != WMError::WM_OK) {
        WLOGFE("GetTopWindowId failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    return FindWindowById(topWinId);
}

std::vector<sptr<Window>> WindowImpl::GetSubWindow(uint32_t parentId)
{
    if (subWindowMap_.find(parentId) == subWindowMap_.end()) {
        WLOGFE("Cannot parentWindow with id: %{public}u!", parentId);
        return std::vector<sptr<Window>>();
    }
    return std::vector<sptr<Window>>(subWindowMap_[parentId].begin(), subWindowMap_[parentId].end());
}

void WindowImpl::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    std::unordered_set<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowCtxSet(
        ignoreWindowContexts.begin(), ignoreWindowContexts.end());
    std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl map size: %{public}u", static_cast<uint32_t>(windowMap_.size()));
    for (const auto& winPair : windowMap_) {
        auto window = winPair.second.second;
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "impl window is null");
            continue;
        }
        if (ignoreWindowCtxSet.count(window->GetContext()) == 0) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
            window->UpdateConfiguration(configuration);
        } else {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "skip impl win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
        }
    }
}

std::shared_ptr<RSSurfaceNode> WindowImpl::GetSurfaceNode() const
{
    return surfaceNode_;
}

Rect WindowImpl::GetRect() const
{
    return property_->GetWindowRect();
}

Rect WindowImpl::GetRequestRect() const
{
    return property_->GetRequestRect();
}

WindowType WindowImpl::GetType() const
{
    return property_->GetWindowType();
}

WindowMode WindowImpl::GetWindowMode() const
{
    return property_->GetWindowMode();
}

float WindowImpl::GetAlpha() const
{
    return property_->GetAlpha();
}

WindowState WindowImpl::GetWindowState() const
{
    return state_;
}

WMError WindowImpl::SetFocusable(bool isFocusable)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetFocusable(isFocusable);
    if (state_ == WindowState::STATE_SHOWN) {
        return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_FOCUSABLE);
    }
    return WMError::WM_OK;
}

bool WindowImpl::GetFocusable() const
{
    return property_->GetFocusable();
}

WMError WindowImpl::SetTouchable(bool isTouchable)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetTouchable(isTouchable);
    if (state_ == WindowState::STATE_SHOWN) {
        return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_TOUCHABLE);
    }
    return WMError::WM_OK;
}

bool WindowImpl::GetTouchable() const
{
    return property_->GetTouchable();
}

const std::string& WindowImpl::GetWindowName() const
{
    return name_;
}

uint32_t WindowImpl::GetWindowId() const
{
    return property_->GetWindowId();
}

uint64_t WindowImpl::GetDisplayId() const
{
    return property_->GetDisplayId();
}

uint32_t WindowImpl::GetWindowFlags() const
{
    return property_->GetWindowFlags();
}

uint32_t WindowImpl::GetRequestWindowModeSupportType() const
{
    return property_->GetRequestWindowModeSupportType();
}

uint32_t WindowImpl::GetWindowModeSupportType() const
{
    return property_->GetWindowModeSupportType();
}

bool WindowImpl::IsMainHandlerAvailable() const
{
    return isMainHandlerAvailable_;
}

SystemBarProperty WindowImpl::GetSystemBarPropertyByType(WindowType type) const
{
    auto curProperties = property_->GetSystemBarProperty();
    return curProperties[type];
}

WMError WindowImpl::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect, int32_t apiVersion)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WLOGI("GetAvoidAreaByType Search Type: %{public}u", static_cast<uint32_t>(type));
    uint32_t windowId = property_->GetWindowId();
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetAvoidAreaByType(windowId, type, avoidArea, rect);
    if (ret != WMError::WM_OK) {
        WLOGFE("GetAvoidAreaByType errCode:%{public}d winId:%{public}u Type is :%{public}u.",
            static_cast<int32_t>(ret), property_->GetWindowId(), static_cast<uint32_t>(type));
    }
    return ret;
}

WMError WindowImpl::SetWindowType(WindowType type)
{
    WLOGFD("window id: %{public}u, type:%{public}u.", property_->GetWindowId(), static_cast<uint32_t>(type));
    if (type != WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW && !Permission::IsSystemCalling() &&
        !Permission::IsStartByHdcd()) {
        WLOGFE("set window type permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (state_ == WindowState::STATE_CREATED) {
        if (!(WindowHelper::IsAppWindow(type) || WindowHelper::IsSystemWindow(type))) {
            WLOGFE("window type is invalid %{public}u.", type);
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        property_->SetWindowType(type);
        UpdateDecorEnable();
        AdjustWindowAnimationFlag();
        return WMError::WM_OK;
    }
    if (property_->GetWindowType() != type) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowMode(WindowMode mode)
{
    WLOGI("Window %{public}u mode %{public}u", property_->GetWindowId(), static_cast<uint32_t>(mode));
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsWindowModeSupported(GetWindowModeSupportType(), mode)) {
        WLOGE("window %{public}u do not support mode: %{public}u",
            property_->GetWindowId(), static_cast<uint32_t>(mode));
        return WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }
    if (state_ == WindowState::STATE_CREATED || state_ == WindowState::STATE_HIDDEN) {
        UpdateMode(mode);
    } else if (state_ == WindowState::STATE_SHOWN) {
        WindowMode lastMode = property_->GetWindowMode();
        property_->SetWindowMode(mode);
        UpdateDecorEnable();
        WMError ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_MODE);
        if (ret != WMError::WM_OK) {
            property_->SetWindowMode(lastMode);
            return ret;
        }
        // set client window mode if success.
        UpdateMode(mode);
    }
    if (property_->GetWindowMode() != mode) {
        WLOGFE("set window mode filed! id: %{public}u.", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

WMError WindowImpl::SetAlpha(float alpha)
{
    WLOGI("Window %{public}u alpha %{public}f", property_->GetWindowId(), alpha);
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set alpha permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetAlpha(alpha);
    surfaceNode_->SetAlpha(alpha);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetTransform(const Transform& trans)
{
    WLOGI("Window %{public}u", property_->GetWindowId());
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    Transform oriTrans = property_->GetTransform();
    property_->SetTransform(trans);
    WMError ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetTransform errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
        property_->SetTransform(oriTrans); // reset to ori transform when update failed
    }
    if (property_->IsDisplayZoomOn()) {
        TransformSurfaceNode(property_->GetZoomTransform());
    } else {
        TransformSurfaceNode(trans);
    }
    return ret;
}

const Transform& WindowImpl::GetTransform() const
{
    return property_->GetTransform();
}

const Transform& WindowImpl::GetZoomTransform() const
{
    return property_->GetZoomTransform();
}

WMError WindowImpl::AddWindowFlag(WindowFlag flag)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (flag == WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED && state_ != WindowState::STATE_CREATED) {
        WLOGFE("Only support add show when locked when window create, id: %{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (flag == WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE && !Permission::IsSystemCalling()) {
        WLOGFE("set forbid split move permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    uint32_t updateFlags = property_->GetWindowFlags() | (static_cast<uint32_t>(flag));
    return SetWindowFlags(updateFlags);
}

WMError WindowImpl::RemoveWindowFlag(WindowFlag flag)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (flag == WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED && state_ != WindowState::STATE_CREATED) {
        WLOGFE("Only support remove show when locked when window create, id: %{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (flag == WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE && !Permission::IsSystemCalling()) {
        WLOGFE("set forbid split move permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    uint32_t updateFlags = property_->GetWindowFlags() & (~(static_cast<uint32_t>(flag)));
    return SetWindowFlags(updateFlags);
}

WMError WindowImpl::SetWindowFlags(uint32_t flags)
{
    WLOGI("Window %{public}u flags %{public}u", property_->GetWindowId(), flags);
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (property_->GetWindowFlags() == flags) {
        return WMError::WM_OK;
    }
    auto oriFlags = property_->GetWindowFlags();
    property_->SetWindowFlags(flags);
    if (state_ == WindowState::STATE_CREATED || state_ == WindowState::STATE_HIDDEN) {
        return WMError::WM_OK;
    }
    WMError ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_FLAGS);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetWindowFlags errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
        property_->SetWindowFlags(oriFlags);
    }
    return ret;
}

void WindowImpl::OnNewWant(const AAFwk::Want& want)
{
    WLOGI("Window [name:%{public}s, id:%{public}u]", name_.c_str(), property_->GetWindowId());
    if (uiContent_ != nullptr) {
        uiContent_->OnNewWant(want);
    }
}

WMError WindowImpl::NapiSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage,
        type == BackupAndRestoreType::NONE ? WindowSetUIContentType::DEFAULT : WindowSetUIContentType::RESTORE,
        type, ability, 1);
}

WMError WindowImpl::NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage,
        type == BackupAndRestoreType::NONE ? WindowSetUIContentType::DEFAULT : WindowSetUIContentType::RESTORE,
        type, ability);
}

WMError WindowImpl::SetUIContentByName(
    const std::string& contentInfo, napi_env env, napi_value storage, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage, WindowSetUIContentType::BY_NAME,
        BackupAndRestoreType::NONE, ability);
}

WMError WindowImpl::SetUIContentByAbc(
    const std::string& contentInfo, napi_env env, napi_value storage, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage, WindowSetUIContentType::BY_ABC,
        BackupAndRestoreType::NONE, ability);
}

std::unique_ptr<Ace::UIContent> WindowImpl::UIContentCreate(AppExecFwk::Ability* ability, void* env, int isAni)
{
    if (isAni) {
        return  ability != nullptr ? Ace::UIContent::Create(ability) :
            Ace::UIContent::CreateWithAniEnv(context_.get(), reinterpret_cast<ani_env*>(env));
    } else {
        return  ability != nullptr ? Ace::UIContent::Create(ability) :
            Ace::UIContent::Create(context_.get(), reinterpret_cast<NativeEngine*>(env));
    }
}
Ace::UIContentErrorCode WindowImpl::UIContentInitByName(Ace::UIContent* uiContent,
    const std::string& contentInfo, void* storage, int isAni)
{
    if (isAni) {
        return uiContent->InitializeByNameWithAniStorage(this, contentInfo, (ani_object)storage);
    } else {
        return uiContent->InitializeByName(this, contentInfo, (napi_value)storage);
    }
}

template<typename T>
Ace::UIContentErrorCode WindowImpl::UIContentInit(Ace::UIContent* uiContent, T contentInfo,
    void* storage, int isAni)
{
    if (isAni) {
        return uiContent->InitializeWithAniStorage(this, contentInfo, (ani_object)storage);
    } else {
        return uiContent->Initialize(this, contentInfo, (napi_value)storage);
    }
}
template<typename T>
Ace::UIContentErrorCode WindowImpl::UIContentInit(Ace::UIContent* uiContent, T contentInfo,
    void* storage, const std::string& contentName, int isAni)
{
    if (isAni) {
        return uiContent->InitializeWithAniStorage(this, contentInfo, (ani_object)storage, contentName);
    } else {
        return uiContent->Initialize(this, contentInfo, (napi_value)storage, contentName);
    }
}

Ace::UIContentErrorCode WindowImpl::UIContentRestore(Ace::UIContent* uiContent, const std::string& contentInfo,
    void* storage, Ace::ContentInfoType infoType, int isAni)
{
    if (isAni) {
        return uiContent->Restore(this, contentInfo, (ani_object)storage, infoType);
    } else {
        return uiContent->Restore(this, contentInfo, (napi_value)storage, infoType);
    }
}


WMError WindowImpl::SetUIContentInner(const std::string& contentInfo, void* env, void* storage,
    WindowSetUIContentType setUIContentType, BackupAndRestoreType restoreType, AppExecFwk::Ability* ability, int isAni)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "loadContent");
    if (!IsWindowValid()) {
        WLOGFD("interrupt set uicontent because window is invalid! window state: %{public}d", state_);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WLOGFD("NapiSetUIContent: %{public}s", contentInfo.c_str());
    if (uiContent_) {
        uiContent_->Destroy();
    }
    std::unique_ptr<Ace::UIContent> uiContent = UIContentCreate(ability, (void*)env, isAni);
    if (uiContent == nullptr) {
        WLOGFE("fail to NapiSetUIContent id: %{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }

    OHOS::Ace::UIContentErrorCode aceRet = OHOS::Ace::UIContentErrorCode::NO_ERRORS;
    switch (setUIContentType) {
        default:
        case WindowSetUIContentType::DEFAULT: {
            auto routerStack = GetRestoredRouterStack();
            auto type = GetAceContentInfoType(BackupAndRestoreType::RESOURCESCHEDULE_RECOVERY);
            if (!routerStack.empty() && UIContentRestore(uiContent.get(), routerStack, storage, type,
                isAni) == Ace::UIContentErrorCode::NO_ERRORS) {
                TLOGI(WmsLogTag::WMS_LIFE, "Restore router stack succeed.");
                break;
            }
            if (!intentParam_.empty()) {
                TLOGI(WmsLogTag::WMS_LIFE, "Default setIntentParam, isColdStart:%{public}u", isIntentColdStart_);
                uiContent->SetIntentParam(intentParam_, std::move(loadPageCallback_), isIntentColdStart_);
                intentParam_ = "";
            }
            aceRet = UIContentInit(uiContent.get(), contentInfo, storage, isAni);
            break;
        }
        case WindowSetUIContentType::RESTORE:
            aceRet = UIContentRestore(uiContent.get(), contentInfo, storage, GetAceContentInfoType(restoreType), isAni);
            break;
        case WindowSetUIContentType::BY_NAME:
            if (!intentParam_.empty()) {
                TLOGI(WmsLogTag::WMS_LIFE, "By name setIntentParam, isColdStart:%{public}u", isIntentColdStart_);
                uiContent->SetIntentParam(intentParam_, std::move(loadPageCallback_), isIntentColdStart_);
                intentParam_ = "";
            }
            aceRet = UIContentInitByName(uiContent.get(), contentInfo, storage, isAni);
            break;
        case WindowSetUIContentType::BY_ABC:
            aceRet = UIContentInit(uiContent.get(), GetAbcContent(contentInfo), storage, contentInfo, isAni);
            break;
    }
    // make uiContent available after Initialize/Restore
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        uiContent_ = std::move(uiContent);
    }

    if (isIgnoreSafeAreaNeedNotify_) {
        uiContent_->SetIgnoreViewSafeArea(isIgnoreSafeArea_);
    }
    UpdateDecorEnable(true);

    if (state_ == WindowState::STATE_SHOWN) {
        // UIContent may be nullptr when show window, need to notify again when window is shown
        uiContent_->Foreground();
        UpdateTitleButtonVisibility();
        Ace::ViewportConfig config;
        Rect rect = GetRect();
        config.SetSize(rect.width_, rect.height_);
        config.SetPosition(rect.posX_, rect.posY_);
        auto display = SingletonContainer::IsDestroyed() ? nullptr :
            SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
        if (display == nullptr) {
            WLOGFE("get display failed displayId:%{public}" PRIu64", window id:%{public}u", property_->GetDisplayId(),
                property_->GetWindowId());
            return WMError::WM_ERROR_NULLPTR;
        }
        float virtualPixelRatio = display->GetVirtualPixelRatio();
        config.SetDensity(virtualPixelRatio);
        config.SetDisplayId(GetDisplayId());
        auto displayInfo = display->GetDisplayInfo();
        if (displayInfo != nullptr) {
            config.SetOrientation(static_cast<int32_t>(displayInfo->GetDisplayOrientation()));
            TLOGI(WmsLogTag::WMS_LIFE, "notify window orientation change end.");
        }
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::UNDEFINED, nullptr);
        WLOGFD("notify uiContent window size change end");
    }
    if (aceRet != OHOS::Ace::UIContentErrorCode::NO_ERRORS) {
        WLOGFE("failed to init or restore uicontent with file %{public}s. errorCode: %{public}d",
            contentInfo.c_str(), static_cast<uint16_t>(aceRet));
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

float WindowImpl::GetVirtualPixelRatio()
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "virtualPixelRatio: %{public}f", virtualPixelRatio_.load());
    return virtualPixelRatio_.load();
}

std::shared_ptr<std::vector<uint8_t>> WindowImpl::GetAbcContent(const std::string& abcPath)
{
    std::filesystem::path abcFile { abcPath };
    if (abcFile.empty() || !abcFile.is_absolute() || !std::filesystem::exists(abcFile)) {
        WLOGFE("abc file path is not valid");
        return nullptr;
    }
    int begin, end;
    std::fstream file(abcFile, std::ios::in | std::ios::binary);
    if (!file) {
        WLOGFE("abc file is not valid");
        return nullptr;
    }
    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();
    int len = end - begin;
    WLOGFD("abc file: %{public}s, size: %{public}d", abcPath.c_str(), len);

    if (len <= 0) {
        WLOGFE("abc file size is 0");
        return nullptr;
    }
    std::vector<uint8_t> abcBytes(len);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(abcBytes.data()), len);
    return std::make_shared<std::vector<uint8_t>>(abcBytes);
}

Ace::UIContent* WindowImpl::GetUIContent() const
{
    return uiContent_.get();
}

Ace::UIContent* WindowImpl::GetUIContentWithId(uint32_t winId) const
{
    return nullptr;
}

std::string WindowImpl::GetContentInfo(BackupAndRestoreType type)
{
    WLOGFD("GetContentInfo");
    if (type == BackupAndRestoreType::NONE) {
        return "";
    }

    if (uiContent_ == nullptr) {
        WLOGFE("fail to GetContentInfo id: %{public}u", property_->GetWindowId());
        return "";
    }
    return uiContent_->GetContentInfo(GetAceContentInfoType(type));
}

WMError WindowImpl::SetRestoredRouterStack(const std::string& routerStack)
{
    TLOGD(WmsLogTag::WMS_LIFE, "Set restored router stack.");
    restoredRouterStack_ = routerStack;
    return WMError::WM_OK;
}

std::string WindowImpl::GetRestoredRouterStack()
{
    TLOGD(WmsLogTag::WMS_LIFE, "Get restored router stack.");
    return std::move(restoredRouterStack_);
}

ColorSpace WindowImpl::GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut)
{
    for (auto item: colorSpaceConvertMap) {
        if (item.surfaceColorGamut == colorGamut) {
            return item.colorSpace;
        }
    }
    return ColorSpace::COLOR_SPACE_DEFAULT;
}

GraphicColorGamut WindowImpl::GetSurfaceGamutFromColorSpace(ColorSpace colorSpace)
{
    for (auto item: colorSpaceConvertMap) {
        if (item.colorSpace == colorSpace) {
            return item.surfaceColorGamut;
        }
    }
    return GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
}

bool WindowImpl::IsSupportWideGamut()
{
    return true;
}

void WindowImpl::SetColorSpace(ColorSpace colorSpace)
{
    if (!IsWindowValid()) {
        return;
    }
    if (surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "surface node is nullptr, winId: %{public}u", GetWindowId());
        return;
    }
    auto surfaceGamut = GetSurfaceGamutFromColorSpace(colorSpace);
    surfaceNode_->SetColorSpace(surfaceGamut);
}

ColorSpace WindowImpl::GetColorSpace()
{
    if (!IsWindowValid()) {
        return ColorSpace::COLOR_SPACE_DEFAULT;
    }
    if (surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "surface node is nullptr, winId: %{public}u", GetWindowId());
        return ColorSpace::COLOR_SPACE_DEFAULT;
    }
    auto surfaceGamut = surfaceNode_->GetColorSpace();
    return GetColorSpaceFromSurfaceGamut(surfaceGamut);
}

std::shared_ptr<Media::PixelMap> WindowImpl::Snapshot()
{
    if (!IsWindowValid()) {
        return nullptr;
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    auto isSucceeded = RSInterfaces::GetInstance().TakeSurfaceCapture(surfaceNode_, callback);
    std::shared_ptr<Media::PixelMap> pixelMap;
    if (isSucceeded) {
        pixelMap = callback->GetResult(2000); // wait for <= 2000ms
    } else {
        pixelMap = SingletonContainer::Get<WindowAdapter>().GetSnapshot(property_->GetWindowId());
    }
    if (pixelMap != nullptr) {
        WLOGFD("WMS-Client Save WxH=%{public}dx%{public}d", pixelMap->GetWidth(), pixelMap->GetHeight());
    } else {
        WLOGFE("Failed to get pixelmap, return nullptr!");
    }
    return pixelMap;
}

WMError WindowImpl::SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap)
{
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

void WindowImpl::DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    if (params.size() == 1 && params[0] == PARAM_DUMP_HELP) { // 1: params num
        WLOGFD("Dump ArkUI help Info");
        Ace::UIContent::ShowDumpHelp(info);
        SingletonContainer::Get<WindowAdapter>().NotifyDumpInfoResult(info);
        return;
    }
    WLOGFD("ArkUI:DumpInfo");
    if (uiContent_ != nullptr) {
        uiContent_->DumpInfo(params, info);
    }
    SingletonContainer::Get<WindowAdapter>().NotifyDumpInfoResult(info);
}

WMError WindowImpl::UpdateSystemBarProperties(
    const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
{
    for (auto& [systemBarType, systemBarPropertyFlag] : systemBarPropertyFlags) {
        if (systemBarProperties.find(systemBarType) == systemBarProperties.end()) {
            TLOGE(WmsLogTag::WMS_IMMS, "system bar type is invalid");
            return WMError::WM_DO_NOTHING;
        }
        auto property = GetSystemBarPropertyByType(systemBarType);
        property.enable_ = systemBarPropertyFlag.enableFlag ?
            systemBarProperties.at(systemBarType).enable_ : property.enable_;
        property.backgroundColor_ = systemBarPropertyFlag.backgroundColorFlag ?
            systemBarProperties.at(systemBarType).backgroundColor_ : property.backgroundColor_;
        property.contentColor_ = systemBarPropertyFlag.contentColorFlag ?
            systemBarProperties.at(systemBarType).contentColor_ : property.contentColor_;
        property.enableAnimation_ = systemBarPropertyFlag.enableAnimationFlag ?
            systemBarProperties.at(systemBarType).enableAnimation_ : property.enableAnimation_;

        if (systemBarPropertyFlag.enableFlag) {
            property.settingFlag_ |= SystemBarSettingFlag::ENABLE_SETTING;
        }
        if (systemBarPropertyFlag.backgroundColorFlag || systemBarPropertyFlag.contentColorFlag) {
            property.settingFlag_ |= SystemBarSettingFlag::COLOR_SETTING;
        }

        if (systemBarPropertyFlag.enableFlag || systemBarPropertyFlag.backgroundColorFlag ||
            systemBarPropertyFlag.contentColorFlag || systemBarPropertyFlag.enableAnimationFlag) {
            auto err = SetSystemBarProperty(systemBarType, property);
            if (err != WMError::WM_OK) {
                return err;
            }
        }
    }
    return WMError::WM_OK;
}

WMError WindowImpl::UpdateSystemBarPropertyForPage(WindowType type,
    const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag)
{
    return SetSystemBarProperty(type, systemBarProperty);
}

WMError WindowImpl::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    WLOGI("Window %{public}u type %{public}u enable:%{public}u, bgColor:%{public}x, Color:%{public}x ",
        property_->GetWindowId(), static_cast<uint32_t>(type), property.enable_,
        property.backgroundColor_, property.contentColor_);
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (GetSystemBarPropertyByType(type) == property) {
        return WMError::WM_OK;
    }
    property_->SetSystemBarProperty(type, property);
    if (state_ == WindowState::STATE_CREATED || state_ == WindowState::STATE_HIDDEN) {
        return WMError::WM_OK;
    }
    WMError ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetSystemBarProperty errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
    }
    return ret;
}

WMError WindowImpl::SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
    const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    SystemBarProperty current = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto flagIter = propertyFlags.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto propertyIter = properties.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    if ((flagIter != propertyFlags.end() && flagIter->second.contentColorFlag) &&
        (propertyIter != properties.end() && current.contentColor_ != propertyIter->second.contentColor_)) {
        current.contentColor_ = propertyIter->second.contentColor_;
        current.settingFlag_ = static_cast<SystemBarSettingFlag>(
            static_cast<uint32_t>(propertyIter->second.settingFlag_) |
            static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
        WLOGI("Window:%{public}u %{public}s set status bar content color %{public}u",
            GetWindowId(), GetWindowName().c_str(), current.contentColor_);
        return SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, current);
    }
    return WMError::WM_OK;
}

WMError WindowImpl::GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties)
{
    if (property_ != nullptr) {
        WLOGI("Window:%{public}u", GetWindowId());
        properties[WindowType::WINDOW_TYPE_STATUS_BAR] = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    } else {
        WLOGFE("inner property is null");
    }
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

WMError WindowImpl::SetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
{
    return WMError::WM_OK;
}

WMError WindowImpl::UpdateSystemBarProperty(bool status)
{
    if (!IsWindowValid()) {
        WLOGFE("PutSystemBarProperty errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW), property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty naviProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    if (status) {
        statusProperty.enable_ = false;
        naviProperty.enable_ = false;
    } else {
        statusProperty.enable_ = true;
        naviProperty.enable_ = true;
    }

    if ((GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR) == statusProperty) &&
        (GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR) == naviProperty)) {
        return WMError::WM_OK;
    }
    if (!(GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR) == statusProperty)) {
        property_->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusProperty);
    }
    if (!(GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR) == naviProperty)) {
        property_->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, naviProperty);
    }
    if (state_ == WindowState::STATE_CREATED || state_ == WindowState::STATE_HIDDEN) {
        return WMError::WM_OK;
    }

    WMError ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetSystemBarProperty errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
    }
    return ret;
}

WMError WindowImpl::SetLayoutFullScreen(bool status)
{
    WLOGI("Window %{public}u status: %{public}u", property_->GetWindowId(), status);
    if (!IsWindowValid() ||
        !WindowHelper::IsWindowModeSupported(GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FULLSCREEN)) {
        WLOGFE("invalid window or fullscreen mode is not be supported, winId:%{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WMError ret = WMError::WM_OK;
    uint32_t version = 0;
    if ((context_ != nullptr) && (context_->GetApplicationInfo() != nullptr)) {
        version = context_->GetApplicationInfo()->apiCompatibleVersion;
    }
    ret = SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetWindowMode errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
        return ret;
    }
    isIgnoreSafeArea_ = status;
    // 10 ArkUI new framework support after API10
    if (version >= 10) {
        if (uiContent_ != nullptr) {
            uiContent_->SetIgnoreViewSafeArea(status);
        }
        isIgnoreSafeAreaNeedNotify_ = true;
    } else {
        if (status) {
            ret = RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
            if (ret != WMError::WM_OK) {
                WLOGFE("RemoveWindowFlag errCode:%{public}d winId:%{public}u",
                    static_cast<int32_t>(ret), property_->GetWindowId());
                return ret;
            }
        } else {
            ret = AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
            if (ret != WMError::WM_OK) {
                WLOGFE("AddWindowFlag errCode:%{public}d winId:%{public}u",
                    static_cast<int32_t>(ret), property_->GetWindowId());
                return ret;
            }
        }
    }
    enableImmersiveMode_ = status;
    return ret;
}

WMError WindowImpl::SetFullScreen(bool status)
{
    WLOGI("Window %{public}u status: %{public}d", property_->GetWindowId(), status);
    if (!IsWindowValid() ||
        !WindowHelper::IsWindowModeSupported(GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FULLSCREEN)) {
        WLOGFE("invalid window or fullscreen mode is not be supported, winId:%{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WMError ret = UpdateSystemBarProperty(status);
    if (ret != WMError::WM_OK) {
        WLOGFE("UpdateSystemBarProperty errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
    }
    ret = SetLayoutFullScreen(status);
    if (ret != WMError::WM_OK) {
        WLOGFE("SetLayoutFullScreen errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), property_->GetWindowId());
    }
    return ret;
}

WMError WindowImpl::SetFloatingMaximize(bool isEnter)
{
    WLOGFI("id:%{public}d SetFloatingMaximize status: %{public}d", property_->GetWindowId(), isEnter);
    if (!IsWindowValid() ||
        !WindowHelper::IsWindowModeSupported(GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FULLSCREEN)) {
        WLOGFE("invalid window or maximize mode is not be supported, winId:%{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (isEnter && GetGlobalMaximizeMode() != MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        WMError ret = SetFullScreen(true);
        if (ret == WMError::WM_OK) {
            property_->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
        }
        return ret;
    }

    if (isEnter && GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
        if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
            SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        }
    }
    property_->SetMaximizeMode(isEnter ? MaximizeMode::MODE_AVOID_SYSTEM_BAR : MaximizeMode::MODE_RECOVER);
    property_->SetWindowSizeChangeReason(WindowSizeChangeReason::RESIZE);
    return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
}

WMError WindowImpl::SetAspectRatio(float ratio)
{
    WLOGFI("windowId: %{public}u, ratio: %{public}f", GetWindowId(), ratio);
    if (!WindowHelper::IsMainWindow(GetType())) {
        WLOGFE("Invalid operation, windowId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    if (MathHelper::NearZero(ratio) || ratio < 0.0f) {
        WLOGFE("Invalid param, ratio: %{public}f", ratio);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    property_->SetAspectRatio(ratio);
    if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        WLOGFD("window is hidden or created! id: %{public}u, ratio: %{public}f ", property_->GetWindowId(), ratio);
        return WMError::WM_OK;
    }
    auto ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO);
    if (ret != WMError::WM_OK) {
        WLOGFE("Set AspectRatio failed. errorCode: %{public}u", ret);
    }
    return ret;
}

WMError WindowImpl::ResetAspectRatio()
{
    if (!IsWindowValid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Window is invalid");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }

    WLOGFI("windowId: %{public}u", GetWindowId());
    if (!WindowHelper::IsMainWindow(GetType())) {
        WLOGFE("Invalid operation, windowId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    property_->SetAspectRatio(0.0);
    if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        WLOGFD("window is hidden or created! id: %{public}u", property_->GetWindowId());
        return WMError::WM_OK;
    }
    UpdateProperty(PropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO);
    return WMError::WM_OK;
}

void WindowImpl::MapFloatingWindowToAppIfNeeded()
{
    if (!WindowHelper::IsAppFloatingWindow(GetType()) || context_.get() == nullptr) {
        return;
    }

    WLOGFI("In");
    std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
    for (const auto& winPair : windowMap_) {
        auto win = winPair.second.second;
        if (win->GetType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
            context_.get() == win->GetContext().get()) {
            sptr<WindowImpl> selfImpl(this);
            appFloatingWindowMap_[win->GetWindowId()].push_back(selfImpl);
            WLOGFD("Map FloatingWindow %{public}u to AppMainWindow %{public}u, type is %{public}u",
                GetWindowId(), win->GetWindowId(), GetType());
            return;
        }
    }
}

void WindowImpl::MapDialogWindowToAppIfNeeded()
{
    if (GetType() != WindowType::WINDOW_TYPE_DIALOG) {
        return;
    }

    std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
    for (const auto& winPair : windowMap_) {
        auto win = winPair.second.second;
        if (win->GetType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
            context_.get() == win->GetContext().get()) {
            sptr<WindowImpl> selfImpl(this);
            appDialogWindowMap_[win->GetWindowId()].push_back(selfImpl);
            WLOGFD("Map DialogWindow %{public}u to AppMainWindow %{public}u", GetWindowId(), win->GetWindowId());
            return;
        }
    }
}

WMError WindowImpl::UpdateProperty(PropertyChangeAction action)
{
    return SingletonContainer::Get<WindowAdapter>().UpdateProperty(property_, action);
}

void WindowImpl::GetConfigurationFromAbilityInfo()
{
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
    if (abilityContext == nullptr) {
        WLOGFE("id:%{public}u is not ability Window", property_->GetWindowId());
        return;
    }
    auto abilityInfo = abilityContext->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        WLOGFE("id:%{public}u Ability window get ability info failed", property_->GetWindowId());
        return;
    }

    // get support modes configuration
    uint32_t windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(abilityInfo->windowModes);
    if (windowModeSupportType == 0) {
        WLOGFD("mode config param is 0, all modes is supported");
        windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    }
    WLOGFD("winId: %{public}u, windowModeSupportType: %{public}u", GetWindowId(), windowModeSupportType);
    SetRequestWindowModeSupportType(windowModeSupportType);

    // get window size limits configuration
    WindowLimits sizeLimits;
    sizeLimits.maxWidth_ = abilityInfo->maxWindowWidth;
    sizeLimits.maxHeight_ = abilityInfo->maxWindowHeight;
    sizeLimits.minWidth_ = abilityInfo->minWindowWidth;
    sizeLimits.minHeight_ = abilityInfo->minWindowHeight;
    sizeLimits.maxRatio_ = static_cast<float>(abilityInfo->maxWindowRatio);
    sizeLimits.minRatio_ = static_cast<float>(abilityInfo->minWindowRatio);
    property_->SetSizeLimits(sizeLimits);

    // get orientation configuration
    OHOS::AppExecFwk::DisplayOrientation displayOrientation =
        static_cast<OHOS::AppExecFwk::DisplayOrientation>(
            static_cast<uint32_t>(abilityInfo->orientation));
    if (ABILITY_TO_WMS_ORIENTATION_MAP.count(displayOrientation) == 0) {
        WLOGFE("id:%{public}u Do not support this Orientation type", property_->GetWindowId());
        return;
    }
    Orientation orientation = ABILITY_TO_WMS_ORIENTATION_MAP.at(displayOrientation);
    if (orientation < Orientation::BEGIN || orientation > Orientation::END) {
        WLOGFE("Set orientation from ability failed");
        return;
    }
    property_->SetRequestedOrientation(orientation);
}

void WindowImpl::UpdateTitleButtonVisibility()
{
    WLOGFD("[Client] UpdateTitleButtonVisibility");
    if (uiContent_ == nullptr || !IsDecorEnable()) {
        return;
    }
    auto windowModeSupportType = GetWindowModeSupportType();
    bool hideSplitButton = !(windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY);
    // not support fullscreen in split and floating mode, or not support float in fullscreen mode
    bool hideMaximizeButton = (!(windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN) &&
        (GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING || WindowHelper::IsSplitWindowMode(GetWindowMode()))) ||
        (!(windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING) &&
        GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN);
    WLOGD("[Client] [hideSplit, hideMaximize]: [%{public}d, %{public}d]", hideSplitButton, hideMaximizeButton);
    uiContent_->HideWindowTitleButton(hideSplitButton, hideMaximizeButton, false, false);
}

bool WindowImpl::IsAppMainOrSubOrFloatingWindow()
{
    // App main window need decor config, stretchable config and effect config
    // App sub window and float window need effect config
    if (WindowHelper::IsAppWindow(GetType())) {
        return true;
    }

    if (WindowHelper::IsAppFloatingWindow(GetType())) {
        std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
        for (const auto& winPair : windowMap_) {
            auto win = winPair.second.second;
            if (win != nullptr && win->GetType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
                context_.get() == win->GetContext().get()) {
                isAppFloatingWindow_ = true;
                return true;
            }
        }
    }
    return false;
}

void WindowImpl::SetSystemConfig()
{
    if (!IsAppMainOrSubOrFloatingWindow()) {
        return;
    }
    if (SingletonContainer::Get<WindowAdapter>().GetSystemConfig(windowSystemConfig_) == WMError::WM_OK) {
        if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
            WLOGFD("get system decor enable:%{public}d", windowSystemConfig_.isSystemDecorEnable_);
            property_->SetDecorEnable(windowSystemConfig_.isSystemDecorEnable_);
            WLOGFD("get stretchable enable:%{public}d", windowSystemConfig_.isStretchable_);
            property_->SetStretchable(windowSystemConfig_.isStretchable_);
            // if window mode is undefined, set it from configuration
            if (property_->GetWindowMode() == WindowMode::WINDOW_MODE_UNDEFINED) {
                WLOGFD("get default window mode:%{public}u", windowSystemConfig_.defaultWindowMode_);
                property_->SetWindowMode(windowSystemConfig_.defaultWindowMode_);
            }
            if (property_->GetLastWindowMode() == WindowMode::WINDOW_MODE_UNDEFINED) {
                property_->SetLastWindowMode(windowSystemConfig_.defaultWindowMode_);
            }
        }
    }
}

KeyboardAnimationConfig WindowImpl::GetKeyboardAnimationConfig()
{
    return { windowSystemConfig_.animationIn_, windowSystemConfig_.animationOut_ };
}

WMError WindowImpl::WindowCreateCheck(uint32_t parentId)
{
    if (vsyncStation_ == nullptr || !vsyncStation_->IsVsyncReceiverCreated()) {
        return WMError::WM_ERROR_NULLPTR;
    }
    // check window name, same window names are forbidden
    {
        std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
        if (windowMap_.find(name_) != windowMap_.end()) {
            WLOGFE("WindowName(%{public}s) already exists.", name_.c_str());
            return WMError::WM_ERROR_REPEAT_OPERATION;
        }
    }
    if (CheckCameraFloatingWindowMultiCreated(property_->GetWindowType())) {
        WLOGFE("Camera Floating Window already exists.");
        return WMError::WM_ERROR_REPEAT_OPERATION;
    }
    if (parentId == INVALID_WINDOW_ID) {
        if (WindowHelper::IsSystemSubWindow(property_->GetWindowType()) ||
            WindowHelper::IsSubWindow(property_->GetWindowType())) {
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        return WMError::WM_OK;
    }

    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
        property_->SetParentId(parentId);
    } else {
        sptr<Window> parentWindow = nullptr;
        {
            std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
            for (const auto& winPair : windowMap_) {
                if (winPair.second.first == parentId) {
                    property_->SetParentId(parentId);
                    parentWindow = winPair.second.second;
                    break;
                }
            }
        }
        if (WindowHelper::IsSystemSubWindow(property_->GetWindowType())) {
            if (parentWindow == nullptr) {
                return WMError::WM_ERROR_INVALID_PARENT;
            }
            if (!parentWindow->IsAllowHaveSystemSubWindow()) {
                return WMError::WM_ERROR_INVALID_PARENT;
            }
        }
    }
    if (property_->GetParentId() != parentId) {
        WLOGFE("Parent Window does not exist. ParentId is %{public}u", parentId);
        return WMError::WM_ERROR_INVALID_PARENT;
    }

    return WMError::WM_OK;
}

void WindowImpl::ChangePropertyByApiVersion()
{
    uint32_t version = 0;
    if ((context_ != nullptr) && (context_->GetApplicationInfo() != nullptr)) {
        version = context_->GetApplicationInfo()->apiCompatibleVersion;
    }
    // 10 ArkUI new framework support after API10
    if (version >= 10) {
        if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
            SystemBarProperty statusSystemBarProperty(true, 0x00FFFFFF, 0xFF000000);
            SystemBarProperty navigationSystemBarProperty(true, 0x00FFFFFF, 0xFF000000);
            property_->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusSystemBarProperty);
            property_->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, navigationSystemBarProperty);
        }
    }
}

void WindowImpl::SetDefaultDisplayIdIfNeed()
{
    auto displayId = property_->GetDisplayId();
    if (displayId == DISPLAY_ID_INVALID) {
        auto defaultDisplayId = SingletonContainer::IsDestroyed() ? DISPLAY_ID_INVALID :
            SingletonContainer::Get<DisplayManager>().GetDefaultDisplayId();
        defaultDisplayId = (defaultDisplayId == DISPLAY_ID_INVALID)? 0 : defaultDisplayId;
        property_->SetDisplayId(defaultDisplayId);
        TLOGI(WmsLogTag::WMS_LIFE, "Reset displayId: %{public}" PRIu64, defaultDisplayId);
    }
}

WMError WindowImpl::Create(uint32_t parentId, const std::shared_ptr<AbilityRuntime::Context>& context)
{
    WLOGFD("Window[%{public}s] Create", name_.c_str());
    WMError ret = WindowCreateCheck(parentId);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    SetDefaultDisplayIdIfNeed();
    context_ = context;
    sptr<WindowImpl> window(this);
    sptr<IWindow> windowAgent(new WindowAgent(window));
    static std::atomic<uint32_t> tempWindowId = 0;
    uint32_t windowId = tempWindowId++; // for test
    sptr<IRemoteObject> token = context_ ? context_->GetToken() : nullptr;
    if (token) {
        property_->SetTokenState(true);
    }
    ChangePropertyByApiVersion();
    InitAbilityInfo();
    SetSystemConfig();

    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        GetConfigurationFromAbilityInfo();
    } else if (property_->GetWindowMode() == WindowMode::WINDOW_MODE_UNDEFINED) {
        property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    }

    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_VOLUME_OVERLAY && surfaceNode_) {
        surfaceNode_->SetFrameGravity(Gravity::TOP_LEFT);
    }

    ret = SingletonContainer::Get<WindowAdapter>().CreateWindow(windowAgent, property_, surfaceNode_,
        windowId, token);
    RecordLifeCycleExceptionEvent(LifeCycleEvent::CREATE_EVENT, ret);
    if (ret != WMError::WM_OK) {
        WLOGFE("create window failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    property_->SetWindowId(windowId);
    if (surfaceNode_) {
        surfaceNode_->SetWindowId(windowId);
    }
    sptr<Window> self(this);
    {
        std::unique_lock<std::shared_mutex> lock(windowMapMutex_);
        windowMap_.insert(std::make_pair(name_, std::pair<uint32_t, sptr<Window>>(windowId, self)));
    }
    if (parentId != INVALID_WINDOW_ID) {
        subWindowMap_[property_->GetParentId()].push_back(window);
    }

    MapFloatingWindowToAppIfNeeded();
    MapDialogWindowToAppIfNeeded();
    UpdateDecorEnable();

    state_ = WindowState::STATE_CREATED;
    InputTransferStation::GetInstance().AddInputWindow(self);
    needRemoveWindowInputChannel_ = true;
    RegisterWindowInspectorCallback();
    return ret;
}

WMError WindowImpl::GetWindowTypeForArkUI(WindowType parentWindowType, WindowType& windowType)
{
    if (parentWindowType == WindowType::WINDOW_TYPE_SCENE_BOARD ||
        parentWindowType == WindowType::WINDOW_TYPE_DESKTOP) {
        windowType = WindowType::WINDOW_TYPE_SYSTEM_FLOAT;
    } else if (WindowHelper::IsUIExtensionWindow(parentWindowType)) {
        windowType = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    } else if (WindowHelper::IsSystemWindow(parentWindowType)) {
        windowType = WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW;
    } else {
        windowType = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_SUB, "parentWindowType:%{public}d, windowType:%{public}d", parentWindowType, windowType);
    return WMError::WM_OK;
}

bool WindowImpl::PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (uiContent_ != nullptr) {
        return uiContent_->ProcessKeyEvent(keyEvent, true);
    }
    return false;
}

void WindowImpl::InitAbilityInfo()
{
    AbilityInfo info;
    info.bundleName_ = SysCapUtil::GetBundleName();
    auto originalAbilityInfo = GetOriginalAbilityInfo();
    if (originalAbilityInfo != nullptr) {
        info.abilityName_ = originalAbilityInfo->name;
    } else {
        WLOGFD("original ability info is null %{public}s", name_.c_str());
    }
    property_->SetAbilityInfo(info);
}

std::shared_ptr<AppExecFwk::AbilityInfo> WindowImpl::GetOriginalAbilityInfo() const
{
    if (context_ == nullptr) {
        WLOGFD("context is null %{public}s", name_.c_str());
        return nullptr;
    }

    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
    if (abilityContext == nullptr) {
        WLOGFD("abilityContext is null %{public}s", name_.c_str());
        return nullptr;
    }
    return abilityContext->GetAbilityInfo();
}

WMError WindowImpl::BindDialogTarget(sptr<IRemoteObject> targetToken)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t windowId = property_->GetWindowId();
    WMError ret = SingletonContainer::Get<WindowAdapter>().BindDialogTarget(windowId, targetToken);
    if (ret != WMError::WM_OK) {
        WLOGFE("bind window failed with errCode:%{public}d", static_cast<int32_t>(ret));
    }

    return ret;
}

void WindowImpl::ResetInputWindow(uint32_t winId)
{
    TLOGI(WmsLogTag::WMS_EVENT, "Id:%{public}u", winId);
    InputTransferStation::GetInstance().AddInputWindow(this);
}

void WindowImpl::DestroyDialogWindow()
{
    // remove from appDialogWindowMap_
    for (auto& dialogWindows: appDialogWindowMap_) {
        for (auto iter = dialogWindows.second.begin(); iter != dialogWindows.second.end(); ++iter) {
            if ((*iter) == nullptr) {
                continue;
            }
            if ((*iter)->GetWindowId() == GetWindowId()) {
                dialogWindows.second.erase(iter);
                break;
            }
        }
    }

    // Destroy app dialog window if exist
    if (appDialogWindowMap_.count(GetWindowId()) > 0) {
        auto& dialogWindows = appDialogWindowMap_.at(GetWindowId());
        for (auto iter = dialogWindows.begin(); iter != dialogWindows.end(); iter = dialogWindows.begin()) {
            if ((*iter) == nullptr) {
                dialogWindows.erase(iter);
                continue;
            }
            (*iter)->Destroy(false);
        }
        appDialogWindowMap_.erase(GetWindowId());
    }
}

void WindowImpl::DestroyFloatingWindow()
{
    // remove from appFloatingWindowMap_
    TLOGI(WmsLogTag::WMS_LIFE, "Remove from appFloatingWindowMap_");
    for (auto& floatingWindows: appFloatingWindowMap_) {
        for (auto iter = floatingWindows.second.begin(); iter != floatingWindows.second.end(); ++iter) {
            if ((*iter) == nullptr) {
                continue;
            }
            if ((*iter)->GetWindowId() == GetWindowId()) {
                floatingWindows.second.erase(iter);
                break;
            }
        }
    }

    // Destroy app floating window if exist
    TLOGI(WmsLogTag::WMS_LIFE, "Destroy app floating window if exist");
    if (appFloatingWindowMap_.count(GetWindowId()) > 0) {
        auto& floatingWindows = appFloatingWindowMap_.at(GetWindowId());
        for (auto iter = floatingWindows.begin(); iter != floatingWindows.end(); iter = floatingWindows.begin()) {
            if ((*iter) == nullptr) {
                floatingWindows.erase(iter);
                continue;
            }
            (*iter)->Destroy();
        }
        appFloatingWindowMap_.erase(GetWindowId());
    }
}

void WindowImpl::DestroySubWindow()
{
    if (subWindowMap_.count(property_->GetParentId()) > 0) { // remove from subWindowMap_
        auto& subWindows = subWindowMap_.at(property_->GetParentId());
        for (auto iter = subWindows.begin(); iter < subWindows.end(); ++iter) {
            if ((*iter) == nullptr) {
                continue;
            }
            if ((*iter)->GetWindowId() == GetWindowId()) {
                subWindows.erase(iter);
                break;
            }
        }
    }

    if (subWindowMap_.count(GetWindowId()) > 0) { // remove from subWindowMap_ and windowMap_
        auto& subWindows = subWindowMap_.at(GetWindowId());
        for (auto iter = subWindows.begin(); iter != subWindows.end(); iter = subWindows.begin()) {
            if ((*iter) == nullptr) {
                subWindows.erase(iter);
                continue;
            }
            (*iter)->Destroy(false);
        }
        subWindowMap_[GetWindowId()].clear();
        subWindowMap_.erase(GetWindowId());
    }
}

void WindowImpl::ClearVsyncStation()
{
    if (vsyncStation_ != nullptr) {
        vsyncStation_->Destroy();
    }
}

WMError WindowImpl::Destroy(uint32_t reason)
{
    return Destroy(true, true, reason);
}

WMError WindowImpl::Destroy(bool needNotifyServer, bool needClearListener, uint32_t reason)
{
    if (!IsWindowValid()) {
        return WMError::WM_OK;
    }

    WLOGI("Window %{public}u Destroy, reason: %{public}u", property_->GetWindowId(), reason);
    WindowInspector::GetInstance().UnregisterGetWMSWindowListCallback(GetWindowId());
    WMError ret = WMError::WM_OK;
    if (needNotifyServer) {
        NotifyBeforeDestroy(GetWindowName());
        if (subWindowMap_.count(GetWindowId()) > 0) {
            for (auto& subWindow : subWindowMap_.at(GetWindowId())) {
                NotifyBeforeSubWindowDestroy(subWindow);
            }
        }
        ret = SingletonContainer::Get<WindowAdapter>().DestroyWindow(property_->GetWindowId());
        RecordLifeCycleExceptionEvent(LifeCycleEvent::DESTROY_EVENT, ret);
        if (ret != WMError::WM_OK) {
            WLOGFE("destroy window failed with errCode:%{public}d", static_cast<int32_t>(ret));
            if (GetType() != WindowType::WINDOW_TYPE_DIALOG) {
                return ret;
            }
        }
    } else {
        WLOGI("no need to destroy");
    }

    if (needRemoveWindowInputChannel_) {
        InputTransferStation::GetInstance().RemoveInputWindow(property_->GetWindowId());
    }
    {
        std::unique_lock<std::shared_mutex> lock(windowMapMutex_);
        windowMap_.erase(GetWindowName());
    }
    if (needClearListener) {
        ClearListenersById(GetWindowId());
    }
    DestroySubWindow();
    DestroyFloatingWindow();
    DestroyDialogWindow();
    ClearVsyncStation();
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        state_ = WindowState::STATE_DESTROYED;
    }
    return ret;
}

bool WindowImpl::NeedToStopShowing()
{
    if (!WindowHelper::IsMainWindow(property_->GetWindowType())) {
        return false;
    }
    // show failed when current mode is not support or window only supports split mode and can show when locked
    bool isShowWhenLocked = GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    if (!WindowHelper::IsWindowModeSupported(GetWindowModeSupportType(), GetWindowMode()) ||
        WindowHelper::IsOnlySupportSplitAndShowWhenLocked(isShowWhenLocked, GetWindowModeSupportType())) {
        WLOGFE("current mode is not supported, windowId: %{public}u, "
            "windowModeSupportType: %{public}u, winMode: %{public}u",
            property_->GetWindowId(), GetWindowModeSupportType(), GetWindowMode());
        return true;
    }
    return false;
}

WMError WindowImpl::UpdateSurfaceNodeAfterCustomAnimation(bool isAdd)
{
    WLOGI("id: %{public}u UpdateRsTree, isAdd:%{public}u",
          property_->GetWindowId(), isAdd);
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsSystemWindow(property_->GetWindowType())) {
        WLOGFE("only system window can set");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    AdjustWindowAnimationFlag(false); // false means update rs tree with default option
    // need time out check
    WMError ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
    if (ret != WMError::WM_OK) {
        WLOGFE("UpdateProperty failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    ret = SingletonContainer::Get<WindowAdapter>().UpdateRsTree(property_->GetWindowId(), isAdd);
    if (ret != WMError::WM_OK) {
        WLOGFE("UpdateRsTree failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    return WMError::WM_OK;
}

void WindowImpl::AdjustWindowAnimationFlag(bool withAnimation)
{
    // when show/hide with animation
    // use custom animation when transitionController exists; else use default animation
    WindowType winType = property_->GetWindowType();
    bool isAppWindow = WindowHelper::IsAppWindow(winType);
    if (withAnimation && !isAppWindow && animationTransitionController_) {
        // use custom animation
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
    } else if ((isAppWindow && needDefaultAnimation_) || (withAnimation && !animationTransitionController_)) {
        // use default animation
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::DEFAULT));
    } else if (winType == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::INPUTE));
    } else {
        // with no animation
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::NONE));
    }
}

WMError WindowImpl::PreProcessShow(uint32_t reason, bool withAnimation)
{
    if (state_ == WindowState::STATE_FROZEN) {
        WLOGFE("window is frozen, can not be shown, windowId: %{public}u", property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    SetDefaultOption();
    SetWindowModeSupportType(GetRequestWindowModeSupportType());
    AdjustWindowAnimationFlag(withAnimation);

    if (NeedToStopShowing()) { // true means stop showing
        return WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }

    // update title button visibility when show
    UpdateTitleButtonVisibility();
    return WMError::WM_OK;
}

void WindowImpl::NotifyMainWindowDidForeground(uint32_t reason)
{
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        NotifyAfterDidForeground(reason);
    }
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
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, __PRETTY_FUNCTION__);
    WLOGFD("Window Show [name:%{public}s, id:%{public}u, mode: %{public}u], reason:%{public}u, "
        "withAnimation:%{public}d", name_.c_str(), property_->GetWindowId(), GetWindowMode(), reason, withAnimation);
    if (IsShowWithOptions()) {
        SetShowWithOptions(false);
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    UpdateDecorEnable(true);
    if (static_cast<WindowStateChangeReason>(reason) == WindowStateChangeReason::KEYGUARD ||
        static_cast<WindowStateChangeReason>(reason) == WindowStateChangeReason::TOGGLING) {
        state_ = WindowState::STATE_SHOWN;
        NotifyAfterForeground();
        return WMError::WM_OK;
    }
    if (state_ == WindowState::STATE_SHOWN) {
        if (property_->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
            SingletonContainer::Get<WindowAdapter>().MinimizeAllAppWindows(property_->GetDisplayId());
        } else {
            WLOGI("window is already shown id: %{public}u", property_->GetWindowId());
            SingletonContainer::Get<WindowAdapter>().ProcessPointDown(property_->GetWindowId(), false);
        }
        // when show sub window, check its parent state
        sptr<Window> parent = FindWindowById(property_->GetParentId());
        if (parent != nullptr && parent->GetWindowState() == WindowState::STATE_HIDDEN) {
            WLOGFD("sub window can not show, because main window hide");
            return WMError::WM_OK;
        } else {
            NotifyAfterForeground(true, false);
        }
        NotifyMainWindowDidForeground(reason);
        return WMError::WM_OK;
    }
    WMError ret = PreProcessShow(reason, withAnimation);
    if (ret != WMError::WM_OK) {
        NotifyForegroundFailed(ret);
        return ret;
    }
    // this lock solves the multithreading problem when reading WindowState
    std::lock_guard<std::recursive_mutex> lock(windowStateMutex_);
    ret = SingletonContainer::Get<WindowAdapter>().AddWindow(property_);
    RecordLifeCycleExceptionEvent(LifeCycleEvent::SHOW_EVENT, ret);
    if (ret == WMError::WM_OK) {
        UpdateWindowStateWhenShow();
        NotifyMainWindowDidForeground(reason);
    } else {
        NotifyForegroundFailed(ret);
        WLOGFE("show window id:%{public}u errCode:%{public}d", property_->GetWindowId(), static_cast<int32_t>(ret));
    }
    // systemui make startbar resident, when refactor immersive, this code can delete
    if (property_->GetRequestedOrientation() == Orientation::HORIZONTAL
        || property_->GetRequestedOrientation() == Orientation::REVERSE_HORIZONTAL) {
        RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    }
    needNotifyFocusLater_ = false;
    return ret;
}

WMError WindowImpl::ShowKeyboard(uint32_t callingWindowId, uint64_t tgtDisplayId, KeyboardEffectOption effectOption)
{
    return Show();
}

WMError WindowImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    return Hide(reason, withAnimation, isFromInnerkits, false);
}

WMError WindowImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach)
{
    WLOGD("id:%{public}u Hide, reason:%{public}u, Animation:%{public}d",
        property_->GetWindowId(), reason, withAnimation);
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WindowStateChangeReason stateChangeReason = static_cast<WindowStateChangeReason>(reason);
    if (stateChangeReason == WindowStateChangeReason::KEYGUARD ||
        stateChangeReason == WindowStateChangeReason::TOGGLING) {
        state_ = stateChangeReason == WindowStateChangeReason::KEYGUARD ?
            WindowState::STATE_FROZEN : WindowState::STATE_HIDDEN;
        NotifyAfterBackground();
        return WMError::WM_OK;
    }
    if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        WLOGI("already hidden, id: %{public}u", property_->GetWindowId());
        NotifyBackgroundFailed(WMError::WM_DO_NOTHING);
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (WindowHelper::IsSystemWindow(property_->GetWindowType())) {
        AdjustWindowAnimationFlag(withAnimation);
        // when show(true) with default, hide() with None, to adjust animationFlag to disabled default animation
        ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
        if (ret != WMError::WM_OK) {
            WLOGFE("UpdateProperty failed with errCode:%{public}d", static_cast<int32_t>(ret));
            return ret;
        }
    }
    ret = SingletonContainer::Get<WindowAdapter>().RemoveWindow(property_->GetWindowId(), isFromInnerkits);
    RecordLifeCycleExceptionEvent(LifeCycleEvent::HIDE_EVENT, ret);
    if (ret != WMError::WM_OK) {
        WLOGFE("hide errCode:%{public}d for winId:%{public}u", static_cast<int32_t>(ret), property_->GetWindowId());
        return ret;
    }
    UpdateWindowStateWhenHide();
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        NotifyAfterDidBackground(reason);
    }
    uint32_t animationFlag = property_->GetAnimationFlag();
    if (animationFlag == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
        animationTransitionController_->AnimationForHidden();
    }
    ResetMoveOrDragState();
    escKeyEventTriggered_ = false;
    return ret;
}

WMError WindowImpl::MoveTo(int32_t x, int32_t y, bool isMoveToGlobal, MoveConfiguration moveConfiguration)
{
    WLOGFD("id:%{public}d MoveTo %{public}d %{public}d",
          property_->GetWindowId(), x, y);
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    Rect rect = (WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) ?
        GetRect() : property_->GetRequestRect();
    Rect moveRect = { x, y, rect.width_, rect.height_ }; // must keep w/h, which may maintain stashed resize info
    property_->SetRequestRect(moveRect);
    {
        // this lock solves the multithreading problem when reading WindowState
        std::lock_guard<std::recursive_mutex> lock(windowStateMutex_);
        if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        WLOGFD("window is hidden or created! id: %{public}u, oriPos: [%{public}d, %{public}d, "
               "movePos: [%{public}d, %{public}d]", property_->GetWindowId(), rect.posX_, rect.posY_, x, y);
        return WMError::WM_OK;
        }
    }

    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
        WLOGFE("fullscreen window could not moveto, winId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    property_->SetWindowSizeChangeReason(WindowSizeChangeReason::MOVE);
    return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_RECT);
}

WMError WindowImpl::Resize(uint32_t width, uint32_t height, const RectAnimationConfig& rectAnimationConfig)
{
    WLOGFD("id:%{public}d Resize %{public}u %{public}u",
          property_->GetWindowId(), width, height);
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    Rect rect = (WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) ?
        GetRect() : property_->GetRequestRect();
    Rect resizeRect = { rect.posX_, rect.posY_, width, height };
    property_->SetRequestRect(resizeRect);
    property_->SetDecoStatus(false);
    {
        // this lock solves the multithreading problem when reading WindowState
        std::lock_guard<std::recursive_mutex> lock(windowStateMutex_);
        if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        WLOGFD("window is hidden or created! id: %{public}u, oriRect: [%{public}u, %{public}u], "
               "resizeRect: [%{public}u, %{public}u]", property_->GetWindowId(), rect.width_,
               rect.height_, width, height);
        return WMError::WM_OK;
        }
    }

    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
        WLOGFE("fullscreen window could not resize, winId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    property_->SetWindowSizeChangeReason(WindowSizeChangeReason::RESIZE);
    return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_RECT);
}

WMError WindowImpl::SetWindowGravity(WindowGravity gravity, uint32_t percent)
{
    WLOGFD("id:%{public}d SetWindowGravity %{public}u %{public}u",
        property_->GetWindowId(), gravity, percent);

    return SingletonContainer::Get<WindowAdapter>().SetWindowGravity(property_->GetWindowId(), gravity, percent);
}

WMError WindowImpl::SetKeepScreenOn(bool keepScreenOn)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetKeepScreenOn(keepScreenOn);
    if (state_ == WindowState::STATE_SHOWN) {
        return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    }
    return WMError::WM_OK;
}

bool WindowImpl::IsKeepScreenOn() const
{
    if (!IsWindowValid()) {
        return false;
    }
    return property_->IsKeepScreenOn();
}

WMError WindowImpl::SetTurnScreenOn(bool turnScreenOn)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetTurnScreenOn(turnScreenOn);
    if (state_ == WindowState::STATE_SHOWN) {
        return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    }
    return WMError::WM_OK;
}

bool WindowImpl::IsTurnScreenOn() const
{
    if (!IsWindowValid()) {
        return false;
    }
    return property_->IsTurnScreenOn();
}

WMError WindowImpl::SetBackgroundColor(uint32_t color)
{
    // 0xff000000: ARGB style, means Opaque color.
    const bool isAlphaZero = !(color & 0xff000000);
    auto abilityInfo = property_->GetAbilityInfo();
    if (isAlphaZero && WindowHelper::IsMainWindow(property_->GetWindowType())) {
        auto& reportInstance = SingletonContainer::Get<WindowInfoReporter>();
        reportInstance.ReportZeroOpacityInfoImmediately(abilityInfo.bundleName_,
            abilityInfo.abilityName_);
    }

    if (uiContent_ != nullptr) {
        uiContent_->SetBackgroundColor(color);
        return WMError::WM_OK;
    }
    WLOGI("ace is null, Id: %{public}u", GetWindowId());
    if (aceAbilityHandler_ != nullptr) {
        aceAbilityHandler_->SetBackgroundColor(color);
        return WMError::WM_OK;
    }
    WLOGFE("FA mode could not set bg color: %{public}u", GetWindowId());
    return WMError::WM_ERROR_INVALID_OPERATION;
}

uint32_t WindowImpl::GetBackgroundColor() const
{
    if (uiContent_ != nullptr) {
        return uiContent_->GetBackgroundColor();
    }
    WLOGD("uiContent is nullptr, windowId: %{public}u, use FA mode", GetWindowId());
    if (aceAbilityHandler_ != nullptr) {
        return aceAbilityHandler_->GetBackgroundColor();
    }
    WLOGFE("FA mode does not get bg color: %{public}u", GetWindowId());
    return 0xffffffff; // means no background color been set, default color is white
}

WMError WindowImpl::SetBackgroundColor(const std::string& color)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t colorValue;
    if (ColorParser::Parse(color, colorValue)) {
        WLOGD("SetBackgroundColor: window: %{public}s, value: [%{public}s, %{public}u]",
            name_.c_str(), color.c_str(), colorValue);
        return SetBackgroundColor(colorValue);
    }
    WLOGFE("invalid color string: %{public}s", color.c_str());
    return WMError::WM_ERROR_INVALID_PARAM;
}

WMError WindowImpl::SetTransparent(bool isTransparent)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    ColorParam backgroundColor;
    backgroundColor.value = GetBackgroundColor();
    if (isTransparent) {
        backgroundColor.argb.alpha = 0x00; // 0x00: completely transparent
        return SetBackgroundColor(backgroundColor.value);
    } else {
        backgroundColor.value = GetBackgroundColor();
        if (backgroundColor.argb.alpha == 0x00) {
            backgroundColor.argb.alpha = 0xff; // 0xff: completely opaque
            return SetBackgroundColor(backgroundColor.value);
        }
    }
    return WMError::WM_OK;
}

bool WindowImpl::IsTransparent() const
{
    if (!IsWindowValid()) {
        return false;
    }
    ColorParam backgroundColor;
    backgroundColor.value = GetBackgroundColor();
    WLOGFD("color: %{public}u, alpha: %{public}u", backgroundColor.value, backgroundColor.argb.alpha);
    return backgroundColor.argb.alpha == 0x00; // 0x00: completely transparent
}

WMError WindowImpl::SetBrightness(float brightness)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if ((brightness < MINIMUM_BRIGHTNESS &&
         std::fabs(brightness - UNDEFINED_BRIGHTNESS) >= std::numeric_limits<float>::min()) ||
         brightness > MAXIMUM_BRIGHTNESS) {
        WLOGFE("invalid brightness value: %{public}f", brightness);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!WindowHelper::IsAppWindow(GetType())) {
        WLOGFE("non app window does not support set brightness, type: %{public}u", GetType());
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    property_->SetBrightness(brightness);
    if (state_ == WindowState::STATE_SHOWN) {
        return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);
    }
    return WMError::WM_OK;
}

float WindowImpl::GetBrightness() const
{
    return property_->GetBrightness();
}

WMError WindowImpl::ChangeCallingWindowId(uint32_t windowId)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->ChangeCallingWindowId(windowId);
    return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_CALLING_WINDOW);
}

void WindowImpl::RecordLifeCycleExceptionEvent(LifeCycleEvent event, WMError errCode) const
{
    if (!(errCode > WMError::WM_ERROR_NEED_REPORT_BASE && errCode < WMError::WM_ERROR_NEED_REPORT_END)) {
        return;
    }
    std::ostringstream oss;
    oss << "life cycle is abnormal: " << "window_name: " << name_
        << ", id:" << GetWindowId() << ", event: " << TransferLifeCycleEventToString(event)
        << ", errCode: " << static_cast<int32_t>(errCode) << ";";
    std::string info = oss.str();
    WLOGI("window life cycle exception: %{public}s", info.c_str());
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "WINDOW_LIFE_CYCLE_EXCEPTION",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", getpid(),
        "UID", getuid(),
        "MSG", info);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

std::string WindowImpl::TransferLifeCycleEventToString(LifeCycleEvent type) const
{
    std::string event;
    switch (type) {
        case LifeCycleEvent::CREATE_EVENT:
            event = "CREATE";
            break;
        case LifeCycleEvent::SHOW_EVENT:
            event = "SHOW";
            break;
        case LifeCycleEvent::HIDE_EVENT:
            event = "HIDE";
            break;
        case LifeCycleEvent::DESTROY_EVENT:
            event = "DESTROY";
            break;
        default:
            event = "UNDEFINE";
            break;
    }
    return event;
}

WMError WindowImpl::SetPrivacyMode(bool isPrivacyMode)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WLOGFD("id : %{public}u, SetPrivacyMode, %{public}u", GetWindowId(), isPrivacyMode);
    property_->SetPrivacyMode(isPrivacyMode);
    return UpdateProperty(PropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
}

bool WindowImpl::IsPrivacyMode() const
{
    if (!IsWindowValid()) {
        return false;
    }
    return property_->GetPrivacyMode();
}

void WindowImpl::SetSystemPrivacyMode(bool isSystemPrivacyMode)
{
    WLOGFD("id : %{public}u, SetSystemPrivacyMode, %{public}u", GetWindowId(), isSystemPrivacyMode);
    property_->SetSystemPrivacyMode(isSystemPrivacyMode);
    UpdateProperty(PropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
}

WMError WindowImpl::SetSnapshotSkip(bool isSkip)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set snapshot skip permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    property_->SetSnapshotSkip(isSkip);
    auto ret = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    WLOGFD("id : %{public}u, set snapshot skip end. isSkip:%{public}u, systemPrivacyMode:%{public}u, ret:%{public}u",
        GetWindowId(), isSkip, property_->GetSystemPrivacyMode(), ret);
    return WMError::WM_OK;
}

/** @note @window.hierarchy */
WMError WindowImpl::RaiseToAppTop()
{
    if (!IsWindowValid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Window is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    auto parentId = property_->GetParentId();
    if (parentId == INVALID_WINDOW_ID) {
        WLOGFE("Only the children of the main window can be raised!");
        return WMError::WM_ERROR_INVALID_PARENT;
    }

    if (!WindowHelper::IsSubWindow(property_->GetWindowType())) {
        WLOGFE("Must be app sub window window!");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    if (state_ != WindowState::STATE_SHOWN) {
        WLOGFE("The sub window must be shown!");
        return WMError::WM_DO_NOTHING;
    }

    return SingletonContainer::Get<WindowAdapter>().RaiseToAppTop(GetWindowId());
}

WMError WindowImpl::DisableAppWindowDecor()
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("disable app window decor permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!WindowHelper::IsMainWindow(property_->GetWindowType())) {
        WLOGFE("window decoration is invalid on sub window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    WLOGI("disable app window decoration.");
    windowSystemConfig_.isSystemDecorEnable_ = false;
    UpdateDecorEnable(true);
    return WMError::WM_OK;
}

bool WindowImpl::IsDecorEnable() const
{
    bool enable = windowSystemConfig_.isSystemDecorEnable_ &&
        WindowHelper::IsMainWindow(property_->GetWindowType());
    WLOGFD("get decor enable %{public}d", enable);
    return enable;
}

WMError WindowImpl::Maximize()
{
    WLOGI("id: %{public}u Maximize", property_->GetWindowId());
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        return SetFullScreen(true);
    } else {
        WLOGI("Maximize fail, not main window");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
}

WMError WindowImpl::MaximizeFloating()
{
    WLOGI("id: %{public}u MaximizeFloating", property_->GetWindowId());
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        return SetFloatingMaximize(true);
    } else {
        WLOGI("MaximizeFloating fail, not main window");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
}

WMError WindowImpl::SetGlobalMaximizeMode(MaximizeMode mode)
{
    WLOGI("id: %{public}u SetGlobalMaximizeMode: %{public}u", property_->GetWindowId(),
        static_cast<uint32_t>(mode));
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        SingletonContainer::Get<WindowAdapter>().SetMaximizeMode(mode);
        return WMError::WM_OK;
    } else {
        WLOGI("SetGlobalMaximizeMode fail, not main window");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
}

MaximizeMode WindowImpl::GetGlobalMaximizeMode() const
{
    return SingletonContainer::Get<WindowAdapter>().GetMaximizeMode();
}

WMError WindowImpl::SetImmersiveModeEnabledState(bool enable)
{
    TLOGD(WmsLogTag::WMS_IMMS, "WindowImpl id: %{public}u SetImmersiveModeEnabledState: %{public}u",
        property_->GetWindowId(), static_cast<uint32_t>(enable));
    if (!IsWindowValid() ||
        !WindowHelper::IsWindowModeSupported(GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FULLSCREEN)) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid window or fullscreen mode is not be supported, winId:%{public}u",
            property_->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const WindowType curWindowType = GetType();
    if (!WindowHelper::IsMainWindow(curWindowType) && !WindowHelper::IsSubWindow(curWindowType)) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    enableImmersiveMode_ = enable;
    const WindowMode mode = GetWindowMode();
    if (mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
        return SetLayoutFullScreen(enableImmersiveMode_);
    }
    return WMError::WM_OK;
}

bool WindowImpl::GetImmersiveModeEnabledState() const
{
    if (!IsWindowValid()) {
        return false;
    }
    return enableImmersiveMode_;
}

WMError WindowImpl::NotifyWindowTransition(TransitionReason reason)
{
    sptr<WindowTransitionInfo> fromInfo = new(std::nothrow) WindowTransitionInfo();
    sptr<WindowTransitionInfo> toInfo = new(std::nothrow) WindowTransitionInfo();
    if (fromInfo == nullptr || toInfo == nullptr) {
        WLOGFE("new windowTransitionInfo failed");
        return WMError::WM_ERROR_NO_MEM;
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
    if (abilityContext == nullptr) {
        WLOGFE("id:%{public}d is not ability Window", property_->GetWindowId());
        return WMError::WM_ERROR_NO_MEM;
    }
    auto abilityInfo = abilityContext->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    fromInfo->SetBundleName(context_->GetBundleName());
    fromInfo->SetAbilityName(abilityInfo->name);
    fromInfo->SetWindowMode(property_->GetWindowMode());
    fromInfo->SetWindowRect(property_->GetWindowRect());
    fromInfo->SetAbilityToken(context_->GetToken());
    fromInfo->SetWindowType(property_->GetWindowType());
    fromInfo->SetDisplayId(property_->GetDisplayId());
    fromInfo->SetTransitionReason(reason);
    return SingletonContainer::Get<WindowAdapter>().NotifyWindowTransition(fromInfo, toInfo);
}

WMError WindowImpl::Minimize()
{
    WLOGI("id: %{public}u Minimize", property_->GetWindowId());
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        if (context_ != nullptr) {
            WMError ret = NotifyWindowTransition(TransitionReason::MINIMIZE);
            if (ret != WMError::WM_OK) {
                WLOGI("Minimize without animation ret:%{public}u", static_cast<uint32_t>(ret));
                AAFwk::AbilityManagerClient::GetInstance()->MinimizeAbility(context_->GetToken(), true);
            }
        } else {
            Hide();
        }
    }
    return WMError::WM_OK;
}

WMError WindowImpl::Recover()
{
    WLOGI("id: %{public}u Normalize", property_->GetWindowId());
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        if (property_->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
            property_->GetMaximizeMode() == MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
            SetFloatingMaximize(false);
            return WMError::WM_OK;
        }
        SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    }
    return WMError::WM_OK;
}

WMError WindowImpl::Close()
{
    WLOGI("id: %{public}u Close", property_->GetWindowId());
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
        if (!abilityContext) {
            return Destroy();
        }
        sptr<AAFwk::IPrepareTerminateCallback> callback = this;
        if (AAFwk::AbilityManagerClient::GetInstance()->PrepareTerminateAbility(abilityContext->GetToken(),
            callback) != ERR_OK) {
            WLOGFW("RegisterWindowManagerServiceHandler failed, do close window");
            PendingClose();
            return WMError::WM_OK;
        }
    }
    return WMError::WM_OK;
}

void WindowImpl::DoPrepareTerminate()
{
    WLOGFI("do pending close by ability");
    PendingClose();
}

void WindowImpl::PendingClose()
{
    WLOGFD("begin");
    WMError ret = NotifyWindowTransition(TransitionReason::CLOSE_BUTTON);
    if (ret != WMError::WM_OK) {
        WLOGI("Close without animation ret:%{public}u", static_cast<uint32_t>(ret));
        auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
        if (abilityContext != nullptr) {
            abilityContext->CloseAbility();
        }
    }
}

WMError WindowImpl::RequestFocus() const
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().RequestFocus(property_->GetWindowId());
}

void WindowImpl::SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    inputEventConsumer_ = inputEventConsumer;
}

WMError WindowImpl::RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(lifecycleListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(lifecycleListeners_[GetWindowId()], listener);
}

WMError WindowImpl::RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(windowChangeListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(windowChangeListeners_[GetWindowId()], listener);
}

WMError WindowImpl::RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    WMError ret = RegisterListener(avoidAreaChangeListeners_[GetWindowId()], listener);
    if (avoidAreaChangeListeners_[GetWindowId()].size() == 1) {
        SingletonContainer::Get<WindowAdapter>().UpdateAvoidAreaListener(property_->GetWindowId(), true);
    }
    return ret;
}

WMError WindowImpl::UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    WMError ret = UnregisterListener(avoidAreaChangeListeners_[GetWindowId()], listener);
    if (avoidAreaChangeListeners_[GetWindowId()].empty()) {
        SingletonContainer::Get<WindowAdapter>().UpdateAvoidAreaListener(property_->GetWindowId(), false);
    }
    return ret;
}

WMError WindowImpl::RegisterDragListener(const sptr<IWindowDragListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return RegisterListener(windowDragListeners_, listener);
}

WMError WindowImpl::UnregisterDragListener(const sptr<IWindowDragListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return UnregisterListener(windowDragListeners_, listener);
}

WMError WindowImpl::RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return RegisterListener(displayMoveListeners_, listener);
}

WMError WindowImpl::UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return UnregisterListener(displayMoveListeners_, listener);
}

void WindowImpl::RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func)
{
    WLOGFD("Start register");
    notifyNativefunc_ = std::move(func);
}

WMError WindowImpl::RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(occupiedAreaChangeListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(occupiedAreaChangeListeners_[GetWindowId()], listener);
}

WMError WindowImpl::RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(touchOutsideListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(touchOutsideListeners_[GetWindowId()], listener);
}

WMError WindowImpl::RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("register animation transition controller permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    animationTransitionController_ = listener;
    wptr<WindowProperty> propertyToken(property_);
    wptr<IAnimationTransitionController> animationTransitionControllerToken(animationTransitionController_);
    if (uiContent_) {
        uiContent_->SetNextFrameLayoutCallback([propertyToken, animationTransitionControllerToken]() {
            auto property = propertyToken.promote();
            auto animationTransitionController = animationTransitionControllerToken.promote();
            if (!property || !animationTransitionController) {
                return;
            }
            uint32_t animationFlag = property->GetAnimationFlag();
            if (animationFlag == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
                // CustomAnimation is enabled when animationTransitionController_ exists
                animationTransitionController->AnimationForShown();
            }
        });
    }
    return WMError::WM_OK;
}

WMError WindowImpl::RegisterScreenshotListener(const sptr<IScreenshotListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(screenshotListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(screenshotListeners_[GetWindowId()], listener);
}

WMError WindowImpl::RegisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u start register", GetWindowId());
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(screenshotAppEventListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u start unregister", GetWindowId());
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(screenshotAppEventListeners_[GetWindowId()], listener);
}

WMError WindowImpl::RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
{
    WLOGFD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(dialogTargetTouchListeners_[GetWindowId()], listener);
}

WMError WindowImpl::UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(dialogTargetTouchListeners_[GetWindowId()], listener);
}

void WindowImpl::RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener)
{
    WLOGFD("Start register");
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    dialogDeathRecipientListener_[GetWindowId()] = listener;
}

void WindowImpl::UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener)
{
    WLOGFD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    dialogDeathRecipientListener_[GetWindowId()] = nullptr;
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

template <typename T>
EnableIfSame<T, IWindowLifeCycle, std::vector<sptr<IWindowLifeCycle>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IWindowLifeCycle>> lifecycleListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : lifecycleListeners_[GetWindowId()]) {
            lifecycleListeners.push_back(listener);
        }
    }
    return lifecycleListeners;
}

template <typename T>
EnableIfSame<T, IWindowChangeListener, std::vector<sptr<IWindowChangeListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IWindowChangeListener>> windowChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : windowChangeListeners_[GetWindowId()]) {
            windowChangeListeners.push_back(listener);
        }
    }
    return windowChangeListeners;
}

template <typename T>
EnableIfSame<T, IAvoidAreaChangedListener, std::vector<sptr<IAvoidAreaChangedListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IAvoidAreaChangedListener>> avoidAreaChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : avoidAreaChangeListeners_[GetWindowId()]) {
            avoidAreaChangeListeners.push_back(listener);
        }
    }
    return avoidAreaChangeListeners;
}

template <typename T>
EnableIfSame<T, IDisplayMoveListener, std::vector<sptr<IDisplayMoveListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IDisplayMoveListener>> displayMoveListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        for (auto &listener : displayMoveListeners_) {
            displayMoveListeners.push_back(listener);
        }
    }
    return displayMoveListeners;
}

template <typename T>
EnableIfSame<T, IScreenshotListener, std::vector<sptr<IScreenshotListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IScreenshotListener>> screenshotListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : screenshotListeners_[GetWindowId()]) {
            screenshotListeners.push_back(listener);
        }
    }
    return screenshotListeners;
}

template <typename T>
EnableIfSame<T, IScreenshotAppEventListener, std::vector<IScreenshotAppEventListenerSptr>> WindowImpl::GetListeners()
{
    std::vector<IScreenshotAppEventListenerSptr> screenshotAppEventListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : screenshotAppEventListeners_[GetWindowId()]) {
            screenshotAppEventListeners.push_back(listener);
        }
    }
    return screenshotAppEventListeners;
}

template <typename T>
EnableIfSame<T, ITouchOutsideListener, std::vector<sptr<ITouchOutsideListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<ITouchOutsideListener>> touchOutsideListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : touchOutsideListeners_[GetWindowId()]) {
            touchOutsideListeners.push_back(listener);
        }
    }
    return touchOutsideListeners;
}

template <typename T>
EnableIfSame<T, IDialogTargetTouchListener, std::vector<sptr<IDialogTargetTouchListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IDialogTargetTouchListener>> dialogTargetTouchListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : dialogTargetTouchListeners_[GetWindowId()]) {
            dialogTargetTouchListeners.push_back(listener);
        }
    }
    return dialogTargetTouchListeners;
}

template <typename T>
EnableIfSame<T, IWindowDragListener, std::vector<sptr<IWindowDragListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IWindowDragListener>> windowDragListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        for (auto &listener : windowDragListeners_) {
            windowDragListeners.push_back(listener);
        }
    }
    return windowDragListeners;
}

template <typename T>
EnableIfSame<T, IOccupiedAreaChangeListener, std::vector<sptr<IOccupiedAreaChangeListener>>> WindowImpl::GetListeners()
{
    std::vector<sptr<IOccupiedAreaChangeListener>> occupiedAreaChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        for (auto &listener : occupiedAreaChangeListeners_[GetWindowId()]) {
            occupiedAreaChangeListeners.push_back(listener);
        }
    }
    return occupiedAreaChangeListeners;
}

template <typename T>
EnableIfSame<T, IDialogDeathRecipientListener, wptr<IDialogDeathRecipientListener>> WindowImpl::GetListener()
{
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return dialogDeathRecipientListener_[GetWindowId()];
}

void WindowImpl::SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler)
{
    if (handler == nullptr) {
        WLOGI("ace ability handler is nullptr");
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    aceAbilityHandler_ = handler;
}

void WindowImpl::SetRequestWindowModeSupportType(uint32_t windowModeSupportType)
{
    property_->SetRequestWindowModeSupportType(windowModeSupportType);
    SetWindowModeSupportType(windowModeSupportType);
}

void WindowImpl::SetWindowModeSupportType(uint32_t windowModeSupportType)
{
    property_->SetWindowModeSupportType(windowModeSupportType);
}

void WindowImpl::UpdateRect(const struct Rect& rect, bool decoStatus, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (state_ == WindowState::STATE_DESTROYED) {
        WLOGFW("invalid window state");
        return;
    }
    auto display = SingletonContainer::IsDestroyed() ? nullptr :
        SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        WLOGFE("get display failed displayId:%{public}" PRIu64", window id:%{public}u", property_->GetDisplayId(),
            property_->GetWindowId());
        return;
    }
    Rect lastOriRect = property_->GetWindowRect();

    property_->SetDecoStatus(decoStatus);
    if (reason == WindowSizeChangeReason::HIDE) {
        property_->SetRequestRect(rect);
        return;
    }
    property_->SetWindowRect(rect);

    // update originRect when floating window show for the first time.
    if (!isOriginRectSet_ && WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) {
        property_->SetOriginRect(rect);
        isOriginRectSet_ = true;
    }
    WLOGFD("winId:%{public}u, rect[%{public}d, %{public}d, %{public}u, %{public}u], reason:%{public}u",
        property_->GetWindowId(), rect.posX_, rect.posY_, rect.width_, rect.height_, reason);
    Rect rectToAce = rect;
    // update rectToAce for stretchable window
    if (windowSystemConfig_.isStretchable_ && WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) {
        if (IsStretchableReason(reason)) {
            rectToAce = property_->GetOriginRect();
        } else {
            property_->SetOriginRect(rect);
        }
    }
    ScheduleUpdateRectTask(rectToAce, lastOriRect, reason, rsTransaction, display);
}

void WindowImpl::ScheduleUpdateRectTask(const Rect& rectToAce, const Rect& lastOriRect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction, const sptr<class Display>& display)
{
    auto task = [weakThis = wptr(this), reason, rsTransaction, rectToAce, lastOriRect, display]() mutable {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is null");
            return;
        }
        auto rsUIContext = window->GetRSUIContext();
        if (rsTransaction) {
            RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
            rsTransaction->Begin();
        }
        RSAnimationTimingProtocol protocol;
        protocol.SetDuration(600);
        auto curve = RSAnimationTimingCurve::CreateCubicCurve(0.2, 0.0, 0.2, 1.0);
        RSNode::OpenImplicitAnimation(rsUIContext, protocol, curve);
        if ((rectToAce != lastOriRect) || (reason != window->lastSizeChangeReason_)) {
            window->NotifySizeChange(rectToAce, reason, rsTransaction);
            window->lastSizeChangeReason_ = reason;
        }
        window->UpdateViewportConfig(rectToAce, display, reason, rsTransaction);
        RSNode::CloseImplicitAnimation(rsUIContext);
        if (rsTransaction) {
            rsTransaction->Commit();
        }
        window->postTaskDone_ = true;
    };
    ResSchedReport::GetInstance().RequestPerfIfNeed(reason, GetType(), GetWindowMode());
    if (reason == WindowSizeChangeReason::ROTATION) {
        postTaskDone_ = false;
        handler_->PostTask(task, "wms:UpdateRect");
    } else {
        if ((rectToAce != lastOriRect) || (reason != lastSizeChangeReason_) || !postTaskDone_) {
            NotifySizeChange(rectToAce, reason, rsTransaction);
            lastSizeChangeReason_ = reason;
            postTaskDone_ = true;
        }
        UpdateViewportConfig(rectToAce, display, reason, rsTransaction);
    }
}

void WindowImpl::UpdateMode(WindowMode mode)
{
    WLOGI("UpdateMode %{public}u", mode);
    property_->SetWindowMode(mode);
    UpdateTitleButtonVisibility();
    UpdateDecorEnable(true);
}

void WindowImpl::UpdateWindowModeSupportType(uint32_t windowModeSupportType)
{
    WLOGFD("windowModeSupportType: %{public}u, winId: %{public}u", windowModeSupportType, GetWindowId());
    SetWindowModeSupportType(windowModeSupportType);
    UpdateTitleButtonVisibility();
}

void WindowImpl::HandleBackKeyPressedEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    std::shared_ptr<IInputEventConsumer> inputEventConsumer;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        inputEventConsumer = inputEventConsumer_;
    }
    SingletonContainer::Get<WindowInfoReporter>().ReportBackButtonInfoImmediately();

    bool isConsumed = false;
    if (inputEventConsumer != nullptr) {
        WLOGD("Transfer back key event to inputEventConsumer");
        isConsumed = inputEventConsumer->OnInputEvent(keyEvent);
    } else if (uiContent_ != nullptr) {
        WLOGD("Transfer back key event to uiContent");
        isConsumed = uiContent_->ProcessBackPressed();
    } else {
        WLOGFE("There is no back key event consumer");
    }
    if (isConsumed) {
        WLOGD("Back key event is consumed");
        return;
    }
    PerformBack();
}

void WindowImpl::PerformBack()
{
    auto task = [weakThis = wptr(this)]() {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is null");
            return;
        }
        if (!WindowHelper::IsMainWindow(window->property_->GetWindowType())) {
            WLOGD("it is not a main window");
            return;
        }
        auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(window->context_);
        if (abilityContext == nullptr) {
            WLOGFE("abilityContext is null");
            return;
        }
        bool needMoveToBackground = false;
        int ret = abilityContext->OnBackPressedCallBack(needMoveToBackground);
        if (ret == ERR_OK && needMoveToBackground) {
            abilityContext->MoveAbilityToBackground();
            WLOGD("id: %{public}u closed, to move Ability: %{public}u",
                  window->property_->GetWindowId(), needMoveToBackground);
            return;
        }
        // TerminateAbility will invoke last ability, CloseAbility will not.
        bool shouldTerminateAbility = WindowHelper::IsFullScreenWindow(window->property_->GetWindowMode());
        if (shouldTerminateAbility) {
            abilityContext->TerminateSelf();
        } else {
            abilityContext->CloseAbility();
        }
        WLOGD("id: %{public}u closed, to kill Ability: %{public}u",
              window->property_->GetWindowId(), static_cast<uint32_t>(shouldTerminateAbility));
    };
    handler_->PostTask(task, "WindowImpl::PerformBack");
}

void WindowImpl::ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    int32_t keyCode = keyEvent->GetKeyCode();
    int32_t keyAction = keyEvent->GetKeyAction();
    WLOGFD("action: %{public}d", keyAction);
    bool shouldMarkProcess = true;
    if (keyCode == MMI::KeyEvent::KEYCODE_BACK && keyAction == MMI::KeyEvent::KEY_ACTION_UP) {
        HandleBackKeyPressedEvent(keyEvent);
    } else {
        std::shared_ptr<IInputEventConsumer> inputEventConsumer;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            inputEventConsumer = inputEventConsumer_;
        }
        if (inputEventConsumer != nullptr) {
            WLOGD("Transfer key event to inputEventConsumer");
            (void)inputEventConsumer->OnInputEvent(keyEvent);
            shouldMarkProcess = false;
        } else if (uiContent_ != nullptr) {
            WLOGD("Transfer key event to uiContent");
            bool handled = static_cast<bool>(uiContent_->ProcessKeyEvent(keyEvent));
            if (!handled && keyCode == MMI::KeyEvent::KEYCODE_ESCAPE &&
                GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN &&
                property_->GetMaximizeMode() == MaximizeMode::MODE_FULL_FILL &&
                keyAction == MMI::KeyEvent::KEY_ACTION_DOWN && !escKeyEventTriggered_) {
                WLOGI("recover from fullscreen cause KEYCODE_ESCAPE");
                Recover();
            }
            if (keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_ESCAPE) {
                escKeyEventTriggered_ = (keyAction == MMI::KeyEvent::KEY_ACTION_UP) ? false : true;
            }
            shouldMarkProcess = !handled;
        } else {
            WLOGFE("There is no key event consumer");
        }
    }
    if (GetType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
        WLOGFI("DispatchKeyEvent: %{public}u", GetWindowId());
        SingletonContainer::Get<WindowAdapter>().DispatchKeyEvent(GetWindowId(), keyEvent);
        keyEvent->MarkProcessed();
        return;
    }
    if (shouldMarkProcess) {
        keyEvent->MarkProcessed();
    }
}

void WindowImpl::HandleModeChangeHotZones(int32_t posX, int32_t posY)
{
    if (!WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) {
        return;
    }

    ModeChangeHotZones hotZones;
    auto res = SingletonContainer::Get<WindowAdapter>().GetModeChangeHotZones(property_->GetDisplayId(), hotZones);
    WLOGD("[HotZone] Window %{public}u, Pointer[%{public}d, %{public}d]", GetWindowId(), posX, posY);
    if (res == WMError::WM_OK) {
        WLOGD("[HotZone] Fullscreen [%{public}d, %{public}d, %{public}u, %{public}u]", hotZones.fullscreen_.posX_,
            hotZones.fullscreen_.posY_, hotZones.fullscreen_.width_, hotZones.fullscreen_.height_);
        WLOGD("[HotZone] Primary [%{public}d, %{public}d, %{public}u, %{public}u]", hotZones.primary_.posX_,
            hotZones.primary_.posY_, hotZones.primary_.width_, hotZones.primary_.height_);
        WLOGD("[HotZone] Secondary [%{public}d, %{public}d, %{public}u, %{public}u]", hotZones.secondary_.posX_,
            hotZones.secondary_.posY_, hotZones.secondary_.width_, hotZones.secondary_.height_);

        if (WindowHelper::IsPointInTargetRectWithBound(posX, posY, hotZones.fullscreen_)) {
            SetFullScreen(true);
        } else if (WindowHelper::IsPointInTargetRectWithBound(posX, posY, hotZones.primary_)) {
            SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
        } else if (WindowHelper::IsPointInTargetRectWithBound(posX, posY, hotZones.secondary_)) {
            SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
        }
    }
}

void WindowImpl::UpdatePointerEventForStretchableWindow(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        WLOGFW("Point item is invalid");
        return;
    }
    const Rect& originRect = property_->GetOriginRect();
    PointInfo originPos =
        WindowHelper::CalculateOriginPosition(originRect, GetRect(),
        { pointerItem.GetDisplayX(), pointerItem.GetDisplayY() });
    pointerItem.SetDisplayX(originPos.x);
    pointerItem.SetDisplayY(originPos.y);
    pointerItem.SetWindowX(originPos.x - originRect.posX_);
    pointerItem.SetWindowY(originPos.y - originRect.posY_);
    pointerEvent->UpdatePointerItem(pointerEvent->GetPointerId(), pointerItem);
}

void WindowImpl::UpdateDragType(int32_t startPointPosX, int32_t startPointPosY)
{
    const auto& startRectExceptCorner = moveDragProperty_->startRectExceptCorner_;
    if (startPointPosX > startRectExceptCorner.posX_ &&
        (startPointPosX < startRectExceptCorner.posX_ +
        static_cast<int32_t>(startRectExceptCorner.width_))) {
        moveDragProperty_->dragType_ = DragType::DRAG_BOTTOM_OR_TOP;
    } else if (startPointPosY > startRectExceptCorner.posY_ &&
        (startPointPosY < startRectExceptCorner.posY_ +
        static_cast<int32_t>(startRectExceptCorner.height_))) {
        moveDragProperty_->dragType_ = DragType::DRAG_LEFT_OR_RIGHT;
    } else if ((startPointPosX <= startRectExceptCorner.posX_ && startPointPosY <= startRectExceptCorner.posY_) ||
        (startPointPosX >= startRectExceptCorner.posX_ + static_cast<int32_t>(startRectExceptCorner.width_) &&
         startPointPosY >= startRectExceptCorner.posY_ + static_cast<int32_t>(startRectExceptCorner.height_))) {
        moveDragProperty_->dragType_ = DragType::DRAG_LEFT_TOP_CORNER;
    } else {
        moveDragProperty_->dragType_ = DragType::DRAG_RIGHT_TOP_CORNER;
    }
}

void WindowImpl::CalculateStartRectExceptHotZone(float vpr)
{
    TransformHelper::Vector2 hotZoneScale(1, 1);
    if (property_->isNeedComputerTransform()) {
        property_->ComputeTransform();
        hotZoneScale = WindowHelper::CalculateHotZoneScale(property_->GetTransformMat());
    }

    const auto& startPointRect = GetRect();
    auto& startRectExceptFrame = moveDragProperty_->startRectExceptFrame_;
    startRectExceptFrame.posX_ = startPointRect.posX_ +
        static_cast<int32_t>(WINDOW_FRAME_WIDTH * vpr / hotZoneScale.x_);
    startRectExceptFrame.posY_ = startPointRect.posY_ +
        static_cast<int32_t>(WINDOW_FRAME_WIDTH * vpr / hotZoneScale.y_);
    startRectExceptFrame.width_ = startPointRect.width_ -
        static_cast<uint32_t>((WINDOW_FRAME_WIDTH + WINDOW_FRAME_WIDTH) * vpr / hotZoneScale.x_);
    startRectExceptFrame.height_ = startPointRect.height_ -
        static_cast<uint32_t>((WINDOW_FRAME_WIDTH + WINDOW_FRAME_WIDTH) * vpr / hotZoneScale.y_);

    auto& startRectExceptCorner =  moveDragProperty_->startRectExceptCorner_;
    startRectExceptCorner.posX_ = startPointRect.posX_ +
        static_cast<int32_t>(WINDOW_FRAME_CORNER_WIDTH * vpr / hotZoneScale.x_);
    startRectExceptCorner.posY_ = startPointRect.posY_ +
        static_cast<int32_t>(WINDOW_FRAME_CORNER_WIDTH * vpr / hotZoneScale.y_);
    startRectExceptCorner.width_ = startPointRect.width_ -
        static_cast<uint32_t>((WINDOW_FRAME_CORNER_WIDTH + WINDOW_FRAME_CORNER_WIDTH) * vpr / hotZoneScale.x_);
    startRectExceptCorner.height_ = startPointRect.height_ -
        static_cast<uint32_t>((WINDOW_FRAME_CORNER_WIDTH + WINDOW_FRAME_CORNER_WIDTH) * vpr / hotZoneScale.y_);
}

bool WindowImpl::IsPointInDragHotZone(int32_t startPointPosX, int32_t startPointPosY, int32_t sourceType)
{
    // calculate rect with hotzone
    Rect rectWithHotzone;
    rectWithHotzone.posX_ = GetRect().posX_ - static_cast<int32_t>(HOTZONE_POINTER);
    rectWithHotzone.posY_ = GetRect().posY_ - static_cast<int32_t>(HOTZONE_POINTER);
    rectWithHotzone.width_ = GetRect().width_ + HOTZONE_POINTER * 2;   // 2: calculate width need
    rectWithHotzone.height_ = GetRect().height_ + HOTZONE_POINTER * 2; // 2: calculate height need

    if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
        !WindowHelper::IsPointInTargetRectWithBound(startPointPosX, startPointPosY, rectWithHotzone)) {
        return false;
    } else if ((!WindowHelper::IsPointInTargetRect(startPointPosX,
        startPointPosY, moveDragProperty_->startRectExceptFrame_)) ||
        (!WindowHelper::IsPointInWindowExceptCorner(startPointPosX,
        startPointPosY, moveDragProperty_->startRectExceptCorner_))) {
        return true;
    }
    return false;
}

void WindowImpl::StartMove()
{
    if (!WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) {
        WLOGE("[StartMove] current window can not be moved, windowId %{public}u", GetWindowId());
        return;
    }
    if (!moveDragProperty_->pointEventStarted_ || moveDragProperty_->startDragFlag_) {
        WLOGE("[StartMove] pointerEvent has not been started, or is dragging now");
        return;
    }
    moveDragProperty_->startMoveFlag_ = true;
    SingletonContainer::Get<WindowAdapter>().NotifyServerReadyToMoveOrDrag(property_->GetWindowId(),
        property_, moveDragProperty_);
    WLOGI("[StartMove] windowId %{public}u", GetWindowId());
}

void WindowImpl::ResetMoveOrDragState()
{
    if (!WindowHelper::IsMainWindow(GetType())) {
        return;
    }
    moveDragProperty_->pointEventStarted_ = false;
    moveDragProperty_->startDragFlag_ = false;
    moveDragProperty_->startMoveFlag_ = false;
    UpdateRect(GetRect(), property_->GetDecoStatus(), WindowSizeChangeReason::DRAG_END);
}

void WindowImpl::ReadyToMoveOrDragWindow(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const MMI::PointerEvent::PointerItem& pointerItem)
{
    if (moveDragProperty_->pointEventStarted_) {
        return;
    }

    moveDragProperty_->startPointRect_ = GetRect();
    moveDragProperty_->startPointPosX_ = pointerItem.GetDisplayX();
    moveDragProperty_->startPointPosY_ = pointerItem.GetDisplayY();
    moveDragProperty_->startPointerId_ = pointerEvent->GetPointerId();
    moveDragProperty_->targetDisplayId_ = pointerEvent->GetTargetDisplayId();
    moveDragProperty_->sourceType_ = pointerEvent->GetSourceType();
    moveDragProperty_->pointEventStarted_ = true;

    // calculate window inner rect except frame
    auto display = SingletonContainer::IsDestroyed() ? nullptr :
        SingletonContainer::Get<DisplayManager>().GetDisplayById(moveDragProperty_->targetDisplayId_);
    if (display == nullptr) {
        WLOGFE("get display failed moveDragProperty targetDisplayId:%{public}u, window id:%{public}u",
            moveDragProperty_->targetDisplayId_, property_->GetWindowId());
        return;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        WLOGFE("get display info failed moveDragProperty targetDisplayId:%{public}u, window id:%{public}u",
            moveDragProperty_->targetDisplayId_, property_->GetWindowId());
        return;
    }
    float vpr = display->GetVirtualPixelRatio();
    int32_t startPointPosX = moveDragProperty_->startPointPosX_ + displayInfo->GetOffsetX();
    int32_t startPointPosY = moveDragProperty_->startPointPosY_ + displayInfo->GetOffsetY();

    CalculateStartRectExceptHotZone(vpr);

    if (GetType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        moveDragProperty_->startMoveFlag_ = true;
        SingletonContainer::Get<WindowAdapter>().NotifyServerReadyToMoveOrDrag(property_->GetWindowId(),
            property_, moveDragProperty_);
    } else if (IsPointInDragHotZone(startPointPosX, startPointPosY, moveDragProperty_->sourceType_)
        && property_->GetMaximizeMode() != MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        moveDragProperty_->startDragFlag_ = true;
        UpdateDragType(startPointPosX, startPointPosY);
        SingletonContainer::Get<WindowAdapter>().NotifyServerReadyToMoveOrDrag(property_->GetWindowId(),
            property_, moveDragProperty_);
    }
    return;
}

void WindowImpl::EndMoveOrDragWindow(int32_t posX, int32_t posY, int32_t pointId, int32_t sourceType)
{
    if (pointId != moveDragProperty_->startPointerId_ || sourceType != moveDragProperty_->sourceType_) {
        return;
    }

    if (moveDragProperty_->startDragFlag_) {
        SingletonContainer::Get<WindowAdapter>().ProcessPointUp(GetWindowId());
        moveDragProperty_->startDragFlag_ = false;
    }

    if (moveDragProperty_->startMoveFlag_) {
        SingletonContainer::Get<WindowAdapter>().ProcessPointUp(GetWindowId());
        moveDragProperty_->startMoveFlag_ = false;
        HandleModeChangeHotZones(posX, posY);
    }
    moveDragProperty_->pointEventStarted_ = false;
    ResSchedReport::GetInstance().StopPerfIfNeed();
}

void WindowImpl::ConsumeMoveOrDragEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t pointId = pointerEvent->GetPointerId();
    int32_t sourceType = pointerEvent->GetSourceType();
    if (!pointerEvent->GetPointerItem(pointId, pointerItem) ||
        (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
        pointerEvent->GetButtonId() != MMI::PointerEvent::MOUSE_BUTTON_LEFT)) {
        WLOGFW("invalid pointerEvent");
        return;
    }
    int32_t pointDisplayX = pointerItem.GetDisplayX();
    int32_t pointDisplayY = pointerItem.GetDisplayY();
    int32_t action = pointerEvent->GetPointerAction();
    int32_t targetDisplayId = pointerEvent->GetTargetDisplayId();
    switch (action) {
        // Ready to move or drag
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN: {
            const auto& rect = GetRect();
            ReadyToMoveOrDragWindow(pointerEvent, pointerItem);
            if (IsPointerEventConsumed()) {
                ResSchedReport::GetInstance().TrigClick();
            }
            TLOGD(WmsLogTag::WMS_EVENT, "windowId:%{public}u, pointId:%{public}d, sourceType:%{public}d, "
                  "hasPointStarted:%{public}d, startMove:%{public}d, startDrag:%{public}d, targetDisplayId:"
                  "%{public}d, pointPos:[%{private}d, %{private}d], winRect:[%{public}d, %{public}d, %{public}u, "
                  "%{public}u]", GetWindowId(), pointId, sourceType, moveDragProperty_->pointEventStarted_,
                  moveDragProperty_->startMoveFlag_, moveDragProperty_->startDragFlag_, targetDisplayId,
                  pointDisplayX, pointDisplayY, rect.posX_, rect.posY_, rect.width_, rect.height_);
            break;
        }
        // End move or drag
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL: {
            EndMoveOrDragWindow(pointDisplayX, pointDisplayY, pointId, sourceType);
            WLOGFD("[Client Point Up/Cancel]: windowId: %{public}u, action: %{public}d, sourceType: %{public}d, "
                "startMove: %{public}d, startDrag: %{public}d", GetWindowId(), action, sourceType,
                moveDragProperty_->startMoveFlag_, moveDragProperty_->startDragFlag_);
            break;
        }
        default:
            break;
    }
}

bool WindowImpl::IsPointerEventConsumed()
{
    return moveDragProperty_->startDragFlag_ || moveDragProperty_->startMoveFlag_;
}

void WindowImpl::TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        WLOGFE("The pointer event is nullptr");
        return;
    }
    if (windowSystemConfig_.isStretchable_ && GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING) {
        UpdatePointerEventForStretchableWindow(pointerEvent);
    }
    std::shared_ptr<IInputEventConsumer> inputEventConsumer;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        inputEventConsumer = inputEventConsumer_;
    }
    if (inputEventConsumer != nullptr) {
        WLOGFD("Transfer pointer event to inputEventConsumer");
        if (!(inputEventConsumer->OnInputEvent(pointerEvent))) {
            WLOGFI("The Input event consumer consumes pointer event failed.");
            pointerEvent->MarkProcessed();
        }
    } else if (uiContent_ != nullptr) {
        WLOGFD("Transfer pointer event to uiContent");
        if (!(uiContent_->ProcessPointerEvent(pointerEvent))) {
            WLOGFI("The UI content consumes pointer event failed.");
            pointerEvent->MarkProcessed();
        }
    } else {
        WLOGFW("pointerEvent is not consumed, windowId: %{public}u", GetWindowId());
        pointerEvent->MarkProcessed();
    }
}

uint32_t WindowImpl::CalculatePointerDirection(int32_t pointerX, int32_t pointerY)
{
    UpdateDragType(pointerX, pointerY);
    return STYLEID_MAP.at(moveDragProperty_->dragType_);
}

void WindowImpl::HandlePointerStyle(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        WLOGFE("Get pointeritem failed");
        pointerEvent->MarkProcessed();
        return;
    }
    auto action = pointerEvent->GetPointerAction();
    uint32_t windowId = static_cast<uint32_t>(pointerEvent->GetAgentWindowId());
    int32_t mousePointX = pointerItem.GetDisplayX();
    int32_t mousePointY = pointerItem.GetDisplayY();
    int32_t sourceType = pointerEvent->GetSourceType();
    uint32_t oldStyleID = mouseStyleID_;
    uint32_t newStyleID = 0;
    if (WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode())) {
        auto display = SingletonContainer::IsDestroyed() ? nullptr :
            SingletonContainer::Get<DisplayManager>().GetDisplayById(pointerEvent->GetTargetDisplayId());
        if (display == nullptr || display->GetDisplayInfo() == nullptr) {
            WLOGFE("get display failed displayId:%{public}" PRIu64", window id:%{public}u",
                property_->GetDisplayId(), property_->GetWindowId());
            return;
        }
        float vpr = display->GetVirtualPixelRatio();
        CalculateStartRectExceptHotZone(vpr);
        if (IsPointInDragHotZone(mousePointX, mousePointY, sourceType) &&
            property_->GetMaximizeMode() != MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
            newStyleID = CalculatePointerDirection(mousePointX, mousePointY);
        } else if (action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
            newStyleID = MMI::MOUSE_ICON::DEFAULT;
        }
    } else if (GetType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        newStyleID = (GetRect().width_ > GetRect().height_) ?
            MMI::MOUSE_ICON::NORTH_SOUTH : MMI::MOUSE_ICON::WEST_EAST;
        if (action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
            newStyleID = MMI::MOUSE_ICON::DEFAULT; // when receive up event, set default style
        }
    }
    TLOGD(WmsLogTag::WMS_EVENT, "winId:%{public}u, Mouse posX:%{private}u, posY:%{private}u, action:%{public}u, "
           "winRect posX:%{public}u, posY:%{public}u, W:%{public}u, H:%{public}u, "
           "newStyle:%{public}u, oldStyle:%{public}u",
           windowId, mousePointX, mousePointY, action, GetRect().posX_,
           GetRect().posY_, GetRect().width_, GetRect().height_, newStyleID, oldStyleID);
    if (oldStyleID != newStyleID) {
        MMI::PointerStyle pointerStyle;
        pointerStyle.id = static_cast<int32_t>(newStyleID);
        int32_t res = MMI::InputManager::GetInstance()->SetPointerStyle(windowId, pointerStyle);
        if (res != 0) {
            WLOGFE("set pointer style failed, res is %{public}u", res);
            return;
        }
        mouseStyleID_ = newStyleID;
    }
}

void WindowImpl::PerfLauncherHotAreaIfNeed(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_CANCEL) {
        return;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t pointId = pointerEvent->GetPointerId();
    if (!pointerEvent->GetPointerItem(pointId, pointerItem)) {
        WLOGFW("invalid pointerEvent");
        return;
    }
    auto display = SingletonContainer::IsDestroyed() ? nullptr :
        SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        return;
    }
    auto displayHeight = display->GetHeight();
    constexpr float HOT_RATE = 0.07;
    auto height = static_cast<int32_t>(displayHeight * HOT_RATE);
    int32_t pointDisplayY = pointerItem.GetDisplayY();
    if (pointDisplayY > displayHeight - height) {
        ResSchedReport::GetInstance().AnimationBoost();
    }
#endif
}

void WindowImpl::ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    // If windowRect transformed, transform event back to its origin position
    if (property_) {
        property_->UpdatePointerEvent(pointerEvent);
    }
    int32_t action = pointerEvent->GetPointerAction();
    if (action == MMI::PointerEvent::POINTER_ACTION_MOVE || action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        ResSchedReport::GetInstance().TrigSlide(GetType(), true);
    }
    if (action == MMI::PointerEvent::POINTER_ACTION_UP || action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP ||
        action == MMI::PointerEvent::POINTER_ACTION_CANCEL) {
        ResSchedReport::GetInstance().TrigSlide(GetType(), false);
    }
    if ((action == MMI::PointerEvent::POINTER_ACTION_MOVE || action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) &&
        pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        HandlePointerStyle(pointerEvent);
    }
    PerfLauncherHotAreaIfNeed(pointerEvent);
    if (action == MMI::PointerEvent::POINTER_ACTION_DOWN || action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        WLOGFD("WMS process point down, id:%{public}u, action: %{public}d", GetWindowId(), action);
        if (GetType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT) {
            MMI::PointerEvent::PointerItem pointerItem;
            if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
                WLOGFW("Point item is invalid");
                pointerEvent->MarkProcessed();
                return;
            }
            if (!WindowHelper::IsPointInTargetRect(pointerItem.GetDisplayX(), pointerItem.GetDisplayY(), GetRect())) {
                NotifyAfterUnfocused(false);
                pointerEvent->MarkProcessed();
                return;
            }
        }
        if (property_ != nullptr) {
            SingletonContainer::Get<WindowAdapter>().ProcessPointDown(property_->GetWindowId());
        }
    }

    // If point event type is up, should reset start move flag
    if (WindowHelper::IsMainFloatingWindow(GetType(), GetWindowMode()) ||
        GetType() == WindowType::WINDOW_TYPE_DOCK_SLICE || (action == MMI::PointerEvent::POINTER_ACTION_UP ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP || action == MMI::PointerEvent::POINTER_ACTION_CANCEL)) {
        ConsumeMoveOrDragEvent(pointerEvent);
    }

    if (IsPointerEventConsumed()) {
        pointerEvent->MarkProcessed();
        return;
    }

    TransferPointerEvent(pointerEvent);
}

void WindowImpl::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    if (vsyncStation_ != nullptr) {
        vsyncStation_->RequestVsync(vsyncCallback);
    }
}

int64_t WindowImpl::GetVSyncPeriod()
{
    if (vsyncStation_ != nullptr) {
        return vsyncStation_->GetVSyncPeriod();
    }
    return 0;
}

void WindowImpl::UpdateFocusStatus(bool focused)
{
    if (!IsWindowValid()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Window is invalid");
        return;
    }

    WLOGFD("IsFocused: %{public}d, id: %{public}u", focused, property_->GetWindowId());
    isFocused_ = focused;
    if (focused) {
        HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "FOCUS_WINDOW",
            OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "PID", getpid(),
            "UID", getuid(),
            "BUNDLE_NAME", property_->GetAbilityInfo().bundleName_,
            "WINDOW_TYPE", static_cast<uint32_t>(GetType()));
        if (state_ <= WindowState::STATE_CREATED || state_ == WindowState::STATE_HIDDEN) {
            needNotifyFocusLater_ = true;
            return;
        }
        NotifyAfterFocused();
    } else {
        NotifyAfterUnfocused();
    }
}

bool WindowImpl::IsFocused() const
{
    if (!IsWindowValid()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Window is invalid");
        return false;
    }

    return isFocused_;
}

void WindowImpl::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (uiContent_ != nullptr) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace impl win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent_->UpdateConfiguration(configuration);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, impl win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, subWinSize=%{public}u, display=%{public}" PRIu64,
        GetWindowId(), static_cast<uint32_t>(subWindowMap_.size()), GetDisplayId());
    if (subWindowMap_.count(GetWindowId()) == 0) {
        return;
    }
    for (auto& subWindow : subWindowMap_.at(GetWindowId())) {
        if (subWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "impl sub window is null");
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, display=%{public}" PRIu64,
            subWindow->GetWindowId(), subWindow->GetDisplayId());
        subWindow->UpdateConfiguration(configuration);
    }
}

void WindowImpl::UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (uiContent_ != nullptr) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace impl win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent_->UpdateConfiguration(configuration, resourceManager);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, impl win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, subWinSize=%{public}u, display=%{public}" PRIu64,
        GetWindowId(), static_cast<uint32_t>(subWindowMap_.size()), GetDisplayId());
    if (subWindowMap_.count(GetWindowId()) == 0) {
        return;
    }
    for (auto& subWindow : subWindowMap_.at(GetWindowId())) {
        if (subWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "impl sub window is null");
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, display=%{public}" PRIu64,
            subWindow->GetWindowId(), subWindow->GetDisplayId());
        subWindow->UpdateConfigurationForSpecified(configuration, resourceManager);
    }
}

void WindowImpl::UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (uiContent_ != nullptr) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace impl win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent_->UpdateConfigurationSyncForAll(configuration);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, impl win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, subWinSize=%{public}u, display=%{public}" PRIu64,
        GetWindowId(), static_cast<uint32_t>(subWindowMap_.size()), GetDisplayId());
    if (subWindowMap_.count(GetWindowId()) == 0) {
        return;
    }
    for (auto& subWindow : subWindowMap_.at(GetWindowId())) {
        if (subWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "impl sub window is null");
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, display=%{public}" PRIu64,
            subWindow->GetWindowId(), subWindow->GetDisplayId());
        subWindow->UpdateConfigurationSync(configuration);
    }
}

void WindowImpl::UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl map size: %{public}u", static_cast<uint32_t>(windowMap_.size()));
    for (const auto& winPair : windowMap_) {
        if (auto window = winPair.second.second) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "impl win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
            window->UpdateConfigurationSync(configuration);
        }
    }
}

void WindowImpl::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u type %{public}d area %{public}s",
          property_->GetWindowId(), type, avoidArea->ToString().c_str());
    auto display = SingletonContainer::IsDestroyed() ? nullptr :
        SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    UpdateViewportConfig(GetRect(), display, WindowSizeChangeReason::AVOID_AREA_CHANGE, nullptr, {{type, *avoidArea}});
    NotifyAvoidAreaChange(avoidArea, type);
}

void WindowImpl::UpdateViewportConfig(const Rect& rect, const sptr<Display>& display, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (uiContent_ == nullptr) {
        return;
    }
    Ace::ViewportConfig config;
    config.SetSize(rect.width_, rect.height_);
    config.SetPosition(rect.posX_, rect.posY_);
    config.SetDisplayId(GetDisplayId());
    if (display) {
        float virtualPixelRatio = display->GetVirtualPixelRatio();
        virtualPixelRatio_.store(virtualPixelRatio);
        config.SetDensity(virtualPixelRatio);
        auto displayInfo = display->GetDisplayInfo();
        if (displayInfo != nullptr) {
            config.SetOrientation(static_cast<int32_t>(displayInfo->GetDisplayOrientation()));
        }
    }
    uiContent_->UpdateViewportConfig(config, reason, rsTransaction, avoidAreas, occupiedAreaInfo_);
    WLOGFD("Id:%{public}u, windowRect:[%{public}d, %{public}d, %{public}u, %{public}u]",
        property_->GetWindowId(), rect.posX_, rect.posY_, rect.width_, rect.height_);
}

void WindowImpl::UpdateDecorEnable(bool needNotify)
{
    WLOGFD("Start");
    if (WindowHelper::IsMainWindow(property_->GetWindowType())) {
        bool enable = windowSystemConfig_.isSystemDecorEnable_ &&
            WindowHelper::IsWindowModeSupported(windowSystemConfig_.decorWindowModeSupportType_, GetWindowMode());
        WLOGFD("Decor enable: %{public}d", static_cast<int32_t>(enable));
        property_->SetDecorEnable(enable);
    } else {
        property_->SetDecorEnable(false);
    }
    if (needNotify) {
        if (uiContent_ != nullptr) {
            uiContent_->UpdateWindowMode(GetWindowMode(), property_->GetDecorEnable());
            WLOGFD("Notify uiContent window mode change end");
        }
        NotifyModeChange(GetWindowMode(), property_->GetDecorEnable());
    }
}

void WindowImpl::UpdateWindowStateUnfrozen()
{
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
    if (abilityContext != nullptr && windowTag_ == WindowTag::MAIN_WINDOW) {
        WLOGFD("DoAbilityForeground KEYGUARD, id: %{public}u", GetWindowId());
        AAFwk::AbilityManagerClient::GetInstance()->DoAbilityForeground(abilityContext->GetToken(),
            static_cast<uint32_t>(WindowStateChangeReason::KEYGUARD));
    } else if (state_ != WindowState::STATE_SHOWN) {
        state_ = WindowState::STATE_SHOWN;
        NotifyAfterForeground();
    }
}

void WindowImpl::UpdateWindowState(WindowState state)
{
    WLOGFI("id: %{public}u, State to set:%{public}u", GetWindowId(), state);
    if (!IsWindowValid()) {
        return;
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
    switch (state) {
        case WindowState::STATE_FROZEN: {
            if (abilityContext != nullptr && windowTag_ == WindowTag::MAIN_WINDOW) {
                WLOGFD("DoAbilityBackground KEYGUARD, id: %{public}u", GetWindowId());
                AAFwk::AbilityManagerClient::GetInstance()->DoAbilityBackground(abilityContext->GetToken(),
                    static_cast<uint32_t>(WindowStateChangeReason::KEYGUARD));
            } else {
                state_ = WindowState::STATE_FROZEN;
                NotifyAfterBackground(false, true);
            }
            break;
        }
        case WindowState::STATE_UNFROZEN: {
            UpdateWindowStateUnfrozen();
            break;
        }
        case WindowState::STATE_SHOWN: {
            if (abilityContext != nullptr && windowTag_ == WindowTag::MAIN_WINDOW) {
                WLOGFD("WindowState::STATE_SHOWN, id: %{public}u", GetWindowId());
                AAFwk::AbilityManagerClient::GetInstance()->DoAbilityForeground(abilityContext->GetToken(),
                    static_cast<uint32_t>(WindowStateChangeReason::TOGGLING));
            } else {
                state_ = WindowState::STATE_SHOWN;
                NotifyAfterForeground();
            }
            break;
        }
        case WindowState::STATE_HIDDEN: {
            if (abilityContext != nullptr && windowTag_ == WindowTag::MAIN_WINDOW &&
                state_ == WindowState::STATE_SHOWN) {
                WLOGFD("WindowState: STATE_SHOWN, id: %{public}u", GetWindowId());
                AAFwk::AbilityManagerClient::GetInstance()->DoAbilityBackground(abilityContext->GetToken(),
                    static_cast<uint32_t>(WindowStateChangeReason::NORMAL));
            } else {
                Hide(static_cast<uint32_t>(WindowStateChangeReason::NORMAL), false);
            }
            break;
        }
        default: {
            WLOGFE("windowState to set is invalid");
            break;
        }
    }
}

WmErrorCode WindowImpl::UpdateWindowStateWhenShow()
{
    state_ = WindowState::STATE_SHOWN;
    if (WindowHelper::IsMainWindow(property_->GetWindowType()) ||
        WindowHelper::IsSystemMainWindow(property_->GetWindowType())) {
        // update subwindow subWindowState_ and notify subwindow shown or not
        UpdateSubWindowStateAndNotify(GetWindowId());
        NotifyAfterForeground();
    } else if (GetType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
        subWindowState_ = WindowState::STATE_SHOWN;
        NotifyAfterForeground();
    } else {
        uint32_t parentId = property_->GetParentId();
        sptr<Window> parentWindow = FindWindowById(parentId);
        if (parentWindow == nullptr) {
            WLOGE("parent window is null");
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
        if (parentWindow->GetWindowState() == WindowState::STATE_HIDDEN) {
            // not notify user shown and update subwindowState_
            subWindowState_ = WindowState::STATE_HIDDEN;
        } else if (parentWindow->GetWindowState() == WindowState::STATE_SHOWN) {
            NotifyAfterForeground();
            subWindowState_ = WindowState::STATE_SHOWN;
        }
    }
    if (needNotifyFocusLater_ && isFocused_) {
        UpdateFocusStatus(true);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode WindowImpl::UpdateWindowStateWhenHide()
{
    state_ = WindowState::STATE_HIDDEN;
    if (WindowHelper::IsSystemMainWindow(property_->GetWindowType()) ||
        WindowHelper::IsMainWindow(property_->GetWindowType())) {
        // main window need to update subwindow subWindowState_ and notify subwindow shown or not
        UpdateSubWindowStateAndNotify(GetWindowId());
        NotifyAfterBackground();
    } else if (GetType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
        subWindowState_ = WindowState::STATE_HIDDEN;
        NotifyAfterBackground();
    } else {
        uint32_t parentId = property_->GetParentId();
        sptr<Window> parentWindow = FindWindowById(parentId);
        if (parentWindow == nullptr) {
            WLOGE("parent window is null");
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
        if (subWindowState_ == WindowState::STATE_SHOWN) {
            NotifyAfterBackground();
        }
        subWindowState_ = WindowState::STATE_HIDDEN;
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode WindowImpl::UpdateSubWindowStateAndNotify(uint32_t parentId)
{
    if (subWindowMap_.find(parentId) == subWindowMap_.end()) {
        WLOGFD("main window: %{public}u has no child node", parentId);
        return WmErrorCode::WM_OK;
    }
    std::vector<sptr<WindowImpl>> subWindows = subWindowMap_[parentId];
    if (subWindows.empty()) {
        WLOGFD("main window: %{public}u, its subWindowMap is empty", parentId);
        return WmErrorCode::WM_OK;
    }
    // when main window hide and subwindow whose state is shown should hide and notify user
    if (state_ == WindowState::STATE_HIDDEN) {
        for (auto subwindow : subWindows) {
            if (subwindow->GetWindowState() == WindowState::STATE_SHOWN &&
                subwindow->subWindowState_ == WindowState::STATE_SHOWN) {
                subwindow->NotifyAfterBackground();
            }
            subwindow->subWindowState_ = WindowState::STATE_HIDDEN;
        }
    // when main window show and subwindow whose state is shown should show and notify user
    } else if (state_ == WindowState::STATE_SHOWN) {
        for (auto subwindow : subWindows) {
            if (subwindow->GetWindowState() == WindowState::STATE_SHOWN &&
                subwindow->subWindowState_ == WindowState::STATE_HIDDEN) {
                subwindow->NotifyAfterForeground();
                subwindow->subWindowState_ = WindowState::STATE_SHOWN;
            } else {
                subwindow->subWindowState_ = WindowState::STATE_HIDDEN;
            }
        }
    }
    return WmErrorCode::WM_OK;
}

sptr<WindowProperty> WindowImpl::GetWindowProperty()
{
    return property_;
}

void WindowImpl::RestoreSplitWindowMode(uint32_t mode)
{
    if (!IsWindowValid()) {
        return;
    }
    auto windowMode = static_cast<WindowMode>(mode);
    if (windowMode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || windowMode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        UpdateMode(windowMode);
    }
}

void WindowImpl::UpdateDragEvent(const PointInfo& point, DragEvent event)
{
    NotifyDragEvent(point, event);
}

void WindowImpl::NotifyDragEvent(const PointInfo& point, DragEvent event)
{
    auto windowDragListeners = GetListeners<IWindowDragListener>();
    Rect rect = GetRect();
    for (auto& listener : windowDragListeners) {
        if (listener != nullptr) {
            listener->OnDrag(point.x - rect.posX_, point.y - rect.posY_, event);
        }
    }
}

void WindowImpl::UpdateDisplayId(DisplayId from, DisplayId to)
{
    WLOGFD("update displayId. win %{public}u", GetWindowId());
    NotifyDisplayMoveChange(from, to);
    property_->SetDisplayId(to);
}

void WindowImpl::UpdateOccupiedAreaChangeInfo(const sptr<OccupiedAreaChangeInfo>& info,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    WLOGFD("Update OccupiedArea, id: %{public}u", property_->GetWindowId());
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        occupiedAreaInfo_ = info;
        WLOGFI("occupiedAreaeInfo changed, rect:[%{public}u, %{public}u, %{public}u, %{public}u]",
            info->rect_.posX_, info->rect_.posY_, info->rect_.width_, info->rect_.height_);
    }
    auto display = SingletonContainer::IsDestroyed() ? nullptr :
        SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    UpdateViewportConfig(GetRect(), display, WindowSizeChangeReason::OCCUPIED_AREA_CHANGE, nullptr, avoidAreas);
    NotifyOccupiedAreaChange(info, rsTransaction);
}

void WindowImpl::UpdateActiveStatus(bool isActive)
{
    WLOGFD("window active status: %{public}d, id: %{public}u", isActive, property_->GetWindowId());
    if (isActive) {
        NotifyAfterActive();
    } else {
        NotifyAfterInactive();
    }
}

void WindowImpl::NotifyScreenshot()
{
    auto screenshotListeners = GetListeners<IScreenshotListener>();
    for (auto& screenshotListener : screenshotListeners) {
        if (screenshotListener != nullptr) {
            screenshotListener->OnScreenshot();
        }
    }
}

WMError WindowImpl::NotifyScreenshotAppEvent(ScreenshotEventType type)
{
    auto screenshotAppEventListeners = GetListeners<IScreenshotAppEventListener>();
    for (auto& screenshotAppEventListener : screenshotAppEventListeners) {
        if (screenshotAppEventListener != nullptr) {
            screenshotAppEventListener->OnScreenshotAppEvent(type);
        }
    }
    return WMError::WM_OK;
}

void WindowImpl::NotifyTouchOutside()
{
    auto touchOutsideListeners = GetListeners<ITouchOutsideListener>();
    for (auto& touchOutsideListener : touchOutsideListeners) {
        if (touchOutsideListener != nullptr) {
            touchOutsideListener->OnTouchOutside();
        }
    }
}

void WindowImpl::NotifyTouchDialogTarget(int32_t posX, int32_t posY)
{
    SingletonContainer::Get<WindowAdapter>().ProcessPointDown(property_->GetWindowId());
    auto dialogTargetTouchListeners = GetListeners<IDialogTargetTouchListener>();
    for (auto& dialogTargetTouchListener : dialogTargetTouchListeners) {
        if (dialogTargetTouchListener != nullptr) {
            dialogTargetTouchListener->OnDialogTargetTouch();
        }
    }
}

void WindowImpl::NotifyDestroy()
{
    auto dialogDeathRecipientListener = GetListener<IDialogDeathRecipientListener>();
    if (dialogDeathRecipientListener != nullptr) {
        dialogDeathRecipientListener->OnDialogDeathRecipient();
    }
}

void WindowImpl::NotifyForeground()
{
    NotifyAfterForeground();
}

void WindowImpl::NotifyBackground()
{
    NotifyAfterBackground();
}

void WindowImpl::NotifyForegroundInteractiveStatus(bool interactive)
{
    WLOGFI("NotifyForegroundInteractiveStatus %{public}d", interactive);
    if (!IsWindowValid() || state_ != WindowState::STATE_SHOWN) {
        return;
    }
    if (interactive) {
        NotifyAfterResumed();
    } else {
        NotifyAfterPaused();
    }
}

void WindowImpl::NotifyMMIServiceOnline(uint32_t winId)
{
    TLOGI(WmsLogTag::WMS_EVENT, "Id:%{public}u", winId);
    ResetInputWindow(winId);
}

void WindowImpl::TransformSurfaceNode(const Transform& trans)
{
    if (surfaceNode_ == nullptr) {
        return;
    }
    surfaceNode_->SetPivotX(trans.pivotX_);
    surfaceNode_->SetPivotY(trans.pivotY_);
    surfaceNode_->SetScaleX(trans.scaleX_);
    surfaceNode_->SetScaleY(trans.scaleY_);
    surfaceNode_->SetTranslateX(trans.translateX_);
    surfaceNode_->SetTranslateY(trans.translateY_);
    surfaceNode_->SetTranslateZ(trans.translateZ_);
    surfaceNode_->SetRotationX(trans.rotationX_);
    surfaceNode_->SetRotationY(trans.rotationY_);
    surfaceNode_->SetRotation(trans.rotationZ_);
}

void WindowImpl::UpdateZoomTransform(const Transform& trans, bool isDisplayZoomOn)
{
    WLOGFD("%{public}s zoomTrans, pivotX:%{public}f, pivotY:%{public}f, scaleX:%{public}f, scaleY:%{public}f"
        ", transX:%{public}f, transY:%{public}f, transZ:%{public}f, rotateX:%{public}f, rotateY:%{public}f "
        "rotateZ:%{public}f", property_->GetWindowName().c_str(), trans.pivotX_, trans.pivotY_, trans.scaleX_,
        trans.scaleY_, trans.translateX_, trans.translateY_, trans.translateZ_, trans.rotationX_,
        trans.rotationY_, trans.rotationZ_);
    property_->SetZoomTransform(trans);
    property_->SetDisplayZoomState(isDisplayZoomOn);
}

void WindowImpl::ClearListenersById(uint32_t winId)
{
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    ClearUselessListeners(screenshotListeners_, winId);
    ClearUselessListeners(touchOutsideListeners_, winId);
    ClearUselessListeners(dialogTargetTouchListeners_, winId);
    ClearUselessListeners(lifecycleListeners_, winId);
    ClearUselessListeners(windowChangeListeners_, winId);
    ClearUselessListeners(avoidAreaChangeListeners_, winId);
    ClearUselessListeners(occupiedAreaChangeListeners_, winId);
    ClearUselessListeners(dialogDeathRecipientListener_, winId);
}

void WindowImpl::NotifyAfterForeground(bool needNotifyListeners, bool needNotifyUiContent)
{
    if (needNotifyListeners) {
        auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterForeground, lifecycleListeners);
    }
    if (needNotifyUiContent) {
        CALL_UI_CONTENT(Foreground);
    }
    if (!intentParam_.empty()) {
        auto uiContent = GetUIContent();
        if (uiContent != nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "SetIntentParam, isColdStart:%{public}u", isIntentColdStart_);
            uiContent->SetIntentParam(intentParam_, std::move(loadPageCallback_), isIntentColdStart_);
            intentParam_ = "";
        } else {
            TLOGE(WmsLogTag::WMS_LIFE, "uiContent is nullptr.");
        }
    }
}

void WindowImpl::NotifyAfterDidForeground(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d", reason);
    if (reason != static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL)) {
        TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d no need notify did foreground", reason);
        return;
    }
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "handler is nullptr");
        return;
    }
    const char* const where = __func__;
    handler_->PostTask([weak = wptr(this), where] {
        auto window = weak.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s window is nullptr", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s execute", where);
        auto lifecycleListeners = window->GetListeners<IWindowLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterDidForeground, lifecycleListeners);
    }, where, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void WindowImpl::NotifyAfterBackground(bool needNotifyListeners, bool needNotifyUiContent)
{
    if (needNotifyListeners) {
        auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterBackground, lifecycleListeners);
    }
    if (needNotifyUiContent) {
        CALL_UI_CONTENT(Background);
    }
}

void WindowImpl::NotifyAfterDidBackground(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d", reason);
    if (reason != static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL)) {
        TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d no need notify did background", reason);
        return;
    }
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "handler is nullptr");
        return;
    }
    const char* const where = __func__;
    handler_->PostTask([weak = wptr(this), where] {
        auto window = weak.promote();
        if (window == nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s window is nullptr", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s execute", where);
        auto lifecycleListeners = window->GetListeners<IWindowLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterDidBackground, lifecycleListeners);
    }, where, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void WindowImpl::NotifyAfterFocused()
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterFocused, lifecycleListeners);
    CALL_UI_CONTENT(Focus);
}

void WindowImpl::NotifyAfterUnfocused(bool needNotifyUiContent)
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    // use needNotifyUinContent to separate ui content callbacks
    CALL_LIFECYCLE_LISTENER(AfterUnfocused, lifecycleListeners);
    if (needNotifyUiContent) {
        CALL_UI_CONTENT(UnFocus);
    }
}

void WindowImpl::NotifyAfterResumed()
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterResumed, lifecycleListeners);
}

void WindowImpl::NotifyAfterPaused()
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterPaused, lifecycleListeners);
}

void WindowImpl::NotifyBeforeDestroy(std::string windowName)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (uiContent_ != nullptr) {
        auto uiContent = std::move(uiContent_);
        uiContent_ = nullptr;
        uiContent->Destroy();
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "uiContent is Destroy, win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    if (notifyNativefunc_) {
        notifyNativefunc_(windowName);
    }
}

void WindowImpl::NotifyBeforeSubWindowDestroy(sptr<WindowImpl> window)
{
    auto uiContent = window->GetUIContent();
    if (uiContent != nullptr) {
        uiContent->Destroy();
    }
    if (window->GetNativeDestroyCallback()) {
        window->GetNativeDestroyCallback()(window->GetWindowName());
    }
}

void WindowImpl::NotifyAfterActive()
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterActive, lifecycleListeners);
}

void WindowImpl::NotifyAfterInactive()
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterInactive, lifecycleListeners);
}

void WindowImpl::NotifyForegroundFailed(WMError ret)
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER_WITH_PARAM(ForegroundFailed, lifecycleListeners, static_cast<int32_t>(ret));
}

void WindowImpl::NotifyBackgroundFailed(WMError ret)
{
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER_WITH_PARAM(BackgroundFailed, lifecycleListeners, static_cast<int32_t>(ret));
}

bool WindowImpl::IsStretchableReason(WindowSizeChangeReason reason)
{
    return reason == WindowSizeChangeReason::DRAG || reason == WindowSizeChangeReason::DRAG_END ||
           reason == WindowSizeChangeReason::DRAG_START || reason == WindowSizeChangeReason::RECOVER ||
           IsMoveToOrDragMove(reason) || reason == WindowSizeChangeReason::UNDEFINED;
}

void WindowImpl::NotifySizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    auto windowChangeListeners = GetListeners<IWindowChangeListener>();
    for (auto& listener : windowChangeListeners) {
        if (listener != nullptr) {
            listener->OnSizeChange(rect, reason, rsTransaction);
        }
    }
}

void WindowImpl::NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    auto avoidAreaChangeListeners = GetListeners<IAvoidAreaChangedListener>();
    for (auto& listener : avoidAreaChangeListeners) {
        if (listener != nullptr) {
            listener->OnAvoidAreaChanged(*avoidArea, type);
        }
    }
}

void WindowImpl::NotifyDisplayMoveChange(DisplayId from, DisplayId to)
{
    auto displayMoveListeners = GetListeners<IDisplayMoveListener>();
    for (auto& listener : displayMoveListeners) {
        if (listener != nullptr) {
            listener->OnDisplayMove(from, to);
        }
    }
}

void WindowImpl::NotifyModeChange(WindowMode mode, bool hasDeco)
{
    auto windowChangeListeners = GetListeners<IWindowChangeListener>();
    for (auto& listener : windowChangeListeners) {
        if (listener != nullptr) {
            listener->OnModeChange(mode, hasDeco);
        }
    }
}

void WindowImpl::NotifyOccupiedAreaChange(const sptr<OccupiedAreaChangeInfo>& info,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    auto occupiedAreaChangeListeners = GetListeners<IOccupiedAreaChangeListener>();
    for (auto& listener : occupiedAreaChangeListeners) {
        if (listener != nullptr) {
            listener->OnSizeChange(info, rsTransaction);
        }
    }
}

void WindowImpl::SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel)
{
    needRemoveWindowInputChannel_ = needRemoveWindowInputChannel;
}

Rect WindowImpl::GetSystemAlarmWindowDefaultSize(Rect defaultRect)
{
    auto display = SingletonContainer::IsDestroyed() ? nullptr :
        SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        WLOGFE("get display failed displayId:%{public}" PRIu64", window id:%{public}u", property_->GetDisplayId(),
            property_->GetWindowId());
        return defaultRect;
    }
    uint32_t width = static_cast<uint32_t>(display->GetWidth());
    uint32_t height = static_cast<uint32_t>(display->GetHeight());
    WLOGFD("width:%{public}u, height:%{public}u, displayId:%{public}" PRIu64"",
        width, height, property_->GetDisplayId());
    uint32_t alarmWidth = static_cast<uint32_t>((static_cast<float>(width) *
        SYSTEM_ALARM_WINDOW_WIDTH_RATIO));
    uint32_t alarmHeight = static_cast<uint32_t>((static_cast<float>(height) *
        SYSTEM_ALARM_WINDOW_HEIGHT_RATIO));

    Rect rect = { static_cast<int32_t>((width - alarmWidth) / 2), static_cast<int32_t>((height - alarmHeight) / 2),
        alarmWidth, alarmHeight }; // divided by 2 to middle the window
    return rect;
}

void WindowImpl::SetDefaultOption()
{
    switch (property_->GetWindowType()) {
        case WindowType::WINDOW_TYPE_STATUS_BAR:
        case WindowType::WINDOW_TYPE_NAVIGATION_BAR:
        case WindowType::WINDOW_TYPE_VOLUME_OVERLAY:
        case WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT:
        case WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW: {
            property_->SetRequestRect(GetSystemAlarmWindowDefaultSize(property_->GetRequestRect()));
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            break;
        }
        case WindowType::WINDOW_TYPE_KEYGUARD: {
            RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
            break;
        }
        case WindowType::WINDOW_TYPE_DRAGGING_EFFECT: {
            property_->SetWindowFlags(0);
            break;
        }
        case WindowType::WINDOW_TYPE_APP_COMPONENT: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::NONE));
            break;
        }
        case WindowType::WINDOW_TYPE_TOAST:
        case WindowType::WINDOW_TYPE_FLOAT:
        case WindowType::WINDOW_TYPE_SYSTEM_FLOAT:
        case WindowType::WINDOW_TYPE_FLOAT_CAMERA:
        case WindowType::WINDOW_TYPE_VOICE_INTERACTION:
        case WindowType::WINDOW_TYPE_LAUNCHER_DOCK:
        case WindowType::WINDOW_TYPE_SEARCHING_BAR:
        case WindowType::WINDOW_TYPE_SCREENSHOT:
        case WindowType::WINDOW_TYPE_GLOBAL_SEARCH:
        case WindowType::WINDOW_TYPE_DIALOG: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            break;
        }
        case WindowType::WINDOW_TYPE_BOOT_ANIMATION:
        case WindowType::WINDOW_TYPE_POINTER: {
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_DOCK_SLICE: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_SYSTEM_TOAST: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetTouchable(false);
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_SCREEN_CONTROL: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetTouchable(false);
            property_->SetFocusable(false);
            SetAlpha(0);
            break;
        }
        default:
            break;
    }
}

bool WindowImpl::IsWindowValid() const
{
    bool res = ((state_ > WindowState::STATE_INITIAL) && (state_ < WindowState::STATE_BOTTOM));
    if (!res) {
        WLOGW("already destroyed or not created! id: %{public}u", GetWindowId());
    }
    return res;
}

bool WindowImpl::IsLayoutFullScreen() const
{
    if (!IsWindowValid()) {
        return false;
    }
    auto mode = GetWindowMode();
    return (mode == WindowMode::WINDOW_MODE_FULLSCREEN && isIgnoreSafeArea_);
}

bool WindowImpl::IsFullScreen() const
{
    if (!IsWindowValid()) {
        return false;
    }
    auto statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto naviProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    return (IsLayoutFullScreen() && !statusProperty.enable_ && !naviProperty.enable_);
}

void WindowImpl::SetRequestedOrientation(Orientation orientation, bool needAnimation)
{
    if (!IsWindowValid()) {
        TLOGE(WmsLogTag::DEFAULT, "window is invalid");
        return;
    }
    if (property_->GetRequestedOrientation() == orientation) {
        return;
    }
    property_->SetRequestedOrientation(orientation);
    if (state_ == WindowState::STATE_SHOWN) {
        UpdateProperty(PropertyChangeAction::ACTION_UPDATE_ORIENTATION);
    }
}

Orientation WindowImpl::GetRequestedOrientation()
{
    if (!IsWindowValid()) {
        TLOGE(WmsLogTag::DEFAULT, "window is invalid");
        return Orientation::UNSPECIFIED;
    }
    return property_->GetRequestedOrientation();
}

WMError WindowImpl::SetTouchHotAreas(const std::vector<Rect>& rects)
{
    std::vector<Rect> lastTouchHotAreas;
    property_->GetTouchHotAreas(lastTouchHotAreas);

    property_->SetTouchHotAreas(rects);
    WMError result = UpdateProperty(PropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    if (result != WMError::WM_OK) {
        property_->SetTouchHotAreas(lastTouchHotAreas);
    }
    return result;
}

void WindowImpl::GetRequestedTouchHotAreas(std::vector<Rect>& rects) const
{
    property_->GetTouchHotAreas(rects);
}

WMError WindowImpl::SetAPPWindowLabel(const std::string& label)
{
    if (uiContent_ == nullptr) {
        WLOGFE("uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent_->SetAppWindowTitle(label);
    WLOGI("Set app window label success, label : %{public}s", label.c_str());
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
    WLOGI("Set app window icon success");
    return WMError::WM_OK;
}

bool WindowImpl::CheckCameraFloatingWindowMultiCreated(WindowType type)
{
    if (type != WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        return false;
    }

    {
        std::shared_lock<std::shared_mutex> lock(windowMapMutex_);
        for (auto& winPair : windowMap_) {
            if (winPair.second.second->GetType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
                return true;
            }
        }
    }
    uint32_t accessTokenId = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
    property_->SetAccessTokenId(accessTokenId);
    TLOGI(WmsLogTag::DEFAULT, "Create camera float window, TokenId=%{private}u", accessTokenId);
    return false;
}

WMError WindowImpl::SetCornerRadius(float cornerRadius)
{
    WLOGI("Window %{public}s set corner radius %{public}f", name_.c_str(), cornerRadius);
    surfaceNode_->SetCornerRadius(cornerRadius);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowRadius(float radius)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set shadow radius permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set shadow radius %{public}f", name_.c_str(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    surfaceNode_->SetShadowRadius(radius);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowColor(std::string color)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set shadow color permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set shadow color %{public}s", name_.c_str(), color.c_str());
    uint32_t colorValue;
    if (!ColorParser::Parse(color, colorValue)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    surfaceNode_->SetShadowColor(colorValue);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowOffsetX(float offsetX)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set shadow offset x permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set shadow offsetX %{public}f", name_.c_str(), offsetX);
    surfaceNode_->SetShadowOffsetX(offsetX);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetShadowOffsetY(float offsetY)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set shadow offset y permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set shadow offsetY %{public}f", name_.c_str(), offsetY);
    surfaceNode_->SetShadowOffsetY(offsetY);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetBlur(float radius)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set blur permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set blur radius %{public}f", name_.c_str(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    radius = ConvertRadiusToSigma(radius);
    WLOGFI("[Client] Window %{public}s set blur radius after conversion %{public}f", name_.c_str(), radius);
    surfaceNode_->SetFilter(RSFilter::CreateBlurFilter(radius, radius));
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetBackdropBlur(float radius)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set backdrop blur permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set backdrop blur radius %{public}f", name_.c_str(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    radius = ConvertRadiusToSigma(radius);
    WLOGFI("[Client] Window %{public}s set backdrop blur radius after conversion %{public}f", name_.c_str(), radius);
    surfaceNode_->SetBackgroundFilter(RSFilter::CreateBlurFilter(radius, radius));
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::SetBackdropBlurStyle(WindowBlurStyle blurStyle)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set backdrop blur style permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    WLOGI("Window %{public}s set backdrop blur style %{public}u", name_.c_str(), blurStyle);
    if (blurStyle < WindowBlurStyle::WINDOW_BLUR_OFF || blurStyle > WindowBlurStyle::WINDOW_BLUR_THICK) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (blurStyle == WindowBlurStyle::WINDOW_BLUR_OFF) {
        surfaceNode_->SetBackgroundFilter(nullptr);
    } else {
        auto display = SingletonContainer::IsDestroyed() ? nullptr :
            SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
        if (display == nullptr) {
            WLOGFE("get display failed displayId:%{public}" PRIu64", window id:%{public}u", property_->GetDisplayId(),
                property_->GetWindowId());
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        surfaceNode_->SetBackgroundFilter(RSFilter::CreateMaterialFilter(static_cast<int>(blurStyle),
                                                                         display->GetVirtualPixelRatio()));
    }
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowImpl::NotifyMemoryLevel(int32_t level)
{
    WLOGFD("id: %{public}u, notify memory level: %{public}d", property_->GetWindowId(), level);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (uiContent_ == nullptr) {
        WLOGFE("Window %{public}s notify memory level failed, ace is null.", name_.c_str());
        return WMError::WM_ERROR_NULLPTR;
    }
    // notify memory level
    uiContent_->NotifyMemoryLevel(level);
    return WMError::WM_OK;
}

bool WindowImpl::IsAllowHaveSystemSubWindow()
{
    auto windowType = property_->GetWindowType();
    if (WindowHelper::IsSystemSubWindow(windowType) ||
        WindowHelper::IsSubWindow(windowType) ||
        windowType == WindowType::WINDOW_TYPE_DIALOG) {
        WLOGI("type %{public}u not allowed to add subwindow", windowType);
        return false;
    }
    return true;
}

void WindowImpl::SetNeedDefaultAnimation(bool needDefaultAnimation)
{
    needDefaultAnimation_= needDefaultAnimation;
}

WMError WindowImpl::SetTextFieldAvoidInfo(double textFieldPositionY, double textFieldHeight)
{
    property_->SetTextFieldPositionY(textFieldPositionY);
    property_->SetTextFieldHeight(textFieldHeight);
    UpdateProperty(PropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO);
    return WMError::WM_OK;
}

void WindowImpl::RegisterWindowInspectorCallback()
{
    auto getWMSWindowListCallback = [weakThis = wptr(this)]() -> std::optional<WindowListInfo> {
        if (auto window = weakThis.promote()) {
            return std::make_optional<WindowListInfo>({
                window->GetWindowName(), window->GetWindowId(),
                static_cast<uint32_t>(window->GetType()), window->GetRect()
            });
        } else {
            return std::nullopt;
        }
    };
    WindowInspector::GetInstance().RegisterGetWMSWindowListCallback(GetWindowId(), std::move(getWMSWindowListCallback));
}

uint32_t WindowImpl::GetApiTargetVersion() const
{
    uint32_t version = 0;
    if ((context_ != nullptr) && (context_->GetApplicationInfo() != nullptr)) {
        version = static_cast<uint32_t>(context_->GetApplicationInfo()->apiTargetVersion) % API_VERSION_MOD;
    }
    return version;
}

WMError WindowImpl::GetWindowPropertyInfo(WindowPropertyInfo& windowPropertyInfo)
{
    if (!IsWindowValid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    windowPropertyInfo.windowRect = GetRect();
    auto uicontent = GetUIContent();
    if (uicontent == nullptr) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "uicontent is nullptr");
    } else {
        uicontent->GetWindowPaintSize(windowPropertyInfo.drawableRect);
    }
    windowPropertyInfo.type = GetType();
    windowPropertyInfo.isLayoutFullScreen = IsLayoutFullScreen();
    windowPropertyInfo.isFullScreen = IsFullScreen();
    windowPropertyInfo.isTouchable = GetTouchable();
    windowPropertyInfo.isFocusable = GetFocusable();
    windowPropertyInfo.name = GetWindowName();
    windowPropertyInfo.isPrivacyMode = IsPrivacyMode();
    windowPropertyInfo.isKeepScreenOn = IsKeepScreenOn();
    windowPropertyInfo.brightness = GetBrightness();
    windowPropertyInfo.isTransparent = IsTransparent();
    windowPropertyInfo.id = GetWindowId();
    windowPropertyInfo.displayId = GetDisplayId();
    return WMError::WM_OK;
}

std::shared_ptr<RSUIDirector> WindowImpl::GetRSUIDirector() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, windowId: %{public}u",
          RSAdapterUtil::RSUIDirectorToStr(rsUIDirector_).c_str(), GetWindowId());
    return rsUIDirector_;
}

std::shared_ptr<RSUIContext> WindowImpl::GetRSUIContext() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    auto rsUIContext = rsUIDirector_ ? rsUIDirector_->GetRSUIContext() : nullptr;
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, windowId: %{public}u",
          RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str(), GetWindowId());
    return rsUIContext;
}

WMError WindowImpl::SetIntentParam(const std::string& intentParam,
    const std::function<void()>& loadPageCallback, bool isColdStart)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    intentParam_ = intentParam;
    loadPageCallback_ = loadPageCallback;
    isIntentColdStart_ = isColdStart;
    return WMError::WM_OK;
}
} // namespace Rosen
} // namespace OHOS
