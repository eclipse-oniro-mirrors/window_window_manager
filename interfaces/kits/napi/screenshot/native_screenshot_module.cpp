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

#include "native_screenshot_module.h"

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <image_type.h>
#include <iosfwd>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <memory>
#include <napi/native_api.h>
#include <napi/native_common.h>
#include <string>
#include <type_traits>

#include "display_manager.h"
#include "pixel_map.h"
#include "pixel_map_napi.h"
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "dm_napi_common.h"

namespace OHOS::Rosen {
namespace save {
static const uint32_t PIXMAP_VECTOR_ONLY_SDR_SIZE = 1;
static const uint32_t PIXMAP_VECTOR_SIZE = 2;
static const uint32_t SDR_PIXMAP = 0;
static const uint32_t HDR_PIXMAP = 1;
static const uint32_t MAX_ARRAY_SIZE = 1024;
struct Option {
    Media::Rect rect;
    Media::Size size;
    int rotation = 0;
    DisplayId displayId = 0;
    bool isNeedNotify = true;
    bool isNeedPointer = true;
    bool isCaptureFullOfScreen = false;
    std::vector<uint64_t> blackWindowIds = {};
};

struct Param {
    DmErrorCode wret;
    Option option;
    std::string errMessage;
    bool useInputOption;
    bool validInputParam;
    std::shared_ptr<Media::PixelMap> image;
    Media::Rect imageRect;
    bool isPick;
};

struct HdrParam {
    DmErrorCode wret;
    Option option;
    std::string errMessage;
    bool useInputOption;
    bool validInputParam;
    std::vector<std::shared_ptr<Media::PixelMap>> imageVec;
    Media::Rect imageRect;
    bool isPick;
};

static napi_valuetype GetType(napi_env env, napi_value root)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, root, &res);
    return res;
}

static void GetDisplayId(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: GetDisplayId");
    napi_value displayId;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "displayId", &displayId));
    if (displayId != nullptr && GetType(env, displayId) == napi_number) {
        int64_t dispId;
        NAPI_CALL_RETURN_VOID(env, napi_get_value_int64(env, displayId, &dispId));
        param->option.displayId = static_cast<DisplayId>(dispId);
        TLOGI(WmsLogTag::DMS, "GetDisplayId success, displayId = %{public}" PRIu64"", param->option.displayId);
    } else {
        TLOGI(WmsLogTag::DMS, "GetDisplayId failed, invalid param, use default displayId = 0");
    }
}

static void GetRotation(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: GetRotation");
    napi_value rotation;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "rotation", &rotation));
    if (rotation != nullptr && GetType(env, rotation) == napi_number) {
        NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, rotation, &param->option.rotation));
        TLOGI(WmsLogTag::DMS, "GetRotation success, rotation = %{public}d", param->option.rotation);
    } else {
        TLOGI(WmsLogTag::DMS, "GetRotation failed, invalid param, use default rotation = 0");
    }
}

static void GetScreenRect(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: GetScreenRect");
    napi_value screenRect;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "screenRect", &screenRect));
    if (screenRect != nullptr && GetType(env, screenRect) == napi_object) {
        TLOGI(WmsLogTag::DMS, "get ScreenRect success");

        napi_value left;
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, screenRect, "left", &left));
        if (left != nullptr && GetType(env, left) == napi_number) {
            NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, left, &param->option.rect.left));
            TLOGI(WmsLogTag::DMS, "left success, left = %{public}d", param->option.rect.left);
        } else {
            TLOGI(WmsLogTag::DMS, "left failed, invalid param, use default left = 0");
        }

        napi_value top;
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, screenRect, "top", &top));
        if (top != nullptr && GetType(env, top) == napi_number) {
            NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, top, &param->option.rect.top));
            TLOGI(WmsLogTag::DMS, "top success, top = %{public}d", param->option.rect.top);
        } else {
            TLOGI(WmsLogTag::DMS, "top failed, invalid param, use default top = 0");
        }

        napi_value width;
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, screenRect, "width", &width));
        if (width != nullptr && GetType(env, width) == napi_number) {
            NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, width, &param->option.rect.width));
            TLOGI(WmsLogTag::DMS, "width success, width = %{public}d", param->option.rect.width);
        } else {
            TLOGI(WmsLogTag::DMS, "width failed, invalid param, use default width = 0");
        }

        napi_value height;
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, screenRect, "height", &height));
        if (height != nullptr && GetType(env, height) == napi_number) {
            NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, height, &param->option.rect.height));
            TLOGI(WmsLogTag::DMS, "height success, height = %{public}d", param->option.rect.height);
        } else {
            TLOGI(WmsLogTag::DMS, "height failed, invalid param, use default height = 0");
        }
    } else {
        TLOGI(WmsLogTag::DMS, "get ScreenRect failed, use default ScreenRect param");
    }
}

