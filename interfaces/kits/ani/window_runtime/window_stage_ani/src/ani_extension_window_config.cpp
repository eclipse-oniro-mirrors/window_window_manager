/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed AniExtensionWindowConfig::on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_extension_window_config.h"
#include <string>
#include "ani_window_utils.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
static std::map<ani_ref, AniExtensionWindowConfig*> localObjs;

const std::map<ApiWindowType, std::string> API_TO_ANI_STRING_TYPE_MAP {
    {ApiWindowType::TYPE_BASE,                 "TYPE_APP"                  },
    {ApiWindowType::TYPE_APP,                  "TYPE_APP"                  },
    {ApiWindowType::TYPE_SYSTEM_ALERT,         "TYPE_SYSTEM_ALERT"         },
    {ApiWindowType::TYPE_INPUT_METHOD,         "TYPE_INPUT_METHOD"         },
    {ApiWindowType::TYPE_STATUS_BAR,           "TYPE_STATUS_BAR"           },
    {ApiWindowType::TYPE_PANEL,                "TYPE_PANEL"                },
    {ApiWindowType::TYPE_KEYGUARD,             "TYPE_KEYGUARD"             },
    {ApiWindowType::TYPE_VOLUME_OVERLAY,       "TYPE_VOLUME_OVERLAY"       },
    {ApiWindowType::TYPE_NAVIGATION_BAR,       "TYPE_NAVIGATION_BAR"       },
    {ApiWindowType::TYPE_FLOAT,                "TYPE_FLOAT"                },
    {ApiWindowType::TYPE_WALLPAPER,            "TYPE_WALLPAPER"            },
    {ApiWindowType::TYPE_DESKTOP,              "TYPE_DESKTOP"              },
    {ApiWindowType::TYPE_LAUNCHER_RECENT,      "TYPE_LAUNCHER_RECENT"      },
    {ApiWindowType::TYPE_LAUNCHER_DOCK,        "TYPE_LAUNCHER_DOCK"        },
    {ApiWindowType::TYPE_VOICE_INTERACTION,    "TYPE_VOICE_INTERACTION"    },
    {ApiWindowType::TYPE_POINTER,              "TYPE_POINTER"              },
    {ApiWindowType::TYPE_FLOAT_CAMERA,         "TYPE_FLOAT_CAMERA"         },
    {ApiWindowType::TYPE_DIALOG,               "TYPE_DIALOG"               },
    {ApiWindowType::TYPE_SCREENSHOT,           "TYPE_SCREENSHOT"           },
    {ApiWindowType::TYPE_SYSTEM_TOAST,         "TYPE_SYSTEM_TOAST"         },
    {ApiWindowType::TYPE_DIVIDER,              "TYPE_DIVIDER"              },
    {ApiWindowType::TYPE_GLOBAL_SEARCH,        "TYPE_GLOBAL_SEARCH"        },
    {ApiWindowType::TYPE_HANDWRITE,            "TYPE_HANDWRITE"            },
};
} // namespace

ani_enum_item GetAniModalityType(ani_env* env, ModalityType enumObj)
{
    ani_enum enumType;
    if (ANI_OK != env->FindEnum("@ohos.window.window.ModalityType", &enumType)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ModalityType not found");
        return nullptr;
    }
    ani_enum_item enumItem = nullptr;
    env->Enum_GetEnumItemByIndex(enumType, static_cast<ani_int>(enumObj), &enumItem);

    return enumItem;
}

