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
#include "screen.h"
#include "singleton_container.h"

namespace OHOS {
namespace Rosen {

class ScreenAniUtils {
public:
static ani_status GetStdString(ani_env *env, ani_string ani_str, std::string &result);

static ani_object CreateAniUndefined(ani_env* env);

static ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);

static ani_status CallAniFunctionVoid(ani_env *env, const char* ns,
    const char* fn, const char* signature, ...);

static ani_status ConvertScreen(ani_env *env, sptr<Screen> screen, ani_object screenAni);

static void ConvertScreenMode(ani_env* env, sptr<SupportedScreenModes> mode, ani_object obj);

static ani_status ConvertScreens(ani_env *env, std::vector<sptr<Screen>> screen, ani_object& screensAni);

static ani_object NewNativeObject(ani_env* env, const std::string& objName);

static ani_array NewNativeArray(ani_env* env, const std::string& objName, uint32_t size);

static ani_enum_item CreateAniEnum(ani_env* env, const char* enum_descriptor, ani_size index);

static DmErrorCode GetVirtualScreenOption(ani_env* env, ani_object options, VirtualScreenOption& option);

static ani_status GetSurfaceFromAni(ani_env* env, ani_string surfaceIdAniStr, sptr<Surface>& surface);

static ani_status GetMultiScreenPositionOptionsFromAni(ani_env* env, ani_object screenOptionsAni,
    MultiScreenPositionOptions& mainScreenOptions);

static ani_object CreateDisplayIdVectorAniObject(ani_env* env, std::vector<DisplayId>& displayIds);

static ani_object CreateAniArray(ani_env* env, size_t size);

static ani_status GetRectFromAni(ani_env* env, ani_object mainScreenRegionAni, DMRect& mainScreenRegion);

static ani_status GetScreenIdArrayFromAni(ani_env* env, ani_object mirrorScreen,
    std::vector<ScreenId>& mirrorScreenIds);

static ani_status GetExpandOptionFromAni(ani_env* env, ani_object optionAniObj, ExpandOption& expandOption);
};
}
}
#endif