/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_EXTENSION_WINDOW_IMPL_H
#define OHOS_ROSEN_EXTENSION_WINDOW_IMPL_H

#include <atomic>
#include <optional>
#include <shared_mutex>
#include "extension_window.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
class ExtensionWindowImpl : public ExtensionWindow {
public:
    ExtensionWindowImpl(sptr<Window> sessionImpl);
    ~ExtensionWindowImpl();
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea) override;
    sptr<Window> GetWindow() override;
    WMError HideNonSecureWindows(bool shouldHide) override;
    WMError SetWaterMarkFlag(bool isEnable) override;
    WMError HidePrivacyContentForHost(bool needHide) override;
    
private:
    sptr<Window> windowExtensionSessionImpl_;
};
} // namespace Rosen
} // namespace OHOS

#endif //OHOS_ROSEN_EXTENSION_WINDOW_IMPL_H
