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
#ifndef OHOS_ROSEN_MOCK_VSYNC_STATION_H
#define OHOS_ROSEN_MOCK_VSYNC_STATION_H

#include "vsync_station.h"

#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class MockVsyncStation : public VsyncStation {
public:
    explicit MockVsyncStation(NodeId nodeId = 0,
                              const std::shared_ptr<AppExecFwk::EventHandler>& vsyncHandler = nullptr)
        : VsyncStation(nodeId, vsyncHandler) {}

    MOCK_METHOD(void, RequestVsync, (const std::shared_ptr<VsyncCallback>& vsyncCallback), (override));
    MOCK_METHOD(int64_t, GetVSyncPeriod, (), (override));
    MOCK_METHOD(std::optional<uint32_t>, GetFps, (), (override));
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_MOCK_VSYNC_STATION_H
