/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BUNDLE_INFO_HELPER_H
#define BUNDLE_INFO_HELPER_H

#include <string>
#include "screen_cache.h"
#include "sys_cap_util.h"

namespace OHOS {
namespace Rosen {
class BundleInfoHelper {
public:
    static std::string GetCurBundleName();
    static std::string GetBundleNameByPid(const int32_t agentPid);
};
} // namespace OHOS
} // namespace Rosen
#endif // BUNDLE_INFO_HELPER_H