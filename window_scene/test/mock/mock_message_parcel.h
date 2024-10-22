/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef MOCK_MESSAGE_PARCEL_H
#define MOCK_MESSAGE_PARCEL_H
namespace OHOS {
namespace Rosen {
class MockMessageParcel {
public:
    static void ClearAllErrorFlag();
    static void SetWriteBoolErrorFlag(bool flag);
    static void SetWriteInt32ErrorFlag(bool flag);
    static void SetWriteInt64ErrorFlag(bool flag);
    static void SetWriteUint32ErrorFlag(bool flag);
    static void SetWriteFloatErrorFlag(bool flag);
    static void SetWriteString16ErrorFlag(bool flag);
    static void SetWriteParcelableErrorFlag(bool flag);
    static void SetWriteInterfaceTokenErrorFlag(bool flag);
    static void SetReadInt32ErrorFlag(bool flag);
    static void SetReadInt64ErrorFlag(bool flag);
};
}
}
#endif