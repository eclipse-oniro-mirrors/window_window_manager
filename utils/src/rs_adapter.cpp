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

#include "rs_adapter.h"

#include <parameters.h>

#include "window_manager_hilog.h"

#define RETURN_IF_PARAM_IS_NULL(param, ...)                              \
    do {                                                                 \
        if (!param) {                                                    \
            TLOGE(WmsLogTag::WMS_SCB, "The %{public}s is null", #param); \
            return __VA_ARGS__;                                          \
        }                                                                \
    } while (false)                                                      \

namespace OHOS {
namespace Rosen {

InvokerType RSTransactionAdapter::invokerType_ = InvokerType::NONE;

// LCOV_EXCL_START
RSTransactionAdapter::RSTransactionAdapter(const std::shared_ptr<RSUIContext>& rsUIContext)
{
    rsUIContext_ = rsUIContext;
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        rsTransHandler_ = rsUIContext_ ? rsUIContext_->GetRSTransaction() : nullptr;
        rsTransProxy_ = nullptr;
        TLOGD(WmsLogTag::WMS_SCB, "Use RSTransactionHandler: %{public}s",
              RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str());
    } else {
        rsTransHandler_ = nullptr;
        rsTransProxy_ = RSTransactionProxy::GetInstance();
        TLOGD(WmsLogTag::WMS_SCB, "Fallback to RSTransactionProxy");
    }
}

RSTransactionAdapter::RSTransactionAdapter(const std::shared_ptr<RSNode>& rsNode) :
    RSTransactionAdapter(rsNode ? rsNode->GetRSUIContext() : nullptr) {}

RSTransactionAdapter::RSTransactionAdapter(const std::shared_ptr<RSUIDirector>& rsUIDirector) :
    RSTransactionAdapter(rsUIDirector ? rsUIDirector->GetRSUIContext() : nullptr) {}

std::shared_ptr<RSUIContext> RSTransactionAdapter::GetRSUIContext() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    return rsUIContext_;
}

template<typename Func>
void RSTransactionAdapter::InvokeTransaction(Func&& func, const char* caller)
{
    invokerType_ = InvokerType::NONE;
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        RETURN_IF_PARAM_IS_NULL(rsTransHandler_);
        invokerType_ = InvokerType::RS_TRANSACTION_HANDLER;
        TLOGND(WmsLogTag::WMS_SCB, "%{public}s: Use RSTransactionHandler: %{public}s",
               caller, RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str());
        func(rsTransHandler_);
    } else {
        RETURN_IF_PARAM_IS_NULL(rsTransProxy_);
        invokerType_ = InvokerType::RS_TRANSACTION_PROXY;
        TLOGND(WmsLogTag::WMS_SCB, "%{public}s: Fallback to RSTransactionProxy", caller);
        func(rsTransProxy_);
    }
}

void RSTransactionAdapter::Begin()
{
    InvokeTransaction(
        [](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->Begin();
        }, __func__);
}

void RSTransactionAdapter::Commit(uint64_t timestamp)
{
    InvokeTransaction(
        [timestamp](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->Commit(timestamp);
        }, __func__);
}

void RSTransactionAdapter::FlushImplicitTransaction(uint64_t timestamp, const std::string& abilityName)
{
    InvokeTransaction(
        [timestamp, &abilityName](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->FlushImplicitTransaction(timestamp, abilityName);
        }, __func__);
}
// LCOV_EXCL_STOP

void RSTransactionAdapter::FlushImplicitTransaction(
    const std::shared_ptr<RSUIContext>& rsUIContext, uint64_t timestamp, const std::string& abilityName)
{
    invokerType_ = InvokerType::NONE;
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        auto rsTransHandler = rsUIContext ? rsUIContext->GetRSTransaction() : nullptr;
        RETURN_IF_PARAM_IS_NULL(rsTransHandler);
        invokerType_ = InvokerType::RS_TRANSACTION_HANDLER;
        TLOGD(WmsLogTag::WMS_SCB, "Use RSTransactionHandler: %{public}s",
              RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str());
        rsTransHandler->FlushImplicitTransaction(timestamp, abilityName);
    } else {
        auto rsTransProxy = RSTransactionProxy::GetInstance();
        RETURN_IF_PARAM_IS_NULL(rsTransProxy);
        invokerType_ = InvokerType::RS_TRANSACTION_PROXY;
        TLOGD(WmsLogTag::WMS_SCB, "Fallback to RSTransactionProxy");
        rsTransProxy->FlushImplicitTransaction(timestamp, abilityName);
    }
}

