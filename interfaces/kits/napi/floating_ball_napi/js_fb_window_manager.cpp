/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include <refbase.h>

#include "js_fb_window_manager.h"
#include "js_fb_utils.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "js_fb_window_controller.h"
#include "floating_ball_manager.h"
#include "js_err_utils.h"
#include "float_window_error_msg.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const char* ARKUI_WINDOW_FB_CREATE = "ArkUI.window.fb.create";

const char* ARKUI_WINDOW_FB_CREATE_BOOL = "ArkUI.window.fb.create.bool";
const char* ARKUI_WINDOW_FB_ISFLOATINGBALLENABLED_BOOL = "ArkUI.window.fb.isFloatingBallEnabled.bool";
}

JsFbWindowManager::JsFbWindowManager()
{
}

JsFbWindowManager::~JsFbWindowManager()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFbWindowManager release");
}

napi_value JsFbWindowManager::CreateFbController(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFbWindowManagerInit create");
    JsFbWindowManager* me = CheckParamsAndGetThis<JsFbWindowManager>(env, info);
    return (me != nullptr) ? me->OnCreateFbController(env, info) : nullptr;
}

napi_value JsFbWindowManager::OnCreateFbController(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    // 解析应用传参FloatingBallConfiguration对象
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        return NapiThrowInvalidParam(env, "[FBWindow][create]msg: Missing args when creating fbController",
            ARKUI_WINDOW_FB_CREATE, ARKUI_WINDOW_FB_CREATE_BOOL);
    }

    napi_value config = argv[0];
    if (config == nullptr) {
        return NapiThrowInvalidParam(env,
            "[FBWindow][create]msg: Failed to convert object to fbConfiguration or fbConfiguration is null",
            ARKUI_WINDOW_FB_CREATE, ARKUI_WINDOW_FB_CREATE_BOOL);
    }

    napi_value contextPtrValue = nullptr;
    if (napi_get_named_property(env, config, "context", &contextPtrValue) != napi_ok) {
        return NapiThrowInvalidParam(env,
            "[FBWindow][create]msg: Failed to get context", ARKUI_WINDOW_FB_CREATE, ARKUI_WINDOW_FB_CREATE_BOOL);
    }
    void* contextPtr = nullptr;
    napi_unwrap(env, contextPtrValue, &contextPtr);
    if (contextPtr == nullptr) {
        return NapiThrowInvalidParam(env, "[FBWindow][create]msg: The context parameter is null.",
            ARKUI_WINDOW_FB_CREATE, ARKUI_WINDOW_FB_CREATE_BOOL);
    }
    return NapiSendTask(env, contextPtr);
}

WmErrorCode JsFbWindowManager::ValidateAndSetupMainWindow(void* contextPtr,
    const sptr<FloatingBallController>& fbController, std::string& errMsg)
{
    if (!FloatingBallManager::isSupportFloatingBall_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Device is not phone or pad, do not support floating ball");
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (context == nullptr) {
        errMsg = "The application context or main window is invalid.";
        return WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR;
    }
    sptr<Window> mainWindow = Window::GetMainWindowWithContext(context->lock());
    if (mainWindow == nullptr) {
        errMsg = "The application context or main window is invalid.";
        return WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR;
    }
    if (fbController == nullptr) {
        errMsg = "System internal error, such as null pointer or insufficient memory.";
        return WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR;
    }
    fbController->UpdateMainWindow(mainWindow);
    return WmErrorCode::WM_OK;
}

napi_value JsFbWindowManager::NapiSendTask(napi_env env, void* contextPtr)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    std::shared_ptr<std::string> errMsgPtr = std::make_shared<std::string>("");
    sptr<FloatingBallController> fbController = sptr<FloatingBallController>::MakeSptr(nullptr, 0, contextPtr);
    NapiAsyncTask::ExecuteCallback execute = [contextPtr, errCodePtr, errMsgPtr, fbController, this] {
        if (errCodePtr == nullptr || errMsgPtr == nullptr) {
            return;
        }
        WmErrorCode errCode = ValidateAndSetupMainWindow(contextPtr, fbController, *errMsgPtr);
        if (errCode != WmErrorCode::WM_OK) {
            *errCodePtr = errCode;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, errCode);
        }
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr, errMsgPtr, fbController](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr || errMsgPtr == nullptr || fbController == nullptr) {
                task.Reject(env, JsErrUtils::CreateFloatWindowJsError(env, FloatWindowModule::FLOATING_BALL,
                "createFloatingBall", WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, ""));
                return;
            }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateJsFbControllerObject(env, fbController));
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FB_CREATE_BOOL, 1);
        } else {
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FB_CREATE_BOOL, 0);
            task.Reject(env, JsErrUtils::CreateFloatWindowJsError(env, FloatWindowModule::FLOATING_BALL,
                            "createFloatingBall", *errCodePtr, *errMsgPtr));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFbController::OnCreateFbController",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

void JsFbWindowManager::Finalizer(napi_env env, void* data, void* hint)
{
    std::unique_ptr<JsFbWindowManager>(static_cast<JsFbWindowManager*>(data));
}

napi_value JsFbWindowManager::IsFbEnabled(napi_env env, napi_callback_info info)
{
    JsFbWindowManager* me = CheckParamsAndGetThis<JsFbWindowManager>(env, info);
    return (me != nullptr) ? me->OnIsFbEnabled(env, info) : nullptr;
}

napi_value JsFbWindowManager::OnIsFbEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "OnIsFbEnabled called");
    HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FB_ISFLOATINGBALLENABLED_BOOL, 1);
    return CreateJsValue(env, FloatingBallManager::isSupportFloatingBall_);
}

napi_value JsFbWindowManagerInit(napi_env env, napi_value exportObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFbWindowManagerInit");
    if (env == nullptr || exportObj == nullptr) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "JsFbWindowManagerInit env or exportObj is nullptr");
        return nullptr;
    }
    std::unique_ptr<JsFbWindowManager> jsFbManager = std::make_unique<JsFbWindowManager>();
    napi_wrap(env, exportObj, jsFbManager.release(), JsFbWindowManager::Finalizer, nullptr, nullptr);
    const char* moduleName = "FloatingBall";
    BindNativeFunction(env, exportObj, "create", moduleName, JsFbWindowManager::CreateFbController);
    BindNativeFunction(env, exportObj, "isFloatingBallEnabled", moduleName, JsFbWindowManager::IsFbEnabled);
    InitFbEnums(env, exportObj);
    return NapiGetUndefined(env);
}
} // namespace Rosen
} // namespace OHOS