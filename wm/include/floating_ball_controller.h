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
 
#ifndef OHOS_FLOATING_BALL_CONTROLLER_H
#define OHOS_FLOATING_BALL_CONTROLLER_H
 
#include <refbase.h>
#include <ability_context.h>
#include "window.h"
#include "wm_common.h"
#include "floating_ball_option.h"
#include "floating_ball_interface.h"
#include "floating_ball_report.h"
#include <string>

namespace OHOS {
namespace Rosen {
using namespace Ace;
class FloatingBallController : public RefBase {
public:
    FloatingBallController(const sptr<Window>& mainWindow, const uint32_t& mainWindowId, void* context);
    virtual ~FloatingBallController();
    WMErrorResult StartFloatingBall(sptr<FbOption>& option);
    WMErrorResult StartFloatingBallSingle(const sptr<FbOption>& option, bool showWhenCreate = true);
    WMErrorResult StopFloatingBall(const std::string& reason);
    WMErrorResult UpdateFloatingBall(sptr<FbOption>& option);
    WMErrorResult SetInApplicationVisible(bool isVisible);
    WMErrorResult RestoreMainWindow(const std::shared_ptr<AAFwk::Want>& want);
    WMErrorResult DestroyFloatingBallWindow(const std::string& reason);
    sptr<Window> GetFbWindow() const;
    WMErrorResult StopFloatingBallFromClient();
    WMErrorResult StopFloatingBallFromClientSingle();

    WMErrorResult RegisterFbLifecycle(const sptr<IFbLifeCycle>& listener);
    WMErrorResult RegisterFbClickObserver(const sptr<IFbClickObserver>& listener);
    WMErrorResult RegisterFbDestroyObserver(const sptr<IFbDestroyObserver>& listener);
    WMErrorResult UnRegisterFbLifecycle(const sptr<IFbLifeCycle>& listener);
    WMErrorResult UnRegisterFbClickObserver(const sptr<IFbClickObserver>& listener);
    WMErrorResult UnRegisterFbDestroyObserver(const sptr<IFbDestroyObserver>& listener);
 
    void OnFloatingBallClick();
    void OnFloatingBallDestroy(const std::string& reason);
 
    WMErrorResult GetFloatingBallWindowInfo(uint32_t& windowId);

    void UpdateMainWindow(const sptr<Window>& mainWindow);
    FbWindowState GetCurState();
    void SetOption(const sptr<FbOption> &option);
    sptr<FbOption> GetOption();
    void SetBindState(bool isBind);
    bool IsBind();
    void SetBindWindowId(uint32_t windowId);
    void SetShowWhenCreate(bool showWhenCreate);
private:
    WMErrorResult CreateFloatingBallWindow(const sptr<FbOption>& option);
    WMErrorResult PrepareStartFloatingBall(const sptr<FbOption>& option, bool showWhenCreate);
    WMErrorResult StartFloatingBallInner(const sptr<FbOption>& option);
 
    template<typename T> WMErrorResult RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMErrorResult UnRegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
 
    void OnFloatingBallStart();
    void OnFloatingBallStop();
 
    wptr<FloatingBallController> weakRef_ = nullptr;
    sptr<Window> window_ = nullptr;
    sptr<Window> mainWindow_ = nullptr;
    uint32_t mainWindowId_ = 0;
    uint32_t templateType_ = 0;
    FbWindowState curState_ = FbWindowState::STATE_UNDEFINED;
    void* contextPtr_ = nullptr;
    bool stopFromClient_ = false;
    bool visibleInApp_ = true;

    sptr<FbOption> option_ = nullptr;
    bool bindState_ = false;
    uint32_t bindWindowId_ = INVALID_WINDOW_ID;
    std::string id_;

    std::vector<sptr<IFbLifeCycle>> fbLifeCycleListeners_;
    std::vector<sptr<IFbClickObserver>> fbClickObservers_;
    std::vector<sptr<IFbDestroyObserver>> fbDestroyObservers_;

    std::mutex controllerMutex_;

    class WindowLifeCycleListener : public IWindowLifeCycle {
    public:
        void AfterDestroyed() override;
    };
    sptr<IWindowLifeCycle> mainWindowLifeCycleListener_ = nullptr;
    std::mutex listenerMutex_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_FLOATING_BALL_CONTROLLER_H