static void GetImageSize(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: ImageSize");
    napi_value imageSize;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "imageSize", &imageSize));
    if (imageSize != nullptr && GetType(env, imageSize) == napi_object) {
        napi_value width;
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, imageSize, "width", &width));
        if (width != nullptr && GetType(env, width) == napi_number) {
            NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, width, &param->option.size.width));
            TLOGI(WmsLogTag::DMS, "width success, width = %{public}d", param->option.size.width);
        } else {
            TLOGI(WmsLogTag::DMS, "width failed, invalid param, use default width = 0");
        }

        napi_value height;
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, imageSize, "height", &height));
        if (height != nullptr && GetType(env, height) == napi_number) {
            NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, height, &param->option.size.height));
            TLOGI(WmsLogTag::DMS, "height success, height = %{public}d", param->option.size.height);
        } else {
            TLOGI(WmsLogTag::DMS, "height failed, invalid param, use default height = 0");
        }
    }
}

static void IsNeedNotify(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: IsNeedNotify");
    napi_value isNeedNotify;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "isNotificationNeeded", &isNeedNotify));
    if (isNeedNotify != nullptr && GetType(env, isNeedNotify) == napi_boolean) {
        NAPI_CALL_RETURN_VOID(env, napi_get_value_bool(env, isNeedNotify, &param->option.isNeedNotify));
        TLOGI(WmsLogTag::DMS, "IsNeedNotify: %{public}d", param->option.isNeedNotify);
    } else {
        TLOGI(WmsLogTag::DMS, "IsNeedNotify failed, invalid param, use default true.");
    }
}

static void IsNeedPointer(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: IsNeedPointer");
    napi_value isNeedPointer;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "isPointerNeeded", &isNeedPointer));
    if (isNeedPointer != nullptr && GetType(env, isNeedPointer) == napi_boolean) {
        NAPI_CALL_RETURN_VOID(env, napi_get_value_bool(env, isNeedPointer, &param->option.isNeedPointer));
        TLOGI(WmsLogTag::DMS, "IsNeedPointer: %{public}d", param->option.isNeedPointer);
    } else {
        TLOGI(WmsLogTag::DMS, "IsNeedPointer failed, invalid param, use default true.");
    }
}

static void ConvertWindowIdList(napi_env env, std::unique_ptr<Param>& param, napi_value& argv)
{
    param->validInputParam = false;
    napi_value blackWindowIds;
    napi_status status = napi_get_named_property(env, argv, "blackWindowIds", &blackWindowIds);
    if (status != napi_ok) {
        TLOGD(WmsLogTag::DMS, "no black window ids.");
        param->validInputParam = true;
        return;
    }
    uint32_t size = 0;
    if (GetType(env, blackWindowIds) != napi_object ||
        napi_get_array_length(env, blackWindowIds, &size) == napi_invalid_arg) {
        TLOGD(WmsLogTag::DMS, "no surface id list");
        param->validInputParam = true;
        return;
    }
    if (size > MAX_ARRAY_SIZE) {
        TLOGE(WmsLogTag::DMS, "size bigger than 1024, size: %{public}u", size);
        return;
    }
    std::vector<uint64_t> persistentIds;
    for (uint32_t i = 0; i < size; i++) {
        int64_t persistentId = 0;
        napi_value element = nullptr;
        napi_get_element(env, blackWindowIds, i, &element);
        if (napi_get_value_int64(env, element, &persistentId) != napi_ok) {
            TLOGE(WmsLogTag::DMS, "window id is not number");
            return;
        }
        if (persistentId < 0) {
            TLOGE(WmsLogTag::DMS, "window id is negative: %{public}" PRId64, persistentId);
            continue;
        }
        param->option.blackWindowIds.push_back(static_cast<uint64_t>(persistentId));
    }
    param->validInputParam = true;
    return;
}

