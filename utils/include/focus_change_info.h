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

#ifndef OHOS_ROSEN_FOCUS_CHANGE_INFO_H
#define OHOS_ROSEN_FOCUS_CHANGE_INFO_H

#include <cstdint>
#include <parcel.h>
#include <iremote_object.h>

#include "wm_common.h"

namespace OHOS::Rosen {
class FocusChangeInfo : public Parcelable {
public:
    /**
     * @brief Default construct of FocusChangeInfo
     */
    FocusChangeInfo() = default;

    /**
     * @brief Construct of FocusChangeInfo
     */
    FocusChangeInfo(uint32_t winId, DisplayId displayId, DisplayId realDisplayId, DisplayGroupId displayGroupId,
        int32_t pid, int32_t uid, WindowType type, const sptr<IRemoteObject>& abilityToken): windowId_(winId),
        displayId_(displayId), realDisplayId_(realDisplayId), displayGroupId_(displayGroupId), pid_(pid), uid_(uid),
        windowType_(type),  abilityToken_(abilityToken) {};

    /**
     * @brief Deconstruct of FocusChangeInfo
     */
    ~FocusChangeInfo() = default;

    /**
     * @brief Marshalling FocusChangeInfo.
     *
     * @param parcel Package of FocusChangeInfo.
     * @return True means marshall FocusChangeInfo success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel& parcel) const
    {
        bool ret = parcel.WriteInt32(windowId_) && parcel.WriteUint64(displayId_) &&
            parcel.WriteUint64(realDisplayId_) && parcel.WriteUint64(displayGroupId_) &&
            parcel.WriteInt32(pid_) && parcel.WriteInt32(uid_) &&
            parcel.WriteUint32(static_cast<uint32_t>(windowType_));
        if (!ret) {
            return false;
        }

        if (abilityToken_) {
            return parcel.WriteBool(true) && (static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(abilityToken_);
        } else {
            return parcel.WriteBool(false);
        }
    }

    static FocusChangeInfo* Unmarshalling(Parcel& parcel)
    {
        auto focusChangeInfo = new FocusChangeInfo();
        bool res = parcel.ReadInt32(focusChangeInfo->windowId_) && parcel.ReadUint64(focusChangeInfo->displayId_) &&
            parcel.ReadUint64(focusChangeInfo->realDisplayId_) && parcel.ReadUint64(focusChangeInfo->displayGroupId_) &&
            parcel.ReadInt32(focusChangeInfo->pid_) && parcel.ReadInt32(focusChangeInfo->uid_);
        if (!res) {
            delete focusChangeInfo;
            return nullptr;
        }
        focusChangeInfo->windowType_ = static_cast<WindowType>(parcel.ReadUint32());
        if (parcel.ReadBool()) {
            focusChangeInfo->abilityToken_ = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        }
        return focusChangeInfo;
    }

    int32_t windowId_ = INVALID_WINDOW_ID;
    DisplayId displayId_ = 0;
    DisplayId realDisplayId_ = 0;
    DisplayGroupId displayGroupId_ = 0;
    int32_t pid_ = -1;
    int32_t uid_ = -1;
    WindowType windowType_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    sptr<IRemoteObject> abilityToken_;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_FOCUS_CHANGE_INFO_H