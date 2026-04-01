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
#include "sessionstubnorthbound_fuzzer.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SessionStubNorthBoundTest(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_APP_REMOVE_STARTING_WINDOW), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_TITLE_AND_DOCK_HOVER_SHOW_CHANGE), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_RESTORE_MAIN_WINDOW), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SUB_MODAL_TYPE_CHANGE), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_MAIN_MODAL_TYPE_CHANGE), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_SUPPORT_WINDOW_MODES), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_WINDOW_CORNER_RADIUS), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_START_MOVING_WITH_COORDINATE), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_FLAG), parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_WINDOW_RECT_AUTO_SAVE), parcel, reply, option);
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
    info.abilityName_ = "stubNorthBoundFuzzTest";
    info.bundleName_ = "stubNorthBoundFuzzTest";
    sptr<Session> sessionStub = new (std::nothrow) Session(info);
    if (sessionStub == nullptr) {
        return false;
    }

    SessionStubNorthBoundTest(sessionStub, parcel);

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