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

#include "oh_window_pip.h"

#include "oh_window_comm.h"
#include "web_picture_in_picture_controller_interface.h"
#include "window_manager_hilog.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t MAX_CONTROL_TYPE_NUM = 8;
    constexpr uint32_t MAX_CONTROL_STATUS_NUM = 1;
    std::shared_mutex idMapMutex_;
    std::map<uint32_t, sptr<WebPictureInPictureControllerInterface>> g_ControllerIds;
}

inline bool IsIdMapFull()
{
    std::shared_lock<std::shared_mutex> lock(idMapMutex_);
    if (g_ControllerIds.size() >= static_cast<size_t>(UINT32_MAX - 1)) {
        return true;
    }
    return false;
}

inline uint32_t FindNextAvailableId()
{
    if (IsIdMapFull()) {
        return 0;
    }
    uint32_t nextId = 0;
    std::unique_lock<std::shared_mutex> lock(idMapMutex_);
    while (nextId < UINT32_MAX && g_ControllerIds.find(nextId) != g_ControllerIds.end()) {
        nextId++;
    }
    return nextId;
}

inline sptr<WebPictureInPictureControllerInterface> GetControllerFromId(uint32_t controllerId)
{
    std::shared_lock<std::shared_mutex> lock(idMapMutex_);
    if (g_ControllerIds.find(controllerId) == g_ControllerIds.end()) {
        return nullptr;
    }
    return g_ControllerIds.at(controllerId);
}
} // namespace Rosen
} // namespace OHOS

namespace {
/*
 * Used to map from WMError to WindowManager_ErrorCode.
 */
const std::unordered_map<WMError,              WindowManager_ErrorCode> OH_WINDOW_TO_ERROR_CODE_MAP {
    { WMError::WM_OK,                          WindowManager_ErrorCode::OK                                            },
    { WMError::WM_ERROR_INVALID_PARAM,         WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM      },
    { WMError::WM_ERROR_DEVICE_NOT_SUPPORT,    WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED },
    { WMError::WM_ERROR_PIP_DESTROY_FAILED,    WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_DESTROY_FAILED   },
    { WMError::WM_ERROR_PIP_STATE_ABNORMALLY,  WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_STATE_ABNORMAL   },
    { WMError::WM_ERROR_PIP_CREATE_FAILED,     WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_CREATE_FAILED    },
    { WMError::WM_ERROR_PIP_INTERNAL_ERROR,    WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_INTERNAL_ERROR   },
    { WMError::WM_ERROR_PIP_REPEAT_OPERATION,
        WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_REPEATED_OPERATION },
};

inline WindowManager_ErrorCode GetErrorCodeFromWMError(WMError error)
{
    if (OH_WINDOW_TO_ERROR_CODE_MAP.find(error) == OH_WINDOW_TO_ERROR_CODE_MAP.end()) {
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_INTERNAL_ERROR;
    }
    return OH_WINDOW_TO_ERROR_CODE_MAP.at(error);
}
} //namespace

