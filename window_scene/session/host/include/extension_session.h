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

#ifndef OHOS_ROSEN_WINDOW_SCENE_EXTENSION_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_EXTENSION_SESSION_H
#include <future>

#include "key_event.h"
#include "want.h"

#include "extension_data_handler.h"
#include "session/host/include/session.h"

namespace OHOS::Rosen {
    bool IsExtensionSessionInvalid(int32_t persistentId);

class WindowEventChannelListener : public IRemoteStub<IWindowEventChannelListener> {
public:
    explicit WindowEventChannelListener() = default;
    void SetTransferKeyEventForConsumedParams(int32_t keyEventId, bool isPreImeEvent,
        const std::shared_ptr<std::promise<bool>>& isConsumedPromise, const std::shared_ptr<WSError>& retCode);
    void ResetTransferKeyEventForConsumedParams();
    void ResetTransferKeyEventForConsumedParams(bool isConsumed, WSError retCode);
    void OnTransferKeyEventForConsumed(int32_t keyEventId, bool isPreImeEvent,
                                       bool isConsumed, WSError retCode) override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    std::mutex transferKeyEventForConsumedMutex_;
    int32_t keyEventId_ = 0;
    bool isPreImeEvent_ = false;
    std::shared_ptr<std::promise<bool>> isConsumedPromise_ = nullptr;
    std::shared_ptr<WSError> retCode_ = nullptr;
};

class ChannelDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit ChannelDeathRecipient(const sptr<WindowEventChannelListener>& listener): listener_(listener) {}
    virtual void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;
    
private:
    sptr<WindowEventChannelListener> listener_ = nullptr;
};

using NotifyTransferAbilityResultFunc = std::function<void(uint32_t resultCode, const AAFwk::Want& want)>;
using NotifyTransferExtensionDataFunc = std::function<void(const AAFwk::WantParams& wantParams)>;
using NotifyRemoteReadyFunc = std::function<void()>;
using NotifySyncOnFunc = std::function<void()>;
using NotifyAsyncOnFunc = std::function<void()>;
using NotifyGetAvoidAreaByTypeFunc = std::function<AvoidArea(AvoidAreaType type, int32_t apiVersion)>;
using NotifyBindModalFunc = std::function<void()>;
using NotifyExtensionEventFunc = std::function<void(uint32_t notifyEvent)>;
using GetStatusBarHeightFunc = std::function<int32_t()>;

class ExtensionSession : public Session {
public:
    struct ExtensionSessionEventCallback : public RefBase {
        NotifyTransferAbilityResultFunc transferAbilityResultFunc_;
        NotifyTransferExtensionDataFunc transferExtensionDataFunc_;
        NotifyRemoteReadyFunc notifyRemoteReadyFunc_;
        NotifySyncOnFunc notifySyncOnFunc_;
        NotifyAsyncOnFunc notifyAsyncOnFunc_;
        NotifyGetAvoidAreaByTypeFunc notifyGetAvoidAreaByTypeFunc_;
        NotifyBindModalFunc notifyBindModalFunc_;
        NotifyExtensionEventFunc notifyExtensionEventFunc_;
        GetStatusBarHeightFunc getStatusBarHeightFunc_;
    };

    explicit ExtensionSession(const SessionInfo& info);
    virtual ~ExtensionSession();
    SessionType GetSessionType() const override
    {
        return SessionType::ExtensionSession;
    }
    std::shared_ptr<IDataHandler> GetExtensionDataHandler() const;
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler,
        const std::shared_ptr<AppExecFwk::EventHandler>& exportHandler) override;
    WSError Connect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
        sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
        const std::string& identityToken = "") override;
    WSError ConnectInner(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
        sptr<WindowSessionProperty> property, sptr<IRemoteObject> token, int32_t pid, int32_t uid,
        const std::string& identityToken = "") override;

    AvoidArea GetAvoidAreaByType(AvoidAreaType type, const WSRect& rect = WSRect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) override;
    int32_t GetStatusBarHeight() override;

    WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;
    WSError TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want) override;
    int32_t TransferExtensionData(const AAFwk::WantParams& wantParams) override;
    WSError TransferComponentData(const AAFwk::WantParams& wantParams);
    WSErrorCode TransferComponentDataSync(const AAFwk::WantParams& wantParams,
                                          AAFwk::WantParams& reWantParams);
    WSError TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) override;
    WSError TransferAccessibilityHoverEvent(float pointX, float pointY, int32_t sourceType, int32_t eventType,
        int64_t timeMs);
    WSError TransferAccessibilityChildTreeRegister(uint32_t windowId, int32_t treeId, int64_t accessibilityId);
    WSError TransferAccessibilityChildTreeUnregister();
    WSError TransferAccessibilityDumpChildInfo(const std::vector<std::string>& params, std::vector<std::string>& info);
    void NotifySyncOn() override;
    void NotifyAsyncOn() override;
    WSError NotifyDensityFollowHost(bool isFollowHost, float densityValue = 1.0f);
    WSError UpdateSessionViewportConfig(const SessionViewportConfig& config);
    void TriggerBindModalUIExtension() override;
    void RegisterExtensionSessionEventCallback(const sptr<ExtensionSessionEventCallback>& extSessionEventCallback);
    WSError TransferKeyEventForConsumed(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool& isTimeOut, bool isPreImeEvent = false);
    WSError TransferKeyEventAsync(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool isPreImeEvent = false);
    sptr<ExtensionSessionEventCallback> GetExtensionSessionEventCallback();
    WSError Background(bool isFromClient = false, const std::string& identityToken = "") override;
    void NotifyExtensionEventAsync(uint32_t notifyEvent) override;
    WSError NotifyDumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info);
    WSError SendExtensionData(MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void SetIsTransparentUIExtension(bool isTransparentUIExtension)
    {
        isTransparentUIExtension_ = isTransparentUIExtension;
    }
    bool IsTransparentUIExtension() const { return isTransparentUIExtension_; }

private:
    sptr<ExtensionSessionEventCallback> extSessionEventCallback_ = nullptr;
    bool isFirstTriggerBindModal_ = true;
    sptr<ChannelDeathRecipient> channelDeath_ = nullptr;
    sptr<WindowEventChannelListener> channelListener_ = nullptr;
    std::shared_ptr<Extension::DataHandler> dataHandler_;
    bool isTransparentUIExtension_ = false;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_EXTENSION_SESSION_H
