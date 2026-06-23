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

#include "marshalling_helper.h"
#include <securec.h>
#include <iremote_broker.h>
#include <fuzzer/FuzzedDataProvider.h>
#include "ability_context.h"
#include "ability_context_impl.h"
#include "fb_fuzzer.h"
#include "floating_ball_controller.h"
#include "floating_ball_manager.h"
#include "floating_ball_option.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
constexpr size_t DATA_MAX_SIZE = 1024;
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

class MockFbListener : public IFbLifeCycle,
                       public IFbClickObserver {
public:
    MockFbListener() = default;
    virtual ~MockFbListener() = default;

    void OnFloatingBallStart() override {};
    void OnFloatingBallStop() override {};

    void OnClickEvent() override {};
};

void CheckFbControllerFunctionsPart(sptr<FloatingBallController> controller, const uint8_t* data, size_t size)
{
    if (data == nullptr || (size < DATA_MIN_SIZE || size > DATA_MAX_SIZE)) {
        return;
    }

    auto fbLifeCycleListener = sptr<MockFbListener>::MakeSptr();
    controller->RegisterFbLifecycle(fbLifeCycleListener);
    controller->UnRegisterFbLifecycle(fbLifeCycleListener);

    auto fbClickObserverListener = sptr<MockFbListener>::MakeSptr();
    controller->RegisterFbClickObserver(fbClickObserverListener);
    controller->UnRegisterFbClickObserver(fbClickObserverListener);

    std::shared_ptr<AAFwk::Want> want = nullptr;
    controller->RestoreMainWindow(want);

    want = std::make_shared<AAFwk::Want>();
    std::stringstream ss;
    for (size_t i = 0; i < size; ++i) {
        ss << data[i];
    }
    want->SetParam("__startParams", ss.str());
    controller->RestoreMainWindow(want);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }
    size_t startPos = 0;

    uint32_t windowId;
    startPos += GetObject(windowId, data + startPos, size - startPos);
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<FloatingBallController> controller = sptr<FloatingBallController>::MakeSptr(window, windowId, nullptr);
    if (controller == nullptr) {
        return false;
    }

    sptr<FbOption> option = sptr<FbOption>::MakeSptr();
    
    std::string title = "";
    std::string content = "";
    option->SetTitle(title);
    option->SetContent(content);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    option->SetIcon(pixelMap);

    void* contextPtr = nullptr;
    controller->contextPtr_ = contextPtr;
    controller->CreateFloatingBallWindow(option);

    controller->StartFloatingBall(option);
    controller->StartFloatingBallSingle(option, true);

    FuzzedDataProvider fdp(data, size);
    title = fdp.ConsumeRandomLengthString();
    controller->UpdateFloatingBall(option);

    CheckFbControllerFunctionsPart(controller, data + startPos, size - startPos);
    controller->DestroyFloatingBallWindow("fuzztest");
    return true;
}

void FloatingBallManagerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    size_t startPos = 0;

    uint32_t windowId;
    startPos += GetObject(windowId, data + startPos, size - startPos);

    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<FloatingBallController> controller = sptr<FloatingBallController>::MakeSptr(window, windowId, nullptr);

    FloatingBallManager::IsActiveController(controller);
    FloatingBallManager::SetActiveController(controller);
    FloatingBallManager::RemoveActiveController(controller);
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::FloatingBallManagerFuzzTest(data, size);
    return 0;
}