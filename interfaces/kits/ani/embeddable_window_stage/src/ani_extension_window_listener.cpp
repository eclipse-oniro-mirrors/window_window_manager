/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "ani_extension_window_listener.h"

#include <hitrace_meter.h>

#include "ani.h"
#include <ani_signature_builder.h>
#include "ani_window_utils.h"
#include "event_handler.h"
#include "event_runner.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
using namespace arkts::ani_signature;
namespace {
constexpr const char* ETS_WINDOW_SIZE_CHANGE_CB = "sizeChangeCallback";
constexpr const char* ETS_AVOID_AREA_CHANGE_CB = "avoidAreaChangeCallback";
constexpr const char* ETS_KEYBOARD_HEIGHT_CHANGE_CB = "keyboardHeightChangeCallback";
constexpr const char* ETS_WINDOW_RECT_CHANGE_CB = "windowRectChangeCallback";
}

AniExtensionWindowListener::~AniExtensionWindowListener()
{
    if (aniCallback_ != nullptr) {
        env_->GlobalReference_Delete(aniCallback_);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]~AniExtensionWindowListener");
}

void AniExtensionWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void AniExtensionWindowListener::CallSizeChangeCallback(ani_object size)
{
    ani_status ret {};
    ani_function fn {};
    ani_namespace ns {};
    if ((ret = env_->FindNamespace(ETS_UIEXTENSION_HOST_NAMESPACE_DESCRIPTOR, &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find namespace failed, ret: %{public}u", ret);
        return;
    }
    if ((ret = env_->Namespace_FindFunction(ns, ETS_WINDOW_SIZE_CHANGE_CB, nullptr, &fn)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find function failed, ret: %{public}u", ret);
        return;
    }
    if ((ret = env_->Function_Call_Void(fn, aniCallback_, size)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Call function failed, ret: %{public}u", ret);
        return;
    }
}

void AniExtensionWindowListener::OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]OnSizeChange, [%{public}u, %{public}u], reason=%{public}u",
        rect.width_, rect.height_, reason);
    if (currentWidth_ == rect.width_ && currentHeight_ == rect.height_ && reason != WindowSizeChangeReason::DRAG_END) {
        TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]No need to notify size change");
        return;
    }
    auto onSizeChangeTask = [self = weakRef_, rect, vm = vm_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniExtensionWindowListener::OnSizeChange");
        auto thisListener = self.promote();
        auto aniVm = AniVm(vm);
        auto eng = aniVm.GetAniEnv();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]thisListener, eng or callback is nullptr");
            return;
        }
        thisListener->CallSizeChangeCallback(AniWindowUtils::CreateAniSize(eng, rect.width_, rect.height_));
    };
    if (reason == WindowSizeChangeReason::ROTATION) {
        onSizeChangeTask();
    } else {
        if (!eventHandler_) {
            TLOGE(WmsLogTag::WMS_UIEXT, "Get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(onSizeChangeTask, "wms:AniExtensionWindowListener::SizeChangeCallback", 0,
                                AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
    currentWidth_ = rect.width_;
    currentHeight_ = rect.height_;
}

void AniExtensionWindowListener::OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
    const sptr<OccupiedAreaChangeInfo>& info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]");
    auto task = [self = weakRef_, vm = vm_, avoidArea, type] {
        auto thisListener = self.promote();
        auto aniVm = AniVm(vm);
        auto eng = aniVm.GetAniEnv();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]thisListener, eng or callback is nullptr");
            return;
        }
        auto nativeAvoidArea = AniWindowUtils::CreateAniAvoidArea(eng, avoidArea, type);
        AniWindowUtils::CallAniFunctionVoid(eng, ETS_UIEXTENSION_NAMESPACE_DESCRIPTOR, ETS_AVOID_AREA_CHANGE_CB,
            nullptr, thisListener->aniCallback_, nativeAvoidArea, static_cast<ani_int>(type));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniExtensionWindowListener::AvoidAreaChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniExtensionWindowListener::OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_UIEXT,
        "[ANI]OccupiedAreaChangeInfo, type: %{public}u, input rect:[%{public}d, %{public}d, %{public}u, %{public}u]",
        static_cast<uint32_t>(info->type_), info->rect_.posX_, info->rect_.posY_, info->rect_.width_,
        info->rect_.height_);
    auto onSizeChangeTask = [self = weakRef_, info, vm = vm_] () {
        auto thisListener = self.promote();
        auto aniVm = AniVm(vm);
        auto eng = aniVm.GetAniEnv();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]thisListener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, ETS_UIEXTENSION_HOST_NAMESPACE_DESCRIPTOR,
            ETS_KEYBOARD_HEIGHT_CHANGE_CB, nullptr, thisListener->aniCallback_,
            static_cast<ani_int>(info->rect_.height_));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(onSizeChangeTask, "wms:AniExtensionWindowListener::SizeChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniExtensionWindowListener::OnRectChange(Rect rect, WindowSizeChangeReason reason)
{
    if (currRect_ == rect) {
        TLOGD(WmsLogTag::WMS_UIEXT, "Skip redundant rect update");
        return;
    }
    // js callback should run in js thread
    const char* const where = __func__;
    auto onRectChangeTask = [self = weakRef_, rect, vm = vm_, where] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsExtensionWindowListener::OnRectChange");
        auto thisListener = self.promote();
        auto aniVm = AniVm(vm);
        auto env = aniVm.GetAniEnv();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s This listener or env is nullptr", where);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, ETS_UIEXTENSION_NAMESPACE_DESCRIPTOR,
            ETS_WINDOW_RECT_CHANGE_CB, nullptr, thisListener->aniCallback_,
            AniWindowUtils::CreateAniRect(env, rect), ComponentRectChangeReason::HOST_WINDOW_RECT_CHANGE);
    };
    eventHandler_->PostTask(onRectChangeTask, "wms:AniExtensionWindowListener::RectChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
    currRect_ = rect;
}

} // namespace Rosen
} // namespace OHOS
