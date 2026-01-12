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

#include "scene_board_judgement.h"
#include "screen_session_manager/include/fold_screen_controller/fold_screen_controller.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
class FoldScreenControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static ScreenSessionManager& ssm_;
};

ScreenSessionManager& FoldScreenControllerTest::ssm_ = ScreenSessionManager::GetInstance();

void FoldScreenControllerTest::SetUpTestCase()
{
}

void FoldScreenControllerTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void FoldScreenControllerTest::SetUp()
{
}

void FoldScreenControllerTest::TearDown()
{
}

namespace {

    /**
     * @tc.name: GetFoldScreenPolicy
     * @tc.desc: test function :GetFoldScreenPolicy
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetFoldScreenPolicy, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        DisplayDeviceType productType = DisplayDeviceType::SINGLE_DISPLAY_DEVICE;
        auto ret = fsc_.GetFoldScreenPolicy(productType);
        ASSERT_NE(ret, nullptr);

        productType = DisplayDeviceType::DOUBLE_DISPLAY_DEVICE;
        ret = fsc_.GetFoldScreenPolicy(productType);
        ASSERT_NE(ret, nullptr);

        productType = DisplayDeviceType::SINGLE_DISPLAY_POCKET_DEVICE;
        ret = fsc_.GetFoldScreenPolicy(productType);
        ASSERT_NE(ret, nullptr);

        productType = DisplayDeviceType::DISPLAY_DEVICE_UNKNOWN;
        ret = fsc_.GetFoldScreenPolicy(productType);
        ASSERT_EQ(ret, nullptr);
    }

    /**
     * @tc.name: SetDisplayMode01
     * @tc.desc: test function :SetDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetDisplayMode01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        FoldDisplayMode displayMode = FoldDisplayMode::FULL;
        fsc_.SetDisplayMode(displayMode);
        ASSERT_EQ(fsc_.GetDisplayMode(), FoldDisplayMode::UNKNOWN);
    }

    /**
     * @tc.name: SetDisplayMode02
     * @tc.desc: test function :SetDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetDisplayMode02, TestSize.Level1)
    {
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            GTEST_SKIP();
        }
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        ASSERT_NE(fsc_.foldScreenPolicy_, nullptr);
        auto mode = fsc_.GetDisplayMode();
        FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
        fsc_.SetDisplayMode(displayMode);
        if (ssm_.IsFoldable()) {
            ASSERT_EQ(fsc_.GetDisplayMode(), displayMode);
            fsc_.SetDisplayMode(mode);
        } else {
            ASSERT_EQ(fsc_.GetDisplayMode(), FoldDisplayMode::UNKNOWN);
        }
    }

    /**
     * @tc.name: LockDisplayStatus01
     * @tc.desc: test function :LockDisplayStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, LockDisplayStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        bool locked = false;
        fsc_.LockDisplayStatus(locked);
        ASSERT_EQ(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: LockDisplayStatus02
     * @tc.desc: test function :LockDisplayStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, LockDisplayStatus02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        bool locked = false;
        fsc_.LockDisplayStatus(locked);
        ASSERT_EQ((fsc_.foldScreenPolicy_)->lockDisplayStatus_, locked);
    }

    /**
     * @tc.name: ForceSetFoldStatusAndLock01
     * @tc.desc: test function :ForceSetFoldStatusAndLock
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, ForceSetFoldStatusAndLock01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());
        fsc_.foldScreenPolicy_ = nullptr;
        DMError ret = fsc_.ForceSetFoldStatusAndLock(FoldStatus::FOLDED);
        ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    }

    /**
     * @tc.name: ForceSetFoldStatusAndLock02
     * @tc.desc: test function :ForceSetFoldStatusAndLock
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, ForceSetFoldStatusAndLock02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        LOG_SetCallback(MyLogCallback);
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());
        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        bool wasFoldStatusLocked = fsc_.foldScreenPolicy_->GetPhysicalFoldLockFlag();
        FoldStatus previousForceFoldStatus = fsc_.foldScreenPolicy_->GetForceFoldStatus();
        DMError ret = fsc_.ForceSetFoldStatusAndLock(FoldStatus::FOLDED);
        if (ret == DMError::DM_OK) {
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetPhysicalFoldLockFlag(), true);
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetForceFoldStatus(), FoldStatus::FOLDED);
            fsc_.RestorePhysicalFoldStatus();
        } else {
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetPhysicalFoldLockFlag(), wasFoldStatusLocked);
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetForceFoldStatus(), previousForceFoldStatus);
        }
    }

    /**
     * @tc.name: RestorePhysicalFoldStatus01
     * @tc.desc: test function :RestorePhysicalFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, RestorePhysicalFoldStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());
        fsc_.foldScreenPolicy_ = nullptr;
        DMError ret = fsc_.RestorePhysicalFoldStatus();
        ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    }

    /**
     * @tc.name: RestorePhysicalFoldStatus02
     * @tc.desc: test function :RestorePhysicalFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, RestorePhysicalFoldStatus02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        LOG_SetCallback(MyLogCallback);
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());
        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        bool wasFoldStatusLocked = fsc_.foldScreenPolicy_->GetPhysicalFoldLockFlag();
        FoldStatus previousForcedFoldStatus = fsc_.foldScreenPolicy_->GetForceFoldStatus();
        g_errLog.clear();

        fsc_.ForceSetFoldStatusAndLock(FoldStatus::FOLDED);
        DMError ret = fsc_.RestorePhysicalFoldStatus();
        if (ret == DMError::DM_OK) {
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetPhysicalFoldLockFlag(), false);
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetForceFoldStatus(), FoldStatus::UNKNOWN);
        } else {
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetPhysicalFoldLockFlag(), wasFoldStatusLocked);
            ASSERT_EQ(fsc_.foldScreenPolicy_->GetForceFoldStatus(), previousForcedFoldStatus);
        }

        g_errLog.clear();
    }

    /**
     * @tc.name: GetPhysicalFoldLockFlagAndPhysicalFoldStatus01
     * @tc.desc: test function :GetPhysicalFoldLockFlag and GetPhysicalFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetPhysicalFoldLockFlagAndPhysicalFoldStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());
        auto wasFoldStatus = fsc_.GetFoldStatus();
        fsc_.foldScreenPolicy_ = nullptr;
        ASSERT_EQ(fsc_.GetPhysicalFoldLockFlag(), false);
        ASSERT_EQ(fsc_.GetPhysicalFoldStatus(), wasFoldStatus);
    }

    /**
     * @tc.name: GetPhysicalFoldLockFlagAndPhysicalFoldStatus02
     * @tc.desc: test function :GetPhysicalFoldLockFlag and GetPhysicalFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetPhysicalFoldLockFlagAndPhysicalFoldStatus02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());
        auto wasFoldStatus = fsc_.GetFoldStatus();
        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        DMError ret = fsc_.ForceSetFoldStatusAndLock(FoldStatus::FOLDED);
        if (ret == DMError::DM_OK) {
            ASSERT_EQ(fsc_.GetPhysicalFoldLockFlag(), true);
        } else {
            ASSERT_EQ(fsc_.GetPhysicalFoldStatus(), wasFoldStatus);
        }
    }

    /**
     * @tc.name: GetDisplayMode
     * @tc.desc: test function :GetDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetDisplayMode01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        auto ret = fsc_.GetDisplayMode();
        ASSERT_EQ(ret, FoldDisplayMode::UNKNOWN);
    }

    /**
     * @tc.name: GetDisplayMode
     * @tc.desc: foldScreenPolicy_ is not nullptr
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetDisplayMode02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        auto ret = fsc_.GetDisplayMode();
        ASSERT_EQ(ret, FoldDisplayMode::UNKNOWN);
    }

    /**
     * @tc.name: GetFoldStatus01
     * @tc.desc: test function :GetFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetFoldStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        auto ret = fsc_.GetFoldStatus();
        ASSERT_EQ(ret, FoldStatus::UNKNOWN);
    }

    /**
     * @tc.name: GetFoldStatus02
     * @tc.desc: test function :GetFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetFoldStatus02, TestSize.Level1)
    {
        g_errLog.clear();
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        ASSERT_NE(fsc_.foldScreenPolicy_, nullptr);

        FoldStatus wasFoldStatus = fsc_.GetFoldStatus();
        DMError ret = fsc_.ForceSetFoldStatusAndLock(FoldStatus::FOLDED);
        if (ret == DMError::DM_OK) {
            EXPECT_EQ(fsc_.GetFoldStatus(), fsc_.foldScreenPolicy_->GetForceFoldStatus());
            EXPECT_TRUE(g_errLog.find("foldScreenPolicy_ lock fold status") == std::string::npos);
            fsc_.RestorePhysicalFoldStatus();
        } else {
            EXPECT_EQ(fsc_.GetFoldStatus(), wasFoldStatus);
        }
        g_errLog.clear();
    }

    /**
     * @tc.name: SetFoldStatus01
     * @tc.desc: test function :SetFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetFoldStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        FoldStatus foldStatus = FoldStatus::HALF_FOLD;
        fsc_.foldScreenPolicy_ = nullptr;
        fsc_.SetFoldStatus(foldStatus);
        ASSERT_EQ(fsc_.foldScreenPolicy_, nullptr);
        ASSERT_EQ(fsc_.GetFoldStatus(), FoldStatus::UNKNOWN);
    }

    /**
     * @tc.name: SetFoldStatus02
     * @tc.desc: test function :SetFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetFoldStatus02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        FoldStatus foldStatus = FoldStatus::HALF_FOLD;
        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.SetFoldStatus(foldStatus);
        ASSERT_EQ(fsc_.GetFoldStatus(), foldStatus);
    }

    /**
     * @tc.name: GetCurrentFoldCreaseRegion
     * @tc.desc: test function :GetCurrentFoldCreaseRegion
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetCurrentFoldCreaseRegion01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        auto ret = fsc_.GetCurrentFoldCreaseRegion();
        ASSERT_EQ(ret, nullptr);
    }

    /**
     * @tc.name: GetCurrentFoldCreaseRegion
     * @tc.desc: test function :GetCurrentFoldCreaseRegion
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetCurrentFoldCreaseRegion02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        auto ret = fsc_.GetCurrentFoldCreaseRegion();
        ASSERT_EQ(ret, fsc_.foldScreenPolicy_->GetCurrentFoldCreaseRegion());
    }

    /**
     * @tc.name: GetCurrentScreenId01
     * @tc.desc: test function :GetCurrentScreenId
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetCurrentScreenId01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        auto ret = fsc_.GetCurrentScreenId();
        ASSERT_EQ(ret, 0);
    }

    /**
     * @tc.name: GetCurrentScreenId02
     * @tc.desc: test function :GetCurrentScreenId
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetCurrentScreenId02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        auto ret = fsc_.GetCurrentScreenId();
        ASSERT_NE(ret, 0);
    }

    /**
     * @tc.name: GetCurrentScreenId03
     * @tc.desc: test function :GetCurrentScreenId
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetCurrentScreenId03, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        auto ret = fsc_.GetCurrentScreenId();
        ASSERT_EQ(ret, 0);

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        ret = fsc_.GetCurrentScreenId();
        ASSERT_NE(ret, 0);
    }

    /**
     * @tc.name: SetOnBootAnimation01
     * @tc.desc: test function :SetOnBootAnimation
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetOnBootAnimation01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        bool onBootAnimation = false;
        fsc_.foldScreenPolicy_ = nullptr;
        fsc_.SetOnBootAnimation(onBootAnimation);
        EXPECT_TRUE(g_errLog.find("foldScreenPolicy_is null") != std::string::npos);
        g_errLog.clear();
        LOG_SetCallback(nullptr);
    }

    /**
     * @tc.name: SetOnBootAnimation02
     * @tc.desc: test function :SetOnBootAnimation
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetOnBootAnimation02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        bool onBootAnimation = true;
        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.SetOnBootAnimation(onBootAnimation);
        EXPECT_TRUE(g_errLog.find("foldScreenPolicy_is null") == std::string::npos);
        g_errLog.clear();
        LOG_SetCallback(nullptr);
    }

    /**
     * @tc.name: SetOnBootAnimation03
     * @tc.desc: test function :SetOnBootAnimation
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetOnBootAnimation03, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        bool onBootAnimation = true;
        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.SetOnBootAnimation(onBootAnimation);
        ASSERT_EQ(onBootAnimation, true);
        fsc_.foldScreenPolicy_ = nullptr;
        bool onBootAnimation01 = false;
        fsc_.SetOnBootAnimation(onBootAnimation01);
        ASSERT_EQ(onBootAnimation, true);
    }

    /**
     * @tc.name: UpdateForPhyScreenPropertyChange01
     * @tc.desc: test function :UpdateForPhyScreenPropertyChange
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, UpdateForPhyScreenPropertyChange01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        fsc_.UpdateForPhyScreenPropertyChange();
        ASSERT_EQ(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: UpdateForPhyScreenPropertyChange02
     * @tc.desc: test function :UpdateForPhyScreenPropertyChange
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, UpdateForPhyScreenPropertyChange02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.UpdateForPhyScreenPropertyChange();
        ASSERT_NE(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: AddOrRemoveDisplayNodeToTree01
     * @tc.desc: test function :AddOrRemoveDisplayNodeToTree
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, AddOrRemoveDisplayNodeToTree01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        ScreenId screenId = 1;
        int32_t command = 0;

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.AddOrRemoveDisplayNodeToTree(screenId, command);
        ASSERT_NE(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: AddOrRemoveDisplayNodeToTree02
     * @tc.desc: test function :AddOrRemoveDisplayNodeToTree
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, AddOrRemoveDisplayNodeToTree02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        ScreenId screenId = 0;
        int32_t command = 1;

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.AddOrRemoveDisplayNodeToTree(screenId, command);
        ASSERT_NE(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: GetdisplayModeRunningStatus&SetdisplayModeChangeStatus01
     * @tc.desc: test function :GetdisplayModeRunningStatus&SetdisplayModeChangeStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, ModeChangeStatusTest01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        bool status = false;

        fsc_.SetdisplayModeChangeStatus(status);
        auto ret = fsc_.GetdisplayModeRunningStatus();

        ASSERT_EQ(ret, false);
    }

    /**
     * @tc.name: GetdisplayModeRunningStatus&SetdisplayModeChangeStatus02
     * @tc.desc: test function :GetdisplayModeRunningStatus&SetdisplayModeChangeStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, ModeChangeStatusTest02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        bool status = true;

        fsc_.SetdisplayModeChangeStatus(status);
        auto ret = fsc_.GetdisplayModeRunningStatus();

        ASSERT_EQ(ret, true);
    }

    /**
     * @tc.name: GetLastCacheDisplayMode01
     * @tc.desc: test function :GetLastCacheDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetLastCacheDisplayMode01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
        fsc_.SetDisplayMode(displayMode);
        auto ret = fsc_.GetLastCacheDisplayMode();

        ASSERT_EQ(ret, FoldDisplayMode::UNKNOWN);
    }

    /**
     * @tc.name: GetLastCacheDisplayMode02
     * @tc.desc: test function :GetLastCacheDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetLastCacheDisplayMode02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        FoldDisplayMode displayMode = FoldDisplayMode::FULL;
        fsc_.SetDisplayMode(displayMode);
        auto ret = fsc_.GetLastCacheDisplayMode();

        ASSERT_EQ(ret, FoldDisplayMode::FULL);
    }

    /**
     * @tc.name: GetModeChangeRunningStatus
     * @tc.desc: test function :GetModeChangeRunningStatus
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetModeChangeRunningStatus, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        auto ret = fsc_.GetModeChangeRunningStatus();

        ASSERT_EQ(ret, false);
    }

    /**
     * @tc.name: ExitCoordination01
     * @tc.desc: test function :ExitCoordination
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, ExitCoordination01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = new FoldScreenPolicy();
        fsc_.ExitCoordination();

        ASSERT_NE(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: ExitCoordination02
     * @tc.desc: test function :ExitCoordination
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, ExitCoordination02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        FoldScreenController fsc_(mutex, std::shared_ptr<TaskScheduler>());

        fsc_.foldScreenPolicy_ = nullptr;
        fsc_.ExitCoordination();

        ASSERT_EQ(fsc_.foldScreenPolicy_, nullptr);
    }

    /**
     * @tc.name: GetTentMode
     * @tc.desc: test function :GetTentMode
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetTentMode, TestSize.Level1)
    {
        if (!ssm_.IsFoldable() || FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            GTEST_SKIP();
        }
        ASSERT_NE(ssm_.foldScreenController_, nullptr);
            auto tentMode = ssm_.foldScreenController_->GetTentMode();
            ASSERT_EQ(tentMode, false);
    }

    /**
     * @tc.name: OnTentModeChanged01
     * @tc.desc: test function :OnTentModeChanged
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, OnTentModeChanged01, TestSize.Level1)
    {
        if (!FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()||
            !FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            GTEST_SKIP();
        }
        bool isTentMode = false;
        ssm_.foldScreenController_->OnTentModeChanged(isTentMode);
        ASSERT_EQ(ssm_.foldScreenController_->GetTentMode(), false);
    }

    /**
     * @tc.name: OnTentModeChanged02
     * @tc.desc: test function :OnTentModeChanged
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, OnTentModeChanged02, TestSize.Level1)
    {
        if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
            return;
        }
        if (ssm_.IsFoldable()) {
            bool isTentMode = true;
            ssm_.foldScreenController_->OnTentModeChanged(isTentMode);
            ASSERT_EQ(ssm_.foldScreenController_->GetTentMode(), true);
        }
    }

    /**
     * @tc.name: GetModeMatchStatus_ShouldReturnUnknow_WhenFoldScreenPolicyNull
     * @tc.desc: Test GetModeMatchStatu method when foldScreenPolicy_ is null
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetModeMatchStatus_ShouldReturnUnknow_WhenFoldScreenPolicyNull,
        TestSize.Level1)
    {
        //Arrange
        std::recursive_mutex displayInfoMutex;
        auto screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController foldScreenController(displayInfoMutex, screenPowerTaskScheduler);
        foldScreenController.foldScreenPolicy_ = nullptr;

        //Act
        auto result = foldScreenController.GetModeMatchStatus();

        //Assert
        EXPECT_EQ(result, FoldDisplayMode::UNKNOWN);
    }

    /**
     * @tc.name: GetModeMatchStatus_ShouldReturnMode_WhenFoldScreenPolicyIsNoNull
     * @tc.desc: Test GetModeMatchStatu method when foldScreenPolicy_ is not null
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetModeMatchStatus_ShouldReturnUnknow_WhenFoldScreenPolicyIsNotNull,
        TestSize.Level1)
    {
        //Arrange
        std::recursive_mutex displayInfoMutex;
        auto screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController foldScreenController(displayInfoMutex, screenPowerTaskScheduler);

        //Set a mock foldScreenPolicy_
        auto foldScreenPolicy = sptr<FoldScreenPolicy>::MakeSptr();
        foldScreenController.foldScreenPolicy_ = foldScreenPolicy;

        //Act
        auto result = foldScreenController.GetModeMatchStatus();

        //Assert
        EXPECT_EQ(result, foldScreenPolicy->GetModeMatchStatus());
    }

    /**
     * @tc.name: GetScreenSnapshotRect_NullPolicy
     * @tc.desc: test function :GetScreenSnapshotRect
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetScreenSnapshotRect_NullPolicy, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        auto screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController controller(mutex, screenPowerTaskScheduler);
        controller.foldScreenPolicy_ = nullptr;

        Drawing::Rect expectedRect = {0, 0, 0, 0};
        Drawing::Rect actualRect = controller.GetScreenSnapshotRect();

        EXPECT_EQ(expectedRect.left_, actualRect.left_);
        EXPECT_EQ(expectedRect.top_, actualRect.top_);
        EXPECT_EQ(expectedRect.right_, actualRect.right_);
        EXPECT_EQ(expectedRect.bottom_, actualRect.bottom_);
    }

    /**
     * @tc.name: GetScreenSnapshotRect_ValidPolicy
     * @tc.desc: test function :GetScreenSnapshotRect
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetScreenSnapshotRect_ValidPolicy, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        auto screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController controller(mutex, screenPowerTaskScheduler);
        auto mockPolicy = sptr<FoldScreenPolicy>::MakeSptr();
        controller.foldScreenPolicy_ = mockPolicy;
        Drawing::Rect expectedRect = {0, 0, 0, 0};
        Drawing::Rect actualRect = controller.GetScreenSnapshotRect();
        EXPECT_EQ(expectedRect.left_, actualRect.left_);
        EXPECT_EQ(expectedRect.top_, actualRect.top_);
        EXPECT_EQ(expectedRect.right_, actualRect.right_);
        EXPECT_EQ(expectedRect.bottom_, actualRect.bottom_);
    }

    /**
     * @tc.name: SetMainScreenRegion_ShouldSetMainScreenRegion_WhenFoldScreenPolicyIsNotNull
     * @tc.desc: Test SetMainScreenRegion method when foldScreenPolicy_ is not null
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetMainScreenRegion_ShouldSetMainScreenRegion_WhenFoldScreenPolicyIsNotNull,
        TestSize.Level1)
    {
        //Arrange
        LOG_SetCallback(MyLogCallback);
        std::recursive_mutex displayInfoMutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController foldScreenController(displayInfoMutex, screenPowerTaskScheduler);
        DMRect mainScreenRegion = {0, 0, 1920, 1080};
        foldScreenController.foldScreenPolicy_ = nullptr;

        //Act
        foldScreenController.SetMainScreenRegion(mainScreenRegion);

        //Assert
        EXPECT_TRUE(g_errLog.find("foldScreenPolicy_ is null") != std::string::npos);
    }

    /**
     * @tc.name: SetMainScreenRegion_ShouldSetMainScreenRegion_WhenFoldScreenPolicyIsNotNull
     * @tc.desc: Test SetMainScreenRegion method when foldScreenPolicy_ is null
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetMainScreenRegion_ShouldSetMainScreenRegion_WhenFoldScreenPolicyIsNull,
        TestSize.Level1)
    {
        //Arrange
        LOG_SetCallback(MyLogCallback);
        std::recursive_mutex displayInfoMutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController foldScreenController(displayInfoMutex, screenPowerTaskScheduler);
        DMRect mainScreenRegion = {0, 0, 1920, 1080};
        auto mockPolicy = sptr<FoldScreenPolicy>::MakeSptr();
        foldScreenController.foldScreenPolicy_ = mockPolicy;

        //Act
        foldScreenController.SetMainScreenRegion(mainScreenRegion);

        //Assert
        EXPECT_TRUE(g_errLog.find("foldScreenPolicy_ is null") == std::string::npos);
    }

    /**
     * @tc.name: GetTentMode_ShouldReturnFalse_WhenSensorFoldStateManagerIsNull
     * @tc.desc: Test GetTentMode function when sensorFoldStateManager_ is null
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetTentMode_ShouldReturnFalse_WhenSensorFoldStateManagerIsNull,
        TestSize.Level1)
    {
        std::recursive_mutex displayInfoMutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController foldScreenController(displayInfoMutex, screenPowerTaskScheduler);
        foldScreenController.sensorFoldStateManager_ = nullptr;
        EXPECT_FALSE(foldScreenController.GetTentMode());
    }

    /**
     * @tc.name: GetTentMode_ShouldReturnFalse_WhenSensorFoldStateManagerIsNotNull
     * @tc.desc: Test GetTentMode function when sensorFoldStateManager_ is not null
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, GetTentMode_ShouldReturnFalse_WhenSensorFoldStateManagerIsNotNull,
        TestSize.Level1)
    {
        std::recursive_mutex displayInfoMutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController foldScreenController(displayInfoMutex, screenPowerTaskScheduler);
        foldScreenController.sensorFoldStateManager_ = new SensorFoldStateManager();
        foldScreenController.sensorFoldStateManager_->SetTentMode(1);
        EXPECT_TRUE(foldScreenController.GetTentMode());
    }

    /**
     * @tc.name: SetIsClearingBootAnimation
     * @tc.desc: test function :SetIsClearingBootAnimation
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, SetIsClearingBootAnimation, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        auto fsc_ = sptr<FoldScreenController>::MakeSptr(mutex, std::shared_ptr<TaskScheduler>());
        fsc_->foldScreenPolicy_ = nullptr;
        fsc_->SetIsClearingBootAnimation(false);
        fsc_->foldScreenPolicy_ = fsc_->GetFoldScreenPolicy(DisplayDeviceType::SINGLE_DISPLAY_DEVICE);
        fsc_->SetIsClearingBootAnimation(false);
        EXPECT_FALSE(fsc_->foldScreenPolicy_->isClearingBootAnimation_);
        fsc_->SetIsClearingBootAnimation(true);
        EXPECT_TRUE(fsc_->foldScreenPolicy_->isClearingBootAnimation_);
    }

    /**
     * @tc.name: NotifyRunSensorFoldStateManager
     * @tc.desc: test function :NotifyRunSensorFoldStateManager
     * @tc.type: FUNC
     */
    HWTEST_F(FoldScreenControllerTest, NotifyRunSensorFoldStateManager, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::shared_ptr<TaskScheduler>();
        FoldScreenController fsc_(mutex, screenPowerTaskScheduler);
        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        fsc_.NotifyRunSensorFoldStateManager();
        EXPECT_TRUE(g_errLog.find("TaskSequenceProcess") != std::string::npos);
        LOG_SetCallback(nullptr);
        g_errLog.clear();
    }
}
} // namespace Rosen
} // namespace OHOS

