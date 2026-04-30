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

#include "js_windowenv_manager.h"

#include "dm_common.h"
#include "js_extension_window.h"
#include "js_window.h"
#include "ui_content.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const int32_t INVALID_INSTANCEID = -1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_FOUR = 4;
}
napi_value FindJsExtensionWindowById(napi_env env, int32_t id);

static napi_value CreateJsNumber(napi_env env, uint64_t value)
{
    napi_value result = nullptr;
    napi_create_int64(env, static_cast<int64_t>(value), &result);
    return result;
}

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

sptr<Window> GetWindowByInstanceId(int32_t instanceId)
{
    int32_t windowId = Ace::UIContent::GetUIContentWindowID(instanceId);
    auto uicontent = Ace::UIContent::GetUIContent(instanceId);
    if (!uicontent) {
        TLOGE(WmsLogTag::DEFAULT, "uicontent nullptr instanceId: %{public}d, windowId: %{public}d",
            instanceId, windowId);
        return nullptr;
    }
    auto window = sptr<Window>(uicontent->GetUIContentWindow());
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "window nullptr %{public}d", windowId);
        return nullptr;
    }
    return window;
}

JsWindowEnvManager::JsWindowEnvManager()
{
}

JsWindowEnvManager::~JsWindowEnvManager()
{
}

void JsWindowEnvManager::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::DEFAULT, "Finalizer");
    std::unique_ptr<JsWindowEnvManager>(static_cast<JsWindowEnvManager*>(data));
}

napi_value JsWindowEnvManager::FindWindowById(napi_env env, napi_callback_info info)
{
    JsWindowEnvManager* me = CheckParamsAndGetThis<JsWindowEnvManager>(env, info);
    return (me != nullptr) ? me->OnFindWindowById(env, info) : nullptr;
}

napi_value JsWindowEnvManager::OnFindWindowById(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DEFAULT, "Invalid param size");
        return NapiGetUndefined(env);
    }
    int32_t instanceId = INVALID_INSTANCEID;
    if (!ConvertFromJsValue(env, argv[0], instanceId) || instanceId == INVALID_INSTANCEID) {
        TLOGE(WmsLogTag::DEFAULT, "invalid instanceId value: %{public}d", instanceId);
        return NapiGetUndefined(env);
    }
    auto window = GetWindowByInstanceId(instanceId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "window nullptr");
        return nullptr;
    }
    if (window->GetType() == WindowType::WINDOW_TYPE_UI_EXTENSION) {
        return FindJsExtensionWindowById(env, window->GetWindowPersistentId());
    }
    return CreateJsWindowObject(env, window);
}


napi_value JsWindowEnvManager::GetDisplayId(napi_env env, napi_callback_info info)
{
    JsWindowEnvManager* me = CheckParamsAndGetThis<JsWindowEnvManager>(env, info);
    return (me != nullptr) ? me->OnGetDisplayId(env, info) : nullptr;
}

napi_value JsWindowEnvManager::OnGetDisplayId(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DEFAULT, "Invalid param size");
        return NapiGetUndefined(env);
    }
    int32_t instanceId = INVALID_INSTANCEID;
    if (!ConvertFromJsValue(env, argv[0], instanceId) || instanceId == INVALID_INSTANCEID) {
        TLOGE(WmsLogTag::DEFAULT, "invalid instanceId value: %{public}d", instanceId);
        return NapiGetUndefined(env);
    }
    auto window = GetWindowByInstanceId(instanceId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "window nullptr");
        return nullptr;
    }
    if (window->GetType() == WindowType::WINDOW_TYPE_UI_EXTENSION && !window->GetIsAtomicService()) {
        return CreateJsNumber(env, static_cast<uint64_t>(DISPLAY_ID_INVALID));
    }
    return CreateJsNumber(env, window->GetDisplayId());
}

napi_value JsWindowEnvManagerInit(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::DEFAULT, "JsWindowEnvManagerInit");

    if (env == nullptr || exportObj == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "JsWindowEnvManagerInit env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowEnvManager> jsWinEnvManager = std::make_unique<JsWindowEnvManager>();
    napi_wrap(env, exportObj, jsWinEnvManager.release(), JsWindowEnvManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsWindowEnvManager";
    BindNativeFunction(env, exportObj, "findWindowById", moduleName, JsWindowEnvManager::FindWindowById);
    BindNativeFunction(env, exportObj, "getDisplayId", moduleName, JsWindowEnvManager::GetDisplayId);
    return NapiGetUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS
