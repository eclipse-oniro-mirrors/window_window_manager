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
#ifndef OHOS_ANI_DISPLAY_UTILS_H
#define OHOS_ANI_DISPLAY_UTILS_H

#include <hitrace_meter.h>

#include "ani.h"
#include "display_ani.h"
#include "dm_common.h"
#include "display_manager.h"
#include "refbase.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"
#include "surface_utils.h"
#include "screen_manager.h"

namespace OHOS {
namespace Rosen {

class DisplayAniUtils {
public:
static void ConvertRect(DMRect rect, ani_object rectObj, ani_env* env);

static void ConvertWaterArea(WaterfallDisplayAreaRects waterfallDisplayAreaRects,
    ani_object waterfallObj, ani_env *env);

static void ConvertDisplayPhysicalResolution(std::vector<DisplayPhysicalResolution> &displayPhysicalArray,
    ani_object arrayObj, ani_env *env);

static void ConvertRoundedCorner(const std::vector<RoundedCorner>& roundedCornerArray,
    ani_object arrayObj, ani_env *env);

static ani_enum_item CreateAniEnum(ani_env* env, const char* enum_descriptor, ani_size index);

static ani_status CvtDisplay(sptr<Display> display, ani_env* env, ani_object obj);

static void CvtDisplayHelper(sptr<Display> display, ani_env* env, ani_object obj, sptr<DisplayInfo> info);

static ani_status GetStdString(ani_env *env, ani_string ani_str, std::string &result);

static ani_status GetAniArrayInt(ani_env *env, ani_object arrayObj, std::vector<int32_t>& arrayNative);

static ani_status NewAniObject(ani_env* env, ani_class cls, const char *signature, ani_object* result, ...);

static ani_status NewAniObjectNoParams(ani_env* env, ani_class cls, ani_object* object);

static ani_object CreateAniUndefined(ani_env* env);

static ani_object CreateOptionalInt(ani_env *env, ani_int value);

static void CreateAniArrayInt(ani_env* env, ani_size size, ani_array *aniArray, std::vector<uint32_t> vec);

static void CreateAniArrayDouble(ani_env* env, ani_size size, ani_array *aniArray, std::vector<float> vec);

static ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);

static DmErrorCode GetVirtualScreenOptionFromAni(
    ani_env* env, ani_object virtualScreenConfigObj, VirtualScreenOption& option);

static bool GetSurfaceFromAni(ani_env* env, ani_string surfaceIdAniValue, sptr<Surface>& surface);

static ani_status CallAniFunctionVoid(ani_env* env, const char* ns,
    const char* fn, const char* signature, ...);

static ani_object CreateRectObject(ani_env* env);

static ani_object CreatePositionObject(ani_env* env);
 
static ani_object CreateRelativePostionObject(ani_env* env);
 
static DmErrorCode GetPositionFromAni(ani_env* env, Position& globalPosition, ani_object positionObj);
 
static DmErrorCode SetPositionObj(ani_env* env, Position& globalPosition, ani_object positionObj);
 
static DmErrorCode GetRelativePostionFromAni(
    ani_env* env, RelativePosition& relativePosition, ani_object relativePositionObj);
 
static DmErrorCode SetRelativePostionObj(
    ani_env* env, RelativePosition& relativePosition, ani_object relativePositionObj);
 
static void SetFoldCreaseRegion(ani_env* env, FoldCreaseRegion& region, ani_object foldCreaseRegionObj);

static std::shared_ptr<DisplayAni> FindAniDisplayObject(sptr<Display> display, DisplayId displayId);

static void DisposeAniDisplayObject(DisplayId displayId);

static ani_status CvtBrightnessInfo(ani_env* env, ani_object obj, ScreenBrightnessInfo brightnessInfo);

static ani_object CreateBrightnessInfoObject(ani_env* env);

static ani_status GetStdStringVector(ani_env* env, ani_object arryObj, std::vector<std::string>& result);
};
}
}
#endif
