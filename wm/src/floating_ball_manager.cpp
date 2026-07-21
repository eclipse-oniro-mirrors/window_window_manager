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

#include "floating_ball_manager.h"

#include "parameters.h"
#include "floating_ball_controller.h"
#include "window_manager_hilog.h"
#include "window_scene_session_impl.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string ACTION_CLICK = "click";
const std::string ACTION_CLOSE = "close";

const std::map<std::string, std::function<void(const std::string&)>> FB_ACTION_MAP {
    {ACTION_CLICK, FloatingBallManager::DoActionClick},
    {ACTION_CLOSE, FloatingBallManager::DoActionClose},
};
}

bool FloatingBallManager::isSupportFloatingBall_ = false;
sptr<FloatingBallController> FloatingBallManager::activeController_ = nullptr;

bool FloatingBallManager::HasActiveController()
{
    return activeController_ != nullptr;
}

bool FloatingBallManager::IsActiveController(const wptr<FloatingBallController>& fbController)
{
    if (!HasActiveController()) {
        return false;
    }
    bool res = fbController.GetRefPtr() == activeController_.GetRefPtr();
    TLOGD(WmsLogTag::WMS_SYSTEM, "res: %{public}u", res);
    return res;
}

void FloatingBallManager::SetActiveController(const sptr<FloatingBallController>& fbController)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "in");
    activeController_ = fbController;
}

void FloatingBallManager::RemoveActiveController(const wptr<FloatingBallController>& fbController)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "in");
    if (HasActiveController() && fbController.GetRefPtr() == activeController_.GetRefPtr()) {
        activeController_ = nullptr;
    }
}

// LCOV_EXCL_START
void FloatingBallManager::DoActionClick(const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "click in, reason %{public}s", reason.c_str());
    if (auto controller = GetActiveController()) {
        controller->OnFloatingBallClick();
    }
}

void FloatingBallManager::DoActionClose(const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "close in, reason %{public}s", reason.c_str());
    if (auto controller = GetActiveController()) {
        controller->StopFloatingBall(reason);
    }
}

void FloatingBallManager::DoDestroy()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "in");
    if (auto controller = GetActiveController()) {
        controller->DestroyFloatingBallWindow("DoDestroy");
    }
}

void FloatingBallManager::DoFbActionEvent(const std::string& actionName, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "in, actionName %{public}s, reason %{public}s", actionName.c_str(), reason.c_str());
    auto func = FB_ACTION_MAP.find(actionName);
    if (func == FB_ACTION_MAP.end()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "no func to process");
        return;
    }
    func->second(reason);
}

// LCOV_EXCL_STOP

}
}