/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_scene_session_manager.h"

#include <ui_content.h>

#include "ani.h"
#include "ani_scene_session_utils.h"
#include "root_scene.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

ani_status AniSceneSessionManager::Init(ani_env* env, ani_namespace nsp)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] AniSceneSessionManager Init");
    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(nsp, "setSceneSessionManagerRef", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setSceneSessionManagerRef func fail %{public}u", ret);
        return ret;
    }
    std::unique_ptr<AniSceneSessionManager> aniSceneSessionManager = std::make_unique<AniSceneSessionManager>();
    ret = env->Function_Call_Void(setObjFunc, aniSceneSessionManager.get());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setSceneSessionManagerRef func fail %{public}u", ret);
        return ret;
    }
    return ret;
}

ani_object AniSceneSessionManager::GetRootSceneUIContext(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] AniSceneSessionManager GetRootSceneUIContext");
    AniSceneSessionManager* sceneSessionManager = reinterpret_cast<AniSceneSessionManager*>(nativeObj);
    return sceneSessionManager->OnGetRootSceneUIContext(env);
}

ani_object AniSceneSessionManager::OnGetRootSceneUIContext(ani_env* env)
{
    if (RootScene::staticRootScene_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Root scene is nullptr");
        return AniSceneSessionUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    const auto& uiContent = RootScene::staticRootScene_->GetUIContent();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] uiContent is nullptr");
        return AniSceneSessionUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    ani_object uiContext = uiContent->GetUIAniContext();
    if (uiContext == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] uiContext obtained from jsEngine is nullptr");
        return AniSceneSessionUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return uiContext;
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    ani_status ret;
    ani_env *env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return ANI_NOT_FOUND;
    }
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.sceneSessionManager.sceneSessionManager", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array funcs = {
        ani_native_function {"getRootSceneUIContextSync", "l:C{@ohos.arkui.UIContext.UIContext}",
            reinterpret_cast<void *>(AniSceneSessionManager::GetRootSceneUIContext)}
    };
    if ((ret = env->Namespace_BindNativeFunctions(nsp, funcs.data(), funcs.size()))) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] bind namespace fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    AniSceneSessionManager::Init(env, nsp);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
}

}  // namespace Rosen
}  // namespace OHOS