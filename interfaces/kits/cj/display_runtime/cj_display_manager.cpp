/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cj_display_manager.h"

#include <mutex>
#include <securec.h>

#include "cj_display_impl.h"
#include "display_manager.h"
#include "dm_common.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

std::map<std::string, std::map<int64_t, sptr<CJDisplayListener>>> CJDisplayManager::cjCbMap_;
std::mutex CJDisplayManager::mtx_;

static void SetDisplayObject(sptr<Display>& obj, RetStruct& ret)
{
    auto result = DisplayImpl::CreateDisplayImpl(obj);
    if (result == nullptr || ret.data == nullptr) {
        TLOGE(WmsLogTag::DMS, "[GetDefaultDisplaySync] ERROR Failed to create DisplayImpl.");
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
        return;
    }
    int64_t* dataPtr = static_cast<int64_t*>(ret.data);
    if (dataPtr == nullptr) {
        TLOGE(WmsLogTag::DMS, "[SetDisplayObject] ERROR Failed to create dataPtr.");
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
        return;
    }
    dataPtr[ret.len] = result->GetID();
    ret.len += 1;
    ret.code = static_cast<int32_t>(DmErrorCode::DM_OK);
}

static void SetDisplaysArrayObject(std::vector<sptr<Display>>& list, RetStruct& ret)
{
    int64_t* displayImplIdList = static_cast<int64_t*>(malloc(sizeof(int64_t) * list.size()));
    if (displayImplIdList == nullptr) {
        TLOGE(WmsLogTag::DMS, "[SetDisplaysArrayObject] ERROR Failed to create displayImplIdList.");
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
        return;
    }
    ret.data = displayImplIdList;
    ret.len = 0;
    for (auto& display : list) {
        if (display == nullptr) {
            continue;
        }
        SetDisplayObject(display, ret);
        if (ret.code != 0) {
            TLOGE(WmsLogTag::DMS, "[SetDisplaysArrayObject] ERROR Create display failed in %{public}" PRId64 " obj",
                ret.len);
            free(displayImplIdList);
            ret.data = nullptr;
            return;
        }
    }
}

static void SetCRect(const DMRect& row, CRect* ptr)
{
    ptr->left = row.posX_;
    ptr->top = row.posY_;
    ptr->width = row.width_;
    ptr->height = row.height_;
}

static CRect* CreateCreaseRects(std::vector<DMRect>& list)
{
    int32_t number = static_cast<int32_t>(list.size());
    CRect* result = static_cast<CRect*>(malloc(sizeof(CRect) * number));
    if (result == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < number; i++) {
        SetCRect(list[i], (result + i));
    }
    return result;
}

static CFoldCreaseRegion* CreateCFoldCreaseRegionObject(sptr<FoldCreaseRegion>& foldCreaseRegion)
{
    CFoldCreaseRegion* region = static_cast<CFoldCreaseRegion*>(malloc(sizeof(CFoldCreaseRegion)));
    if (region == nullptr) {
        return nullptr;
    }
    int ret = memset_s(region, sizeof(CFoldCreaseRegion), 0, sizeof(CFoldCreaseRegion));
    if (ret != 0) {
        free(region);
        return nullptr;
    }
    region->displayId = static_cast<uint32_t>(foldCreaseRegion->GetDisplayId());
    std::vector<DMRect> creaseRects = foldCreaseRegion->GetCreaseRects();

    region->number = static_cast<int64_t>(creaseRects.size());
    region->creaseRects = CreateCreaseRects(creaseRects);
    if (region->creaseRects == nullptr) {
        TLOGE(WmsLogTag::DMS, "[CreateCreaseRects] ERROR Failed to create creaseRects.");
        free(region);
        return nullptr;
    }
    return region;
}

RetStruct CJDisplayManager::GetDefaultDisplaySync()
{
    RetStruct ret = { .code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL), .len = 0, .data = nullptr };
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplaySync();
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DisplayManager] Get default display is nullptr");
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN);
        return ret;
    }
    int64_t* displayImplId = static_cast<int64_t*>(malloc(sizeof(int64_t)));
    if (displayImplId == nullptr) {
        TLOGE(WmsLogTag::DMS, "[GetDefaultDisplaySync] ERROR Failed to create displayImplId.");
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN);
        return ret;
    }
    ret.data = displayImplId;
    ret.len = 0;
    SetDisplayObject(display, ret);
    return ret;
}