static void IsCaptureFullOfScreen(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    TLOGI(WmsLogTag::DMS, "Get Screenshot Option: isCaptureFullOfScreen");
    napi_value isCaptureFullOfScreen;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "isCaptureFullOfScreen", &isCaptureFullOfScreen));
    if (isCaptureFullOfScreen != nullptr && GetType(env, isCaptureFullOfScreen) == napi_boolean) {
        NAPI_CALL_RETURN_VOID(env, napi_get_value_bool(env, isCaptureFullOfScreen,
            &param->option.isCaptureFullOfScreen));
        TLOGI(WmsLogTag::DMS, "isCaptureFullOfScreen: %{public}d", param->option.isCaptureFullOfScreen);
    } else {
        TLOGI(WmsLogTag::DMS, "isCaptureFullOfScreen failed, invalid param, use default false.");
    }
}

static void GetScreenshotParam(napi_env env, std::unique_ptr<Param> &param, napi_value &argv)
{
    if (param == nullptr) {
        TLOGI(WmsLogTag::DMS, "param == nullptr, use default param");
        return;
    }
    GetDisplayId(env, param, argv);
    GetRotation(env, param, argv);
    GetScreenRect(env, param, argv);
    GetImageSize(env, param, argv);
    IsNeedNotify(env, param, argv);
    IsNeedPointer(env, param, argv);
    IsCaptureFullOfScreen(env, param, argv);
    ConvertWindowIdList(env, param, argv);
}

static void AsyncGetScreenshot(napi_env env, std::unique_ptr<Param> &param)
{
    if (!param->validInputParam) {
        TLOGE(WmsLogTag::DMS, "Invalid Input Param!");
        param->image = nullptr;
        param->wret = DmErrorCode::DM_ERROR_INVALID_PARAM;
        param->errMessage = "[screenshot][pick]msg: Get Screenshot Failed: Invalid input param";
        return;
    }
    CaptureOption option = { param->option.displayId, param->option.isNeedNotify, param->option.isNeedPointer,
        param->option.isCaptureFullOfScreen};
    if (!param->isPick && (!option.isNeedNotify_ || !option.isNeedPointer_)) {
        if (param->useInputOption) {
            param->image = DisplayManager::GetInstance().GetScreenshotWithOption(option,
                param->option.rect, param->option.size, param->option.rotation, &param->wret);
        } else {
            param->image = DisplayManager::GetInstance().GetScreenshotWithOption(option, &param->wret);
        }
    } else {
        if (param->useInputOption) {
            TLOGI(WmsLogTag::DMS, "Get Screenshot by input option");
            SnapShotConfig snapConfig;
            snapConfig.displayId_ = param->option.displayId;
            snapConfig.imageRect_ = param->option.rect;
            snapConfig.imageSize_ = param->option.size;
            snapConfig.rotation_ = param->option.rotation;
            snapConfig.isCaptureFullOfScreen_ = param->option.isCaptureFullOfScreen;
            param->image = DisplayManager::GetInstance().GetScreenshotwithConfig(snapConfig, &param->wret, true);
        } else if (param->isPick) {
            TLOGI(WmsLogTag::DMS, "Get Screenshot by picker");
            param->image = DisplayManager::GetInstance().GetSnapshotByPicker(param->imageRect, &param->wret);
        } else {
            TLOGI(WmsLogTag::DMS, "Get Screenshot by default option");
            param->image = DisplayManager::GetInstance().GetScreenshot(param->option.displayId, &param->wret, true,
                param->option.isCaptureFullOfScreen);
        }
    }
    if (param->image == nullptr && param->wret == DmErrorCode::DM_OK) {
        TLOGI(WmsLogTag::DMS, "Get Screenshot failed!");
        param->wret = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        param->errMessage = "[screenshot][pick]msg: Get Screenshot failed: Screenshot image is nullptr";
        return;
    }
}

