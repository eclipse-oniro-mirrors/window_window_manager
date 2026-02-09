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

#ifndef MULTI_SCREEN_CHANGE_UTILS_H
#define MULTI_SCREEN_CHANGE_UTILS_H

#include <screen_manager/screen_types.h>
#include "session/screen/include/screen_session.h"
#include "zidl/screen_session_manager_client_interface.h"

namespace OHOS {
namespace Rosen {
class MultiScreenChangeUtils {
public:
    MultiScreenChangeUtils() = delete;

    static void ScreenDensityChangeNotify(sptr<ScreenSession> innerScreen,
        sptr<ScreenSession> externalScreen);

    static void ScreenPropertyChangeNotify(sptr<ScreenSession>& innerScreen,
        sptr<ScreenSession>& externalScreen);

    static void ScreenExtendPositionChange(sptr<ScreenSession>& innerScreen,
        sptr<ScreenSession>& externalScreen);

    static void SetScreenAvailableStatus(sptr<ScreenSession>& screenSession,
        bool isScreenAvailable);

    static void SetExternalScreenOffScreenRendering(sptr<ScreenSession>& innerScreen,
        sptr<ScreenSession>& externalScreen);

    static void ScreenCombinationChange(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen,
        ScreenCombination externalCombination);

    static void ScreenMainPositionChange(sptr<ScreenSession>& innerScreen,
        sptr<ScreenSession>& externalScreen);

    static void ScreenPhysicalInfoChange(sptr<ScreenSession>& innerScreen,
        sptr<ScreenSession>& externalScreen);

    static void CreateMirrorSession(sptr<ScreenSession>& mainSession,
        sptr<ScreenSession>& screenSession);

    static void ScreenConnectionChange(sptr<IScreenSessionManagerClient> ssmClient,
        sptr<ScreenSession>& mainSession, ScreenEvent screenEvent);

    static void CreateExtendSession(sptr<ScreenSession>& screenSession);

    static void SetMultiScreenModeChangeTracker(std::string changeProc);

private:
    static void ScreenNameChange(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
    static void ScreenRSIdChange(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
    static void ScreenPropertyChange(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
    static void ScreenSerialNumberChange(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
    static void ScreenActiveModesChange(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
    static void SetScreenNotifyFlag(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
    static void ExchangeScreenSupportedRefreshRate(
        sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen);
};
} // Rosen
} // OHOS
#endif // MULTI_SCREEN_CHANGE_UTILS_H