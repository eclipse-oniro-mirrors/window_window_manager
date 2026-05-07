/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_WM_TEST_UT_MOCK_DISPLAY_MANAGER_ADAPTER_LITE_H
#define FRAMEWORKS_WM_TEST_UT_MOCK_DISPLAY_MANAGER_ADAPTER_LITE_H
#include <gmock/gmock.h>

#include "display_manager_adapter_lite.h"

namespace OHOS {
namespace Rosen {
class MockDisplayManagerAdapterLite : public DisplayManagerAdapterLite {
public:
    MOCK_METHOD0(Clear, void());
    MOCK_METHOD2(RegisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD2(UnregisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD0(GetDefaultDisplayInfo, sptr<DisplayInfo>(int32_t userId));
    MOCK_METHOD1(GetDisplayInfo, sptr<DisplayInfo>(DisplayId displayId));

    MOCK_METHOD1(WakeUpBegin, bool(PowerStateChangeReason reason));
    MOCK_METHOD0(WakeUpEnd, bool());
    MOCK_METHOD1(SuspendBegin, bool(PowerStateChangeReason reason));
    MOCK_METHOD0(SuspendEnd, bool());
    MOCK_METHOD1(SetDisplayState, bool(DisplayState state));
    MOCK_METHOD1(GetDisplayState, DisplayState(DisplayId displayId));
    MOCK_METHOD2(SetScreenSwitchState, DMError(ScreenClosedState screenClosedState, bool isScreenOn));
};

class MockScreenManagerAdapterLite : public ScreenManagerAdapterLite {
public:
    MOCK_METHOD0(Clear, void());
    MOCK_METHOD2(RegisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD2(UnregisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD3(SetSpecifiedScreenPower, bool(ScreenId screenId, ScreenPowerState state,
        PowerStateChangeReason reason));
    MOCK_METHOD2(SetScreenPowerForAll, bool(ScreenPowerState state, PowerStateChangeReason reason));
    MOCK_METHOD1(GetScreenPower, ScreenPowerState(ScreenId dmsScreenId));
};
}
} // namespace OHOS

#endif // FRAMEWORKS_WM_TEST_UT_MOCK_DISPLAY_MANAGER_ADAPTER_LITE_H