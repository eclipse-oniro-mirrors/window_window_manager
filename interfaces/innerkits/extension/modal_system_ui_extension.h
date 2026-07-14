/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef WINDOW_MANAGER_MODAL_SYSTEM_UI_EXTENSION_H
#define WINDOW_MANAGER_MODAL_SYSTEM_UI_EXTENSION_H

#include <string>
#include <element_name.h>
#include <ability_connect_callback_interface.h>
#include <ability_connect_callback_stub.h>

namespace OHOS {
namespace Rosen {
class ModalSystemUiExtension {
public:
    ModalSystemUiExtension();
    ~ModalSystemUiExtension();

    bool CreateModalUIExtension(const AAFwk::Want& want);
    bool CreateModalUIExtension(const AAFwk::Want& want, const int32_t userId);
    static std::string ToString(const AAFwk::WantParams& wantParams);

private:
    class DialogAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
    public:
        explicit DialogAbilityConnection(const AAFwk::Want& want) : want_(want) {}
        virtual ~DialogAbilityConnection() = default;

        void OnAbilityConnectDone(const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject,
            int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;

    private:
        AAFwk::Want want_;
        bool SendWant(const sptr<IRemoteObject>& remoteObject);
    };

    sptr<OHOS::AAFwk::IAbilityConnection> dialogConnectionCallback_{ nullptr };
};
} // namespace Rosen
} // namespace OHOS

#endif // WINDOW_MANAGER_MODAL_SYSTEM_UI_EXTENSION_H