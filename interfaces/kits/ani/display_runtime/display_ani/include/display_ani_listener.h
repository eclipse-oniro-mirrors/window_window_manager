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

#ifndef OHOS_ANI_DISPLAY_LISTENER_H
#define OHOS_ANI_DISPLAY_LISTENER_H

#include <mutex>

#include "ani.h"
#include "display_manager.h"
#include "dm_common.h"
#include "event_handler.h"
#include "refbase.h"

namespace OHOS {
namespace Rosen {
class DisplayAniListener : public DisplayManager::IDisplayListener,
                          public DisplayManager::IPrivateWindowListener,
                          public DisplayManager::IFoldStatusListener,
                          public DisplayManager::IFoldAngleListener,
                          public DisplayManager::ICaptureStatusListener,
                          public DisplayManager::IDisplayModeListener,
                          public DisplayManager::IAvailableAreaListener,
                          public DisplayManager::IBrightnessInfoListener,
                          public DisplayManager::IDisplayAttributeListener {
public:
    DisplayAniListener(ani_env* env)
        : env_(env), weakRef_(wptr<DisplayAniListener> (this)) {}
    ~DisplayAniListener() override;
    void SetMainEventHandler();
    void AddCallback(const std::string& type, ani_ref callback);
    void RemoveAllCallback();
    void RemoveCallback(ani_env* env, const std::string& type, ani_ref callback);
    void OnCreate(DisplayId id) override;
    void OnDestroy(DisplayId id) override;
    void OnChange(DisplayId id) override;
    void OnPrivateWindow(bool hasPrivate) override;
    void OnFoldStatusChanged(FoldStatus foldStatus) override;
    void OnFoldAngleChanged(std::vector<float> foldAngles) override;
    void OnCaptureStatusChanged(bool isCapture) override;
    void OnDisplayModeChanged(FoldDisplayMode displayMode) override;
    void OnAvailableAreaChanged(DMRect area) override;
    ani_status CallAniMethodVoid(ani_object object, const char* cls,
        const char* method, const char* signature, ...);
    void OnBrightnessInfoChanged(DisplayId id, const ScreenBrightnessInfo& info) override;
    void OnAttributeChange(DisplayId displayId, const std::vector<std::string>& attribuutes) override;

private:
    void RemoveDuplicateMethods(ani_env* env, std::vector<ani_ref>& callbacks);
    void ProcessAttributeCallbacks(ani_env* env, const std::vector<std::string>& attributes, DisplayId displayId);
    ani_env* env_;
    std::mutex aniCallbackMtx_;
    std::map<std::string, std::vector<ani_ref>> aniCallback_;
    wptr<DisplayAniListener> weakRef_  = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
};
const std::string ANI_EVENT_ADD = "add";
const std::string ANI_EVENT_REMOVE = "remove";
const std::string ANI_EVENT_CHANGE = "change";
const std::string ANI_EVENT_PRIVATE_MODE_CHANGE = "privateModeChange";
const std::string ANI_EVENT_FOLD_STATUS_CHANGED = "foldStatusChange";
const std::string ANI_EVENT_FOLD_ANGLE_CHANGED = "foldAngleChange";
const std::string ANI_EVENT_CAPTURE_STATUS_CHANGED = "captureStatusChange";
const std::string ANI_EVENT_DISPLAY_MODE_CHANGED = "foldDisplayModeChange";
const std::string ANI_EVENT_AVAILABLE_AREA_CHANGED = "availableAreaChange";
const std::string ANI_EVENT_BRIGHTNESS_INFO_CHANGED = "brightnessInfoChange";
}  // namespace Rosen
}  // namespace OHOS
#endif