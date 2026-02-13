/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCREEN_SCENE_H
#define OHOS_ROSEN_SCREEN_SCENE_H

#include <mutex>

#include "vsync_station.h"
#include "window.h"
#include "window_helper.h"

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;
namespace OHOS::AppExecFwk {
class EventHandler;
} // namespace OHOS::AppExecFwk

namespace OHOS::Ace {
class UIContent;
} // namespace OHOS::Ace

namespace OHOS {
namespace Rosen {
class ScreenScene : public Window {
public:
    ScreenScene(std::string name);
    virtual ~ScreenScene();

    void LoadContent(const std::string& contentUrl, napi_env env, napi_value storage,
        AbilityRuntime::Context* context);
    void UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason);
    void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;

    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    int64_t GetVSyncPeriod() override;
    void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType = 0) override;
    void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void SetOnConfigurationUpdatedCallback(
        const std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)>& callback);
    void OnBundleUpdated(const std::string& bundleName);
    void SetFrameLayoutFinishCallback(std::function<void()>&& callback);

    void SetDisplayDensity(float density);

    void SetDisplayOrientation(int32_t orientation);

    DisplayId GetDisplayId() const override;
    void SetDisplayId(DisplayId displayId);

    bool IsSystemWindow() const override { return WindowHelper::IsSystemWindow(GetType()); }
    bool IsAppWindow() const override { return WindowHelper::IsAppWindow(GetType()); }

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
        return 1; // 1 for root and screen
    }

    Ace::UIContent* GetUIContent() const override;
    WMError Destroy(uint32_t reason = 0) override;

    std::string GetClassType() const override { return "ScreenScene"; }

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> GetRSUIDirector() const override;
    std::shared_ptr<RSUIContext> GetRSUIContext() const override;

private:
    void RegisterInputEventListener();
    mutable std::mutex mutex_;
    std::unique_ptr<Ace::UIContent> uiContent_;
    float density_ = 1.0f;
    int32_t orientation_;
    DisplayId displayId_;
    WindowType type_ = WindowType::WINDOW_TYPE_SCENE_BOARD;
    std::string name_;
    std::function<void()> frameLayoutFinishCb_ = nullptr;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)> configurationUpdateCallback_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_SCREEN_SCENE_H
