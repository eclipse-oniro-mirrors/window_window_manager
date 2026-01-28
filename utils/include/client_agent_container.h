/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_CLIENT_AGENT_MANAGER_H
#define OHOS_ROSEN_CLIENT_AGENT_MANAGER_H

#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include "agent_death_recipient.h"
#include "window_manager_hilog.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Rosen {
constexpr int32_t INVALID_PID_ID = -1;
template <typename T1, typename T2>
class ClientAgentContainer {
public:
    ClientAgentContainer();
    virtual ~ClientAgentContainer() = default;

    bool RegisterAgent(const sptr<T1>& agent, T2 type);
    bool RegisterAttributeAgent(uintptr_t key, const sptr<T1>& agent, std::vector<T2>& attributes);
    bool UnregisterAgent(const sptr<T1>& agent, T2 type);
    bool UnRegisterAllAttributeAgent(uintptr_t key, const sptr<T1>& agent);
    bool UnRegisterAttribute(uintptr_t key, const sptr<T1>& agent, const std::vector<T2>& attributesOff);
    std::set<sptr<T1>> GetAgentsByType(T2 type);
    void SetAgentDeathCallback(std::function<void(const sptr<IRemoteObject>&)> callback);
    int32_t GetAgentPid(const sptr<T1>& agent);
    std::map<uintptr_t, std::pair<sptr<T1>, std::set<T2>>> GetAttributeAgentsMap();

private:
    void RemoveAgent(const sptr<IRemoteObject>& remoteObject);
    bool UnregisterAgentLocked(std::set<sptr<T1>>& agents, const sptr<IRemoteObject>& agent);

    static constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "ClientAgentContainer"};

    struct finder_t {
        explicit finder_t(sptr<IRemoteObject> remoteObject) : remoteObject_(remoteObject) {}

        bool operator()(sptr<T1> agent)
        {
            if (agent == nullptr) {
                WLOGFE("agent is invalid");
                return false;
            }
            return agent->AsObject() == remoteObject_;
        }

        sptr<IRemoteObject> remoteObject_;
    };

    std::recursive_mutex mutex_;
    std::map<T2, std::set<sptr<T1>>> agentMap_;
    std::map<uintptr_t, std::pair<sptr<T1>, std::set<T2>>> attributeAgentMap_;
    std::map<sptr<T1>, int32_t> agentPidMap_;
    sptr<AgentDeathRecipient> deathRecipient_;
    std::function<void(const sptr<IRemoteObject>&)> deathCallback_;
};

template<typename T1, typename T2>
ClientAgentContainer<T1, T2>::ClientAgentContainer() : deathRecipient_(
    new AgentDeathRecipient([this](const sptr<IRemoteObject>& remoteObject) { this->RemoveAgent(remoteObject); })) {}

template<typename T1, typename T2>
bool ClientAgentContainer<T1, T2>::RegisterAgent(const sptr<T1>& agent, T2 type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return false;
    }
    agentMap_[type].insert(agent);
    agentPidMap_[agent] = IPCSkeleton::GetCallingPid();
    if (deathRecipient_ == nullptr || !agent->AsObject()->AddDeathRecipient(deathRecipient_)) {
        WLOGFI("failed to add death recipient");
    }
    return true;
}

template<typename T1, typename T2>
bool ClientAgentContainer<T1, T2>::RegisterAttributeAgent(uintptr_t key, const sptr<T1>& agent,
    std::vector<T2>& attributes)
{
    TLOGI(WmsLogTag::DMS, "called");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return false;
    }
    auto it = attributeAgentMap_.find(key);
    if (it != attributeAgentMap_.end()) {
        for (const auto& attr : attributes) {
            it->second.second.insert(attr);
        }
    } else {
        std::set<std::string> attrSet(attributes.begin(), attributes.end());
        attributeAgentMap_[key] = {agent, attrSet};
        agentPidMap_[agent] = IPCSkeleton::GetCallingPid();
    }
 
    auto iter = attributeAgentMap_.find(key);
    if (iter != attributeAgentMap_.end()) {
        std::ostringstream oss;
        oss << "current listened attribute list:[";
        for (const auto attribute : iter->second.second) {
            oss << attribute << ",";
        }
        TLOGI(WmsLogTag::DMS, "%{public}s]", oss.str().c_str());
    }
    if (deathRecipient_ == nullptr || !agent->AsObject()->AddDeathRecipient(deathRecipient_)) {
        WLOGFI("failed to add death recipient");
    }
    return true;
}

