/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
const char* g_domainContents[static_cast<uint32_t>(WmsLogTag::END)] = {
    "WMS",
    "DMS",
    "WMSMain",
    "WMSSub",
    "WMSScb",
    "WMSDialog",
    "WMSSystem",
    "WMSFocus",
    "WMSLayout",
    "WMSImms",
    "WMSLife",
    "WMSKeyboard",
    "WMSEvent",
    "WMSUiext",
    "WMSPiP",
    "WMSRecover",
    "WMSMultiUser",
    "WMSToast",
    "WMSMultiWindow",
    "InputKeyFlow",
    "WMSPipeLine",
    "WMSHierarchy",
    "WMSPattern",
    "WMSAttribute",
    "WMSPc",
    "WMSDecor",
    "WMSLayoutPc",
    "WMSStartupPage",
    "WMSCompat",
    "WMSRotation",
    "WMSAnimation",
};

const char* getDomainContents(WmsLogTag tag)
{
    return DOMAIN_CONTENTS_MAP.count(tag) ? DOMAIN_CONTENTS_MAP.at(tag) : "";
}
} // namespace OHOS
}