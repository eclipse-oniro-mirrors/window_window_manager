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

#include "dms_test_base.h"
#include "product_config_mock.h"
#include "parameters.h"

namespace OHOS {
std::string system::DeviceParamMock::currentDeviceType_ = "PLR";
namespace Rosen {
std::string ProductConfigMock::currentDeviceType_ = "PLR";

DmsTestBase::DmsTestBase()
{
    RsMockImpl::GetInstance().Init(ProductConfigMock::ResetAndGetDeviceDefaultConfig());
}

void DmsTestBase::TriggerScreenChange(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason)
{
    RsMockImpl::GetInstance().TriggerScreenChange(screenId, screenEvent, reason);
}

void DmsTestBase::TriggerScreenConnect(ScreenId screenId)
{
    RsMockImpl::GetInstance().TriggerScreenChange(screenId, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
}

void DmsTestBase::TriggerScreenDisconnect(ScreenId screenId)
{
    RsMockImpl::GetInstance().TriggerScreenChange(screenId, ScreenEvent::DISCONNECTED, ScreenChangeReason::DEFAULT);
}

bool DmsTestBase::WaitScreenConnectFinish(ScreenId screenId)
{
    const int32_t MAX_RETRY_CNT = 10;
    const int32_t SLEEP_TIME = 20000;
    int32_t retryCnt = 0;
    while (retryCnt < MAX_RETRY_CNT) {
        auto session = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
        if (session != nullptr) {
            return true;
        }
        usleep(SLEEP_TIME);
        retryCnt++;
    };

    return false;
}

bool DmsTestBase::WaitScreenDisconnectFinish(ScreenId screenId)
{
    const int32_t MAX_RETRY_CNT = 10;
    const int32_t SLEEP_TIME = 20000;
    int32_t retryCnt = 0;
    while (retryCnt < MAX_RETRY_CNT) {
        auto session = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
        if (session == nullptr) {
            return true;
        }
        usleep(SLEEP_TIME);
        retryCnt++;
    };

    return false;
}

bool DmsTestBase::CreatScreen(ScreenId screenId)
{
    TriggerScreenConnect(screenId);
    return WaitScreenConnectFinish(screenId);
}
}  // namespace Rosen
}  // namespace OHOS