ani_object CreatAniSubWindowOptions(ani_env* env, const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig)
{
    if (extensionWindowConfig == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ExtensionWindowConfig is null");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.ExtConfigSubWindowOptions", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniOptions;
    ret = env->Object_New(aniClass, aniCtor, &aniOptions);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(aniClass, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find method failed, ret: %{public}u", ret);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::unique_ptr<AniExtensionWindowConfig> config =
        std::make_unique<AniExtensionWindowConfig>(extensionWindowConfig);
    env->Object_CallMethod_Void(aniOptions, setObjFunc, reinterpret_cast<ani_long>(config.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(aniOptions, &ref) == ANI_OK) {
        config->SetAniRef(ref);
        localObjs.insert(std::pair(ref, config.release()));
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] create global ref fail");
    }

    SubWindowOptions subWindowOptions = extensionWindowConfig->subWindowOptions;
    AniWindowUtils::CallAniMethodVoid(env, aniOptions, aniClass, "<set>zLevel", nullptr,
        AniWindowUtils::CreateOptionalInt(env, static_cast<ani_int>(subWindowOptions.zLevel)));
    AniWindowUtils::CallAniMethodVoid(env, aniOptions, aniClass, "<set>maximizeSupported", nullptr,
        AniWindowUtils::CreateOptionalBool(env, static_cast<ani_boolean>(subWindowOptions.maximizeSupported)));
    AniWindowUtils::CallAniMethodVoid(env, aniOptions, aniClass, "<set>modalityType", nullptr,
        GetAniModalityType(env, subWindowOptions.modalityType));

    return aniOptions;
}

ani_enum_item GetAniWindowType(ani_env* env, int32_t apiType)
{
    ApiWindowType type = ApiWindowType::TYPE_BASE;
    if (apiType >= static_cast<int32_t>(ApiWindowType::TYPE_BASE) &&
        apiType < static_cast<int32_t>(ApiWindowType::TYPE_END)) {
        type = static_cast<ApiWindowType>(apiType);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] invalid windowType: %{public}d", apiType);
    }
    std::string typeName = "TYPE_APP";
    if (API_TO_ANI_STRING_TYPE_MAP.count(type) != 0) {
        typeName = API_TO_ANI_STRING_TYPE_MAP.at(type);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] invalid ApiWindowType: %{public}d", static_cast<int32_t>(type));
    }

    ani_enum enumType;
    if (ANI_OK != env->FindEnum("@ohos.window.window.WindowType", &enumType)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] WindowType not found");
        return nullptr;
    }
    ani_enum_item enumItem = nullptr;
    env->Enum_GetEnumItemByName(enumType, typeName.c_str(), &enumItem);

    return enumItem;
}

