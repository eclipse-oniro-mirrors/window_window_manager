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

/**
 * @file rs_sync_transaction_controller.h
 * @brief this file is used to control the execution of synchronous transactions.
 */

#ifndef RENDER_SERVICE_CLIENT_CORE_PIPELINE_RS_SYNC_TRANSACTION_CONTROLLER_H
#define RENDER_SERVICE_CLIENT_CORE_PIPELINE_RS_SYNC_TRANSACTION_CONTROLLER_H

#include <mutex>
#include <vector>
#include <event_handler.h>

namespace OHOS {
namespace Rosen {
class RSTransaction;

class RSSyncTransactionController {
public:
    static RSSyncTransactionController* GetInstance()
    {
        static RSSyncTransactionController instance_;
        return &instance_;
    }

    void OpenSyncTransaction(std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr, bool isInnerProcess = false)
    {
    }

    void CloseSyncTransaction(std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr) {}

    std::shared_ptr<RSTransaction> GetRSTransaction()
    {
        return nullptr;
    }
};
}  // namespace Rosen
}  // namespace OHOS

#endif  // RENDER_SERVICE_CLIENT_CORE_PIPELINE_RS_SYNC_TRANSACTION_CONTROLLER_H
