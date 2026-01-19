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

#ifndef OHOS_ROSEN_IBUNDLE_MGR_MOCKER_H
#define OHOS_ROSEN_IBUNDLE_MGR_MOCKER_H

#include <gmock/gmock.h>
#include "iremote_object.h"

namespace OHOS::AppExecFwk {
class IBundleMgr;
struct AbilityInfo;
struct ApplicationInfo;
enum ApplicationFlag;
struct BundleInfo;
} // namespace OHOS::AppExecFwk

namespace OHOS {
namespace Rosen {
class IBundleMgrMocker : public IRemoteStub<AppExecFwk::IBundleMgr> {
public:
    IBundleMgrMocker() {};
    ~IBundleMgrMocker() {};

    MOCK_METHOD3(GetApplicationInfos, bool(const AppExecFwk::ApplicationFlag flag,
        const int32_t userId, std::vector<AppExecFwk::ApplicationInfo>& appInfos));
    MOCK_METHOD3(GetApplicationInfosV9, ErrCode(int32_t flags,
        int32_t userId, std::vector<AppExecFwk::ApplicationInfo>& appInfos));
    MOCK_METHOD4(GetApplicationInfo, bool(const std::string& appName, const AppExecFwk::ApplicationFlag flag,
        const int32_t userId, AppExecFwk::ApplicationInfo& appInfo));
    MOCK_METHOD4(GetBundleInfoV9, ErrCode(const std::string& bundleName, int32_t flags,
        AppExecFwk::BundleInfo& bundleInfo, int32_t userId));
    MOCK_METHOD3(QueryLauncherAbilityInfos, ErrCode(const AAFwk::Want &want, int32_t userId,
        std::vector<AppExecFwk::AbilityInfo>& abilityInfos));
    MOCK_METHOD6(GetPluginAbilityInfo, ErrCode(const std::string &hostBundleName, const std::string &pluginBundleName,
        const std::string &pluginModuleName, const std::string &pluginAbilityName,
        const int32_t userId, AppExecFwk::AbilityInfo &abilityInfo));
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_IBUNDLE_MGR_MOCKER_H