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
#ifndef SCREENSHOT_MANAGER_ANI_H
#define SCREENSHOT_MANAGER_ANI_H
 
#include <mutex>
#include "dm_common.h"
#include "refbase.h"
#include "pixel_map.h"
 
#include "ani.h"
 
namespace OHOS::Rosen {
 
struct Option {
    Media::Rect rect;
    Media::Size size;
    int rotation = 0;
    DisplayId displayId = 0;
    bool isNeedNotify = true;
    bool isNeedPointer = true;
    bool isCaptureFullOfScreen = true;
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
 
class ScreenshotManagerAni {
public:
    explicit ScreenshotManagerAni();
    static ani_object Save(ani_env* env, ani_object options);
    static ani_object SaveHdrPicture(ani_env* env, ani_object options);
    static ani_object Capture(ani_env* env, ani_object options);
    static ani_object Pick(ani_env* env);
    static void InitScreenshotManagerAni(ani_namespace nsp, ani_env* env);
    static void GetScreenshot(ani_env *env, std::unique_ptr<Param> &param);
    static void GetHdrScreenshot(ani_env* env, std::unique_ptr<HdrParam> &param);
    static void GetScreenshotCapture(std::unique_ptr<Param>& param);
    static void GetScreenshotPick(std::unique_ptr<Param>& param);
};
}
#endif