// LCOV_EXCL_START
void RSTransactionAdapter::FlushImplicitTransaction(
    const std::unordered_set<std::shared_ptr<RSUIContext>>& rsUIContexts,
    uint64_t timestamp, const std::string& abilityName)
{
    if (!RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        FlushImplicitTransaction(std::shared_ptr<RSUIContext>(nullptr), timestamp, abilityName);
        return;
    }
    for (const auto& rsUIContext : rsUIContexts) {
        FlushImplicitTransaction(rsUIContext, timestamp, abilityName);
    }
}

void RSTransactionAdapter::FlushImplicitTransaction(
    std::initializer_list<std::shared_ptr<RSUIContext>> rsUIContexts,
    uint64_t timestamp, const std::string& abilityName)
{
    if (!RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        FlushImplicitTransaction(std::shared_ptr<RSUIContext>(nullptr), timestamp, abilityName);
        return;
    }
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContextSet;
    for (const auto& rsUIContext : rsUIContexts) {
        rsUIContextSet.insert(rsUIContext);
    }
    FlushImplicitTransaction(rsUIContextSet, timestamp, abilityName);
}

void RSTransactionAdapter::FlushImplicitTransaction(
    const std::shared_ptr<RSUIDirector>& rsUIDirector, uint64_t timestamp, const std::string& abilityName)
{
    FlushImplicitTransaction(rsUIDirector ? rsUIDirector->GetRSUIContext() : nullptr, timestamp, abilityName);
}

void RSTransactionAdapter::FlushImplicitTransaction(
    std::initializer_list<std::shared_ptr<RSUIDirector>> rsUIDirectors,
    uint64_t timestamp, const std::string& abilityName)
{
    if (!RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        FlushImplicitTransaction(std::shared_ptr<RSUIContext>(nullptr), timestamp, abilityName);
        return;
    }
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContexts;
    for (const auto& rsUIDirector : rsUIDirectors) {
        rsUIContexts.insert(rsUIDirector ? rsUIDirector->GetRSUIContext() : nullptr);
    }
    FlushImplicitTransaction(rsUIContexts, timestamp, abilityName);
}
// LCOV_EXCL_STOP

void RSTransactionAdapter::FlushImplicitTransaction(
    const std::shared_ptr<RSNode>& rsNode, uint64_t timestamp, const std::string& abilityName)
{
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
    FlushImplicitTransaction(rsNode ? rsNode->GetRSUIContext() : nullptr, timestamp, abilityName);
}

// LCOV_EXCL_START
void RSTransactionAdapter::FlushImplicitTransaction(
    std::initializer_list<std::shared_ptr<RSNode>> rsNodes,
    uint64_t timestamp, const std::string& abilityName)
{
    if (!RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        FlushImplicitTransaction(std::shared_ptr<RSUIContext>(nullptr), timestamp, abilityName);
        return;
    }
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContexts;
    for (const auto& rsNode : rsNodes) {
        TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
        rsUIContexts.insert(rsNode ? rsNode->GetRSUIContext() : nullptr);
    }
    FlushImplicitTransaction(rsUIContexts, timestamp, abilityName);
}

AutoRSTransaction::AutoRSTransaction(const std::shared_ptr<RSUIContext>& rsUIContext, bool enable)
{
    if (enable) {
        rsTransAdapter_ = std::make_shared<RSTransactionAdapter>(rsUIContext);
        rsTransAdapter_->Begin();
    } else {
        rsTransAdapter_ = nullptr;
    }
}

