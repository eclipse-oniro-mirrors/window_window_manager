/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_ROOT_SCENE_H
#define OHOS_ROSEN_ROOT_SCENE_H

#include <mutex>

#include "vsync_station.h"
#include "window.h"
#include "window_helper.h"
#include "ws_common.h"

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;
namespace OHOS::AppExecFwk {
class EventHandler;
class LauncherService;
} // namespace OHOS::AppExecFwk

namespace OHOS::Ace {
class UIContent;
} // namespace OHOS::Ace

namespace OHOS {
namespace Rosen {
class RSNode;
using GetSessionAvoidAreaByTypeCallback = std::function<AvoidArea(AvoidAreaType, bool)>;
using GetStatusBarHeightCallback = std::function<uint32_t()>;
using UpdateRootSceneRectCallback = std::function<void(const Rect& rect)>;
using UpdateRootSceneAvoidAreaCallback = std::function<void()>;
using NotifyWatchFocusActiveChangeCallback = std::function<void(bool isFocusActive)>;
using UIContentResult = std::pair<Ace::UIContent*, bool>;

class RootScene : public Window {
public:
    RootScene();
    virtual ~RootScene();

    void LoadContent(const std::string& contentUrl, napi_env env, napi_value storage,
        AbilityRuntime::Context* context);
    void UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason);
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});
    void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) override;

    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    int64_t GetVSyncPeriod() override;
    void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType = 0) override;

    /*
     * Window Immersive
     */
    bool IsLastFrameLayoutFinished();
    void OnFlushUIParams();
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect = Rect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) override;
    WMError GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type, AvoidArea& avoidArea,
        const Rect& rect = Rect::EMPTY_RECT) override;
    void RegisterGetSessionAvoidAreaByTypeCallback(GetSessionAvoidAreaByTypeCallback&& callback);
    uint32_t GetStatusBarHeight() const override;
    void RegisterGetStatusBarHeightCallback(GetStatusBarHeightCallback&& callback);
    void RegisterUpdateRootSceneRectCallback(UpdateRootSceneRectCallback&& callback);
    WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    void NotifyAvoidAreaChangeForRoot(const sptr<AvoidArea>& avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info = nullptr);
    void RegisterUpdateRootSceneAvoidAreaCallback(UpdateRootSceneAvoidAreaCallback&& callback);
    bool IsSystemWindow() const override { return WindowHelper::IsSystemWindow(GetType()); }
    bool IsAppWindow() const override { return WindowHelper::IsAppWindow(GetType()); }
    void GetExtensionConfig(AAFwk::WantParams& want) const override;

    /*
     * Keyboard Window
     */
    WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;

    /*
     * watch
     */
    void RegisterNotifyWatchFocusActiveChangeCallback(NotifyWatchFocusActiveChangeCallback&& callback);

    const std::shared_ptr<AbilityRuntime::Context> GetContext() const override { return context_.lock(); }

    void OnBundleUpdated(const std::string& bundleName);
    static void SetOnConfigurationUpdatedCallback(
        const std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)>& callback);
    void SetFrameLayoutFinishCallback(std::function<void()>&& callback);

    void SetDisplayDensity(float density) { density_ = density; }

    void SetDisplayId(DisplayId displayId) { displayId_ = displayId; }

    DisplayId GetDisplayId() const override { return displayId_; }

    void SetDisplayOrientation(int32_t orientation);

    float GetDisplayDensity() { return density_; }

    WindowState GetWindowState() const override { return WindowState::STATE_SHOWN; }

    WindowType GetType() const override { return type_; }

    const std::string& GetWindowName() const override { return name_; }

    uint32_t GetWindowId() const override { return 1; } // 1 for root

    Ace::UIContent* GetUIContent() const override { return uiContent_.get(); }

    UIContentResult GetUIContentByDisplayId(DisplayId displayId);

    void AddRootScene(DisplayId displayId, wptr<Window> window);

    void RemoveRootScene(DisplayId displayId);

    void SetUiDvsyncSwitch(bool dvsyncSwitch) override;

    void SetTouchEvent(int32_t touchType) override;

    /*
     * Window Layout
     */
    std::shared_ptr<Rosen::RSNode> GetRSNodeByStringID(const std::string& stringId);
    void SetTopWindowBoundaryByID(const std::string& stringId);
    bool HasRequestedVsync() const { return vsyncStation_->HasRequestedVsync(); }

    /*
     * Window Property
     */
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;

    static sptr<RootScene> staticRootScene_;

    std::string GetClassType() const override { return "RootScene"; }

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> GetRSUIDirector() const override;
    std::shared_ptr<RSUIContext> GetRSUIContext() const override;

private:
    void RegisterInputEventListener();

    std::unique_ptr<Ace::UIContent> uiContent_;
    std::unordered_map<DisplayId, wptr<Window>> rootSceneMap_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    sptr<AppExecFwk::LauncherService> launcherService_;
    float density_ = 1.0f;
    DisplayId displayId_ = DISPLAY_ID_INVALID;
    int32_t orientation_ = 0;
    WindowType type_ = WindowType::WINDOW_TYPE_SCENE_BOARD;
    std::string name_ = "EntryView";

    static std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)> configurationUpdatedCallback_;
    std::function<void()> frameLayoutFinishCb_ = nullptr;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    std::weak_ptr<AbilityRuntime::Context> context_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;

    /*
     * Window Immersive
     */
    GetSessionAvoidAreaByTypeCallback getSessionAvoidAreaByTypeCallback_ = nullptr;
    GetStatusBarHeightCallback getStatusBarHeightCallback_ = nullptr;
    UpdateRootSceneRectCallback updateRootSceneRectCallback_ = nullptr;
    UpdateRootSceneAvoidAreaCallback updateRootSceneAvoidAreaCallback_ = nullptr;
    mutable std::mutex mutex_;
    std::unordered_set<sptr<IAvoidAreaChangedListener>, SptrHash<IAvoidAreaChangedListener>> avoidAreaChangeListeners_;

    /*
     * Keyboard Window
     */
    mutable std::mutex occupiedAreaMutex_;
    std::unordered_set<sptr<IOccupiedAreaChangeListener>, SptrHash<IOccupiedAreaChangeListener>>
        occupiedAreaChangeListeners_;

    /*
     * watch
     */
    NotifyWatchFocusActiveChangeCallback notifyWatchFocusActiveChangeCallback_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_ROOT_SCENE_H
