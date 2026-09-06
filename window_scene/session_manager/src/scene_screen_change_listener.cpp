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

#include "scene_screen_change_listener.h"

#include "scene_session_manager.h"
#include "window_coordinate_helper.h"

namespace OHOS {
namespace Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(SceneScreenChangeListener);

void SceneScreenChangeListener::OnPropertyChange(
    const ScreenProperty& newProperty, ScreenPropertyChangeReason reason, ScreenId screenId)
{
    if (reason == ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE) {
        HandleRelativePositionChange(newProperty, screenId);
    }
}

void SceneScreenChangeListener::HandleRelativePositionChange(const ScreenProperty& newProperty, ScreenId screenId)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "screenId: %{public}" PRIu64 ", x: %{public}d, y: %{public}d",
        screenId, newProperty.GetX(), newProperty.GetY());
    // In global coordinates, when the screen position changes,
    // the rect information of the contained windows needs to be updated.
    auto sceneSessions = SceneSessionManager::GetInstance().GetSceneSessions(screenId);
    for (const auto& session : sceneSessions) {
        if (!session) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "session is nullptr");
            continue;
        }
        auto globalDisplayRect =
            WindowCoordinateHelper::ConvertToGlobalDisplayRect(screenId, session->GetSessionRect());
        session->UpdateGlobalDisplayRect(globalDisplayRect, SizeChangeReason::SCREEN_RELATIVE_POSITION_CHANGE);
    }
}
} // namespace Rosen
} // namespace OHOS
