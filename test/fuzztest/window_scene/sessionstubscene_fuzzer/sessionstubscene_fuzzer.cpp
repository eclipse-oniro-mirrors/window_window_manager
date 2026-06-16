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

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "marshalling_helper.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "session/host/include/session.h"
#include "sessionstubscene_fuzzer.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SessionStubTestUpdateCode(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_CUSTOM_ANIMATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_RECTCHANGE_LISTENER_REGISTERED),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_RECT),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_CUSTOM_ANIMATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_PROPERTY),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SEND_POINTEREVENT_FOR_MOVE_DRAG),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_RAISE_TO_APP_TOP),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_RAISE_ABOVE_TARGET),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_RAISE_MAIN_WINDOW_ABOVE_TARGET),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_RAISE_APP_MAIN_WINDOW),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_NEED_AVOID),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_ADJUST_KEYBOARD_LAYOUT),
        parcel, reply, option);
    return;
}

void SessionStubTestSetCode(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SET_MAXIMIZE_MODE),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SET_LANDSCAPE_MULTI_WINDOW),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SET_CALLING_SESSION_ID),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SET_CUSTOM_DECOR_HEIGHT),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_MARK_PROCESSED),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SESSION_EVENT),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SYNC_SESSION_EVENT),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_BACKPRESSED),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_GET_AVOID_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_GET_MAXIMIZE_MODE),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_PROCESS_POINT_DOWN_SESSION),
        parcel, reply, option);
    return;
}

void SessionStubTestCompatibleMode(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_GET_SELECT_MODE),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_NOTIFY_SPLIT_RATIO_CHANGED),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_NOTIFY_PAGE_ENABLE),
        parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    
    MessageParcel parcel;

    parcel.WriteInterfaceToken(SessionStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    SessionInfo info;
    info.abilityName_ = "stubConnectFuzzTest";
    info.bundleName_ = "stubConnectFuzzTest";
    sptr<Session> sessionStub = new (std::nothrow) Session(info);
    if (sessionStub == nullptr) {
        return false;
    }

    SessionStubTestUpdateCode(sessionStub, parcel);
    SessionStubTestSetCode(sessionStub, parcel);
    SessionStubTestCompatibleMode(sessionStub, parcel);
    return true;
}
} // namespace.OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}