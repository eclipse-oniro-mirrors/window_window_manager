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
#include "message_option.h"
#include "message_parcel.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "session/host/include/session.h"
#include "float_view_template_info.h"
#include "want_params.h"
#include "sessionstubfloatview_fuzzer.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 8;
constexpr uint32_t MAX_STRING_LEN = 256;
}

void SessionStubFloatViewUpdateTest(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (SessionInterfaceCode::TRANS_ID_UPDATE_FLOAT_VIEW), parcel, reply, option);
}

void SessionStubFloatViewRestoreTest(sptr<Session> sessionStub, MessageParcel& parcel)
{
    MessageParcel reply;
    MessageOption option;
    parcel.RewindRead(0);
    sessionStub->OnRemoteRequest(static_cast<uint32_t>
        (SessionInterfaceCode::TRANS_ID_RESTORE_FLOAT_VIEW_MAIN_WINDOW), parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    SessionInfo info;
    info.abilityName_ = "floatViewFuzzTest";
    info.bundleName_ = "floatViewFuzzTest";
    sptr<Session> sessionStub = new (std::nothrow) Session(info);
    if (sessionStub == nullptr) {
        return false;
    }

    MessageParcel parcel;
    parcel.WriteInterfaceToken(SessionStub::GetDescriptor());

    FloatViewTemplateInfo fvTemplateInfo;
    fvTemplateInfo.template_ = fdp.ConsumeIntegral<uint32_t>();
    fvTemplateInfo.visibleInApp_ = fdp.ConsumeBool();
    fvTemplateInfo.bindWindowId_ = fdp.ConsumeIntegral<uint32_t>();
    fvTemplateInfo.showWhenCreate_ = fdp.ConsumeBool();
    fvTemplateInfo.isBind_ = fdp.ConsumeBool();
    fvTemplateInfo.id_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    fvTemplateInfo.rect_.posX_ = fdp.ConsumeIntegral<int32_t>();
    fvTemplateInfo.rect_.posY_ = fdp.ConsumeIntegral<int32_t>();
    fvTemplateInfo.rect_.width_ = fdp.ConsumeIntegral<uint32_t>();
    fvTemplateInfo.rect_.height_ = fdp.ConsumeIntegral<uint32_t>();
    parcel.WriteParcelable(&fvTemplateInfo);

    SessionStubFloatViewUpdateTest(sessionStub, parcel);

    parcel.RewindRead(0);
    parcel.WriteInterfaceToken(SessionStub::GetDescriptor());
    parcel.WriteBuffer(data, size);
    SessionStubFloatViewRestoreTest(sessionStub, parcel);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}