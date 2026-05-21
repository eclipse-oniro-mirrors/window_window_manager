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
#include "session_stage_stub.h"
#include "session_stage_ipc_interface_code.h"
#include "sessionstagestublayout_fuzzer.h"
#include "window_session_impl.h"
#include "window_option.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

static void SessionStageStubLayoutTestGroup1(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel,
    MessageParcel& reply, MessageOption& option)
{
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_LAYOUT_FINISH_AFTER_WINDOW_MODE_CHANGE),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SINGLE_HAND_TRANSFORM),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_DRAG_ACTIVATED),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_WINDOW_MODE_FOR_UI_TEST),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_ANIMATION_SPEED),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SUB_WINDOW_AFTER_PARENT_WINDOW_SIZE_CHANGE),
        parcel, reply, option);
}

static void SessionStageStubLayoutTestGroup2(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel,
    MessageParcel& reply, MessageOption& option)
{
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SUB_WINDOW_AFTER_PARENT_WINDOW_STATUS_CHANGE),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_ATTACHED_WINDOW_LIMITS),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SYNC_ALL_ATTACHED_LIMITS_TO_CHILD),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_REBIND_ATTACH_AFTER_PARENT_CHANGE),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_CONFIG_DOCK_AUTO_HIDE),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_IS_START_MOVING),
        parcel, reply, option);
}

void SessionStageStubLayoutTest(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;

    SessionStageStubLayoutTestGroup1(stageStub, parcel, reply, option);
    SessionStageStubLayoutTestGroup2(stageStub, parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    
    MessageParcel parcel;

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

    SessionStageStubLayoutTest(stageStub, parcel);
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