RetStruct CJDisplayManager::GetAllDisplays()
{
    RetStruct ret = { .code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL), .len = 0, .data = nullptr };
    std::vector<sptr<Display>> displays = SingletonContainer::Get<DisplayManager>().GetAllDisplays();
    if (displays.empty()) {
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN);
        TLOGE(WmsLogTag::DMS, "[GetAllDisplays] ERROR Failed to get all displays.");
        return ret;
    }
    SetDisplaysArrayObject(displays, ret);
    return ret;
}

RetStruct CJDisplayManager::HasPrivateWindow(uint32_t displayId)
{
    RetStruct ret = { .code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL), .len = 0, .data = nullptr };
    int64_t displayId_ = static_cast<int64_t>(displayId);
    bool* hasPrivateWindow = static_cast<bool*>(malloc(sizeof(bool)));
    if (hasPrivateWindow == nullptr) {
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
        return ret;
    }
    *hasPrivateWindow = false;
    DmErrorCode errorCode = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<DisplayManager>().HasPrivateWindow(displayId_, *hasPrivateWindow));
    TLOGI(WmsLogTag::DMS,
        "[DisplayManager] Display id = %{public}" PRIu64 ", hasPrivateWindow = %{public}u err = %{public}d",
        static_cast<uint64_t>(displayId_), *hasPrivateWindow, errorCode);
    if (errorCode != DmErrorCode::DM_OK) {
        ret.code = static_cast<int32_t>(errorCode);
        free(hasPrivateWindow);
        return ret;
    }
    ret.code = static_cast<int32_t>(DmErrorCode::DM_OK);
    ret.data = static_cast<void*>(hasPrivateWindow);
    return ret;
}

bool CJDisplayManager::IsFoldable()
{
    return SingletonContainer::Get<DisplayManager>().IsFoldable();
}

void CJDisplayManager::SetFoldDisplayMode(uint32_t mode)
{
    FoldDisplayMode innerMode = static_cast<FoldDisplayMode>(mode);
    SingletonContainer::Get<DisplayManager>().SetFoldDisplayMode(innerMode);
}

uint32_t CJDisplayManager::GetFoldStatus()
{
    return static_cast<uint32_t>(SingletonContainer::Get<DisplayManager>().GetFoldStatus());
}

uint32_t CJDisplayManager::GetFoldDisplayMode()
{
    return static_cast<uint32_t>(SingletonContainer::Get<DisplayManager>().GetFoldDisplayMode());
}

RetStruct CJDisplayManager::GetCurrentFoldCreaseRegion()
{
    RetStruct result = {
        .code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL), .len = 0, .data = nullptr
    };
    sptr<FoldCreaseRegion> region = SingletonContainer::Get<DisplayManager>().GetCurrentFoldCreaseRegion();
    if (region == nullptr) {
        result.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
        return result;
    }
    result.data = CreateCFoldCreaseRegionObject(region);
    result.code = static_cast<int32_t>(DmErrorCode::DM_OK);
    if (result.data == nullptr) {
        result.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
    }
    return result;
}

bool CJDisplayManager::IsCaptured()
{
    return SingletonContainer::Get<DisplayManager>().IsCaptured();
}

static void SetDisplayPhysicalResolutionObject(DisplayPhysicalResolution& resolution, RetStruct& ret)
{
    CDisplayPhysicalResolution* dataPtr = static_cast<CDisplayPhysicalResolution*>(ret.data);
    dataPtr[ret.len].foldDisplayMode = static_cast<uint32_t>(resolution.foldDisplayMode_);
    dataPtr[ret.len].physicalWidth = resolution.physicalWidth_;
    dataPtr[ret.len].physicalHeight = resolution.physicalHeight_;
    ret.len++;
}

static void SetDisplayPhysicalResolutionArrayObject(
    std::vector<DisplayPhysicalResolution>& resolutionList, RetStruct& ret)
{
    CDisplayPhysicalResolution* resolutions =
        static_cast<CDisplayPhysicalResolution*>(malloc(sizeof(CDisplayPhysicalResolution) * resolutionList.size()));
    if (resolutions == nullptr) {
        TLOGE(WmsLogTag::DMS,
            "[SetDisplayPhysicalResolutionArrayObject] ERROR Failed to create display physical resolution list.");
        ret.code = static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL);
        return;
    }
    ret.data = resolutions;
    ret.len = 0;
    for (auto& resolution : resolutionList) {
        SetDisplayPhysicalResolutionObject(resolution, ret);
    }
}