static void AsyncGetScreenHDRshot(napi_env env, std::unique_ptr<HdrParam>& param)
{
    if (!param->validInputParam) {
        TLOGE(WmsLogTag::DMS, "Invalid Input Param!");
        param->imageVec = {};
        param->wret = DmErrorCode::DM_ERROR_INVALID_PARAM;
        param->errMessage = "Get Screenshot Failed: Invalid input param";
        return;
    }
    CaptureOption option = { param->option.displayId, param->option.isNeedNotify, param->option.isNeedPointer,
        param->option.isCaptureFullOfScreen};
    if (!option.isNeedNotify_) {
        param->imageVec = DisplayManager::GetInstance().GetScreenHDRshotWithOption(option, param->wret);
    } else {
        TLOGI(WmsLogTag::DMS, "Get Screenshot by default option");
        param->imageVec = DisplayManager::GetInstance().GetScreenHDRshot(param->option.displayId, param->wret,
            true, param->option.isCaptureFullOfScreen);
    }
    if ((param->imageVec.size() != PIXMAP_VECTOR_SIZE || param->imageVec[SDR_PIXMAP] == nullptr) &&
        param->wret == DmErrorCode::DM_OK) {
        TLOGI(WmsLogTag::DMS, "Get Screenshot failed!");
        param->wret = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        param->errMessage = "Get Screenshot failed: Screenshot imageVec is nullptr";
        return;
    }
}

napi_value CreateJsNumber(napi_env env, int32_t value)
{
    napi_value valRet = nullptr;
    napi_create_int32(env, value, &valRet);
    return valRet;
}

napi_value CreateJsRectObject(napi_env env, Media::Rect imageRect)
{
    napi_value objValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objValue));
    napi_set_named_property(env, objValue, "left", CreateJsNumber(env, imageRect.left));
    napi_set_named_property(env, objValue, "top", CreateJsNumber(env, imageRect.top));
    napi_set_named_property(env, objValue, "width", CreateJsNumber(env, imageRect.width));
    napi_set_named_property(env, objValue, "height", CreateJsNumber(env, imageRect.height));
    return objValue;
}

napi_value CreateJsPickerObject(napi_env env, std::unique_ptr<Param> &param)
{
    napi_value objValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objValue));
    if (param == nullptr) {
        napi_value result;
        TLOGE(WmsLogTag::DMS, "param nullptr.");
        NAPI_CALL(env, napi_get_undefined(env, &result));
        return result;
    }
    napi_set_named_property(env, objValue, "pixelMap", OHOS::Media::PixelMapNapi::CreatePixelMap(env, param->image));
    napi_set_named_property(env, objValue, "pickRect", CreateJsRectObject(env, param->imageRect));
    TLOGI(WmsLogTag::DMS, "pick end");
    return objValue;
}

