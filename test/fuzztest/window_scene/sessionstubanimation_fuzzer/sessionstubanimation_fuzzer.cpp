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

#include <iremote_broker.h>

#include "session_manager.h"
#include "sessionstubanimation_fuzzer.h"
#include "window_scene_session_impl.h"
#include "window_option.h"
#include "zidl/session_ipc_interface_code.h"
#include "zidl/session_proxy.h"
#include "zidl/session_stage_ipc_interface_code.h"
#include "zidl/session_stage_stub.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

void SessionStubAnimationTest(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;

    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_GET_TARGET_ORIENTATION_CONFIG_INFO),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_UPDATE_ROTATION_CHANGE),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_WINDOW_SHADOWS),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_FRAMERECT_FOR_PARTIAL_ZOOMIN),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_WINDOW_TRANSITION_ANIMATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionInterfaceCode::TRANS_ID_SET_PREFERRED_ORIENTATION_WITH_RESULT),
        parcel, reply, option);
}

void SessionStageStubAnimationTest(sptr<WindowSessionImpl> stageStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;

    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionStageInterfaceCode::TRANS_ID_SET_CURRENT_ROTATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_PROPERTY),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_CHANGE),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionStageInterfaceCode::TRANS_ID_GET_SCREEN_NODE_COUNT),
        parcel, reply, option);
    parcel.RewindRead(0);
    stageStub->OnRemoteRequest(static_cast<uint32_t>
        (Rosen::SessionStageInterfaceCode::TRANS_ID_GET_SCENE_NODE_COUNT_WITH_CALLBACK),
        parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI1(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    parcel.WriteInterfaceToken(SessionStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    SessionInfo info;
    info.abilityName_ = "stubAnimationFuzzTest";
    info.bundleName_ = "stubAnimationFuzzTest";
    sptr<Session> sessionStub = new (std::nothrow) Session(info);
    if (sessionStub == nullptr) {
        return false;
    }

    SessionStubAnimationTest(sessionStub, parcel);
    return true;
}

bool DoSomethingInterestingWithMyAPI2(const uint8_t* data, size_t size)
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

    SessionStageStubAnimationTest(stageStub, parcel);
    return true;
}
} // namespace.OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI1(data, size);
    OHOS::DoSomethingInterestingWithMyAPI2(data, size);
    return 0;
}