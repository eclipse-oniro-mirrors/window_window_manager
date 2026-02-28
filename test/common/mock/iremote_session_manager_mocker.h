/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_TEST_COMMON_MOCK_IREMOTE_SESSION_MANAGER_MOCKER
#define OHOS_ROSEN_TEST_COMMON_MOCK_IREMOTE_SESSION_MANAGER_MOCKER

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "session_manager_service_interface.h"

namespace OHOS::Rosen {
class IRemoteSessionManagerMocker : public IRemoteStub<ISessionManagerService> {
public:
    IRemoteSessionManagerMocker() = default;
    ~IRemoteSessionManagerMocker() = default;

    // func mocked and will be tested
    MOCK_METHOD(sptr<IRemoteObject>, GetSceneSessionManager, (), (override));
    MOCK_METHOD(sptr<IRemoteObject>, GetSceneSessionManagerLite, (), (override));
};
} // namespace OHOS::Rosen
#endif
