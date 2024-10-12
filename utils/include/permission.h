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

#ifndef WM_PERMISSION_H
#define WM_PERMISSION_H

#include <string>

namespace OHOS {
namespace Rosen {
class Permission {
public:
    static bool IsSystemServiceCalling(bool needPrintLog = true, bool isLocalSysCalling = false);
    static bool IsSystemCalling(bool isLocalSysCalling = false);
    static bool CheckCallingPermission(const std::string& permission);
    static bool IsStartByHdcd(bool isLocalSysCalling = false);
    static bool IsSystemCallingOrStartByHdcd(bool isLocalSysCalling = false);
    static bool IsStartByInputMethod();
    static bool CheckIsCallingBundleName(const std::string name);
};
} // Rosen
} // OHOS
#endif // WM_PERMISSION_H