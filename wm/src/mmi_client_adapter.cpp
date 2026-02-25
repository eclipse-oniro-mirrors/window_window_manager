/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "mmi_client_adapter.h"
#include "pointer_event.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
bool IsHoverDown(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    return pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER &&
        pointerEvent->GetSourceType() ==  MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
}

bool PointerEventWriteToParcel(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, Parcel& data)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    return pointerEvent->WriteToParcel(data);
}
} // namespace Rosen
} // namespace OHOS