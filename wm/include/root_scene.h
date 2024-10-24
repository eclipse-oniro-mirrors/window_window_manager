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
using GetSessionRectCallback = std::function<WSRect(AvoidAreaType)>;

class RootScene : public Window {
public:
    RootScene();
    virtual ~RootScene();

    void LoadContent(const std::string& contentUrl, napi_env env, napi_value storage,
        AbilityRuntime::Context* context);
    void UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason);
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;

    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    int64_t GetVSyncPeriod() override;
    void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType = 0) override;

    void OnBundleUpdated(const std::string& bundleName);
    static void SetOnConfigurationUpdatedCallback(
        const std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)>& callback);
    void SetFrameLayoutFinishCallback(std::function<void()>&& callback);

    void SetGetSessionRectCallback(GetSessionRectCallback&& callback)
    {
        getSessionRectCallback_ = std::move(callback);
    }

    void SetDisplayDensity(float density)
    {
        density_ = density;
    }

    void SetDisplayOrientation(int32_t orientation);

    float GetDisplayDensity()
    {
        return density_;
    }

    WindowState GetWindowState() const override
    {
        return WindowState::STATE_SHOWN;
    }

    WindowType GetType() const override
    {
        return type_;
    }

    const std::string& GetWindowName() const override
    {
        return name_;
    }

    uint32_t GetWindowId() const override
    {
        return 1; // 1 for root
    }

    Ace::UIContent* GetUIContent() const override
    {
        return uiContent_.get();
    }
    
    void SetUiDvsyncSwitch(bool dvsyncSwitch) override;

    WMError GetSessionRectByType(AvoidAreaType type, WSRect& rect);

    static sptr<RootScene> staticRootScene_;

private:
    void RegisterInputEventListener();

    std::unique_ptr<Ace::UIContent> uiContent_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    sptr<AppExecFwk::LauncherService> launcherService_;
    float density_ = 1.0f;
    int32_t orientation_ = 0;
    WindowType type_ = WindowType::WINDOW_TYPE_SCENE_BOARD;
    std::string name_ = "EntryView";

    static std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)> configurationUpdatedCallback_;
    std::function<void()> frameLayoutFinishCb_ = nullptr;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;

    GetSessionRectCallback getSessionRectCallback_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_ROOT_SCENE_H
