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

#include <map>
#include "mock_accesstoken_kit.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"

using namespace OHOS::Security::AccessToken;
namespace OHOS::Rosen {
namespace {
bool g_isSystemApp = true;
bool g_isUseTokenMap = false;
bool g_isMockGetTokenTypeFlagRet = true;
int32_t g_accessTokenKitRet = 0;
std::map<std::string, int32_t> g_tokenMap;
}

void MockAccesstokenKit::MockIsSystemApp(const bool isSystemApp)
{
    g_isSystemApp = isSystemApp;
}

void MockAccesstokenKit::MockIsSACalling(const bool mockGetTokenTypeFlagRet)
{
    g_isMockGetTokenTypeFlagRet = mockGetTokenTypeFlagRet;
}

void MockAccesstokenKit::MockAccessTokenKitRet(int32_t accessTokenKitRet)
{
    g_accessTokenKitRet = accessTokenKitRet;
}

void MockAccesstokenKit::MockIsUseTokenMap(const bool useTokenMap)
{
    g_isUseTokenMap = useTokenMap;
}

void MockAccesstokenKit::MockTokenMap(const std::string& permissionName, int32_t available)
{
    g_tokenMap[permissionName] = available;
}

void MockAccesstokenKit::ChangeMockStateToInit()
{
    g_isSystemApp = true;
    g_isMockGetTokenTypeFlagRet = true;
    g_accessTokenKitRet = 0;
    g_isUseTokenMap = false;
    g_tokenMap.clear();
}
}

namespace OHOS::Security::AccessToken {
bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t tokenId)
{
    return Rosen::g_isSystemApp;
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenId)
{
    if (Rosen::g_isMockGetTokenTypeFlagRet) {
        return Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    }
    return Security::AccessToken::ATokenTypeEnum::TOKEN_INVALID;
}

int32_t AccessTokenKit::VerifyAccessToken(uint32_t AccessTokenID, const std::string& permissionName)
{
    if (Rosen::g_isUseTokenMap) {
        return Rosen::g_tokenMap[permissionName];
    }
    return Rosen::g_accessTokenKitRet;
}
} // namespace OHOS::Security::AccessToken