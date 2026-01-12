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

#ifndef OHOS_WINDOW_ADAPTER_LITE_H
#define OHOS_WINDOW_ADAPTER_LITE_H

#include <shared_mutex>
#include <refbase.h>
#include <zidl/window_manager_agent_interface.h>

#include "singleton_delegator.h"
#include "window_property.h"
#include "wm_single_instance.h"
#include "zidl/window_manager_lite_interface.h"

namespace OHOS {
namespace Rosen {
class WMSDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    WMSDeathRecipient(const int32_t userId = INVALID_USER_ID);
    virtual void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;
private:
    int32_t userId_;
};

class WindowAdapterLite : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(WindowAdapterLite);
public:
    static WindowAdapterLite& GetInstance(const int32_t userId);

    using WMSConnectionChangedCallbackFunc = std::function<void(int32_t, int32_t, bool)>;
    virtual void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID);
    virtual void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                 std::vector<sptr<FocusChangeInfo>>& allFocusInfoList);
    virtual WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError CheckWindowId(int32_t windowId, int32_t& pid);
    virtual WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos);
    virtual WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease);
    virtual void ClearWindowAdapter();
    virtual WMError GetWindowModeType(WindowModeType& windowModeType);
    virtual WMError RaiseWindowToTop(int32_t persistentId);
    virtual WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo);
    virtual WMError UpdateAnimationSpeedWithPid(pid_t pid, float speed);
    virtual WMError GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo);
    WMError RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc);
    WMError UnregisterWMSConnectionChangedListener();
    virtual WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos);
    virtual WMError GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo);
    virtual WMError ClearMainSessions(const std::vector<int32_t>& persistentIds);
    virtual WMError ClearMainSessions(const std::vector<int32_t>& persistentIds, std::vector<int32_t>& clearFailedIds);
    virtual WMError GetWindowStyleType(WindowStyleType& windowStyleType);
    virtual WMError TerminateSessionByPersistentId(int32_t persistentId);
    virtual WMError CloseTargetFloatWindow(const std::string& bundleName);
    virtual WMError CloseTargetPiPWindow(const std::string& bundleName);
    virtual WMError GetCurrentPiPWindowInfo(std::string& bundleName);
    virtual WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos);
    virtual WMError SetGlobalDragResizeType(DragResizeType dragResizeType);
    virtual WMError GetGlobalDragResizeType(DragResizeType& dragResizeType);
    virtual WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType);
    virtual WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType);
    virtual WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy);
    virtual WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos);
    virtual WMError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    virtual WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
       std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap);

private:
    friend class sptr<WindowAdapterLite>;
    ~WindowAdapterLite() override;
    WindowAdapterLite(const int32_t userId = INVALID_USER_ID);

    static inline SingletonDelegator<WindowAdapterLite> delegator;
    bool InitSSMProxy();

    /*
     * Window Recover
     */
    uint32_t observedFlags_ = 0;
    uint32_t interestedFlags_ = 0;
    WMError RecoverWindowPropertyChangeFlag();

    /*
     * Multi user and multi screen
     */
    int32_t userId_;
    static std::unordered_map<int32_t, sptr<WindowAdapterLite>> windowAdapterLiteMap_;
    static std::mutex windowAdapterLiteMapMutex_;
    void OnUserSwitch();
    void ReregisterWindowManagerLiteAgent();

    sptr<IWindowManagerLite> GetWindowManagerServiceProxy() const;

    mutable std::mutex mutex_;
    sptr<IWindowManagerLite> windowManagerServiceProxy_ = nullptr;
    sptr<WMSDeathRecipient> wmsDeath_ = nullptr;
    bool isProxyValid_ = false;
    bool isRegisteredUserSwitchListener_ = false;
    // above guarded by mutex_

    std::mutex windowManagerLiteAgentMapMutex_;
    std::map<WindowManagerAgentType, std::set<sptr<IWindowManagerAgent>>> windowManagerLiteAgentMap_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_ADAPTER_H
