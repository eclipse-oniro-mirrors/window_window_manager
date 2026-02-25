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

#ifndef ANI_FB_WINDOW_LISTENER_H
#define ANI_FB_WINDOW_LISTENER_H

#include <map>
#include <mutex>

#include "refbase.h"
#include "window.h"
#include "event_handler.h"

#include "floating_ball_interface.h"
#include "class_var_definition.h"
#include "window_manager.h"
#include "wm_common.h"
#include "ani_fb_window_utils.h"

namespace OHOS {
namespace Rosen {

class AniFbWindowListener : public IFbLifeCycle,
                        public IFbClickObserver {
public:
    AniFbWindowListener(ani_env *env, ani_vm *vm, ani_ref callback)
        : env_(env), vm_(vm), aniCallback_(callback),
        weakRef_(wptr<AniFbWindowListener>(this)) {}
    ~AniFbWindowListener();

    ani_ref GetAniCallback() {return aniCallback_;}
    void SetAniCallback(ani_ref aniCallback) {aniCallback_ = aniCallback;}
    void SetMainEventHandler();
    void OnClickEvent() override;
    void OnFloatingBallStart() override;
    void OnFloatingBallStop() override;
private:
    ani_env *env_ = nullptr;
    [[maybe_unused]] ani_vm *vm_ = nullptr;
    ani_ref aniCallback_;
    wptr<AniFbWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    
    void OnFbListenerCallback(const FloatingBallState &state);
};  // class AniFbWindowListener
}  // namespace Rosen
}  // namespace OHOS

#endif  // ANI_FB_WINDOW_LISTENER_H