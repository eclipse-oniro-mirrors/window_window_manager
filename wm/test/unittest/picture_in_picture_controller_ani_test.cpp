/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include <gmock/gmock.h>
#include "ability_context_impl.h"
#include "mock_session.h"
#include "parameters.h"
#include "picture_in_picture_controller_ani.h"
#include "picture_in_picture_manager.h"
#include "float_window_manager.h"
#include "float_view_controller.h"
#include "float_view_manager.h"
#include "window.h"
#include "window_session_impl.h"
#include "wm_common.h"
#include "xcomponent_controller.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MockWindow : public Window {
public:
    MockWindow() {};
    ~MockWindow() {};
    MOCK_METHOD5(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus,
        int32_t requestId, int32_t scbRequestId));
    MOCK_METHOD2(Destroy, WMError(uint32_t reason, bool isFromInnerkits));
    MOCK_METHOD1(NotifyPrepareClosePiPWindow, WMError(const bool isWeb));
    MOCK_METHOD4(SetAutoStartPiP, void(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height));
    MOCK_CONST_METHOD0(GetWindowState, WindowState());
};

class MockAni : public ani_env {
    ani_status GetVM(ani_vm **result)
    {
        return ANI_OK;
    }
};

class MockAniVmApi {
public:
    static ani_status GetEnv(ani_vm *vm, uint32_t version, ani_env **result)
    {
        if (result != nullptr) {
            *result = nullptr;
        }
        return ANI_OK;
    }
};

class MockAniInteractionApi {
public:
    static ani_status GetVM(ani_env *env, ani_vm **result)
    {
        static struct __ani_vm_api vmApi = {};
        vmApi.GetEnv = &MockAniVmApi::GetEnv;
        static struct __ani_vm vm = {};
        vm.c_api = &vmApi;
        if (result != nullptr) {
            *result = &vm;
        }
        return ANI_OK;
    }
};

class MockXComponentController : public XComponentController {
public:
    MockXComponentController() {}
    ~MockXComponentController() {}
    MOCK_METHOD2(GetGlobalPosition, XComponentControllerErrorCode(float& offsetX, float& offsetY));
    MOCK_METHOD2(GetSize, XComponentControllerErrorCode(float& width, float& height));
    MOCK_METHOD1(SetExtController,
        XComponentControllerErrorCode(std::shared_ptr<XComponentController> xComponentController));
    MOCK_METHOD1(ResetExtController,
        XComponentControllerErrorCode(std::shared_ptr<XComponentController> xComponentController));
};

class MockPictureInPictureControllerAni : public PictureInPictureControllerAni {
public:
    MockPictureInPictureControllerAni(sptr<PipOptionAni> pipOption, sptr<Window> mainWindow, uint32_t mainWindowId,
        ani_env* env) : PictureInPictureControllerAni(pipOption, mainWindow, mainWindowId, env) {}
    ~MockPictureInPictureControllerAni() {};
    MOCK_METHOD1(GetNavigationController, NavigationController*(const std::string& navId));

    // Expose the protected members/methods required by the NotifyStateChangeInner tests
    // so that they can be exercised from a non-derived test fixture.
    using PictureInPictureControllerAni::NotifyStateChangeInner;
    sptr<PipOptionAni>& GetPipOptionRef() { return pipOption_; }
    ani_vm*& GetVmRef() { return vm_; }
    std::shared_ptr<AppExecFwk::EventHandler>& GetEventHandlerRef() { return eventHandler_; }
    wptr<PictureInPictureControllerAni>& GetWeakRef() { return weakRef_; }
};

class MockNavigationController : public NavigationController {
public:
    void SetInPIPMode(int32_t handle)
    {
        handle_ = handle;
    }

    void DeletePIPMode(int32_t handle)
    {
        handle_ = 0;
    }

    MOCK_METHOD0(IsNavDestinationInTopStack, bool());
    MOCK_METHOD0(GetTopHandle, int32_t());
public:
    int handle_ {0};
};

class PictureInPictureControllerAniTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    static constexpr uint32_t waitSyncInNs = 200000;

    sptr<MockPictureInPictureControllerAni> pipControl_ {nullptr};
    std::shared_ptr<MockNavigationController> naviControl_ {nullptr};
};

