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

#ifndef FRAMEWORKS_WM_TEST_UT_MOCK_WINDOW_ADAPTER_LITE_H
#define FRAMEWORKS_WM_TEST_UT_MOCK_WINDOW_ADAPTER_LITE_H
#include <gmock/gmock.h>

#include "window_adapter_lite.h"

namespace OHOS {
namespace Rosen {
class MockWindowAdapterLite : public WindowAdapterLite {
public:
    MOCK_METHOD(void, GetFocusWindowInfo, (FocusChangeInfo& focusInfo, DisplayId displayId));
    MOCK_METHOD(void, ClearWindowAdapter, ());
    MOCK_METHOD2(RegisterWindowManagerAgent, WMError(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent));
    MOCK_METHOD2(UnregisterWindowManagerAgent, WMError(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent));
    MOCK_METHOD1(GetWindowModeType, WMError(WindowModeType& windowModeType));
    MOCK_METHOD2(GetMainWindowInfos, WMError(int32_t topNum,
        std::vector<MainWindowInfo>& topNInfo));
    MOCK_METHOD1(GetAllMainWindowInfos, WMError(std::vector<MainWindowInfo>& infos));
    MOCK_METHOD1(ClearMainSessions, WMError(const std::vector<int32_t>& persistentIds));
    MOCK_METHOD2(ClearMainSessions, WMError(const std::vector<int32_t>& persistentIds,
        std::vector<int32_t>& clearFailedIds));
    MOCK_METHOD1(TerminateSessionByPersistentId, WMError(int32_t persistentId));
    MOCK_METHOD(WMError, CloseTargetFloatWindow, (const std::string& bundleName));
    MOCK_METHOD(WMError, CloseTargetPiPWindow, (const std::string& bundleName));
    MOCK_METHOD1(GetCurrentPiPWindowInfo, WMError(std::string& bundleName));
    MOCK_METHOD1(GetAccessibilityWindowInfo, WMError(std::vector<sptr<AccessibilityWindowInfo>>& infos));
    MOCK_METHOD2(GetDisplayIdByWindowId, WMError(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap));
};
}
} // namespace OHOS

#endif // FRAMEWORKS_WM_TEST_UT_MOCK_WINDOW_ADAPTER_LITE_H