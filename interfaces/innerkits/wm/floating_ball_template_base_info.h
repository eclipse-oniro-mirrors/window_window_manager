/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FLOATING_BALL_TEMPLATE_BASE_INFO_H
#define OHOS_FLOATING_BALL_TEMPLATE_BASE_INFO_H

#include "wm_common.h"

namespace OHOS::Rosen {
class FloatingBallTemplateBaseInfo {
public:
    FloatingBallTemplateBaseInfo() = default;
    FloatingBallTemplateBaseInfo(const uint32_t& templateType, const std::string& title,
        const std::string& content, const std::string& color, const bool isVisibleInApp,
        uint32_t textUpdateAnimationType, bool isBind, uint32_t bindWindowId, bool showWhenCreate,
        const std::string &id) : template_(templateType),
        title_(title), content_(content), backgroundColor_(color), isVisibleInApp_(isVisibleInApp),
        textUpdateAnimationType_(textUpdateAnimationType), isBind_(isBind),
        bindWindowId_(bindWindowId), showWhenCreate_(showWhenCreate),
        id_(id) {};
    virtual ~FloatingBallTemplateBaseInfo() = default;

    FloatingBallTemplateBaseInfo(const FloatingBallTemplateBaseInfo& other) =  default;
    FloatingBallTemplateBaseInfo& operator =(const FloatingBallTemplateBaseInfo& other) =  default;

    uint32_t template_ {};
    std::string title_ {};
    std::string content_ {};
    std::string backgroundColor_ {};
    bool isVisibleInApp_ = true;
    uint32_t textUpdateAnimationType_ {};
    bool isBind_ {false};
    uint32_t bindWindowId_ {INVALID_WINDOW_ID};
    bool showWhenCreate_ { true };
    std::string id_ {};
    uint32_t updateMode_ = static_cast<uint32_t>(FloatingBallUpdateMode::DEFAULT);
};
} // namespace OHOS::Rosen
#endif // OHOS_FLOATING_BALL_TEMPLATE_BASE_INFO_H