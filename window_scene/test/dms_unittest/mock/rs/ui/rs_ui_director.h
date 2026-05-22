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

#ifndef RENDER_SERVICE_CLIENT_CORE_UI_RS_UI_DIRECTOR_H
#define RENDER_SERVICE_CLIENT_CORE_UI_RS_UI_DIRECTOR_H

#include <atomic>
#include <functional>
#include <mutex>
#include <iremote_object.h>

namespace OHOS {
namespace Rosen {

class RSUIDirector final {
public:
    static std::shared_ptr<RSUIDirector> Create(sptr<IRemoteObject> connectToRenderRemote = nullptr,
        std::shared_ptr<RSUIContext> rsUIContext = nullptr)
    {
        return std::make_shared<RSUIDirector>();
    }
    RSUIDirector()
    {
        rsUIContext_ = nullptr;
    }
    ~RSUIDirector() = default;
    std::shared_ptr<RSUIContext> GetRSUIContext() const
    {
        return rsUIContext_;
    }
    void Init(bool shouldCreateRenderThread, bool isMultiInstance, std::shared_ptr<RSUIContext> rsUIContext)
    {
        rsUIContext_ = rsUIContext;
    }

    std::shared_ptr<RSUIContext> rsUIContext_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS

/** @} */
#endif  // RENDER_SERVICE_CLIENT_CORE_UI_RS_UI_DIRECTOR_H
