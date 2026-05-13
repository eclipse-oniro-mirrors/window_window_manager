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

#include "marshalling_helper.h"
#include "message_option.h"
#include "message_parcel.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "session/host/include/session.h"
#include "sessionstublayout_fuzzer.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SessionStubLayoutTest(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SET_SYSTEM_DRAG_ENABLE),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_CLIENT_RECT),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_GET_GLOBAL_SCALED_RECT),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_SET_FOLLOW_PARENT_LAYOUT_ENABLED),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(
        static_cast<uint32_t>(Rosen::SessionInterfaceCode::TRANS_ID_NOTIFY_RELATED_WINDOWS_LIMITS_CHANGED),
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
    info.abilityName_ = "stubLayoutFuzzTest";
    info.bundleName_ = "stubLayoutFuzzTest";
    sptr<Session> sessionStub = new (std::nothrow) Session(info);
    if (sessionStub == nullptr) {
        return false;
    }

    SessionStubLayoutTest(sessionStub, parcel);

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