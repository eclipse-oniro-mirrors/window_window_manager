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

#include "iremote_object_mocker.h"
#include "screen_session_manager_client.h"
#include "zidl/screen_session_manager_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class ScreenSessionManagerClientAvailableAreaTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    ScreenSessionManagerClient* client_ = nullptr;
};

void ScreenSessionManagerClientAvailableAreaTest::SetUp()
{
    client_ = &ScreenSessionManagerClient::GetInstance();
    std::lock_guard<std::mutex> lock(client_->screenSessionMapMutex_);
    client_->screenSessionMap_.clear();
}

void ScreenSessionManagerClientAvailableAreaTest::TearDown()
{
    std::lock_guard<std::mutex> lock(client_->screenSessionMapMutex_);
    client_->screenSessionMap_.clear();
    client_->screenSessionManager_ = nullptr;
    client_ = nullptr;
}

namespace {
/**
 * @tc.name: UpdateAvailableArea_SyncLocalCache
 * @tc.desc: UpdateAvailableArea writes local ScreenSession cache before forwarding to server
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientAvailableAreaTest, UpdateAvailableArea_SyncLocalCache, TestSize.Level1)
{
    ScreenId screenId = 0;
    DMRect area { 10, 20, 100, 200 };
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    client_->screenSessionManager_ = new ScreenSessionManagerProxy(iRemoteObjectMocker);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), 0);
    client_->screenSessionMap_.emplace(screenId, screenSession);

    client_->UpdateAvailableArea(screenId, area);

    EXPECT_EQ(screenSession->GetScreenProperty().GetAvailableArea(), area);
}

/**
 * @tc.name: UpdateSuperFoldAvailableArea_SyncLocalBArea
 * @tc.desc: UpdateSuperFoldAvailableArea writes local main screen cache with bArea
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientAvailableAreaTest, UpdateSuperFoldAvailableArea_SyncLocalBArea, TestSize.Level1)
{
    ScreenId screenId = 0;
    DMRect bArea { 1, 2, 100, 200 };
    DMRect cArea { 3, 4, 50, 60 };
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    client_->screenSessionManager_ = new ScreenSessionManagerProxy(iRemoteObjectMocker);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), 0);
    client_->screenSessionMap_.emplace(screenId, screenSession);

    client_->UpdateSuperFoldAvailableArea(screenId, bArea, cArea);

    EXPECT_EQ(screenSession->GetScreenProperty().GetAvailableArea(), bArea);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