napi_value Resolve(napi_env env, std::unique_ptr<Param> &param)
{
    napi_value result;
    napi_value error;
    napi_value code;
    bool isThrowError = true;
    if (param->wret != DmErrorCode::DM_OK) {
        napi_create_error(env, nullptr, nullptr, &error);
        napi_create_int32(env, (int32_t)param->wret, &code);
    }
    switch (param->wret) {
        case DmErrorCode::DM_ERROR_NO_PERMISSION:
            napi_set_named_property(env, error, "DM_ERROR_NO_PERMISSION", code);
            break;
        case DmErrorCode::DM_ERROR_INVALID_PARAM:
            napi_set_named_property(env, error, "DM_ERROR_INVALID_PARAM", code);
            break;
        case DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT:
            napi_set_named_property(env, error, "DM_ERROR_DEVICE_NOT_SUPPORT", code);
            break;
        case DmErrorCode::DM_ERROR_SYSTEM_INNORMAL:
            napi_set_named_property(env, error, "DM_ERROR_SYSTEM_INNORMAL", code);
            break;
        default:
            isThrowError = false;
            TLOGI(WmsLogTag::DMS, "screen shot default.");
            break;
    }
    TLOGI(WmsLogTag::DMS, "screen shot ret=%{public}d.", param->wret);
    if (isThrowError) {
        TLOGE(WmsLogTag::DMS, "screen shot isThrowError ret=%{public}d.", param->wret);
        napi_throw(env, error);
        return error;
    }
    if (param->wret != DmErrorCode::DM_OK) {
        NAPI_CALL(env, napi_get_undefined(env, &result));
        return result;
    }
    if (param->isPick) {
        TLOGI(WmsLogTag::DMS, "Resolve Screenshot by picker");
        return CreateJsPickerObject(env, param);
    }
    if (param->image != nullptr) {
        TLOGI(WmsLogTag::DMS, "Screenshot image Width %{public}d, Height %{public}d",
            param->image->GetWidth(), param->image->GetHeight());
    }
    napi_value jsImage = OHOS::Media::PixelMapNapi::CreatePixelMap(env, param->image);
    return jsImage;
}

napi_value CreateJsPixelmapVec(napi_env env, std::unique_ptr<HdrParam>& param)
{
    napi_value jsPixelmapVec = nullptr;
    napi_value result;
    uint32_t pixReturnJsVecLength = 0;
    if (param->imageVec[HDR_PIXMAP] == nullptr) {
        pixReturnJsVecLength = PIXMAP_VECTOR_ONLY_SDR_SIZE;
    } else {
        pixReturnJsVecLength = PIXMAP_VECTOR_SIZE;
    }
    napi_create_array_with_length(env, pixReturnJsVecLength, &jsPixelmapVec);
    if (jsPixelmapVec == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create pixelmap array");
        NAPI_CALL(env, napi_get_undefined(env, &result));
        return result;
    }
    uint32_t index = 0;
    for (const auto& pixelmap : param->imageVec) {
        if (pixelmap == nullptr) {
            break;
        }
        napi_set_element(env, jsPixelmapVec, index++, OHOS::Media::PixelMapNapi::CreatePixelMap(env, pixelmap));
    }
    return jsPixelmapVec;
}

napi_value HDRResolve(napi_env env, std::unique_ptr<HdrParam>& param)
{
    napi_status ret = napi_ok;
    napi_value result;
    napi_value error;
    napi_value code;
    bool isThrowError = true;
    if (param->wret != DmErrorCode::DM_OK) {
        napi_create_error(env, nullptr, nullptr, &error);
        napi_create_int32(env, (int32_t)param->wret, &code);
    }
    switch (param->wret) {
        case DmErrorCode::DM_ERROR_NO_PERMISSION:
            ret = napi_set_named_property(env, error, "DM_ERROR_NO_PERMISSION", code);
            if (ret != napi_ok) {
                TLOGE(WmsLogTag::DMS, "napi_set_named_property error, code is %{public}d", ret);
            }
            break;
        case DmErrorCode::DM_ERROR_NOT_SYSTEM_APP:
            ret = napi_set_named_property(env, error, "DM_ERROR_NOT_SYSTEM_APP", code);
            if (ret != napi_ok) {
                TLOGE(WmsLogTag::DMS, "napi_set_named_property error, code is %{public}d", ret);
            }
            break;
        case DmErrorCode::DM_ERROR_INVALID_SCREEN:
            ret = napi_set_named_property(env, error, "DM_ERROR_INVALID_SCREEN", code);
            if (ret != napi_ok) {
                TLOGE(WmsLogTag::DMS, "napi_set_named_property error, code is %{public}d", ret);
            }
            break;
        case DmErrorCode::DM_ERROR_SYSTEM_INNORMAL:
            ret = napi_set_named_property(env, error, "DM_ERROR_SYSTEM_INNORMAL", code);
            if (ret != napi_ok) {
                TLOGE(WmsLogTag::DMS, "napi_set_named_property error, code is %{public}d", ret);
            }
            break;
        case DmErrorCode::DM_ERROR_ILLEGAL_PARAM:
            ret = napi_set_named_property(env, error, "DM_ERROR_ILLEGAL_PARAM", code);
            if (ret != napi_ok) {
                TLOGE(WmsLogTag::DMS, "napi_set_named_property error, code is %{public}d", ret);
            }
            break;
        default:
            isThrowError = false;
            break;
    }
    TLOGI(WmsLogTag::DMS, "screen shot ret=%{public}d.", param->wret);
    if (isThrowError) {
        napi_throw(env, error);
        return error;
    }
    if (param->wret != DmErrorCode::DM_OK) {
        NAPI_CALL(env, napi_get_undefined(env, &result));
        return result;
    }

    return CreateJsPixelmapVec(env, param);
}

