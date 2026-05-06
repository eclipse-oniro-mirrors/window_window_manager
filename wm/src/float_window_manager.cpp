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
#include "float_window_manager.h"

#include <random>
#include <chrono>

#include "window.h"
#include "window_manager_hilog.h"

#include "float_view_manager.h"
#include "floating_ball_manager.h"
#include "picture_in_picture_manager.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int64_t MAX_RANDOM = 999999;
}

std::recursive_mutex FloatWindowManager::relationMutex_;
std::map<sptr<FloatViewController>, sptr<FloatingBallController>> FloatWindowManager::floatViewToFloatingBallMap_ = {};

std::mutex FloatWindowManager::windowMutex_;
uint32_t FloatWindowManager::floatViewCnt_ = 0;
uint32_t FloatWindowManager::floatingBallCnt_ = 0;
uint32_t FloatWindowManager::pipCnt_ = 0;

void FloatWindowManager::ProcessBindFloatViewStateChange(const wptr<FloatViewController> &fvControllerWeak,
    const FvWindowState state)
{
    auto fvController = fvControllerWeak.promote();
    if (fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "fvController is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    auto it = floatViewToFloatingBallMap_.find(fvController);
    if (it == floatViewToFloatingBallMap_.end() || it->second == nullptr) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "can not find floating ball by float view or null floating ball controller");
        return;
    }
    auto fbController = it->second;

    if (state == FvWindowState::FV_STATE_STARTED) {
        fvController->SetShowWhenCreate(true);
        fbController->SetShowWhenCreate(false);
        return;
    }
    if (state == FvWindowState::FV_STATE_IN_FLOATING_BALL) {
        fvController->SetShowWhenCreate(false);
        fbController->SetShowWhenCreate(true);
    }
}

std::string FloatWindowManager::GetControllerId()
{
    auto time = std::to_string(std::chrono::time_point_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now()).time_since_epoch().count());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, MAX_RANDOM);
    auto randomNumber = distr(gen);
    std::stringstream ss;
    ss << time << "_" << randomNumber;
    return ss.str();
}

WMError FloatWindowManager::Bind(const sptr<FloatViewController> &fvController,
    const sptr<FloatingBallController> &fbController, const FbOption &fbOption)
{
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    if (!IsFloatingBallStateValid(fbController) || fbController->IsBind()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "floating ball state not supported or has been bound");
        return WMError::WM_ERROR_FB_INVALID_STATE;
    }
    if (!IsFloatViewStateValid(fvController) || fvController->IsBind()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "float view state not supported or has been bound");
        return WMError::WM_ERROR_FV_INVALID_STATE;
    }
    fbController->SetOption(sptr<FbOption>::MakeSptr(fbOption));
    fbController->SetBindState(true);
    fvController->SetBindState(true);
    AddRelation(fvController, fbController);
    return WMError::WM_OK;
}

bool FloatWindowManager::IsFloatingBallStateValid(const sptr<FloatingBallController> &fbController)
{
    auto fbState = fbController->GetCurState();
    return fbState == FbWindowState::STATE_UNDEFINED || fbState == FbWindowState::STATE_STOPPED;
}

bool FloatWindowManager::IsFloatViewStateValid(const sptr<FloatViewController> &fvController)
{
    auto fvState = fvController->GetCurState();
    return fvState == FvWindowState::FV_STATE_UNDEFINED || fvState == FvWindowState::FV_STATE_STOPPED ||
           fvState == FvWindowState::FV_STATE_ERROR;
}

WMError FloatWindowManager::UnBind(const sptr<FloatViewController> &fvController,
    const sptr<FloatingBallController> &fbController)
{
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    if (fbController == nullptr || !IsFloatingBallStateValid(fbController) || !fbController->IsBind()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "floating ball state not supported or has not been bound");
        return WMError::WM_ERROR_FB_INVALID_STATE;
    }
    if (fvController == nullptr || !IsFloatViewStateValid(fvController) || !fvController->IsBind()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "float view state not supported or has not been bound");
        return WMError::WM_ERROR_FV_INVALID_STATE;
    }

    auto it = floatViewToFloatingBallMap_.find(fvController);
    if (it == floatViewToFloatingBallMap_.end() || it->second == nullptr ||
        it->second.GetRefPtr() != fbController.GetRefPtr()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "fvController and fbController are not a bound pair");
        return WMError::WM_ERROR_FV_INVALID_STATE;
    }
    fbController->SetBindState(false);
    fvController->SetBindState(false);
    fbController->SetBindWindowId(INVALID_WINDOW_ID);
    fvController->SetBindWindowId(INVALID_WINDOW_ID);
    RemoveRelation(fvController, fbController);
    return WMError::WM_OK;
}

