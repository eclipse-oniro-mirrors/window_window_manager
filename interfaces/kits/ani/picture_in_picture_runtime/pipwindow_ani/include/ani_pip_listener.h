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

#ifndef ANI_PIP_LISTENER_H
#define ANI_PIP_LISTENER_H

#include <memory>
#include <map>
#include <mutex>

#include "refbase.h"
#include "window.h"
#include "ani.h"
#include "event_handler.h"
#include "event_runner.h"

#include "window_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "class_var_definition.h"
#include "picture_in_picture_interface.h"

namespace OHOS {
namespace Rosen {

class AniPiPListener : public IPiPLifeCycle,
                             public IPiPActionObserver,
                             public IPiPControlObserver,
                             public IPiPWindowSize,
                             public IPiPActiveStatusObserver {
public:
    AniPiPListener(ani_env* env, ani_vm* vm, ani_ref callbackRef)
        : env_(env), vm_(vm), aniCallback_(callbackRef),
        weakRef_(wptr<AniPiPListener>(this)) {}
    ~AniPiPListener() override;

    ani_ref GetAniCallback() {return aniCallback_;}
    void SetAniCallback(ani_ref aniCallback) {aniCallback_ = aniCallback;}
    void SetMainEventHandler();

    // IPiPLifeCycle
    void OnPreparePictureInPictureStart(PiPStateChangeReason reason) override;
    void OnPictureInPictureStart(PiPStateChangeReason reason) override;
    void OnPreparePictureInPictureStop(PiPStateChangeReason reason) override;
    void OnPictureInPictureStop(PiPStateChangeReason reason) override;
    void OnRestoreUserInterface(PiPStateChangeReason reason) override;
    void OnPictureInPictureOperationError(int32_t errorCode) override;

    // IPiPActionObserver
    void OnActionEvent(const std::string& actionEvent, int32_t statusCode) override;

    // IPiPControlObserver
    void OnControlEvent(PiPControlType controlType, PiPControlStatus statusCode) override;

    // IPiPWindowSize
    void OnPipSizeChange(const PiPWindowSize& size) override;

    // IPiPActiveStatusObserver
    void OnActiveStatusChange(const bool& status) override;

private:
    void OnPipStateChangeDispatch(const PiPState& state, const PiPStateChangeReason& reason);
    void OnPipStateChangeDispatchError(const int32_t& errorCode);
    std::string GetStringByStateChangeReason(const PiPStateChangeReason& reasonCode);

    ani_env* env_ = nullptr;
     [[maybe_unused]] ani_vm* vm_ = nullptr;
    ani_ref aniCallback_ = nullptr; // GlobalRef

    wptr<AniPiPListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
};

} // namespace Rosen
} // namespace OHOS

#endif // ANI_PIP_LISTENER_H