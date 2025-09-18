/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 
#include "get_snapshot_callback.h"
#include "hilog_tag_wrapper.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
 
namespace OHOS {
namespace Rosen {
 
void GetSnapshotCallback::RegisterFunc(const NotifyGetMainWindowSnapshotFunc& func)
{
    getMainWindowSnapshotFunc_ = func;
}
 
void GetSnapshotCallback::OnReceived(WMError errCode,
    const std::vector<std::shared_ptr<OHOS::Media::PixelMap>>& pixelMaps)
{
    TLOGI(WmsLogTag::WMS_LIFE, "execute getMainWindowSnapshotFunc_");
    if (getMainWindowSnapshotFunc_) {
        getMainWindowSnapshotFunc_(errCode, pixelMaps);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "OnReceived failed, getMainWindowSnapshotFunc_ is null");
    }
}

void GetSnapshotCallback::OnNotifyResult()
{
    TLOGI(WmsLogTag::WMS_LIFE, "get snapshot result");
    future_.SetValue(true);
}

void GetSnapshotCallback::GetSyncResult(long timeOut)
{
    TLOGI(WmsLogTag::WMS_LIFE, "wait snapshot result");
    future_.GetResult(timeOut);
}
} // namespace Rosen
} // namespace OHOS