AutoRSTransaction::AutoRSTransaction(const std::shared_ptr<RSNode>& rsNode, bool enable) :
    AutoRSTransaction(rsNode ? rsNode->GetRSUIContext() : nullptr, enable)
{
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
}

AutoRSTransaction::AutoRSTransaction(const std::shared_ptr<RSTransactionAdapter>& rsTransAdapter, bool enable)
{
    if (rsTransAdapter != nullptr && enable) {
        rsTransAdapter_ = rsTransAdapter;
        rsTransAdapter_->Begin();
    } else {
        rsTransAdapter_ = nullptr;
    }
}

AutoRSTransaction::~AutoRSTransaction()
{
    if (rsTransAdapter_) {
        rsTransAdapter_->Commit();
    }
}

InvokerType RSSyncTransactionAdapter::invokerType_ = InvokerType::NONE;

RSSyncTransactionAdapter::RSSyncTransactionAdapter(const std::shared_ptr<RSUIContext>& rsUIContext)
{
    rsUIContext_ = rsUIContext;
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        rsSyncTransHandler_ = rsUIContext_ ? rsUIContext_->GetSyncTransactionHandler() : nullptr;
        rsSyncTransController_ = nullptr;
        TLOGD(WmsLogTag::WMS_SCB, "Use RSSyncTransactionHandler: %{public}s",
              RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str());
    } else {
        rsSyncTransHandler_ = nullptr;
        rsSyncTransController_ = RSSyncTransactionController::GetInstance();
        TLOGD(WmsLogTag::WMS_SCB, "Fallback to RSSyncTransactionController");
    }
}

RSSyncTransactionAdapter::RSSyncTransactionAdapter(const std::shared_ptr<RSNode>& rsNode) :
    RSSyncTransactionAdapter(rsNode ? rsNode->GetRSUIContext() : nullptr) {}

template<typename ReturnType, typename Func>
ReturnType RSSyncTransactionAdapter::InvokeSyncTransaction(Func&& func, const char* caller)
{
    invokerType_ = InvokerType::NONE;
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        if (rsSyncTransHandler_) {
            invokerType_ = InvokerType::RS_SYNC_TRANSACTION_HANDLER;
            TLOGND(WmsLogTag::WMS_SCB, "%{public}s: Use RSSyncTransactionHandler: %{public}s",
                   caller, RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str());
            return func(rsSyncTransHandler_);
        }
    } else {
        if (rsSyncTransController_) {
            invokerType_ = InvokerType::RS_SYNC_TRANSACTION_CONTROLLER;
            TLOGND(WmsLogTag::WMS_SCB, "%{public}s: Fallback to RSSyncTransactionController", caller);
            return func(rsSyncTransController_);
        }
    }
    TLOGNE(WmsLogTag::WMS_SCB,
           "%{public}s: Failed to invoke RSSyncTransaction", caller);
    if constexpr (!std::is_void_v<ReturnType>) {
        return nullptr;
    }
}

std::shared_ptr<RSUIContext> RSSyncTransactionAdapter::GetRSUIContext() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str());
    return rsUIContext_;
}

std::shared_ptr<RSTransaction> RSSyncTransactionAdapter::GetRSTransaction()
{
    return InvokeSyncTransaction<std::shared_ptr<RSTransaction>>(
        [](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker, std::shared_ptr<RSTransaction>(nullptr));
            return invoker->GetRSTransaction();
        },
        __func__);
}

void RSSyncTransactionAdapter::OpenSyncTransaction(const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    InvokeSyncTransaction<void>(
        [handler](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->OpenSyncTransaction(handler);
        },
        __func__);
}

void RSSyncTransactionAdapter::CloseSyncTransaction(const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    InvokeSyncTransaction<void>(
        [handler](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->CloseSyncTransaction(handler);
        },
        __func__);
}

