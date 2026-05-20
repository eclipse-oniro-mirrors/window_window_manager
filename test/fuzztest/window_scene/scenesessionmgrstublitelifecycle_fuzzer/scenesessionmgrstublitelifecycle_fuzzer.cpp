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

#include "scenesessionmgrstublitelifecycle_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "marshalling_helper.h"
#include "scene_session_manager_stub.h"
#include "scene_session_manager_lite.h"
#include "scene_session_manager_lite_stub.h"
#include "scene_session_manager_lite_interface.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SceneSessionMgrLiteLifecycleIpcTest(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_ALL_MAIN_WINDOW_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_CLEAR_MAIN_SESSIONS), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_COLLABORATOR), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_COLLABORATOR), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_TERMINATE_SESSION_BY_PERSISTENT_ID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_MAIN_WINDOW_STATES_BY_PID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_NOTIFY_APP_USE_CONTROL_LIST), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_MINIMIZE_MAIN_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_LOCK_SESSION_BY_ABILITY_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_HAS_FLOAT_FOREGROUND), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_IDS), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_BUNDLES),
        parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_APP_INSTANCE),
        parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_ANIMATION_SPEED_WITH_PID), parcel, reply, option);
}

void SceneSessionMgrLiteLifecycleIpcSecTest(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_SESSION_LIFECYCLE_LISTENER), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_RECENT_MAIN_SESSION_INFO_LIST), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_CREATE_NEW_INSTANCE_KEY), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_ROUTER_STACK_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REMOVE_INSTANCE_KEY), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_TRANSFER_SESSION_TO_TARGET_SCREEN), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_KIOSK_APP_LIST), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_ENTER_KIOSK_MODE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_EXIT_KIOSK_MODE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND_BY_PERSISTENTID), parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    MessageParcel parcel;
    parcel.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor());
    parcel.WriteBuffer(data, size);
    SceneSessionMgrLiteLifecycleIpcTest(parcel);
    SceneSessionMgrLiteLifecycleIpcSecTest(parcel);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
