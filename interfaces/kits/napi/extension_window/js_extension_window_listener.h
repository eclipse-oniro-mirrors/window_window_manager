/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_EXTENSION_WINDOW_LISTENER_H
#define OHOS_JS_EXTENSION_WINDOW_LISTENER_H

#include <map>
#include <mutex>

#include "class_var_definition.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window.h"
#include "window_manager.h"
#include "wm_common.h"
#include "js_extension_window_utils.h"
#include "js_window_utils.h"

namespace OHOS {
namespace Rosen {
class JsExtensionWindowListener : public IWindowChangeListener,
                                  public IWindowRectChangeListener,
                                  public IAvoidAreaChangedListener,
                                  public IWindowLifeCycle,
                                  public IOccupiedAreaChangeListener,
                                  public IDisplayIdChangeListener,
                                  public ISystemDensityChangeListener,
                                  public IScreenshotListener,
                                  public IExtensionSecureLimitChangeListener,
                                  public IKeyboardDidShowListener,
                                  public IKeyboardDidHideListener,
                                  public IWindowStatusChangeListener {
public:
    JsExtensionWindowListener(napi_env env, std::shared_ptr<NativeReference> callback)
        : env_(env), jsCallBack_(callback), weakRef_(wptr<JsExtensionWindowListener> (this)) {
    }
    ~JsExtensionWindowListener();
    void OnSizeChange(Rect rect, WindowSizeChangeReason reason,
                      const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnRectChange(Rect rect, WindowSizeChangeReason reason) override;
    void OnModeChange(WindowMode mode, bool hasDeco) override;
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info = nullptr) override;
    void OnDisplayIdChanged(DisplayId displayId) override;
    void OnSystemDensityChanged(float density) override;
    void OnScreenshot() override;
    void OnSecureLimitChange(bool isLimit) override;
    void OnKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void OnKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void OnWindowStatusChange(WindowStatus status) override;
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterFocused() override;
    void AfterUnfocused() override;
    void AfterResumed() override;
    void AfterPaused() override;
    void AfterDestroyed() override;
    void OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
                      const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void CallJsMethod(const char* methodName, napi_value const * argv = nullptr, size_t argc = 0);
    void SetMainEventHandler();

private:
    Rect currRect_;
    napi_env env_ = nullptr;
    WindowState state_ {WindowState::STATE_INITIAL};
    std::shared_ptr<NativeReference> jsCallBack_;
    wptr<JsExtensionWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    DEFINE_VAR_DEFAULT_FUNC_SET(bool, IsDeprecatedInterface, isDeprecatedInterface, false)
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_JS_EXTENSION_WINDOW_LISTENER_H
