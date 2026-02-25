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

#ifndef OHOS_ROSEN_WINDOW_SCENE_STARTING_WINDOW_RDB_MANAGER_H
#define OHOS_ROSEN_WINDOW_SCENE_STARTING_WINDOW_RDB_MANAGER_H

#include <atomic>
#include <event_handler.h>
#include <memory>
#include <mutex>
#include <utility>

#include "rdb_helper.h"

#include "interfaces/include/ws_common.h"
#include "wms_rdb_open_callback.h"

namespace OHOS::AppExecFwk {
class IBundleMgr;
struct AbilityInfo;
struct BundleInfo;
class LauncherService;
} // namespace OHOS::AppExecFwk

namespace OHOS::Global::Resource {
class ResourceManager;
} // namespace OHOS::Global::Resource

namespace OHOS {
namespace Rosen {
struct StartingWindowRdbItemKey {
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    bool darkMode;
};

class StartingWindowRdbManager final : public std::enable_shared_from_this<StartingWindowRdbManager> {
public:
    StartingWindowRdbManager(const WmsRdbConfig& rdbConfig,
        std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr);
    ~StartingWindowRdbManager();

    bool Init();
    bool InsertData(const StartingWindowRdbItemKey& key, const StartingWindowInfo& value);
    bool BatchInsert(int64_t& outInsertNum,
        const std::vector<std::pair<StartingWindowRdbItemKey, StartingWindowInfo>>& inputValues);
    bool DeleteDataByBundleName(const std::string& bundleName);
    bool DeleteAllData();
    bool QueryData(const StartingWindowRdbItemKey& key, StartingWindowInfo& value);
    std::string GetStartWindowValFromProfile(const AppExecFwk::AbilityInfo& abilityInfo,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr,
        const std::string& key, const std::string& defaultVal);

private:
    std::shared_ptr<NativeRdb::RdbStore> GetRdbStore();
    void DelayClearRdbStore(int32_t delay = 0);
    
    std::mutex rdbMutex_;
    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    WmsRdbConfig wmsRdbConfig_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_SCENE_STARTING_WINDOW_RDB_MANAGER_H