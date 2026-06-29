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

#include "session/host/include/atomicservice_basic_engine_plugin.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
#ifdef WINDOW_ATOMIC_SERVICE_ATTRIBUTION_ENABLE
AtomicserviceIconInfo::AtomicserviceIconInfo() : bundleName_(""), moduleName_(""), abilityName_(""), circleIcon_(""),
    eyelashRingIcon_(""), appName_(""), deviceTypes_({}), resizable_(0), supportWindowMode_({})
{}

std::string AtomicserviceIconInfo::GetBundleName() const
{
    return bundleName_;
}

std::string AtomicserviceIconInfo::GetModuleName() const
{
    return moduleName_;
}

std::string AtomicserviceIconInfo::GetAbilityName() const
{
    return abilityName_;
}

std::string AtomicserviceIconInfo::GetCircleIcon() const
{
    return circleIcon_;
}

std::string AtomicserviceIconInfo::GetEyelashRingIcon() const
{
    return eyelashRingIcon_;
}

std::string AtomicserviceIconInfo::GetAppName() const
{
    return appName_;
}

std::vector<std::string> AtomicserviceIconInfo::GetDeviceTypes() const
{
    return deviceTypes_;
}

std::vector<std::string> AtomicserviceIconInfo::GetSupportWindowMode() const
{
    return supportWindowMode_;
}

int32_t AtomicserviceIconInfo::GetResizable() const
{
    return resizable_;
}

void AtomicserviceIconInfo::SetBundleName(const std::string& val)
{
    bundleName_ = val;
}

void AtomicserviceIconInfo::SetModuleName(const std::string& val)
{
    moduleName_ = val;
}

void AtomicserviceIconInfo::SetAbilityName(const std::string& val)
{
    abilityName_ = val;
}

void AtomicserviceIconInfo::SetCircleIcon(const std::string& val)
{
    circleIcon_ = val;
}

void AtomicserviceIconInfo::SetEyelashRingIcon(const std::string& val)
{
    eyelashRingIcon_ = val;
}

void AtomicserviceIconInfo::SetAppName(const std::string& val)
{
    appName_ = val;
}

void AtomicserviceIconInfo::SetDeviceTypes(const std::vector<std::string>& val)
{
    deviceTypes_ = val;
}

void AtomicserviceIconInfo::SetSupportWindowMode(const std::vector<std::string>& val)
{
    supportWindowMode_ = val;
}

void AtomicserviceIconInfo::SetResizable(int32_t val)
{
    resizable_ = val;
}

bool AtomicserviceIconInfo::Marshalling(Parcel& parcel) const
{
    return true;
}

AtomicserviceIconInfo *AtomicserviceIconInfo::Unmarshalling(Parcel& parcel)
{
    AtomicserviceIconInfo *data = new AtomicserviceIconInfo();
    return data;
}

AtomicServiceBasicEnginePlugin::AtomicServiceBasicEnginePlugin()
{
    // if ACE_ENGINE_PLUGIN_PATH is defined, load so pack
#ifdef ACE_ENGINE_PLUGIN_PATH
    atomicServiceBasicEngine_ = dlopen(
        (char*)ACE_ENGINE_PLUGIN_PATH, RTLD_LAZY);
    if (atomicServiceBasicEngine_ == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "dlopen lib %{public}s false", (char*)ACE_ENGINE_PLUGIN_PATH);
        return;
    }
#else
    TLOGW(WmsLogTag::WMS_LIFE, "ACE_ENGINE_PLUGIN_PATH is not defined");
    return;
#endif
    // load function GetAtomicserviceIconInfoPlugin
    getAtomicserviceIconInfoPlugin_ = (GetAtomicserviceIconInfoPlugin*)(dlsym(
        atomicServiceBasicEngine_, "GetAtomicserviceIconInfoPlugin"));
    if (getAtomicserviceIconInfoPlugin_ == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "fail to dlsym GetAtomicserviceIconInfoPlugin, reason: %{public}s", dlerror());
        return;
    }
    // load function FreeData
    freeData_ = (FreeData*)(dlsym(atomicServiceBasicEngine_, "FreeAtomicserviceIconInfoPlugin"));
    if (freeData_ == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "fail to dlsym FreeData, reason: %{public}s", dlerror());
        return;
    }
}

AtomicServiceBasicEnginePlugin::~AtomicServiceBasicEnginePlugin()
{
    // close dlopen so pack
    if (atomicServiceBasicEngine_ != nullptr) {
        dlclose(atomicServiceBasicEngine_);
        atomicServiceBasicEngine_ = nullptr;
    }
    ReleaseData();
    getAtomicserviceIconInfoPlugin_ = nullptr;
    freeData_ = nullptr;
}

AtomicServiceBasicEnginePlugin& AtomicServiceBasicEnginePlugin::GetInstance()
{
    static AtomicServiceBasicEnginePlugin instance;
    return instance;
}

__attribute__((no_sanitize("cfi"))) AtomicserviceIconInfo* AtomicServiceBasicEnginePlugin::GetParamsFromAtomicServiceBasicEngine(
    const std::string& bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (getAtomicserviceIconInfoPlugin_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "getAtomicserviceIconInfoPlugin_ is nullptr");
        return nullptr;
    }
    int32_t ret = getAtomicserviceIconInfoPlugin_(bundleName, &atomicserviceIconInfo_);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed, bundleName:%{public}s", bundleName.c_str());
        return nullptr;
    }
    return atomicserviceIconInfo_;
}

__attribute__((no_sanitize("cfi"))) int32_t AtomicServiceBasicEnginePlugin::ReleaseData()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // free data
    if (freeData_ != nullptr && atomicserviceIconInfo_ != nullptr) {
        freeData_(atomicserviceIconInfo_);
        atomicserviceIconInfo_ = nullptr;
        return 0;
    }
    TLOGW(WmsLogTag::WMS_LIFE, "dlsym FreeData interface failed, can't release data");
    return -1;
}
#endif // WINDOW_ATOMIC_SERVICE_ATTRIBUTION_ENABLE
} //namespace OHOS::Rosen
