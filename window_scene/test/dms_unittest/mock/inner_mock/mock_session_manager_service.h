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

#ifndef OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H
#define OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H

#include "dm_common.h"

namespace OHOS {
namespace Rosen {
class MockSessionManagerService {
public:
    static MockSessionManagerService& GetInstance()
    {
        static MockSessionManagerService instance_;
        return instance_;
    }
    void AddSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds, int32_t userId) {}
    void RemoveSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds, int32_t userId) {}
    void NotifyWMSConnected(int32_t userId, DisplayId screenId, bool isColdStart) {}
    void GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<uint64_t>& windowIdList, std::vector<uint64_t>& surfaceNodeIds) {}
    void SetScreenPrivacyWindowTagSwitch(
        uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable) {}

private:
    MockSessionManagerService() = default;
    ~MockSessionManagerService() = default;
};
}
}

#endif  // OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H