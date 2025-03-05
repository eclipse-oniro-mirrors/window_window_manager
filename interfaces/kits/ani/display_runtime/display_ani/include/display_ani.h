/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef DISPLAY_ANI_H
#define DISPLAY_ANI_H

#include "ani.h"
#include "display.h"
#include "display_ani_register_manager.h"

namespace OHOS {
namespace Rosen {

class DisplayAni {
public:
    explicit DisplayAni(const std::shared_ptr<OHOS::Rosen::Display>& display);

    static ani_object getCutoutInfo(ani_env* env, ani_object obj);
private:
    std::mutex mtx_;
    std::unique_ptr<OHOS::Rosen::DisplayAniRegisterManager> registerManager_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif