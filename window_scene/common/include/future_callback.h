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

#ifndef OHOS_ROSEN_FUTURE_CALLBACK_H
#define OHOS_ROSEN_FUTURE_CALLBACK_H

#include "future.h"
#include "future_callback_stub.h"
#include "interfaces/include/ws_common.h"

namespace OHOS {
namespace Rosen {
class FutureCallback : public FutureCallbackStub {
public:
    WSError OnUpdateSessionRect(const WSRect& rect) override;
    virtual WSRect GetResult(long timeOut) override;
    virtual void ResetLock() override;
private:
    RunnableFuture<WSRect> future_{};
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_FUTURE_CALLBACK_H