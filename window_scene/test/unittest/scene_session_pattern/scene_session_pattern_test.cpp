#include <gtest/gtest.h>

#include "session/host/include/scene_session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
 
class scene_session_pattern_test : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void scene_session_pattern_test::SetUpTestCase() {}

void scene_session_pattern_test::TearDownTestCase()
{
}

void scene_session_pattern_test::SetUp() {}

void scene_session_pattern_test::TearDown() {}

namespace {

/**
 * @tc.name: NotifySnapshotUpdate
 * @tc.desc: NotifySnapshotUpdate
 * @tc.type: FUNC
 */
HWTEST_F(scene_session_pattern_test, NotifySnapshotUpdate, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WMError ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->collaboratorType_ = static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
    ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);
}


} // namespace
} // namespace Rosen
} // namespace OHOS
