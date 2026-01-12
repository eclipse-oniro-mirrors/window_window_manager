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

#include "multi_screen_manager.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "common_test_utils.h"
#include "test_client.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
class MultiScreenManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<ScreenSessionManagerClientTest> testClient_;
    void SetAceessTokenPermission(const std::string processName);
};

void MultiScreenManagerTest::SetUpTestCase()
{
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void MultiScreenManagerTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void MultiScreenManagerTest::SetUp()
{
}

void MultiScreenManagerTest::TearDown()
{
}

namespace {

/**
 * @tc.name: FilterPhysicalAndVirtualScreen
 * @tc.desc: FilterPhysicalAndVirtualScreen func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, FilterPhysicalAndVirtualScreen, TestSize.Level1)
{
    std::vector<ScreenId> allScreenIds = {2000, 2001, 0};
    std::vector<ScreenId> physicalScreenIds;
    std::vector<ScreenId> virtualScreenIds;
    MultiScreenManager::GetInstance().FilterPhysicalAndVirtualScreen(allScreenIds,
        physicalScreenIds, virtualScreenIds);
    EXPECT_EQ(virtualScreenIds.size(), 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: VirtualScreenMirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch01, TestSize.Level1)
{
    ScreenId mainScreenId = 1;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, DMRect::NONE(),
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(screenGroupId, 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: mainScreen is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch02, TestSize.Level1)
{
    ScreenId mainScreenId = 1002;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[mainScreenId] = session;
    auto ret = MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds,
        DMRect::NONE(), screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: ret != DMError::DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch03, TestSize.Level1)
{
    ScreenId mainScreenId = 1002;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[mainScreenId] = session;
    ScreenId rsId = 1002;
    std::string name = "ok";
    ScreenCombination combination =  ScreenCombination::SCREEN_ALONE;
    sptr<ScreenSessionGroup> sessiongroup = new ScreenSessionGroup(mainScreenId, rsId, name, combination);
    ScreenSessionManager::GetInstance().smsScreenGroupMap_[mainScreenId] = sessiongroup;
    auto ret = MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, DMRect::NONE(),
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch04
 * @tc.desc: VirtualScreenMirrorSwitch func04
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch04, TestSize.Level1)
{
    ScreenId mainScreenId = 1003;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, DMRect::NONE(),
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(screenGroupId, 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch05
 * @tc.desc: VirtualScreenMirrorSwitch func05
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch05, TestSize.Level1)
{
    ScreenId mainScreenId = 101;
    std::vector<ScreenId> ScreenIds = {1, 2};
    ScreenId screenGroupId;
    MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, DMRect::NONE(),
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(screenGroupId, 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch06
 * @tc.desc: VirtualScreenMirrorSwitch func06
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch06, TestSize.Level1)
{
    ScreenId mainScreenId = 102;
    std::vector<ScreenId> ScreenIds = {1, 3};
    ScreenId screenGroupId;
    MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, DMRect::NONE(),
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(screenGroupId, 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: VirtualScreenMirrorSwitch func DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch07, TestSize.Level1)
{
    ScreenId mainScreenId = 1;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    DMRect myRect = {0, 0, 400, 600};
    MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, myRect,
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(screenGroupId, 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: mainScreen is not nullptr and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch08, TestSize.Level1)
{
    ScreenId mainScreenId = 1002;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    sptr<ScreenSession> session = new ScreenSession();
    DMRect myRect = {0, 0, 400, 600};
    ScreenSessionManager::GetInstance().screenSessionMap_[mainScreenId] = session;
    auto ret = MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds,
        myRect, screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: ret != DMError::DM_OK and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch09, TestSize.Level1)
{
    ScreenId mainScreenId = 1002;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[mainScreenId] = session;
    ScreenId rsId = 1002;
    std::string name = "ok";
    DMRect myRect = {0, 0, 400, 600};
    ScreenCombination combination =  ScreenCombination::SCREEN_ALONE;
    sptr<ScreenSessionGroup> sessiongroup = new ScreenSessionGroup(mainScreenId, rsId, name, combination);
    ScreenSessionManager::GetInstance().smsScreenGroupMap_[mainScreenId] = sessiongroup;
    auto ret = MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, myRect,
        screenGroupId, {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch
 * @tc.desc: PhysicalScreenMirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch01, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, DMRect::NONE(),
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch
 * @tc.desc: defaultSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch02, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, DMRect::NONE(),
        {Rotation::ROTATION_0, true});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch
 * @tc.desc: defaultSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch03, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[1002] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, DMRect::NONE(),
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
    ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, DMRect::NONE(),
        {Rotation::ROTATION_0, false}, true);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch04
 * @tc.desc: defaultSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch04, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {101, 102};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, DMRect::NONE(),
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch05
 * @tc.desc: defaultSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch05, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {12, 13};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[12] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, DMRect::NONE(),
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch06
 * @tc.desc: and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch06, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    DMRect myRect = {0, 0, 400, 600};
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, myRect,
        {Rotation::ROTATION_0, false});
    EXPECT_NE(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch07
 * @tc.desc: defaultSession != nullptr and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch07, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    DMRect myRect = {0, 0, 400, 600};
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, myRect,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch08
 * @tc.desc: defaultSession != nullptr and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch08, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    DMRect myRect = {0, 0, 400, 600};
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[1002] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, myRect,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch09
 * @tc.desc: defaultSession != nullptr and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch09, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {101, 102};
    sptr<ScreenSession> session = new ScreenSession();
    DMRect myRect = {0, 0, 400, 600};
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, myRect,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}


/**
 * @tc.name: PhysicalScreenMirrorSwitch10
 * @tc.desc: defaultSession != nullptr and DMRect not null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch10, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {12, 13};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    DMRect myRect = {0, 0, 400, 600};
    ScreenSessionManager::GetInstance().screenSessionMap_[12] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(0, screenIds, myRect,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}


/**
 * @tc.name: PhysicalScreenUniqueSwitch
 * @tc.desc: PhysicalScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch01
 * @tc.desc: PhysicalScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch01, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch02
 * @tc.desc: screenSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch02, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1002] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1003] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch03
 * @tc.desc: screenSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch03, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = nullptr;
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1002] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1003] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch04
 * @tc.desc: screenSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch04, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1002] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1003] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch_CustomScbScreenCreate
 * @tc.desc: screenSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch05, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    session->SetInnerName("CustomScbScreen");
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1002] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    session->SetInnerName("UNKOWN");
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1003] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch01
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch01, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =new ScreenSession();
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch02
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch02, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch03, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {1012, 1002};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch04, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {12, 1001};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch05, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {12, 1002};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch06, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {12, 1003};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch07, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {1003, 1003};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch08, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {1003, 1002};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch09, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {1003, 1001};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch010, TestSize.Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {1004, 1001};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch11
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch11, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = new ScreenSession();
    std::vector<ScreenId> screenIds = {1003, 11};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch12
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch12, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = new ScreenSession();
    std::vector<ScreenId> screenIds = {1004, 1001};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch13
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch13, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1002};
    sptr<ScreenSession> screenSession = new ScreenSession();
    screenSession->SetInnerName("CustomScbScreen");
    ScreenSessionManager::GetInstance().screenSessionMap_[
       1002] = screenSession;
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds,
        UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyScreenConnectCompletion01
 * @tc.desc: NotifyScreenConnectCompletion func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, NotifyScreenConnectCompletion01, TestSize.Level1)
{
    MultiScreenManager::GetInstance().uniqueScreenTimeoutMap_.clear();
    MultiScreenManager::GetInstance().uniqueScreenTimeoutMap_[1100] = false;
    MultiScreenManager::GetInstance().NotifyScreenConnectCompletion(1100);
    EXPECT_EQ(MultiScreenManager::GetInstance().uniqueScreenTimeoutMap_[1100], true);
}

/**
 * @tc.name: UniqueSwitch
 * @tc.desc: UniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, UniqueSwitch01, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {};
    std::vector<DisplayId> displayIds;
    DMError ret = MultiScreenManager::GetInstance().UniqueSwitch(screenIds, displayIds, UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: UniqueSwitch
 * @tc.desc: screenIds is not empty
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, UniqueSwitch02, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1001, 1002};
    std::vector<DisplayId> displayIds;
    DMError ret = MultiScreenManager::GetInstance().UniqueSwitch(screenIds, displayIds, UniqueScreenRotationOptions());
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: MirrorSwitch
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {};
    ScreenId screenGroupId = 0;
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, DMRect::NONE(), screenGroupId,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}


/**
 * @tc.name: MirrorSwitch01
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch01, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1001, 1002};
    ScreenId screenGroupId = 0;
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, DMRect::NONE(), screenGroupId,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: MirrorSwitch02
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch02, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    ScreenId screenGroupId = 0;
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, DMRect::NONE(), screenGroupId,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: MirrorSwitch03
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch03, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1003, 1002};
    ScreenId screenGroupId = 0;
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, DMRect::NONE(), screenGroupId,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: MirrorSwitch04
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch04, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {1003, 2};
    ScreenId screenGroupId = 0;
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, DMRect::NONE(), screenGroupId,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: MirrorSwitch
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch05, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {};
    ScreenId screenGroupId = 0;
    DMRect myRect = {0, 0, 400, 600};
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, myRect, screenGroupId,
        {Rotation::ROTATION_0, false});
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: MultiScreenModeChange
 * @tc.desc: firstSession == nullptr,secondarySession == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MultiScreenModeChange01, TestSize.Level1)
{
    sptr<ScreenSession> firstSession = nullptr;
    sptr<ScreenSession> secondarySession = nullptr;
    MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, "mirror");

    secondarySession = new ScreenSession();
    MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, "mirror");

    firstSession = new ScreenSession();
    MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, "mirror");
    EXPECT_NE(secondarySession, nullptr);
}

/**
 * @tc.name: MultiScreenModeChange
 * @tc.desc: firstSession == nullptr,secondarySession == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MultiScreenModeChange02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    testClient_ = new ScreenSessionManagerClientTest();
    ScreenSessionManager::GetInstance().SetClient(testClient_);
    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);

    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, "extend");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_EXTEND);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: MultiScreenModeChange
 * @tc.desc: firstSession == nullptr,secondarySession == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MultiScreenModeChange03, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    testClient_ = new ScreenSessionManagerClientTest();
    ScreenSessionManager::GetInstance().SetClient(testClient_);
    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);

    firstSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, "mirror");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMainChange
 * @tc.desc: scbClient null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMainChange01, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSessionManager::GetInstance().clientProxy_ = nullptr;
    MultiScreenManager::GetInstance().DoFirstMainChange(firstSession, secondarySession, "unknown");
    EXPECT_TRUE(g_logMsg.find("param error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMainChange
 * @tc.desc: param error
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMainChange02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    testClient_ = new ScreenSessionManagerClientTest();
    ScreenSessionManager::GetInstance().SetClient(testClient_);
    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MultiScreenManager::GetInstance().DoFirstMainChange(firstSession, secondarySession, "unknown");
    EXPECT_TRUE(g_logMsg.find("param error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMainChange
 * @tc.desc: DoFirstMainChangeExtend
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMainChange03, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);

    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);
    MultiScreenManager::GetInstance().DoFirstMainChange(firstSession, secondarySession, "extend");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_EXTEND);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMainChange
 * @tc.desc: main change extend, no need to change
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMainChange04, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_ALONE);

    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);
    MultiScreenManager::GetInstance().DoFirstMainChange(firstSession, secondarySession, "extend");
    ASSERT_NE(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_EXTEND);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMainChange
 * @tc.desc: DoFirstMainChangeMirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMainChange05, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);

    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);
    MultiScreenManager::GetInstance().DoFirstMainChange(firstSession, secondarySession, "mirror");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMainChange
 * @tc.desc: main change mirror, no need to change
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMainChange06, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_ALONE);

    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);
    MultiScreenManager::GetInstance().DoFirstMainChange(firstSession, secondarySession, "mirror");
    ASSERT_NE(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMirrorChange
 * @tc.desc: scbClient null
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMirrorChange01, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSessionManager::GetInstance().clientProxy_ = nullptr;
    MultiScreenManager::GetInstance().DoFirstMirrorChange(firstSession, secondarySession, "unknown");
    EXPECT_FALSE(g_logMsg.find("scbClient null") != std::string::npos ||
        g_logMsg.find("param error") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMirrorChange
 * @tc.desc: param error
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMirrorChange02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    testClient_ = new ScreenSessionManagerClientTest();
    ScreenSessionManager::GetInstance().SetClient(testClient_);
    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MultiScreenManager::GetInstance().DoFirstMirrorChange(firstSession, secondarySession, "unknown");
    EXPECT_TRUE(g_logMsg.find("scbClient null") == std::string::npos &&
        g_logMsg.find("aram error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMirrorChange
 * @tc.desc: DoFirstMirrorChangeExtend
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMirrorChange03, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);
    MultiScreenManager::GetInstance().DoFirstMirrorChange(firstSession, secondarySession, "extend");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_EXTEND);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstMirrorChange
 * @tc.desc: DoFirstMirrorChangeMirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstMirrorChange04, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    ASSERT_NE(ScreenSessionManager::GetInstance().GetClientProxy(), nullptr);
    MultiScreenManager::GetInstance().DoFirstMirrorChange(firstSession, secondarySession, "mirror");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstExtendChange
 * @tc.desc: param error
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstExtendChange01, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MultiScreenManager::GetInstance().DoFirstExtendChange(firstSession, secondarySession, "unknown");
    EXPECT_TRUE(g_logMsg.find("param error") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstExtendChange
 * @tc.desc: extend change extend
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstExtendChange02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    MultiScreenManager::GetInstance().DoFirstExtendChange(firstSession, secondarySession, "extend");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_EXTEND);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: DoFirstExtendChange
 * @tc.desc: extend change mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, DoFirstExtendChange03, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    auto firstSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption1, displayManagerAgent1->AsObject());
    auto secondarySession = ScreenSessionManager::GetInstance().GetScreenSession(screenId1);

    MultiScreenManager::GetInstance().DoFirstExtendChange(firstSession, secondarySession, "mirror");
    ASSERT_EQ(secondarySession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(screenId1);
}

/**
 * @tc.name: SetLastScreenMode
 * @tc.desc: SetLastScreenMode func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, SetLastScreenMode, TestSize.Level1)
{
    ScreenId mainScreenId = 0;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    ScreenId realId = MultiScreenManager::GetInstance().lastScreenMode_.first;
    MultiScreenMode realScreenMode = MultiScreenManager::GetInstance().lastScreenMode_.second;
    EXPECT_EQ(realId, mainScreenId);
    EXPECT_EQ(realScreenMode, secondaryScreenMode);
}

/**
 * @tc.name: InternalScreenOnChange01
 * @tc.desc: external mirror to internal mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOnChange01, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    ScreenId mainScreenId = internalId;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(true, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOnChange02
 * @tc.desc: no need to change or paramater error
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOnChange02, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    ScreenId mainScreenId = externalId;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);

    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOnChange03
 * @tc.desc: external mirror to internal extend
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOnChange03, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    ScreenId mainScreenId = internalId;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_EXTEND;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(true, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_EXTEND, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOnChange04
 * @tc.desc: external mirror to external extend
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOnChange04, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    ScreenId mainScreenId = externalId;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_EXTEND;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);

    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_EXTEND, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOnChange05
 * @tc.desc: mode not restored
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOnChange05, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    ScreenId mainScreenId = SCREEN_ID_INVALID;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);

    ScreenId realId = MultiScreenManager::GetInstance().lastScreenMode_.first;
    MultiScreenMode realScreenMode = MultiScreenManager::GetInstance().lastScreenMode_.second;
    EXPECT_EQ(realId, mainScreenId);
    EXPECT_EQ(realScreenMode, secondaryScreenMode);
    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOnChange06
 * @tc.desc: session nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOnChange06, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    sptr<ScreenSession> externalSession = nullptr;

    ScreenId mainScreenId = internalId;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
    }
}

/**
 * @tc.name: InternalScreenOffChange01
 * @tc.desc: internal mirror to external mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOffChange01, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetIsCurrentInUse(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(true);
    externalSession->SetIsCurrentInUse(true);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().InternalScreenOffChange(internalSession, externalSession);

    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOffChange02
 * @tc.desc: no need to change or paramater error
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOffChange02, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetIsCurrentInUse(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetIsCurrentInUse(true);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().InternalScreenOffChange(internalSession, externalSession);

    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOffChange03
 * @tc.desc: internal extend to external mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOffChange03, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetIsCurrentInUse(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(true);
    externalSession->SetIsCurrentInUse(true);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().InternalScreenOffChange(internalSession, externalSession);

    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOffChange04
 * @tc.desc: external extend to external mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOffChange04, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetIsCurrentInUse(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetIsCurrentInUse(true);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().InternalScreenOffChange(internalSession, externalSession);

    EXPECT_EQ(true, internalSession->GetIsExtend());
    EXPECT_EQ(false, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: InternalScreenOffChange05
 * @tc.desc: session nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, InternalScreenOffChange05, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    sptr<ScreenSession> externalSession = nullptr;

    ScreenId mainScreenId = internalId;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
    MultiScreenManager::GetInstance().InternalScreenOffChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
    }
}

/**
 * @tc.name: ExternalScreenDisconnectChange01
 * @tc.desc: paramater error
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, ExternalScreenDisconnectChange01, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(true);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().ExternalScreenDisconnectChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(true, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: ExternalScreenDisconnectChange02
 * @tc.desc: external mirror to internal mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, ExternalScreenDisconnectChange02, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().ExternalScreenDisconnectChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(true, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: ExternalScreenDisconnectChange03
 * @tc.desc: internal extend to internal mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, ExternalScreenDisconnectChange03, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(true);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().ExternalScreenDisconnectChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(true, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: ExternalScreenDisconnectChange04
 * @tc.desc: external extend to internal mirror
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, ExternalScreenDisconnectChange04, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(true);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    ScreenId externalId = 11;
    ScreenSessionConfig externalConfig = {
        .screenId = externalId,
    };
    sptr<ScreenSession> externalSession = new ScreenSession(externalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    externalSession->SetIsInternal(false);
    externalSession->SetIsExtend(false);
    externalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[externalId] = externalSession;
    }

    MultiScreenManager::GetInstance().ExternalScreenDisconnectChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(true, externalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    EXPECT_EQ(ScreenCombination::SCREEN_MIRROR, externalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(externalId);
    }
}

/**
 * @tc.name: ExternalScreenDisconnectChange05
 * @tc.desc: session nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, ExternalScreenDisconnectChange05, TestSize.Level1)
{
    ScreenId internalId = 0;
    ScreenSessionConfig internalConfig = {
        .screenId = internalId,
    };
    sptr<ScreenSession> internalSession = new ScreenSession(internalConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    internalSession->SetIsInternal(true);
    internalSession->SetIsExtend(false);
    internalSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_[internalId] = internalSession;
    }

    sptr<ScreenSession> externalSession = nullptr;

    MultiScreenManager::GetInstance().ExternalScreenDisconnectChange(internalSession, externalSession);

    EXPECT_EQ(false, internalSession->GetIsExtend());
    EXPECT_EQ(ScreenCombination::SCREEN_MAIN, internalSession->GetScreenCombination());
    {
        std::lock_guard<std::recursive_mutex>
            lock(ScreenSessionManager::GetInstance().screenSessionMapMutex_);
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(internalId);
    }
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: A is located to the left of B
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching01, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 200, 0};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: B is located to the left of A
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching02, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 200, 0};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 0, 0};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: A is located to the upper of B
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching03, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 0, 100};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: B is located to the upper of A
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching04, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 0, 100};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 0, 0};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: B is located in the lower right corner of A
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching05, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 200, 100};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: A is located in the lower right corner of B
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching06, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 200, 100};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 0, 0};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: B has no contact with A
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching07, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 300, 0};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: AreScreensTouching
 * @tc.desc: B has no contact with A
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, AreScreensTouching08, TestSize.Level1)
{
    ScreenProperty mainProperty = ScreenProperty();
    RRect mainBounds;
    mainBounds.rect_.width_ = 200;
    mainBounds.rect_.height_ = 100;
    mainProperty.SetBounds(mainBounds);
    DisplayId mainScreenId = 0;
    sptr<ScreenSession> mainScreenSession = new (std::nothrow) ScreenSession(mainScreenId, mainProperty, 0);
    ASSERT_NE(mainScreenSession, nullptr);

    ScreenProperty secondProperty = ScreenProperty();
    RRect secondBounds;
    secondBounds.rect_.width_ = 200;
    secondBounds.rect_.height_ = 100;
    secondProperty.SetBounds(secondBounds);
    DisplayId secondScreenId = 1;
    sptr<ScreenSession> secondScreenSession = new (std::nothrow) ScreenSession(secondScreenId, secondProperty, 0);
    ASSERT_NE(secondScreenSession, nullptr);

    MultiScreenPositionOptions mainScreenOptions = {mainScreenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {secondScreenId, 0, 200};
    auto ret = MultiScreenManager::GetInstance().AreScreensTouching(mainScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, false);
}
}
} // namespace Rosen
} // namespace OHOS