template<typename ReturnType, typename Func>
ReturnType RSSyncTransactionAdapter::InvokeSyncTransaction(
    const std::shared_ptr<RSUIContext>& rsUIContext, Func&& func, const char* caller)
{
    invokerType_ = InvokerType::NONE;
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        auto rsSyncTransHandler = rsUIContext ? rsUIContext->GetSyncTransactionHandler() : nullptr;
        if (rsSyncTransHandler) {
            invokerType_ = InvokerType::RS_SYNC_TRANSACTION_HANDLER;
            TLOGND(WmsLogTag::WMS_SCB, "%{public}s: Use RSSyncTransactionHandler: %{public}s",
                   caller, RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str());
            return func(rsSyncTransHandler);
        }
    } else {
        auto rsSyncTransController = RSSyncTransactionController::GetInstance();
        if (rsSyncTransController) {
            invokerType_ = InvokerType::RS_SYNC_TRANSACTION_CONTROLLER;
            TLOGND(WmsLogTag::WMS_SCB, "%{public}s: Fallback to RSSyncTransactionController", caller);
            return func(rsSyncTransController);
        }
    }
    TLOGNE(WmsLogTag::WMS_SCB,
           "%{public}s: Failed to invoke RSSyncTransaction", caller);
    if constexpr (!std::is_void_v<ReturnType>) {
        return nullptr;
    }
}

std::shared_ptr<RSTransaction> RSSyncTransactionAdapter::GetRSTransaction(
    const std::shared_ptr<RSUIContext>& rsUIContext)
{
    return InvokeSyncTransaction<std::shared_ptr<RSTransaction>>(
        rsUIContext,
        [](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker, std::shared_ptr<RSTransaction>(nullptr));
            return invoker->GetRSTransaction();
        },
        __func__);
}

std::shared_ptr<RSTransaction> RSSyncTransactionAdapter::GetRSTransaction(const std::shared_ptr<RSNode>& rsNode)
{
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
    return GetRSTransaction(rsNode ? rsNode->GetRSUIContext() : nullptr);
}

void RSSyncTransactionAdapter::OpenSyncTransaction(const std::shared_ptr<RSUIContext>& rsUIContext,
    bool isInnerProcess, const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    InvokeSyncTransaction<void>(
        rsUIContext,
        [isInnerProcess, handler](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->OpenSyncTransaction(handler, isInnerProcess);
        },
        __func__);
}

void RSSyncTransactionAdapter::OpenSyncTransaction(
    const std::shared_ptr<RSUIContext>& rsUIContext, const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    InvokeSyncTransaction<void>(
        rsUIContext,
        [handler](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->OpenSyncTransaction(handler);
        },
        __func__);
}

void RSSyncTransactionAdapter::OpenSyncTransaction(
    const std::shared_ptr<RSNode>& rsNode, const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
    OpenSyncTransaction(rsNode ? rsNode->GetRSUIContext() : nullptr, handler);
}

void RSSyncTransactionAdapter::CloseSyncTransaction(
    const std::shared_ptr<RSUIContext>& rsUIContext, const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    InvokeSyncTransaction<void>(
        rsUIContext,
        [handler](auto& invoker) {
            RETURN_IF_PARAM_IS_NULL(invoker);
            invoker->CloseSyncTransaction(handler);
        },
        __func__);
}

void RSSyncTransactionAdapter::CloseSyncTransaction(
    const std::shared_ptr<RSNode>& rsNode, const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
    CloseSyncTransaction(rsNode ? rsNode->GetRSUIContext() : nullptr, handler);
}

AutoRSSyncTransaction::AutoRSSyncTransaction(
    const std::shared_ptr<RSUIContext>& rsUIContext,
    bool needFlushImplicitTransaction,
    const std::shared_ptr<AppExecFwk::EventHandler>& handler) : handler_(handler)
{
    if (needFlushImplicitTransaction) {
        RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
    }
    rsSyncTransAdapter_ = std::make_shared<RSSyncTransactionAdapter>(rsUIContext);
    rsSyncTransAdapter_->OpenSyncTransaction(handler_);
}

