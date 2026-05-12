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

#ifndef OHOS_JS_FB_WINDOW_LISTENER_H
#define OHOS_JS_FB_WINDOW_LISTENER_H

#include "native_engine/native_value.h"
#include "floating_ball_interface.h"
#include "refbase.h"
#include "wm_common.h"
#include "window_histogram_management.h"


namespace OHOS {
namespace Rosen {
class JsFbWindowListener : public IFbLifeCycle,
                           public IFbClickObserver {
public:
    JsFbWindowListener(napi_env env, const std::shared_ptr<NativeReference>& callback)
        : env_(env), jsCallBack_(callback) {}
    ~JsFbWindowListener() override;
    std::shared_ptr<NativeReference> GetCallbackRef() const;
    void OnFloatingBallStart() override;
    void OnFloatingBallStop() override;

    void OnClickEvent() override;

private:
    void OnFbListenerCallback(const FloatingBallState& state);
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallBack_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_FB_WINDOW_LISTENER_H */
