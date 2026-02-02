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

#include "scenesessionmgrlstubuec_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "marshalling_helper.h"
#include "scene_session_manager_lite.h"
#include "scene_session_manager_lite_stub.h"
#include "scene_session_manager_lite_interface.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SceneSessionMgrLsStubFuzzer01(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_IS_FOCUS_WINDOW_PARENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_SESSION_SNAPSHOT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_ALL_GROUP_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_SET_GLOBAL_DRAG_RESIZE_TYPE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_SET_APP_DRAG_RESIZE_TYPE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_APP_DRAG_RESIZE_TYPE), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_SET_APP_KEY_FRAME_POLICY), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UI_EXTENSION_CREATION_CHECK), parcel, reply, option);
}

void SceneSessionMgrLsStubFuzzer02(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_SESSION_SCREEN_LOCK), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_CLOSE_TARGET_PIP_WINDOW), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_CURRENT_PIP_WINDOW_INFO), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UNSET_PIP_ENABLED_BY_SCREENID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_PIP_CHG_LISTENER), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_PIP_CHG_LISTENER), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_DISPLAYID_BY_WINDOWID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_PARENT_WINDOW_ID), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_SET_SESSION_ICON_FOR_THIRD_PARTY), parcel, reply, option);
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(static_cast<uint32_t>(ISceneSessionManagerLite::
        SceneSessionManagerLiteMessage::TRANS_ID_GET_MAIN_WINDOW_INFO_BY_TOKEN), parcel, reply, option);
}

void SceneSessionMgrLsUecTestCode(MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_UI_EXTENSION_CREATION_CHECK),
        parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    
    MessageParcel parcel;

    parcel.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    SceneSessionMgrLsUecTestCode(parcel);
    SceneSessionMgrLsStubFuzzer01(parcel);
    SceneSessionMgrLsStubFuzzer02(parcel);
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