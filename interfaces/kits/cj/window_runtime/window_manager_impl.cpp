/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "window_manager_impl.h"
#include "ability.h"
#include "ability_context.h"
#include "accesstoken_kit.h"
#include "bundle_constants.h"
#include "display_manager.h"
#include "ipc_skeleton.h"
#include "singleton_container.h"
#include "window_helper.h"
#include "window_impl.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "window_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

bool CheckCallingPermission(std::string permission)
{
    TLOGD(WmsLogTag::WMS_SUB, "Permission: %{public}s", permission.c_str());
    if (!permission.empty() &&
        Security::AccessToken::AccessTokenKit::VerifyAccessToken(IPCSkeleton::GetCallingTokenID(), permission)
        != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TLOGE(WmsLogTag::WMS_SUB, "Permission %{public}s is not granted", permission.c_str());
        return false;
    }
    return true;
}

static int32_t CreateNewSystemWindow(OHOS::AbilityRuntime::Context* ctx,
    sptr<WindowOption> windowOption, int64_t* windowId)
{
    if (windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "New window option failed");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    if (ctx == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Context is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY);
    }
    auto sctx = ctx->shared_from_this();
    auto context = std::weak_ptr<AbilityRuntime::Context>(sctx);
    if (windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT ||
        windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        auto abilityContext = Context::ConvertTo<AbilityRuntime::AbilityContext>(context.lock());
        if (abilityContext != nullptr) {
            if (!CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
                TLOGE(WmsLogTag::WMS_SUB, "TYPE_FLOAT CheckCallingPermission failed");
                return static_cast<int32_t>(WmErrorCode::WM_ERROR_NO_PERMISSION);
            }
        }
    }
    WMError wmError = WMError::WM_OK;
    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption, context.lock(), wmError);
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    if (window != nullptr && wmErrorCode == WmErrorCode::WM_OK) {
        auto instance = CreateCjWindowObject(window);
        if (!instance) {
            return static_cast<int32_t>(WMError::WM_ERROR_NO_MEM);
        }
        
        *windowId = instance->GetID();
    }
    return static_cast<int32_t>(wmErrorCode);
}

int32_t WindowManagerImpl::CreateWindow(WindowParameters window)
{
    WindowOption option;
    option.SetWindowName(window.name);
    if (window.winType >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
        window.winType < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
        option.SetWindowType(CJ_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(window.winType)));
    } else {
        option.SetWindowType(static_cast<WindowType>(window.winType));
    }
    if (window.displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(window.displayId)) == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to parse config");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    option.SetDisplayId(window.displayId);
    option.SetParentId(window.parentId);
    sptr<WindowOption> windowOption = new(std::nothrow) WindowOption(option);
    if (WindowHelper::IsSystemWindow(option.GetWindowType())) {
        TLOGI(WmsLogTag::WMS_SUB, "CreateNewSystemWindow");
        return CreateNewSystemWindow(window.context, windowOption, window.windowId);
    }
    return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
}

int32_t WindowManagerImpl::SetWindowLayoutMode(uint32_t mode)
{
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().SetWindowLayoutMode(WindowLayoutMode(mode)));
    return static_cast<int32_t>(ret);
}

int32_t WindowManagerImpl::MinimizeAll(int64_t displayId, int32_t excludeWindowId)
{
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().MinimizeAllAppWindows(
            static_cast<uint64_t>(displayId), static_cast<int32_t>(excludeWindowId)));
    return static_cast<int32_t>(ret);
}

int32_t WindowManagerImpl::FindWindow(std::string name, int64_t& windowId)
{
    sptr<CJWindowImpl> windowImpl = FindCjWindowObject(name);
    if (windowImpl != nullptr) {
        windowId = windowImpl->GetID();
        return WINDOW_SUCCESS;
    } else {
        sptr<Window> window = Window::Find(name);
        if (window == nullptr) {
            return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        } else {
            sptr<CJWindowImpl> newWindow = CreateCjWindowObject(window);
            if (newWindow == nullptr) {
                return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            }
            windowId = newWindow->GetID();
            return WINDOW_SUCCESS;
        }
    }
}

/** @note @window.hierarchy */
int32_t WindowManagerImpl::GetLastWindow(OHOS::AbilityRuntime::Context* ctx, int64_t& id)
{
    sptr<Window> window = nullptr;
    if (ctx == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[WindowManager]Stage mode without context");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto sctx = ctx->shared_from_this();
    auto context = std::weak_ptr<AbilityRuntime::Context>(sctx);
    if (sctx == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[WindowManager]Stage mode without context");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    window = Window::GetTopWindowWithContext(context.lock());
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "[WindowManager]Get top window failed");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return FindWindow(window->GetWindowName(), id);
}

int32_t WindowManagerImpl::ShiftAppWindowFocus(int32_t sourceWindowId,
                                               int32_t targetWindowId)
{
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<WindowManager>().ShiftAppWindowFocus(sourceWindowId, targetWindowId));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Shift application window focus failed");
    }
    return static_cast<int32_t>(ret);
}
}
}
