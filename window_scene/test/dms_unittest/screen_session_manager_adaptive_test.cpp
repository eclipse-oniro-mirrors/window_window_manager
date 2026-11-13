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
#include <ipc_types.h>
#include <message_option.h>
#include <message_parcel.h>

#include "screen_session_manager.h"
#include "display_info.h"
#include "dm_common.h"
#include "display_manager_interface_code.h"
#include "zidl/screen_session_manager_stub.h"
#include "../mock/mock_accesstoken_kit.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t TEST_UID = 100;
constexpr float TEST_RATIO = 0.5f;
std::string g_adaptiveLog;
void AdaptiveLogCallback(const LogType type, const LogLevel level, const unsigned int domain,
    const char* tag, const char* msg)
{
    g_adaptiveLog += msg;
}
}

class ScreenSessionManagerAdaptiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static ScreenSessionManager* ssm_;
    sptr<DisplayInfo> MakeDisplayInfo();
    DMAdaptiveResolutionConfig MakeDmConfig(bool enable = true, float ratio = TEST_RATIO);
};

ScreenSessionManager* ScreenSessionManagerAdaptiveTest::ssm_ = nullptr;

void ScreenSessionManagerAdaptiveTest::SetUpTestCase()
{
    // the singleton follows the sibling DMS suites: destroying a full instance mid-run
    // leaks its scheduler thread and trips the exit-time double-free check
    ssm_ = &ScreenSessionManager::GetInstance();
}

void ScreenSessionManagerAdaptiveTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerAdaptiveTest::SetUp()
{
    ssm_->adaptiveResolutionMap_.clear();
    ssm_->displayHookMap_.clear();
}

void ScreenSessionManagerAdaptiveTest::TearDown()
{
    ssm_->adaptiveResolutionMap_.clear();
    ssm_->displayHookMap_.clear();
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(100000); // 100ms
}

sptr<DisplayInfo> ScreenSessionManagerAdaptiveTest::MakeDisplayInfo()
{
    auto displayInfo = sptr<DisplayInfo>::MakeSptr();
    displayInfo->SetWidth(1216);
    displayInfo->SetHeight(2688);
    displayInfo->SetAvailableWidth(1216);
    displayInfo->SetAvailableHeight(2688);
    displayInfo->SetVirtualPixelRatio(3.0f);
    displayInfo->SetDensityInCurResolution(3.0f);
    displayInfo->SetDefaultVirtualPixelRatio(3.0f);
    displayInfo->SetXDpi(480.0f);
    displayInfo->SetYDpi(460.0f);
    return displayInfo;
}

DMAdaptiveResolutionConfig ScreenSessionManagerAdaptiveTest::MakeDmConfig(bool enable, float ratio)
{
    DMAdaptiveResolutionConfig config;
    config.scaleRatio_ = ratio;
    config.autoMetrics_ = true;
    config.notifyDisplay_ = false;
    return config;
}

