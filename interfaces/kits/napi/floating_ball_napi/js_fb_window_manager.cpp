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
#include "floating_ball_controller.h"
#include "js_fb_window_controller.h"
#include "floating_ball_manager.h"
#include "js_err_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const char* ARKUI_WINDOW_FB_CREATE = "ArkUI.window.fb.create";
const char* ARKUI_WINDOW_FB_ISFLOATINGBALLENABLED = "ArkUI.window.fb.isFloatingBallEnabled";
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
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env, "Missing args when creating fbController");
    }

    napi_value config = argv[0];
    if (config == nullptr) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env, "Failed to convert object to fbConfiguration or fbConfiguration is null");
    }

    napi_value contextPtrValue = nullptr;
    napi_get_named_property(env, config, "context", &contextPtrValue);
    void* contextPtr = nullptr;
    napi_unwrap(env, contextPtrValue, &contextPtr);
    if (contextPtr == nullptr) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_FB_CREATE_FAILED);
        return NapiThrowInvalidParam(env, "Context is null.");
    }
    return NapiSendTask(env, contextPtr);
}

napi_value JsFbWindowManager::NapiSendTask(napi_env env, void* contextPtr)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    sptr<FloatingBallController> fbController = sptr<FloatingBallController>::MakeSptr(nullptr, 0, contextPtr);
    NapiAsyncTask::ExecuteCallback execute = [contextPtr, errCodePtr, fbController] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (!FloatingBallManager::IsSupportFloatingBall()) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Device is not phone or pad, do not support floating ball");
            *errCodePtr = WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
            return;
        }
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
        if (context == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR);
            return;
        }
        sptr<Window> mainWindow = Window::GetMainWindowWithContext(context->lock());
        if (mainWindow == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR);
            return;
        }
        if (fbController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR);
            return;
        }
        fbController->UpdateMainWindow(mainWindow);
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr, fbController](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr || fbController == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR));
                return;
            }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateJsFbControllerObject(env, fbController));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_CREATE, WmErrorCode::WM_OK);
        } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "JsFbController::OnStartFloatingBall failed."));
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
    bool isSupportFloatingBall = FloatingBallManager::IsSupportFloatingBall();
    if (!isSupportFloatingBall) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ISFLOATINGBALLENABLED,
                                         WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ISFLOATINGBALLENABLED, WmErrorCode::WM_OK);
    }
    return CreateJsValue(env, isSupportFloatingBall);
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