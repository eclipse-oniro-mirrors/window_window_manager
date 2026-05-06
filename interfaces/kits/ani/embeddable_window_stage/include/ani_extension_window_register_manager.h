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

#ifndef OHOS_ANI_EXTENSION_WINDOW_REGISTER_MANAGER_H
#define OHOS_ANI_EXTENSION_WINDOW_REGISTER_MANAGER_H

#include <mutex>

#include "ani.h"
#include "ani_extension_window_listener.h"
#include "refbase.h"
#include "window.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

class AniExtensionWindowRegisterManager {
public:
    AniExtensionWindowRegisterManager();
    ~AniExtensionWindowRegisterManager();
    WmErrorCode RegisterListener(sptr<Window>& window, const std::string& type,
        CaseType caseType, ani_env* env, ani_object fn);
    WmErrorCode UnregisterListener(sptr<Window>& window, const std::string& type,
        CaseType caseType, ani_env* env, ani_object fn);

private:
    enum class ListenerType : uint32_t {
        WINDOW_SIZE_CHANGE_CB,
        AVOID_AREA_CHANGE_CB,
        WINDOW_EVENT_CB,
        WINDOW_STAGE_EVENT_CB,
        WINDOW_RECT_CHANGE_CB,
    };

    bool IsCallbackRegistered(ani_env* env, const std::string& type, ani_object fn);
    WmErrorCode ProcessWindowChangeRegister(sptr<AniExtensionWindowListener>& listener,
        sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessAvoidAreaChangeRegister(sptr<AniExtensionWindowListener>& listener,
        sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessLifeCycleEventRegister(sptr<AniExtensionWindowListener>& listener,
        sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessWindowRectChangeRegister(sptr<AniExtensionWindowListener>& listener,
        sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessRegister(CaseType caseType, sptr<AniExtensionWindowListener>& listener,
        sptr<Window>& window, const std::string& type, bool isRegister);
    WmErrorCode UnregisterAllListeners(CaseType caseType, sptr<Window>& window,
        const std::string& type, ani_env* env);
    WmErrorCode UnregisterSpecificListener(CaseType caseType, sptr<Window>& window,
        const std::string& type, ani_env* env, ani_object fn);
    std::map<std::string, std::map<ani_ref, sptr<AniExtensionWindowListener>>> aniCbMap_;
    std::mutex mtx_;
    std::map<CaseType, std::map<std::string, ListenerType>> listenerCodeMap_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_ANI_EXTENSION_WINDOW_REGISTER_MANAGER_H
