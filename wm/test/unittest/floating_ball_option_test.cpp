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

#include <gmock/gmock.h>
#include "parameters.h"
#include "floating_ball_option.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class FloatingBallOptionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FloatingBallOptionTest::SetUpTestCase()
{
}

void FloatingBallOptionTest::TearDownTestCase()
{
}

void FloatingBallOptionTest::SetUp()
{
}

void FloatingBallOptionTest::TearDown()
{
}

namespace {
/**
 * @tc.name: TestParamSetAndGet
 * @tc.desc: TestParamSetAndGet
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallOptionTest, TestParamSetAndGet, TestSize.Level1)
{
    auto option = sptr<FbOption>::MakeSptr();
    uint32_t templateType = 1;
    std::string title = "title";
    std::string content = "content";
    std::string backgroundColor = "#RRGGBB";
    std::string titleColor = "#RGGGBB";
    std::string contentColor = "#RRGGGB";
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    option->SetTemplate(templateType);
    option->SetTitle(title);
    option->SetContent(content);
    option->SetBackgroundColor(backgroundColor);
    option->SetTitleColor(titleColor);
    option->SetContentColor(contentColor);
    option->SetIcon(icon);
    uint32_t textUpdateAnimationType = 0;
    option->SetTextUpdateAnimationType(textUpdateAnimationType);
    EXPECT_EQ(templateType, option->GetTemplate());
    EXPECT_EQ(title, option->GetTitle());
    EXPECT_EQ(content, option->GetContent());
    EXPECT_EQ(nullptr, option->GetIcon());
    EXPECT_EQ(backgroundColor, option->GetBackgroundColor());
    EXPECT_EQ(titleColor, option->GetTitleColor());
    EXPECT_EQ(contentColor, option->GetContentColor());
    EXPECT_EQ(textUpdateAnimationType, option->GetTextUpdateAnimationType());

    FloatingBallTemplateBaseInfo fbTemplateBaseInfo;
    option->GetFbTemplateBaseInfo(fbTemplateBaseInfo);
    EXPECT_EQ(title, fbTemplateBaseInfo.title_);
    EXPECT_EQ(templateType, fbTemplateBaseInfo.template_);
    EXPECT_EQ(content, fbTemplateBaseInfo.content_);
    EXPECT_EQ(backgroundColor, fbTemplateBaseInfo.backgroundColor_);
    EXPECT_EQ(titleColor, fbTemplateBaseInfo.titleColor_);
    EXPECT_EQ(contentColor, fbTemplateBaseInfo.contentColor_);
    EXPECT_EQ(textUpdateAnimationType, fbTemplateBaseInfo.textUpdateAnimationType_);
}
/**
 * @tc.name: TestTextUpdateAnimationTypeSetAndGet
 * @tc.desc: TestTextUpdateAnimationTypeSetAndGet
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallOptionTest, TestTextUpdateAnimationTypeSetAndGet, TestSize.Level1)
{
    auto option = sptr<FbOption>::MakeSptr();
    uint32_t animateType = 0;
    option->SetTextUpdateAnimationType(animateType);
    EXPECT_EQ(animateType, option->GetTextUpdateAnimationType());

    animateType = 1;
    option->SetTextUpdateAnimationType(animateType);
    EXPECT_EQ(animateType, option->GetTextUpdateAnimationType());
}
/**
 * @tc.name: TestTextUpdateAnimationTypeInTemplateBaseInfo
 * @tc.desc: TestTextUpdateAnimationTypeInTemplateBaseInfo
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallOptionTest, TestTextUpdateAnimationTypeInTemplateBaseInfo, TestSize.Level1)
{
    auto option = sptr<FbOption>::MakeSptr();
    uint32_t animateType = 1;
    option->SetTextUpdateAnimationType(animateType);

    FloatingBallTemplateBaseInfo fbTemplateBaseInfo;
    option->GetFbTemplateBaseInfo(fbTemplateBaseInfo);
    EXPECT_EQ(animateType, fbTemplateBaseInfo.textUpdateAnimationType_);
}

/**
 * @tc.name: TestInvalidColor
 * @tc.desc: TestInvalidColor
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallOptionTest, TestInvalidColor, TestSize.Level1)
{
    auto option = sptr<FbOption>::MakeSptr();
    uint32_t templateType = 2;
    std::string title = "title";
    std::string content = "content";
    std::string backgroundColor = "#ffdd395e";
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    uint32_t textUpdateAnimationType = 0;
    std::string s = "";
    option->SetTemplate(templateType);
    option->SetTitle(title);
    option->SetContent(content);
    option->SetIcon(icon);
    option->SetBackgroundColor(backgroundColor);
    option->SetTextUpdateAnimationType(textUpdateAnimationType);
 
    EXPECT_TRUE(option->IsValid(s));
 
    std::string invalidColor = "1234";
    option->SetTitleColor(invalidColor);
    EXPECT_FALSE(option->IsValid(s));
    EXPECT_EQ(s, "TitleColor is invalid");
 
    option->SetTitleColor("");
    option->SetContentColor(invalidColor);
    EXPECT_FALSE(option->IsValid(s));
    EXPECT_EQ(s, "ContentColor is invalid");
}
 
/**
 * @tc.name: TestInvalidColor
 * @tc.desc: TestInvalidColor
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallOptionTest, TestTextColor, TestSize.Level1)
{
    auto option = sptr<FbOption>::MakeSptr();
    uint32_t templateType = 2;
    std::string title = "title";
    std::string content = "content";
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    uint32_t textUpdateAnimationType = 0;
    std::string s = "";
    option->SetTemplate(templateType);
    option->SetTitle(title);
    option->SetContent(content);
    option->SetIcon(icon);
    option->SetTextUpdateAnimationType(textUpdateAnimationType);
 
    std::string color = "#ffdd395e";
    option->SetTitleColor(color);
    EXPECT_FALSE(option->IsValid(s));
    EXPECT_EQ(s, "When setting the text color, the background color must be set");
 
    option->SetContentColor(color);
    EXPECT_FALSE(option->IsValid(s));
    EXPECT_EQ(s, "When setting the text color, the background color must be set");
 
    option->SetTitleColor("");
    EXPECT_FALSE(option->IsValid(s));
    EXPECT_EQ(s, "When setting the text color, the background color must be set");
 
    option->SetContentColor("");
    EXPECT_TRUE(option->IsValid(s));
 
    option->SetBackgroundColor(color);
    option->SetContentColor(color);
    EXPECT_TRUE(option->IsValid(s));
 
    option->SetContentColor("");
    option->SetTitleColor(color);
    EXPECT_TRUE(option->IsValid(s));
}
}
}
}
