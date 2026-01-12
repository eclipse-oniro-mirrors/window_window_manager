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

#ifndef OHOS_ROSEN_WINDOW_MANAGER_LITE_H
#define OHOS_ROSEN_WINDOW_MANAGER_LITE_H

#include <iremote_object.h>
#include <memory>
#include <mutex>
#include <refbase.h>
#include <vector>
#include "focus_change_info.h"
#include "window_drawing_content_info.h"
#include "window_manager.h"
#include "window_visibility_info.h"
#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
/**
 * @class WindowManagerLite
 *
 * @brief WindowManagerLite used to manage window.
 */
class WindowManagerLite : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(WindowManagerLite);
    friend class WindowManagerAgentLite;
    friend class WMSDeathRecipient;
    friend class SSMDeathRecipient;
public:
    static WindowManagerLite& GetInstance(const int32_t userId);
    static WMError RemoveInstanceByUserId(const int32_t userId);

    /**
     * @brief Register focus changed listener.
     *
     * @param listener IFocusChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener);

    /**
     * @brief Unregister focus changed listener.
     *
     * @param listener IFocusChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener);

    /**
     * @brief Register all display group info changed listener.
     *
     * @param listener IAllGroupInfoChangedListener.
     * @return WM_OK means register success, others mean register failure.
     */
    WMError RegisterAllGroupInfoChangedListener(const sptr<IAllGroupInfoChangedListener>& listener);

    /**
     * @brief Unregister all display group info changed listener.
     *
     * @param listener IAllGroupInfoChangedListener.
     * @return WM_OK means unregister success, others mean unregister failure.
     */
    WMError UnregisterAllGroupInfoChangedListener(const sptr<IAllGroupInfoChangedListener>& listener);

    /**
     * @brief Register visibility changed listener.
     *
     * @param listener IVisibilityChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener);

    /**
     * @brief Unregister visibility changed listener.
     *
     * @param listener IVisibilityChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener);

    /**
     * @brief Get visibility window info.
     *
     * @param infos Visible window infos
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const;

    /**
     * @brief update screen lock status.
     *
     * @param bundleName bundle name of app.
     * @param isRelease whether release the screen lock.
     * @return WM_OK means update success, others means update failed.
     */
    WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease);

    /**
     * @brief Get focus window.
     *
     * @param focusInfo Focus window info.
     * @return FocusChangeInfo object about focus window.
     */
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID);

    /**
     * @brief Get all group infomation.
     *
     * @param displayId2GroupIdMap display id to display group id map.
     * @param allFocusInfoList focus infomation in every display group.
     * @return void.
     */
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList);

    /**
     * @brief Register drawingcontent changed listener.
     *
     * @param listener IDrawingContentChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener);

    /**
     * @brief Unregister drawingcontent changed listener.
     *
     * @param listener IDrawingContentChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener);

    /**
     * @brief Register window mode listener.
     *
     * @param listener IWindowModeChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener);

    /**
     * @brief Unregister window mode listener.
     *
     * @param listener IWindowModeChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener);

    /**
     * @brief Register camera window changed listener.
     *
     * @param listener ICameraWindowChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterCameraWindowChangedListener(const sptr<ICameraWindowChangedListener>& listener);

    /**
     * @brief Unregister camera window changed listener.
     *
     * @param listener ICameraWindowChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterCameraWindowChangedListener(const sptr<ICameraWindowChangedListener>& listener);

    /**
     * @brief Get window mode type.
     *
     * @param void
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetWindowModeType(WindowModeType& windowModeType) const;

    /**
     * @brief Get top num main window info.
     *
     * @param topNum the num of top window
     * @param topNInfo the top num window infos
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo);

    /**
     * @brief Set the animation speed for a specific process.
     *
     * @param pid process id.
     * @param speed The animation speed.
     * @return WM_OK means set success, others means set failed.
     */
    WMError UpdateAnimationSpeedWithPid(pid_t pid, float speed);

    /**
     * @brief Get keyboard calling window information.
     *
     * @param callingWindowInfo calling window information
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo);

    /**
     * @brief Get all main window info.
     *
     * @param infos the all main window info.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) const;

    /**
     * @brief Get main window info by ability token
     *
     * @param abilityToken ability token
     * @param windowInfo main window info
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo);

    /**
     * @brief Clear a specified set of sessions.
     *
     * @param persistentIds a vector of session persistentId.
     * @return WM_OK means clear session success, others means clear failed.
     */
    WMError ClearMainSessions(const std::vector<int32_t>& persistentIds);

    /**
     * @brief Clear a specified set of sessions.
     *
     * @param persistentIds a vector of session persistentId.
     * @param clearFailedIds a vector of session persistentId which is clear failed.
     * @return WM_OK means clear session success, others means clear failed.
     */
    WMError ClearMainSessions(const std::vector<int32_t>& persistentIds, std::vector<int32_t>& clearFailedIds);

    /**
     * @brief raise window to top by windowId
     *
     * @param persistentId this window to raise
     * @return WM_OK if raise success
     */
    WMError RaiseWindowToTop(int32_t persistentId);

    /**
     * @brief Register WMS connection status changed listener.
     * @attention Callable only by u0 system user. A process only supports successful registration once.
     * When the foundation service restarts, you need to re-register the listener.
     * If you want to re-register, please call UnregisterWMSConnectionChangedListener first.
     *
     * @param listener IWMSConnectionChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWMSConnectionChangedListener(const sptr<IWMSConnectionChangedListener>& listener);

    /**
     * @brief Unregister WMS connection status changed listener.
     * @attention Callable only by u0 system user.
     *
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWMSConnectionChangedListener();

    /**
     * @brief Register WindowStyle changed listener.
     *
     * @param listener IWindowStyleChangedListener
     * @return WM_OK means register success, others means unregister failed.
     */
    WMError RegisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener);

    /**
     * @brief Unregister WindowStyle changed listener.
     *
     * @param listener IWindowStyleChangedListener
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener);

    /**
     * @brief Register a listener to detect display changes for the keyboard calling window.
     *
     * @param listener IKeyboardCallingWindowDisplayChangedListener
     * @return WM_OK means register success, others means unregister failed.
     */
    WMError RegisterCallingWindowDisplayChangedListener(
        const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener);

    /**
     * @brief Unregister the listener that detects display changes for the keyboard calling window.
     *
     * @param listener IKeyboardCallingWindowDisplayChangedListener
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterCallingWindowDisplayChangedListener(
        const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener);

    /**
     * @brief Get window style type.
     *
     * @param windowStyleType WindowType
     * @return @return WM_OK means get window style success, others means failed.
     */
    WindowStyleType GetWindowStyleType();

    /**
     * @brief Terminate session by persistentId and start caller.
     * @persistentId persistentId to be terminated.
     *
     * @return WM_OK means Terminate success, others means Terminate failed.
     */
    WMError TerminateSessionByPersistentId(int32_t persistentId);

    /**
     * @brief Close target float window.
     *
     * @param bundleName the target float window need to be closed.
     * @return WM_OK means Close success, others means Close failed.
     */
    WMError CloseTargetFloatWindow(const std::string& bundleName);

    /**
     * @brief Register listener for PiP window state changed.
     *
     * @param listener the PiP state changed listener.
     * @return WM_OK means Register success, others means Register failed.
     */
    WMError RegisterPiPStateChangedListener(const sptr<IPiPStateChangedListener>& listener);

    /**
     * @brief Unregister listener for PiP window state changed.
     *
     * @param listener the PiP state changed listener.
     * @return WM_OK means Unregister success, others means Unregister failed.
     */
    WMError UnregisterPiPStateChangedListener(const sptr<IPiPStateChangedListener>& listener);

    /**
     * @brief Close target PiP Window by bundleName.
     *
     * @param bundleName the target PiP Window need to be closed.
     * @return WM_OK means Close success, others means Close failed.
     */
    WMError CloseTargetPiPWindow(const std::string& bundleName);

    /**
     * @brief Get current show PiP Window info.
     *
     * @param bundleName the current PiP window bundleName.
     * @return WM_OK means Get success, others means Get failed.
     */
    WMError GetCurrentPiPWindowInfo(std::string& bundleName);

    /**
     * @brief Get accessibility window info.
     *
     * @param infos WindowInfos used for Accessibility.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const;

    /**
     * @brief Register window updated listener.
     *
     * @param listener IWindowUpdateListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener);

    /**
     * @brief Unregister window updated listener.
     *
     * @param listener IWindowUpdateListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener);

    /**
     * @brief notify window info change.
     *
     * @param flags mark the changed value.
     * @param windowInfoList the changed window info list.
     * @return WM_OK means notify success, others means notify failed.
     */
    void NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList);

    /**
     * @brief Register window info change callback.
     *
     * @param observedInfo Property which to observe.
     * @param listener Listener to observe window info.
     * @return WM_OK means register success, others means failed.
     */
    WMError RegisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    
    /**
     * @brief Unregister window info change callback.
     *
     * @param observedInfo Property which to observe.
     * @param listener Listener to observe window info.
     * @return WM_OK means unregister success, others means failed.
     */
    WMError UnregisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);

    /**
     * @brief Set global drag resize type.
     * this priority is highest.
     *
     * @param dragResizeType global drag resize type to set
     * @return WM_OK means set success, others means failed.
     */
    WMError SetGlobalDragResizeType(DragResizeType dragResizeType);

    /**
     * @brief Get global drag resize type.
     * if it is RESIZE_TYPE_UNDEFINED, return default value.
     *
     * @param dragResizeType global drag resize type to get
     * @return WM_OK means get success, others means failed.
     */
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType);

    /**
     * @brief Set drag resize type of specific app.
     * only when global value is RESIZE_TYPE_UNDEFINED, this value take effect.
     *
     * @param bundleName bundleName of specific app
     * @param dragResizeType drag resize type to set
     * @return WM_OK means set success, others means failed.
     */
    WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType);

    /**
     * @brief Get drag resize type of specific app.
     * effective order:
     *  1. global value
     *  2. app value
     *  3. default value
     *
     * @param bundleName bundleName of specific app
     * @param dragResizeType drag resize type to get
     * @return WM_OK means get success, others means failed.
     */
    WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType);

    /**
     * @brief Set drag key frame type of specific app.
     * effective order:
     *  1. resize when drag
     *  2. key frame
     *  3. default value
     *
     * @param bundleName bundleName of specific app
     * @param keyFramePolicy param of key frame
     * @return WM_OK means set success, others means failed.
     */
    WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy);

    /**
     * @brief List window info.
     *
     * @param windowInfoOption Option for selecting window info.
     * @param infos Window info.
     * @return WM_OK means get success, others means get failed.
     */
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos) const;

    /**
     * @brief Send pointer event for hover.
     *
     * @param pointerEvent The pointer event for hover.
     * @return WM_OK means send success, others means send failed.
     */
    WMError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    /**
     * @brief Get displayId by windowId.
     *
     * @param windowIds list of window ids that need to get screen ids
     * @param windowDisplayIdMap map of windows and displayIds
     * @return WM_OK means get success, others means failed.
     */
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap);