RetStruct CJDisplayManager::GetAllDisplayPhysicalResolution()
{
    RetStruct result = { .code = static_cast<int32_t>(DMError::DM_OK), .len = 0, .data = nullptr };
    std::vector<DisplayPhysicalResolution> displayPhysicalArray =
        SingletonContainer::Get<DisplayManager>().GetAllDisplayPhysicalResolution();
    if (displayPhysicalArray.empty()) {
        result.code = static_cast<int32_t>(DMError::DM_ERROR_NULLPTR);
        TLOGE(
            WmsLogTag::DMS, "[GetAllDisplayPhysicalResolution] ERROR Failed to get all displays physical resolution.");
        return result;
    }
    SetDisplayPhysicalResolutionArrayObject(displayPhysicalArray, result);
    return result;
}

bool CJDisplayManager::IfCallbackRegistered(const std::string& type, int64_t callbackId)
{
    if (CJDisplayManager::cjCbMap_.empty() ||
        CJDisplayManager::cjCbMap_.find(type) == CJDisplayManager::cjCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "IfCallbackRegistered methodName %{public}s not registered", type.c_str());
        return false;
    }

    for (auto& iter : CJDisplayManager::cjCbMap_[type]) {
        if (iter.first == callbackId) {
            TLOGE(WmsLogTag::DMS, "IfCallbackRegistered callback already registered!");
            return true;
        }
    }
    return false;
}

int32_t CJDisplayManager::OnUnregisterAllDisplayListenerWithType(const std::string& type)
{
    TLOGD(WmsLogTag::DMS, "CJDisplayManager::OnUnregisterAllDisplayListenerWithType is called");
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(CJDisplayManager::UnregisterAllDisplayListenerWithType(type));
    if (ret != DmErrorCode::DM_OK) {
        if (ret != DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            ret = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
        TLOGE(WmsLogTag::DMS, "Failed to unregister display listener with type %{public}s", type.c_str());
    }
    return static_cast<int32_t>(ret);
}

DMError CJDisplayManager::UnregisterAllDisplayListenerWithType(const std::string& type)
{
    TLOGD(WmsLogTag::DMS, "CJDisplayManager::UnregisterAllDisplayListenerWithType is called");
    if (CJDisplayManager::cjCbMap_.empty() ||
        CJDisplayManager::cjCbMap_.find(type) == CJDisplayManager::cjCbMap_.end()) {
        TLOGI(
            WmsLogTag::DMS, "UnregisterAllDisplayListenerWithType methodName %{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }

    std::lock_guard<std::mutex> lock(CJDisplayManager::mtx_);
    DMError ret = DMError::DM_OK;
    for (auto it = cjCbMap_[type].begin(); it != cjCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
            sptr<DisplayManager::IDisplayListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
        } else if (type == EVENT_PRIVATE_MODE_CHANGE) {
            sptr<DisplayManager::IPrivateWindowListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterPrivateWindowListener(thisListener);
        } else if (type == EVENT_AVAILABLE_AREA_CHANGED) {
            sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
        } else if (type == EVENT_FOLD_STATUS_CHANGED) {
            sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
        } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
            sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
        } else if (type == EVENT_FOLD_ANGLE_CHANGED) {
            sptr<DisplayManager::IFoldAngleListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldAngleListener(thisListener);
        } else if (type == EVENT_CAPTURE_STATUS_CHANGED) {
            sptr<DisplayManager::ICaptureStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterCaptureStatusListener(thisListener);
        } else {
            ret = DMError::DM_ERROR_INVALID_PARAM;
        }
        CJDisplayManager::cjCbMap_[type].erase(it++);
        TLOGI(WmsLogTag::DMS, "Unregister display listener with type %{public}s  ret: %{public}u", type.c_str(), ret);
    }
    cjCbMap_[type].clear();
    return ret;
}

int32_t CJDisplayManager::OnRegisterDisplayListenerWithType(const std::string& type, int64_t callbackId)
{
    TLOGD(WmsLogTag::DMS, "CJDisplayManager::OnRegisterDisplayListenerWithType is called");
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(CJDisplayManager::RegisterDisplayListenerWithType(type, callbackId));
    if (ret != DmErrorCode::DM_OK) {
        if (ret != DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            ret = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
        TLOGE(WmsLogTag::DMS, "Failed to register display listener with type %{public}s", type.c_str());
    }
    return static_cast<int32_t>(ret);
}

DMError CJDisplayManager::RegisterDisplayListenerWithType(const std::string& type, int64_t callbackId)
{
    TLOGD(WmsLogTag::DMS, "CJDisplayManager::RegisterDisplayListenerWithType is called");
    if (IfCallbackRegistered(type, callbackId)) {
        TLOGI(WmsLogTag::DMS, "RegisterDisplayListenerWithType callback with type %{public}s already registered",
            type.c_str());
        return DMError::DM_OK;
    }
    std::lock_guard<std::mutex> lock(CJDisplayManager::mtx_);
    sptr<CJDisplayListener> displayListener = new (std::nothrow) CJDisplayListener();
    DMError ret = DMError::DM_OK;
    if (displayListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayListener is nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayListener(displayListener);
    } else if (type == EVENT_PRIVATE_MODE_CHANGE) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterPrivateWindowListener(displayListener);
    } else if (type == EVENT_FOLD_STATUS_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterFoldStatusListener(displayListener);
    } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayModeListener(displayListener);
    } else if (type == EVENT_AVAILABLE_AREA_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterAvailableAreaListener(displayListener);
    } else if (type == EVENT_FOLD_ANGLE_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterFoldAngleListener(displayListener);
    } else if (type == EVENT_CAPTURE_STATUS_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterCaptureStatusListener(displayListener);
    } else {
        TLOGE(WmsLogTag::DMS, "RegisterDisplayListenerWithType failed, %{public}s not support", type.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "RegisterDisplayListenerWithType failed, ret %{public}u", ret);
        return ret;
    }
    displayListener->AddCallback(type, callbackId);
    CJDisplayManager::cjCbMap_[type][callbackId] = displayListener;
    return DMError::DM_OK;
}

