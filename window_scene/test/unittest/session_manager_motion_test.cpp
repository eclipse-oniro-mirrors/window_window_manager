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

#include <gtest/gtest.h>
#include "parameters.h"
#include "session_manager.h"
#include "motion_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class SessionManagerMotionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SessionManager> sm_ = nullptr;
    int32_t userId_ = 100;
    std::string isConcurrentuser_;
};

void SessionManagerMotionTest::SetUpTestCase()
{
}

void SessionManagerMotionTest::TearDownTestCase()
{
}

void SessionManagerMotionTest::SetUp()
{
    MotionManager::GetInstance().Reset();
    isConcurrentuser_ = OHOS::system::GetParameter("persist.dms.concurrentuser", "");
    OHOS::system::SetParameter("persist.dms.concurrentuser", "true");
    sm_ = &SessionManager::GetInstance(userId_);
}

void SessionManagerMotionTest::TearDown()
{
    SessionManager::sessionManagerMap_.clear();
    OHOS::system::SetParameter("persist.dms.concurrentuser", isConcurrentuser_);
    usleep(SLEEP_TIME_US);
}

HWTEST_F(SessionManagerMotionTest, RegisterMotionSensor01, TestSize.Level1)
{
    ASSERT_NE(sm_, nullptr);
    bool ret = sm_->RegisterMotionSensor(static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    EXPECT_TRUE(ret);
}

HWTEST_F(SessionManagerMotionTest, RegisterMotionSensor02, TestSize.Level1)
{
    ASSERT_NE(sm_, nullptr);
    bool ret = sm_->RegisterMotionSensor(static_cast<int32_t>(MotionType::SMART_MOTION_TYPE));
    EXPECT_TRUE(ret);
}

HWTEST_F(SessionManagerMotionTest, RegisterMotionSensor03, TestSize.Level1)
{
    ASSERT_NE(sm_, nullptr);
    bool ret = sm_->RegisterMotionSensor(static_cast<int32_t>(MotionType::SMART_MOTION_ENHANCE_TYPE));
    EXPECT_TRUE(ret);
}

HWTEST_F(SessionManagerMotionTest, UnregisterMotionSensor01, TestSize.Level1)
{
    ASSERT_NE(sm_, nullptr);
    bool ret = sm_->UnregisterMotionSensor(static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    EXPECT_TRUE(ret);
}

HWTEST_F(SessionManagerMotionTest, UnregisterMotionSensor02, TestSize.Level1)
{
    ASSERT_NE(sm_, nullptr);
    bool ret = sm_->UnregisterMotionSensor(static_cast<int32_t>(MotionType::SMART_MOTION_TYPE));
    EXPECT_TRUE(ret);
}

}
}
