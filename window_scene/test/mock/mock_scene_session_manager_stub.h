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

#ifndef OHOS_ROSEN_MOCK_SCENE_SESSION_MANAGER_STUB_H
#define OHOS_ROSEN_MOCK_SCENE_SESSION_MANAGER_STUB_H

#include "zidl/scene_session_manager_stub.h"
#include <gmock/gmock.h>

namespace OHOS::Rosen {
class MockSceneSessionManagerStub : public SceneSessionManagerStub {
public:
    MOCK_METHOD(WSError, SetSessionLabel, (const sptr<IRemoteObject>& token, const std::string& label), (override));
    MOCK_METHOD(WSError, SetSessionIcon, (const sptr<IRemoteObject>& token,
        const std::shared_ptr<Media::PixelMap>& icon), (override));
    MOCK_METHOD(WSError, IsValidSessionIds, (const std::vector<int32_t>& sessionIds, std::vector<bool>& results),
        (override));
    MOCK_METHOD(WSError, PendingSessionToForeground, (const sptr<IRemoteObject>& token, int32_t windowMode),
        (override));
    MOCK_METHOD(WSError, PendingSessionToBackgroundForDelegator, (const sptr<IRemoteObject>& token,
        bool shouldBackToCaller, int32_t reason), (override));
    MOCK_METHOD(WSError, GetFocusSessionToken, (sptr<IRemoteObject>& token, DisplayId displayId), (override));
    MOCK_METHOD(WSError, GetFocusSessionElement, (AppExecFwk::ElementName& element, DisplayId displayId), (override));
    MOCK_METHOD(WSError, RegisterSessionListener, (const sptr<ISessionListener>& listener), (override));
    MOCK_METHOD(WSError, UnRegisterSessionListener, (const sptr<ISessionListener>& listener), (override));
    MOCK_METHOD(WSError, GetSessionInfos, (const std::string& deviceId, int32_t numMax,
        std::vector<SessionInfoBean>& sessionInfos), (override));
    MOCK_METHOD(WSError, GetSessionInfo, (const std::string& deviceId, int32_t persistentId,
        SessionInfoBean& sessionInfo), (override));
    MOCK_METHOD(WSError, GetSessionInfoByContinueSessionId, (const std::string& continueSessionId,
        SessionInfoBean& sessionInfo), (override));
    MOCK_METHOD(WSError, SetSessionContinueState, (const sptr<IRemoteObject>& token,
        const ContinueState& continueState), (override));
    MOCK_METHOD(WSError, TerminateSessionNew, (const sptr<AAFwk::SessionInfo> info,
        bool needStartCaller, bool isFromBroker), (override));
    MOCK_METHOD(WSError, GetSessionDumpInfo, (const std::vector<std::string>& params, std::string& info), (override));
    MOCK_METHOD(WSError, GetSessionSnapshot, (const std::string& deviceId,
        int32_t persistentId, SessionSnapshot& snapshot, bool isLowResolution), (override));
    MOCK_METHOD(WSError, ClearSession, (int32_t persistentId), (override));
    MOCK_METHOD(WSError, ClearAllSessions, (), (override));
    MOCK_METHOD(WSError, LockSession, (int32_t sessionId), (override));
    MOCK_METHOD(WSError, UnlockSession, (int32_t sessionId), (override));
    MOCK_METHOD(WSError, MoveSessionsToForeground,
        (const std::vector<std::int32_t>& sessionIds, int32_t topSessionId), (override));
    MOCK_METHOD(WSError, MoveSessionsToBackground,
        (const std::vector<std::int32_t>& sessionIds, std::vector<std::int32_t>& result), (override));
    MOCK_METHOD(WSError, UnregisterIAbilityManagerCollaborator, (int32_t type), (override));
    MOCK_METHOD(WMError, GetProcessSurfaceNodeIdByPersistentId, (const int32_t pid,
        const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds), (override));
    MOCK_METHOD(WMError, SkipSnapshotByUserIdAndBundleNames,
        (int32_t userId, const std::vector<std::string>& bundleNameList), (override));
    
    // IWindowManager overrides
    MOCK_METHOD(WMError, CreateWindow, (sptr<IWindow>& window, sptr<WindowProperty>& property,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, uint32_t& windowId, sptr<IRemoteObject> token), (override));
    MOCK_METHOD(WMError, AddWindow, (sptr<WindowProperty>& property), (override));
    MOCK_METHOD(WMError, RemoveWindow, (uint32_t windowId, bool isFromInnerkits), (override));
    MOCK_METHOD(WMError, DestroyWindow, (uint32_t windowId, bool onlySelf), (override));
    MOCK_METHOD(WMError, RequestFocus, (uint32_t windowId), (override));
    MOCK_METHOD(WMError, GetTopWindowId, (uint32_t mainWinId, uint32_t& topWinId), (override));
    MOCK_METHOD(WMError, GetParentMainWindowId, (int32_t windowId, int32_t& mainWindowId), (override));
    MOCK_METHOD(WMError, UpdateProperty,
        (sptr<WindowProperty>& windowProperty, PropertyChangeAction action, bool isAsyncTask), (override));
    MOCK_METHOD(WMError, RaiseToAppTop, (uint32_t windowId), (override));
    MOCK_METHOD(WMError, MinimizeByWindowId, (const std::vector<int32_t>& windowIds), (override));
    MOCK_METHOD(WSError, ShiftAppWindowFocus, (int32_t sourcePersistentId, int32_t targetPersistentId), (override));
    MOCK_METHOD(WSError, GetHostWindowRect, (int32_t hostWindowId, Rect& rect), (override));
    MOCK_METHOD(WMError, GetWindowModeType, (WindowModeType& windowModeType), (override));
    MOCK_METHOD(WMError, GetWindowIdsByCoordinate,
        (DisplayId displayId, int32_t windowNumber, int32_t x, int32_t y, std::vector<int32_t>& windowIds), (override));
    MOCK_METHOD(WMError, SetGlobalDragResizeType, (DragResizeType dragResizeType), (override));
    MOCK_METHOD(WMError, GetGlobalDragResizeType, (DragResizeType& dragResizeType), (override));
    MOCK_METHOD(WMError, SetAppDragResizeType,
        (const std::string& bundleName, DragResizeType dragResizeType), (override));
    MOCK_METHOD(WMError, ShiftAppWindowPointerEvent,
        (int32_t sourcePersistentId, int32_t targetPersistentId, int32_t fingerId), (override));
    MOCK_METHOD(WMError, SetStartWindowBackgroundColor, (const std::string& moduleName,
        const std::string& abilityName, uint32_t color, int32_t uid), (override));
    MOCK_METHOD(WSError, UseImplicitAnimation, (int32_t hostWindowId, bool useImplicit), (override));
    MOCK_METHOD(WMError, CreateUIEffectController, (const sptr<IUIEffectControllerClient>& controllerClient,
        sptr<IUIEffectController>& controller, int32_t& controllerId), (override));
    MOCK_METHOD(WMError, AddSessionBlackList, (const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags), (override));
    MOCK_METHOD(WMError, RemoveSessionBlackList, (const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags), (override));
    MOCK_METHOD(WMError, GetPiPSettingSwitchStatus, (bool& switchStatus), (override));
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_MOCK_SCENE_SESSION_MANAGER_STUB_H