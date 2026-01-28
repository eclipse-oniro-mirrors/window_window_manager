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

#ifndef WINDOW_MANAGER_IMPL_H
#define WINDOW_MANAGER_IMPL_H
 
#include "window.h"
 
namespace OHOS {
namespace Rosen {
struct WindowParameters {
    std::string name;
    uint32_t winType;
    OHOS::AbilityRuntime::Context* context;
    int64_t displayId;
    int64_t parentId;
    int64_t* windowId;
};

class WindowManagerImpl {
public:
    static int32_t CreateWindow(WindowParameters window);
    static int32_t SetWindowLayoutMode(uint32_t mode);
    static int32_t MinimizeAll(int64_t displayId, int32_t excludeWindowId = 0);
    static int32_t FindWindow(std::string name, int64_t& windowId);
    static int32_t GetLastWindow(OHOS::AbilityRuntime::Context* ctx, int64_t& id);
    static int32_t ShiftAppWindowFocus(int32_t sourceWindowId, int32_t targetWindowId);
    
private:
};
}
}
#endif
