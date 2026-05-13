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

#ifndef RENDER_SERVICE_CLIENT_CORE_UI_RS_UI_CONTEXT_H
#define RENDER_SERVICE_CLIENT_CORE_UI_RS_UI_CONTEXT_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include "transaction/rs_transaction.h"
#include "transaction/rs_sync_transaction_handler.h"
#include "rs_node.h"
#include "pipeline/rs_node_map.h"
#include "transaction/rs_render_interface.h"

namespace OHOS {
namespace Rosen {

class RSNodeMapV2 {
public:
    const std::shared_ptr<RSBaseNode> GetNode(NodeId id) const
    {
        return std::make_shared<RSBaseNode>();
    }
};
class RSUIContext : public std::enable_shared_from_this<RSUIContext> {
public:
    RSUIContext()
    {
        static uint64_t cnt_ = 1;
        token_ = cnt_++;
    }
    virtual ~RSUIContext() = default;

    uint64_t GetToken()
    {
        return token_;
    }

    std::shared_ptr<RSTransactionHandler> GetRSTransaction()
    {
        return rsTransactionHandler_;
    }

    std::shared_ptr<RSSyncTransactionHandler> GetSyncTransactionHandler()
    {
        return rsSyncTransactionHandler_;
    }

    inline const RSNodeMapV2& GetNodeMap() const
    {
        return nodeMap_;
    }

    std::shared_ptr<RSRenderInterface> GetRSRenderInterface()
    {
        return rsRenderInterface_;
    }

    uint64_t token_;
    std::shared_ptr<RSTransactionHandler> rsTransactionHandler_;
    std::shared_ptr<RSSyncTransactionHandler> rsSyncTransactionHandler_;
    RSNodeMapV2 nodeMap_;
    std::shared_ptr<RSRenderInterface> rsRenderInterface_;
};

}  // namespace Rosen
}  // namespace OHOS

/** @} */
#endif  // RENDER_SERVICE_CLIENT_CORE_UI_RS_UI_CONTEXT_H