namespace {
/**
 * @tc.name: UpdateAdaptiveHookMapVariants
 * @tc.desc: permission, enable store, invalid ratios and same-config update branches
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdaptiveTest, UpdateAdaptiveHookMapVariants, TestSize.Level1)
{
    // non-system caller is rejected without touching the map
    MockAccesstokenKit::MockIsSACalling(false);   // token type: not native/shell
    MockAccesstokenKit::MockIsSystemApp(false);  // and not a system app either
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, MakeDmConfig(true));
    EXPECT_TRUE(ssm_->adaptiveResolutionMap_.empty());
    MockAccesstokenKit::ChangeMockStateToInit();

    // valid enable stores the config keyed by uid
    auto config = MakeDmConfig(true);
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, config);
    EXPECT_EQ(ssm_->adaptiveResolutionMap_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionMap_[TEST_UID].scaleRatio_, TEST_RATIO);
    // re-applying a changed config updates the stored value
    auto changed = MakeDmConfig(true, 0.75f);
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, changed);
    EXPECT_EQ(ssm_->adaptiveResolutionMap_[TEST_UID].scaleRatio_, 0.75f);
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, changed);
    EXPECT_EQ(ssm_->adaptiveResolutionMap_[TEST_UID].scaleRatio_, 0.75f);

    // out-of-range ratios stay inactive whatever enable says
    ssm_->adaptiveResolutionMap_.clear();
    auto invalid = MakeDmConfig(true, 1.0f);
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, invalid);
    EXPECT_TRUE(ssm_->adaptiveResolutionMap_.empty());
    invalid.scaleRatio_ = 0.0f;
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, invalid);
    EXPECT_TRUE(ssm_->adaptiveResolutionMap_.empty());
    // an existing entry is erased when the ratio turns invalid
    ssm_->adaptiveResolutionMap_[TEST_UID] = MakeDmConfig(true);
    invalid.scaleRatio_ = -0.5f;
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, invalid);
    EXPECT_TRUE(ssm_->adaptiveResolutionMap_.empty());
}

/**
 * @tc.name: HookQueriesVariants
 * @tc.desc: active-config lookup and the display-hooked query
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdaptiveTest, HookQueriesVariants, TestSize.Level1)
{
    DMAdaptiveResolutionConfig out;
    out.scaleRatio_ = 0.9f;
    EXPECT_FALSE(ssm_->IsAdaptiveResolutionActive(TEST_UID, out));
    EXPECT_EQ(out.scaleRatio_, 0.9f); // untouched on miss
    ssm_->adaptiveResolutionMap_[TEST_UID] = MakeDmConfig(true);
    EXPECT_TRUE(ssm_->IsAdaptiveResolutionActive(TEST_UID, out));
    EXPECT_EQ(out.scaleRatio_, TEST_RATIO);

    // non-system callers get a plain false without probing the hook state
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ssm_->displayHookMap_[TEST_UID][0] = DMHookInfo();
    EXPECT_FALSE(ssm_->IsDisplayHookedByUid(TEST_UID));
    ssm_->displayHookMap_.erase(TEST_UID);
    MockAccesstokenKit::ChangeMockStateToInit();

    EXPECT_FALSE(ssm_->IsDisplayHookedByUid(TEST_UID));
    DMHookInfo hookInfo;
    // two-level map since the dpi-force-scale rework: inner key is the displayId
    ssm_->displayHookMap_[TEST_UID][0] = hookInfo;
    EXPECT_TRUE(ssm_->IsDisplayHookedByUid(TEST_UID));
    ssm_->displayHookMap_.erase(TEST_UID);
    EXPECT_FALSE(ssm_->IsDisplayHookedByUid(TEST_UID));
}

/**
 * @tc.name: ApplyAdaptiveHookVariants
 * @tc.desc: no-config, autoMetrics, explicit/zero overrides and the min-pixel clamp
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdaptiveTest, ApplyAdaptiveHookVariants, TestSize.Level1)
{
    // without a hook: display info passes through unchanged
    auto displayInfo = MakeDisplayInfo();
    ssm_->ApplyAdaptiveResolutionHook(displayInfo, TEST_UID);
    EXPECT_EQ(displayInfo->GetWidth(), 1216);
    EXPECT_EQ(displayInfo->GetVirtualPixelRatio(), 3.0f);

    // autoMetrics scales size and metrics; dpi follows density x ratio x 160
    ssm_->adaptiveResolutionMap_[TEST_UID] = MakeDmConfig(true);
    displayInfo = MakeDisplayInfo();
    ssm_->ApplyAdaptiveResolutionHook(displayInfo, TEST_UID);
    EXPECT_EQ(displayInfo->GetWidth(), 608);  // lround(1216 * 0.5)
    EXPECT_EQ(displayInfo->GetHeight(), 1344); // lround(2688 * 0.5)
    EXPECT_EQ(displayInfo->GetAvailableWidth(), 608u);
    EXPECT_EQ(displayInfo->GetAvailableHeight(), 1344u);
    EXPECT_EQ(displayInfo->GetVirtualPixelRatio(), 1.5f);
    EXPECT_EQ(displayInfo->GetDensityInCurResolution(), 1.5f);
    EXPECT_EQ(displayInfo->GetDefaultVirtualPixelRatio(), 1.5f);
    EXPECT_EQ(displayInfo->GetXDpi(), 240.0f);
    EXPECT_EQ(displayInfo->GetYDpi(), 230.0f);
    EXPECT_EQ(displayInfo->GetDpi(), 240); // lround(3.0 * 0.5 * 160)

    // explicit overrides replace the metrics; dpi follows density x 160
    DMAdaptiveResolutionConfig override;
    override.scaleRatio_ = TEST_RATIO;
    override.autoMetrics_ = false;
    override.density_ = 2.0f;
    override.xDpi_ = 320.0f;
    override.yDpi_ = 330.0f;
    ssm_->adaptiveResolutionMap_[TEST_UID] = override;
    displayInfo = MakeDisplayInfo();
    ssm_->ApplyAdaptiveResolutionHook(displayInfo, TEST_UID);
    EXPECT_EQ(displayInfo->GetWidth(), 608); // size still follows the ratio
    EXPECT_EQ(displayInfo->GetVirtualPixelRatio(), 2.0f);
    EXPECT_EQ(displayInfo->GetXDpi(), 320.0f);
    EXPECT_EQ(displayInfo->GetYDpi(), 330.0f);
    EXPECT_EQ(displayInfo->GetDpi(), 320); // lround(2.0 * 160)

    // zero overrides keep the original metrics and skip the dpi rewrite
    DMAdaptiveResolutionConfig zero;
    zero.scaleRatio_ = TEST_RATIO;
    zero.autoMetrics_ = false;
    ssm_->adaptiveResolutionMap_[TEST_UID] = zero;
    displayInfo = MakeDisplayInfo();
    displayInfo->SetDpi(300);
    ssm_->ApplyAdaptiveResolutionHook(displayInfo, TEST_UID);
    EXPECT_EQ(displayInfo->GetWidth(), 608); // size still scaled
    EXPECT_EQ(displayInfo->GetVirtualPixelRatio(), 3.0f);
    EXPECT_EQ(displayInfo->GetXDpi(), 480.0f);
    EXPECT_EQ(displayInfo->GetDpi(), 300); // untouched: no density override

    // tiny sizes clamp to one pixel instead of collapsing to zero
    ssm_->adaptiveResolutionMap_[TEST_UID] = MakeDmConfig(true);
    auto tiny = sptr<DisplayInfo>::MakeSptr();
    tiny->SetWidth(1);
    tiny->SetHeight(0);
    tiny->SetVirtualPixelRatio(0.0f);
    tiny->SetDensityInCurResolution(0.0f);
    tiny->SetDefaultVirtualPixelRatio(0.0f);
    tiny->SetXDpi(0.0f);
    tiny->SetYDpi(0.0f);
    tiny->SetDpi(300);
    ssm_->ApplyAdaptiveResolutionHook(tiny, TEST_UID);
    EXPECT_EQ(tiny->GetWidth(), 1); // max(1, lround(1 * 0.5)) = 1
    EXPECT_EQ(tiny->GetHeight(), 1);
    EXPECT_EQ(tiny->GetVirtualPixelRatio(), 0.0f); // zero metrics are kept
    EXPECT_EQ(tiny->GetDpi(), 300); // kept: invalid density skips the dpi rewrite
}

/**
 * @tc.name: UpdateAdaptiveHookNotifySemantics
 * @tc.desc: system clears follow the enabled config; SA disables follow the passed one
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdaptiveTest, UpdateAdaptiveHookNotifySemantics, TestSize.Level1)
{
    g_adaptiveLog.clear();
    LOG_SetCallback(AdaptiveLogCallback);
    auto enabled = MakeDmConfig(true);
    enabled.notifyDisplay_ = true;
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, enabled);
    EXPECT_EQ(ssm_->adaptiveResolutionMap_.size(), 1u);
    g_adaptiveLog.clear();

    // the clear request carries no settings; the notify follows the enabled config
    DMAdaptiveResolutionConfig clear;
    clear.notifyDisplay_ = false;
    clear.systemModified_ = true;
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, false, clear);
    EXPECT_TRUE(ssm_->adaptiveResolutionMap_.empty());
    EXPECT_TRUE(g_adaptiveLog.find("[AdaptiveDisplayNotify]") != std::string::npos);
    g_adaptiveLog.clear();

    // SA disable with notifyDisplay_=false: the passed config speaks for itself
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, true, enabled);
    g_adaptiveLog.clear();
    auto saDisable = MakeDmConfig(false);
    ssm_->UpdateAdaptiveResolutionHook(TEST_UID, false, saDisable);
    EXPECT_TRUE(ssm_->adaptiveResolutionMap_.empty());
    EXPECT_TRUE(g_adaptiveLog.find("[AdaptiveDisplayNotify]") == std::string::npos);
    LOG_SetCallback(nullptr);
    g_adaptiveLog.clear();
}

/**
 * @tc.name: StubParcelVariants
 * @tc.desc: the UPDATE_ADAPTIVE and IS_DISPLAY_HOOKED IPC entries
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdaptiveTest, StubParcelVariants, TestSize.Level1)
{
    // drive the real singleton so the stub entries reach the SSM handlers
    auto stub = ssm_;
    ASSERT_NE(stub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor()));
    data.WriteInt32(TEST_UID);
    data.WriteBool(true);
    auto config = MakeDmConfig(true);
    data.WriteFloat(config.scaleRatio_);
    data.WriteBool(config.autoMetrics_);
    data.WriteBool(config.notifyDisplay_);
    data.WriteFloat(config.density_);
    data.WriteFloat(config.xDpi_);
    data.WriteFloat(config.yDpi_);
    data.WriteBool(config.systemModified_);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_ADAPTIVE_RESOLUTION_HOOK);
    EXPECT_EQ(stub->OnRemoteRequest(code, data, reply, option), ERR_NONE);

    MessageParcel queryData;
    MessageParcel queryReply;
    ASSERT_TRUE(queryData.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor()));
    queryData.WriteInt32(TEST_UID);
    code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_DISPLAY_HOOKED_BY_UID);
    EXPECT_EQ(stub->OnRemoteRequest(code, queryData, queryReply, option), ERR_NONE);
    bool hooked = true;
    ASSERT_TRUE(queryReply.ReadBool(hooked));
    EXPECT_FALSE(hooked);
}
}
} // namespace Rosen
} // namespace OHOS
