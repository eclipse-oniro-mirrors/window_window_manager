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

#ifndef OHOS_PICTURE_IN_PICTURE_CONTROLLER_BASE_H
#define OHOS_PICTURE_IN_PICTURE_CONTROLLER_BASE_H

#define PIP_WINDOW_NAME "pip_window"

#include <ability_context.h>
#include <refbase.h>
#include "navigation_controller.h"
#include "picture_in_picture_interface.h"
#include "picture_in_picture_option.h"
#include "pip_report.h"
#include "xcomponent_controller.h"
#include "window.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
enum class StartPipType : uint32_t {
    NULL_START = 0,
    USER_START = 1,
    AUTO_START = 2,
    ONE_STEP_START = 3,
    NATIVE_START = 4,
};

enum class StopPipType : uint32_t {
    NULL_STOP = 0,
    USER_STOP = 1,
    OTHER_PACKAGE_STOP = 2,
    PACKAGE_STOP = 3,
    NATIVE_STOP = 4,
};

namespace PipConst {
    constexpr int32_t PIP_SUCCESS = 1;
    constexpr int32_t FAILED = 0;
    const int DEFAULT_ASPECT_RATIOS[] = {16, 9};
}

using namespace Ace;
class PictureInPictureControllerBase : public RefBase {
public:
    PictureInPictureControllerBase() { weakRef_ = this; }
    PictureInPictureControllerBase(sptr<PipOption> pipOption, sptr<Window> mainWindow, uint32_t windowId, napi_env env);
    virtual ~PictureInPictureControllerBase();
    WMError StopPictureInPicture(bool destroyWindow, StopPipType stopPipType, bool withAnim = true);
    WMError StopPictureInPictureFromClient();
    WMError DestroyPictureInPictureWindow();
    sptr<Window> GetPipWindow() const;
    uint32_t GetMainWindowId() const;
    void SetControllerId(uint32_t controllerId);
    uint32_t GetControllerId() const;
    void SetPipWindow(sptr<Window> window);
    void UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status);
    bool IsContentSizeChanged(float width, float height, float posX, float posY);
    void DoActionEvent(const std::string& actionName, int32_t status);
    void DoControlEvent(PiPControlType controlType, PiPControlStatus status);
    void LocateSource();
    void PipSizeChange(double width, double height, double scale);
    void PreRestorePictureInPicture();
    void GetPipPossible(bool& pipPossible);
    void ActiveStatusChange(bool status);
    WMError RegisterPiPLifecycle(const sptr<IPiPLifeCycle>& listener);
    WMError RegisterPiPActionObserver(const sptr<IPiPActionObserver>& listener);
    WMError RegisterPiPControlObserver(const sptr<IPiPControlObserver>& listener);
    WMError RegisterPiPWindowSize(const sptr<IPiPWindowSize>& listener);
    WMError RegisterPiPTypeNodeChange(const sptr<IPiPTypeNodeObserver>& listener);
    WMError RegisterPiPStart(const sptr<IPiPStartObserver>& listener);
    WMError RegisterPiPActiveStatusChange(const sptr<IPiPActiveStatusObserver>& listener);
    WMError UnregisterPiPLifecycle(const sptr<IPiPLifeCycle>& listener);
    WMError UnregisterPiPActionObserver(const sptr<IPiPActionObserver>& listener);
    WMError UnregisterPiPControlObserver(const sptr<IPiPControlObserver>& listener);
    WMError UnregisterPiPWindowSize(const sptr<IPiPWindowSize>& listener);
    WMError UnRegisterPiPTypeNodeChange(const sptr<IPiPTypeNodeObserver>& listener);
    WMError UnregisterPiPStart(const sptr<IPiPStartObserver>& listener);
    WMError UnregisterPiPActiveStatusChange(const sptr<IPiPActiveStatusObserver>& listener);
    void UnregisterAllPiPLifecycle();
    void UnregisterAllPiPControlObserver();
    void UnregisterAllPiPWindowSize();
    void UnregisterAllPiPStart();
    std::vector<sptr<IPiPLifeCycle>> GetPictureInPictureLifecycle() const;
    std::vector<sptr<IPiPActionObserver>> GetPictureInPictureActionObserver() const;
    std::vector<sptr<IPiPControlObserver>> GetPictureInPictureControlObserver() const;
    std::vector<sptr<IPiPWindowSize>> GetPictureInPictureSizeObserver() const;
    std::vector<sptr<IPiPStartObserver>> GetPictureInPictureStartObserver() const;
    PiPWindowState GetControllerState() const;
    void OnPictureInPictureStart();
    void SetSurfaceId(uint64_t surfaceId);
    static bool GetPipEnabled();
    uint64_t GetSurfaceId() const;
    bool GetPipSettingSwitchStatusEnabled();
    bool GetPiPSettingSwitchStatus();
    bool isWeb_ = false;
    void SetStateChangeReason(PiPStateChangeReason reason);
    PiPStateChangeReason GetStateChangeReason() const;
    WMError IsPiPActive(bool& status);

    // diffrent between normal and web
    virtual WMError StartPictureInPicture(StartPipType startType) = 0;
    virtual void UpdateContentSize(int32_t width, int32_t height) = 0;
    virtual WMError SetXComponentController(std::shared_ptr<XComponentController> xComponentController) = 0;
    virtual void RestorePictureInPictureWindow() = 0;

    // normal
    virtual void SetAutoStartEnabled(bool enable) {}
    virtual void IsAutoStartEnabled(bool& enable) const {};
    virtual void UpdateContentNodeRef(napi_ref nodeRef) {};
    virtual void PrepareSource() {};
    virtual napi_ref GetCustomNodeController() { return nullptr; };
    virtual napi_ref GetTypeNode() const { return nullptr; };
    virtual bool IsTypeNodeEnabled() const { return false; };

    // web
    virtual uint8_t GetWebRequestId() { return 0; };
    virtual void SetPipInitialSurfaceRect(int32_t positionX, int32_t positionY, uint32_t width, uint32_t height) {};

