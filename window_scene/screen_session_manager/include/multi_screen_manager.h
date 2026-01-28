/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ROSEN_MULTI_SCREEN_MANAGER_H
#define OHOS_ROSEN_MULTI_SCREEN_MANAGER_H

#include <hitrace_meter.h>
#include <transaction/rs_interfaces.h>

#include "dm_common.h"
#include "screen_session_manager.h"
#include "wm_single_instance.h"

namespace OHOS::Rosen {

class MultiScreenManager {
WM_DECLARE_SINGLE_INSTANCE_BASE(MultiScreenManager);
public:
    DMError VirtualScreenUniqueSwitch(sptr<ScreenSession> screenSession, const std::vector<ScreenId>& screenIds,
        const UniqueScreenRotationOptions& rotationOptions);

    DMError UniqueSwitch(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds,
        const UniqueScreenRotationOptions& rotationOptions);

    DMError MirrorSwitch(const ScreenId mainScreenId, const std::vector<ScreenId>& screenIds, DMRect mainScreenRegion,
        ScreenId& screenGroupId, const RotationOption& rotationOption, bool forceMirror = false);

    void MultiScreenModeChange(sptr<ScreenSession> mainSession, sptr<ScreenSession> secondarySession,
        const std::string& operateType);

    void MultiScreenModeChange(ScreenId mainScreenId, ScreenId secondaryScreenId, const std::string& operateType);

    void SetLastScreenMode(ScreenId mainScreenId, MultiScreenMode secondaryScreenMode);

    void InternalScreenOnChange(sptr<ScreenSession> internalSession, sptr<ScreenSession> externalSession);

    void InternalScreenOffChange(sptr<ScreenSession> internalSession, sptr<ScreenSession> externalSession);

    void ExternalScreenDisconnectChange(sptr<ScreenSession> internalSession, sptr<ScreenSession> externalSession);

    bool AreScreensTouching(sptr<ScreenSession> mainScreenSession, sptr<ScreenSession> secondScreenSession,
        MultiScreenPositionOptions mainScreenOptions, MultiScreenPositionOptions secondScreenOption);

    void MultiScreenReportDataToRss(std::string multiScreenType, std::string status);
    void NotifyScreenConnectCompletion(ScreenId screenId);
private:
    MultiScreenManager();
    ~MultiScreenManager();

    void FilterPhysicalAndVirtualScreen(const std::vector<ScreenId>& allScreenIds,
        std::vector<ScreenId>& physicalScreenIds, std::vector<ScreenId>& virtualScreenIds);

    DMError VirtualScreenMirrorSwitch(const ScreenId mainScreenId, const std::vector<ScreenId>& screenIds,
        DMRect mainScreenRegion, ScreenId& screenGroupId, const RotationOption& rotationOption,
        bool forceMirror = false);

    DMError PhysicalScreenMirrorSwitch(const ScreenId mainScreenId, const std::vector<ScreenId>& screenIds,
        DMRect mainScreenRegion, const RotationOption& rotationOption, bool forceMirror = false);

    DMError PhysicalScreenUniqueSwitch(const std::vector<ScreenId>& screenIds);

    /* pc multi screen switch */
    void DoFirstExtendChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
        const std::string& operateType);

    void DoFirstMainChangeExtend(sptr<IScreenSessionManagerClient> scbClient, sptr<ScreenSession> firstSession,
        sptr<ScreenSession> secondarySession);

    void DoFirstMainChangeMirror(sptr<IScreenSessionManagerClient> scbClient, sptr<ScreenSession> firstSession,
        sptr<ScreenSession> secondarySession);

    void DoFirstMirrorChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
        const std::string& operateType);
    void DoFirstMirrorChangeExtend(sptr<IScreenSessionManagerClient> scbClient, sptr<ScreenSession> firstSession,
        sptr<ScreenSession> secondarySession);
    void DoFirstMirrorChangeMirror(sptr<IScreenSessionManagerClient> scbClient, sptr<ScreenSession> firstSession,
        sptr<ScreenSession> secondarySession);

    void DoFirstMainChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
        const std::string& operateType);
    void DoFirstExtendChangeExtend(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession);
    void DoFirstExtendChangeMirror(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession);

    std::pair<ScreenId, MultiScreenMode> lastScreenMode_;  // main screen id & secondary screen mode

    void BlockScreenConnect(sptr<ScreenSession>& screenSession, ScreenId screenId);
    std::mutex uniqueScreenMutex_;
    std::condition_variable uniqueScreenCV_;
    std::map<ScreenId, bool> uniqueScreenTimeoutMap_;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_MULTI_SCREEN_MANAGER_H
