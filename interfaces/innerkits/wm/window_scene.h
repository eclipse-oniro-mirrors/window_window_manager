/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNERKITS_WINDOW_SCENE_H
#define INTERFACES_INNERKITS_WINDOW_SCENE_H

#include <mutex>

#include "window.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS::AppExecFwk {
class Configuration;
class Ability;
class ElementName;
}

namespace OHOS {
namespace Rosen {
class WindowScene : public RefBase {
public:
    /**
     * Default constructor used to create an empty WindowScene instance.
     */
    WindowScene() = default;

    /**
     * Default deconstructor used to deconstruct.
     *
     */
    ~WindowScene();

    /**
     * Init a WindowScene instance based on the parameters displayId, context, listener and option.
     *
     * @param displayId the id of current display
     * @param context current ability context
     * @param listener the life cycle listener of the window
     * @param option the settings for window, such as WindowType, width, height, etc
     * @return the error code of window
     */
    WMError Init(DisplayId displayId, const std::shared_ptr<AbilityRuntime::Context>& context,
        sptr<IWindowLifeCycle>& listener, sptr<WindowOption> option = nullptr);

    /**
     * Init a WindowScene instance based on the parameters displayId, context, listener and option.
     *
     * @param displayId the id of current display
     * @param context current ability context
     * @param listener the life cycle listener of the window
     * @param option the settings for window, such as WindowType, width, height, etc
     * @param iSession session token of window session
     * @param identityToken identity token of sceneSession
     * @return the error code of window
     */
    WMError Init(DisplayId displayId, const std::shared_ptr<AbilityRuntime::Context>& context,
        sptr<IWindowLifeCycle>& listener, sptr<WindowOption> option, const sptr<IRemoteObject>& iSession,
        const std::string& identityToken = "", bool isModuleAbilityHookEnd = false);

    /**
     * Create a window instance based on the parameters windowName and option.
     *
     * @param windowName the id of this window
     * @param option the settings for window, such as WindowType, width, height, etc.
     * @return the shared pointer of window
     */
    sptr<Window> CreateWindow(const std::string& windowName, sptr<WindowOption>& option) const;

    /**
     * Get shared pointer of main window.
     * Locks mainWindowMutex_
     *
     * @return the shared pointer of window
     */
    sptr<Window> GetMainWindow() const;

    /**
     * Get a set of sub window.
     *
     * @return a set of sub window
     */
    std::vector<sptr<Window>> GetSubWindow();

    /**
     * window go foreground.
     *
     * @param reason the reason of window to go to foreground, default 0.
     * @return the error code of window
     */
    WMError GoForeground(uint32_t reason = 0);

    /**
     * Window go background.
     *
     * @param reason the reason of window to go to background, default 0.
     * @return the error code of window
     */
    WMError GoBackground(uint32_t reason = 0);

    /**
     * Window go distroy.
     *
     * @param reason the reason of window to go to destroy, default 0.
     * @return the error code of window
     */
    WMError GoDestroy(uint32_t reason = 0);

    /**
     * Hook Window go destroy.
     *
     * @return the error code of window
     */
    WMError GoDestroyHookWindow();

    /**
     * Window go resume.
     *
     * @return the error code of window
     */
    WMError GoResume();

    /**
     * Window go pause.
     *
     * @return the error code of window
     */
    WMError GoPause();

    /**
     * Window handle new want.
     *
     * @param want ability want.
     * @return the error code of window
     */
    WMError OnNewWant(const AAFwk::Want& want);

    /**
     * Request to get the focus.
     *
     * @return the error code of window
     */
    WMError RequestFocus() const;

    /**
     * Update ability configuration.
     *
     * @param configuration the configuration of ability
     */
    void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration);

    /**
     * @brief Update ability configuration for specified window.
     *
     * @param configuration the configuration of ability
     * @param resourceManager the resource manager
     */
    void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager);

    /**
     * Set main window system bar property
     *
     * @param type the type of window
     * @param property the property of system bar
     * @return the error code of window
     */
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) const;

    /**
     * Get content info of main window.
     *
     * @return content info of main window
     */
    std::string GetContentInfo(BackupAndRestoreType type = BackupAndRestoreType::CONTINUATION) const;

    /**
     * @brief Handle and notify memory.
     *
     * @param level memory level
     * @return the error code of window
     */
    WMError NotifyMemoryLevel(int32_t level);

    /**
     * @brief Set the navDestinationInfo of atomicService to arkui.
     *
     * @param navDestinationInfo navDestinationInfo in atomicService hap
     * @return the error code of window
     */
    WMError SetNavDestinationInfo(const std::string& navDestinationInfo);

    /**
     * @brief Set the bundleName, moduleName and abilityName of the hooked window
     *
     * @param elementName includes bundleName, moduleName and abilityName
     * @return the error code of window
     */
    WMError SetHookedWindowElementInfo(const AppExecFwk::ElementName& elementName);

public:
    static const DisplayId DEFAULT_DISPLAY_ID = 0;

private:
    void OnLastStrongRef(const void *) override;

private:
    mutable std::mutex mainWindowMutex_;
    sptr<Window> mainWindow_ = nullptr;
    // Above guarded by mainWindowMutex_

    uint32_t mainWindowId_ = 0;
};

} // namespace Rosen
} // namespace OHOS
#endif // INTERFACES_INNERKITS_WINDOW_SCENE_H
