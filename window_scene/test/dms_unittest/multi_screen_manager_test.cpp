/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

using namespace testing;
using namespace testing::ext;

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
};

void MultiScreenManagerTest::SetUpTestCase()
{
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
HWTEST_F(MultiScreenManagerTest, FilterPhysicalAndVirtualScreen, Function | SmallTest | Level1)
{
    std::vector<ScreenId> allScreenIds = {1, 2, 3};
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
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch01, Function | SmallTest | Level1)
{
    ScreenId mainScreenId = 1;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, screenGroupId);
    EXPECT_EQ(screenGroupId, 0);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: mainScreen is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch02, Function | SmallTest | Level1)
{
    ScreenId mainScreenId = 1002;
    std::vector<ScreenId> ScreenIds = {2, 3};
    ScreenId screenGroupId;
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[mainScreenId] = session;
    auto ret = MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, screenGroupId);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: VirtualScreenMirrorSwitch
 * @tc.desc: ret != DMError::DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenMirrorSwitch03, Function | SmallTest | Level1)
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
    auto ret = MultiScreenManager::GetInstance().VirtualScreenMirrorSwitch(mainScreenId, ScreenIds, screenGroupId);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch
 * @tc.desc: PhysicalScreenMirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch01, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch
 * @tc.desc: defaultSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch02, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
        ScreenSessionManager::GetInstance().defaultScreenId_] = session;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenMirrorSwitch
 * @tc.desc: defaultSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenMirrorSwitch03, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {1002, 1003};
    sptr<ScreenSession> session = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[
        ScreenSessionManager::GetInstance().defaultScreenId_] = session;
    sptr<ScreenSession> session1 = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_[1002] = session1;
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenMirrorSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch
 * @tc.desc: PhysicalScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().PhysicalScreenUniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: PhysicalScreenUniqueSwitch
 * @tc.desc: screenSession != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, PhysicalScreenUniqueSwitch02, Function | SmallTest | Level1)
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
 * @tc.name: VirtualScreenUniqueSwitch01
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch01, Function | SmallTest | Level1)
{
    sptr<ScreenSession> screenSession =new ScreenSession();
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: VirtualScreenUniqueSwitch02
 * @tc.desc: VirtualScreenUniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, VirtualScreenUniqueSwitch02, Function | SmallTest | Level1)
{
    sptr<ScreenSession> screenSession =nullptr;
    std::vector<ScreenId> screenIds = {2, 3};
    DMError ret = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(screenSession, screenIds);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UniqueSwitch
 * @tc.desc: UniqueSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, UniqueSwitch01, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {};
    DMError ret = MultiScreenManager::GetInstance().UniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: UniqueSwitch
 * @tc.desc: screenIds is not empty
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, UniqueSwitch02, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {1001, 1002};
    DMError ret = MultiScreenManager::GetInstance().UniqueSwitch(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: MirrorSwitch
 * @tc.desc: MirrorSwitch func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenManagerTest, MirrorSwitch, Function | SmallTest | Level1)
{
    std::vector<ScreenId> screenIds = {};
    ScreenId screenGroupId = 0;
    DMError ret = MultiScreenManager::GetInstance().MirrorSwitch(1, screenIds, screenGroupId);
    EXPECT_EQ(ret, DMError::DM_OK);
}
}
} // namespace Rosen
} // namespace OHOS