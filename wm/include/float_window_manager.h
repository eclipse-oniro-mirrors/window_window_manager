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
#ifndef OHOS_FLOAT_WINDOW_MANAGER_H
#define OHOS_FLOAT_WINDOW_MANAGER_H

#include "window.h"
#include "float_view_controller.h"
#include "floating_ball_controller.h"

#include <condition_variable>

namespace OHOS {
namespace Rosen {
class FloatWindowManager {
public:
    FloatWindowManager() = default;
    virtual ~FloatWindowManager() = default;
    static WMError Bind(const sptr<FloatViewController> &fvController, const sptr<FloatingBallController> &fbController,
        const FbOption &fbOption);
    static WMError UnBind(const sptr<FloatViewController> &fvController,
        const sptr<FloatingBallController> &fbController);
    static WMError StartBindFloatView(const wptr<FloatViewController> &fvControllerWeak);
    static WMError StartBindFloatingBall(const wptr<FloatingBallController> &fbControllerWeak,
        const sptr<FbOption> &option);
    static WMError StopBindFloatView(const wptr<FloatViewController> &fvControllerWeak);
    static WMError StopBindFloatingBall(const wptr<FloatingBallController> &fbControllerWeak);
    static std::string GetControllerId();

    // Bound-pair helpers
    static sptr<FloatingBallController> GetBoundFloatingBall(const sptr<FloatViewController>& fvController);
    static sptr<FloatViewController> GetBoundFloatView(const sptr<FloatingBallController>& fbController);

    static sptr<Window> CreateFbWindow(sptr<WindowOption> &windowOption,
        const FloatingBallTemplateBaseInfo &templateInfo, const std::shared_ptr<Media::PixelMap>& icon,
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError &error,
        const wptr<FloatingBallController> &fbControllerWeak);
    static sptr<Window> CreateFvWindow(sptr<WindowOption> &windowOption,
        const FloatViewTemplateInfo &templateInfo, const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
        WMError &error, const wptr<FloatViewController> &fvControllerWeak);
    static sptr<Window> CreatePipWindow(sptr<WindowOption> &windowOption, const PiPTemplateInfo &templateInfo,
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError &error);
    static WMError DestroyFloatWindow(const sptr<Window> &window);

    static void ProcessBindFloatViewStateChange(const wptr<FloatViewController> &fvControllerWeak,
        const FvWindowState state);

private:
    static std::recursive_mutex relationMutex_;
    static std::map<sptr<FloatViewController>, sptr<FloatingBallController>> floatViewToFloatingBallMap_;

    static sptr<FloatViewController> FindFloatViewByFloatingBall(const sptr<FloatingBallController>& fbController);

    static void AddRelation(const sptr<FloatViewController> &fvController,
        const sptr<FloatingBallController> &fbController);
    static void RemoveRelation(const sptr<FloatViewController> &fvController,
        const sptr<FloatingBallController> &fbController);
    static bool IsFloatingBallStateValid(const sptr<FloatingBallController> &fbController);
    static bool IsFloatViewStateValid(const sptr<FloatViewController> &fvController);

    static std::mutex windowMutex_;
    static uint32_t floatViewCnt_;
    static uint32_t floatingBallCnt_;
    static uint32_t pipCnt_;
};
} // namespace Rosen
} // namespace OHOS
#endif
