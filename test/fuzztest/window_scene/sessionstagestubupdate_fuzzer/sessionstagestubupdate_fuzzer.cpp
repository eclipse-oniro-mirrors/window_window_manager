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

#include "sessionstagestubupdate_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "marshalling_helper.h"
#include "session_stage_stub.h"
#include "session_stage_ipc_interface_code.h"
#include "window_session_impl.h"
#include "window_option.h"

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

    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA_SYNC),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_AVOID_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_DUMP_SESSSION_ELEMENT_INFO),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAY_MOVE),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SWITCH_FREEMULTIWINDOW),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_FOLLOW_HOST),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_APP_HOOK_WINDOW_INFO),
        parcel, reply, option);
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