void FloatWindowManager::AddRelation(const sptr<FloatViewController> &fvController,
    const sptr<FloatingBallController> &fbController)
{
    floatViewToFloatingBallMap_[fvController] = fbController;
}

void FloatWindowManager::RemoveRelation(const sptr<FloatViewController> &fvController,
    const sptr<FloatingBallController> &fbController)
{
    auto it = floatViewToFloatingBallMap_.find(fvController);
    if (it != floatViewToFloatingBallMap_.end() && it->second != nullptr && fbController != nullptr &&
        it->second.GetRefPtr() == fbController.GetRefPtr()) {
        floatViewToFloatingBallMap_.erase(it);
    }
}

WMError FloatWindowManager::StartBindFloatView(const wptr<FloatViewController> &fvControllerWeak)
{
    auto fvController = fvControllerWeak.promote();
    if (fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "fvController is null");
        return WMError::WM_DO_NOTHING;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    auto it = floatViewToFloatingBallMap_.find(fvController);
    if (it == floatViewToFloatingBallMap_.end() || it->second == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "can not find floating ball by float view or null floating ball controller");
        return WMError::WM_DO_NOTHING;
    }
    auto fbController = it->second;

    auto ret = fvController->StartFloatViewSingle(true);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "start float view failed when bind");
        return ret;
    }
    auto fvWindow = fvController->GetWindow();
    if (fvWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "float view window is nullptr");
        fvController->StopFloatViewFromClientSingle();
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    fbController->SetBindWindowId(fvWindow->GetWindowId());
    ret = fbController->StartFloatingBallSingle(fbController->GetOption(), false);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "start floating ball failed when bind");
        fbController->SetBindWindowId(INVALID_WINDOW_ID);
        fvController->StopFloatViewFromClientSingle();
        return WMError::WM_DO_NOTHING;
    }
    auto fbWindow = fbController->GetFbWindow();
    if (fbWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "floating ball window is nullptr");
        fbController->SetBindWindowId(INVALID_WINDOW_ID);
        fvController->StopFloatViewFromClientSingle();
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    fvController->SetBindWindowId(fbWindow->GetWindowId());
    return WMError::WM_OK;
}

WMError FloatWindowManager::StartBindFloatingBall(const wptr<FloatingBallController> &fbControllerWeak,
    const sptr<FbOption> &option)
{
    auto fbController = fbControllerWeak.promote();
    if (fbController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "fbController is null");
        return WMError::WM_DO_NOTHING;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    auto fvController = FindFloatViewByFloatingBall(fbController);
    if (fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "can not find float view by floating ball or null float view controller");
        return WMError::WM_DO_NOTHING;
    }

    auto ret = fbController->StartFloatingBallSingle(option, true);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "start floating ball failed when bind");
        return ret;
    }
    auto fbWindow = fbController->GetFbWindow();
    if (fbWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "floating ball window is nullptr");
        fbController->StopFloatingBallFromClientSingle();
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    fvController->SetBindWindowId(fbWindow->GetWindowId());

    ret = fvController->StartFloatViewSingle(false);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "start float view failed when bind");
        fvController->SetBindWindowId(INVALID_WINDOW_ID);
        fbController->StopFloatingBallFromClientSingle();
        return WMError::WM_DO_NOTHING;
    }
    auto fvWindow = fvController->GetWindow();
    if (fvWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "float view window is nullptr");
        fvController->SetBindWindowId(INVALID_WINDOW_ID);
        fbController->StopFloatingBallFromClientSingle();
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    fbController->SetBindWindowId(fvWindow->GetWindowId());
    return WMError::WM_OK;
}

WMError FloatWindowManager::StopBindFloatView(const wptr<FloatViewController> &fvControllerWeak)
{
    auto fvController = fvControllerWeak.promote();
    if (fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "fvController is null");
        return WMError::WM_DO_NOTHING;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    auto it = floatViewToFloatingBallMap_.find(fvController);
    if (it == floatViewToFloatingBallMap_.end() || it->second == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "can not find floating ball by float view or null floating ball controller");
        return WMError::WM_DO_NOTHING;
    }
    auto fbController = it->second;
    auto ret = fvController->StopFloatViewFromClientSingle();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "stop float view failed when bind");
        return ret;
    }
    ret = fbController->StopFloatingBallFromClientSingle();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "stop floating ball failed when bind");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    return WMError::WM_OK;
}

WMError FloatWindowManager::StopBindFloatingBall(const wptr<FloatingBallController> &fbControllerWeak)
{
    auto fbController = fbControllerWeak.promote();
    if (fbController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "fbController is null");
        return WMError::WM_DO_NOTHING;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    auto fvController = FindFloatViewByFloatingBall(fbController);
    if (fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "can not find float view by floating ball or null float view controller");
        return WMError::WM_DO_NOTHING;
    }

    auto ret = fbController->StopFloatingBallFromClientSingle();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "stop floating ball failed when bind");
        return ret;
    }
    ret = fvController->StopFloatViewFromClientSingle();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "stop float view failed when bind");
        return WMError::WM_ERROR_FB_INTERNAL_ERROR;
    }
    return WMError::WM_OK;
}

