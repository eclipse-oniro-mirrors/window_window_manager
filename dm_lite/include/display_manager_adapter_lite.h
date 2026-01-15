/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at,
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software,
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_LITE_H
#define FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_LITE_H

#include <map>
#include <mutex>

#include "display_lite.h"
#include "display_manager_lite_proxy.h"
#include "dm_common.h"
#include "singleton_delegator.h"
#include "zidl/idisplay_manager_agent.h"

namespace OHOS::Rosen {
class BaseAdapterLite {
public:
    virtual ~BaseAdapterLite();
    virtual DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    virtual DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    virtual void Clear();
    virtual bool IsScreenLessDevice();
protected:
    bool InitDMSProxy();
    std::recursive_mutex mutex_;
    sptr<DisplayManagerLiteProxy> displayManagerServiceProxy_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> dmsDeath_ = nullptr;
    bool isProxyValid_ { false };
};

class DMSDeathRecipientLite : public IRemoteObject::DeathRecipient {
public:
    explicit DMSDeathRecipientLite(BaseAdapterLite& adapter);
    virtual void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;
private:
    BaseAdapterLite& adapter_;
};

class DisplayManagerAdapterLite : public BaseAdapterLite {
WM_DECLARE_SINGLE_INSTANCE(DisplayManagerAdapterLite);
public:
    virtual sptr<DisplayInfo> GetDefaultDisplayInfo(int32_t userId = CONCURRENT_USER_ID_DEFAULT);
    virtual std::vector<DisplayId> GetAllDisplayIds(int32_t userId = CONCURRENT_USER_ID_DEFAULT);
    virtual bool IsFoldable();
    virtual FoldStatus GetFoldStatus();
    virtual FoldDisplayMode GetFoldDisplayMode();
    virtual void SetFoldDisplayMode(const FoldDisplayMode);
    virtual void SetFoldDisplayModeAsync(const FoldDisplayMode);
    virtual sptr<DisplayInfo> GetDisplayInfo(DisplayId displayId);
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId);
    virtual VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId);
    virtual bool GetKeyboardState();
    /*
     * used by powermgr
     */
    virtual bool WakeUpBegin(PowerStateChangeReason reason);
    virtual bool WakeUpEnd();
    virtual bool SuspendBegin(PowerStateChangeReason reason);
    virtual bool SuspendEnd();
    virtual ScreenId GetInternalScreenId();
    virtual bool SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);
    virtual bool SetDisplayState(DisplayState state);
    virtual DisplayState GetDisplayState(DisplayId displayId);
    virtual bool TryToCancelScreenOff();
    virtual bool SetScreenBrightness(uint64_t screenId, uint32_t level);
    virtual uint32_t GetScreenBrightness(uint64_t screenId);
    virtual DMError SetSystemKeyboardStatus(bool isTpKeyboardOn = false);
    virtual DMError IsOnboardDisplay(DisplayId displayId, bool& isOnboardDisplay);
private:
    static inline SingletonDelegator<DisplayManagerAdapterLite> delegator;
};

class ScreenManagerAdapterLite : public BaseAdapterLite {
WM_DECLARE_SINGLE_INSTANCE(ScreenManagerAdapterLite);
public:
    /*
     * used by powermgr
     */
    virtual DMError GetPhysicalScreenIds(std::vector<ScreenId>& screenIds);
    virtual bool SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);
    virtual bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason);
    virtual ScreenPowerState GetScreenPower(ScreenId dmsScreenId);
    virtual ScreenPowerState GetScreenPower();
    virtual void SyncScreenPowerState(ScreenPowerState state);
    virtual sptr<ScreenInfo> GetScreenInfo(ScreenId screenId);
    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos);
    virtual bool SynchronizePowerStatus(ScreenPowerState state);
    virtual DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio);
private:
    static inline SingletonDelegator<ScreenManagerAdapterLite> delegator;
};

} // namespace OHOS::Rosen
#endif // FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_LITE_H
