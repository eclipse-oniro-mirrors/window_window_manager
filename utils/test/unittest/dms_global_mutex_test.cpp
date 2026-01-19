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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "dms_global_mutex.h"

#include <functional>
#include <thread>
#include <vector>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Rosen::DmUtils;

namespace OHOS {
namespace Rosen {
namespace DmUtils {
namespace {

class DmsGlobalMutexTest : public Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: DmsGlobalMutexTest
 * @tc.desc: DmsGlobalMutexTest test
 * @tc.type: FUNC
 */
HWTEST_F(DmsGlobalMutexTest, HoldLock_MultiThread_Counter, TestSize.Level1)
{
    std::atomic<int> counter = 0;
    constexpr int n = 5;
    std::vector<std::thread> threads;

    for (int i = 0; i < n; ++i) {
        threads.emplace_back([&] {
            HoldLock lock;
            ++counter;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter.load(), n);
    EXPECT_TRUE(HoldLock::resMtx.try_lock());
    HoldLock::resMtx.unlock();
}

/**
 * @tc.name: DmsGlobalMutexTest
 * @tc.desc: DmsGlobalMutexTest test
 * @tc.type: FUNC
 */
HWTEST_F(DmsGlobalMutexTest, VipPriorityEvent_LockTime, TestSize.Level1)
{
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
    auto threadFunc = []() {
        DmUtils::HoldLock holdLock(IPCPriority::VIP);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    };
    std::thread t1(threadFunc);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    DmUtils::HoldLock holdLock(IPCPriority::VIP);
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
    t1.join();
}

/**
 * @tc.name: DmsGlobalMutexTest
 * @tc.desc: DmsGlobalMutexTest test
 * @tc.type: FUNC
 */
HWTEST_F(DmsGlobalMutexTest, DropLock_NoLock, TestSize.Level1)
{
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
    {
        DmUtils::DropLock dropLock;
        EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
    }

    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
}

/**
 * @tc.name: DmsGlobalMutexTest
 * @tc.desc: DmsGlobalMutexTest test
 * @tc.type: FUNC
 */
HWTEST_F(DmsGlobalMutexTest, DropLock_MultiThread, TestSize.Level1)
{
    std::mutex printMtx;
    std::atomic<bool> flag{ false };

    auto threadFunc = [&printMtx, &flag]() {
        DmUtils::HoldLock holdLock;
        {
            DmUtils::DropLock dropLock;
            std::lock_guard<std::mutex> lck(printMtx);
            flag = true;
        }
    };

    std::thread t1(threadFunc);
    std::thread t2(threadFunc);

    t1.join();
    t2.join();

    EXPECT_TRUE(flag.load());
}

/**
 * @tc.name: DmsGlobalMutexTest
 * @tc.desc: DmsGlobalMutexTest test
 * @tc.type: FUNC
 */
HWTEST_F(DmsGlobalMutexTest, DropLock_SafeWaitFor, TestSize.Level1)
{
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> ready{ false };

    std::thread t([&mtx, &cv, &ready] {
        std::unique_lock<std::mutex> lock(mtx);
        DmUtils::safe_wait_for(cv, lock, std::chrono::seconds(1), [&] { return ready.load(); });
        EXPECT_TRUE(ready.load());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ready = true;
    cv.notify_one();
    t.join();
}

} // namespace
} // namespace DmUtils
} // namespace Rosen
} // namespace OHOS