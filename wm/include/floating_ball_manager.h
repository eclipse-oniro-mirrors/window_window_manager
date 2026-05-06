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
 
#ifndef OHOS_FLOATING_BALL_MANAGER_H
#define OHOS_FLOATING_BALL_MANAGER_H
 
 
#include "window.h"
#include "wm_common.h"
#include "floating_ball_controller.h"
 
namespace OHOS {
namespace Rosen {
class FloatingBallManager {
public:
    FloatingBallManager() = default;
    virtual ~FloatingBallManager() = default;
    static bool HasActiveController();
    static bool IsActiveController(const wptr<FloatingBallController>& fbController);
    static void SetActiveController(const sptr<FloatingBallController>& fbController);
    static void RemoveActiveController(const wptr<FloatingBallController>& fbController);

    static sptr<FloatingBallController> GetActiveController() { return activeController_; }

    static void DoActionClick();
    static void DoActionClose();
    static void DoDestroy();

    static void DoFbActionEvent(const std::string& actionName);

    static bool IsSupportFloatingBall();

private:
    // controller in use
    static sptr<FloatingBallController> activeController_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_FLOATING_BALL_MANAGER_H