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

#include "floatwindowmanager_fuzzer.h"
#include <securec.h>
#include <fuzzer/FuzzedDataProvider.h>
#include "float_window_manager.h"
#include "float_view_controller.h"
#include "floating_ball_controller.h"
#include "floating_ball_option.h"
#include "float_view_option.h"
#include "window.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 4;
constexpr uint8_t FV_WINDOW_STATE_SIZE = 9;
}

template<class T>
size_t GetObject(T& object, const uint8_t* data, size_t size)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return 0;
    }
    return memcpy_s(&object, objectSize, data, objectSize) == EOK ? objectSize : 0;
}

void FloatWindowManagerBindFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    uint32_t windowId = fdp.ConsumeIntegral<uint32_t>();
    sptr<Window> mainWindow = sptr<Window>::MakeSptr();

    FvOption fvOption;
    std::string uiPath = fdp.ConsumeRandomLengthString(64);
    fvOption.SetUIPath(uiPath);
    uint32_t fvTemplateType = fdp.ConsumeIntegral<uint32_t>();
    fvOption.SetTemplate(fvTemplateType);
    Rect rect;
    rect.posX_ = fdp.ConsumeIntegral<int32_t>();
    rect.posY_ = fdp.ConsumeIntegral<int32_t>();
    rect.width_ = fdp.ConsumeIntegral<uint32_t>();
    rect.height_ = fdp.ConsumeIntegral<uint32_t>();
    fvOption.SetRect(rect);

    napi_env env = nullptr;
    sptr<FloatViewController> fvController = sptr<FloatViewController>::MakeSptr(fvOption, env);
    sptr<FloatingBallController> fbController = sptr<FloatingBallController>::MakeSptr(mainWindow, windowId, nullptr);

    FbOption fbOption;
    std::string title = fdp.ConsumeRandomLengthString(64);
    std::string content = fdp.ConsumeRandomLengthString(64);
    fbOption.SetTitle(title);
    fbOption.SetContent(content);
    uint32_t templateType = fdp.ConsumeIntegral<uint32_t>();
    fbOption.SetTemplate(templateType);

    FloatWindowManager::Bind(fvController, fbController, fbOption);
    FloatWindowManager::UnBind(fvController, fbController);
}

void FloatWindowManagerStartBindFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    uint32_t windowId = fdp.ConsumeIntegral<uint32_t>();
    sptr<Window> mainWindow = sptr<Window>::MakeSptr();

    FvOption fvOption;
    std::string uiPath = fdp.ConsumeRandomLengthString(64);
    fvOption.SetUIPath(uiPath);
    uint32_t fvTemplateType = fdp.ConsumeIntegral<uint32_t>();
    fvOption.SetTemplate(fvTemplateType);
    Rect rect;
    rect.posX_ = fdp.ConsumeIntegral<int32_t>();
    rect.posY_ = fdp.ConsumeIntegral<int32_t>();
    rect.width_ = fdp.ConsumeIntegral<uint32_t>();
    rect.height_ = fdp.ConsumeIntegral<uint32_t>();
    fvOption.SetRect(rect);

    napi_env env = nullptr;
    sptr<FloatViewController> fvController = sptr<FloatViewController>::MakeSptr(fvOption, env);
    sptr<FloatingBallController> fbController = sptr<FloatingBallController>::MakeSptr(mainWindow, windowId, nullptr);

    wptr<FloatViewController> fvControllerWeak = fvController;
    wptr<FloatingBallController> fbControllerWeak = fbController;

    sptr<FbOption> option = sptr<FbOption>::MakeSptr();
    std::string title = fdp.ConsumeRandomLengthString(64);
    option->SetTitle(title);
    uint32_t fbTemplateType = fdp.ConsumeIntegral<uint32_t>();
    option->SetTemplate(fbTemplateType);

    FloatWindowManager::StartBindFloatView(fvControllerWeak);
    FloatWindowManager::StartBindFloatingBall(fbControllerWeak, option);
    FloatWindowManager::StopBindFloatView(fvControllerWeak);
    FloatWindowManager::StopBindFloatingBall(fbControllerWeak);
}

void FloatWindowManagerGetBoundFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    uint32_t windowId = fdp.ConsumeIntegral<uint32_t>();
    sptr<Window> mainWindow = sptr<Window>::MakeSptr();

    FvOption fvOption;
    std::string uiPath = fdp.ConsumeRandomLengthString(64);
    fvOption.SetUIPath(uiPath);
    Rect rect;
    rect.posX_ = fdp.ConsumeIntegral<int32_t>();
    rect.posY_ = fdp.ConsumeIntegral<int32_t>();
    rect.width_ = fdp.ConsumeIntegral<uint32_t>();
    rect.height_ = fdp.ConsumeIntegral<uint32_t>();
    fvOption.SetRect(rect);

    napi_env env = nullptr;
    sptr<FloatViewController> fvController = sptr<FloatViewController>::MakeSptr(fvOption, env);
    sptr<FloatingBallController> fbController = sptr<FloatingBallController>::MakeSptr(mainWindow, windowId, nullptr);

    FloatWindowManager::GetBoundFloatingBall(fvController);
    FloatWindowManager::GetBoundFloatView(fbController);
}

void FloatWindowManagerDestroyFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    sptr<Window> window = sptr<Window>::MakeSptr();
    uint8_t windowTypeIndex = fdp.ConsumeIntegral<uint8_t>() % 3;
    uint32_t windowId = fdp.ConsumeIntegral<uint32_t>();
    (void)windowTypeIndex;
    (void)windowId;
    FloatWindowManager::DestroyFloatWindow(window);
}

void FloatWindowManagerProcessStateChangeFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    sptr<Window> mainWindow = sptr<Window>::MakeSptr();

    FvOption fvOption;
    std::string uiPath = fdp.ConsumeRandomLengthString(64);
    fvOption.SetUIPath(uiPath);
    Rect rect;
    rect.posX_ = fdp.ConsumeIntegral<int32_t>();
    rect.posY_ = fdp.ConsumeIntegral<int32_t>();
    rect.width_ = fdp.ConsumeIntegral<uint32_t>();
    rect.height_ = fdp.ConsumeIntegral<uint32_t>();
    fvOption.SetRect(rect);

    napi_env env = nullptr;
    sptr<FloatViewController> fvController = sptr<FloatViewController>::MakeSptr(fvOption, env);

    wptr<FloatViewController> fvControllerWeak = fvController;

    uint8_t stateIndex = fdp.ConsumeIntegral<uint8_t>() % FV_WINDOW_STATE_SIZE;
    FvWindowState state = static_cast<FvWindowState>(stateIndex);

    FloatWindowManager::ProcessBindFloatViewStateChange(fvControllerWeak, state);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    FloatWindowManagerBindFuzzTest(data, size);
    FloatWindowManagerStartBindFuzzTest(data, size);
    FloatWindowManagerGetBoundFuzzTest(data, size);
    FloatWindowManagerDestroyFuzzTest(data, size);
    FloatWindowManagerProcessStateChangeFuzzTest(data, size);

    return true;
}

} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}