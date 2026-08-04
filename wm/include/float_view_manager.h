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
#ifndef OHOS_FLOAT_VIEW_MANAGER_H
#define OHOS_FLOAT_VIEW_MANAGER_H

#include "window.h"
#include "float_view_controller.h"
 
namespace OHOS {
namespace Rosen {
class FloatViewManager {
public:
    FloatViewManager() = default;
    virtual ~FloatViewManager() = default;
    static bool isSupportFloatView_;

    static bool HasActiveController();
    static bool IsActiveController(const wptr<FloatViewController>& fvControllerWeak);
    static void SetActiveController(const wptr<FloatViewController>& fvControllerWeak);
    static void RemoveActiveController(const wptr<FloatViewController>& fvControllerWeak);

    static sptr<FloatViewController> GetActiveController() { return activeController_; }

    static void AddController(uint32_t windowId, wptr<FloatViewController> controller);
    static void RemoveController(uint32_t windowId);
    static wptr<FloatViewController> GetController(uint32_t windowId);

    static void DoActionCloseByMainWindow(uint32_t mainWindowId, const std::string& reason);
    static void DoActionEvent(uint32_t windowId, const std::string& actionName, const std::string& reason);
    static void DoActionStart(uint32_t windowId, const std::string& reason = "");
    static void DoActionClose(uint32_t mainWindowId, const std::string& reason);
    static void DoActionHide(uint32_t windowId, const std::string& reason);
    static void DoActionInSidebar(uint32_t windowId, const std::string& reason);
    static void DoActionInFloatingBall(uint32_t windowId, const std::string& reason);

    static void SyncFvWindowInfo(uint32_t windowId, const FloatViewWindowInfo& windowInfo, const std::string& reason);
    static void SyncFvLimits(uint32_t windowId, const std::map<uint32_t, FloatViewLimits>& limits);
private:
    // controller in use
    static sptr<FloatViewController> activeController_;
    static std::mutex controllerMapMutex_;
    static std::map<uint32_t, wptr<FloatViewController>> windowId2Controller_;
};
} // namespace Rosen
} // namespace OHOS
#endif