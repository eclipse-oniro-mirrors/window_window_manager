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

#include "bundle_info_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}

class BundleInfoHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void BundleInfoHelperTest::SetUpTestCase()
{
}

void BundleInfoHelperTest::TearDownTestCase()
{
}

void BundleInfoHelperTest::SetUp()
{
}

void BundleInfoHelperTest::TearDown()
{
    g_errLog.clear();
}

namespace {

/**
 * @tc.name: GetCurBundleNameTest
 * @tc.desc: GetCurBundleNameTest
 * @tc.type: FUNC
 */
HWTEST_F(BundleInfoHelperTest, GetCurBundleNameTest, TestSize.Level1)
{
    std::shared_ptr<BundleInfo> bundleInfoPtr = std::make_shared<BundleInfo>();
    bundleInfoPtr->name_ = "test";
    SysCapUtil::pidBundleInfoMap_[0] = bundleInfoPtr;
    BundleInfoHelper::GetCurBundleName();
    BundleInfoHelper::GetBundleNameByPid(1);
    EXPECT_TRUE(SysCapUtil::GetBundleInfo(0) != nullptr);
    SysCapUtil::pidBundleInfoMap_.clear();
}

/**
 * @tc.name: GetBundleNameByPidTest
 * @tc.desc: GetBundleNameByPidTest
 * @tc.type: FUNC
 */
HWTEST_F(BundleInfoHelperTest, GetBundleNameByPidTest, TestSize.Level1)
{
    std::shared_ptr<BundleInfo> bundleInfoPtr = std::make_shared<BundleInfo>();
    bundleInfoPtr->name_ = "test";
    SysCapUtil::pidBundleInfoMap_[0] = bundleInfoPtr;
    BundleInfoHelper::GetBundleNameByPid(0);
    BundleInfoHelper::GetBundleNameByPid(1);
    EXPECT_TRUE(SysCapUtil::GetBundleInfo(0) != nullptr);
    SysCapUtil::pidBundleInfoMap_.clear();
}
}
} // namespace Rosen
} // namespace OHOS