void PictureInPictureControllerAniTest::SetUpTestCase()
{
}

void PictureInPictureControllerAniTest::TearDownTestCase()
{
}

void PictureInPictureControllerAniTest::SetUp()
{
    sptr<MockWindow> mw = sptr<MockWindow>::MakeSptr();
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    uint32_t mainWindowId = 100;
    const struct __ani_interaction_api api = {
        .GetVM = &MockAniInteractionApi::GetVM
    };
    ani_env env = {.c_api = &api};
    pipControl_ = sptr<MockPictureInPictureControllerAni>::MakeSptr(option, mw, mainWindowId, &env);
    ASSERT_NE(nullptr, pipControl_);
    naviControl_ = std::make_shared<MockNavigationController>();
    ASSERT_NE(nullptr, naviControl_);
}

void PictureInPictureControllerAniTest::TearDown()
{
    pipControl_ = nullptr;
    naviControl_ = nullptr;
}

class MockPiPActiveStatus : public IPiPActiveStatusObserver {
public:
    void OnActiveStatusChange(const bool& status) override
    {
        return;
    }
};

namespace {
class MockAbilityContextImpl : public AbilityRuntime::AbilityContextImpl {
public:
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override
    {
        GTEST_LOG_(INFO) << "GetApplicationInfo";
        return std::make_shared<AppExecFwk::ApplicationInfo>();
    }
};

// Counts how many times the posted state-change task reaches vm->GetEnv.
std::atomic<int32_t> g_aniGetEnvCount {0};

ani_status GetEnvOkNull(ani_vm *vm, uint32_t version, ani_env **result)
{
    (void)vm;
    (void)version;
    ++g_aniGetEnvCount;
    if (result != nullptr) {
        *result = nullptr;
    }
    return ANI_OK;
}

ani_status GetEnvFail(ani_vm *vm, uint32_t version, ani_env **result)
{
    (void)vm;
    (void)version;
    ++g_aniGetEnvCount;
    if (result != nullptr) {
        *result = nullptr;
    }
    return ANI_ERROR;
}

ani_status GetEnvOkNonNull(ani_vm *vm, uint32_t version, ani_env **result)
{
    (void)vm;
    (void)version;
    ++g_aniGetEnvCount;
    static int32_t dummyEnv = 0;
    if (result != nullptr) {
        *result = reinterpret_cast<ani_env*>(&dummyEnv);
    }
    return ANI_OK;
}

ani_vm* BuildFakeVmOkNull()
{
    static struct __ani_vm_api api = {};
    static struct __ani_vm vm = {};
    api.GetEnv = &GetEnvOkNull;
    vm.c_api = &api;
    return &vm;
}

ani_vm* BuildFakeVmFail()
{
    static struct __ani_vm_api api = {};
    static struct __ani_vm vm = {};
    api.GetEnv = &GetEnvFail;
    vm.c_api = &api;
    return &vm;
}

ani_vm* BuildFakeVmOkNonNull()
{
    static struct __ani_vm_api api = {};
    static struct __ani_vm vm = {};
    api.GetEnv = &GetEnvOkNonNull;
    vm.c_api = &api;
    return &vm;
}

static constexpr uint32_t WAIT_FOR_ANI_GET_ENV_TIME = 5;

bool WaitForAniGetEnv(int32_t expectedMin, int timeoutMs = 1000)
{
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count() < timeoutMs) {
        if (g_aniGetEnvCount.load() >= expectedMin) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_ANI_GET_ENV_TIME));
    }
    return g_aniGetEnvCount.load() >= expectedMin;
}

