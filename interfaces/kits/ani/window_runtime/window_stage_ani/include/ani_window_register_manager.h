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

#ifndef OHOS_ANI_WINDOW_REGISTER_MANAGER_H
#define OHOS_ANI_WINDOW_REGISTER_MANAGER_H

#include <map>
#include <mutex>

#include "ani_window_listener.h"
#include "refbase.h"
#include "window.h"

namespace OHOS {
namespace Rosen {

class AniWindowRegisterManager {
public:
    AniWindowRegisterManager();
    ~AniWindowRegisterManager();
    WmErrorCode RegisterListener(sptr<Window> window, const std::string& type, CaseType caseType,
        ani_env* env, ani_ref callback, ani_long timeout);
    WmErrorCode UnregisterListener(sptr<Window> window, const std::string& type, CaseType caseType,
        ani_env* env, ani_ref callback);
private:
    bool IsCallbackRegistered(ani_env* env, std::string type, ani_ref jsListenerObject);
    WmErrorCode ProcessWindowChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessSystemAvoidAreaChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessAvoidAreaChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessLifeCycleEventRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessWindowStageLifeCycleEventRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessOccupiedAreaChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessKeyboardDidShowRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessKeyboardDidHideRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessKeyboardWillShowRegister(sptr<AniWindowListener> listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessKeyboardWillHideRegister(sptr<AniWindowListener> listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessSystemBarChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
    ani_env* env);
    WmErrorCode ProcessTouchOutsideRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessDialogTargetTouchRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessWindowNoInteractionRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env, ani_long timeout);
    WmErrorCode ProcessScreenshotRegister(sptr<AniWindowListener> listener, sptr<Window> window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessDialogDeathRecipientRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessGestureNavigationEnabledChangeRegister(sptr<AniWindowListener> listener,
        sptr<Window> window, bool isRegister, ani_env* env);
    WmErrorCode ProcessWaterMarkFlagChangeRegister(sptr<AniWindowListener> listener,
        sptr<Window> window, bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowVisibilityChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessOcclusionStateChangeRegister(const sptr<AniWindowListener>& listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessFrameMetricsMeasuredChangeRegister(const sptr<AniWindowListener>& listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessSystemDensityChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessDisplayIdChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowStatusChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowTitleButtonRectChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowWillCloseRegister(const sptr<AniWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowRectChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessSubWindowCloseRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowHighlightChangeRegister(sptr<AniWindowListener> listener, sptr<Window> window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessMainWindowCloseRegister(const sptr<AniWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessExtensionSecureLimitChangeRegister(const sptr<AniWindowListener>& listener,
        const sptr<Window>& window, bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowStageListener(RegisterListenerType registerListenerType,
        const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessWindowListener(RegisterListenerType registerListenerType,
        const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister,
        ani_env* env, ani_long timeout);
    WmErrorCode ProcessWindowManagerListener(RegisterListenerType registerListenerType,
        const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister,
        ani_env* env);
    WmErrorCode ProcessWindowRotationChangeRegister(const sptr<AniWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessRectChangeInGlobalDisplayRegister(const sptr<AniWindowListener>& listener,
        const sptr<Window>& window, bool isRegister, ani_env* env);
    WmErrorCode ProcessWindowStatusDidChangeRegister(const sptr<AniWindowListener>& listener,
        const sptr<Window>& window, bool isRegister, ani_env* env);
    WmErrorCode ProcessAcrossDisplaysChangeRegister(const sptr<AniWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessScreenshotAppEventRegister(const sptr<AniWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, ani_env* env);
    WmErrorCode ProcessFreeWindowModeChangeRegister(const sptr<AniWindowListener>& listener,
        const sptr<Window>& window, bool isRegister, ani_env* env);
    WmErrorCode ProcessListener(RegisterListenerType registerListenerType, CaseType caseType,
        const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister, ani_env* env,
        ani_long timeout);
    std::map<std::string, std::map<ani_ref, sptr<AniWindowListener>>> jsCbMap_;
    std::mutex mtx_;
};
} // namespace Rosen
} // namespace OHOS

#endif