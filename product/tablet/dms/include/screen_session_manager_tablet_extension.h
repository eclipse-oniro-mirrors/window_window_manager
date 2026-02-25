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

#ifndef SCREEN_SESSION_MANAGER_TABLET_EXTENSION_H
#define SCREEN_SESSION_MANAGER_TABLET_EXTENSION_H

#include "screen_property.h"
#include "screen_session.h"
#include "screen_session_manager.h"
#include "window_manager_hilog.h"
#include <mutex>
#include <thread>
#include "screen_session_manager_pc_extension.h"
#include "screen_session_manager_phone_extension.h"

namespace OHOS {
namespace Rosen {
namespace TabletExtension {
class ScreenSessionManagerExt : public OHOS::Rosen::PCExtension::ScreenSessionManagerExt,
                                public OHOS::Rosen::PhoneExtension::ScreenSessionManagerExt {
    WM_DECLARE_SINGLE_INSTANCE(ScreenSessionManagerExt)
public:
    sptr<ScreenSession> GetPhysicalScreenSession(ScreenId screenId, ScreenId defScreenId, ScreenProperty property);
    void NotifyCaptureStatusChangedGlobal();
    void GetAndMergeEdidInfo(sptr<ScreenSession> screenSession);
    void OnScreenChangeDefault(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason);
    void ScreenConnectionChanged(sptr<ScreenSession>& screenSession, ScreenId screenId,
        ScreenEvent screenEvent, bool phyMirrorEnable);
    bool IsNeedAddInputServiceAbility();
};
}
} // namespace Rosen
} // namespace OHOS
#endif