ani_object CreatAniSystemWindowOptions(ani_env* env,
    const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig)
{
    if (extensionWindowConfig == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ExtensionWindowConfig is null");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.ExtConfigSystemWindowOptions", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniOptions;
    ret = env->Object_New(aniClass, aniCtor, &aniOptions);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(aniClass, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find method failed, ret: %{public}u", ret);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::unique_ptr<AniExtensionWindowConfig> config =
        std::make_unique<AniExtensionWindowConfig>(extensionWindowConfig);
    env->Object_CallMethod_Void(aniOptions, setObjFunc, reinterpret_cast<ani_long>(config.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(aniOptions, &ref) == ANI_OK) {
        config->SetAniRef(ref);
        localObjs.insert(std::pair(ref, config.release()));
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] create global ref fail");
    }
    
    return aniOptions;
}

ani_enum_item GetAniExtensionWindowAttribute(ani_env* env, ExtensionWindowAttribute enumObj)
{
    ani_enum enumType;
    if (ANI_OK != env->FindEnum("@ohos.window.window.ExtensionWindowAttribute", &enumType)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ExtensionWindowAttribute not found");
        return nullptr;
    }
    ani_enum_item enumItem = nullptr;
    env->Enum_GetEnumItemByIndex(enumType, static_cast<ani_int>(enumObj), &enumItem);

    return enumItem;
}

ani_object CreatAniRect(ani_env* env, const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig)
{
    if (extensionWindowConfig == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ExtensionWindowConfig is null");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.ExtConfigRect", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", ":", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniRect;
    ret = env->Object_New(aniClass, aniCtor, &aniRect);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to create new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(aniClass, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find method failed, ret: %{public}u", ret);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::unique_ptr<AniExtensionWindowConfig> config =
        std::make_unique<AniExtensionWindowConfig>(extensionWindowConfig);
    env->Object_CallMethod_Void(aniRect, setObjFunc, reinterpret_cast<ani_long>(config.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(aniRect, &ref) == ANI_OK) {
        config->SetAniRef(ref);
        localObjs.insert(std::pair(ref, config.release()));
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] create global ref fail");
    }
    
    return aniRect;
}

ani_object CreateAniExtensionWindowConfig(ani_env* env,
    const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.ExtensionWindowConfigInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniConfig;
    ret = env->Object_New(aniClass, aniCtor, &aniConfig);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::unique_ptr<AniExtensionWindowConfig> config =
        std::make_unique<AniExtensionWindowConfig>(extensionWindowConfig);
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(aniClass, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find method failed, ret: %{public}u", ret);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    env->Object_CallMethod_Void(aniConfig, setObjFunc, reinterpret_cast<ani_long>(config.get()));
    ani_ref ref = nullptr;
    if (env->GlobalReference_Create(aniConfig, &ref) == ANI_OK) {
        config->SetAniRef(ref);
        localObjs.insert(std::pair(ref, config.release()));
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] create global ref fail");
    }

    return aniConfig;
}

AniExtensionWindowConfig::AniExtensionWindowConfig(const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig)
    : extensionWindowConfig_(extensionWindowConfig)
{
}

AniExtensionWindowConfig::~AniExtensionWindowConfig() = default;

void AniExtensionWindowConfig::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* config = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (config != nullptr) {
        auto obj = localObjs.find(reinterpret_cast<ani_ref>(config->GetAniRef()));
        if (obj != localObjs.end()) {
            delete obj->second;
            localObjs.erase(obj);
        }
        env->GlobalReference_Delete(config->GetAniRef());
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] aniConfig is nullptr");
    }
}

ani_string AniExtensionWindowConfig::OnGetWindowName(ani_env* env)
{
    ani_string windowName;
    AniWindowUtils::GetAniString(env, extensionWindowConfig_->windowName, &windowName);
    return windowName;
}

ani_enum_item AniExtensionWindowConfig::OnGetWindowAttribute(ani_env* env)
{
    return GetAniExtensionWindowAttribute(env, extensionWindowConfig_->windowAttribute);
}

ani_object AniExtensionWindowConfig::OnGetWindowRect(ani_env* env)
{
    return CreatAniRect(env, extensionWindowConfig_);
}

ani_int AniExtensionWindowConfig::OnGetWindowRectLeft(ani_env* env)
{
    return static_cast<ani_int>(extensionWindowConfig_->windowRect.posX_);
}

ani_int AniExtensionWindowConfig::OnGetWindowRectTop(ani_env* env)
{
    return static_cast<ani_int>(extensionWindowConfig_->windowRect.posY_);
}

ani_int AniExtensionWindowConfig::OnGetWindowRectWidth(ani_env* env)
{
    return static_cast<ani_int>(extensionWindowConfig_->windowRect.width_);
}

ani_int AniExtensionWindowConfig::OnGetWindowRectHeight(ani_env* env)
{
    return static_cast<ani_int>(extensionWindowConfig_->windowRect.height_);
}

ani_object AniExtensionWindowConfig::OnGetSubWindowOptions(ani_env* env)
{
    return CreatAniSubWindowOptions(env, extensionWindowConfig_);
}

ani_string AniExtensionWindowConfig::OnGetSubWindowOptionsTitle(ani_env* env)
{
    ani_string title;
    AniWindowUtils::GetAniString(env, extensionWindowConfig_->subWindowOptions.title, &title);
    return title;
}

ani_boolean AniExtensionWindowConfig::OnGetSubWindowOptionsDecorEnabled(ani_env* env)
{
    return static_cast<ani_boolean>(extensionWindowConfig_->subWindowOptions.decorEnabled);
}

ani_boolean AniExtensionWindowConfig::OnGetSubWindowOptionsIsModal(ani_env* env)
{
    return static_cast<ani_boolean>(extensionWindowConfig_->subWindowOptions.isModal);
}

ani_boolean AniExtensionWindowConfig::OnGetSubWindowOptionsIsTopmost(ani_env* env)
{
    return static_cast<ani_boolean>(extensionWindowConfig_->subWindowOptions.isTopmost);
}

ani_object AniExtensionWindowConfig::OnGetSystemWindowOptions(ani_env* env)
{
    return CreatAniSystemWindowOptions(env, extensionWindowConfig_);
}

ani_enum_item AniExtensionWindowConfig::OnGetSystemWindowOptionsWindowType(ani_env* env)
{
    return GetAniWindowType(env, extensionWindowConfig_->systemWindowOptions.windowType);
}

void AniExtensionWindowConfig::OnSetWindowName(ani_env* env, ani_string value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    std::string result;
    AniWindowUtils::GetStdString(env, value, result);
    extensionWindowConfig_->windowName = result;
}

void AniExtensionWindowConfig::OnSetWindowAttribute(ani_env* env, ani_enum_item value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_int enumValue;
    env->EnumItem_GetValue_Int(value, &enumValue);
    extensionWindowConfig_->windowAttribute = static_cast<ExtensionWindowAttribute>(enumValue);
}

void AniExtensionWindowConfig::OnSetWindowRect(ani_env* env, ani_object value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    int32_t posX = 0;
    int32_t posY = 0;
    int32_t width = 0;
    int32_t height = 0;
    bool ret_bool = AniWindowUtils::GetIntObject(env, "left", value, posX);
    ret_bool &= AniWindowUtils::GetIntObject(env, "top", value, posY);
    ret_bool &= AniWindowUtils::GetIntObject(env, "width", value, width);
    ret_bool &= AniWindowUtils::GetIntObject(env, "height", value, height);
    if (!ret_bool) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Get rect pos failed");
        return;
    }
    extensionWindowConfig_->windowRect = { posX, posY, width, height };
}

void AniExtensionWindowConfig::OnSetWindowRectLeft(ani_env* env, ani_int value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->windowRect.posX_ = static_cast<int32_t>(value);
}

void AniExtensionWindowConfig::OnSetWindowRectTop(ani_env* env, ani_int value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->windowRect.posY_ = static_cast<int32_t>(value);
}

void AniExtensionWindowConfig::OnSetWindowRectWidth(ani_env* env, ani_int value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->windowRect.width_ = static_cast<uint32_t>(value);
}

void AniExtensionWindowConfig::OnSetWindowRectHeight(ani_env* env, ani_int value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->windowRect.height_ = static_cast<uint32_t>(value);
}

void AniExtensionWindowConfig::OnSetSubWindowOptions(ani_env* env, ani_object value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_boolean isUndefined;
    env->Reference_IsUndefined(value, &isUndefined);
    if (isUndefined) {
        TLOGI(WmsLogTag::DEFAULT, "SubWindowOptions is undefined ");
        extensionWindowConfig_->subWindowOptions.title = "";
        extensionWindowConfig_->subWindowOptions.decorEnabled = false;
        extensionWindowConfig_->subWindowOptions.isModal = false;
        extensionWindowConfig_->subWindowOptions.isTopmost = false;
        return;
    }

    ani_ref titleRef;
    env->Object_GetPropertyByName_Ref(value, "title", &titleRef);
    std::string title;
    AniWindowUtils::GetStdString(env, static_cast<ani_string>(titleRef), title);
    extensionWindowConfig_->subWindowOptions.title = title;
    
    ani_boolean decorEnabled = false;
    env->Object_GetPropertyByName_Boolean(value, "decorEnabled", &decorEnabled);
    extensionWindowConfig_->subWindowOptions.decorEnabled = decorEnabled;

    bool isModal = false;
    AniWindowUtils::GetPropertyBoolObject(env, "isModal", value, isModal);
    extensionWindowConfig_->subWindowOptions.isModal = isModal;

    bool isTopmost = false;
    AniWindowUtils::GetPropertyBoolObject(env, "isTopmost", value, isTopmost);
    extensionWindowConfig_->subWindowOptions.isTopmost = isTopmost;
}

void AniExtensionWindowConfig::OnSetSubWindowOptionsTitle(ani_env* env, ani_string value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    std::string title;
    AniWindowUtils::GetStdString(env, value, title);
    extensionWindowConfig_->subWindowOptions.title = title;
}

void AniExtensionWindowConfig::OnSetSubWindowOptionsDecorEnabled(ani_env* env, ani_boolean value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->subWindowOptions.decorEnabled = static_cast<bool>(value);
}

void AniExtensionWindowConfig::OnSetSubWindowOptionsIsModal(ani_env* env, ani_boolean value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->subWindowOptions.isModal = static_cast<bool>(value);
}

void AniExtensionWindowConfig::OnSetSubWindowOptionsIsTopmost(ani_env* env, ani_boolean value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    extensionWindowConfig_->subWindowOptions.isTopmost = static_cast<bool>(value);
}

void AniExtensionWindowConfig::OnSetSystemWindowOptions(ani_env* env, ani_object value)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_boolean isUndefined;
    env->Reference_IsUndefined(value, &isUndefined);
    if (isUndefined) {
        TLOGI(WmsLogTag::DEFAULT, "SystemWindowOptions is undefined ");
        extensionWindowConfig_->systemWindowOptions.windowType = -1;
        return;
    }
    ani_int ret;
    ani_ref result;
    env->Object_GetPropertyByName_Ref(value, "windowType", &result);
    env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(result), &ret);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] winType: %{public}u", static_cast<int32_t>(ret));
    extensionWindowConfig_->systemWindowOptions.windowType = static_cast<int32_t>(ret);
}