int32_t OH_PictureInPicture_CreatePipConfig(PictureInPicture_PipConfig* pipConfig)
{
    if (pipConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    PiPConfig* config = new PiPConfig();
    *pipConfig = reinterpret_cast<PictureInPicture_PipConfig>(config);
    TLOGI(WmsLogTag::WMS_PIP, "pipConfig created");
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_DestroyPipConfig(PictureInPicture_PipConfig* pipConfig)
{
    if (pipConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto config = reinterpret_cast<PiPConfig*>(*pipConfig);
    delete config;
    *pipConfig = nullptr;
    TLOGI(WmsLogTag::WMS_PIP, "pipConfig destroyed");
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_SetPipMainWindowId(PictureInPicture_PipConfig pipConfig, uint32_t mainWindowId)
{
    if (pipConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto config = reinterpret_cast<PiPConfig*>(pipConfig);
    config->mainWindowId = mainWindowId;
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_SetPipTemplateType(PictureInPicture_PipConfig pipConfig,
    PictureInPicture_PipTemplateType pipTemplateType)
{
    if (pipConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto config = reinterpret_cast<PiPConfig*>(pipConfig);
    config->pipTemplateType = static_cast<uint32_t>(pipTemplateType);
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_SetPipRect(PictureInPicture_PipConfig pipConfig, uint32_t width, uint32_t height)
{
    if (pipConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto config = reinterpret_cast<PiPConfig*>(pipConfig);
    config->width = width;
    config->height = height;
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_SetPipControlGroup(PictureInPicture_PipConfig pipConfig,
    PictureInPicture_PipControlGroup* controlGroup, uint8_t controlGroupLength)
{
    if (pipConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto config = reinterpret_cast<PiPConfig*>(pipConfig);
    if (controlGroup != nullptr) {
        config->controlGroup = std::vector<uint32_t>(controlGroup, controlGroup + controlGroupLength);
    } else {
        config->controlGroup = {};
    }
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_SetPipNapiEnv(PictureInPicture_PipConfig pipConfig, void* env)
{
    if (pipConfig == nullptr || env == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig or env is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto config = reinterpret_cast<PiPConfig*>(pipConfig);
    config->env = reinterpret_cast<napi_env>(env);
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_CreatePip(PictureInPicture_PipConfig pipConfig, uint32_t* controllerId)
{
    if (pipConfig == nullptr || controllerId == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipConfig or controllerId is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    if (IsIdMapFull()) {
        TLOGE(WmsLogTag::WMS_PIP, "all IDs are used");
        return  WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_INTERNAL_ERROR;
    }
    auto config = reinterpret_cast<PiPConfig*>(pipConfig);
    auto webPipControllerInterface = OHOS::sptr<WebPictureInPictureControllerInterface>::MakeSptr();
    auto errCode = webPipControllerInterface->Create(*config);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "pipInterface create failed");
        return GetErrorCodeFromWMError(errCode);
    }
    uint32_t id = FindNextAvailableId();
    *controllerId = id;
    std::unique_lock<std::shared_mutex> lock(idMapMutex_);
    g_ControllerIds[id] = webPipControllerInterface;
    TLOGI(WmsLogTag::WMS_PIP, "create success! controllerId: %{public}u", id);
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_DeletePip(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    if (pipController->IsStarted()) {
        TLOGE(WmsLogTag::WMS_PIP, "could not delete before stopPip: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    std::unique_lock<std::shared_mutex> lock(idMapMutex_);
    g_ControllerIds.erase(controllerId);
    return WindowManager_ErrorCode::OK;
}

int32_t OH_PictureInPicture_StartPip(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->StartPip(controllerId));
}

int32_t OH_PictureInPicture_StopPip(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->StopPip());
}

int32_t OH_PictureInPicture_UpdatePipContentSize(uint32_t controllerId, uint32_t width, uint32_t height)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UpdateContentSize(width, height));
}

int32_t OH_PictureInPicture_UpdatePipControlStatus(uint32_t controllerId, PictureInPicture_PipControlType controlType,
    PictureInPicture_PipControlStatus status)
{
    auto typeNum = static_cast<uint32_t>(controlType);
    auto statusNum = static_cast<uint32_t>(status);
    if (typeNum > MAX_CONTROL_TYPE_NUM || statusNum > MAX_CONTROL_STATUS_NUM) {
        TLOGE(WmsLogTag::WMS_PIP, "param error. controlType: %{public}d, status: %{public}d", typeNum, statusNum);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UpdatePiPControlStatus(static_cast<PiPControlType>(controlType),
        static_cast<PiPControlStatus>(status)));
}

int32_t OH_PictureInPicture_SetPipControlEnabled(uint32_t controllerId, PictureInPicture_PipControlType controlType,
    bool enabled)
{
    auto typeNum = static_cast<uint32_t>(controlType);
    if (typeNum > MAX_CONTROL_TYPE_NUM) {
        TLOGE(WmsLogTag::WMS_PIP, "param error. controlType: %{public}d", typeNum);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->setPiPControlEnabled(static_cast<PiPControlType>(controlType),
        enabled));
}

int32_t OH_PictureInPicture_SetParentWindowId(uint32_t controllerId, uint32_t windowId)
{
    if (windowId <= 0) {
        TLOGE(WmsLogTag::WMS_PIP, "windowId invalid: %{public}d", windowId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->SetPipParentWindowId(windowId));
}

int32_t OH_PictureInPicture_SetPipInitialSurfaceRect(uint32_t controllerId, int32_t positionX, int32_t positionY,
    uint32_t width, uint32_t height)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->SetPipInitialSurfaceRect(positionX, positionY, width, height));
}

int32_t OH_PictureInPicture_UnsetPipInitialSurfaceRect(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnsetPipInitialSurfaceRect());
}

int32_t OH_PictureInPicture_RegisterStartPipCallback(uint32_t controllerId, WebPipStartPipCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->RegisterStartPipListener(callback));
}

int32_t OH_PictureInPicture_UnregisterStartPipCallback(uint32_t controllerId, WebPipStartPipCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterStartPipListener(callback));
}

int32_t OH_PictureInPicture_UnregisterAllStartPipCallbacks(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterAllPiPStart());
}

int32_t OH_PictureInPicture_RegisterLifecycleListener(uint32_t controllerId, WebPipLifecycleCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->RegisterLifeCycleListener(
        reinterpret_cast<OHOS::Rosen::NativePipLifeCycleCallback>(callback)));
}

int32_t OH_PictureInPicture_UnregisterLifecycleListener(uint32_t controllerId, WebPipLifecycleCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterLifeCycleListener(
        reinterpret_cast<OHOS::Rosen::NativePipLifeCycleCallback>(callback)));
}

int32_t OH_PictureInPicture_UnregisterAllLifecycleListeners(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterAllPiPLifecycle());
}

int32_t OH_PictureInPicture_RegisterControlEventListener(uint32_t controllerId, WebPipControlEventCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->RegisterControlEventListener(
        reinterpret_cast<OHOS::Rosen::NativePipControlEventCallback>(callback)));
}

int32_t OH_PictureInPicture_UnregisterControlEventListener(uint32_t controllerId, WebPipControlEventCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterControlEventListener(
        reinterpret_cast<OHOS::Rosen::NativePipControlEventCallback>(callback)));
}

int32_t OH_PictureInPicture_UnregisterAllControlEventListeners(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterAllPiPControlObserver());
}

int32_t OH_PictureInPicture_RegisterResizeListener(uint32_t controllerId, WebPipResizeCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->RegisterResizeListener(callback));
}

int32_t OH_PictureInPicture_UnregisterResizeListener(uint32_t controllerId, WebPipResizeCallback callback)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is nullptr");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterResizeListener(callback));
}

int32_t OH_PictureInPicture_UnregisterAllResizeListeners(uint32_t controllerId)
{
    auto pipController = GetControllerFromId(controllerId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controllerId not found: %{public}d", controllerId);
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM;
    }
    return GetErrorCodeFromWMError(pipController->UnregisterAllPiPWindowSize());
}