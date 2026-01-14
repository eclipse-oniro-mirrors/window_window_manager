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

#include "scenesessionmgrstubcreate_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "marshalling_helper.h"
#include "scene_session_manager.h"
#include "scene_session_manager_stub.h"
#include "scene_session_manager_interface.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SceneSessionMgrCreateTestCode(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_CREATE_AND_CONNECT_SPECIFIC_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_REQUEST_FOCUS), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_TERMINATE_SESSION_NEW), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_RECONNECT_SCENE_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_CONNECT_SPECIFIC_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_ADD_EXTENSION_WINDOW_STAGE_TO_SCB), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_ADD_OR_REMOVE_SECURE_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION_WITH_DETACH_CALLBACK),
        parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_BIND_DIALOG_TARGET), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_CLEAR_SESSION), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_CLEAR_ALL_SESSIONS), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_ANIMATE_TO_WINDOW), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_CREATE_UI_EFFECT_CONTROLLER), parcel, reply, option);
}

void SceneSessionStubFuzzer01(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO_BY_ABILITY_TOKEN), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GLOBAL_COORDINATE_TO_RELATIVE_COORDINATE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_ROOT_UI_CONTENT_REMOTE_OBJ), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_OCCLUSION_STATE_LISTENER), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_ALL_MAIN_WINDOW_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_MAIN_WINDOW_SNAPSHOT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_APP_WATERMARK_IMAGE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_RECOVER_APP_WATERMARK_IMAGE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_FREE_MULTI_WINDOW_ENABLE_STATE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_PROCESS_SURFACENODEID_BY_PERSISTENTID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_PROCESS_WATERMARK), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH), parcel, reply, option);    
}

void SceneSessionStubFuzzer02(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_IS_FREE_MULTI_WINDOW), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_DISPLAYID_BY_WINDOWID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_APP_DRAG_RESIZE_TYPE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_START_WINDOW_BACKGROUND_COLOR), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_IMAGE_FOR_RECENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_IMAGE_FOR_RECENT_PIXELMAP), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_REMOVE_IMAGE_FOR_RECENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_ADD_SESSION_BLACK_LIST), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_REMOVE_SESSION_BLACK_LIST), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_PIP_SWITCH_STATUS), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_PIP_IS_PIP_ENABLED), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG), parcel, reply, option);
}

void SceneSessionStubFuzzer03(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_MINIMIZE_ALL_WINDOW), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_SPECIFIC_WINDOW_ZINDEX), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SUPPORT_ROTATION_REGISTERED), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_RESET_SPECIFIC_WINDOW_ZINDEX), parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    
    MessageParcel parcel;

    parcel.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    SceneSessionMgrCreateTestCode(parcel);
    SceneSessionStubFuzzer01(parcel);
    SceneSessionStubFuzzer02(parcel);
    SceneSessionStubFuzzer03(parcel);
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