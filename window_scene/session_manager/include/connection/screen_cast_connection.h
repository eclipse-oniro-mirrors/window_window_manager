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

#ifndef OHOS_ROSEN_SCREEN_CAST_CONNECTION_H
#define OHOS_ROSEN_SCREEN_CAST_CONNECTION_H

#include <string>
#include <memory>

#include "screen_session_ability_connection.h"

namespace OHOS::Rosen {
class ScreenCastConnection {
public:
    static ScreenCastConnection &GetInstance();

    bool CastConnectExtension(const int32_t &paramFlag);
    void CastDisconnectExtension();
    void SetBundleName(const std::string &bundleName);
    void SetAbilityName(const std::string &abilityName);
    std::string GetBundleName() const;
    std::string GetAbilityName() const;
    bool IsConnectedSync();
    int32_t SendMessageToCastService(const int32_t &transCode, MessageParcel &data, MessageParcel &reply);

private:
    explicit ScreenCastConnection() = default;
    ~ScreenCastConnection() = default;

private:
    std::unique_ptr<ScreenSessionAbilityConnection> abilityConnection_ = nullptr;
    std::string bundleName_{""};
    std::string abilityName_{""};
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_SCREEN_CAST_CONNECTION_H