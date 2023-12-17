/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at,
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software,
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_DM_DISPLAY_LITE_H
#define FOUNDATION_DM_DISPLAY_LITE_H

#include <string>
#include "dm_common.h"
#include "noncopyable.h"

namespace OHOS::Rosen {
class DisplayInfo;
class CutoutInfo;

class DisplayLite : public RefBase {
friend class DisplayManagerLite;
public:
    ~DisplayLite();
    DisplayLite(const DisplayLite&) = delete;
    DisplayLite(DisplayLite&&) = delete;
    DisplayLite& operator=(const DisplayLite&) = delete;
    DisplayLite& operator=(DisplayLite&&) = delete;
protected:
    // No more methods or variables can be defined here.
    DisplayLite(const std::string& name, sptr<DisplayInfo> info);
private:
    // No more methods or variables can be defined here.
    void UpdateDisplayInfo(sptr<DisplayInfo>) const;
    class Impl;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_DISPLAY_LITE_H