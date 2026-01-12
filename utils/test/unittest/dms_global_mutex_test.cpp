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

/** @tc.name: HoldLock_MultiThread_Counter

@tc.desc: 多线程中 HoldLock 应正确计数，锁只在最后一个释放时才释放

@tc.type: FUNC

@tc.level: Level1 */
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
    // 验证锁在所有线程结束后才释放
    EXPECT_TRUE(HoldLock::resMtx.try_lock());
    HoldLock::resMtx.unlock();
}

/** @tc.name: VipPriorityEvent_LockTime

@tc.desc: 该测试用例验证在Vip优先级场景下，被锁阻塞时等锁时长是否符合预期。

@tc.type: FUNC

@tc.level: Level1 */
HWTEST_F(DmsGlobalMutexTest, VipPriorityEvent_LockTime, TestSize.Level1)
{
    // 当前没有锁
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);

    auto threadFunc = []() {
        DmUtils::HoldLock holdLock(IPCPriority::VIP);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    };
    std::thread t1(threadFunc);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // 当前线程vip任务trylock失败
    DmUtils::HoldLock holdLock(IPCPriority::VIP);
    // trylock失败，lockStatus状态不变
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
    t1.join();
}

/** @tc.name: DropLock_NoLock

@tc.desc: 该测试用例验证在无锁状态下，DropLock 对象的构造和析构不会改变锁的状态（即不产生副作用）。

@tc.type: FUNC

@tc.level: Level1 */
HWTEST_F(DmsGlobalMutexTest, DropLock_NoLock, TestSize.Level1)
{
    // 当前没有锁
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);

    {
        DmUtils::DropLock dropLock;
        EXPECT_FALSE(DmUtils::HoldLock::lockStatus); // 未持有锁，不会改变
    }

    // 退出作用域后，状态不变
    EXPECT_FALSE(DmUtils::HoldLock::lockStatus);
}

/** @tc.name: DropLock_MultiThread

@tc.desc: 测试多线程下DropLock临时释放锁并成功设置标志

@tc.type: FUNC

@tc.level: Level1 */
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

/** @tc.name: DropLock_MultiThread

@tc.desc: 测试safe_wait_for在等待时临时释放锁并正确响应条件变量信号。

@tc.type: FUNC

@tc.level: Level1 */
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