template<typename T1, typename T2>
bool ClientAgentContainer<T1, T2>::UnRegisterAllAttributeAgent(uintptr_t key, const sptr<T1>& agent)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return false;
    }
    if (attributeAgentMap_.count(key) == 0) {
        WLOGFD("repeat unregister agent");
        return true;
    }
    auto agentPidIt = agentPidMap_.find(agent);
    if (agentPidIt != agentPidMap_.end()) {
        int32_t agentPid = agentPidMap_[agent];
        agentPidMap_.erase(agentPidIt);
        WLOGFD("agent pid: %{public}d unregistered", agentPid);
    }
    attributeAgentMap_.erase(key);
    agent->AsObject()->RemoveDeathRecipient(deathRecipient_);
    return true;
}

template<typename T1, typename T2>
bool ClientAgentContainer<T1, T2>::UnRegisterAttribute(uintptr_t key, const sptr<T1>& agent,
    const std::vector<T2>& attributesOff)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return false;
    }
    if (attributeAgentMap_.count(key) == 0) {
        WLOGFD("repeat unregister agent");
        return true;
    }
    auto& attributes = attributeAgentMap_.at(key).second;
    for (auto attribute : attributesOff) {
        auto iter = attributes.find(attribute);
        if (iter == attributes.end()) {
            WLOGFD("could not find this attribute: %{public}s", attribute.c_str());
            continue;
        }
        attributes.erase(iter);
    }
    return true;
}

template<typename T1, typename T2>
std::map<uintptr_t, std::pair<sptr<T1>, std::set<T2>>> ClientAgentContainer<T1, T2>::GetAttributeAgentsMap()
{
    return attributeAgentMap_;
}

template<typename T1, typename T2>
bool ClientAgentContainer<T1, T2>::UnregisterAgent(const sptr<T1>& agent, T2 type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return false;
    }
    if (agentMap_.count(type) == 0) {
        WLOGFD("repeat unregister agent");
        return true;
    }
    auto& agents = agentMap_.at(type);
    UnregisterAgentLocked(agents, agent->AsObject());
    agent->AsObject()->RemoveDeathRecipient(deathRecipient_);
    return true;
}

template<typename T1, typename T2>
std::set<sptr<T1>> ClientAgentContainer<T1, T2>::GetAgentsByType(T2 type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agentMap_.count(type) == 0) {
        WLOGFD("no such type of agent registered! type:%{public}u", type);
        return std::set<sptr<T1>>();
    }
    return agentMap_.at(type);
}

template<typename T1, typename T2>
bool ClientAgentContainer<T1, T2>::UnregisterAgentLocked(std::set<sptr<T1>>& agents,
    const sptr<IRemoteObject>& agent)
{
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return false;
    }
    auto iter = std::find_if(agents.begin(), agents.end(), finder_t(agent));
    if (iter == agents.end()) {
        WLOGFD("could not find this agent");
        return false;
    }
    auto agentPidIt = agentPidMap_.find(*iter);
    if (agentPidIt != agentPidMap_.end()) {
        int32_t agentPid = agentPidMap_[*iter];
        agentPidMap_.erase(agentPidIt);
        WLOGFD("agent pid: %{public}d unregistered", agentPid);
    }
    agents.erase(iter);
    WLOGFD("agent unregistered");
    return true;
}

template<typename T1, typename T2>
void ClientAgentContainer<T1, T2>::RemoveAgent(const sptr<IRemoteObject>& remoteObject)
{
    TLOGD(WmsLogTag::DEFAULT, "Enter");
    if (remoteObject == nullptr) {
        WLOGFE("remoteObject is invalid");
        return;
    }
    if (deathCallback_ != nullptr) {
        deathCallback_(remoteObject);
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    static bool isEntryAgain = false;
    if (isEntryAgain) {
        WLOGFW("UnregisterAgentLocked entry again");
    }
    isEntryAgain = true;
    for (auto& elem : agentMap_) {
        if (UnregisterAgentLocked(elem.second, remoteObject)) {
            break;
        }
    }
    remoteObject->RemoveDeathRecipient(deathRecipient_);
    isEntryAgain = false;
}

template<typename T1, typename T2>
void ClientAgentContainer<T1, T2>::SetAgentDeathCallback(std::function<void(const sptr<IRemoteObject>&)> callback)
{
    deathCallback_ = callback;
}

template<typename T1, typename T2>
int32_t ClientAgentContainer<T1, T2>::GetAgentPid(const sptr<T1>& agent)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (agent == nullptr) {
        WLOGFE("agent is invalid");
        return INVALID_PID_ID;
    }
    if (agentPidMap_.count(agent) == 0) {
        WLOGFE("agent pid not found");
        return INVALID_PID_ID;
    }
    return agentPidMap_[agent];
}
}
}
#endif // OHOS_ROSEN_CLIENT_AGENT_MANAGER_H
