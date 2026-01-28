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

#ifndef OHOS_JS_DISPLAY_H
#define OHOS_JS_DISPLAY_H
#include <js_runtime_utils.h>
#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>
#include <refbase.h>

#include "cutout_info.h"
#include "display.h"
#include "dm_common.h"
#include "js_display_listener.h"

namespace OHOS {
namespace Rosen {
enum class DmsColorSpace : uint32_t {
    UNKNOWN = 0,
    ADOBE_RGB = 1,
    BT2020_HLG = 2,
    BT2020_PQ = 3,
    BT601_EBU = 4,
    BT601_SMPTE_C = 5,
    BT709 = 6,
    P3_HLG = 7,
    P3_PQ = 8,
    DISPLAY_P3 = 9,
    SRGB = 10,
    LINEAR_SRGB = 11,
    LINEAR_P3 = 12,
    LINEAR_BT2020 = 13,
};

enum class HDRFormat : uint32_t {
    NONE = 0,
    VIDEO_HLG = 1,
    VIDEO_HDR10 = 2,
    VIDEO_HDR_VIVID = 3,
    IMAGE_HDR_VIVID_DUAL = 4,
    IMAGE_HDR_VIVID_SINGLE = 5,
    IMAGE_HDR_ISO_DUAL = 6,
    IMAGE_HDR_ISO_SINGLE = 7,
};

std::shared_ptr<NativeReference> FindJsDisplayObject(DisplayId displayId);
napi_value CreateJsDisplayObject(napi_env env, sptr<Display>& display);
napi_value CreateJsCutoutInfoObject(napi_env env, sptr<CutoutInfo> cutoutInfo);
napi_value CreateJsRectObject(napi_env env, DMRect rect);
napi_value CreateJsBrightnessInfo(napi_env env, const ScreenBrightnessInfo& brightnessInfo);
napi_value CreateJsWaterfallDisplayAreaRectsObject(napi_env env,
    WaterfallDisplayAreaRects waterfallDisplayAreaRects);
napi_value CreateJsBoundingRectsArrayObject(napi_env env, std::vector<DMRect> boundingRects);
napi_value CreateJsDisplayPhysicalInfoObject(napi_env env, DisplayPhysicalResolution physicalInfo);
napi_value NapiGetUndefined(napi_env env);
napi_valuetype GetType(napi_env env, napi_value value);
napi_value CreateJsFoldCreaseRegionObject(napi_env env, const FoldCreaseRegion& region);
napi_value CreateJsCreaseRectsArrayObject(napi_env env, const std::vector<DMRect>& creaseRects);
napi_value CreateJsRoundedCorner(napi_env env, const std::vector<RoundedCorner>& roundedCorner);
class JsDisplay final {
public:
    explicit JsDisplay(const sptr<Display>& display);
    ~JsDisplay();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value GetCutoutInfo(napi_env env, napi_callback_info info);
    static napi_value HasImmersiveWindow(napi_env env, napi_callback_info info);
    static napi_value GetAvailableArea(napi_env env, napi_callback_info info);
    static napi_value GetSupportedColorSpaces(napi_env env, napi_callback_info info);
    static napi_value GetSupportedHDRFormats(napi_env env, napi_callback_info info);
    static napi_value GetDisplayCapability(napi_env env, napi_callback_info info);
    static napi_value RegisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    static napi_value GetLiveCreaseRegion(napi_env env, napi_callback_info info);
    sptr<Display> GetDisplay() const { return display_; }
    static napi_value GetRoundedCorner(napi_env env, napi_callback_info info);

private:
    sptr<Display> display_ = nullptr;
    std::map<std::string, std::map<std::unique_ptr<NativeReference>, sptr<JsDisplayListener>>> jsCbMap_;
    std::mutex mtx_;
    napi_value OnGetCutoutInfo(napi_env env, napi_callback_info info);
    napi_value OnHasImmersiveWindow(napi_env env, napi_callback_info info);
    napi_value OnGetAvailableArea(napi_env env, napi_callback_info info);
    napi_value OnGetSupportedColorSpaces(napi_env env, napi_callback_info info);
    napi_value OnGetSupportedHDRFormats(napi_env env, napi_callback_info info);
    napi_value OnGetDisplayCapability(napi_env env, napi_callback_info info);
    napi_value OnRegisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    std::unique_ptr<AbilityRuntime::NapiAsyncTask> CreateEmptyAsyncTask(napi_env env,
        napi_value lastParam, napi_value* result);
    DMError RegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value);
    DMError UnregisterAllDisplayListenerWithType(const std::string& type);
    DMError UnRegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value);
    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    napi_value OnGetLiveCreaseRegion(napi_env env, napi_callback_info info);
    napi_value OnGetRoundedCorner(napi_env env, napi_callback_info info);
};
enum class DisplayStateMode : uint32_t {
    STATE_UNKNOWN = 0,
    STATE_OFF,
    STATE_ON,
    STATE_DOZE,
    STATE_DOZE_SUSPEND,
    STATE_VR,
    STATE_ON_SUSPEND
};
}  // namespace Rosen
}  // namespace OHOS
#endif