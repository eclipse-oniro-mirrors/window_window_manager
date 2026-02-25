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

#include "anomaly_detection.h"
#include <gtest/gtest.h>
#include "session_manager/include/scene_session_manager.h"
#include "interfaces/include/ws_common.h"
#include "session/host/include/session.h"
#include "session/host/include/scene_session.h"
#include "common/include/window_session_property.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
class AnomalyDetectionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<SceneSessionManager> ssm_;
};

sptr<SceneSessionManager> AnomalyDetectionTest::ssm_ = nullptr;

void AnomalyDetectionTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    ssm_->sceneSessionMap_.clear();
}

void AnomalyDetectionTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void AnomalyDetectionTest::SetUp()
{
}

void AnomalyDetectionTest::TearDown()
{
}

namespace {
sptr<SceneSession> GetSceneSession(std::string name)
{
    SessionInfo info;
    info.abilityName_ = name;
    info.bundleName_ = name;
    auto result = sptr<SceneSession>::MakeSptr(info, nullptr);
    if (result != nullptr) {
        result->property_ = sptr<WindowSessionProperty>::MakeSptr();
        if (result->property_ == nullptr) {
            return nullptr;
        }
    }

    return result;
}

void SessionVisibleTest(sptr<SceneSessionManager> ssm_)
{
    ssm_->sceneSessionMap_.insert({0, nullptr});
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();

    auto sceneSession = GetSceneSession("SessionVisibleTest");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->isVisible_ = false;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

void ZeroOrderTest(sptr<SceneSessionManager> ssm_)
{
    auto sceneSession = GetSceneSession("ZeroOrderTest");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->zOrder_ = 0;
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

void EQOrderTest(sptr<SceneSessionManager> ssm_)
{
    int32_t sameOrder = 1;
    int32_t persistentId1 = 1;

    auto sceneSession1 = GetSceneSession("EQOrderTest1");
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->isVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    sceneSession1->sessionInfo_.persistentId_ = persistentId1;
    sceneSession1->zOrder_ = sameOrder;
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({sceneSession1->GetPersistentId(), sceneSession1});

    int32_t persistentId2 = 2;
    auto sceneSession2 = GetSceneSession("EQOrderTest2");
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->isVisible_ = true;
    sceneSession2->state_ = SessionState::STATE_ACTIVE;
    sceneSession2->sessionInfo_.persistentId_ = persistentId2;
    sceneSession2->zOrder_ = sameOrder;
    sceneSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({sceneSession2->GetPersistentId(), sceneSession2});

    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

void WindowTypeTest(sptr<SceneSessionManager> ssm_)
{
    int32_t order = 1;
    int32_t callingId = 1;
    int32_t persistentId = 2;
    auto sceneSession = GetSceneSession("WindowTypeTest");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = persistentId;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->zOrder_ = order;
    sceneSession->property_->SetCallingSessionId(callingId);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    AnomalyDetection::SceneZOrderCheckProcess();

    auto callingSession = GetSceneSession("WindowTypeTestCalling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->persistentId_ = callingId;
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->isVisible_ = true;
    callingSession->state_ = SessionState::STATE_ACTIVE;
    callingSession->zOrder_ = sceneSession->GetZOrder() + 1;
    ssm_->sceneSessionMap_.insert({callingSession->GetPersistentId(), callingSession});
    AnomalyDetection::SceneZOrderCheckProcess();

    callingSession->zOrder_ = sceneSession->GetZOrder() - 1;
    AnomalyDetection::SceneZOrderCheckProcess();

    ssm_->sceneSessionMap_.insert({callingSession->GetPersistentId(), nullptr});
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

void SubWindowTest(sptr<SceneSessionManager> ssm_)
{
    int32_t order = 100;
    int32_t mainId = 1;
    int32_t persistentId = 2;
    auto sceneSession = GetSceneSession("SubWindowTest");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = persistentId;
    sceneSession->zOrder_ = order;
    sceneSession->property_->SetCallingSessionId(mainId);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->property_->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    AnomalyDetection::SceneZOrderCheckProcess();

    auto mainSession = GetSceneSession("SubWindowMainTest");
    ASSERT_NE(mainSession, nullptr);
    mainSession->persistentId_ = mainId;
    mainSession->isVisible_ = true;
    mainSession->state_ = SessionState::STATE_ACTIVE;
    mainSession->zOrder_ = sceneSession->GetZOrder() + 1;
    mainSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({mainSession->GetPersistentId(), mainSession});
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession->parentSession_ = nullptr;
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession->parentSession_ = mainSession;
    AnomalyDetection::SceneZOrderCheckProcess();

    mainSession->zOrder_ = sceneSession->GetZOrder() - 1;
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    AnomalyDetection::SceneZOrderCheckProcess();

    ssm_->sceneSessionMap_.clear();
}

void KeyGUARDTest(sptr<SceneSessionManager> ssm_)
{
    int32_t order = 100;
    auto sceneSession = GetSceneSession("KeyGUARDTest");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->zOrder_ = order;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYGUARD);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

void IsShowWhenLockedTest(sptr<SceneSessionManager> ssm_)
{
    int32_t order = 100;
    int32_t id = 1;

    auto sceneSession1 = GetSceneSession("IsShowWhenLockedTest1");
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYGUARD);
    sceneSession1->zOrder_ = order--;
    sceneSession1->persistentId_ = id++;
    sceneSession1->isVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    ssm_->sceneSessionMap_.insert({sceneSession1->GetPersistentId(), sceneSession1});

    auto sceneSession2 = GetSceneSession("IsShowWhenLockedTest2");
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2->isVisible_ = true;
    sceneSession2->state_ = SessionState::STATE_ACTIVE;
    sceneSession2->zOrder_ = order--;
    sceneSession2->persistentId_ = id++;
    ssm_->sceneSessionMap_.insert({sceneSession2->GetPersistentId(), sceneSession2});
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession2->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession2->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    AnomalyDetection::SceneZOrderCheckProcess();

    sceneSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

void WallPaperTest(sptr<SceneSessionManager> ssm_)
{
    int32_t order = 100;
    int32_t id = 1;
    auto sceneSession1 = GetSceneSession("wallpaperTest");
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_WALLPAPER);
    sceneSession1->zOrder_ = order;
    sceneSession1->persistentId_ = id;
    sceneSession1->isVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    AnomalyDetection::SceneZOrderCheckProcess();
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SceneZOrderCheckProcess
 * @tc.desc: check func SceneZOrderCheckProcess
 * @tc.type: FUNC
 */
HWTEST_F(AnomalyDetectionTest, SceneZOrderCheckProcess, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnomalyDetectionTest: SceneZOrderCheckProcess start";
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    AnomalyDetection::SceneZOrderCheckProcess();
    EXPECT_FALSE(g_errLog.find("ZOrderCheck err, zorder 0") != std::string::npos);
    SessionVisibleTest(ssm_);
    ZeroOrderTest(ssm_);
    EQOrderTest(ssm_);
    WindowTypeTest(ssm_);
    SubWindowTest(ssm_);
    KeyGUARDTest(ssm_);
    IsShowWhenLockedTest(ssm_);
    WallPaperTest(ssm_);
    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "AnomalyDetectionTest: SceneZOrderCheckProcess end";
}
}
}
}