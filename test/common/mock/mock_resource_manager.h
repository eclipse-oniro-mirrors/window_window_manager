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

#ifndef FRAMEWORKS_WM_TEST_UT_MOCK_RESOURCE_MANAGER_H
#define FRAMEWORKS_WM_TEST_UT_MOCK_RESOURCE_MANAGER_H
#include <gmock/gmock.h>
#include <resource_manager.h>

namespace OHOS {
namespace Global {
namespace Resource {
class MockResourceManager : public ResourceManager {
public:
    MockResourceManager() = default;
    ~MockResourceManager() = default;
    MOCK_METHOD3(AddResource, bool(const char* path,  const uint32_t& selectedTypes,
        bool forceReload));
    MOCK_METHOD2(GetColorById, RState(uint32_t id, uint32_t& outValue));
    MOCK_METHOD3(GetMediaById, RState(uint32_t id, std::string& outValue, uint32_t density));
    MOCK_METHOD4(GetMediaDataById, RState(uint32_t id, size_t& len, std::unique_ptr<uint8_t[]>& outValue,
        uint32_t density));
    MOCK_METHOD3(GetResConfigById, RState(uint32_t resId, ResConfig& resConfig, uint32_t density));
    MOCK_METHOD4(GetResConfigByName, RState(const std::string& name, const ResType type, ResConfig& resConfig,
        uint32_t density));
    MOCK_METHOD5(GetThemeIcons, RState(uint32_t resId, std::pair<std::unique_ptr<uint8_t[]>, size_t>& foregroundInfo,
        std::pair<std::unique_ptr<uint8_t[]>, size_t>& backgroundInfo,
        uint32_t density, const std::string& abilityName));
    MOCK_METHOD0(GetThemeMask, std::string());
    MOCK_METHOD4(GetOtherIconsInfo, RState(const std::string& iconName,
        std::unique_ptr<uint8_t[]>& outValue, size_t& len, bool isGlobalMask));
    MOCK_METHOD2(IsRawDirFromHap, RState(const std::string& pathName, bool& outValue));
    MOCK_METHOD1(UpdateOverrideResConfig, RState(ResConfig& resConfig));
    MOCK_METHOD1(GetOverrideResConfig, void(ResConfig& resConfig));
    MOCK_METHOD3(GetDynamicIcon, RState(const std::string& resName,
        std::pair<std::unique_ptr<uint8_t[]>, size_t>& iconInfo, uint32_t density));
    MOCK_METHOD1(HasIconInTheme, bool(const std::string& bundleName));
    MOCK_METHOD2(AddResource, bool(const std::string& path, const std::vector<std::string>& overlayPaths));
    MOCK_METHOD2(RemoveResource, bool(const std::string& path, const std::vector<std::string>& overlayPaths));
    MOCK_METHOD2(UpdateResConfig, RState(ResConfig& resConfig, bool isUpdateTheme));
    MOCK_METHOD1(GetResConfig, void(ResConfig& resConfig));
    MOCK_METHOD2(GetStringById, RState(uint32_t id, std::string& outValue));
    MOCK_METHOD2(GetStringByName, RState(const char* name, std::string& outValue));
    MOCK_METHOD4(GetFormatPluralStringById, RState(std::string& outValue, uint32_t id, int quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>>& jsParams));
    MOCK_METHOD4(GetFormatPluralStringById, RState(std::string& outValue, uint32_t id, Quantity quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>>& jsParams));
    MOCK_METHOD2(AddPatchResource, bool(const char* path, const char* patchPath));
    MOCK_METHOD2(GetThemeDataByName, RState(const char* name, std::map<std::string, ResData>& outValue));
    MOCK_METHOD2(GetThemeDataById, RState(uint32_t id, std::map<std::string, ResData>& outValue));
    MOCK_METHOD2(GetPatternDataById, RState(uint32_t id, std::map<std::string, ResData>& outValue));
    MOCK_METHOD2(GetPatternDataByName, RState(const char* name, std::map<std::string, ResData>& outValue));
    MOCK_METHOD2(GetStringArrayById, RState(uint32_t id, std::vector<std::string>& outValue));
    MOCK_METHOD1(GetOverrideResourceManager, std::shared_ptr<ResourceManager>(
        std::shared_ptr<ResConfig> overrideResConfig));
    MOCK_METHOD4(GetFormatPluralStringByName, RState(std::string& outValue, const char* name, int quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>>& jsParams));
    MOCK_METHOD2(GetStringArrayByName, RState(const char* name, std::vector<std::string>& outValue));
    MOCK_METHOD2(GetPatternById, RState(uint32_t id, std::map<std::string, std::string>& outValue));
    MOCK_METHOD2(GetPatternByName, RState(const char* name, std::map<std::string, std::string>& outValue));
    MOCK_METHOD3(GetPluralStringById, RState(uint32_t id, int quantity, std::string& outValue));
    MOCK_METHOD3(GetPluralStringByName, RState(const char* name, int quantity, std::string& outValue));
    MOCK_METHOD2(GetThemeById, RState(uint32_t id, std::map<std::string, std::string>& outValue));
    MOCK_METHOD2(GetThemeByName, RState(const char* name, std::map<std::string, std::string>& outValue));
    MOCK_METHOD2(GetBooleanById, RState(uint32_t id, bool& outValue));
    MOCK_METHOD2(GetBooleanByName, RState(const char* name, bool& outValue));
    MOCK_METHOD2(GetIntegerById, RState(uint32_t id, int& outValue));
    MOCK_METHOD2(GetIntegerByName, RState(const char* name, int& outValue));
    MOCK_METHOD2(GetFloatById, RState(uint32_t id, float& outValue));
    MOCK_METHOD3(GetFloatById, RState(uint32_t id, float& outValue, std::string& unit));
    MOCK_METHOD2(GetFloatByName, RState(const char* name, float& outValue));
    MOCK_METHOD3(GetFloatByName, RState(const char* name, float& outValue, std::string& unit));
    MOCK_METHOD2(GetIntArrayById, RState(uint32_t id, std::vector<int>& outValue));
    MOCK_METHOD2(GetIntArrayByName, RState(const char* name, std::vector<int>& outValue));
    MOCK_METHOD2(GetColorByName, RState(const char* name, uint32_t& outValue));
    MOCK_METHOD2(GetProfileById, RState(uint32_t id, std::string& outValue));
    MOCK_METHOD2(GetProfileByName, RState(const char* name, std::string& outValue));
    MOCK_METHOD3(GetMediaByName, RState(const char* name, std::string& outValue, uint32_t density));
    MOCK_METHOD2(GetSymbolById, RState(uint32_t id, uint32_t& outValue));
    MOCK_METHOD2(GetSymbolByName, RState(const char* name, uint32_t& outValue));
    MOCK_METHOD2(GetRawFilePathByName, RState(const std::string& name, std::string& outValue));
    MOCK_METHOD2(GetRawFileDescriptor, RState(const std::string& name, RawFileDescriptor& descriptor));
    MOCK_METHOD1(CloseRawFileDescriptor, RState(const std::string& name));
    MOCK_METHOD0(GetResourcePaths, std::vector<std::string>());
    MOCK_METHOD4(GetMediaDataByName, RState(const char* name, size_t& len, std::unique_ptr<uint8_t[]>& outValue,
        uint32_t density));
    MOCK_METHOD3(GetMediaBase64DataById, RState(uint32_t id, std::string& outValue, uint32_t density));
    MOCK_METHOD3(GetMediaBase64DataByName, RState(const char* name, std::string& outValue, uint32_t density));
    MOCK_METHOD3(GetProfileDataById, RState(uint32_t id, size_t& len, std::unique_ptr<uint8_t[]>& outValue));
    MOCK_METHOD3(GetProfileDataByName, RState(const char* name, size_t& len, std::unique_ptr<uint8_t[]>& outValue));
    MOCK_METHOD3(GetRawFileFromHap, RState(const std::string& rawFileName, size_t& len,
        std::unique_ptr<uint8_t[]>& outValue));
    MOCK_METHOD2(GetRawFileDescriptorFromHap, RState(const std::string& rawFileName, RawFileDescriptor& descriptor));
    MOCK_METHOD1(IsLoadHap, RState(std::string& hapPath));
    MOCK_METHOD2(GetRawFileList, RState(const std::string& rawDirPath, std::vector<std::string>& rawfileList));
    MOCK_METHOD5(GetDrawableInfoById, RState(uint32_t id, std::string& type, size_t& len,
        std::unique_ptr<uint8_t[]>& outValue, uint32_t density));
    MOCK_METHOD5(GetDrawableInfoByName, RState(const char* name, std::string& type, size_t& len,
        std::unique_ptr<uint8_t[]>& outValue, uint32_t density));
    MOCK_METHOD3(GetStringFormatById, RState(uint32_t id, std::string& outValue,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>>& jsParams));
    MOCK_METHOD3(GetStringFormatByName, RState(const char* name, std::string& outValue,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>>& jsParams));
    MOCK_METHOD0(GetResourceLimitKeys, uint32_t());
    MOCK_METHOD1(AddAppOverlay, bool(const std::string& path));
    MOCK_METHOD1(RemoveAppOverlay, bool(const std::string& path));
    MOCK_METHOD2(GetRawFdNdkFromHap, RState(const std::string& name, RawFileDescriptor& descriptor));
    MOCK_METHOD2(GetResId, RState(const std::string& resTypeName, uint32_t& resId));
    MOCK_METHOD2(GetLocales, void(std::vector<std::string>& outValue, bool includeSystem));
    MOCK_METHOD5(GetDrawableInfoById, RState(uint32_t id,
        std::tuple<std::string, size_t, std::string>& drawableInfo,
        std::unique_ptr<uint8_t[]>& outValue, uint32_t iconType, uint32_t density));
    MOCK_METHOD5(GetDrawableInfoByName, RState(const char* name,
        std::tuple<std::string, size_t, std::string>& drawableInfo,
        std::unique_ptr<uint8_t[]>& outValue, uint32_t iconType, uint32_t density));
    MOCK_METHOD4(GetFormatPluralStringById, RState(std::string& outValue, uint32_t id, Quantity quantity,
        va_list args));
    MOCK_METHOD4(GetFormatPluralStringByName, RState(std::string& outValue, const char* name, Quantity quantity,
        va_list args));
    MOCK_METHOD4(GetFormatPluralStringByName, RState(std::string& outValue, const char* name, Quantity quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>>& jsParams));
    MOCK_METHOD3(GetStringFormatById, RState(std::string& outValue, uint32_t id, va_list args));
    MOCK_METHOD3(GetStringFormatByName, RState(std::string& outValue, const char* name, va_list args));
    MOCK_METHOD2(GetResName, RState(uint32_t id, std::string &outValue));
    virtual RState GetStringFormatByName(std::string& outValue, const char* name, ...)
    {
        return SUCCESS;
    }
    virtual RState GetPluralStringByIdFormat(std::string& outValue, uint32_t id, int quantity, ...)
    {
        return SUCCESS;
    }
    virtual RState GetPluralStringByNameFormat(std::string& outValue, const char* name, int quantity, ...)
    {
        return SUCCESS;
    }
    virtual RState GetStringFormatById(std::string& outValue, uint32_t id, ...)
    {
        return SUCCESS;
    }
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif // FRAMEWORKS_WM_TEST_UT_MOCK_RESOURCE_MANAGER_H