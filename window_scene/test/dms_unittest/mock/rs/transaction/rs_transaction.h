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

#ifndef RENDER_SERVICE_CLIENT_CORE_UI_RS_TRANSACTION_H
#define RENDER_SERVICE_CLIENT_CORE_UI_RS_TRANSACTION_H

#include <mutex>
#include <parcel.h>
#include <refbase.h>

#include "common/rs_common_def.h"

namespace OHOS {
namespace Rosen {

class RSTransactionProxy {
public:
    static RSTransactionProxy* GetInstance()
    {
        static std::unique_ptr<RSTransactionProxy> instance_ = std::make_unique<RSTransactionProxy>();
        return instance_.get();
    }

    void FlushImplicitTransaction(uint64_t timestamp = 0, const std::string& abilityName = "",
                                  bool dvsyncTimeUpdate = false, uint64_t dvsyncTime = 0)
    {
    }

    void Begin(uint64_t syncId = 0) {}
    void Commit(uint64_t timestamp = 0) {}
};

class RSTransactionHandler {
public:
    void FlushImplicitTransaction(uint64_t timestamp = 0, const std::string& abilityName = "",
                                  bool dvsyncTimeUpdate = false, uint64_t dvsyncTime = 0)
    {
    }

    void Begin(uint64_t syncId = 0) {}
    void Commit(uint64_t timestamp = 0) {}
    std::shared_ptr<RSTransactionHandler> GetRSTransaction()
    {
        return nullptr;
    }
};

class RSTransaction : public Parcelable {
public:
    RSTransaction() = default;
    ~RSTransaction() override = default;
    void SetTransactionHandler(std::shared_ptr<RSTransactionHandler> rsTransactionHandler)
    {
        rsTransactionHandler_ = rsTransactionHandler;
    }

    uint64_t GetSyncId()
    {
        return syncId_;
    }

    std::shared_ptr<RSTransactionHandler> rsTransactionHandler_;
    uint64_t syncId_ = 1;
};

}  // namespace Rosen
}  // namespace OHOS

#endif  // RENDER_SERVICE_CLIENT_CORE_UI_RS_TRANSACTION_H
