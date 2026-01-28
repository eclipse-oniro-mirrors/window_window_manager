/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <vector>

#include "mock_message_parcel.h"
#include "iremote_object.h"
#include "message_parcel.h"
namespace OHOS {
namespace {
bool g_setWriteBoolErrorFlag = false;
bool g_setWriteInt32ErrorFlag = false;
bool g_setWriteInt64ErrorFlag = false;
bool g_setWriteUint32ErrorFlag = false;
bool g_setWriteUint64ErrorFlag = false;
bool g_setWriteFloatErrorFlag = false;
bool g_setWriteString16ErrorFlag = false;
bool g_setWriteStringErrorFlag = false;
bool g_setWriteParcelableErrorFlag = false;
bool g_setWriteInterfaceTokenErrorFlag = false;
bool g_setWriteRemoteObjectErrorFlag = false;
bool g_setReadBoolErrorFlag = false;
bool g_setReadUint32ErrorFlag = false;
bool g_setReadInt32ErrorFlag = false;
bool g_setReadUint64ErrorFlag = false;
bool g_setReadInt64ErrorFlag = false;
bool g_setReadFloatErrorFlag = false;
bool g_setReadStringVectorErrorFlag = false;
bool g_setReadStringErrorFlag = false;
std::vector<int32_t> g_int32Cache;
int32_t g_WriteInt32ErrorCount = 0;
#ifdef ENABLE_MOCK_WRITE_STRING
const static std::string ERROR_FLAG = "error";
#endif
constexpr int32_t ERROR_INT = -1;
constexpr float ERROR_FLOAT = -1.0f;
#ifdef ENABLE_MOCK_WRITE_STRING_VECTOR
constexpr int32_t ERROR_SIZE = 1;
#endif

} // namespace
namespace Rosen {
void MockMessageParcel::ClearAllErrorFlag()
{
    g_setWriteBoolErrorFlag = false;
    g_setWriteInt32ErrorFlag = false;
    g_setWriteInt64ErrorFlag = false;
    g_setWriteUint32ErrorFlag = false;
    g_setWriteUint64ErrorFlag = false;
    g_setWriteFloatErrorFlag = false;
    g_setWriteString16ErrorFlag = false;
    g_setWriteStringErrorFlag = false;
    g_setWriteParcelableErrorFlag = false;
    g_setWriteInterfaceTokenErrorFlag = false;
    g_setWriteRemoteObjectErrorFlag = false;
    g_setReadBoolErrorFlag = false;
    g_setReadUint32ErrorFlag = false;
    g_setReadInt32ErrorFlag = false;
    g_setReadUint64ErrorFlag = false;
    g_setReadInt64ErrorFlag = false;
    g_setReadFloatErrorFlag = false;
    g_setReadStringVectorErrorFlag = false;
    g_setReadStringErrorFlag = false;
}

void MockMessageParcel::SetWriteBoolErrorFlag(bool flag)
{
    g_setWriteBoolErrorFlag = flag;
}

void MockMessageParcel::SetWriteInt32ErrorFlag(bool flag)
{
    g_setWriteInt32ErrorFlag = flag;
}

void MockMessageParcel::SetWriteInt64ErrorFlag(bool flag)
{
    g_setWriteInt64ErrorFlag = flag;
}

void MockMessageParcel::SetWriteUint32ErrorFlag(bool flag)
{
    g_setWriteUint32ErrorFlag = flag;
}

void MockMessageParcel::SetWriteUint64ErrorFlag(bool flag)
{
    g_setWriteUint64ErrorFlag = flag;
}

void MockMessageParcel::SetWriteFloatErrorFlag(bool flag)
{
    g_setWriteFloatErrorFlag = flag;
}

void MockMessageParcel::SetWriteString16ErrorFlag(bool flag)
{
    g_setWriteString16ErrorFlag = flag;
}

void MockMessageParcel::SetWriteStringErrorFlag(bool flag)
{
    g_setWriteStringErrorFlag = flag;
}

void MockMessageParcel::SetWriteParcelableErrorFlag(bool flag)
{
    g_setWriteParcelableErrorFlag = flag;
}

void MockMessageParcel::SetWriteInterfaceTokenErrorFlag(bool flag)
{
    g_setWriteInterfaceTokenErrorFlag = flag;
}

void MockMessageParcel::SetWriteRemoteObjectErrorFlag(bool flag)
{
    g_setWriteRemoteObjectErrorFlag = flag;
}

void MockMessageParcel::SetReadBoolErrorFlag(bool flag)
{
    g_setReadBoolErrorFlag = flag;
}

void MockMessageParcel::SetReadUint32ErrorFlag(bool flag)
{
    g_setReadUint32ErrorFlag = flag;
}

void MockMessageParcel::SetReadInt32ErrorFlag(bool flag)
{
    g_setReadInt32ErrorFlag = flag;
}

void MockMessageParcel::SetReadUint64ErrorFlag(bool flag)
{
    g_setReadUint64ErrorFlag = flag;
}

void MockMessageParcel::SetReadInt64ErrorFlag(bool flag)
{
    g_setReadInt64ErrorFlag = flag;
}

void MockMessageParcel::SetReadFloatErrorFlag(bool flag)
{
    g_setReadFloatErrorFlag = flag;
}

void MockMessageParcel::SetReadStringVectorErrorFlag(bool flag)
{
    g_setReadStringVectorErrorFlag = flag;
}

void MockMessageParcel::SetReadStringErrorFlag(bool flag)
{
    g_setReadStringErrorFlag = flag;
}

void MockMessageParcel::AddInt32Cache(int32_t value)
{
    g_int32Cache.emplace_back(value);
}

void MockMessageParcel::SetWriteInt32ErrorCount(int count)
{
    g_WriteInt32ErrorCount = count;
}
}

bool MessageParcel::WriteInterfaceToken(std::u16string name)
{
    if (g_setWriteInterfaceTokenErrorFlag) {
        return false;
    }
    return true;
}

#ifdef ENABLE_MOCK_WRITE_REMOTE_OBJECT
bool MessageParcel::WriteRemoteObject(const sptr<IRemoteObject> &object)
{
    if (g_setWriteRemoteObjectErrorFlag) {
        return false;
    }
    return true;
}
#endif

bool Parcel::WriteParcelable(const Parcelable* object)
{
    (void)object;
    if (g_setWriteParcelableErrorFlag) {
        return false;
    }
    return true;
}

bool Parcel::WriteBool(bool value)
{
    (void)value;
    return !g_setWriteBoolErrorFlag;
}

bool Parcel::WriteInt32(int32_t value)
{
    (void)value;
    if ((g_setWriteInt32ErrorFlag || value == ERROR_INT) && g_WriteInt32ErrorCount < 1) {
        return false;
    }

    if (g_WriteInt32ErrorCount > 0) {
        g_WriteInt32ErrorCount--;
    }
    return true;
}

bool Parcel::WriteInt64(int64_t value)
{
    (void)value;
    if (g_setWriteInt64ErrorFlag || value == ERROR_INT) {
        return false;
    }
    return true;
}

bool Parcel::WriteUint32(uint32_t value)
{
    (void)value;
    if (g_setWriteUint32ErrorFlag) {
        return false;
    }
    return true;
}

bool Parcel::WriteUint64(uint64_t value)
{
    (void)value;
    if (g_setWriteUint64ErrorFlag) {
        return false;
    }
    return true;
}

bool Parcel::WriteFloat(float value)
{
    (void)value;
    if (g_setWriteFloatErrorFlag || value == ERROR_FLOAT) {
        return false;
    }
    return true;
}

bool Parcel::WriteString16(const std::u16string& value)
{
    (void)value;
    return !g_setWriteString16ErrorFlag;
}

bool Parcel::WriteString(const std::string& value)
{
    (void)value;
    return !g_setWriteStringErrorFlag;
}

bool Parcel::ReadBool(bool& val)
{
    if (g_setReadBoolErrorFlag) {
        return false;
    }
    return true;
}

#ifdef ENABLE_MOCK_READ_UINT32
bool Parcel::ReadUint32(uint32_t& value)
{
    if (g_setReadUint32ErrorFlag) {
        return false;
    }
    return true;
}
#endif

#ifdef ENABLE_MOCK_READ_INT32
bool Parcel::ReadInt32(int32_t& value)
{
    if (g_setReadInt32ErrorFlag && g_int32Cache.size() == 0) {
        return false;
    }
    if (g_int32Cache.size() > 0) {
        value = g_int32Cache[0];
        g_int32Cache.erase(g_int32Cache.begin());
    }
    return true;
}
#endif

bool Parcel::ReadUint64(uint64_t& value)
{
    if (g_setReadUint64ErrorFlag) {
        return false;
    }
    return true;
}

#ifdef ENABLE_MOCK_READ_INT64
bool Parcel::ReadInt64(int64_t& value)
{
    if (g_setReadInt64ErrorFlag) {
        return false;
    }
    return true;
}
#endif

bool Parcel::ReadFloat(float& value)
{
    if (g_setReadFloatErrorFlag) {
        return false;
    }
    return true;
}

#ifdef ENABLE_MOCK_WRITE_STRING_VECTOR
bool Parcel::WriteStringVector(const std::vector<std::string>& val)
{
    (void)val;
    if (val.size() == ERROR_SIZE) {
        return false;
    }
    return true;
}
#endif

bool Parcel::ReadStringVector(std::vector<std::string>* val)
{
    if (g_setReadStringVectorErrorFlag) {
        return false;
    }
    return true;
}

bool Parcel::ReadString(std::string& val)
{
    if (g_setReadStringErrorFlag) {
        return false;
    }
    return true;
}
} // namespace OHOS