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
#ifndef OHOS_ANI_SCREEN_UTILS_H
#define OHOS_ANI_SCREEN_UTILS_H
#include <hitrace_meter.h>
 
#include "ani.h"
#include "pixel_map.h"
#include "pixel_map_taihe_ani.h"
#include "singleton_container.h"
#include "screenshot_ani_manager.h"
 
namespace OHOS::Rosen {
 
class ScreenshotAniUtils {
public:
static ani_status GetStdString(ani_env* env, ani_string ani_str, std::string& result);
 
static ani_object CreateAniUndefined(ani_env* env);

static ani_object CreateRectObject(ani_env* env);

static void ConvertRect(ani_env* env, Media::Rect rect, ani_object rectObj);

static ani_object CreateScreenshotPickInfo(ani_env* env, std::unique_ptr<Param>& param);
 
static ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);
 
static ani_status CallAniFunctionVoid(ani_env* env, const char* ns,
    const char* fn, const char* signature, ...);
 
static void ConvertScreenshot(ani_env* env, std::shared_ptr<Media::PixelMap> image, ani_object obj);
 
static ani_status GetScreenshotParam(ani_env* env, const std::unique_ptr<Param>& param, ani_object options);

static ani_status GetHdrScreenshotParam(ani_env* env, const std::unique_ptr<HdrParam>& param, ani_object options);
 
static ani_object CreateArrayPixelMap(ani_env* env, std::vector<std::shared_ptr<Media::PixelMap>> imageVec);

static ani_status GetScreenshotSize(ani_env* env, const std::unique_ptr<Param>& param, ani_object options);

static ani_status GetScreenshotRect(ani_env* env, const std::unique_ptr<Param>& param, ani_object options);

static ani_status ReadOptionalField(ani_env* env, ani_object obj, const char* fieldName, ani_ref& ref);
static ani_status ReadOptionalLongField(ani_env* env, ani_object obj, const char* fieldName, ani_long& value);
static ani_status ReadOptionalIntField(ani_env* env, ani_object obj, const char* fieldName, int& value);
static ani_status ReadOptionalBoolField(ani_env* env, ani_object obj, const char* fieldName, bool& value);
};
}
#endif