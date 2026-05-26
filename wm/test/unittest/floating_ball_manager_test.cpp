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
#include "floating_ball_manager.h"
#include "wm_common.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class FloatingBallManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class MockWindow : public Window {
public:
    MockWindow() {};
    ~MockWindow() {};
};

void FloatingBallManagerTest::SetUpTestCase()
{
}

void FloatingBallManagerTest::TearDownTestCase()
{
}

void FloatingBallManagerTest::SetUp()
{
}

void FloatingBallManagerTest::TearDown()
{
}

namespace {

/**
 * @tc.name: activeController
 * @tc.desc: activeController
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallManagerTest, activeController, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<FbOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto fbController = sptr<FloatingBallController>::MakeSptr(mw, 101, nullptr);
    // null fbController
    EXPECT_EQ(false, FloatingBallManager::IsActiveController(fbController));
    EXPECT_EQ(false, FloatingBallManager::HasActiveController());
    std::string clickAction = "click";
    std::string closeAction = "close";
    FloatingBallManager::DoFbActionEvent("on");
    FloatingBallManager::DoFbActionEvent(clickAction);
    FloatingBallManager::DoFbActionEvent(closeAction);
    FloatingBallManager::DoDestroy();

    // has fbController
    FloatingBallManager::SetActiveController(fbController);
    EXPECT_EQ(true, FloatingBallManager::IsActiveController(fbController));
    EXPECT_EQ(true, FloatingBallManager::HasActiveController());
    FloatingBallManager::DoFbActionEvent(clickAction);
    FloatingBallManager::DoFbActionEvent(closeAction);
    FloatingBallManager::DoDestroy();
    auto fbController1 = sptr<FloatingBallController>::MakeSptr(mw, 102, nullptr);
    FloatingBallManager::RemoveActiveController(fbController1);
    FloatingBallManager::RemoveActiveController(fbController);
}
}
}
}