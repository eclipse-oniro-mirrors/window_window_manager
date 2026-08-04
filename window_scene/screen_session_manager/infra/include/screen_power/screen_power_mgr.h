
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
 
#ifndef OHOS_ROSEN_DMS_SCREEN_POWER_MGR_H
#define OHOS_ROSEN_DMS_SCREEN_POWER_MGR_H
 
#include <map>
#include <mutex>
 
#include "dm_common.h"
#include "nocopyable.h"
 
#include <screen_manager/screen_types.h>
 
namespace OHOS {
namespace Rosen {
namespace DMS {
 
class ScreenPowerMgr {
    DISALLOW_COPY_AND_MOVE(ScreenPowerMgr);
 
public:
    static ScreenPowerMgr& GetInstance();
    void ClearScreenPowerStatus(ScreenId rsScreenId);
    bool IsScreenPowerOn(ScreenId rsScreenId);
 
    virtual void DoSetScreenPowerStatus(ScreenId rsScreenId, ScreenPowerStatus status);
 
protected:
    ScreenPowerMgr();
    void SetTpFeatureConfig(int32_t tpType, const std::string& tpConfig);
    std::mutex screenPowerStatusMapMutex_;
    std::map<ScreenId, ScreenPowerStatus> screenPowerStatusMap_;
};
 
}  // namespace DMS
}  // namespace Rosen
}  // namespace OHOS
 
#endif