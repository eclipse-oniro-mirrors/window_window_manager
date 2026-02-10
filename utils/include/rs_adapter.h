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

#ifndef OHOS_ROSEN_RS_ADAPTER_H
#define OHOS_ROSEN_RS_ADAPTER_H

#include <initializer_list>
#include <unordered_set>

#include <transaction/rs_sync_transaction_controller.h>
#include <transaction/rs_sync_transaction_handler.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_base_node.h>
#include <ui/rs_node.h>
#include <ui/rs_ui_context.h>
#include <ui/rs_ui_director.h>

#define RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(...)      \
    do {                                                      \
        if (!RSAdapterUtil::IsClientMultiInstanceEnabled()) { \
            return __VA_ARGS__;                               \
        }                                                     \
    } while (false)                                           \

namespace OHOS {
namespace Rosen {
enum class InvokerType {
    RS_TRANSACTION_PROXY = 0,
    RS_TRANSACTION_HANDLER,
    RS_SYNC_TRANSACTION_CONTROLLER,
    RS_SYNC_TRANSACTION_HANDLER,
    NONE,
};

class RSTransactionAdapter {
public:
    explicit RSTransactionAdapter(const std::shared_ptr<RSUIContext>& rsUIContext);
    explicit RSTransactionAdapter(const std::shared_ptr<RSUIDirector>& rsUIDirector);
    explicit RSTransactionAdapter(const std::shared_ptr<RSNode>& rsNode);
    virtual ~RSTransactionAdapter() = default;

    std::shared_ptr<RSUIContext> GetRSUIContext() const;
    virtual void Begin();
    virtual void Commit(uint64_t timestamp = 0);
    void FlushImplicitTransaction(uint64_t timestamp = 0, const std::string& abilityName = "");

