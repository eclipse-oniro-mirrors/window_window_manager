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
#include "window_inspector.h"

#include <dlfcn.h>

#include "nlohmann/json.hpp"

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr char ARK_CONNECT_LIB_PATH[] = "libark_connect_inspector.z.so";
constexpr char SET_WMS_CALLBACK[] = "SetWMSCallback";
constexpr char SEND_WMS_MESSAGE[] = "SendMessage";
const std::string METHOD_NAME = "WMS.windowList";
const std::string COMMAND_NAME = "getCurrentProcessWindowList";

void CreateArkUIInspectorJson(
    const std::vector<WindowListInfo>& windowListInfo, std::string& jsonStr)
{
    nlohmann::ordered_json jsonWindowListInfo;
    jsonWindowListInfo["type"] = "window";
    jsonWindowListInfo["content"] = nlohmann::json::array();
    for (const auto& info : windowListInfo) {
        nlohmann::ordered_json jsonWindowInfo;
        jsonWindowInfo["windowName"] = info.windowName;
        jsonWindowInfo["winId"] = std::to_string(info.windowId);
        jsonWindowInfo["type"] = std::to_string(info.windowType);
        nlohmann::ordered_json jsonRectInfo;
        jsonRectInfo["startX"] = std::to_string(info.windowRect.posX_);
        jsonRectInfo["startY"] = std::to_string(info.windowRect.posY_);
        jsonRectInfo["width"] = std::to_string(info.windowRect.width_);
        jsonRectInfo["height"] = std::to_string(info.windowRect.height_);
        jsonWindowInfo["rect"] = jsonRectInfo;

        jsonWindowListInfo["content"].push_back(std::move(jsonWindowInfo));
    }
    jsonStr = jsonWindowListInfo.dump();
}
} // namespace

WindowInspector& WindowInspector::GetInstance()
{
    static WindowInspector instance;
    return instance;
}

WindowInspector::WindowInspector()
{
    ConnectServer();
}

void WindowInspector::ConnectServer()
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "start connect");
    handlerConnectServerSo_ = dlopen(ARK_CONNECT_LIB_PATH, RTLD_NOLOAD);
    if (handlerConnectServerSo_ == nullptr) {
        handlerConnectServerSo_ = dlopen(ARK_CONNECT_LIB_PATH, RTLD_NOW);
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "noload fail: %{public}s", ARK_CONNECT_LIB_PATH);
    }
    if (handlerConnectServerSo_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "can't open %{public}s", ARK_CONNECT_LIB_PATH);
        return;
    }
    setWMSCallbackFunc_ = reinterpret_cast<SetWMSCallbackFunc>(dlsym(handlerConnectServerSo_, SET_WMS_CALLBACK));
    sendWMSMessageFunc_ = reinterpret_cast<SendWMSMessageFunc>(dlsym(handlerConnectServerSo_, SEND_WMS_MESSAGE));
    if (setWMSCallbackFunc_ == nullptr || sendWMSMessageFunc_ == nullptr) {
        setWMSCallbackFunc_ = nullptr;
        sendWMSMessageFunc_ = nullptr;
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "load failed: %{public}s", dlerror());
        return;
    }
    setWMSCallbackFunc_([this](const char* message) {
        std::string jsonWindowListInfo;
        if (ProcessArkUIInspectorMessage(message, jsonWindowListInfo)) {
            SendMessageToIDE(jsonWindowListInfo);
        }
    });
    isConnectServerSuccess_ = true;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "end connect");
}

bool WindowInspector::IsConnectServerSuccess() const
{
    return isConnectServerSuccess_;
}

void WindowInspector::RegisterGetWMSWindowListCallback(
    uint32_t windowId, GetWMSWindowListCallback&& func)
{
    if (!IsConnectServerSuccess()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u connect failed", windowId);
        return;
    }
    std::unique_lock<std::mutex> lock(callbackMutex_);
    auto [_, result] = getWMSWindowListCallbacks_.insert_or_assign(windowId, std::move(func));
    if (result) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "callback has registered");
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u", windowId);
}

void WindowInspector::UnregisterGetWMSWindowListCallback(uint32_t windowId)
{
    std::unique_lock<std::mutex> lock(callbackMutex_);
    auto result = getWMSWindowListCallbacks_.erase(windowId);
    if (result == 0) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u callback not registered", windowId);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u", windowId);
}

bool WindowInspector::ProcessArkUIInspectorMessage(const std::string& message, std::string& jsonStr)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "receive callback");
    nlohmann::json jsonMessage = nlohmann::json::parse(message, nullptr, false);
    if (jsonMessage.is_discarded()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "json::parse err");
        return false;
    }
    if (!jsonMessage.contains("method") || !jsonMessage["method"].is_string() ||
        jsonMessage["method"].get<std::string>() != METHOD_NAME) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "received method err");
        return false;
    }
    if (!jsonMessage.contains("params") || !jsonMessage["params"].contains("command") ||
        !jsonMessage["params"]["command"].is_string() ||
        jsonMessage["params"]["command"].get<std::string>() != COMMAND_NAME) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "received params.command err");
        return false;
    }
    std::vector<WindowListInfo> windowListInfoVec;
    {
        std::unique_lock<std::mutex> lock(callbackMutex_);
        for (auto& [_, func] : getWMSWindowListCallbacks_) {
            if (auto windowListInfo = func()) {
                windowListInfoVec.push_back(std::move(windowListInfo.value()));
            }
        }
    }
    if (windowListInfoVec.empty()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "has no window");
        return false;
    }
    CreateArkUIInspectorJson(windowListInfoVec, jsonStr);
    return true;
}

void WindowInspector::SendMessageToIDE(const std::string& jsonStr)
{
    sendWMSMessageFunc_(jsonStr);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s", jsonStr.c_str());
}
} // namespace Rosen::OHOS