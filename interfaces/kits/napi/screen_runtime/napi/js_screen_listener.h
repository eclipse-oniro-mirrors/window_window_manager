/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_SCREEN_LISTENER_H
#define OHOS_JS_SCREEN_LISTENER_H

#include <mutex>
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "screen_manager.h"

namespace OHOS {
namespace Rosen {
class JsScreenListener : public ScreenManager::IScreenListener {
public:
    explicit JsScreenListener(napi_env env);
    ~JsScreenListener() override;
    void AddCallback(const std::string& type, napi_value jsListenerObject);
    void RemoveAllCallback();
    void RemoveCallback(napi_env env, const std::string& type, napi_value jsListenerObject);
    void OnConnect(ScreenId id) override;
    void OnDisconnect(ScreenId id) override;
    void OnChange(ScreenId id) override;
    static void CleanEnv(void* obj);

private:
    void CallJsMethod(const std::string& methodName, napi_value const * argv = nullptr, size_t argc = 0);
    napi_env env_ = nullptr;
    std::mutex mtx_;
    wptr<JsScreenListener> weakRef_  = nullptr;
    std::map<std::string, std::vector<std::unique_ptr<NativeReference>>> jsCallBack_;
    napi_value CreateScreenIdArray(napi_env env, const std::vector<ScreenId>& data);
};
extern const std::string EVENT_CONNECT;
extern const std::string EVENT_DISCONNECT;
extern const std::string EVENT_CHANGE;
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_SCREEN_LISTENER_H */