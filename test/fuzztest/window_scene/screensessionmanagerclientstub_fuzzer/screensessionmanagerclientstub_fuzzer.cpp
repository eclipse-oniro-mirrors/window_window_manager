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

#include <cstddef>
#include <cstdint>
#include <array>
#include <parcel.h>

#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "screen_session_manager_client_interface.h"
#include "screen_session_manager_client_stub.h"
#include "screensessionmanagerclientstub_fuzzer.h"
#include "screen_session_manager_client.h"
#include "dm_common.h"
#include <fuzzer/FuzzedDataProvider.h>

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
constexpr size_t MAX_BUFFER_SIZE = 18;
constexpr size_t MAX_LENGTH_STRING = 64;
}

static constexpr std::array<uint32_t, 38> kValidTransIds = {{
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_CONNECTION_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_PROPERTY_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_POWER_STATUS_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SENSOR_ROTATION_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED_WITH_OPTIONS),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_DISPLAY_STATE_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_SHOT),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_IMMERSIVE_STATE_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_SET_SURFACENODEIDS),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_VIRTUAL_SCREEN_DISCONNECTED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_SET_FOLD_DISPLAY_MODE),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SWITCH_USER_CMD),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_CAPTURE_NOTIFY),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_HOVER_STATUS_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_EXTEND_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SECONDARY_REFLEXION_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_CAMERA_BACKSELFIE_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_EXTEND_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_CREATE_SCREEN_SESSION_ONLY),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_MAIN_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_SET_SCREEN_COMBINATION),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_DUMP_SCREEN_SESSION),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_BEFORE_PROPERTY_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_MODE_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_ANIMATE_FINISH_TIMEOUT),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_FOLD_PROPERTY_CHANGED),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_SET_INTERNAL_CLIPTOBOUNDS),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_TENT_MODE_CHANGE),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_TRANS_RS_EVENT_TO_DESKTOP),
    static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
        TRANS_ID_ON_SCREEN_CLOSED_STATE_CHANGE),
}};