AutoRSSyncTransaction::AutoRSSyncTransaction(
    const std::shared_ptr<RSNode>& rsNode,
    bool needFlushImplicitTransaction,
    const std::shared_ptr<AppExecFwk::EventHandler>& handler) :
    AutoRSSyncTransaction(rsNode ? rsNode->GetRSUIContext() : nullptr, needFlushImplicitTransaction, handler)
{
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s", RSAdapterUtil::RSNodeToStr(rsNode).c_str());
}

AutoRSSyncTransaction::AutoRSSyncTransaction(
    const std::shared_ptr<RSSyncTransactionAdapter>& rsSyncTransAdapter,
    bool needFlushImplicitTransaction,
    const std::shared_ptr<AppExecFwk::EventHandler>& handler) :
    rsSyncTransAdapter_(rsSyncTransAdapter), handler_(handler)
{
    if (rsSyncTransAdapter_) {
        if (needFlushImplicitTransaction) {
            RSTransactionAdapter::FlushImplicitTransaction(rsSyncTransAdapter_->GetRSUIContext());
        }
        rsSyncTransAdapter_->OpenSyncTransaction(handler);
    }
}

AutoRSSyncTransaction::~AutoRSSyncTransaction()
{
    if (rsSyncTransAdapter_) {
        rsSyncTransAdapter_->CloseSyncTransaction(handler_);
    }
}

AllowInMultiThreadGuard::AllowInMultiThreadGuard(const std::shared_ptr<RSNode>& rsNode) : rsNode_(rsNode)
{
    if (rsNode_) {
        TLOGD(WmsLogTag::WMS_SCB,
              "Skip check in RS multi-instance: %{public}s", RSAdapterUtil::RSNodeToStr(rsNode_).c_str());
        rsNode_->SetSkipCheckInMultiInstance(true);
    }
}

AllowInMultiThreadGuard::~AllowInMultiThreadGuard()
{
    if (rsNode_) {
        TLOGD(WmsLogTag::WMS_SCB,
              "Reopen check in RS multi-instance: %{public}s", RSAdapterUtil::RSNodeToStr(rsNode_).c_str());
        rsNode_->SetSkipCheckInMultiInstance(false);
    }
}
// LCOV_EXCL_STOP

bool RSAdapterUtil::IsClientMultiInstanceEnabled()
{
    static bool enabled = [] {
        bool value = system::GetParameter("persist.rosen.rsclientmultiinstance.enabled", "1") != "0";
        TLOGNI(WmsLogTag::WMS_SCB, "RS multi-instance enabled: %{public}d", value);
        return value;
    }();
    return enabled;
}

void RSAdapterUtil::InitRSUIDirector(std::shared_ptr<RSUIDirector>& rsUIDirector,
    sptr<IRemoteObject> connectToRenderToken, const std::shared_ptr<RSUIContext>& rsUiContext)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED();
    if (rsUIDirector) {
        TLOGW(WmsLogTag::WMS_SCB,
              "RSUIDirector already exists: %{public}s", RSAdapterUtil::RSUIDirectorToStr(rsUIDirector).c_str());
        return;
    }
    rsUIDirector = RSUIDirector::Create(connectToRenderToken, rsUiContext);
    if (rsUIDirector) {
        TLOGI(WmsLogTag::WMS_SCB, "Create RSUIDirector: %{public}s, rsUIContext: %{public}s",
              RSAdapterUtil::RSUIDirectorToStr(rsUIDirector).c_str(),
              RSAdapterUtil::RSUIContextToStr(rsUiContext).c_str());
    } else {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to create RSUIDirector");
    }
}

