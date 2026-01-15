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

#ifndef OHOS_ANI_WINDOW_UTILS_H
#define OHOS_ANI_WINDOW_UTILS_H

#include <map>

#include "ani.h"
#include "js_window.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "window.h"

#ifndef WINDOW_PREVIEW
#include "window_manager.h"
#else
#include "mock/window_manager.h"
#endif

#include "window_manager_hilog.h"
#include "window_option.h"
#include "window_visibility_info.h"
#include "wm_common.h"
#include "pixel_map.h"

#define RETURN_IF_NULL(param, ...)                                             \
    do {                                                                       \
        if (!param) {                                                          \
            TLOGE(WmsLogTag::DEFAULT, "[ANI] The %{public}s is null", #param); \
            return __VA_ARGS__;                                                \
        }                                                                      \
    } while (false)                                                            \

namespace OHOS {
namespace Rosen {
constexpr Rect g_emptyRect = {0, 0, 0, 0};

class AniWindowUtils {
public:
    static ani_status InitAniCreator(ani_env* env,
        const std::string& aniClassDescriptor, const std::string& aniCtorSignature);
    static ani_object InitAniObjectByCreator(ani_env* env,
        const std::string& aniClassDescriptor, const std::string aniCtorSignature, ...);
    static ani_status GetStdString(ani_env* env, ani_string ani_str, std::string& result);
    static ani_status GetStdStringVector(ani_env* env, ani_object ary, std::vector<std::string>& result);
    static ani_status GetPropertyIntObject(ani_env* env, const char* propertyName, ani_object object, int32_t& result);
    static ani_status GetPropertyDoubleObject(ani_env* env, const char* propertyName,
        ani_object object, double& result);
    static ani_status GetPropertyBoolObject(ani_env* env, const char* propertyName, ani_object object, bool& result);
    static bool GetPropertyRectObject(ani_env* env, const char* propertyName,
        ani_object object, Rect& result);
    static ani_status GetOptionalProperty(ani_env* env, ani_object object, const char* propertyName,
        ani_ref& outPropRef, bool& outIsUndefined);
    static ani_status GetOptionalIntProperty(ani_env* env, const char* propertyName,
        ani_object object, std::optional<ani_int>& optIntProp);
    template <typename EnumType>
    static ani_status GetOptionalEnumProperty(
        ani_env* env, const char* propertyName, ani_object object, std::optional<EnumType>& optEnumProp);
    static bool GetIntObject(ani_env* env, const char* propertyName, ani_object object, int32_t& result);
    static ani_status GetDoubleObject(ani_env* env, ani_object double_object, double& result);
    static ani_status GetBooleanObject(ani_env* env, ani_object boolean_object, bool& result);
    static bool GetPropertyUIntObject(ani_env* env, const char* propertyName, ani_object object, uint32_t& result);
    static ani_status GetIntVector(ani_env* env, ani_object ary, std::vector<int32_t>& result);
    static ani_status GetPropertyLongObject(ani_env* env, const char* propertyName, ani_object object, int64_t& result);
    static ani_status GetEnumValue(ani_env* env, ani_enum_item enumPara, uint32_t& result);
    static ani_status NewAniObject(ani_env* env, const char* cls, const char* signature, ani_object* result, ...);
    static ani_object CreateAniUndefined(ani_env* env);
    static ani_object AniThrowError(ani_env* env, WMError errorCode, std::string msg = "");
    static ani_object AniThrowError(ani_env* env, WmErrorCode errorCode, std::string msg = "");
    static ani_object CreateAniStatusBarProperty(ani_env* env, const SystemBarProperty& prop);
    static ani_object CreateAniWindowDensityInfo(ani_env* env, const WindowDensityInfo& info);
    static ani_object CreateAniWindowSystemBarProperties(ani_env* env, const SystemBarProperty& status,
        const SystemBarProperty& navigation);
    static ani_object CreateAniWindowLayoutInfo(ani_env* env, const WindowLayoutInfo& info);
    static ani_object CreateAniWindowLayoutInfoArray(ani_env* env, const std::vector<sptr<WindowLayoutInfo>>& infos);
    static ani_object CreateAniWindowInfo(ani_env* env, const WindowVisibilityInfo& info);
    static ani_object CreateAniWindowInfoArray(ani_env* env, const std::vector<sptr<WindowVisibilityInfo>>& infos);
    static ani_object CreateAniWindowArray(ani_env* env, std::vector<ani_ref>& windows);
    static ani_object CreateAniSize(ani_env* env, int32_t width, int32_t height);
    static ani_object CreateAniRect(ani_env* env, const Rect& rect);
    static ani_object CreateAniTitleButtonRect(ani_env* env, const TitleButtonRect& rect);
    static ani_object CreateAniDecorButtonStyle(ani_env* env, const DecorButtonStyle& style);
    static ani_status SetOptionalFieldInt(ani_env* env, ani_object obj, ani_class cls,
        const char* method, ani_int aniInt);
    static bool SetDecorButtonStyleFromAni(ani_env* env, DecorButtonStyle& decorButtonStyle,
        const ani_object& decorStyle);
    static bool GetColorMode(ani_env* env, const ani_object& decorStyle, int32_t& colorMode);
    static ani_object CreateAniWindowLimits(ani_env* env, const WindowLimits& windowLimits);
    static ani_object CreateAniAvoidArea(ani_env* env, const AvoidArea& avoidArea,
        AvoidAreaType type, bool useActualVisibility = false);
    static ani_object CreateAniWindowsArray(ani_env* env, std::vector<ani_ref>& windows);
    static ani_object CreateAniFrameMetrics(ani_env* env, const FrameMetrics& metrics);
    static ani_object CreateAniSystemBarTintState(ani_env* env, DisplayId displayId, const SystemBarRegionTints& tints);
    static ani_object CreateAniSystemBarRegionTint(ani_env* env, const SystemBarRegionTint& tint);
    static ani_object CreateAniRotationChangeInfo(ani_env* env, const RotationChangeInfo& info);
    static void ParseRotationChangeResult(ani_env* env, ani_object obj, RotationChangeResult& rotationChangeResult);
    static ani_object CreateAniKeyboardInfo(ani_env* env, const KeyboardPanelInfo& keyboardPanelInfo);
    static ani_object CreateAniAnimationInfo(ani_env* env, const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve);
    static ani_object CreateAniAnimationConfig(ani_env* env, const KeyboardAnimationCurve& curve);
    static ani_status CallAniFunctionVoid(ani_env *env, const char* ns, const char* func, const char* signature, ...);
    static ani_status CallAniMethodVoid(ani_env* env, ani_object object, const char* cls,
        const char* method, const char* signature, ...);
    static ani_status CallAniFunctionRef(ani_env *env, ani_ref& result, ani_ref ani_callback,
        const int32_t args_num, ...);
    static ani_status CallAniMethodVoid(ani_env* env, ani_object object, ani_class cls,
        const char* method, const char* signature, ...);
    static ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);
    static void* GetAbilityContext(ani_env *env, ani_object aniObj);
    static ani_object CreateAniRectObject(ani_env* env, const Rect& rect);
    static ani_object CreateWindowsProperties(ani_env* env, const WindowPropertyInfo& windowPropertyInfo);
    static ani_status CheckPropertyNameUndefined(ani_env* env, const char* propertyName,
        ani_object object, bool& result);
    static bool ParseKeyFramePolicy(ani_env* env, ani_object aniKeyFramePolicy, KeyFramePolicy& keyFramePolicy);
    static ani_object CreateKeyFramePolicy(ani_env* env, const KeyFramePolicy& keyFramePolicy);
    static ani_object CreateAniPixelMapArray(ani_env* env,
        const std::vector<std::shared_ptr<Media::PixelMap>>& pixelMaps);
    static ani_object CreateAniMainWindowInfoArray(ani_env* env, const std::vector<sptr<MainWindowInfo>>& infos);
    static ani_object CreateAniMainWindowInfo(ani_env* env, const MainWindowInfo& info);
    static ani_object CreateProperties(ani_env* env, const sptr<Window>& window);
    static uint32_t GetColorFromAni(ani_env* env, const char* name,
        uint32_t defaultColor, bool& flag, const ani_object& aniObject);
    static bool SetWindowStatusBarContentColor(ani_env* env, ani_object aniObject,
        std::map<WindowType, SystemBarProperty>& properties, std::map<WindowType, SystemBarPropertyFlag>&
        propertyFlags);
    static bool SetWindowNavigationBarContentColor(ani_env* env, ani_object aniObject,
        std::map<WindowType, SystemBarProperty>& properties, std::map<WindowType, SystemBarPropertyFlag>&
        propertyFlags);
    static bool SetSystemBarPropertiesFromAni(ani_env* env,
        std::map<WindowType, SystemBarProperty>& windowBarProperties,
        std::map<WindowType, SystemBarPropertyFlag>& windowPropertyFlags,
        const ani_object& aniProperties,
        const sptr<Window>& window);
    static void GetSystemBarPropertiesFromAni(sptr<Window>& window,
        std::map<WindowType, SystemBarProperty>& newProperties,
        std::map<WindowType, SystemBarPropertyFlag>& newPropertyFlags,
        std::map<WindowType, SystemBarProperty>& properties,
        std::map<WindowType, SystemBarPropertyFlag>& propertyFlags);
    static void UpdateSystemBarProperties(std::map<WindowType, SystemBarProperty>& systemBarProperties,
            const std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags, sptr<Window> windowToken);
    static bool GetSpecificBarStatus(ani_env* env,
        ani_string aniName, ani_boolean aniEnable, ani_object aniAnimation,
        WindowType& type, SystemBarProperty& systemBarProperty, SystemBarPropertyFlag& systemBarPropertyFlag);
    static bool ParseWindowMask(ani_env* env, ani_array windowMaskArray,
        std::vector<std::vector<uint32_t>>& windowMask);
    static bool ParseWindowMaskInnerValue(ani_env* env, ani_array innerArray, std::vector<uint32_t>& elementArray);
    static WmErrorCode ParseTouchableAreas(ani_env* env, ani_array rects, const Rect& windowRect,
        std::vector<Rect>& touchableAreas);
    static bool ParseAndCheckRect(ani_env* env, ani_object rect, const Rect& windowRect, Rect& touchableRect);
    static ani_object CreateOptionalBool(ani_env *env, ani_boolean value);
    static ani_object CreateOptionalInt(ani_env *env, ani_int value);
    static void GetWindowSnapshotConfiguration(ani_env* env, ani_object config,
        WindowSnapshotConfiguration& windowSnapshotConfiguration);
    static bool ParseWindowLimits(ani_env* env, ani_object aniWindowLimits, WindowLimits& windowLimits);
    static bool CheckParaIsUndefined(ani_env* env, ani_object para);
    static ani_object CreateAniPosition(ani_env* env, const Position& position);
    static std::string GetPixelUnitString(const PixelUnit& pixelUnit);

    /**
     * @brief Convert WMError to corresponding WmErrorCode.
     *
     * @param error WMError value to convert.
     * @param defaultCode Value to return if mapping is not found (default: WM_ERROR_STATE_ABNORMALLY).
     * @return Corresponding WmErrorCode or defaultCode if unmapped.
     */
    static WmErrorCode ToErrorCode(WMError error, WmErrorCode defaultCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    /**
     * @brief Checks whether a given ANI object is an instance of the specified class.
     *
     * @param env The ANI environment.
     * @param obj The ANI object to be checked.
     * @param className The fully qualified name of the class to check against.
     * @return true if obj is an instance of the specified class, false otherwise.
     */
    static bool IsInstanceOf(ani_env* env, ani_object obj, const char* className);

    /**
     * @brief Extracts all elements from an ANI array object into a vector of ani_refs.
     *
     * @param env The ANI environment.
     * @param arrayObj The ANI array object from which to extract elements.
     * @return A vector containing ani_ref references to each valid element.
     *         Returns an empty vector if the input is invalid or the extraction fails.
     */
    static std::vector<ani_ref> ExtractArrayElements(ani_env* env, ani_object arrayObj);

    /**
     * @brief Extracts enum values from an ANI array into a vector of type EnumType.
     *
     * @tparam EnumType The enumeration type to which each ANI enum item should be converted.
     * @param env The ANI environment.
     * @param enumArrayObj The ANI array object containing enum items.
     * @return A vector containing converted enum values of type EnumType.
     *         Returns an empty vector if the input is invalid or conversion fails.
     */
    template <typename EnumType>
    static std::vector<EnumType> ExtractEnumValues(ani_env* env, ani_object enumArrayObj);
    static bool ParseSubWindowOptions(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption);
    static bool ParseRectParam(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption);
    static bool ParseModalityParam(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption);
    static bool ParseZLevelParam(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption);
    static bool ParseSubWindowOption(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption);
    static bool ParseRectParams(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption);
    static bool ParseModalityParams(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption);
    static bool ParseZLevelParams(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption);
    template<typename T>
    static ani_object CreateBaseTypeObject(ani_env* env, T value);
};

template <typename EnumType>
std::vector<EnumType> AniWindowUtils::ExtractEnumValues(ani_env* env, ani_object enumArrayObj)
{
    std::vector<EnumType> result;
    RETURN_IF_NULL(env, result);
    RETURN_IF_NULL(enumArrayObj, result);

    auto aniRefs = ExtractArrayElements(env, enumArrayObj);
    if (aniRefs.empty()) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Empty ani enum array");
        return result;
    }

    for (const auto& aniRef : aniRefs) {
        if (aniRef == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] aniRef is nullptr");
            continue;
        }
        ani_int enumValue = 0;
        ani_status ret = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(aniRef), &enumValue);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get enum value, ret: %{public}d", static_cast<int32_t>(ret));
            continue;
        }
        result.push_back(static_cast<EnumType>(enumValue));
    }
    return result;
}

