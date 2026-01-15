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

#include "screen_session_dumper.h"
#include "screen_session_manager.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
#ifdef FOLD_ABILITY_ENABLE
constexpr uint32_t SIZE_TWO = 2;
constexpr uint32_t SIZE_THREE = 3;
constexpr float POSTURE_FIRST = 93;
constexpr float POSTURE_SECOND = 180;
constexpr float POSTURE_THIRD = 0;
constexpr uint16_t HALL_TEST = 1;
const std::string TEST_SECONDARY_SRNSOR_POSTURE = "posture:93,180,0";
const std::string TEST_SECONDARY_SRNSOR_HALL = "hall:1,1";
#endif
}
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}
class ScreenSessionDumperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSessionDumperTest::SetUpTestCase()
{
}

void ScreenSessionDumperTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_US);
}

void ScreenSessionDumperTest::SetUp()
{
}

void ScreenSessionDumperTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: Dump01
 * @tc.desc: Dump
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, Dump01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args;
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    ASSERT_NE(nullptr, dumper);
}

/**
 * @tc.name: Dump02
 * @tc.desc: Dump input for -h
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, Dump02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteDumpCmd();
    ASSERT_TRUE(true);
}

/**
 * @tc.name: Dump03
 * @tc.desc: Dump input for -a
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, Dump03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-a"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteDumpCmd();
    ASSERT_TRUE(true);
}

/**
 * @tc.name: Dump04
 * @tc.desc: Dump input for abnormal
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, Dump04, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-abnormal"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteDumpCmd();
    ASSERT_TRUE(true);
}

/**
 * @tc.name: Dump05
 * @tc.desc: Dump fd less 0
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, Dump05, TestSize.Level1)
{
    int fd = -1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteDumpCmd();
    ASSERT_TRUE(true);
}

/**
 * @tc.name: OutputDumpInfo
 * @tc.desc: test function : OutputDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, OutputDumpInfo, TestSize.Level1)
{
    int fd = -1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->OutputDumpInfo();
    ASSERT_EQ(dumper->fd_, -1);
}

/**
 * @tc.name: ExecuteDumpCmd
 * @tc.desc: test function : ExecuteDumpCmd
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteDumpCmd, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args;
    sptr<ScreenSessionDumper> dumper1 = new ScreenSessionDumper(fd, args);
    dumper1->ExecuteDumpCmd();
    ASSERT_EQ(dumper1->fd_, 1);

    fd = 1;
    args = {u"-h"};
    sptr<ScreenSessionDumper> dumper2 = new ScreenSessionDumper(fd, args);
    dumper2->ExecuteDumpCmd();
    ASSERT_EQ(dumper2->fd_, 1);

    fd = 1;
    args = {u"-a"};
    sptr<ScreenSessionDumper> dumper3 = new ScreenSessionDumper(fd, args);
    dumper3->ExecuteDumpCmd();
    ASSERT_EQ(dumper3->fd_, 1);

    fd = 1;
    args = {u"-f"};
    sptr<ScreenSessionDumper> dumper4 = new ScreenSessionDumper(fd, args);
    dumper4->ExecuteDumpCmd();
    ASSERT_EQ(dumper4->fd_, 1);

    fd = 1;
    args = {u"-z"};
    sptr<ScreenSessionDumper> dumper5 = new ScreenSessionDumper(fd, args);
    dumper5->ExecuteDumpCmd();
    ASSERT_EQ(dumper5->fd_, 1);

    fd = 1;
    args = {u"-y"};
    sptr<ScreenSessionDumper> dumper6 = new ScreenSessionDumper(fd, args);
    dumper6->ExecuteDumpCmd();
    ASSERT_EQ(dumper6->fd_, 1);

    fd = 1;
    args = {u"-p"};
    sptr<ScreenSessionDumper> dumper7 = new ScreenSessionDumper(fd, args);
    dumper7->ExecuteDumpCmd();
    ASSERT_EQ(dumper7->fd_, 1);

    fd = 1;
    args = {u"-g"};
    sptr<ScreenSessionDumper> dumper8 = new ScreenSessionDumper(fd, args);
    dumper8->ExecuteDumpCmd();
    ASSERT_EQ(dumper8->fd_, 1);

    fd = 1;
    args = {u"-ln,1"};
    sptr<ScreenSessionDumper> dumper10 = new ScreenSessionDumper(fd, args);
    dumper10->ExecuteDumpCmd();
    ASSERT_EQ(dumper10->fd_, 1);
}

/**
 * @tc.name: DumpEventTracker
 * @tc.desc: test function : DumpEventTracker
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpEventTracker, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    EventTracker tracker;
    dumper->DumpEventTracker(tracker);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpFreezedPidList
 * @tc.desc: test function : DumpFreezedPidList
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpFreezedPidList, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::set<int32_t> pidList = {1, 2, 3};
    dumper->DumpFreezedPidList(pidList);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ShowHelpInfo
 * @tc.desc: test function : ShowHelpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ShowHelpInfo, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ShowHelpInfo();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ShowAllScreenInfo
 * @tc.desc: test function : ShowAllScreenInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ShowAllScreenInfo, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ShowAllScreenInfo();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpFoldStatus
 * @tc.desc: test function : DumpFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpFoldStatus, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->DumpFoldStatus();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpScreenSessionById
 * @tc.desc: test function : DumpScreenSessionById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpScreenSessionById, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    ScreenId id = 0;
    dumper->DumpScreenSessionById(id);
    ASSERT_EQ(dumper->fd_, 1);

    id = 5;
    dumper->DumpScreenSessionById(id);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpRsInfoById
 * @tc.desc: test function : DumpRsInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpRsInfoById, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    ScreenId id = 0;
    dumper->DumpRsInfoById(id);
    ASSERT_EQ(dumper->fd_, 1);

    id = 5;
    dumper->DumpRsInfoById(id);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpCutoutInfoById
 * @tc.desc: test function : DumpCutoutInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpCutoutInfoById, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    ScreenId id = 0;
    dumper->DumpCutoutInfoById(id);
    ASSERT_EQ(dumper->fd_, 1);

    id = 5;
    dumper->DumpCutoutInfoById(id);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpScreenInfoById
 * @tc.desc: test function : DumpScreenInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpScreenInfoById, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    ScreenId id = 0;
    dumper->DumpScreenInfoById(id);
    ASSERT_EQ(dumper->fd_, 1);

    id = 5;
    dumper->DumpScreenInfoById(id);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: DumpScreenPropertyById
 * @tc.desc: test function : DumpScreenPropertyById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpScreenPropertyById, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    ScreenId id = 0;
    dumper->DumpScreenPropertyById(id);
    ASSERT_EQ(dumper->fd_, 1);

    id = 5;
    dumper->DumpScreenPropertyById(id);
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ShowNotifyFoldStatusChangedInfo
 * @tc.desc: test function : ShowNotifyFoldStatusChangedInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ShowNotifyFoldStatusChangedInfo, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->params_[0] = "0";
    dumper->ShowNotifyFoldStatusChangedInfo();
    ASSERT_EQ(dumper->fd_, 1);

    dumper->params_[0] = "1";
    dumper->ShowNotifyFoldStatusChangedInfo();
    ASSERT_EQ(dumper->fd_, 1);

    dumper->params_[0] = "5";
    dumper->ShowNotifyFoldStatusChangedInfo();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ShowIllegalArgsInfo
 * @tc.desc: test function : ShowIllegalArgsInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ShowIllegalArgsInfo, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->params_[0] = "0";
    dumper->ShowIllegalArgsInfo();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-motion,1";
    dumper ->SetMotionSensorValue(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,2";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,3";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue04, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,4";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue05, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,5";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue06, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,9999";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue07, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,xxxxx";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
    EXPECT_FALSE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetMotionSensorValue
 * @tc.desc: test function : SetMotionSensorValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetMotionSensorValue08, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-motion,,,,,,";
    dumper ->SetMotionSensorValue(input);
    
    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
    EXPECT_FALSE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetRotationLockedValue
 * @tc.desc: test function : SetRotationLockedValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetRotationLockedValue01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-rotationlock,0";
    dumper ->SetRotationLockedValue(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetRotationLockedValue
 * @tc.desc: test function : SetRotationLockedValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetRotationLockedValue02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-rotationlock,1";
    dumper ->SetRotationLockedValue(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetRotationLockedValue
 * @tc.desc: test function : SetRotationLockedValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetRotationLockedValue03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-rotationlock,,,,,";
    dumper ->SetRotationLockedValue(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
    EXPECT_FALSE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetRotationLockedValue
 * @tc.desc: test function : SetRotationLockedValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetRotationLockedValue04, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string input = "-rotationlock,1-";
    dumper ->SetRotationLockedValue(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-publishcastevent,0";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-publishcastevent,1";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-publishcastevent,,,,,";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
    EXPECT_FALSE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent04, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-publishcastevent,1-";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_NE(valueStr.size(), 1);
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent05, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-publishcastevent,a";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_FALSE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent06, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string input = "-publishcastevent=1";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_FALSE(isConditionMet);
}

/**
 * @tc.name: MockSendCastPublishEvent
 * @tc.desc: test function : MockSendCastPublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, MockSendCastPublishEvent07, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper ->MockSendCastPublishEvent("-publishcastevent,7");
    std::string input = "-publishcastevent,7";
    dumper ->MockSendCastPublishEvent(input);

    size_t commaPos = input.find_last_of(',');
    bool isConditionMet = commaPos != std::string::npos;
    EXPECT_TRUE(isConditionMet);

    std::string valueStr = input.substr(commaPos + 1);
    EXPECT_EQ(valueStr.size(), 1);
    EXPECT_TRUE(std::isdigit(valueStr[0]));
}

/**
 * @tc.name: SetHoverStatusChange
 * @tc.desc: test function : SetHoverStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetHoverStatusChange, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    dumper ->SetHoverStatusChange("-hoverStatus,-1");
    dumper ->SetHoverStatusChange("-hoverStatus,-0");
    dumper ->SetHoverStatusChange("-hoverStatus,1");
    dumper ->SetHoverStatusChange("-hoverStatus,4");
    EXPECT_TRUE(g_errLog.find("screenSession is nullptr") != std::string::npos ||
        g_errLog.find("the value is too long") ！= std::string::npos ||
        g_errLog.find("the value is not a number") ！= std::string::npos ||
        g_errLog.find("params is invalid") ！= std::string::npos)
    g_errLog.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: IsValidDisplayModeCommand
 * @tc.desc: test function : IsValidDisplayModeCommand
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, IsValidDisplayModeCommand, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-f"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::string command = "-f";
    bool ret = dumper->IsValidDisplayModeCommand(command);
    ASSERT_EQ(ret, true);

    command = "-m";
    ret = dumper->IsValidDisplayModeCommand(command);
    ASSERT_EQ(ret, true);

    command = "-sub";
    ret = dumper->IsValidDisplayModeCommand(command);
    ASSERT_EQ(ret, true);

    command = "-coor";
    ret = dumper->IsValidDisplayModeCommand(command);
    ASSERT_EQ(ret, true);

    command = "-test";
    ret = dumper->IsValidDisplayModeCommand(command);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: SetLandscapeLock
 * @tc.desc: test function : SetLandscapeLock
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetLandscapeLock, Function | SmallTest | Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetLandscapeLock("-landscapelock,");
    ASSERT_EQ(ScreenSessionManager::GetInstance().extendScreenConnectStatus_, ExtendScreenConnectStatus::UNKNOWN);
    dumper->SetLandscapeLock("-landscapelock,a");
    ASSERT_EQ(ScreenSessionManager::GetInstance().extendScreenConnectStatus_, ExtendScreenConnectStatus::UNKNOWN);
}

/**
 * @tc.name: DumpTentMode
 * @tc.desc: test function : DumpTentMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpTentMode, Function | SmallTest | Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->DumpTentMode();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: IsNumber01
 * @tc.desc: test function : IsNumber
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, IsNumber01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    
    bool ret = dumper->IsNumber("");
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: IsNumber02
 * @tc.desc: test function : IsNumber
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, IsNumber02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    bool ret = dumper->IsNumber("123");
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: IsNumber03
 * @tc.desc: test function : IsNumber
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, IsNumber03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    bool ret = dumper->IsNumber("123aaa");
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: ExecuteInjectCmd201
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd201, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-ontent"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ExecuteInjectCmd202
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd202, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-hoverstatus"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ExecuteInjectCmd203
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd203, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-supertrans"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ExecuteInjectCmd204
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd204, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-posture"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ExecuteInjectCmd205
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd205, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-registerhall"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: ExecuteInjectCmd206
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd206, Function | SmallTest | Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-landscapelock,1"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_NE(ScreenSessionManager::GetInstance().extendScreenConnectStatus_, ExtendScreenConnectStatus::CONNECT);
}

/**
 * @tc.name: ExecuteInjectCmd207
 * @tc.desc: test function : ExecuteInjectCmd2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ExecuteInjectCmd207, Function | SmallTest | Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-landscapelock,0"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->ExecuteInjectCmd2();
    ASSERT_NE(ScreenSessionManager::GetInstance().extendScreenConnectStatus_, ExtendScreenConnectStatus::DISCONNECT);
}

/**
 * @tc.name: DumpMultiUserInfo
 * @tc.desc: test function : DumpMultiUserInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpMultiUserInfo, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    std::vector<int32_t> oldScbPids = {1, 2, 3};
    int32_t userId = 1;
    int32_t scbPid = 1;
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->DumpMultiUserInfo(oldScbPids, userId, scbPid);
    ASSERT_NE(dumper->dumpInfo_, std::string());
}

/**
 * @tc.name: ShowCurrentStatus
 * @tc.desc: test function : ShowCurrentStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ShowCurrentStatus, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-lcd"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->dumpInfo_ = "";
    dumper->ExecuteDumpCmd();
 
    dumper->dumpInfo_ = "";
    ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
    dumper->ShowCurrentStatus(SCREEN_ID_FULL);
    ASSERT_TRUE(dumper->dumpInfo_.find("PANEL_POWER_STATUS_ON") == std::string::npos);
 
    dumper->dumpInfo_ = "";
    ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
    dumper->ShowCurrentStatus(SCREEN_ID_FULL);
    ASSERT_TRUE(dumper->dumpInfo_.find("PANEL_POWER_STATUS_OFF") == std::string::npos);
 
    dumper->dumpInfo_ = "";
    dumper->ShowCurrentStatus(12478);
    ASSERT_TRUE(dumper->dumpInfo_.find("status failed") == std::string::npos);
}

#ifdef FOLD_ABILITY_ENABLE
/**
 * @tc.name: DumpFoldCreaseRegion
 * @tc.desc: test function : DumpFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, DumpFoldCreaseRegion, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->DumpFoldCreaseRegion();
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetHallAndPostureValue
 * @tc.desc: test function : SetHallAndPostureValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetHallAndPostureValue, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper1 = new ScreenSessionDumper(fd, args);
    dumper1->SetHallAndPostureValue("-hoverstatus,-1");
    ASSERT_EQ(dumper1->fd_, 1);

    fd = 1;
    args = {u"-posture"};
    sptr<ScreenSessionDumper> dumper2 = new ScreenSessionDumper(fd, args);
    dumper2->SetHallAndPostureValue("-hoverstatus,string1,string2");
    ASSERT_EQ(dumper2->fd_, 1);
}

/**
 * @tc.name: SetHallAndPostureStatus01
 * @tc.desc: test function : SetHallAndPostureStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetHallAndPostureStatus01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetHallAndPostureStatus("");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetHallAndPostureStatus02
 * @tc.desc: test function : SetHallAndPostureStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetHallAndPostureStatus02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetHallAndPostureStatus("-registerhall,a,a");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetHallAndPostureStatus03
 * @tc.desc: test function : SetHallAndPostureStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetHallAndPostureStatus03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    if (ScreenSessionManager::GetInstance().foldScreenController_) {
        dumper->SetHallAndPostureStatus("-registerhall,1");
        ASSERT_EQ(dumper->fd_, 1);
    }
}

/**
 * @tc.name: SetSuperFoldStatusChange01
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSuperFoldStatusChange02
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSuperFoldStatusChange03
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,a");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSuperFoldStatusChange04
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange04, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,ab");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSuperFoldStatusChange05
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange05, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,-1");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSuperFoldStatusChange06
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange06, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,1");
    ASSERT_EQ(dumper->fd_, 1);
}


/**
 * @tc.name: SetSuperFoldStatusChange07
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange07, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,4");
    ASSERT_EQ(dumper->fd_, 1);

    EXPECT_TRUE(g_errLog.find("set hall value") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSuperFoldStatusChange08
 * @tc.desc: test function : SetSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSuperFoldStatusChange08, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-supertrans,5");
    ASSERT_EQ(dumper->fd_, 1);
    EXPECT_TRUE(g_errLog.find("set hall value") != std::string::npos);
    LOG_SetCallback(nullptr);
}


/**
 * @tc.name: SetSecondaryStatusChange01
 * @tc.desc: test function : SetSecondaryStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSecondaryStatusChange01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSuperFoldStatusChange("-secondary,f");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSecondaryStatusChange02
 * @tc.desc: test function : SetSecondaryStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSecondaryStatusChange02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSecondaryStatusChange("-secondary,m");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: SetSecondaryStatusChange03
 * @tc.desc: test function : SetSecondaryStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetSecondaryStatusChange03, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->SetSecondaryStatusChange("-secondary,g");
    ASSERT_EQ(dumper->fd_, 1);
}

/**
 * @tc.name: IsAllCharDigit01
 * @tc.desc: test function : IsAllCharDigit
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, IsAllCharDigit01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    bool res = dumper->IsAllCharDigit("-secondary,g");
    EXPECT_FALSE(res);
    res = dumper->IsAllCharDigit("111");
    EXPECT_TRUE(res);
}

/**
 * @tc.name: GetPostureAndHall01
 * @tc.desc: test function : GetPostureAndHall
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, GetPostureAndHall01, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    std::vector<std::string> strVec { TEST_SECONDARY_SRNSOR_POSTURE, TEST_SECONDARY_SRNSOR_HALL };
    std::vector<float> postures;
    std::vector<uint16_t> halls;
    bool res = dumper->GetPostureAndHall(strVec, postures, halls);
    EXPECT_TRUE(res);
    EXPECT_EQ(postures.size(), SIZE_THREE);
    EXPECT_EQ(postures[0], POSTURE_FIRST);
    EXPECT_EQ(postures[1], POSTURE_SECOND);
    EXPECT_EQ(postures[SIZE_TWO], POSTURE_THIRD);
    EXPECT_EQ(halls.size(), SIZE_TWO);
    EXPECT_EQ(halls[0], HALL_TEST);
    EXPECT_EQ(halls[1], HALL_TEST);
}

/**
 * @tc.name: GetPostureAndHall02
 * @tc.desc: test function : GetPostureAndHall
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, GetPostureAndHall02, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::vector<float> postures;
    std::vector<uint16_t> halls;
    std::vector<std::string> strVecFirst { "posture:93,180,2", "hall:1,1" };
    bool res = dumper->GetPostureAndHall(strVecFirst, postures, halls);
    EXPECT_FALSE(res);

    postures.clear();
    halls.clear();
    std::vector<std::string> strVecSecond { "posture:181,180,0", "hall:1,1" };
    res = dumper->GetPostureAndHall(strVecSecond, postures, halls);
    EXPECT_TRUE(res);

    postures.clear();
    halls.clear();
    std::vector<std::string> strVecFourth { "posture:90,170,0", "hall:a,1" };
    res = dumper->GetPostureAndHall(strVecFourth, postures, halls);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: TriggerSecondarySensor01
 * @tc.desc: test function : TriggerSecondarySensor
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, TriggerSecondarySensor01, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        return;
    }
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(1, args);
    dumper->TriggerSecondarySensor("posture:178,180,0/hall:1,1");
    FoldStatus status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND);

    dumper->TriggerSecondarySensor("posture:93,180,0/hall:1,1");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND);

    dumper->TriggerSecondarySensor("posture:150,88,0/hall:1,1");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED);

    dumper->TriggerSecondarySensor("posture:3,88,0/hall:1,1");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED);

    dumper->TriggerSecondarySensor("posture:88,3,0/hall:1,0");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::HALF_FOLD);
}

/**
 * @tc.name: TriggerSecondaryFoldStatus01
 * @tc.desc: test function : TriggerSecondaryFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, TriggerSecondaryFoldStatus01, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        return;
    }
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(1, args);
    dumper->TriggerSecondaryFoldStatus("z=1");
    FoldStatus status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::EXPAND);

    dumper->TriggerSecondaryFoldStatus("z=2");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLDED);

    dumper->TriggerSecondaryFoldStatus("z=3");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::HALF_FOLD);

    dumper->TriggerSecondaryFoldStatus("z=11");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND);

    dumper->TriggerSecondaryFoldStatus("z=21");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED);

    dumper->TriggerSecondaryFoldStatus("z=12");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND);

    dumper->TriggerSecondaryFoldStatus("z=22");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED);

    dumper->TriggerSecondaryFoldStatus("z=13");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND);

    dumper->TriggerSecondaryFoldStatus("z=23");
    status = ScreenSessionManager::GetInstance().GetFoldStatus();
    EXPECT_EQ(status, FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED);
}

/**
 * @tc.name: SetFoldStatusLocked
 * @tc.desc: test function : SetFoldStatusLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetFoldStatusLocked, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u""};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    int ret = dumper->SetFoldStatusLocked();
    ASSERT_EQ(ret, -1);

    dumper->params_[0] = "-l";
    ret = dumper->SetFoldStatusLocked();
    ASSERT_EQ(ret, 0);

    dumper->params_[0] = "-u";
    ret = dumper->SetFoldStatusLocked();
    ASSERT_EQ(ret, 0);

    dumper->params_[0] = "-test";
    ret = dumper->SetFoldStatusLocked();
    ASSERT_EQ(ret, -1);
}

/**
 * @tc.name: ForceSetFoldStatusAndLock
 * @tc.desc: test function : ForceSetFoldStatusAndLock
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, ForceSetFoldStatusAndLock, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u""};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);

    std::string emptyInput = "";
    int ret = dumper->ForceSetFoldStatusAndLock(emptyInput);
    ASSERT_EQ(ret, -1);

    std::string noCommaInput = "-ln1";
    ret = dumper->ForceSetFoldStatusAndLock(noCommaInput);
    ASSERT_EQ(ret, -1);

    std::string wrongPrefixInput = "-lx,1";
    ret = dumper->ForceSetFoldStatusAndLock(wrongPrefixInput);
    ASSERT_EQ(ret, -1);

    std::string invalidStatusInput = "-ln,999";
    ret = dumper->ForceSetFoldStatusAndLock(invalidStatusInput);
    ASSERT_EQ(ret, -1);

    if (!ScreenSessionManager::GetInstance().IsFoldable()) {
        GTEST_SKIP();
    }
    std::vector<std::string> validStatusValues = {
        "1", "2", "3", "11", "21", "12", "22", "13", "23"};
    for (const auto& status : validStatusValues) {
        std::string validInput = "-ln," + status;
        ret = dumper->ForceSetFoldStatusAndLock(validInput);
        ASSERT_TRUE(ret == 0 || ret == -1);
    }
    dumper->RestorePhysicalFoldStatus();
}

/**
 * @tc.name: RestorePhysicalFoldStatus
 * @tc.desc: test function : RestorePhysicalFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, RestorePhysicalFoldStatus, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u""};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    int ret = dumper->RestorePhysicalFoldStatus();
    ASSERT_TRUE(ret == 0 || ret == -1);
}

/**
 * @tc.name: SetFoldDisplayMode
 * @tc.desc: test function : SetFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetFoldDisplayMode, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u""};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    int ret = dumper->SetFoldDisplayMode();
    ASSERT_EQ(ret, -1);

    dumper->params_[0] = "-sub";
    ret = dumper->SetFoldDisplayMode();
    ASSERT_EQ(ret, 0);

    dumper->params_[0] = "-coor";
    ret = dumper->SetFoldDisplayMode();
    ASSERT_EQ(ret, 0);

    dumper->params_[0] = "-m";
    ret = dumper->SetFoldDisplayMode();
    ASSERT_EQ(ret, 0);

    dumper->params_[0] = "-f";
    ret = dumper->SetFoldDisplayMode();
    ASSERT_EQ(ret, 0);

    dumper->params_[0] = "-test";
    ret = dumper->SetFoldDisplayMode();
    ASSERT_EQ(ret, -1);
}

/**
 * @tc.name: SetEnterOrExitTentMode
 * @tc.desc: test function : SetEnterOrExitTentMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetEnterOrExitTentMode, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    
    dumper->SetEnterOrExitTentMode("-offtent");
    bool tentMode = ScreenSessionManager::GetInstance().GetTentMode();
    ASSERT_EQ(tentMode, false);
}

/**
 * @tc.name: SetDuringCallState
 * @tc.desc: test function : SetDuringCallState
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionDumperTest, SetDuringCallState, TestSize.Level1)
{
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    dumper->dumpInfo_ = "";

    dumper->SetDuringCallState("-duringcallstate,1");
    ASSERT_TRUE(dumper->dumpInfo_.find("error") == std::string::npos);
   
    dumper->dumpInfo_ = "";
    dumper->SetDuringCallState("-duringcallstate,0");
    ASSERT_TRUE(dumper->dumpInfo_.find("error") == std::string::npos);

    // no ,
    dumper->dumpInfo_ = "";
    dumper->SetDuringCallState("-duringcallstate3");
    ASSERT_EQ(dumper->dumpInfo_, "");

    // != ARG_SET_DURINGCALL_STATE
    dumper->dumpInfo_ = "";
    dumper->SetDuringCallState("uttest");
    ASSERT_EQ(dumper->dumpInfo_, "");

    // the value is too long
    dumper->dumpInfo_ = "";
    dumper->SetDuringCallState("-duringcallstate,12");
    ASSERT_TRUE(dumper->dumpInfo_.find("[error]: the value is too long") != std::string::npos);

    //value is not a number
    dumper->dumpInfo_ = "";
    dumper->SetDuringCallState("-duringcallstate,a");
    ASSERT_TRUE(dumper->dumpInfo_.find("[error]: value is not a number") != std::string::npos);

    // param is invalid
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    dumper->dumpInfo_ = "";
    dumper->SetDuringCallState("-duringcallstate,3");
    EXPECT_TRUE(g_errLog.find("param is invalid") != std::string::npos);
    LOG_SetCallback(nullptr);
}

#endif // FOLD_ABILITY_ENABLE
}
}
}