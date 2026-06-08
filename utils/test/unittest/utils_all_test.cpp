/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <iremote_broker.h>
#include <iremote_object.h>

#include "agent_death_recipient.h"
#include "color_parser.h"
#include "iremote_object_mocker.h"
#include "parameters.h"
#include "perform_reporter.h"
#include "singleton_container.h"
#include "surface_reader_handler_impl.h"
#include "sys_cap_util.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class TestClass {
public:
    std::string name = "testClass";
};

class UtilsAllTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::map<std::string, int32_t> oldStringMap_;
    std::map<int32_t, SingletonContainer::Singleton> oldSingletonMap_;
    std::map<int32_t, std::set<int32_t>> oldDependencySetMap_;
    static std::string isConcurrentuser_;
};

std::string UtilsAllTest::isConcurrentuser_;

void UtilsAllTest::SetUpTestCase()
{
    isConcurrentuser_ = OHOS::system::GetParameter("persist.dms.concurrentuser", "");
    OHOS::system::SetParameter("persist.dms.concurrentuser", "true");
}

void UtilsAllTest::TearDownTestCase()
{
    OHOS::system::SetParameter("persist.dms.concurrentuser", isConcurrentuser_);
}

void UtilsAllTest::SetUp() {}

void UtilsAllTest::TearDown() {}