    static void FlushImplicitTransaction(const std::shared_ptr<RSUIContext>& rsUIContext,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");
    static void FlushImplicitTransaction(const std::unordered_set<std::shared_ptr<RSUIContext>>& rsUIContexts,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");
    static void FlushImplicitTransaction(std::initializer_list<std::shared_ptr<RSUIContext>> rsUIContexts,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");
    static void FlushImplicitTransaction(const std::shared_ptr<RSUIDirector>& rsUIDirector,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");
    static void FlushImplicitTransaction(std::initializer_list<std::shared_ptr<RSUIDirector>> rsUIDirectors,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");
    static void FlushImplicitTransaction(const std::shared_ptr<RSNode>& rsNode,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");
    static void FlushImplicitTransaction(std::initializer_list<std::shared_ptr<RSNode>> rsNodes,
                                         uint64_t timestamp = 0, const std::string& abilityName = "");

private:
    template<typename Func>
    void InvokeTransaction(Func&& func, const char* caller);

    static InvokerType invokerType_;
    std::shared_ptr<RSUIContext> rsUIContext_;
    RSTransactionProxy* rsTransProxy_;
    std::shared_ptr<RSTransactionHandler> rsTransHandler_;
};

class AutoRSTransaction {
public:
    explicit AutoRSTransaction(const std::shared_ptr<RSUIContext>& rsUIContext, bool enable = true);
    explicit AutoRSTransaction(const std::shared_ptr<RSNode>& rsNode, bool enable = true);
    explicit AutoRSTransaction(const std::shared_ptr<RSTransactionAdapter>& rsTransAdapter, bool enable = true);
    ~AutoRSTransaction();
    AutoRSTransaction(const AutoRSTransaction&) = delete;
    AutoRSTransaction& operator=(const AutoRSTransaction&) = delete;
    AutoRSTransaction(AutoRSTransaction&&) = delete;
    AutoRSTransaction& operator=(AutoRSTransaction&&) = delete;

private:
    std::shared_ptr<RSTransactionAdapter> rsTransAdapter_;
};

class RSSyncTransactionAdapter {
public:
    explicit RSSyncTransactionAdapter(const std::shared_ptr<RSUIContext>& rsUIContext);
    explicit RSSyncTransactionAdapter(const std::shared_ptr<RSNode>& rsNode);
    virtual ~RSSyncTransactionAdapter() = default;

    std::shared_ptr<RSUIContext> GetRSUIContext() const;
    std::shared_ptr<RSTransaction> GetRSTransaction();
    virtual void OpenSyncTransaction(const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    virtual void CloseSyncTransaction(const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);

    static std::shared_ptr<RSTransaction> GetRSTransaction(const std::shared_ptr<RSUIContext>& rsUIContext);
    static std::shared_ptr<RSTransaction> GetRSTransaction(const std::shared_ptr<RSNode>& rsNode);
    static void OpenSyncTransaction(const std::shared_ptr<RSUIContext>& rsUIContext, bool isInnerProcess,
        const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    static void OpenSyncTransaction(const std::shared_ptr<RSUIContext>& rsUIContext,
                                    const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    static void OpenSyncTransaction(const std::shared_ptr<RSNode>& rsNode,
                                    const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    static void CloseSyncTransaction(const std::shared_ptr<RSUIContext>& rsUIContext,
                                     const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    static void CloseSyncTransaction(const std::shared_ptr<RSNode>& rsNode,
                                     const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);

private:
    template<typename ReturnType, typename Func>
    ReturnType InvokeSyncTransaction(Func&& func, const char* caller);

    template<typename ReturnType, typename Func>
    static ReturnType InvokeSyncTransaction(
        const std::shared_ptr<RSUIContext>& rsUIContext, Func&& func, const char* caller);

    static InvokerType invokerType_;
    std::shared_ptr<RSUIContext> rsUIContext_;
    RSSyncTransactionController* rsSyncTransController_;
    std::shared_ptr<RSSyncTransactionHandler> rsSyncTransHandler_;
};

class AutoRSSyncTransaction {
public:
    explicit AutoRSSyncTransaction(const std::shared_ptr<RSUIContext>& rsUIContext,
                                   bool needFlushImplicitTransaction = true,
                                   const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    explicit AutoRSSyncTransaction(const std::shared_ptr<RSNode>& rsNode,
                                   bool needFlushImplicitTransaction = true,
                                   const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    explicit AutoRSSyncTransaction(const std::shared_ptr<RSSyncTransactionAdapter>& rsSyncTransAdapter,
                                   bool needFlushImplicitTransaction = true,
                                   const std::shared_ptr<AppExecFwk::EventHandler>& handler = nullptr);
    ~AutoRSSyncTransaction();
    AutoRSSyncTransaction(const AutoRSSyncTransaction&) = delete;
    AutoRSSyncTransaction& operator=(const AutoRSSyncTransaction&) = delete;
    AutoRSSyncTransaction(AutoRSSyncTransaction&&) = delete;
    AutoRSSyncTransaction& operator=(AutoRSSyncTransaction&&) = delete;

private:
    std::shared_ptr<RSSyncTransactionAdapter> rsSyncTransAdapter_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

class AllowInMultiThreadGuard {
public:
    explicit AllowInMultiThreadGuard(const std::shared_ptr<RSNode>& rsNode);
    ~AllowInMultiThreadGuard();
    AllowInMultiThreadGuard(const AllowInMultiThreadGuard&) = delete;
    AllowInMultiThreadGuard& operator=(const AllowInMultiThreadGuard&) = delete;
    AllowInMultiThreadGuard(AllowInMultiThreadGuard&&) = delete;
    AllowInMultiThreadGuard& operator=(AllowInMultiThreadGuard&&) = delete;

private:
    std::shared_ptr<RSNode> rsNode_;
};

class RSAdapterUtil {
public:
    static bool IsClientMultiInstanceEnabled();
    static void InitRSUIDirector(std::shared_ptr<RSUIDirector>& rsUIDirector,
                                 bool shouldCreateRenderThread = true,
                                 bool isMultiInstance = false,
                                 const std::shared_ptr<RSUIContext>& rsUiContext = nullptr);
    static void SetRSUIContext(const std::shared_ptr<RSNode>& rsNode,
                               const std::shared_ptr<RSUIContext>& rsUIContext,
                               bool skipCheckInMultiInstance = false);
    static void SetRSTransactionHandler(const std::shared_ptr<RSTransaction>& rsTransaction,
                                        const std::shared_ptr<RSUIContext>& rsUIContext);
    static void SetSkipCheckInMultiInstance(const std::shared_ptr<RSNode>& rsNode,
                                            bool skipCheckInMultiInstance);
    static const std::shared_ptr<RSBaseNode> GetRSNode(
        const std::shared_ptr<RSUIContext>& rsUIContext, NodeId id);
    static std::string RSUIContextToStr(const std::shared_ptr<RSUIContext>& rsUIContext);
    static std::string RSNodeToStr(const std::shared_ptr<RSNode>& rsNode);
    static std::string RSUIDirectorToStr(const std::shared_ptr<RSUIDirector>& rsUIDirector);
};

inline void RunIfRSClientMultiInstanceEnabled(const std::function<void()>& action)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED();
    action();
}
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_RS_ADAPTER_H