sptr<Window> FloatWindowManager::CreateFbWindow(sptr<WindowOption> &windowOption,
    const FloatingBallTemplateBaseInfo &templateInfo, const std::shared_ptr<Media::PixelMap>& icon,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError &error,
    const wptr<FloatingBallController> &fbControllerWeak)
{
    std::lock_guard<std::mutex> lock(windowMutex_);
    if (floatViewCnt_ > 0) {
        auto fvController = GetBoundFloatView(fbControllerWeak.promote());
        if (fvController == nullptr || !FloatViewManager::IsActiveController(fvController)) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "there are active float view can not create flaotingb ball");
            error = WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS;
            return nullptr;
        }
    }
    sptr<Window> window = Window::CreateFb(windowOption, templateInfo, icon, context, error);
    if (window != nullptr) {
        floatingBallCnt_++;
    }
    return window;
}

sptr<Window> FloatWindowManager::CreateFvWindow(sptr<WindowOption> &windowOption,
    const FloatViewTemplateInfo &templateInfo, const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    WMError &error, const wptr<FloatViewController> &fvControllerWeak)
{
    std::lock_guard<std::mutex> lock(windowMutex_);
    if (pipCnt_ > 0) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "there are active pip, can not create float view");
        error = WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS;
        return nullptr;
    }
    if (floatingBallCnt_ > 0) {
        auto fbController = GetBoundFloatingBall(fvControllerWeak.promote());
        if (fbController == nullptr || !FloatingBallManager::IsActiveController(fbController)) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "there are active floating ball, can not create float view");
            error = WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS;
            return nullptr;
        }
    }
    sptr<Window> window = Window::CreateFv(windowOption, templateInfo, context, error);
    if (window != nullptr) {
        floatViewCnt_++;
    }
    return window;
}

sptr<Window> FloatWindowManager::CreatePipWindow(sptr<WindowOption> &windowOption,
    const PiPTemplateInfo &templateInfo, const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    WMError &error)
{
    std::lock_guard<std::mutex> lock(windowMutex_);
    if (floatViewCnt_ > 0) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "there are active float view, can not create pip");
        error = WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS;
        return nullptr;
    }
    sptr<Window> window = Window::CreatePiP(windowOption, templateInfo, context, error);
    if (window != nullptr) {
        pipCnt_++;
    }
    return window;
}

WMError FloatWindowManager::DestroyFloatWindow(const sptr<Window> &window)
{
    std::lock_guard<std::mutex> lock(windowMutex_);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "window is nullptr when destroy float window");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto type = window->GetType();
    WMError ret = window->Destroy();
    if (type == WindowType::WINDOW_TYPE_FV) {
        floatViewCnt_ = (floatViewCnt_ > 0) ? (floatViewCnt_ - 1) : 0;
    } else if (type == WindowType::WINDOW_TYPE_FB) {
        floatingBallCnt_ = (floatingBallCnt_ > 0) ? (floatingBallCnt_ - 1) : 0;
    } else if (type == WindowType::WINDOW_TYPE_PIP) {
        pipCnt_ = (pipCnt_ > 0) ? (pipCnt_ - 1) : 0;
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "DestroyFloatWindow, type: %{public}d, ret: %{public}d, floatViewCnt: %{public}u, "
        "floatingBallCnt: %{public}u, pipCnt: %{public}u", type, ret, floatViewCnt_, floatingBallCnt_, pipCnt_);
    return ret;
}

sptr<FloatingBallController> FloatWindowManager::GetBoundFloatingBall(const sptr<FloatViewController>& fvController)
{
    if (fvController == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    auto it = floatViewToFloatingBallMap_.find(fvController);
    if (it == floatViewToFloatingBallMap_.end()) {
        return nullptr;
    }
    return it->second;
}

sptr<FloatViewController> FloatWindowManager::GetBoundFloatView(const sptr<FloatingBallController>& fbController)
{
    if (fbController == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(relationMutex_);
    return FindFloatViewByFloatingBall(fbController);
}

sptr<FloatViewController> FloatWindowManager::FindFloatViewByFloatingBall(
    const sptr<FloatingBallController>& fbController)
{
    if (fbController == nullptr) {
        return nullptr;
    }
    for (const auto& [fv, fb] : floatViewToFloatingBallMap_) {
        if (fv == nullptr || fb == nullptr) {
            continue;
        }
        if (fb.GetRefPtr() == fbController.GetRefPtr()) {
            return fv;
        }
    }
    return nullptr;
}

} // namespace Rosen
} // namespace OHOS
