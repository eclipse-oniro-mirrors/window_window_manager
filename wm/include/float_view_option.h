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
 
#ifndef OHOS_FV_OPTION_H
#define OHOS_FV_OPTION_H
#include <refbase.h>
#include <string>

#include "wm_common.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "float_view_template_info.h"
#include "ani.h"

namespace OHOS {
namespace Rosen {
class FvOption : public RefBase {
public:
    explicit FvOption();
    virtual ~FvOption() = default;
 
    void SetTemplate(uint32_t type);
    void SetUIPath(const std::string& uiPath);
    void SetStorage(const std::shared_ptr<NativeReference>& storageRef);
    void SetStorage(ani_env* env, const ani_ref& storageRef);
    void SetRect(const Rect& rect);
    void SetVisibilityInApp(bool visible);
    void SetContext(void* contextPtr);
    void SetShowWhenCreate(bool showWhenCreate);
    void ClearAniReference(ani_env* env);
    void SetCloseConfirm(bool closeConfirm);

    uint32_t GetTemplate() const;
    const std::string& GetUIPath() const;
    std::shared_ptr<NativeReference> GetStorage() const;
    ani_ref GetAniStorage() const;
    Rect GetRect() const;
    bool GetVisibilityInApp() const;
    void* GetContext() const;
    bool GetShowWhenCreate() const;

    bool IsRectValid() const;
    bool IsUIPathValid() const;

    void GetFvTemplateInfo(FloatViewTemplateInfo& fvTemplateInfo) const;
    bool isLoadUIByName {false};
private:
    uint32_t template_ {};
    std::string uiPath_ {};
    std::shared_ptr<NativeReference> storage_ = nullptr;
    ani_ref aniStorage_ = nullptr;
    Rect rect_ {};
    bool visibleInApp_ {true};
    void* contextPtr_ = nullptr;
    bool showWhenCreate_ {true};
    bool closeConfirm_ = false;
};
}
}
#endif //OHOS_FV_OPTION_H