private:
    std::recursive_mutex mutex_;
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    std::unordered_map<WindowInfoKey, uint32_t> interestInfoMap_;

    /**
     * Multi user and multi screen
     */
    friend class sptr<WindowManagerLite>;
    WindowManagerLite(const int32_t userId = INVALID_USER_ID);
    ~WindowManagerLite() override;

    int32_t userId_;
    static std::unordered_map<int32_t, sptr<WindowManagerLite>> windowManagerLiteMap_;
    static std::mutex windowManagerLiteMapMutex_;

    void UpdateFocusStatus(uint32_t windowId, const sptr<IRemoteObject>& abilityToken, WindowType windowType,
        DisplayId displayId, bool focused) const;
    void UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) const;
    void UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd) const;
    void UpdateWindowVisibilityInfo(
        const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos) const;
    void UpdateWindowDrawingContentInfo(
        const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos) const;
    void UpdateWindowModeTypeInfo(WindowModeType type) const;
    void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing) const;
    void UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground) const;
    void OnRemoteDied();
    void OnWMSConnectionChanged(int32_t userId, int32_t screenId, bool isConnected) const;
    WMError NotifyWindowStyleChange(WindowStyleType type);
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
        WindowUpdateType type) const;
    WMError NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo);
    WMError ProcessRegisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    WMError ProcessUnregisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener);
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_MANAGER_LITE_H
