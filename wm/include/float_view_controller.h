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
 
#ifndef OHOS_FLOAT_VIEW_CONTROLLER_H
#define OHOS_FLOAT_VIEW_CONTROLLER_H

#include <vector>

#include "float_view_option.h"
#include "float_view_interface.h"

#include <refbase.h>
#include "window.h"
#include "wm_common.h"
#include "want.h"

namespace OHOS {
namespace Rosen {
class FloatViewController : public RefBase {
public:
    enum class APIType : uint32_t {
        NONE,
        NAPI,
        ANI,
    };
    FloatViewController(const FvOption &option, napi_env env);
    FloatViewController(const FvOption &option, ani_env* env);
    virtual ~FloatViewController();
    WMError StartFloatView();
    WMError StartFloatViewSingle(bool showWhenCreate = true);
    WMError StopFloatViewFromClient();
    WMError StopFloatViewFromClientSingle();
    WMError StopFloatView(const std::string& reason);
    WMError RestoreMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParams);
    WMError SetUIContext(const std::string &contextUrl, const std::shared_ptr<NativeReference>& contentStorage);
    WMError SetUIContext(const std::string &contextUrl, const ani_object& contentStorage);
    WMError SetVisibilityInApp(bool visibleInApp);
    WMError SetWindowSize(const Rect &rect);
    void SyncWindowInfo(uint32_t windowId, const FloatViewWindowInfo& windowInfo, const std::string& reason);
    void SyncLimits(uint32_t windowId, const std::map<uint32_t, FloatViewLimits>& fvLimits);
    FloatViewWindowInfo GetWindowInfo() const;
    sptr<Window> GetWindow() const;
    FvWindowState GetCurState();
    FloatViewTemplate GetTemplateType() const;
    void ChangeState(const FvWindowState &newState);
    bool IsStateWithWindow(FvWindowState state);
    void SetBindState(bool isBind);
    bool IsBind();
    void SetBindWindowId(uint32_t windowId);
    void SetShowWhenCreate(bool showWhenCreate);

    WMError RegisterStateChangeListener(const sptr<IFvStateChangeObserver>& listener);
    WMError UnregisterStateChangeListener(const sptr<IFvStateChangeObserver>& listener);
    WMError RegisterRectChangeListener(const sptr<IFvRectChangeObserver>& listener);
    WMError UnregisterRectChangeListener(const sptr<IFvRectChangeObserver>& listener);
    WMError RegisterLimitsChangeListener(const sptr<IFvLimitsChangeObserver>& listener);
    WMError UnregisterLimitsChangeListener(const sptr<IFvLimitsChangeObserver>& listener);
private:
    WMError StartFloatViewInner();
    WMError PrepareStartFloatView(bool showWhenCreate);
    WMError CreateFloatViewWindow();
    WMError SetFloatViewContext();
    WMError DestroyFloatViewWindow(const std::string& reason);
    WMError SetUIContextInner();
    ani_env* GetEnv() const;

    std::mutex listenerMutex_;
    template<typename T> WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError UnRegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);

    void OnStateChange(const FloatViewState& state, std::string stopReason = "");
    void OnRectChange(const Rect& window, double scale, const std::string& reason);
    void OnLimitsChange(const FloatViewLimits& limits);

    wptr<FloatViewController> weakRef_ = nullptr;
    sptr<Window> window_ = nullptr;
    sptr<Window> mainWindow_ = nullptr;
    uint32_t mainWindowId_ = 0;
    FvWindowState curState_ = FvWindowState::FV_STATE_UNDEFINED;
    bool stopFromClient_ = false;
    FvOption option_;
    napi_env env_ = nullptr;
    ani_vm* vm_ = nullptr;
    APIType type_;
    bool bindState_ = false;
    uint32_t bindWindowId_ = INVALID_WINDOW_ID;
    std::string id_;

    class WindowLifeCycleListener : public IWindowLifeCycle {
    public:
        void AfterDestroyed() override;
    };

    sptr<IWindowLifeCycle> mainWindowLifeCycleListener_ = nullptr;
    std::vector<sptr<IFvStateChangeObserver>> stateChangeObservers_;
    std::vector<sptr<IFvRectChangeObserver>> rectChangeObservers_;
    std::vector<sptr<IFvLimitsChangeObserver>> limitsChangeObservers_;

    std::mutex controllerMutex_;
    FloatViewWindowInfo windowInfo_ {};
};
} // namespace Rosen
} // namespace OHOS
#endif
