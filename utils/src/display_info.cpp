/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "display_info.h"

#include <new>
#include <parcel.h>


namespace OHOS::Rosen {
bool DisplayInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteString(name_) && parcel.WriteUint64(id_) && parcel.WriteUint32(static_cast<uint32_t>(type_)) &&
        parcel.WriteInt32(width_) && parcel.WriteInt32(height_) &&
        parcel.WriteInt32(physicalWidth_) && parcel.WriteInt32(physicalHeight_) &&
        parcel.WriteUint32(refreshRate_) && parcel.WriteUint64(screenId_) &&
        parcel.WriteFloat(virtualPixelRatio_) && parcel.WriteFloat(densityInCurResolution_) &&
        parcel.WriteFloat(defaultVirtualPixelRatio_) &&
        parcel.WriteFloat(xDpi_) && parcel.WriteFloat(yDpi_) &&
        parcel.WriteUint32(static_cast<uint32_t>(rotation_)) &&
        parcel.WriteUint32(static_cast<uint32_t>(orientation_)) &&
        parcel.WriteInt32(offsetX_) && parcel.WriteInt32(offsetY_) &&
        parcel.WriteUint32(static_cast<uint32_t>(displayState_)) &&
        parcel.WriteBool(waterfallDisplayCompressionStatus_) &&
        parcel.WriteInt32(dpi_) && parcel.WriteUint32(static_cast<uint32_t>(displayOrientation_)) &&
        parcel.WriteUInt32Vector(colorSpaces_) && parcel.WriteUInt32Vector(hdrFormats_) &&
        parcel.WriteUint32(defaultDeviceRotationOffset_) &&
        parcel.WriteUint32(availableWidth_) &&
        parcel.WriteUint32(availableHeight_) && parcel.WriteFloat(scaleX_) &&
        parcel.WriteFloat(scaleY_) && parcel.WriteFloat(pivotX_) && parcel.WriteFloat(pivotY_) &&
        parcel.WriteFloat(translateX_) && parcel.WriteFloat(translateY_) &&
        parcel.WriteUint32(static_cast<uint32_t>(screenShape_)) &&
        parcel.WriteUint32(static_cast<uint32_t>(originRotation_)) &&
        parcel.WriteInt32(x_) && parcel.WriteInt32(y_) &&
        parcel.WriteUint32(static_cast<uint32_t>(displaySourceMode_)) &&
        parcel.WriteUInt32Vector(supportedRefreshRate_) &&
        parcel.WriteBool(supportsFocus_) &&
        parcel.WriteBool(supportsInput_) &&
        parcel.WriteString(bundleName_);
}

DisplayInfo *DisplayInfo::Unmarshalling(Parcel &parcel)
{
    DisplayInfo *displayInfo = new(std::nothrow) DisplayInfo();
    if (displayInfo == nullptr) {
        return nullptr;
    }
    uint32_t type = (uint32_t)DisplayType::DEFAULT;
    uint32_t rotation;
    uint32_t originRotation;
    uint32_t orientation;
    uint32_t displayState;
    uint32_t displayOrientation;
    uint32_t screenShape;
    uint32_t displaySourceMode;
    bool res = parcel.ReadString(displayInfo->name_) && parcel.ReadUint64(displayInfo->id_) &&
        parcel.ReadUint32(type) && parcel.ReadInt32(displayInfo->width_) && parcel.ReadInt32(displayInfo->height_) &&
        parcel.ReadInt32(displayInfo->physicalWidth_) && parcel.ReadInt32(displayInfo->physicalHeight_) &&
        parcel.ReadUint32(displayInfo->refreshRate_) && parcel.ReadUint64(displayInfo->screenId_) &&
        parcel.ReadFloat(displayInfo->virtualPixelRatio_) && parcel.ReadFloat(displayInfo->densityInCurResolution_) &&
        parcel.ReadFloat(displayInfo->defaultVirtualPixelRatio_) &&
        parcel.ReadFloat(displayInfo->xDpi_) && parcel.ReadFloat(displayInfo->yDpi_) &&
        parcel.ReadUint32(rotation) && parcel.ReadUint32(orientation) &&
        parcel.ReadInt32(displayInfo->offsetX_) && parcel.ReadInt32(displayInfo->offsetY_) &&
        parcel.ReadUint32(displayState) && parcel.ReadBool(displayInfo->waterfallDisplayCompressionStatus_) &&
        parcel.ReadInt32(displayInfo->dpi_) && parcel.ReadUint32(displayOrientation) &&
        parcel.ReadUInt32Vector(&(displayInfo->colorSpaces_)) && parcel.ReadUInt32Vector(&(displayInfo->hdrFormats_)) &&
        parcel.ReadUint32(displayInfo->defaultDeviceRotationOffset_) &&
        parcel.ReadUint32(displayInfo->availableWidth_) &&
        parcel.ReadUint32(displayInfo->availableHeight_) && parcel.ReadFloat(displayInfo->scaleX_) &&
        parcel.ReadFloat(displayInfo->scaleY_) && parcel.ReadFloat(displayInfo->pivotX_) &&
        parcel.ReadFloat(displayInfo->pivotY_) && parcel.ReadFloat(displayInfo->translateX_) &&
        parcel.ReadFloat(displayInfo->translateY_) && parcel.ReadUint32(screenShape) &&
        parcel.ReadUint32(originRotation) && parcel.ReadInt32(displayInfo->x_) &&
        parcel.ReadInt32(displayInfo->y_) && parcel.ReadUint32(displaySourceMode) &&
        parcel.ReadUInt32Vector(&(displayInfo->supportedRefreshRate_)) &&
        parcel.ReadBool(displayInfo->supportsFocus_) && parcel.ReadBool(displayInfo->supportsInput_) &&
        parcel.ReadString(displayInfo->bundleName_);
    if (!res) {
        delete displayInfo;
        return nullptr;
    }
    displayInfo->type_ = (DisplayType)type;
    displayInfo->rotation_ = static_cast<Rotation>(rotation);
    displayInfo->orientation_ = static_cast<Orientation>(orientation);
    displayInfo->displayState_ = static_cast<DisplayState>(displayState);
    displayInfo->displayOrientation_ = static_cast<DisplayOrientation>(displayOrientation);
    displayInfo->screenShape_ = static_cast<ScreenShape>(screenShape);
    displayInfo->originRotation_ = static_cast<Rotation>(originRotation);
    displayInfo->displaySourceMode_ = static_cast<DisplaySourceMode>(displaySourceMode);
    return displayInfo;
}
} // namespace OHOS::Rosen