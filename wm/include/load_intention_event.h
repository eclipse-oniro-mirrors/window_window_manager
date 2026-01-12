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

#ifndef LOAD_INTENTION_EVENT_H
#define LOAD_INTENTION_EVENT_H

#include <dlfcn.h>
#include <unistd.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "window.h"
#include "ui_content.h"
#include "event_handler.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string INTENTION_EVENT_SO_PATH = "/system/lib64/libintention_event.z.so";
#else
const std::string INTENTION_EVENT_SO_PATH = "/system/lib/libintention_event.z.so";
#endif

bool LoadIntentionEvent(void);
void UnloadIntentionEvent(void);
bool EnableInputEventListener(
    Ace::UIContent* uiContent, std::shared_ptr<AppExecFwk::EventHandler> eventHandler, wptr<Window> window);
}
}
#endif /* LOAD_INTENTION_EVENT_H */