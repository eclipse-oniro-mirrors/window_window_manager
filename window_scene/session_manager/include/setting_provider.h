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

#ifndef OHOS_ROSEN_SETTING_PROVIDER_H
#define OHOS_ROSEN_SETTING_PROVIDER_H

#include "datashare_helper.h"
#include "errors.h"
#include "mutex"
#include "setting_observer.h"

namespace OHOS {
namespace Rosen {
class SettingProvider : public NoCopyable {
public:
    static SettingProvider& GetInstance(int32_t systemAbilityId);
    ErrCode GetStringValue(const std::string& key, std::string& value);
    ErrCode GetIntValue(const std::string& key, int32_t& value);
    ErrCode GetLongValue(const std::string& key, int64_t& value);
    ErrCode GetBoolValue(const std::string& key, bool& value);
    ErrCode PutStringValue(const std::string& key, const std::string& value, bool needNotify = true);
    ErrCode PutIntValue(const std::string& key, int32_t value, bool needNotify = true);
    ErrCode PutLongValue(const std::string& key, int64_t value, bool needNotify = true);
    ErrCode PutBoolValue(const std::string& key, bool value, bool needNotify = true);
    bool IsValidKey(const std::string& key);
    sptr<SettingObserver> CreateObserver(const std::string& key, SettingObserver::UpdateFunc& func);
    static void ExecRegisterCb(const sptr<SettingObserver>& observer);
    ErrCode RegisterObserver(const sptr<SettingObserver>& observer);
    ErrCode UnregisterObserver(const sptr<SettingObserver>& observer);

    ErrCode RegisterObserverByTable(const sptr<SettingObserver>& observer, std::string tableName);
    ErrCode UnregisterObserverByTable(const sptr<SettingObserver>& observer, std::string tableName);
    ErrCode GetIntValueMultiUserByTable(const std::string& key, int32_t& value, std::string tableName);
    ErrCode GetLongValueMultiUserByTable(const std::string& key, int64_t& value, std::string tableName);
    ErrCode GetStringValueMultiUserByTable(const std::string& key, std::string& value, std::string tableName);

protected:
    ~SettingProvider() override;

private:
    static SettingProvider* instance_;
    static std::mutex mutex_;
    static sptr<IRemoteObject> remoteObj_;

    static void Initialize(int32_t systemAbilityId);
    static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper();
    static bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper>& helper);
    static Uri AssembleUri(const std::string& key);

    static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelperMultiUserByTable(std::string tableName);
    static Uri AssembleUriMultiUserByTable(const std::string& key, std::string tableName);
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_SETTING_PROVIDER_H