int32_t CJDisplayManager::OnUnRegisterDisplayListenerWithType(const std::string& type, int64_t callbackId)
{
    TLOGD(WmsLogTag::DMS, "CJDisplayManager::OnUnRegisterDisplayListenerWithType is called");
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(CJDisplayManager::UnRegisterDisplayListenerWithType(type, callbackId));
    if (ret != DmErrorCode::DM_OK) {
        if (ret != DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            ret = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
        TLOGE(WmsLogTag::DMS, "Failed to unregister type %{public}s", type.c_str());
    }
    return static_cast<int32_t>(ret);
}

DMError CJDisplayManager::UnRegisterDisplayListenerWithType(const std::string& type, int64_t callbackId)
{
    TLOGD(WmsLogTag::DMS, "CJDisplayManager::UnRegisterDisplayListenerWithType is called");
    if (CJDisplayManager::cjCbMap_.empty() ||
        CJDisplayManager::cjCbMap_.find(type) == CJDisplayManager::cjCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "methodName %{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    std::lock_guard<std::mutex> lock(CJDisplayManager::mtx_);
    DMError ret = DMError::DM_OK;
    for (auto it = CJDisplayManager::cjCbMap_[type].begin(); it != CJDisplayManager::cjCbMap_[type].end();) {
        if (it->first == callbackId) {
            it->second->RemoveCallback(type, callbackId);
            if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
                sptr<DisplayManager::IDisplayListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
            } else if (type == EVENT_PRIVATE_MODE_CHANGE) {
                sptr<DisplayManager::IPrivateWindowListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterPrivateWindowListener(thisListener);
            } else if (type == EVENT_AVAILABLE_AREA_CHANGED) {
                sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
            } else if (type == EVENT_FOLD_STATUS_CHANGED) {
                sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
            } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
                sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
            } else if (type == EVENT_FOLD_ANGLE_CHANGED) {
                sptr<DisplayManager::IFoldAngleListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldAngleListener(thisListener);
            } else if (type == EVENT_CAPTURE_STATUS_CHANGED) {
                sptr<DisplayManager::ICaptureStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterCaptureStatusListener(thisListener);
            } else {
                ret = DMError::DM_ERROR_INVALID_PARAM;
            }
            CJDisplayManager::cjCbMap_[type].erase(it++);
            TLOGI(
                WmsLogTag::DMS, "Unregister display listener with type %{public}s  ret: %{public}u", type.c_str(), ret);
            break;
        } else {
            it++;
        }
    }
    if (CJDisplayManager::cjCbMap_[type].empty()) {
        CJDisplayManager::cjCbMap_.erase(type);
    }
    return ret;
}
} // namespace Rosen
} // namespace OHOS