napi_value PickFunc(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "%{public}s called", __PRETTY_FUNCTION__);
    napi_value argv[1] = { nullptr };  // the max number of input parameters is 1
    size_t argc = 1;  // the max number of input parameters is 1
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    auto param = std::make_unique<Param>();
    if (param == nullptr) {
        TLOGE(WmsLogTag::DMS, "Create param failed.");
        return nullptr;
    }
    napi_ref ref = nullptr;
    if (argc == 0) {  // 0 valid parameters
        TLOGI(WmsLogTag::DMS, "argc == 0");
        param->validInputParam = true;
    } else if (GetType(env, argv[0]) == napi_function) {  // 1 valid parameters napi_function
        TLOGI(WmsLogTag::DMS, "argc >= 1, argv[0]'s type is napi_function");
        param->validInputParam = true;
        NAPI_CALL(env, napi_create_reference(env, argv[0], 1, &ref));
    } else {  // 0 valid parameters
        TLOGI(WmsLogTag::DMS, "argc != 0");
        param->validInputParam = true;
    }
    param->isPick = true;
    return AsyncProcess<Param>(env, __PRETTY_FUNCTION__, AsyncGetScreenshot, Resolve, ref, param);
}

static void AsyncGetScreenCapture(napi_env env, std::unique_ptr<Param> &param)
{
    if (!param->validInputParam) {
        TLOGE(WmsLogTag::DMS, "screen capture failed!");
        param->wret = DmErrorCode::DM_ERROR_SYSTEM_INNORMAL;
        param->errMessage = "[screenshot][capture]msg: Parameter error. Possible causes: "
            "1. Mandatory parameters are left unspecified; "
            "2. Incorrect parameter types.";
        return;
    }
    CaptureOption captureOption;
    captureOption.displayId_ = param->option.displayId;
    captureOption.isNeedNotify_ = param->option.isNeedNotify;
    captureOption.isNeedPointer_ = param->option.isNeedPointer;
    captureOption.blackWindowIdList_ = param->option.blackWindowIds;
    std::ostringstream oss;
    for (size_t i = 0; i < captureOption.blackWindowIdList_.size(); ++i) {
        oss << captureOption.blackWindowIdList_[i] << ", ";
    }
    TLOGI(WmsLogTag::DMS, "capture option isNeedNotify=%{public}d isNeedPointer=%{public}d  blackWindowIds=%{public}s",
        captureOption.isNeedNotify_, captureOption.isNeedPointer_, oss.str().c_str());
    param->image = DisplayManager::GetInstance().GetScreenCapture(captureOption, &param->wret);
    if (param->image == nullptr && param->wret == DmErrorCode::DM_OK) {
        TLOGI(WmsLogTag::DMS, "screen capture failed!");
        param->wret = DmErrorCode::DM_ERROR_SYSTEM_INNORMAL;
        param->errMessage = "[screenshot][capture]msg: ScreenCapture failed: image is null.";
        return;
    }
}

