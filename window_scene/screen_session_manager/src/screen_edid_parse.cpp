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

#include "screen_edid_parse.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t CHECK_INDEX_BEGIN = 0x8;
constexpr uint32_t CHECK_INDEX_END = 0x11;
constexpr int32_t CHECK_CODE_INVALID = -1;
constexpr uint32_t SLEEP_TIME_US = 10000;
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string EDID_PARSE_SO_PATH = "/system/lib64/libedid_parse.z.so";
#else
const std::string EDID_PARSE_SO_PATH = "/system/lib/libedid_parse.z.so";
#endif
}

static void *g_libHandle = nullptr;

bool LoadEdidPlugin(void)
{
    if (g_libHandle != nullptr) {
        TLOGW(WmsLogTag::DMS, "edid plugin has already exists.");
        return true;
    }
    int32_t cnt = 0;
    int32_t retryTimes = 3;
    do {
        cnt++;
        g_libHandle = dlopen(EDID_PARSE_SO_PATH.c_str(), RTLD_LAZY);
        TLOGI(WmsLogTag::DMS, "dlopen %{public}s, retry cnt: %{public}d", EDID_PARSE_SO_PATH.c_str(), cnt);
        if (g_libHandle == nullptr) {
            TLOGE(WmsLogTag::DMS, "dlopen failed: %{public}s", dlerror());
        }
        usleep(SLEEP_TIME_US);
    } while (!g_libHandle && cnt < retryTimes);
    return g_libHandle != nullptr;
}

void UnloadEdidPlugin(void)
{
    TLOGI(WmsLogTag::DMS, "unload edid plugin.");
    if (g_libHandle != nullptr) {
        dlclose(g_libHandle);
        g_libHandle = nullptr;
    }
}

bool GetEdid(ScreenId rsScreenId, struct BaseEdid &edid)
{
    std::vector<uint8_t> edidData;
    uint8_t outPort;
    int getEdidFromRS = RSInterfaces::GetInstance().GetDisplayIdentificationData(rsScreenId, outPort, edidData);
    if (getEdidFromRS != 0) {
        TLOGE(WmsLogTag::DMS, "get EDID from RS failed.");
        return false;
    }
    uint32_t edidSize = static_cast<uint32_t>(edidData.size());
    int32_t checkCode = GetEdidCheckCode(edidData);
    TLOGW(WmsLogTag::DMS, "EDID data size: %{public}u, check code: %{public}d",
        edidSize, checkCode);
    if (!LoadEdidPlugin()) {
        TLOGE(WmsLogTag::DMS, "dlopen failed.");
        return false;
    }
    ParseEdidFunc ParseBaseEdid = (ParseEdidFunc)(dlsym(g_libHandle, "ParseBaseEdid"));
    if (ParseBaseEdid == nullptr) {
        TLOGE(WmsLogTag::DMS, "ParseBaseEdid null.");
        UnloadEdidPlugin();
        return false;
    }
    int32_t getEdid = ParseBaseEdid(edidData.data(), edidSize, &edid);
    if (getEdid != 0) {
        TLOGE(WmsLogTag::DMS, "parse EDID failed.");
        return false;
    }
    return true;
}

int32_t GetEdidCheckCode(const std::vector<uint8_t>& edidData)
{
    int32_t checkCode = 0;
    if (edidData.size() <= CHECK_INDEX_END) {
        TLOGE(WmsLogTag::DMS, "EDID data size not enough!");
        return CHECK_CODE_INVALID;
    }
    for (uint32_t i = CHECK_INDEX_BEGIN; i <= CHECK_INDEX_END; i++) {
        checkCode += edidData[i];
    }
    return checkCode;
}
}
}