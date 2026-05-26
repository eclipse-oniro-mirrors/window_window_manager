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

#include "scenesessionmgrstubattribute_fuzzer.h"

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

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_GLOBAL_WINDOW_MODE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_TOP_NAV_DEST_NAME), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_LAYOUT_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_LIST_WINDOW_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_REMOVE_SKIP_SELF_ON_VIRTUAL_SCREEN), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_ADD_SKIP_SELF_ON_VIRTUAL_SCREEN), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_SCREEN_LOCK), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_GET_SESSION_INFO_WITH_DISPLAY), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_SET_SCREEN_WATERMARK_IMAGE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_CLEAN_SCREEN_WATERMARK_IMAGE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManager::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManager::
        SceneSessionManagerMessage::TRANS_ID_RECOVER_SCREEN_WATERMARK_IMAGE), parcel, reply, option);
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