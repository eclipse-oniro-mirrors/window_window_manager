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
#include "want.h"
#include "session_info.h"
#include "collaborator_dll_manager.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace Rosen {
 
class CollaboratorDllManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
 
void CollaboratorDllManagerTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup test case";
}
 
void CollaboratorDllManagerTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown test case";
}
 
void CollaboratorDllManagerTest::SetUp()
{
    GTEST_LOG_(INFO) << "setup";
}
 
void CollaboratorDllManagerTest::TearDown()
{
    GTEST_LOG_(INFO) << "teardown";
}
constexpr const char* URI_CHECK_SO_NAME = "libams_broker_ext.z.so";
constexpr const char* URI_CHECK_FUNC_NAME = "PreHandleStartAbility";
 
namespace {
/*
 * Feature: InitDlSymbol
 * Function: InitDlSymbol
 * SubFunction: NA
 * FunctionPoints: InitDlSymbol
 */
HWTEST_F(CollaboratorDllManagerTest, InitDlSymbol_001, TestSize.Level1)
{
    CollaboratorDllWrapper wrapper;
    const char* soName = URI_CHECK_SO_NAME;
    const char* funcName = URI_CHECK_FUNC_NAME;
    bool ret = wrapper.InitDlSymbol(soName, funcName);
    
    soName = "libams_broker_ext.z.so";
    funcName = "testFunc";
    ret = wrapper.InitDlSymbol(soName, funcName);
 
    soName = nullptr;
    funcName = "testFunc";
    ret = wrapper.InitDlSymbol(soName, funcName);
    EXPECT_EQ(ret, true);
}
 
/*
 * Feature: GetFunc
 * Function: GetFunc
 * SubFunction: NA
 * FunctionPoints: GetFunc
 */
HWTEST_F(CollaboratorDllManagerTest, GetFunc_001, TestSize.Level1)
{
    CollaboratorDllWrapper wrapper;
    auto ret = wrapper.GetPreHandleStartAbilityFunc();
    EXPECT_EQ(ret, nullptr);
}
 
/*
 * Feature: GetFunc
 * Function: GetFunc
 * SubFunction: NA
 * FunctionPoints: GetFunc
 */
HWTEST_F(CollaboratorDllManagerTest, PreHandleStartAbility_001, TestSize.Level1)
{
    sptr<AAFwk::SessionInfo> info = sptr<AAFwk::SessionInfo>::MakeSptr();
    std::shared_ptr<AAFwk::Want> tmpWant = std::make_shared<AAFwk::Want>();
    CollaboratorDllManager::PreHandleStartAbility(*tmpWant, info, 0);
    EXPECT_EQ(info->callerTypeForAnco, 0);
}
}
}
}