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

#include "ani_embeddable_window_stage.h"

#include "ani.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

namespace {
/* used for free, ani has no destructor right now, only free when aniObj freed */
static std::map<ani_object, AniEmbeddableWindowStage*> localObjs;
} // namespace

AniEmbeddableWindowStage::AniEmbeddableWindowStage(sptr<Rosen::Window> window, sptr<AAFwk::SessionInfo> sessionInfo)
    : windowExtensionSessionImpl_(window), sessionInfo_(sessionInfo) {}

void AniEmbeddableWindowStage::LoadContent(void* env, std::string content)
{
}

void DropEmbeddableWindowStageByAni(ani_object aniObj)
{
    auto obj = localObjs.find(reinterpret_cast<ani_object>(aniObj));
    if (obj != localObjs.end()) {
        delete obj->second;
        localObjs.erase(obj);
    }
}

AniEmbeddableWindowStage* GetEmbeddableWindowStageFromAni(void* aniObj)
{
    auto obj = localObjs.find(reinterpret_cast<ani_object>(aniObj));
    if (obj == localObjs.end()) {
        return nullptr;
    }
    return obj->second;
}

ani_object CreateAniEmbeddableWindowStage(ani_env* env, sptr<Rosen::Window> window,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] null env");
        return nullptr;
    }
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI] create wstage");

    ani_status ret;
    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowStageInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] null env %{public}u", ret);
        return cls;
    }

    std::unique_ptr<AniEmbeddableWindowStage> aniEmbeddableWindowStage =
        std::make_unique<AniEmbeddableWindowStage>(window, sessionInfo);

    ani_field contextField;
    if ((ret = env->Class_FindField(cls, "nativeWindowStage", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] get field fail %{public}u", ret);
        return nullptr;
    }

    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] get ctor fail %{public}u", ret);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] obj new fail %{public}u", ret);
        return nullptr;
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] get ctor fail %{public}u", ret);
        return nullptr;
    }
    env->Object_CallMethod_Void(obj, setObjFunc, aniEmbeddableWindowStage.get());
    localObjs.insert(std::pair(obj, aniEmbeddableWindowStage.release()));

    return obj;
}

AniEmbeddableWindowStage* GetEmbeddableWindowStageFromEnv(ani_env* env, ani_class cls, ani_object obj)
{
    ani_field nativeObjName {};
    ani_status ret;
    if ((ret = env->Class_FindField(cls, "nativeObj", &nativeObjName)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] obj fetch field %{public}u", ret);
        return nullptr;
    }
    ani_long nativeObj {};
    if ((ret = env->Object_GetField_Long(obj, nativeObjName, &nativeObj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] obj fetch long %{public}u", ret);
        return nullptr;
    }
    return reinterpret_cast<AniEmbeddableWindowStage*>(nativeObj);
}
}  // namespace Rosen
}  // namespace OHOS