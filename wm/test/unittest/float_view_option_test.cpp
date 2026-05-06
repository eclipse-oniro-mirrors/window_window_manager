/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include "float_view_option.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class FloatViewOptionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<FvOption> option_;
};

void FloatViewOptionTest::SetUpTestCase()
{
}

void FloatViewOptionTest::TearDownTestCase()
{
}

void FloatViewOptionTest::SetUp()
{
    option_ = sptr<FvOption>::MakeSptr();
    ASSERT_NE(nullptr, option_);
}

void FloatViewOptionTest::TearDown()
{
    option_ = nullptr;
}

namespace {
/**
 * @tc.name: SetTemplate
 * @tc.desc: Test SetTemplate with various values
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetTemplate, TestSize.Level1)
{
    uint32_t templateType = 1;
    option_->SetTemplate(templateType);
    EXPECT_EQ(templateType, option_->GetTemplate());

    templateType = 0;
    option_->SetTemplate(templateType);
    EXPECT_EQ(templateType, option_->GetTemplate());

    templateType = UINT32_MAX;
    option_->SetTemplate(templateType);
    EXPECT_EQ(templateType, option_->GetTemplate());
}

/**
 * @tc.name: SetUIPath
 * @tc.desc: Test SetUIPath with various paths
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetUIPath, TestSize.Level1)
{
    std::string uiPath = "/path/to/ui";
    option_->SetUIPath(uiPath);
    EXPECT_EQ(uiPath, option_->GetUIPath());

    uiPath = "";
    option_->SetUIPath(uiPath);
    EXPECT_EQ(uiPath, option_->GetUIPath());

    uiPath = "/path/with spaces/special@chars";
    option_->SetUIPath(uiPath);
    EXPECT_EQ(uiPath, option_->GetUIPath());
}

/**
 * @tc.name: SetStorage
 * @tc.desc: Test SetStorage with null storage
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetStorage, TestSize.Level1)
{
    std::shared_ptr<NativeReference> storage = nullptr;
    option_->SetStorage(storage);
    EXPECT_EQ(nullptr, option_->GetStorage());
}

/**
 * @tc.name: SetRect
 * @tc.desc: Test SetRect with various rects
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetRect, TestSize.Level1)
{
    Rect rect {0, 0, 100, 100};
    option_->SetRect(rect);
    EXPECT_EQ(rect, option_->GetRect());

    rect = {0, 0, 0, 0};
    option_->SetRect(rect);
    EXPECT_EQ(rect, option_->GetRect());

    rect = {-100, -100, 100, 100};
    option_->SetRect(rect);
    EXPECT_EQ(rect, option_->GetRect());

    rect = {1000, 1000, 2000, 2000};
    option_->SetRect(rect);
    EXPECT_EQ(rect, option_->GetRect());
}

/**
 * @tc.name: SetVisibilityInApp
 * @tc.desc: Test SetVisibilityInApp with true and false
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetVisibilityInApp, TestSize.Level1)
{
    option_->SetVisibilityInApp(true);
    EXPECT_TRUE(option_->GetVisibilityInApp());

    option_->SetVisibilityInApp(false);
    EXPECT_FALSE(option_->GetVisibilityInApp());
}

/**
 * @tc.name: SetContext
 * @tc.desc: Test SetContext with null and valid pointer
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetContext, TestSize.Level1)
{
    void* contextPtr = nullptr;
    option_->SetContext(contextPtr);
    EXPECT_EQ(nullptr, option_->GetContext());

    int dummy = 42;
    contextPtr = &dummy;
    option_->SetContext(contextPtr);
    EXPECT_EQ(contextPtr, option_->GetContext());
}

/**
 * @tc.name: SetShowWhenCreate
 * @tc.desc: Test SetShowWhenCreate with true and false
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, SetShowWhenCreate, TestSize.Level1)
{
    option_->SetShowWhenCreate(true);
    EXPECT_TRUE(option_->GetShowWhenCreate());

    option_->SetShowWhenCreate(false);
    EXPECT_FALSE(option_->GetShowWhenCreate());
}

/**
 * @tc.name: IsRectValid
 * @tc.desc: Test IsRectValid with various rects
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, IsRectValid, TestSize.Level1)
{
    Rect rect {0, 0, 100, 100};
    option_->SetRect(rect);
    EXPECT_TRUE(option_->IsRectValid());

    rect = {0, 0, 0, 100};
    option_->SetRect(rect);
    EXPECT_FALSE(option_->IsRectValid());

    rect = {0, 0, 100, 0};
    option_->SetRect(rect);
    EXPECT_FALSE(option_->IsRectValid());

    rect = {0, 0, 0, 100};
    option_->SetRect(rect);
    EXPECT_FALSE(option_->IsRectValid());

    rect = {0, 0, 100, 0};
    option_->SetRect(rect);
    EXPECT_FALSE(option_->IsRectValid());
}

/**
 * @tc.name: IsUIPathValid
 * @tc.desc: Test IsUIUIPathValid with valid and empty path
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, IsUIPathValid, TestSize.Level1)
{
    std::string uiPath = "/path/to/ui";
    option_->SetUIPath(uiPath);
    EXPECT_TRUE(option_->IsUIPathValid());

    uiPath = "";
    option_->SetUIPath(uiPath);
    EXPECT_FALSE(option_->IsUIPathValid());
}

/**
 * @tc.name: GetFvTemplateInfo
 * @tc.desc: Test GetFvTemplateInfo with all fields set and default values
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, GetFvTemplateInfo, TestSize.Level1)
{
    uint32_t templateType = 1;
    std::string uiPath = "/path/to/ui";
    Rect rect {0, 0, 100, 100};
    bool visibleInApp = true;
    bool showWhenCreate = false;

    option_->SetTemplate(templateType);
    option_->SetUIPath(uiPath);
    option_->SetRect(rect);
    option_->SetVisibilityInApp(visibleInApp);
    option_->SetShowWhenCreate(showWhenCreate);

    FloatViewTemplateInfo templateInfo;
    option_->GetFvTemplateInfo(templateInfo);

    EXPECT_EQ(templateType, templateInfo.template_);
    EXPECT_EQ(visibleInApp, templateInfo.visibleInApp_);
    EXPECT_EQ(rect, templateInfo.rect_);
    EXPECT_EQ(showWhenCreate, templateInfo.showWhenCreate_);

    option_ = sptr<FvOption>::MakeSptr();
    option_->GetFvTemplateInfo(templateInfo);

    Rect targetRect {0, 0, 0, 0};
    EXPECT_EQ(0, templateInfo.template_);
    EXPECT_TRUE(templateInfo.visibleInApp_);
    EXPECT_EQ(targetRect, templateInfo.rect_);
    EXPECT_TRUE(templateInfo.showWhenCreate_);
}

/**
 * @tc.name: MultipleSetAndGet
 * @tc.desc: Test multiple set and get operations
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, MultipleSetAndGet, TestSize.Level1)
{
    option_->SetTemplate(1);
    EXPECT_EQ(1, option_->GetTemplate());

    option_->SetTemplate(2);
    EXPECT_EQ(2, option_->GetTemplate());

    option_->SetUIPath("/path1");
    EXPECT_EQ("/path1", option_->GetUIPath());

    option_->SetUIPath("/path2");
    EXPECT_EQ("/path2", option_->GetUIPath());

    option_->SetVisibilityInApp(true);
    EXPECT_TRUE(option_->GetVisibilityInApp());

    option_->SetVisibilityInApp(false);
    EXPECT_FALSE(option_->GetVisibilityInApp());
}

/**
 * @tc.name: AllFieldsTest
 * @tc.desc: Test setting all fields
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, AllFieldsTest, TestSize.Level1)
{
    uint32_t templateType = 3;
    std::string uiPath = "/full/path/to/ui";
    Rect rect {50, 50, 200, 300};
    bool visibleInApp = false;
    bool showWhenCreate = true;
    int dummy = 42;
    void* contextPtr = &dummy;

    option_->SetTemplate(templateType);
    option_->SetUIPath(uiPath);
    option_->SetRect(rect);
    option_->SetVisibilityInApp(visibleInApp);
    option_->SetContext(contextPtr);
    option_->SetShowWhenCreate(showWhenCreate);

    EXPECT_EQ(templateType, option_->GetTemplate());
    EXPECT_EQ(uiPath, option_->GetUIPath());
    EXPECT_EQ(visibleInApp, option_->GetVisibilityInApp());
    EXPECT_EQ(contextPtr, option_->GetContext());
    EXPECT_EQ(showWhenCreate, option_->GetShowWhenCreate());
    EXPECT_EQ(rect, option_->GetRect());
}

/**
 * @tc.name: EdgeCasesTest
 * @tc.desc: Test edge cases for all setters
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewOptionTest, EdgeCasesTest, TestSize.Level1)
{
    option_->SetTemplate(UINT32_MAX);
    EXPECT_EQ(UINT32_MAX, option_->GetTemplate());

    option_->SetUIPath("a");
    EXPECT_EQ("a", option_->GetUIPath());

    Rect rect1 {0, 0, 1, 1};
    option_->SetRect(rect1);
    EXPECT_TRUE(option_->IsRectValid());

    Rect rect2 {0, 0, 1, 0};
    option_->SetRect(rect2);
    EXPECT_FALSE(option_->IsRectValid());
}

}
}
}