protected:
    class WindowLifeCycleListener : public IWindowLifeCycle {
    public:
        void AfterDestroyed() override;
    };

    void NotifyOpretationError(WMError errCode, StartPipType startType);
    WMError ShowPictureInPictureWindow(StartPipType startType);
    WMError StartPictureInPictureInner(StartPipType startType);
    WMError StopPictureInPictureInner(StopPipType stopType, bool withAnim);
    template<typename T> WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError WebRegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError WebUnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);

    sptr<PipOption> pipOption_ = nullptr;
    uint32_t controllerId_ = 0;
    std::vector<sptr<IPiPLifeCycle>> pipLifeCycleListeners_;
    std::vector<sptr<IPiPActionObserver>> pipActionObservers_;
    std::vector<sptr<IPiPControlObserver>> pipControlObservers_;
    std::vector<sptr<IPiPWindowSize>> pipWindowSizeListeners_;
    std::vector<sptr<IPiPTypeNodeObserver>> pipTypeNodeObserver_;
    std::vector<sptr<IPiPStartObserver>> pipStartListeners_;
    std::vector<sptr<IPiPActiveStatusObserver>> PiPActiveStatusObserver_;
    sptr<Window> window_ = nullptr;
    sptr<Window> mainWindow_ = nullptr;
    sptr<IWindowLifeCycle> mainWindowLifeCycleListener_ = nullptr;
    uint32_t mainWindowId_ = 0;
    Rect windowRect_ = {0, 0, 0, 0};
    bool isAutoStartEnabled_ = false;
    PiPWindowState curState_ = PiPWindowState::STATE_UNDEFINED;
    bool curActiveStatus_ = false;
    std::shared_ptr<XComponentController> pipXComponentController_ = nullptr;
    napi_env env_ = nullptr;
    bool isStoppedFromClient_ = false;
    int32_t handleId_ = -1;
    uint64_t surfaceId_ = 0;
    PiPStateChangeReason stateChangeReason_ = PiPStateChangeReason::OTHER;

    // diffrent between normal and web
    virtual WMError CreatePictureInPictureWindow(StartPipType startType) = 0;
    virtual void UpdateWinRectByComponent() = 0;
    virtual void SetUIContent() const = 0;
    virtual void UpdatePiPSourceRect() const = 0;

    // normal
    virtual void ResetExtController() {};
    virtual void NotifyNodeUpdate(napi_ref nodeRef) {};

    // web
    virtual WMError SetPipParentWindowId(uint32_t windowId) { return WMError::WM_ERROR_PIP_INTERNAL_ERROR; };

private:
    wptr<PictureInPictureControllerBase> weakRef_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_PICTURE_IN_PICTURE_CONTROLLER_H