/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, CreatePictureInPictureWindow01, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);

    pipControl_->pipOption_ = nullptr;
    StartPipType startType = StartPipType::AUTO_START;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);
    pipControl_->pipOption_ = option;
    option->SetContext(nullptr);
    ASSERT_EQ(nullptr, option->GetContext());
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);
    auto contextPtr = std::make_shared<MockAbilityContextImpl>();
    option->SetContext(&contextPtr);
    pipControl_->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);

    startType = StartPipType::USER_START;
    pipControl_->mainWindow_ = mw;
    pipControl_->pipOption_->SetTypeNodeEnabled(true);

    EXPECT_CALL(*(mw), GetWindowState()).Times(4).
    WillOnce(Return(WindowState::STATE_HIDDEN)).WillOnce(Return(WindowState::STATE_HIDDEN)).
    WillOnce(Return(WindowState::STATE_SHOWN)).WillOnce(Return(WindowState::STATE_SHOWN));
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);

    FloatWindowManager::floatViewCnt_ = 1;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);

    EXPECT_CALL(*(mw), GetWindowState()).Times(2).
    WillOnce(Return(WindowState::STATE_SHOWN)).WillOnce(Return(WindowState::STATE_SHOWN));
    auto option_ = sptr<FvOption>::MakeSptr();
    ASSERT_NE(nullptr, option_);
    sptr<FloatViewController> fvController =
        sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    fvController->curState_ = FvWindowState::FV_STATE_STARTED;
    FloatViewManager::SetActiveController(fvController);
    EXPECT_EQ(WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS,
        pipControl_->CreatePictureInPictureWindow(startType).errCode);
}

/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, CreatePictureInPictureWindow02, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);

    pipControl_->pipOption_ = option;
    StartPipType startType = StartPipType::AUTO_START;
    auto contextPtr = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    option->SetContext(&contextPtr);

    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    pipControl_->pipOption_->SetXComponentController(nullptr);
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);
    pipControl_->pipOption_->SetXComponentController(xComponentController);
    ASSERT_EQ(pipControl_->pipOption_->GetXComponentController(), xComponentController);
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);
    pipControl_->pipOption_->SetXComponentController(nullptr);
    pipControl_->mainWindow_ = mw;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);
}

/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, CreatePictureInPictureWindow03, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);

    pipControl_->pipOption_ = option;
    option->SetDefaultWindowSizeType(1);
    auto contextPtr = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    option->SetContext(&contextPtr);
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    pipControl_->pipOption_->SetXComponentController(xComponentController);
    ASSERT_EQ(pipControl_->pipOption_->GetXComponentController(), xComponentController);
    pipControl_->pipOption_->SetTypeNodeEnabled(true);
    pipControl_->mainWindow_ = mw;
    StartPipType startType = StartPipType::NULL_START;
    EXPECT_CALL(*(mw), GetWindowState()).Times(2)
        .WillOnce(Return(WindowState::STATE_CREATED)).WillOnce(Return(WindowState::STATE_CREATED));
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->CreatePictureInPictureWindow(startType).errCode);
    EXPECT_EQ(1, option->GetDefaultWindowSizeType());
    startType = StartPipType::AUTO_START;
}

