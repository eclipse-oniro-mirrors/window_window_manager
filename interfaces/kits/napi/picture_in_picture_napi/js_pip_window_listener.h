/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_PIP_WINDOW_LISTENER_H
#define OHOS_JS_PIP_WINDOW_LISTENER_H

#include "native_engine/native_value.h"
#include "picture_in_picture_interface.h"
#include "refbase.h"

namespace OHOS {
namespace Rosen {
class JsPiPWindowListener : public IPiPLifeCycle,
                            public IPiPActionObserver,
                            public IPiPControlObserver,
                            public IPiPWindowSize,
                            public IPiPTypeNodeObserver,
                            public IPiPActiveStatusObserver {
public:
    JsPiPWindowListener(napi_env env, const std::shared_ptr<NativeReference>& callback)
        : env_(env), jsCallBack_(callback) {}
    ~JsPiPWindowListener();
    std::shared_ptr<NativeReference> GetCallbackRef() const;
    void OnPreparePictureInPictureStart(PiPStateChangeReason reason) override;
    void OnPictureInPictureStart(PiPStateChangeReason reason) override;
    void OnPreparePictureInPictureStop(PiPStateChangeReason reason) override;
    void OnPictureInPictureStop(PiPStateChangeReason reason) override;
    void OnPictureInPictureOperationError(int32_t errorCode) override;
    void OnRestoreUserInterface(PiPStateChangeReason reason) override;
    void OnActionEvent(const std::string& actionEvent, int32_t statusCode) override;
    void OnControlEvent(PiPControlType controlType, PiPControlStatus statusCode) override;
    void OnPipSizeChange(const PiPWindowSize& size) override;
    void OnPipTypeNodeChange(const napi_ref nodeRef) override;
    void OnActiveStatusChange(bool status) override;

private:
    void OnPipListenerCallback(PiPState state, PiPStateChangeReason reason);
    std::string GetStringByStateChangeReason(PiPStateChangeReason reason);
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallBack_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_PIP_WINDOW_LISTENER_H */
