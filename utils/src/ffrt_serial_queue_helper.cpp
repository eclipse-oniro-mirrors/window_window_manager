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

#include "ffrt_serial_queue_helper.h"

#include "ffrt_inner.h"

namespace OHOS::Rosen {

FfrtSerialQueueHelper::FfrtSerialQueueHelper()
{
    ffrtQueue_ = std::make_unique<ffrt::queue>(ffrt::queue_serial, "FfrtSerialQueueHelper",
        ffrt::queue_attr().qos(ffrt_qos_user_interactive));
}

FfrtSerialQueueHelper::~FfrtSerialQueueHelper() = default;

FfrtSerialQueueHelper& FfrtSerialQueueHelper::GetInstance()
{
    static FfrtSerialQueueHelper instance = FfrtSerialQueueHelper();
    return instance;
}

void FfrtSerialQueueHelper::SubmitTask(std::function<void()>&& task)
{
    ffrtQueue_->submit(task);
}
} // namespace OHOS::Rosen