/**
 * @tc.name: StartPictureInPicture
 * @tc.desc: StartPictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, StartPictureInPicture01, TestSize.Level1)
{
    StartPipType startType = StartPipType::AUTO_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);

    pipControl_->pipOption_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->StartPictureInPicture(startType).errCode);
    pipControl_->pipOption_ = option;
    option->SetContext(nullptr);
    ASSERT_EQ(nullptr, option->GetContext());
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->StartPictureInPicture(startType).errCode);
    auto contextPtr = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    option->SetContext(&contextPtr);
    pipControl_->curState_ = PiPWindowState::STATE_STARTING;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl_->StartPictureInPicture(startType).errCode);
    pipControl_->curState_ = PiPWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl_->StartPictureInPicture(startType).errCode);
}

/**
 * @tc.name: StartPictureInPicture
 * @tc.desc: StartPictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, StartPictureInPicture02, TestSize.Level1)
{
    StartPipType startType = StartPipType::AUTO_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);

    pipControl_->pipOption_ = option;
    auto contextPtr = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    option->SetContext(&contextPtr);
    pipControl_->curState_ = PiPWindowState::STATE_UNDEFINED;

    pipControl_->pipOption_->SetNavigationId("navId");
    pipControl_->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->StartPictureInPicture(startType).errCode);
    pipControl_->pipOption_->SetNavigationId("");
    auto pipControl1 = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    auto pipControl2 = sptr<PictureInPictureController>::MakeSptr(option, mw, 200, nullptr);
    PictureInPictureManager::PutPipControllerInfo(300, pipControl1);
    PictureInPictureManager::IsActiveController(pipControl2);
    pipControl_->mainWindowId_ = 100;
    PictureInPictureManager::IsAttachedToSameWindow(100);
    pipControl_->window_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->StartPictureInPicture(startType).errCode);
    pipControl_->window_ = mw;
    pipControl_->pipOption_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->StartPictureInPicture(startType).errCode);
    pipControl_->pipOption_ = option;
    PictureInPictureManager::RemovePipControllerInfo(300);
    PictureInPictureManager::IsActiveController(pipControl_);
    pipControl_->StartPictureInPicture(startType);
}

/**
 * @tc.name: StartPictureInPicture03
 * @tc.desc: StartPictureInPicture with reachLimit=true, same group window exists but
 *           preState != STATE_STARTED, reuse branch skipped and falls through to
 *           StartPictureInPictureInner
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, StartPictureInPicture03, TestSize.Level1)
{
    StartPipType startType = StartPipType::USER_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto contextPtr = std::make_shared<MockAbilityContextImpl>();
    option->SetContext(&contextPtr);
    pipControl_->pipOption_ = option;
    pipControl_->curState_ = PiPWindowState::STATE_UNDEFINED;
    pipControl_->mainWindowId_ = 100;
    pipControl_->mainWindow_ = mw;

    auto existingControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    existingControl->window_ = mw;
    existingControl->mainWindowId_ = 100;
    // keep existingControl->curState_ as STATE_UNDEFINED so that preState != STATE_STARTED
    PictureInPictureManager::PutPipControllerInfo(700, existingControl);

    // reachLimit=true, window!=nullptr but preState==STATE_UNDEFINED, reuse branch skipped,
    // falls through to StartPictureInPictureInner, create fails
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl_->StartPictureInPicture(startType).errCode);

    PictureInPictureManager::RemovePipControllerInfo(700);
}

/**
 * @tc.name: StartPictureInPicture04
 * @tc.desc: StartPictureInPicture with reachLimit=true, reuse same group window whose
 *           state is STATE_STARTED
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, StartPictureInPicture04, TestSize.Level1)
{
    StartPipType startType = StartPipType::USER_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto contextPtr = std::make_shared<MockAbilityContextImpl>();
    option->SetContext(&contextPtr);
    pipControl_->pipOption_ = option;
    pipControl_->curState_ = PiPWindowState::STATE_UNDEFINED;
    pipControl_->mainWindowId_ = 100;
    pipControl_->mainWindow_ = mw;

    auto existingControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    existingControl->window_ = mw;
    existingControl->mainWindowId_ = 100;
    existingControl->curState_ = PiPWindowState::STATE_STARTED;
    PictureInPictureManager::PutPipControllerInfo(800, existingControl);

    // reachLimit=true, window!=nullptr && preState==STATE_STARTED, reuse path:
    // DoClose then ShowPictureInPictureWindow
    EXPECT_CALL(*(mw), Show(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, pipControl_->StartPictureInPicture(startType).errCode);

    PictureInPictureManager::RemovePipControllerInfo(800);
    // reuse path calls PutPipControllerInfo(window_->GetWindowId(), this), and MockWindow::GetWindowId()
    // returns INVALID_WINDOW_ID(0), so clean up the leaked entry holding pipControl_ (and mainWindow_ mock)
    PictureInPictureManager::RemovePipControllerInfo(INVALID_WINDOW_ID);
}

/**
 * @tc.name: NotifyStateChangeInner01
 * @tc.desc: eventHandler_ is nullptr, the function returns without posting a task
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner01, TestSize.Level1)
{
    pipControl_->GetEventHandlerRef() = nullptr;
    pipControl_->GetVmRef() = BuildFakeVmOkNull();
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::ABOUT_TO_START);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(0, g_aniGetEnvCount.load()) << "no task should be posted when eventHandler_ is null";
}

/**
 * @tc.name: NotifyStateChangeInner02
 * @tc.desc: eventHandler_ has no runner, PostTask returns false and no task is executed
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner02, TestSize.Level1)
{
    pipControl_->GetEventHandlerRef() = std::make_shared<AppExecFwk::EventHandler>(nullptr);
    pipControl_->GetVmRef() = BuildFakeVmOkNull();
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::STARTED);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(0, g_aniGetEnvCount.load()) << "PostTask failure must not execute the task";
}

/**
 * @tc.name: NotifyStateChangeInner03
 * @tc.desc: PostTask succeeds but weakRef_ is null, the task exits at controller null
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner03, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("pip_ani_notify_03");
    ASSERT_NE(nullptr, runner);
    pipControl_->GetEventHandlerRef() = std::make_shared<AppExecFwk::EventHandler>(runner);
    pipControl_->GetWeakRef() = nullptr;
    pipControl_->GetVmRef() = BuildFakeVmOkNull();
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::STOPPED);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(0, g_aniGetEnvCount.load()) << "task must exit before GetEnv when controller is null";
}

/**
 * @tc.name: NotifyStateChangeInner04
 * @tc.desc: PostTask succeeds, controller is valid but vm_ is null, the task exits at vm null
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner04, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("pip_ani_notify_04");
    ASSERT_NE(nullptr, runner);
    pipControl_->GetEventHandlerRef() = std::make_shared<AppExecFwk::EventHandler>(runner);
    ASSERT_NE(nullptr, pipControl_->GetWeakRef().promote());
    pipControl_->GetVmRef() = nullptr;
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::STARTED);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(0, g_aniGetEnvCount.load()) << "task must exit before GetEnv when vm_ is null";
}

/**
 * @tc.name: NotifyStateChangeInner05
 * @tc.desc: PostTask succeeds, vm->GetEnv returns non-OK, the task exits at GetEnv failure
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner05, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("pip_ani_notify_05");
    ASSERT_NE(nullptr, runner);
    pipControl_->GetEventHandlerRef() = std::make_shared<AppExecFwk::EventHandler>(runner);
    ASSERT_NE(nullptr, pipControl_->GetWeakRef().promote());
    pipControl_->GetVmRef() = BuildFakeVmFail();
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::STARTED);
    EXPECT_TRUE(WaitForAniGetEnv(1)) << "task should reach vm->GetEnv";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(1, g_aniGetEnvCount.load()) << "GetEnv should be called exactly once on failure";
}

/**
 * @tc.name: NotifyStateChangeInner06
 * @tc.desc: PostTask succeeds, vm->GetEnv returns OK but env is null, the task exits at env null
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner06, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("pip_ani_notify_06");
    ASSERT_NE(nullptr, runner);
    pipControl_->GetEventHandlerRef() = std::make_shared<AppExecFwk::EventHandler>(runner);
    ASSERT_NE(nullptr, pipControl_->GetWeakRef().promote());
    pipControl_->GetVmRef() = BuildFakeVmOkNull();
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::STARTED);
    EXPECT_TRUE(WaitForAniGetEnv(1)) << "task should reach vm->GetEnv";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(1, g_aniGetEnvCount.load()) << "GetEnv should be called exactly once when env is null";
}

/**
 * @tc.name: NotifyStateChangeInner07
 * @tc.desc: PostTask succeeds, GetEnv returns OK with a non-null env, the task invokes the
 *           inner overload which returns early because no stateChange callback is registered
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerAniTest, NotifyStateChangeInner07, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("pip_ani_notify_07");
    ASSERT_NE(nullptr, runner);
    pipControl_->GetEventHandlerRef() = std::make_shared<AppExecFwk::EventHandler>(runner);
    ASSERT_NE(nullptr, pipControl_->GetWeakRef().promote());
    pipControl_->GetPipOptionRef() = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, pipControl_->GetPipOptionRef());
    EXPECT_EQ(nullptr, pipControl_->GetANIPipContentCallbackRef("stateChange"));
    pipControl_->GetVmRef() = BuildFakeVmOkNonNull();
    g_aniGetEnvCount = 0;
    pipControl_->NotifyStateChangeInner(PiPState::STARTED);
    EXPECT_TRUE(WaitForAniGetEnv(1)) << "task should reach vm->GetEnv";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(1, g_aniGetEnvCount.load()) << "GetEnv should be called exactly once on success";
    EXPECT_EQ(nullptr, pipControl_->GetANIPipContentCallbackRef("stateChange"))
        << "no stateChange callback should be registered so the inner overload returns early";
}

} // namespace
} // namespace Rosen
} // namespace OHOS