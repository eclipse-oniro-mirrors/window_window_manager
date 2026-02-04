/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ANI_SCREEN_LISTENER_H
#define OHOS_ANI_SCREEN_LISTENER_H

#include <mutex>

#include "ani.h"
#include "event_handler.h"
#include "screen_manager.h"
#include "refbase.h"

namespace OHOS {
namespace Rosen {
class ScreenAniListener : public ScreenManager::IScreenListener {
public:
    explicit ScreenAniListener(ani_env* env)
        : env_(env), weakRef_(wptr<ScreenAniListener> (this)) {}
    ~ScreenAniListener() override;
    void AddCallback(const std::string& type, ani_ref callback);
    void RemoveAllCallback();
    void RemoveCallback(ani_env* env, const std::string& type, ani_ref callback);
    void OnConnect(ScreenId id) override;
    void OnDisconnect(ScreenId id) override;
    void OnChange(ScreenId id) override;
    ani_status CallAniMethodVoid(ani_object object, const char* cls,
        const char* method, const char* signature, ...);
    void SetMainEventHandler();

private:
    ani_env* env_;
    std::mutex mtx_;
    std::map<std::string, std::vector<ani_ref>> aniCallback_;
    wptr<ScreenAniListener> weakRef_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
};
extern const std::string ANI_EVENT_CONNECT;
extern const std::string ANI_EVENT_DISCONNECT;
extern const std::string ANI_EVENT_CHANGE;
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_SCREEN_LISTENER_H */