void ScreenSessionManagerClientStubFuzzTestEnhanced(FuzzedDataProvider& fdp,
    ScreenSessionManagerClientStub& clientStub, uint32_t code)
{
    MessageParcel parcel;
    MessageOption option;
    MessageParcel reply;
    parcel.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    switch (code) {
        // No params
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_ANIMATE_FINISH_TIMEOUT):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_DUMP_SCREEN_SESSION):
            break;

        // uint64: screenId / displayId
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_SHOT):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_IMMERSIVE_STATE_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_VIRTUAL_SCREEN_DISCONNECTED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }

        // uint32: enum / status
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_BEFORE_PROPERTY_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_MODE_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_TENT_MODE_CHANGE):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_CLOSED_STATE_CHANGE):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_SET_FOLD_DISPLAY_MODE): {
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }

        // uint64 + uint64: two screenIds/displayIds
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_EXTEND_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_EXTEND_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }

        // uint64 + uint32: screenId + enum/status
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SECONDARY_REFLEXION_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }

        // uint64 + bool: screenId + flag
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_CAMERA_BACKSELFIE_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_SET_INTERNAL_CLIPTOBOUNDS): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }

        // uint64 + float: screenId + rotation
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED):
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteFloat(fdp.ConsumeFloatingPoint<float>());
            break;
        }

        // uint64 + float + bool: screenId + rotation + switchUser
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SENSOR_ROTATION_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteFloat(fdp.ConsumeFloatingPoint<float>());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }

        // uint64 + int32 + bool: screenId + hoverStatus + needRotate
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_HOVER_STATUS_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }

        // uint64 + float + bool + bool: screenId + orientation + needAnimation + ignoreRotationLock
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED_WITH_OPTIONS): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteFloat(fdp.ConsumeFloatingPoint<float>());
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }

        // uint64 + uint64 + uint32: two IDs + screenCombination
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_SET_SCREEN_COMBINATION): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }

        // uint64 + uint64 + uint64: three screenIds
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_MAIN_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }

        // uint32 + uint32 + uint32: event + status + reason
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_POWER_STATUS_CHANGED): {
            parcel.WriteUint32(fdp.ConsumeIntegralInRange<uint32_t>(
                0, static_cast<uint32_t>(DisplayPowerEvent::DISPLAY_END_DREAM)));
            parcel.WriteUint32(fdp.ConsumeIntegralInRange<uint32_t>(
                0, static_cast<uint32_t>(EventStatus::END)));
            parcel.WriteUint32(fdp.ConsumeIntegralInRange<uint32_t>(
                static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON),
                static_cast<uint32_t>(PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN)));
            break;
        }

        // uint64 + int32 + string: screenId + uid + clientName
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_CAPTURE_NOTIFY): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            break;
        }

        // ReadStringVector
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE): {
            size_t vecSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<std::string> strVec;
            for (size_t i = 0; i < vecSize; ++i) {
                strVec.push_back(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            }
            parcel.WriteStringVector(strVec);
            break;
        }

        // ReadInt32Vector + ReadInt32
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SWITCH_USER_CMD): {
            size_t vecSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<int32_t> pidVec;
            for (size_t i = 0; i < vecSize; ++i) {
                pidVec.push_back(fdp.ConsumeIntegral<int32_t>());
            }
            parcel.WriteInt32Vector(pidVec);
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            break;
        }

        // uint64 + ReadUInt64Vector
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_SET_SURFACENODEIDS): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            size_t vecSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<uint64_t> idVec;
            for (size_t i = 0; i < vecSize; ++i) {
                idVec.push_back(fdp.ConsumeIntegral<uint64_t>());
            }
            parcel.WriteUInt64Vector(idVec);
            break;
        }

        // ReadUInt64Vector + ReadUInt64Vector + ReadUInt32Vector + ReadBool
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID): {
            size_t missionSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<uint64_t> missionIds;
            for (size_t i = 0; i < missionSize; ++i) {
                missionIds.push_back(fdp.ConsumeIntegral<uint64_t>());
            }
            parcel.WriteUInt64Vector(missionIds);

            size_t nodeSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<uint64_t> surfaceNodeIds;
            for (size_t i = 0; i < nodeSize; ++i) {
                surfaceNodeIds.push_back(fdp.ConsumeIntegral<uint64_t>());
            }
            parcel.WriteUInt64Vector(surfaceNodeIds);

            size_t typeSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<uint32_t> windowTypeList;
            for (size_t i = 0; i < typeSize; ++i) {
                windowTypeList.push_back(fdp.ConsumeIntegral<uint32_t>());
            }
            parcel.WriteUInt32Vector(windowTypeList);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }

        // uint64 + uint64 + string + RemoteObject + bool
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_CREATE_SCREEN_SESSION_ONLY): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            // Write a RemoteObject — may be null (valid error path) or dummy
            if (fdp.ConsumeBool()) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }

        // Complex: TRANS_ID_ON_SCREEN_CONNECTION_CHANGED
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_SCREEN_CONNECTION_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());   // rsId
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING)); // name
            parcel.WriteBool(fdp.ConsumeBool());                     // isExtend
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING)); // innerName
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());   // screenId
            parcel.WriteUint8(fdp.ConsumeIntegral<uint8_t>());     // screenEvent
            // rotationCorrectionMap
            size_t mapSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(static_cast<uint64_t>(mapSize));
            for (size_t i = 0; i < mapSize; ++i) {
                parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
                parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            }
            parcel.WriteBool(fdp.ConsumeBool());                     // supportsFocus
            // rotationOptions
            parcel.WriteBool(fdp.ConsumeBool());                     // isRotationLocked
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());      // rotation
            // rotationOrientationMap
            size_t rotMapSize = fdp.ConsumeIntegralInRange<size_t>(0, 4);
            parcel.WriteUint64(static_cast<uint64_t>(rotMapSize));
            for (size_t i = 0; i < rotMapSize; ++i) {
                parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
                parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            }
            parcel.WriteBool(fdp.ConsumeBool());                     // isBooting
            // remote object (optional)
            bool hasRemoteObj = fdp.ConsumeBool();
            parcel.WriteBool(hasRemoteObj);
            break;
        }

        // Complex: TRANS_ID_ON_PROPERTY_CHANGED — screenId + RSMarshalling + reason
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_PROPERTY_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());   // screenId
            // Write placeholder bytes for ScreenProperty marshalling — RSMarshallingHelper
            // will attempt to unmarshal; may succeed or fail depending on data
            size_t propSize = fdp.ConsumeIntegralInRange<size_t>(0, 256);
            std::vector<uint8_t> propBytes;
            for (size_t i = 0; i < propSize; ++i) {
                propBytes.push_back(fdp.ConsumeIntegral<uint8_t>());
            }
            if (!propBytes.empty()) {
                parcel.WriteBuffer(propBytes.data(), propBytes.size());
            }
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());   // reason
            break;
        }

        // Complex: TRANS_ID_ON_FOLD_PROPERTY_CHANGED
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_FOLD_PROPERTY_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());   // screenId
            size_t propSize = fdp.ConsumeIntegralInRange<size_t>(0, 256);
            std::vector<uint8_t> propBytes;
            for (size_t i = 0; i < propSize; ++i) {
                propBytes.push_back(fdp.ConsumeIntegral<uint8_t>());
            }
            if (!propBytes.empty()) {
                parcel.WriteBuffer(propBytes.data(), propBytes.size());
            }
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());   // reason
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());   // displayMode
            break;
        }

        // Complex: TRANS_ID_ON_DISPLAY_STATE_CHANGED
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_DISPLAY_STATE_CHANGED): {
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());   // defaultDisplayId
            // DisplayInfo parcelable — write raw bytes for the parcelable
            size_t infoSize = fdp.ConsumeIntegralInRange<size_t>(0, 256);
            std::vector<uint8_t> infoBytes;
            for (size_t i = 0; i < infoSize; ++i) {
                infoBytes.push_back(fdp.ConsumeIntegral<uint8_t>());
            }
            if (!infoBytes.empty()) {
                parcel.WriteBuffer(infoBytes.data(), infoBytes.size());
            }
            // displayInfoMap
            uint32_t mapSize = fdp.ConsumeIntegralInRange<uint32_t>(0, 8);
            parcel.WriteUint32(mapSize);
            for (uint32_t i = 0; i < mapSize; ++i) {
                parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
                size_t diSize = fdp.ConsumeIntegralInRange<size_t>(0, 128);
                std::vector<uint8_t> diBytes;
                for (size_t j = 0; j < diSize; ++j) {
                    diBytes.push_back(fdp.ConsumeIntegral<uint8_t>());
                }
                if (!diBytes.empty()) {
                    parcel.WriteBuffer(diBytes.data(), diBytes.size());
                }
            }
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());   // type
            break;
        }

        // Complex: TRANS_ID_ON_TRANS_RS_EVENT_TO_DESKTOP — type + event unmarshalling
        case static_cast<uint32_t>(IScreenSessionManagerClient::ScreenSessionManagerClientMessage::
            TRANS_ID_ON_TRANS_RS_EVENT_TO_DESKTOP): {
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());   // event type
            size_t eventSize = fdp.ConsumeIntegralInRange<size_t>(0, 256);
            std::vector<uint8_t> eventBytes;
            for (size_t i = 0; i < eventSize; ++i) {
                eventBytes.push_back(fdp.ConsumeIntegral<uint8_t>());
            }
            if (!eventBytes.empty()) {
                parcel.WriteBuffer(eventBytes.data(), eventBytes.size());
            }
            break;
        }

        default:
            break;
    }

    parcel.RewindRead(0);
    clientStub.OnRemoteRequest(code, parcel, reply, option);
}

