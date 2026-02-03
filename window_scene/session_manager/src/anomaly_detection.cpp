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

#include "anomaly_detection.h"
#include <hitrace_meter.h>

#include "session_manager/include/scene_session_manager.h"
#include "window_helper.h"
#include "perform_reporter.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "AnomalyDetection" };
}

void AnomalyDetection::SceneZOrderCheckProcess()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::SceneZOrderCheckProcess");
    TLOGE(WmsLogTag::WMS_HIERARCHY, "ZOrderCheck start");
    bool keyGuardFlag = false;
    uint32_t curZOrder = 0;
    auto func = [&curZOrder, &keyGuardFlag](sptr<SceneSession> session) {
        if ((session == nullptr) || (!SceneSessionManager::GetInstance().IsSessionVisibleForeground(session))) {
            return false;
        }
        // check zorder = 0
        if (session->GetZOrder() == 0) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "ZOrderCheck err, zorder 0");
            ReportZOrderException("check zorder 0", session);
        }
        // repetitive zorder
        if (session->GetZOrder() == curZOrder) {
            TLOGND(WmsLogTag::WMS_HIERARCHY, "ZOrderCheck err, zorder %{public}d", session->GetZOrder());
            ReportZOrderException("check repetitive zorder", session);
        }
        curZOrder = session->GetZOrder();
        // callingSession check for input method
        CheckCallingSession(session);
        // subWindow/dialogWindow
        CheckSubWindow(session);
        // check app session showWhenLocked
        CheckShowWhenLocked(session, keyGuardFlag);
        // wallpaper zOrder check
        CheckWallpaper(session);
        return false;
    };
    SceneSessionManager::GetInstance().TraverseSessionTree(func, false);
}

void AnomalyDetection::CheckCallingSession(const sptr<SceneSession>& session)
{
    if (session->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        uint32_t callingWindowId = session->GetCallingSessionId();
        const auto& callingSession =
            SceneSessionManager::GetInstance().GetSceneSession(static_cast<int32_t>(callingWindowId));
        if ((callingSession != nullptr) && (callingSession->GetZOrder() > session->GetZOrder())) {
            TLOGE(WmsLogTag::WMS_HIERARCHY,
                  "ZOrderCheck err, callingSession: %{public}d curSession: %{public}d",
                  callingSession->GetZOrder(),
                  session->GetZOrder());
            ReportZOrderException("check callingSession check for input", session);
        }
    }
}

void AnomalyDetection::CheckSubWindow(const sptr<SceneSession>& session)
{
    if (WindowHelper::IsSubWindow(session->GetWindowType()) ||
        session->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
        auto mainSession = session->GetParentSession();
        if ((mainSession != nullptr) && (session->GetZOrder() < mainSession->GetZOrder())) {
            TLOGE(WmsLogTag::WMS_HIERARCHY,
                  "ZOrderCheck err, subSession %{public}d mainSession %{public}d",
                  session->GetZOrder(),
                  mainSession->GetZOrder());
            ReportZOrderException("check subWindow and dialogWindow", session);
        }
    }
}

void AnomalyDetection::CheckShowWhenLocked(const sptr<SceneSession>& session, bool& keyGuardFlag)
{
    if (session->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
        keyGuardFlag = true;
        return;
    }
    if (keyGuardFlag && (!session->IsShowWhenLocked()) && (session->IsAppSession())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "ZOrderCheck err %{public}d IsShowWhenLocked", session->GetZOrder());
        ReportZOrderException("check isShowWhenLocked", session);
    }
}

void AnomalyDetection::CheckWallpaper(const sptr<SceneSession>& session)
{
    if (!(session->GetWindowType() == WindowType::WINDOW_TYPE_WALLPAPER)) {
        return;
    }
    constexpr uint32_t defaultWallpaperZOrder = 1;
    if (!SceneSessionManager::GetInstance().IsScreenLocked() && session->GetZOrder() != defaultWallpaperZOrder) {
        TLOGD(WmsLogTag::WMS_HIERARCHY,
              "ZOrderCheck err %{public}d persitentId:%{public}d, wallpaper zOrder abnormal pre",
              session->GetZOrder(), session->GetPersistentId());
        auto task = [session, defaultWallpaperZOrder] {
            if (session == nullptr) {
                return;
            }
            if (session->IsVisibleForeground() && session->GetZOrder() != defaultWallpaperZOrder) {
                TLOGE(WmsLogTag::WMS_HIERARCHY,
                      "ZOrderCheck err %{public}d, persitentId:%{public}d, wallpaper zOrder abnormal",
                      session->GetZOrder(), session->GetPersistentId());
                ReportZOrderException("check wallpaerWhenLocked", session);
            }
        };
        auto handler = SceneSessionManager::GetInstance().GetTaskScheduler();
        const int64_t delayTime = 300;
        if (handler) {
            handler->PostTask(task, "SceneZorderCheckProcessCheckWallpaper", delayTime);
        }
    }
}

void AnomalyDetection::ReportZOrderException(const std::string& errorReason, sptr<SceneSession> session)
{
    if (session == nullptr) {
        return;
    }
    std::ostringstream oss;
    oss << " ZOrderCheck err " << errorReason;
    oss << " cur persistentId: " << session->GetPersistentId() << ",";
    oss << " windowType: " << static_cast<uint32_t>(session->GetWindowType()) << ",";
    oss << " cur ZOrder: " << session->GetZOrder() << ";";
    WindowInfoReporter::GetInstance().ReportWindowException(
        static_cast<int32_t>(WindowDFXHelperType::WINDOW_ZORDER_CHECK), getpid(), oss.str());
}

} // namespace Rosen
} // namespace OHOS