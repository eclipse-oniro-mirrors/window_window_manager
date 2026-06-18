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

#include "scenesessionmgrstubfv_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <parcel.h>
#include <securec.h>
#include <fuzzer/FuzzedDataProvider.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "scene_session_manager.h"
#include "scene_session_manager_stub.h"
#include "scene_session_manager_interface.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 4;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());

    uint32_t templateType = fdp.ConsumeIntegral<uint32_t>();
    parcel.WriteUint32(templateType);

    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);

    SceneSessionManager::GetInstance().OnRemoteRequest(
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FLOAT_VIEW_LIMITS),
        parcel, reply, option);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}