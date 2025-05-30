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

#ifndef WM_SYS_CAP_UTIL_H
#define WM_SYS_CAP_UTIL_H

#include <string>

namespace OHOS {
namespace Rosen {
class SysCapUtil {
public:
    static std::string GetClientName();
    static std::string GetBundleName();
    static uint32_t GetApiCompatibleVersion();
private:
    static std::string GetProcessName();
};
} // Rosen
} // OHOS
#endif // WM_BUNDLE_MGR_UTIL_H