namespace {
/**
 * @tc.name: ADROnRemoteDied01
 * @tc.desc: test AgentDeathRecipient::OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, ADROnRemoteDied01, TestSize.Level1)
{
    sptr<AgentDeathRecipient> deathRecipient = new AgentDeathRecipient(nullptr);

    deathRecipient->OnRemoteDied(nullptr);

    sptr<MockIRemoteObject> remoteObj = new MockIRemoteObject();
    deathRecipient->OnRemoteDied(remoteObj);
    ASSERT_EQ(0, remoteObj->count_);

    deathRecipient->callback_ = [&remoteObj](sptr<IRemoteObject>& remote) { remoteObj->count_ = 1; };
    deathRecipient->OnRemoteDied(remoteObj);
    ASSERT_EQ(1, remoteObj->count_);
}

/**
 * @tc.name: PRCount01
 * @tc.desc: test PerformReporter::count
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, PRCount01, TestSize.Level1)
{
    std::vector<int64_t> timeSpiltsMs = { 0, 1, 2 };
    PerformReporter reporter = PerformReporter("test", timeSpiltsMs);

    reporter.count(0);
    ASSERT_EQ(1, reporter.totalCount_);
    reporter.timeSplitCount_.clear();
    ASSERT_EQ(1, reporter.totalCount_);
}

/**
 * @tc.name: SCAddSingleton01
 * @tc.desc: test SingletonContainer::AddSingleton
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, SCAddSingleton01, TestSize.Level1)
{
    auto& singletonContainer = SingletonContainer::GetInstance();

    singletonContainer.AddSingleton("test", nullptr);
    auto testId = singletonContainer.stringMap["test"];
    singletonContainer.AddSingleton("test", nullptr);
    singletonContainer.AddSingleton("test2", nullptr);
    ASSERT_EQ(testId + 1, singletonContainer.stringMap["test2"]);

    auto testId2 = singletonContainer.stringMap["test2"];
    singletonContainer.singletonMap.erase(testId);
    singletonContainer.singletonMap.erase(testId2);
    singletonContainer.stringMap.erase("test");
    singletonContainer.stringMap.erase("test2");
}

/**
 * @tc.name: SCSetSingleton01
 * @tc.desc: test SingletonContainer::AddSingleton
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, SCSetSingleton01, TestSize.Level1)
{
    auto& singletonContainer = SingletonContainer::GetInstance();

    TestClass* testObj = new TestClass();

    singletonContainer.SetSingleton("test", testObj);
    auto testId = singletonContainer.stringMap["test"];
    auto instance = singletonContainer.GetSingleton("test2");
    ASSERT_EQ(instance, nullptr);

    instance = singletonContainer.GetSingleton("test");
    ASSERT_NE(instance, nullptr);
    ASSERT_EQ(static_cast<TestClass*>(instance)->name, "testClass");

    singletonContainer.SetSingleton("test", nullptr);
    instance = singletonContainer.GetSingleton("test");
    ASSERT_EQ(instance, nullptr);

    singletonContainer.singletonMap.erase(testId);
    singletonContainer.stringMap.erase("test");
    delete testObj;
}

/**
 * @tc.name: SCDependOn01
 * @tc.desc: test SingletonContainer::DependOn
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, SCDependOn01, TestSize.Level1)
{
    auto& singletonContainer = SingletonContainer::GetInstance();

    singletonContainer.AddSingleton("test", nullptr);

    ASSERT_EQ(nullptr, singletonContainer.DependOn("test", "test"));

    auto id = singletonContainer.stringMap["test"];
    auto& testSet = singletonContainer.dependencySetMap[id];
    ASSERT_EQ(1, testSet.size());

    ASSERT_EQ(nullptr, singletonContainer.DependOn("test", "test"));
    id = singletonContainer.stringMap["test"];
    auto& testSet2 = singletonContainer.dependencySetMap[id];
    ASSERT_EQ(1, testSet2.size());

    singletonContainer.singletonMap.erase(id);
    singletonContainer.stringMap.erase("test");
    id = singletonContainer.dependencySetMap.erase(id);
}

/**
 * @tc.name: SRHOnImageAvailable01
 * @tc.desc: test SurfaceReaderHandlerImpl::OnImageAvailable
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, SRHOnImageAvailable, TestSize.Level1)
{
    sptr<SurfaceReaderHandlerImpl> surfaceReaderHandlerImpl = new (std::nothrow) SurfaceReaderHandlerImpl();
    surfaceReaderHandlerImpl->flag_ = false;
    surfaceReaderHandlerImpl->OnImageAvailable(nullptr);
    ASSERT_EQ(true, surfaceReaderHandlerImpl->flag_);
    surfaceReaderHandlerImpl->flag_ = true;
    surfaceReaderHandlerImpl->OnImageAvailable(nullptr);
    ASSERT_EQ(true, surfaceReaderHandlerImpl->flag_);
}

/**
 * @tc.name: SRHGetPixelMap01
 * @tc.desc: test SurfaceReaderHandlerImpl::GetPixelMap
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, SRHGetPixelMap, TestSize.Level1)
{
    sptr<SurfaceReaderHandlerImpl> surfaceReaderHandlerImpl = new (std::nothrow) SurfaceReaderHandlerImpl();
    surfaceReaderHandlerImpl->flag_ = false;
    surfaceReaderHandlerImpl->GetPixelMap();
    ASSERT_EQ(false, surfaceReaderHandlerImpl->flag_);
    surfaceReaderHandlerImpl->flag_ = true;
    surfaceReaderHandlerImpl->GetPixelMap();
}

/**
 * @tc.name: SysCapUtilGetClientName
 * @tc.desc: test SysCapUtil::GetClientName
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, SysCapUtilGetClientName, TestSize.Level1)
{
    ASSERT_NE("", SysCapUtil::GetClientName());
}

/**
 * @tc.name: ConvertErrorToCode
 * @tc.desc: test ConvertErrorToCode
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, ConvertErrorToCode, TestSize.Level1)
{
    EXPECT_EQ(ConvertErrorToCode(WMError::WM_OK), WmErrorCode::WM_OK);
    EXPECT_EQ(ConvertErrorToCode(WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED),
        WmErrorCode::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED);
    WMError error = static_cast<WMError>(-1);
    EXPECT_EQ(ConvertErrorToCode(error), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
}

/**
 * @tc.name: IsMultiInstanceEnabled01
 * @tc.desc: test IsMultiInstanceEnabled when concurrentuser is true
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, IsMultiInstanceEnabled01, TestSize.Level1)
{
    EXPECT_TRUE(IsMultiInstanceEnabled());
}

/**
 * @tc.name: IsValidColorNoAlpha
 * @tc.desc: test IsValidColorNoAlpha
 * @tc.type: FUNC
 */
HWTEST_F(UtilsAllTest, IsValidColorNoAlpha, TestSize.Level1)
{
    EXPECT_TRUE(ColorParser::IsValidColorNoAlpha("#008EF5"));
    EXPECT_TRUE(ColorParser::IsValidColorNoAlpha("#FF008EF5"));
    EXPECT_FALSE(ColorParser::IsValidColorNoAlpha("InvalidColor"));
    EXPECT_FALSE(ColorParser::IsValidColorNoAlpha("#009HG5"));
    EXPECT_FALSE(ColorParser::IsValidColorNoAlpha("##009FF5"));
    EXPECT_FALSE(ColorParser::IsValidColorNoAlpha("#08EF5"));
    EXPECT_FALSE(ColorParser::IsValidColorNoAlpha("#80008EF5"));
}
} // namespace
} // namespace Rosen
} // namespace OHOS