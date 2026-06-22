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

#include "js_window_extension.h"

#include <hitrace_meter.h>
#include <napi_common_want.h>
#include <native_engine/native_reference.h>
#include <native_engine/native_value.h>
#include <js_extension_context.h>
#include <js_runtime_utils.h>

#include "ability_manager_client.h"
#include "js_window.h"
#include "js_window_extension_context.h"
#include "ui_extension_window_command.h"
#include "window_extension_connection.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "window_manager.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JSWindowExtension"};
const std::string RECT_FORM_KEY_POS_X = "ext_pos_x";
const std::string RECT_FORM_KEY_POS_Y = "ext_pos_y";
const std::string RECT_FORM_KEY_HEIGHT = "ext_pos_heigh";
const std::string RECT_FORM_KEY_WIDTH = "ext_pos_width";
const std::string WINDOW_ID = "ext_window_id";
}
int JsWindowExtension::extensionCnt_ = 0;

class DispatchInputEventListener : public IDispatchInputEventListener {
public:
    void OnDispatchPointerEvent(std::shared_ptr<MMI::PointerEvent>& inputEvent) override
    {
        TLOGI(WmsLogTag::WMS_UIEXT, "called");
    }
    void OnDispatchKeyEvent(std::shared_ptr<MMI::KeyEvent>& keyEvent) override
    {
        TLOGI(WmsLogTag::WMS_UIEXT, "called");
    }
};

napi_value AttachWindowExtensionContext(napi_env env, void* value, void *)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "AttachWindowExtensionContext");
    if (value == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<WindowExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsWindowExtensionContext(env, ptr);
    if (object == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get js window extension context");
        return nullptr;
    }
    auto contextObjRef = AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env,
        "application.WindowExtensionContext", &object, 1);
    if (contextObjRef == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get contextObjRef");
        return nullptr;
    }
    auto contextObj = contextObjRef->GetNapiValue();
    if (contextObj == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get context native object");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj,
        AbilityRuntime::DetachCallbackFunc, AttachWindowExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<WindowExtensionContext>(ptr);
    if (workContext == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get window extension context");
        return nullptr;
    }
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void *) {
            TLOGI(WmsLogTag::WMS_UIEXT, "Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<WindowExtensionContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to call napi_wrap");
        delete workContext;
        return nullptr;
    }
    return contextObj;
}

JsWindowExtension* JsWindowExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Create runtime");
    return new JsWindowExtension(static_cast<AbilityRuntime::JsRuntime&>(*runtime));
}

JsWindowExtension::JsWindowExtension(AbilityRuntime::JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsWindowExtension::~JsWindowExtension()
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Called");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
}

void JsWindowExtension::Init(const std::shared_ptr<AbilityRuntime::AbilityLocalRecord>& record,
    const std::shared_ptr<AbilityRuntime::OHOSApplication>& application,
    std::shared_ptr<AbilityRuntime::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WindowExtension Init");
    WindowExtension::Init(record, application, handler, token);
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get srcPath");
        return;
    }
    if (Extension::abilityInfo_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get abilityInfo");
        return;
    }
    if (abilityInfo_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get abilityInfo_");
        return;
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TLOGI(WmsLogTag::WMS_UIEXT, "JsWindowExtension::Init module:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    AbilityRuntime::HandleScope handleScope(jsRuntime_);

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == AbilityRuntime::CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get jsObj_");
        return;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "JsWindowExtension::Init ConvertNativeValueTo.");
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get JsWindowExtension object");
        return;
    }

    BindContext(env, obj);
}

void JsWindowExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get context");
        return;
    }

    napi_value contextObj = CreateJsWindowExtensionContext(jsRuntime_.GetNapiEnv(), context);
    if (contextObj == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get js window extension context");
        return;
    }
    auto shellContextRef = jsRuntime_.LoadSystemModule("application.WindowExtensionContext", &contextObj, 1);
    if (shellContextRef == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get shell context");
        return;
    }
    contextObj = shellContextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<WindowExtensionContext>(context);
    if (workContext == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get window extension context");
        return;
    }
    napi_coerce_to_native_binding_object(env, contextObj,
        AbilityRuntime::DetachCallbackFunc, AttachWindowExtensionContext, workContext, nullptr);
    TLOGI(WmsLogTag::WMS_UIEXT, "JsWindowExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef.release());
    TLOGI(WmsLogTag::WMS_UIEXT, "JsWindowExtension::SetProperty.");
    napi_set_named_property(env, obj, "context", contextObj);

    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void *) {
            TLOGI(WmsLogTag::WMS_UIEXT, "Finalizer for weak_ptr extension context is called");
            delete static_cast<std::weak_ptr<WindowExtensionContext>*>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to call napi_wrap");
        delete workContext;
        return;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "JsWindowExtension::Init end.");
}

