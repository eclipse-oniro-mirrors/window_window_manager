/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCENE_SYSTEM_ABILITY_LISTENER_H
#define OHOS_ROSEN_SCENE_SYSTEM_ABILITY_LISTENER_H
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Rosen {
struct SCBThreadInfo {
    std::string scbUid_;
    std::string scbPid_;
    std::string scbTid_;
    std::string ssmTid_;
    std::string scbBundleName_;
    std::string ssmThreadName_;
};

class SceneSystemAbilityListener : public SystemAbilityStatusChangeStub {
public:
    SceneSystemAbilityListener(const SCBThreadInfo& info) : info_(info) {}
    virtual ~SceneSystemAbilityListener() = default;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    SCBThreadInfo info_;
};
} // namespace Rosen
} // namespace OHOS


#endif