void AniExtensionWindowConfig::OnSetSystemWindowOptionsWindowType(ani_env* env, ani_enum_item value)
{
    ani_int ret;
    env->EnumItem_GetValue_Int(value, &ret);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] winType: %{public}u", static_cast<int32_t>(ret));
    extensionWindowConfig_->systemWindowOptions.windowType = static_cast<int32_t>(ret);
}

static ani_string GetWindowName(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        ani_string ret;
        AniWindowUtils::GetAniString(env, "", &ret);
        return ret;
    }

    return aniConfigPtr->OnGetWindowName(env);
}

static ani_enum_item GetWindowAttribute(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return nullptr;
    }

    return aniConfigPtr->OnGetWindowAttribute(env);
}

static ani_object GetWindowRect(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    return aniConfigPtr->OnGetWindowRect(env);
}

static ani_int GetWindowRectLeft(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_int>(0);
    }

    return aniConfigPtr->OnGetWindowRectLeft(env);
}

static ani_int GetWindowRectTop(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_int>(0);
    }

    return aniConfigPtr->OnGetWindowRectTop(env);
}

static ani_int GetWindowRectWidth(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_int>(0);
    }

    return aniConfigPtr->OnGetWindowRectWidth(env);
}

static ani_int GetWindowRectHeight(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_int>(0);
    }

    return aniConfigPtr->OnGetWindowRectHeight(env);
}

