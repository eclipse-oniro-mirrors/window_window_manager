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

#include <iremote_broker.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "session_manager_service.h"
#include "ability_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "mock_session_manager_service_interface.h"
#include "window_manager_hilog.h"
#include "session_manager/include/scene_session_manager_lite.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionManagerService" };
std::mutex g_instanceMutex;
SessionManagerService* g_sessionManagerService;
}

SessionManagerService::~SessionManagerService()
{
    if (g_sessionManagerService != nullptr) {
        g_sessionManagerService = nullptr;
    }
    WLOGFI("[WMSRecover] ~SessionManagerService");
}

SessionManagerService* SessionManagerService::GetInstance()
{
    if (g_sessionManagerService == nullptr) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        if (g_sessionManagerService == nullptr) {
            WLOGFI("[WMSRecover] new SessionManagerService");
            g_sessionManagerService = new SessionManagerService();
        }
    }
    return g_sessionManagerService;
}

void SessionManagerService::Init()
{
    AAFwk::AbilityManagerClient::GetInstance()->SetSessionManagerService(this->AsObject());
}

void SessionManagerService::NotifySceneBoardAvailable()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WLOGFE("[RECOVER] Failed to get system ability manager.");
        return;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
    if (!remoteObject) {
        WLOGFE("[RECOVER] Get window manager service failed, remote object is nullptr");
        return;
    }

    auto mockSessionManagerServiceProxy = iface_cast<IMockSessionManagerInterface>(remoteObject);
    if (!mockSessionManagerServiceProxy) {
        WLOGFE("[RECOVER] Get mock session manager service proxy failed, nullptr");
        return;
    }
    WLOGFI("[RECOVER] Get mock session manager ok");
    mockSessionManagerServiceProxy->NotifySceneBoardAvailable();
}

sptr<IRemoteObject> SessionManagerService::GetSceneSessionManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (sceneSessionManagerObj_) {
        return sceneSessionManagerObj_;
    }
    sceneSessionManagerObj_ = SceneSessionManager::GetInstance().AsObject();
    return sceneSessionManagerObj_;
}

sptr<IRemoteObject> SessionManagerService::GetSceneSessionManagerLite()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (sceneSessionManagerLiteObj_) {
        return sceneSessionManagerLiteObj_;
    }
    sceneSessionManagerLiteObj_ = SceneSessionManagerLite::GetInstance().AsObject();
    return sceneSessionManagerLiteObj_;
}
} // namesapce OHOS::Rosen