napi_value CaptureFunc(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "%{public}s called", __PRETTY_FUNCTION__);
    napi_value argv[1] = { nullptr };
    size_t argc = 1;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    auto param = std::make_unique<Param>();
    if (param == nullptr) {
        TLOGE(WmsLogTag::DMS, "Create param failed.");
        return nullptr;
    }
    param->option.displayId = DisplayManager::GetInstance().GetDefaultDisplayId();
    napi_ref ref = nullptr;
    param->validInputParam = true;
    if (argc > 0 && GetType(env, argv[0]) == napi_object) {
        TLOGI(WmsLogTag::DMS, "argv[0]'s type is napi_object");
        GetScreenshotParam(env, param, argv[0]);
    } else {
        TLOGI(WmsLogTag::DMS, "use default.");
    }
    return AsyncProcess<Param>(env, __PRETTY_FUNCTION__, AsyncGetScreenCapture, Resolve, ref, param);
}

napi_value MainFunc(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "%{public}s called", __PRETTY_FUNCTION__);
    napi_value argv[2] = {nullptr}; // the max number of input parameters is 2
    size_t argc = 2; // the max number of input parameters is 2
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    auto param = std::make_unique<Param>();
    if (param == nullptr) {
        TLOGE(WmsLogTag::DMS, "Create param failed.");
        return nullptr;
    }
    param->option.displayId = DisplayManager::GetInstance().GetDefaultDisplayId();
    napi_ref ref = nullptr;
    if (argc == 0) { // 0 valid parameters
        TLOGI(WmsLogTag::DMS, "argc == 0");
        param->validInputParam = true;
    } else if (GetType(env, argv[0]) == napi_function) { // 1 valid parameters napi_function
        TLOGI(WmsLogTag::DMS, "argc >= 1, argv[0]'s type is napi_function");
        param->validInputParam = true;
        NAPI_CALL(env, napi_create_reference(env, argv[0], 1, &ref));
    } else if (GetType(env, argv[0]) == napi_object) {
        if ((argc >= 2) && (GetType(env, argv[1]) == napi_function)) { // 2 valid parameters napi_object napi_function
            TLOGI(WmsLogTag::DMS, "argc >= 2, argv[0]'s type is napi_object, argv[1]'s type is napi_function");
            param->validInputParam = true;
            param->useInputOption = true;
            GetScreenshotParam(env, param, argv[0]);
            NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &ref));
        } else { // 1 valid parameters napi_object
            TLOGI(WmsLogTag::DMS, "argc >= 1, argv[0]'s type is napi_object");
            param->validInputParam = true;
            param->useInputOption = true;
            GetScreenshotParam(env, param, argv[0]);
        }
    } else { // 0 valid parameters
        TLOGI(WmsLogTag::DMS, "argc == 0");
        param->validInputParam = true;
    }
    param->isPick = false;
    return AsyncProcess<Param>(env, __PRETTY_FUNCTION__, AsyncGetScreenshot, Resolve, ref, param);
}

napi_value SaveHDRFunc(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "%{public}s called", __PRETTY_FUNCTION__);
    napi_value argv[1] = {nullptr}; // the max number of input parameters is 1
    size_t argc = 1; // the max number of input parameters is 1
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
 
    auto param = std::make_unique<Param>();
    auto hdrParam = std::make_unique<HdrParam>();
    if ((param == nullptr) || (hdrParam == nullptr)) {
        TLOGE(WmsLogTag::DMS, "Create param failed.");
        return nullptr;
    }
    param->option.displayId = DisplayManager::GetInstance().GetDefaultDisplayId();
    napi_ref ref = nullptr;
    if (argc == 0) { // 0 valid parameters
        TLOGI(WmsLogTag::DMS, "argc == 0");
        param->validInputParam = true;
    } else if (GetType(env, argv[0]) == napi_object) {
        TLOGI(WmsLogTag::DMS, "argc = 1, argv[0]'s type is napi_object");
        param->validInputParam = true;
        param->useInputOption = true;
        GetScreenshotParam(env, param, argv[0]);
    } else { // parameters > 1
        TLOGI(WmsLogTag::DMS, "argc == 0");
        param->validInputParam = false;
        param->wret = DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }
    hdrParam->wret = param->wret;
    hdrParam->option = param->option;
    hdrParam->errMessage = param->errMessage;
    hdrParam->validInputParam = param->validInputParam;
    hdrParam->useInputOption = param->useInputOption;
    return AsyncProcess<HdrParam>(env, __PRETTY_FUNCTION__, AsyncGetScreenHDRshot, HDRResolve, ref, hdrParam);
}
} // namespace save