static ani_object GetSubWindowOptions(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    return aniConfigPtr->OnGetSubWindowOptions(env);
}

static ani_string GetSubWindowOptionsTitle(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        ani_string ret;
        AniWindowUtils::GetAniString(env, "", &ret);
        return ret;
    }

    return aniConfigPtr->OnGetSubWindowOptionsTitle(env);
}

static ani_boolean GetSubWindowOptionsDecorEnabled(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_boolean>(false);
    }

    return aniConfigPtr->OnGetSubWindowOptionsDecorEnabled(env);
}

static ani_boolean GetSubWindowOptionsIsModal(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_boolean>(false);
    }

    return aniConfigPtr->OnGetSubWindowOptionsIsModal(env);
}

static ani_boolean GetSubWindowOptionsIsTopmost(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return static_cast<ani_boolean>(false);
    }

    return aniConfigPtr->OnGetSubWindowOptionsIsTopmost(env);
}

static ani_object GetSystemWindowOptions(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    return aniConfigPtr->OnGetSystemWindowOptions(env);
}

static ani_enum_item GetSystemWindowOptionsWindowType(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return nullptr;
    }

    return aniConfigPtr->OnGetSystemWindowOptionsWindowType(env);
}

static void SetWindowName(ani_env* env, ani_object obj, ani_long nativeObj, ani_string value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowName(env, value);
}

static void SetWindowAttribute(ani_env* env, ani_object obj, ani_long nativeObj, ani_enum_item value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowAttribute(env, value);
}