// LCOV_EXCL_START
void RSAdapterUtil::SetRSUIContext(const std::shared_ptr<RSNode>& rsNode,
                                   const std::shared_ptr<RSUIContext>& rsUIContext,
                                   bool skipCheckInMultiInstance)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED();
    if (!rsNode) {
        TLOGE(WmsLogTag::WMS_SCB, "RSNode is null");
        return;
    }
    // The creation of RSUIContext and the use of RSNode may occur in different
    // threads, so skip checks can be set.
    rsNode->SetSkipCheckInMultiInstance(skipCheckInMultiInstance);
    if (!rsUIContext) {
        TLOGE(WmsLogTag::WMS_SCB, "RSUIContext is null, %{public}s",
              RSAdapterUtil::RSNodeToStr(rsNode).c_str());
        return;
    }
    auto originalRSUIContext = rsNode->GetRSUIContext();
    if (rsUIContext == originalRSUIContext) {
        TLOGD(WmsLogTag::WMS_SCB, "RSNode already has the same RSUIContext, %{public}s",
              RSAdapterUtil::RSNodeToStr(rsNode).c_str());
        return;
    }
    rsNode->SetRSUIContext(rsUIContext);
    TLOGD(WmsLogTag::WMS_SCB,
          "%{public}s, skipCheckInMultiInstance = %{public}d, original %{public}s",
          RSAdapterUtil::RSNodeToStr(rsNode).c_str(), skipCheckInMultiInstance,
          RSAdapterUtil::RSUIContextToStr(originalRSUIContext).c_str());
}
// LCOV_EXCL_STOP

void RSAdapterUtil::SetRSTransactionHandler(const std::shared_ptr<RSTransaction>& rsTransaction,
                                            const std::shared_ptr<RSUIContext>& rsUIContext)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED();
    RETURN_IF_PARAM_IS_NULL(rsTransaction);
    RETURN_IF_PARAM_IS_NULL(rsUIContext);
    auto rsTransHandler = rsUIContext->GetRSTransaction();
    RETURN_IF_PARAM_IS_NULL(rsTransHandler);
    rsTransaction->SetTransactionHandler(rsTransHandler);
    TLOGD(WmsLogTag::WMS_SCB, "syncId: %{public}" PRIu64 ", %{public}s",
        rsTransaction->GetSyncId(), RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str());
}

void RSAdapterUtil::SetSkipCheckInMultiInstance(const std::shared_ptr<RSNode>& rsNode,
                                                bool skipCheckInMultiInstance)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED();
    RETURN_IF_PARAM_IS_NULL(rsNode);
    rsNode->SetSkipCheckInMultiInstance(skipCheckInMultiInstance);
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, skipCheckInMultiInstance = %{public}d",
          RSAdapterUtil::RSNodeToStr(rsNode).c_str(), skipCheckInMultiInstance);
}

// LCOV_EXCL_START
const std::shared_ptr<RSBaseNode> RSAdapterUtil::GetRSNode(
    const std::shared_ptr<RSUIContext>& rsUIContext, NodeId id)
{
    if (!RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        return RSNodeMap::Instance().GetNode(id);
    }
    RETURN_IF_PARAM_IS_NULL(rsUIContext, nullptr);
    return rsUIContext->GetNodeMap().GetNode(id);
}
// LCOV_EXCL_STOP

std::string RSAdapterUtil::RSUIContextToStr(const std::shared_ptr<RSUIContext>& rsUIContext)
{
    if (!rsUIContext) {
        return "RSUIContext is null";
    }
    std::ostringstream oss;
    oss << "RSUIContext [token: " << rsUIContext->GetToken()
        << ", tid: "
        << static_cast<int32_t>(rsUIContext->GetToken() >> 32) // 32: tid's offset position in the token
        << "]";
    return oss.str();
}

std::string RSAdapterUtil::RSNodeToStr(const std::shared_ptr<RSNode>& rsNode)
{
    if (!rsNode) {
        return "RSNode is null";
    }
    std::ostringstream oss;
    oss << "RSNode [id: " << rsNode->GetId() << "], "
        << RSUIContextToStr(rsNode->GetRSUIContext());
    return oss.str();
}

std::string RSAdapterUtil::RSUIDirectorToStr(const std::shared_ptr<RSUIDirector>& rsUIDirector)
{
    if (!rsUIDirector) {
        return "RSUIDirector is null";
    }
    std::ostringstream oss;
    oss << "RSUIDirector's " << RSUIContextToStr(rsUIDirector->GetRSUIContext());
    return oss.str();
}
} // namespace Rosen
} // namespace OHOS