template <typename EnumType>
ani_status AniWindowUtils::GetOptionalEnumProperty(
    ani_env* env, const char* propertyName, ani_object object, std::optional<EnumType>& optEnumProp)
{
    optEnumProp.reset();

    ani_ref propRef;
    bool isUndefined;
    ani_status ret = AniWindowUtils::GetOptionalProperty(env, object, propertyName, propRef, isUndefined);
    if (ret != ANI_OK || isUndefined) {
        return ret;
    }

    uint32_t enumValue = 0;
    ret = AniWindowUtils::GetEnumValue(env, static_cast<ani_enum_item>(propRef), enumValue);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[ANI] Failed to get enum value for %{public}s, ret: %{public}d",
              propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    optEnumProp = static_cast<EnumType>(enumValue);
    return ANI_OK;
}

template<typename T>
const char* GetClassName()
{
    if (std::is_same<T, int>::value) {
        return "std.core.Int";
    } else if (std::is_same<T, double>::value) {
        return "std.core.Double";
    } else if (std::is_same<T, long>::value) {
        return "std.core.Long";
    } else {
        return nullptr;
    }
}

template<typename T>
const char* GetCtorSignature()
{
    if (std::is_same<T, int>::value) {
        return "i:";
    } else if (std::is_same<T, double>::value) {
        return "d:";
    } else if (std::is_same<T, long>::value) {
        return "l:";
    } else {
        return nullptr;
    }
}

template<typename T>
ani_object AniWindowUtils::CreateBaseTypeObject(ani_env* env, T value)
{
    static const char* className = GetClassName<T>();
    ani_class cls;
    ani_status ret =  env->FindClass(className, &cls);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found for type");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    // Obtains the signature of a constructor based on the type
    const char* signature = GetCtorSignature<T>();
    ani_method ctor;
    ret = env->Class_FindMethod(cls, "<ctor>", signature, &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found for type");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    // Convert values based on types and create objects
    ani_object obj;
    if (std::is_same<T, int>::value) {
        ret = env->Object_New(cls, ctor, &obj, ani_int(value));
    } else if (std::is_same<T, double>::value) {
        ret = env->Object_New(cls, ctor, &obj, ani_double(value));
    } else if (std::is_same<T, long>::value) {
        ret = env->Object_New(cls, ctor, &obj, ani_long(value));
    } else {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] unsupported type");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] failed to create new obj for type");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return obj;
}
} // namespace Rosen
} // namespace OHOS
#endif