static void SetWindowRect(ani_env* env, ani_object obj, ani_long nativeObj, ani_object value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowRect(env, value);
}

static void SetWindowRectLeft(ani_env* env, ani_object obj, ani_long nativeObj, ani_int value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowRectLeft(env, value);
}

static void SetWindowRectTop(ani_env* env, ani_object obj, ani_long nativeObj, ani_int value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowRectTop(env, value);
}

static void SetWindowRectWidth(ani_env* env, ani_object obj, ani_long nativeObj, ani_int value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowRectWidth(env, value);
}

static void SetWindowRectHeight(ani_env* env, ani_object obj, ani_long nativeObj, ani_int value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetWindowRectHeight(env, value);
}

static void SetSubWindowOptions(ani_env* env, ani_object obj, ani_long nativeObj, ani_object value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSubWindowOptions(env, value);
}

static void SetSubWindowOptionsTitle(ani_env* env, ani_object obj, ani_long nativeObj, ani_string value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSubWindowOptionsTitle(env, value);
}

static void SetSubWindowOptionsDecorEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSubWindowOptionsDecorEnabled(env, value);
}

static void SetSubWindowOptionsIsModal(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSubWindowOptionsIsModal(env, value);
}

static void SetSubWindowOptionsIsTopmost(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSubWindowOptionsIsTopmost(env, value);
}

static void SetSystemWindowOptions(ani_env* env, ani_object obj, ani_long nativeObj, ani_object value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSystemWindowOptions(env, value);
}

static void SetSystemWindowOptionsWindowType(ani_env* env, ani_object obj, ani_long nativeObj, ani_enum_item value)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniExtensionWindowConfig* aniConfigPtr = reinterpret_cast<AniExtensionWindowConfig*>(nativeObj);
    if (aniConfigPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]aniConfigPtr is nullptr");
        return;
    }

    aniConfigPtr->OnSetSystemWindowOptionsWindowType(env, value);
}
} // namespace Rosen
} // namespace OHOS

