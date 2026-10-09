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

#ifndef OHOS_ROSEN_SETTING_OBSERVER_H
#define OHOS_ROSEN_SETTING_OBSERVER_H

#include "data_ability_observer_stub.h"

namespace OHOS {
namespace Rosen {
class SettingObserver : public AAFwk::DataAbilityObserverStub {
public:
    SettingObserver();
    ~SettingObserver() override;
    void OnChange() override;

    void SetKey(const std::string& key);
    const std::string& GetKey();

    // The exact uri this observer was registered on datashare with. Multi-user keys
    // register on a per-user table; unregistering needs the same uri, not whatever the
    // current user would resolve to.
    void SetRegisteredUri(const std::string& uri);
    const std::string& GetRegisteredUri();

    using UpdateFunc = std::function<void(const std::string&)>;
    void SetUpdateFunc(UpdateFunc& func);

private:
    std::string key_ {};
    std::string registeredUri_ {};
    UpdateFunc update_ = nullptr;
};
} // OHOS
} // Rosen

#endif // OHOS_ROSEN_SETTING_OBSERVER_H
