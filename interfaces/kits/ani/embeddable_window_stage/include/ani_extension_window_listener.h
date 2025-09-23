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

#ifndef OHOS_ANI_EXTENSION_WINDOW_LISTENER_H
#define OHOS_ANI_EXTENSION_WINDOW_LISTENER_H

#include <mutex>

#include "ani.h"
#include "class_var_definition.h"
#include "event_handler.h"
#include "refbase.h"
#include "window.h"
#include "window_manager.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
const std::string WINDOW_SIZE_CHANGE_CB = "windowSizeChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";
const std::string WINDOW_STAGE_EVENT_CB = "windowStageEvent";
const std::string WINDOW_EVENT_CB = "windowEvent";
constexpr const char* ETS_UIEXTENSION_HOST_NAMESPACE_DESCRIPTOR = "L@ohos/uiExtensionHost/uiExtensionHost;";

class AniExtensionWindowListener : public IWindowChangeListener,
                                   public IAvoidAreaChangedListener,
                                   public IWindowLifeCycle,
                                   public IOccupiedAreaChangeListener {
public:
    AniExtensionWindowListener(ani_env* env, ani_ref func)
        : env_(env), aniCallback_(func), weakRef_(wptr<AniExtensionWindowListener> (this)) {}
    ~AniExtensionWindowListener();
    ani_ref GetAniCallback() const { return aniCallback_; }
    void SetAniCallback(ani_ref aniCallback) { aniCallback_ = aniCallback; }
    void OnSizeChange(Rect rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info = nullptr) override;
    void OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void SetMainEventHandler();

private:
    void CallSizeChangeCallback(ani_object size);
    uint32_t currentWidth_ = 0;
    uint32_t currentHeight_ = 0;
    ani_env* env_ = nullptr;
    ani_ref aniCallback_;
    wptr<AniExtensionWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    DEFINE_VAR_DEFAULT_FUNC_SET(bool, IsDeprecatedInterface, isDeprecatedInterface, false)
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ANI_EXTENSION_WINDOW_LISTENER_H
