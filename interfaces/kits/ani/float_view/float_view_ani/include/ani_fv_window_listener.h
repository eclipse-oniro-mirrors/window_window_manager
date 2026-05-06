/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
 * Licensed under * Apache License, Version 2.0 (the "License");
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

#ifndef ANI_FV_WINDOW_LISTENER_H
#define ANI_FV_WINDOW_LISTENER_H

#include <map>
#include <mutex>

#include "refbase.h"
#include "window.h"
#include "event_handler.h"

#include "float_view_interface.h"
#include "class_var_definition.h"
#include "window_manager.h"
#include "wm_common.h"
#include "ani_fv_window_utils.h"

namespace OHOS {
namespace Rosen {

class AniFvWindowListener : public IFvStateChangeObserver,
                        public IFvRectChangeObserver,
                        public IFvLimitsChangeObserver {
public:
    AniFvWindowListener(ani_env *env, ani_vm *vm, ani_ref callback)
        : env_(env), vm_(vm), aniCallback_(callback),
        weakRef_(wptr<AniFvWindowListener>(this)) {}
    ~AniFvWindowListener();

    ani_ref GetAniCallback() {return aniCallback_;}
    void SetAniCallback(ani_ref aniCallback) {aniCallback_ = aniCallback;}
    void SetMainEventHandler();
    void OnStateChange(const FloatViewState& state, const std::string& stopReason) override;
    void OnRectangleChange(const Rect& window, double scale, const std::string& reason) override;
    void OnLimitsChange(const FloatViewLimits& limits) override;
private:
    ani_env *env_ = nullptr;
    [[maybe_unused]] ani_vm *vm_ = nullptr;
    ani_ref aniCallback_;
    wptr<AniFvWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    
    void OnFvListenerCallback(const FloatViewState &state);
};  // class AniFvWindowListener
}  // namespace Rosen
}  // namespace OHOS

#endif  // ANI_FV_WINDOW_LISTENER_H