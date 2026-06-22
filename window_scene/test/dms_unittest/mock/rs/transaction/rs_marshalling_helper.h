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

#ifndef RENDER_SERVICE_BASE_TRANSACTION_RS_MARSHALLING_HELPER_H
#define RENDER_SERVICE_BASE_TRANSACTION_RS_MARSHALLING_HELPER_H

#include <map>
#include <memory>
#include <optional>
#include <parcel.h>
#include <thread>

#include "common/rs_common_def.h"

#define RSPARCELVER_ALWAYS 0x100
#define RSPARCELVER_ADD_ANIMTOKEN 0

namespace OHOS {
namespace Rosen {

class RSMarshallingHelper {
public:
    template<typename T>
    struct is_shared_ptr : std::false_type {};

    template<typename T>
    struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

    template<typename T>
    using is_not_pointer_or_shared_ptr = std::enable_if_t<!std::is_pointer<T>::value && !is_shared_ptr<T>::value, bool>;

    template <typename T, typename = is_not_pointer_or_shared_ptr<T>>
    static bool Unmarshalling(Parcel& parcel, T& val)
    {
        if (const uint8_t* buff = parcel.ReadUnpadBuffer(sizeof(T))) {
            if (buff == nullptr) {
                return false;
            }
            val = *(reinterpret_cast<const T*>(buff));
            return true;
        }
        return false;
    }

    template<typename T, typename = is_not_pointer_or_shared_ptr<T>>
    static bool Marshalling(Parcel& parcel, const T& val)
    {
        return parcel.WriteUnpadBuffer(&val, sizeof(T));
    }
};
}  // namespace Rosen
}  // namespace OHOS

#endif  // RENDER_SERVICE_BASE_TRANSACTION_RS_MARSHALLING_HELPER_H