void ScreenSessionManagerClientStubFuzzTestMalformed(FuzzedDataProvider& fdp,
    ScreenSessionManagerClientStub& clientStub, uint32_t code)
{
    MessageParcel parcel;
    MessageOption option;
    MessageParcel reply;
    parcel.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    // Write raw bytes — typically too few or ill-typed, triggering Read* failures in the stub
    size_t rawSize = fdp.ConsumeIntegralInRange<size_t>(0, 256);
    std::vector<uint8_t> rawBytes;
    rawBytes.reserve(rawSize);
    for (size_t i = 0; i < rawSize; ++i) {
        rawBytes.push_back(fdp.ConsumeIntegral<uint8_t>());
    }
    if (!rawBytes.empty()) {
        parcel.WriteBuffer(rawBytes.data(), rawBytes.size());
    }

    parcel.RewindRead(0);
    clientStub.OnRemoteRequest(code, parcel, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);
    ScreenSessionManagerClientStub& clientStub = ScreenSessionManagerClient::GetInstance();

    for (size_t i = 0; i < kValidTransIds.size(); ++i) {
        ScreenSessionManagerClientStubFuzzTestEnhanced(fdp, clientStub, kValidTransIds[i]);
        ScreenSessionManagerClientStubFuzzTestMalformed(fdp, clientStub, kValidTransIds[i]);
    }
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
