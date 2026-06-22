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

#include "pictureinpicture_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "marshalling_helper.h"
#include "window_scene_session_impl.h"
#include "scene_session_manager_lite.h"
#include "scene_session_manager_lite_stub.h"
#include "scene_session_manager_lite_interface.h"
#include "scene_session_manager_interface.h"
#include "session_ipc_interface_code.h"
#include "session_stub.h"
#include "session.h"
#include "session_manager.h"
#include "session_stage_ipc_interface_code.h"
#include "session_stage_stub.h"
#include "window_option.h"
#include "session_proxy.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void PIPSceneSessionMgrStubTest(MessageParcel& parcel, MessageParcel& reply, MessageOption& option)
{
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_PIP_SWITCH_STATUS), parcel,
        reply, option);
    return;
}

bool DoSomethingInterestingWithMyAPI1(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    PIPSceneSessionMgrStubTest(parcel, reply, option);
    return true;
}

void PIPSessionTest(sptr<Session> sessionStub, MessageParcel& parcel, MessageParcel& reply, MessageOption& option)
{
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FRAME_LAYOUT_FINISH), parcel,
                                 reply, option);

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_CONTROL_STATUS),
                                 parcel, reply, option);

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_TEMPLATE_INFO), parcel,
                                 reply, option);

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_AUTOSTART_PIP), parcel, reply,
                                 option);

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_PIP_WINDOW_PREPARE_CLOSE),
                                 parcel, reply, option);

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_RECT), parcel, reply,
                                 option);

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_PIP_PARENT_WINDOWID), parcel,
                                 reply, option);
    return;
}

bool DoSomethingInterestingWithMyAPI2(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(SessionStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    SessionInfo info;
    info.abilityName_ = "pictureInPictureTestFuzzTest";
    info.bundleName_ = "pictureInPictureTestFuzzTest";
    sptr<Session> sessionStub = new (std::nothrow) Session(info);
    if (sessionStub == nullptr) {
        return false;
    }

    PIPSessionTest(sessionStub, parcel, reply, option);
    return true;
}

void PIPSessionStageTest(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel, MessageParcel& reply,
                         MessageOption& option)
{
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_PIPSIZE_CHANGE), parcel,
                               reply, option);

    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_CONTROL_EVENT), parcel,
                               reply, option);
    
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_CLOSE_EXIST_PIP_WINDOW),
                               parcel, reply, option);

    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_ACTION_EVENT), parcel,
                               reply, option);
    return;
}

bool DoSomethingInterestingWithMyAPI3(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    sptr<WindowOption> windowOption = new (std::nothrow) WindowOption();
    if (windowOption == nullptr) {
        return false;
    }
    sptr<WindowSessionImpl> stageStub = new (std::nothrow) WindowSessionImpl(windowOption);
    if (stageStub == nullptr) {
        return false;
    }

    PIPSessionStageTest(stageStub, parcel, reply, option);
    return true;
}

void PIPSceneSessionMgrLiteStubTest(MessageParcel& parcel, MessageParcel& reply, MessageOption& option)
{
    parcel.RewindRead(0);
    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_CLOSE_TARGET_FLOAT_WINDOW),
        parcel, reply, option);

    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_CLOSE_TARGET_PIP_WINDOW),
        parcel, reply, option);

    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_GET_CURRENT_PIP_WINDOW_INFO),
        parcel, reply, option);

    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_SET_PIP_ENABLED_BY_SCREENID),
        parcel, reply, option);

    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_UNSET_PIP_ENABLED_BY_SCREENID),
        parcel, reply, option);

    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_PIP_CHG_LISTENER),
        parcel, reply, option);

    SceneSessionManagerLite::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(
            ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_PIP_CHG_LISTENER),
        parcel, reply, option);
    return;
}

bool DoSomethingInterestingWithMyAPI4(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    PIPSceneSessionMgrLiteStubTest(parcel, reply, option);
    return true;
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI1(data, size);
    OHOS::DoSomethingInterestingWithMyAPI2(data, size);
    OHOS::DoSomethingInterestingWithMyAPI3(data, size);
    OHOS::DoSomethingInterestingWithMyAPI4(data, size);
    return 0;
}