extern "C" {
using namespace OHOS::Rosen;
std::array extensionWindowConfigNativeMethods = {
    ani_native_function {"getWindowName", "l:C{std.core.String}", reinterpret_cast<void *>(GetWindowName)},
    ani_native_function {"getWindowAttribute", "l:C{@ohos.window.window.ExtensionWindowAttribute}",
        reinterpret_cast<void *>(GetWindowAttribute)},
    ani_native_function {"getWindowRect", "l:C{@ohos.window.window.Rect}", reinterpret_cast<void *>(GetWindowRect)},
    ani_native_function {"getSubWindowOptions", "l:C{@ohos.window.window.SubWindowOptions}",
        reinterpret_cast<void *>(GetSubWindowOptions)},
    ani_native_function {"getSystemWindowOptions", "l:C{@ohos.window.window.SystemWindowOptions}",
        reinterpret_cast<void *>(GetSystemWindowOptions)},
    ani_native_function {"setWindowName", "lC{std.core.String}:", reinterpret_cast<void *>(SetWindowName)},
    ani_native_function {"setWindowAttribute", "lC{@ohos.window.window.ExtensionWindowAttribute}:",
        reinterpret_cast<void *>(SetWindowAttribute)},
    ani_native_function {"setWindowRect", "lC{@ohos.window.window.Rect}:", reinterpret_cast<void *>(SetWindowRect)},
    ani_native_function {"setSubWindowOptions", "lC{@ohos.window.window.SubWindowOptions}:",
        reinterpret_cast<void *>(SetSubWindowOptions)},
    ani_native_function {"setSystemWindowOptions", "lC{@ohos.window.window.SystemWindowOptions}:",
        reinterpret_cast<void *>(SetSystemWindowOptions)},
    };

std::array extConfigRectMethods = {
    ani_native_function {"getLeft", "l:i", reinterpret_cast<void *>(GetWindowRectLeft)},
    ani_native_function {"getTop", "l:i", reinterpret_cast<void *>(GetWindowRectTop)},
    ani_native_function {"getWidth", "l:i", reinterpret_cast<void *>(GetWindowRectWidth)},
    ani_native_function {"getHeight", "l:i", reinterpret_cast<void *>(GetWindowRectHeight)},
    ani_native_function {"setLeft", "li:", reinterpret_cast<void *>(SetWindowRectLeft)},
    ani_native_function {"setTop", "li:", reinterpret_cast<void *>(SetWindowRectTop)},
    ani_native_function {"setWidth", "li:", reinterpret_cast<void *>(SetWindowRectWidth)},
    ani_native_function {"setHeight", "li:", reinterpret_cast<void *>(SetWindowRectHeight)},
    };

std::array extConfigSubWindowOptionsMethods = {
    ani_native_function {"getTitle", "l:C{std.core.String}", reinterpret_cast<void *>(GetSubWindowOptionsTitle)},
    ani_native_function {"getDecorEnabled", "l:z", reinterpret_cast<void *>(GetSubWindowOptionsDecorEnabled)},
    ani_native_function {"getIsModal", "l:z", reinterpret_cast<void *>(GetSubWindowOptionsIsModal)},
    ani_native_function {"getIsTopmost", "l:z", reinterpret_cast<void *>(GetSubWindowOptionsIsTopmost)},
    ani_native_function {"setTitle", "lC{std.core.String}:", reinterpret_cast<void *>(SetSubWindowOptionsTitle)},
    ani_native_function {"setDecorEnabled", "lz:", reinterpret_cast<void *>(SetSubWindowOptionsDecorEnabled)},
    ani_native_function {"setIsModal", "lz:", reinterpret_cast<void *>(SetSubWindowOptionsIsModal)},
    ani_native_function {"setIsTopmost", "lz:", reinterpret_cast<void *>(SetSubWindowOptionsIsTopmost)},
    };

std::array extConfigSystemWindowOptionsMethods = {
    ani_native_function {"getWindowType", "l:C{@ohos.window.window.WindowType}",
        reinterpret_cast<void *>(GetSystemWindowOptionsWindowType)},
    ani_native_function {"setWindowType", "lC{@ohos.window.window.WindowType}:",
        reinterpret_cast<void *>(SetSystemWindowOptionsWindowType)},
    };

ani_status ExtConfig_Rect_ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtConfigRect begin");
    ani_status ret {};
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get env failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.ExtConfigRect", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    if ((ret = env->Class_BindNativeMethods(cls, extConfigRectMethods.data(),
        extConfigRectMethods.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Class bind native methods failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtConfigRect end");
    return ANI_OK;
}

ani_status ExtConfig_SubWindowOptions_ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowConfig begin");
    ani_status ret {};
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get env failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.ExtConfigSubWindowOptions", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    if ((ret = env->Class_BindNativeMethods(cls, extConfigSubWindowOptionsMethods.data(),
        extConfigSubWindowOptionsMethods.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Class bind native methods failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowConfig end");
    return ANI_OK;
}

ani_status ExtConfig_SystemWindowOptions_ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowConfig begin");
    ani_status ret {};
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get env failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.ExtConfigSystemWindowOptions", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    if ((ret = env->Class_BindNativeMethods(cls, extConfigSystemWindowOptionsMethods.data(),
        extConfigSystemWindowOptionsMethods.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Class bind native methods failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowConfig end");
    return ANI_OK;
}

ANI_EXPORT ani_status ExtensionWindowConfig_ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowConfig begin");
    ani_status ret {};
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get env failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.ExtensionWindowConfigInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Find class failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    if ((ret = env->Class_BindNativeMethods(cls, extensionWindowConfigNativeMethods.data(),
        extensionWindowConfigNativeMethods.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Class bind native methods failed, ret: %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    ExtConfig_Rect_ANI_Constructor(vm, result);
    ExtConfig_SubWindowOptions_ANI_Constructor(vm, result);
    ExtConfig_SystemWindowOptions_ANI_Constructor(vm, result);
    TLOGD(WmsLogTag::WMS_UIEXT, "[ANI]Init ExtensionWindowConfig end");
    return ANI_OK;
}
}
