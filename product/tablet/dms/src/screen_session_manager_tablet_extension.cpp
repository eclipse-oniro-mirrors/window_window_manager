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

#include "../include/screen_session_manager_tablet_extension.h"
#include <cstdint>
#include <iomanip>
#include "input_manager.h"
#include "screen_session.h"
#include "screen_session_manager.h"
#include "screen_session_manager_pc_extension.h"
#include "screen_session_manager_phone_extension.h"
#include "window_manager_hilog.h"

#ifdef DEVICE_STATUS_ENABLE
#include <interaction_manager.h>
#endif // DEVICE_STATUS_ENABLE

using namespace OHOS::Rosen;
using OHOS::sptr;
using namespace TabletExtension;

#define COMBINE_FUNC(func)                                                                          \
    (ScreenSessionManager::GetPcStatus()) ? OHOS::Rosen::PCExtension::ScreenSessionManagerExt::func \
                                          : OHOS::Rosen::PhoneExtension::ScreenSessionManagerExt::func

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenSessionManagerExt)

sptr<ScreenSession> ScreenSessionManagerExt::GetPhysicalScreenSession(ScreenId screenId,
    ScreenId defScreenId, ScreenProperty property)
{
    return COMBINE_FUNC(GetPhysicalScreenSession(screenId, defScreenId, property));
}

void ScreenSessionManagerExt::NotifyCaptureStatusChangedGlobal()
{
    COMBINE_FUNC(NotifyCaptureStatusChangedGlobal());
}

void ScreenSessionManagerExt::GetAndMergeEdidInfo(sptr<ScreenSession> screenSession)
{
    COMBINE_FUNC(GetAndMergeEdidInfo(screenSession));
}

void ScreenSessionManagerExt::OnScreenChangeDefault(ScreenId screenId,
    ScreenEvent screenEvent, ScreenChangeReason reason)
{
    COMBINE_FUNC(OnScreenChangeDefault(screenId, screenEvent, reason));
}

void ScreenSessionManagerExt::ScreenConnectionChanged(sptr<ScreenSession>& screenSession,
    ScreenId screenId, ScreenEvent screenEvent, bool phyMirrorEnable)
{
    COMBINE_FUNC(ScreenConnectionChanged(screenSession, screenId, screenEvent, phyMirrorEnable));
}

bool ScreenSessionManagerExt::IsNeedAddInputServiceAbility()
{
    return COMBINE_FUNC(IsNeedAddInputServiceAbility());
}