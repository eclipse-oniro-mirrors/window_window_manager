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

#ifndef OHOS_ROSEN_WINDOW_SCENE_ROOT_SCENE_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_ROOT_SCENE_SESSION_H

#include "session/host/include/scene_session.h"

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;
namespace OHOS::AbilityRuntime {
class Context;
} // namespace OHOS::AbilityRuntime

namespace OHOS::Rosen {
class RootSceneSession : public SceneSession {
public:
    using LoadContentFunc =
        std::function<void(const std::string&, napi_env, napi_value, AbilityRuntime::Context*)>;
    using GetUIContentFunc = std::function<Ace::UIContent*(DisplayId displayId)>;
    RootSceneSession() : SceneSession({}, nullptr) {}
    explicit RootSceneSession(const sptr<SpecificSessionCallback>& specificCallback)
        : SceneSession({}, specificCallback) {}
    virtual ~RootSceneSession() = default;

    void SetLoadContentFunc(const LoadContentFunc& loadContentFunc);
    void LoadContent(
        const std::string& contentUrl, napi_env env, napi_value storage, AbilityRuntime::Context* context);
    void SetGetUIContentFunc(GetUIContentFunc&& getUIContentFunc);
    Ace::UIContent* GetUIContent(DisplayId displayId);

    /*
     * Window Layout
     */
    void SetRootSessionRect(const WSRect& rect);

    /*
     * Window Immersive
     */
    AvoidArea GetAvoidAreaByType(AvoidAreaType type, const WSRect& rect = WSRect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) override;
    AvoidArea GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type,
        const WSRect& rect = WSRect::EMPTY_RECT) override;
    WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;

private:
    LoadContentFunc loadContentFunc_;
    GetUIContentFunc getUIContentFunc_;

    /*
     * Window Immersive
     */
    AvoidArea GetAvoidAreaByTypeInner(AvoidAreaType type, bool ignoreVisibility = false);
    void GetSystemAvoidAreaForRoot(const WSRect& rect, AvoidArea& avoidArea, bool ignoreVisibility = false);
    void GetCutoutAvoidAreaForRoot(const WSRect& rect, AvoidArea& avoidArea);
    void GetKeyboardAvoidAreaForRoot(const WSRect& rect, AvoidArea& avoidArea);
    void GetAINavigationBarAreaForRoot(const WSRect& rect, AvoidArea& avoidArea, bool ignoreVisibility = false);
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_ROOT_SCENE_SESSION_H
