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
#include <fuzzer/FuzzedDataProvider.h>

#include <iremote_stub.h>
#include "marshalling_helper.h"
#include "message_option.h"
#include "message_parcel.h"
#include "session/container/include/zidl/session_stage_stub.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"
#include "window_session_impl.h"
#include "window_option.h"
#include "wm_common.h"
#include "sessionstagestubfv_fuzzer.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 8;
constexpr uint32_t MAX_STRING_LEN = 256;
constexpr uint32_t MAX_FV_LIMITS = 10;
}

void SessionStageStubFvActionEventTest(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel,
    FuzzedDataProvider& fdp)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    parcel.WriteInterfaceToken(SessionStageStub::GetDescriptor());

    std::string action = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string reason = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    parcel.WriteString(action);
    parcel.WriteString(reason);

    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_FV_ACTION_EVENT),
        parcel, reply, option);
}

void SessionStageStubFvWindowInfoTest(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel,
    FuzzedDataProvider& fdp)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    parcel.WriteInterfaceToken(SessionStageStub::GetDescriptor());

    FloatViewWindowInfo windowInfo;
    windowInfo.windowRect_.posX_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.windowRect_.posY_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.windowRect_.width_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.windowRect_.height_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.leftRect_.posX_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.leftRect_.posY_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.leftRect_.width_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.leftRect_.height_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.topRect_.posX_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.topRect_.posY_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.topRect_.width_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.topRect_.height_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.rightRect_.posX_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.rightRect_.posY_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.rightRect_.width_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.rightRect_.height_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.bottomRect_.posX_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.bottomRect_.posY_ = fdp.ConsumeIntegral<int32_t>();
    windowInfo.avoidArea_.bottomRect_.width_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.avoidArea_.bottomRect_.height_ = fdp.ConsumeIntegral<uint32_t>();
    windowInfo.scale_ = fdp.ConsumeFloatingPoint<float>();

    parcel.WriteParcelable(&windowInfo);
    std::string reason = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    parcel.WriteString(reason);

    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SYNC_FV_WINDOW_INFO),
        parcel, reply, option);
}

void SessionStageStubFvLimitsTest(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel,
    FuzzedDataProvider& fdp)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    parcel.WriteInterfaceToken(SessionStageStub::GetDescriptor());

    uint8_t size = fdp.ConsumeIntegral<uint8_t>() % MAX_FV_LIMITS;
    parcel.WriteUint32(static_cast<uint32_t>(size));

    for (uint8_t i = 0; i < size; ++i) {
        uint32_t templateType = fdp.ConsumeIntegral<uint32_t>();
        parcel.WriteUint32(templateType);

        FloatViewLimits limits;
        limits.maxWidth_ = fdp.ConsumeIntegral<uint32_t>();
        limits.maxHeight_ = fdp.ConsumeIntegral<uint32_t>();
        limits.minWidth_ = fdp.ConsumeIntegral<uint32_t>();
        limits.minHeight_ = fdp.ConsumeIntegral<uint32_t>();
        parcel.WriteParcelable(&limits);
    }

    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SYNC_FV_LIMITS),
        parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    sptr<WindowOption> windowOption = new (std::nothrow) WindowOption();
    if (windowOption == nullptr) {
        return false;
    }
    sptr<WindowSessionImpl> stageStub = new (std::nothrow) WindowSessionImpl(windowOption);
    if (stageStub == nullptr) {
        return false;
    }

    MessageParcel parcel;

    SessionStageStubFvActionEventTest(stageStub, parcel, fdp);
    SessionStageStubFvWindowInfoTest(stageStub, parcel, fdp);
    SessionStageStubFvLimitsTest(stageStub, parcel, fdp);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}