void SetNamedProperty(napi_env env, napi_value dstObj, const int32_t objValue, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_int32(env, objValue, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

void SetDmErrorObjectProperty(napi_env env, napi_value errorCode)
{
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED, "DM_ERROR_INIT_DMS_PROXY_LOCKED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_IPC_FAILED, "DM_ERROR_IPC_FAILED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_REMOTE_CREATE_FAILED, "DM_ERROR_REMOTE_CREATE_FAILED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_NULLPTR, "DM_ERROR_NULLPTR");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_INVALID_PARAM, "DM_ERROR_INVALID_PARAM");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED, "DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_DEATH_RECIPIENT, "DM_ERROR_DEATH_RECIPIENT");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_INVALID_MODE_ID, "DM_ERROR_INVALID_MODE_ID");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_WRITE_DATA_FAILED, "DM_ERROR_WRITE_DATA_FAILED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_RENDER_SERVICE_FAILED, "DM_ERROR_RENDER_SERVICE_FAILED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_UNREGISTER_AGENT_FAILED, "DM_ERROR_UNREGISTER_AGENT_FAILED");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_INVALID_CALLING, "DM_ERROR_INVALID_CALLING");
    SetNamedProperty(env, errorCode,
        (int32_t)DMError::DM_ERROR_UNKNOWN, "DM_ERROR_UNKNOWN");
}

void SetDmErrorCodeObjectProperty(napi_env env, napi_value dmErrorCode)
{
    SetNamedProperty(env, dmErrorCode,
        (int32_t)DmErrorCode::DM_ERROR_NO_PERMISSION, "DM_ERROR_NO_PERMISSION");
    SetNamedProperty(env, dmErrorCode,
        (int32_t)DmErrorCode::DM_ERROR_INVALID_PARAM, "DM_ERROR_INVALID_PARAM");
    SetNamedProperty(env, dmErrorCode,
        (int32_t)DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT, "DM_ERROR_DEVICE_NOT_SUPPORT");
    SetNamedProperty(env, dmErrorCode,
        (int32_t)DmErrorCode::DM_ERROR_INVALID_SCREEN, "DM_ERROR_INVALID_SCREEN");
    SetNamedProperty(env, dmErrorCode,
        (int32_t)DmErrorCode::DM_ERROR_INVALID_CALLING, "DM_ERROR_INVALID_CALLING");
    SetNamedProperty(env, dmErrorCode,
        (int32_t)DmErrorCode::DM_ERROR_SYSTEM_INNORMAL, "DM_ERROR_SYSTEM_INNORMAL");
}

napi_value ScreenshotModuleInit(napi_env env, napi_value exports)
{
    TLOGI_LIMITN_HOUR(WmsLogTag::DMS, TEN_TIMES, "called");

    napi_value errorCode = nullptr;
    napi_value dmErrorCode = nullptr;
    napi_create_object(env, &errorCode);
    napi_create_object(env, &dmErrorCode);
    SetDmErrorObjectProperty(env, errorCode);
    SetDmErrorCodeObjectProperty(env, dmErrorCode);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("save", save::MainFunc),
        DECLARE_NAPI_FUNCTION("saveHdrPicture", save::SaveHDRFunc),
        DECLARE_NAPI_FUNCTION("pick", save::PickFunc),
        DECLARE_NAPI_FUNCTION("capture", save::CaptureFunc),
        DECLARE_NAPI_PROPERTY("DMError", errorCode),
        DECLARE_NAPI_PROPERTY("DmErrorCode", dmErrorCode),
    };

    NAPI_CALL(env, napi_define_properties(env,
        exports, sizeof(properties) / sizeof(properties[0]), properties));
    return exports;
}
} // namespace OHOS::Rosen

static napi_module g_screenshotModule = {
    .nm_version = 1, // NAPI v1
    .nm_flags = 0, // normal
    .nm_filename = nullptr,
    .nm_register_func = OHOS::Rosen::ScreenshotModuleInit,
    .nm_modname = "screenshot",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_screenshotModule);
}