void JsWindowExtension::GetSrcPath(std::string& srcPath) const
{
    if (!Extension::abilityInfo_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "abilityInfo_ is nullptr");
        return;
    }

    if (!Extension::abilityInfo_->isModuleJson) {
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

sptr<IRemoteObject> JsWindowExtension::OnConnect(const AAFwk::Want& want)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WindowExtension OnConnect %s-%s",
        want.GetElement().GetAbilityName().c_str(), want.GetElement().GetAbilityName().c_str());
    TLOGI(WmsLogTag::WMS_UIEXT, "called.");
    Extension::OnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!jsObj_) {
        TLOGW(WmsLogTag::WMS_UIEXT, "Not found WindowExtension.js");
        return nullptr;
    }
    napi_value value = jsObj_->GetNapiValue();
    auto jsCallback = [want, env, value] {
        napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
        napi_value argv[] = { napiWant };
        CallJsMethod("onConnect", argv, AbilityRuntime::ArraySize(argv), env, value);
    };
    if (napi_send_event(env, jsCallback, napi_eprio_high, "OnConnect") != napi_status::napi_ok) {
        WLOGE("send event failed");
    }

    if (!stub_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "stub is nullptr.");
        return nullptr;
    }
    TLOGD(WmsLogTag::WMS_UIEXT, "Create stub successfully!");
    WindowManager::GetInstance().NotifyWindowExtensionVisibilityChange(getprocpid(), getuid(), true);
    auto context = GetContext();
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "context is nullptr.");
        return nullptr;
    }
    auto client = AAFwk::AbilityManagerClient::GetInstance();
    if (client == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "client is nullptr.");
        return nullptr;
    }
    client->ScheduleCommandAbilityWindowDone(
        context->GetToken(), sessionInfo_, AAFwk::WIN_CMD_FOREGROUND, AAFwk::ABILITY_CMD_FOREGROUND);
    return stub_->AsObject();
}

void JsWindowExtension::OnDisconnect(const AAFwk::Want& want)
{
    Extension::OnDisconnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!jsObj_) {
        TLOGW(WmsLogTag::WMS_UIEXT, "Not found WindowExtension.js");
        return;
    }
    napi_value value = jsObj_->GetNapiValue();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    CallJsMethod("onDisconnect", argv, AbilityRuntime::ArraySize(argv), env, value);
    auto window = stub_ != nullptr ? stub_->GetWindow() : nullptr;
    if (window != nullptr) {
        window->Destroy();
        TLOGI(WmsLogTag::WMS_UIEXT, "Destroy window.");
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "called.");
    WindowManager::GetInstance().NotifyWindowExtensionVisibilityChange(getprocpid(), getuid(), false);
    auto context = GetContext();
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "context is nullptr.");
        return;
    }
    auto client = AAFwk::AbilityManagerClient::GetInstance();
    if (client == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "client is nullptr.");
        return;
    }
    client->ScheduleCommandAbilityWindowDone(
        context->GetToken(), sessionInfo_, AAFwk::WIN_CMD_DESTROY, AAFwk::ABILITY_CMD_DESTROY);
}

void JsWindowExtension::OnStart(const AAFwk::Want& want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "OnStart");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WindowExtension OnStart %s-%s",
        want.GetElement().GetAbilityName().c_str(), want.GetElement().GetAbilityName().c_str());
    sessionInfo_ = sessionInfo;
    Extension::OnStart(want);

    AbilityRuntime::ElementName elementName = want.GetElement();
    std::string windowName = elementName.GetBundleName() + elementName.GetModuleName() +
        elementName.GetAbilityName() + std::to_string(extensionCnt_);
    extensionCnt_++;

    stub_ = new(std::nothrow) WindowExtensionStubImpl(windowName);
    TLOGI(WmsLogTag::WMS_UIEXT, "JsWindowExtension OnStart begin..");
    Rect rect { want.GetIntParam(RECT_FORM_KEY_POS_X, 0),
    want.GetIntParam(RECT_FORM_KEY_POS_Y, 0),
    want.GetIntParam(RECT_FORM_KEY_WIDTH, 0),
    want.GetIntParam(RECT_FORM_KEY_HEIGHT, 0) };
    uint32_t windowId = static_cast<uint32_t>(want.GetIntParam(WINDOW_ID, INVALID_WINDOW_ID));
    if (stub_ != nullptr) {
        auto context = GetContext();
        if (context == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "get context failed");
            return;
        }
        sptr<Window> window = stub_->CreateWindow(rect, windowId, context,
            sessionInfo == nullptr ? nullptr : sessionInfo->sessionToken);
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "create window failed");
            return;
        }
        OnWindowCreated();
        TLOGI(WmsLogTag::WMS_UIEXT,
            "ability context onWindowReady rect x =%{public}d y=%{public}d w=%{public}d h=%{public}d ",
            rect.posX_, rect.posY_, rect.width_, rect.height_);
    }
}

void JsWindowExtension::OnWindowCreated() const
{
    napi_env env = jsRuntime_.GetNapiEnv();
    if (stub_ == nullptr) {
        WLOGE("stub is nullptr");
        return;
    }
    auto jsCallback = [stub = stub_, env] {
        auto window = stub->GetWindow();
        if (window == nullptr) {
            WLOGE("get window failed");
            return;
        }
        napi_value value = CreateJsWindowObject(env, window);
        if (value == nullptr) {
            WLOGE("Create js window failed");
            return;
        }
        napi_value argv[] = { value };
        CallJsMethod("onWindowReady", argv, AbilityRuntime::ArraySize(argv), env, value);
    };
    if (napi_send_event(env, jsCallback, napi_eprio_high, "OnWindowCreated") != napi_status::napi_ok) {
        WLOGE("send event failed");
    }
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

napi_value CallJsMethod(const char* name, napi_value const * argv, size_t argc,
    napi_env env, napi_value value)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "called (%{public}s), begin", name);
    if (value == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get WindowExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, value, name, &method);
    if (method == nullptr || GetType(env, method) != napi_function) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get '%{public}s' from WindowExtension object", name);
        return nullptr;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "(%{public}s), success", name);
    napi_value result = nullptr;
    napi_call_function(env, value, method, argc, argv, &result);
    return result;
}
} // namespace Rosen
} // namespace OHOS
