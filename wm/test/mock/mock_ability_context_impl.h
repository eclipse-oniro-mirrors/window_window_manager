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

#ifndef MOCK_ABILITY_CONTEXT_IMPL_H
#define MOCK_ABILITY_CONTEXT_IMPL_H

#include "ability_context_impl.h"

namespace OHOS {
namespace Rosen {
class MockAbilityContextImpl : public AbilityRuntime::AbilityContextImpl {
public:
    bool IsContext(size_t contextTypeId) override { return false; }

    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();

    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override { return hapModuleInfo_; }

    AppExecFwk::BrandColorStrategy GetBrandColorStrategy() override { return brandColorStrategy_; }

    AppExecFwk::BrandColorStrategy brandColorStrategy_ = AppExecFwk::BrandColorStrategy::BRAND_COLOR_FOLLOW_SYSTEM;
};
} // OHOS
} // Rosen

#endif
