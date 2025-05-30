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

void AnomalyDetection::CheckCallingSession(sptr<SceneSession>& session)
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

void AnomalyDetection::CheckSubWindow(sptr<SceneSession>& session)
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

void AnomalyDetection::CheckShowWhenLocked(sptr<SceneSession>& session, bool& keyGuardFlag)
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

void AnomalyDetection::CheckWallpaper(sptr<SceneSession>& session)
{
    if (session->GetWindowType() == WindowType::WINDOW_TYPE_WALLPAPER) {
        constexpr uint32_t defaultWallpaperZOrder = 1;
        if (!SceneSessionManager::GetInstance().IsScreenLocked() && session->GetZOrder() != defaultWallpaperZOrder) {
            TLOGE(
                WmsLogTag::WMS_HIERARCHY, "ZOrderCheck err %{public}d wallpaper zOrder abnormal", session->GetZOrder());
            ReportZOrderException("check wallpaperWhenLocked", session);
        }
    }
}

void AnomalyDetection::FocusCheckProcess(int32_t focusedId, int32_t nextId)
{
    if (nextId == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_FOCUS, "FocusCheck err: invalid id, focusedId:%{public}d nextId:%{public}d",
            focusedId, nextId);
        ReportFocusException("invalid id", focusedId, nextId, nullptr);
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

void AnomalyDetection::ReportFocusException(const std::string& errorReason, int32_t focusedId, int32_t nextId,
    sptr<SceneSession> session)
{
    std::ostringstream oss;
    oss << " FocusCheck err " << errorReason;
    if (session != nullptr) {
        oss << " cur persistentId: " << session->GetPersistentId() << ",";
        oss << " windowType: " << static_cast<uint32_t>(session->GetWindowType()) << ",";
    }
    oss << " focusedId: " << focusedId << ",";
    oss << " nextId: " << nextId << ";";
    WindowInfoReporter::GetInstance().ReportWindowException(
        static_cast<int32_t>(WindowDFXHelperType::WINDOW_FOCUS_CHECK), getpid(), oss.str());
}
} // namespace Rosen
} // namespace OHOS