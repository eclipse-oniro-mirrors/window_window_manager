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

#ifndef OHOS_ROSEN_WINDOW_ANOMALY_DETECTION_H
#define OHOS_ROSEN_WINDOW_ANOMALY_DETECTION_H

#include "session/host/include/scene_session.h"

namespace OHOS {
namespace Rosen {
class AnomalyDetection {
public:
    static void SceneZOrderCheckProcess();

private:
    static void ReportZOrderException(const std::string& errorReason, sptr<SceneSession> session);
    static void CheckCallingSession(const sptr<SceneSession>& session);
    static void CheckSubWindow(const sptr<SceneSession>& session);
    static void CheckShowWhenLocked(const sptr<SceneSession>& session, bool& keyGuardFlag);
    static void CheckWallpaper(const sptr<SceneSession>& session);
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_ANOMALY_DETECTION_H