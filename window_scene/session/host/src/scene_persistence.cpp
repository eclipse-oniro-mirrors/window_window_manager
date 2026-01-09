/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "session/host/include/scene_persistence.h"

#include <sys/stat.h>

#include <hitrace_meter.h>
#include <image_packer.h>
#include <parameters.h>

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "ScenePersistence" };
constexpr const char* UNDERLINE_SEPARATOR = "_";
constexpr const char* ASTC_IMAGE_FORMAT_LOW = "image/astc/8*8";
constexpr const char* ASTC_IMAGE_FORMAT_HIGH = "image/astc/4*4";
constexpr const char* ASTC_IMAGE_SUFFIX = ".astc";
constexpr uint8_t ASTC_IMAGE_QUALITY = 20;

constexpr const char* IMAGE_FORMAT = "image/png";
constexpr const char* IMAGE_SUFFIX = ".png";
constexpr uint8_t IMAGE_QUALITY = 100;
constexpr int32_t ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT = 1024;
constexpr double ICON_IMAGE_MAX_SCALE = 1;
constexpr uint8_t SUCCESS = 0;
} // namespace

std::string ScenePersistence::snapshotDirectory_;
std::string ScenePersistence::updatedIconDirectory_;
std::shared_ptr<WSFFRTHelper> ScenePersistence::snapshotFfrtHelper_;
bool ScenePersistence::isAstcEnabled_ = false;

bool ScenePersistence::CreateSnapshotDir(const std::string& directory)
{
    snapshotDirectory_ = directory + "/SceneSnapShot/";
    if (mkdir(snapshotDirectory_.c_str(), S_IRWXU)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "mkdir failed or the directory already exists");
        return false;
    }
    return true;
}

bool ScenePersistence::CreateUpdatedIconDir(const std::string& directory)
{
    updatedIconDirectory_ = directory + "/UpdatedIcon/";
    if (mkdir(updatedIconDirectory_.c_str(), S_IRWXU)) {
        TLOGD(WmsLogTag::DEFAULT, "mkdir failed or the directory already exists");
        return false;
    }
    return true;
}

void ScenePersistence::SetSnapshotCapacity(SnapshotStatus capacity)
{
    capacity_ = capacity;
}

ScenePersistence::ScenePersistence(const std::string& bundleName, int32_t persistentId, SnapshotStatus capacity)
    : bundleName_(bundleName), persistentId_(persistentId), capacity_(capacity)
{
    InitAstcEnabled();
    auto suffix = isAstcEnabled_ ? ASTC_IMAGE_SUFFIX : IMAGE_SUFFIX;
    for (int32_t screenStatus = SCREEN_UNKNOWN; screenStatus < SCREEN_COUNT; screenStatus++) {
        snapshotPath_[screenStatus] = snapshotDirectory_ + bundleName + UNDERLINE_SEPARATOR +
            std::to_string(persistentId) + UNDERLINE_SEPARATOR + std::to_string(screenStatus) + suffix;
    }
    snapshotFreeMultiWindowPath_ = snapshotDirectory_ + bundleName + UNDERLINE_SEPARATOR +
        std::to_string(persistentId) + suffix;
    updatedIconPath_ = updatedIconDirectory_ + bundleName + IMAGE_SUFFIX;
    if (snapshotFfrtHelper_ == nullptr) {
        snapshotFfrtHelper_ = std::make_shared<WSFFRTHelper>();
    }
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "HandsetSmartWindow");
    isPcWindow_ = (multiWindowUIType == "FreeFormMultiWindow");
}

ScenePersistence::~ScenePersistence()
{
    ClearSnapshotPath();
}

void ScenePersistence::ClearSnapshotPath()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "persistentId: %{public}d", persistentId_);
    int ret = 0;
    for (const auto& snapshotPath : snapshotPath_) {
        ret = remove(snapshotPath.c_str());
        TLOGE(WmsLogTag::WMS_PATTERN, "ret: %{public}d", ret);
    }
    ret = remove(snapshotFreeMultiWindowPath_.c_str());
    TLOGE(WmsLogTag::WMS_PATTERN, "ret: %{public}d", ret);
}

std::shared_ptr<WSFFRTHelper> ScenePersistence::GetSnapshotFfrtHelper() const
{
    return snapshotFfrtHelper_;
}

void ScenePersistence::InitAstcEnabled()
{
    static bool isAstcEnabled = system::GetBoolParameter("persist.multimedia.image.astc.enabled", true);
    isAstcEnabled_ = isAstcEnabled;
}

bool ScenePersistence::IsAstcEnabled()
{
    return isAstcEnabled_;
}

void ScenePersistence::SaveSnapshot(const std::shared_ptr<Media::PixelMap>& pixelMap,
    const std::function<void()> resetSnapshotCallback, SnapshotStatus key, DisplayOrientation rotate,
    bool freeMultiWindow)
{
    savingSnapshotSum_.fetch_add(1);
    SetIsSavingSnapshot(true);
    TLOGI(WmsLogTag::WMS_PATTERN, "isSavingSnapshot:%{public}d", isSavingSnapshot_.load());
    std::string path = freeMultiWindow ? snapshotFreeMultiWindowPath_ : snapshotPath_[key];
    auto task = [weakThis = wptr(this), pixelMap, resetSnapshotCallback,
        savingSnapshotSum = savingSnapshotSum_.load(), key, rotate, path, freeMultiWindow]() {
        auto scenePersistence = weakThis.promote();
        if (scenePersistence == nullptr || pixelMap == nullptr ||
            path.find('/') == std::string::npos) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "scenePersistence %{public}s nullptr, pixelMap %{public}s nullptr",
                scenePersistence == nullptr ? "" : "not", pixelMap == nullptr ? "" : "not");
            resetSnapshotCallback();
            return;
        }

        TLOGNI(WmsLogTag::WMS_PATTERN, "Save snapshot begin");
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SaveSnapshot %s", path.c_str());
        OHOS::Media::ImagePacker imagePacker;
        OHOS::Media::PackOption option;
        const char *astcImageFormat = scenePersistence->isPcWindow_ ? ASTC_IMAGE_FORMAT_LOW : ASTC_IMAGE_FORMAT_HIGH;
        option.format = IsAstcEnabled() ? astcImageFormat : IMAGE_FORMAT;
        option.quality = IsAstcEnabled() ? ASTC_IMAGE_QUALITY : IMAGE_QUALITY;
        option.numberHint = 1;

        scenePersistence->SetSnapshotSize(key, freeMultiWindow, { pixelMap->GetWidth(), pixelMap->GetHeight() });
        std::lock_guard lock(scenePersistence->savingSnapshotMutex_);
        remove(path.c_str());
        if (imagePacker.StartPacking(path, option)) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "Save snapshot failed, starting packing error");
            resetSnapshotCallback();
            return;
        }
        if (imagePacker.AddImage(*pixelMap)) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "Save snapshot failed, adding image error");
            resetSnapshotCallback();
            return;
        }
        int64_t packedSize = 0;
        if (imagePacker.FinalizePacking(packedSize)) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "Save snapshot failed, finalizing packing error");
            resetSnapshotCallback();
            return;
        }
        // If the current num is equals to the latest num, it is the last saveSnapshot task
        if (savingSnapshotSum == scenePersistence->savingSnapshotSum_.load()) {
            resetSnapshotCallback();
        }
        scenePersistence->rotate_[key] = rotate;
        TLOGNI(WmsLogTag::WMS_PATTERN, "Save snapshot end, packed size %{public}" PRIu64, packedSize);
    };
    snapshotFfrtHelper_->SubmitTask(std::move(task), "SaveSnapshot" + path);
}

bool ScenePersistence::IsSavingSnapshot()
{
    return isSavingSnapshot_.load();
}

void ScenePersistence::SetIsSavingSnapshot(bool isSavingSnapshot)
{
    isSavingSnapshot_.store(isSavingSnapshot);
}

void ScenePersistence::ResetSnapshotCache()
{
    isSavingSnapshot_.store(false);
}

void ScenePersistence::RenameSnapshotFromOldPersistentId(const int32_t& oldPersistentId)
{
    auto task = [weakThis = wptr(this), oldPersistentId]() {
        auto scenePersistence = weakThis.promote();
        if (scenePersistence == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "scenePersistence is nullptr");
            return;
        }
        for (int32_t screenStatus = SCREEN_UNKNOWN; screenStatus < SCREEN_COUNT; screenStatus++) {
            scenePersistence->RenameSnapshotFromOldPersistentId(oldPersistentId, screenStatus);
        }
        auto suffix = scenePersistence->isAstcEnabled_ ? ASTC_IMAGE_SUFFIX : IMAGE_SUFFIX;
        std::string oldSnapshotFreeMultiWindowPath = snapshotDirectory_ + scenePersistence->bundleName_ +
            UNDERLINE_SEPARATOR + std::to_string(oldPersistentId) + suffix;
        auto& snapshotPath = scenePersistence->snapshotFreeMultiWindowPath_;
        std::lock_guard lock(scenePersistence->savingSnapshotMutex_);
        int ret = std::rename(oldSnapshotFreeMultiWindowPath.c_str(), snapshotPath.c_str());
        if (ret == 0) {
            TLOGNI(WmsLogTag::WMS_PATTERN, "Rename snapshot from %{public}s to %{public}s.",
                oldSnapshotFreeMultiWindowPath.c_str(), snapshotPath.c_str());
        } else {
            TLOGNW(WmsLogTag::WMS_PATTERN, "Failed to rename snapshot from %{public}s to %{public}s.",
                oldSnapshotFreeMultiWindowPath.c_str(), snapshotPath.c_str());
        }
    };
    snapshotFfrtHelper_->SubmitTask(std::move(task), "RenameSnapshotFromOldPersistentId"
        + std::to_string(oldPersistentId));
}

void ScenePersistence::RenameSnapshotFromOldPersistentId(const int32_t& oldPersistentId, SnapshotStatus key)
{
    auto& snapshotPath = snapshotPath_[key];
    auto suffix = isAstcEnabled_ ? ASTC_IMAGE_SUFFIX : IMAGE_SUFFIX;
    std::string oldSnapshotPath = snapshotDirectory_ + bundleName_ + UNDERLINE_SEPARATOR +
        std::to_string(oldPersistentId) + UNDERLINE_SEPARATOR + std::to_string(key) + suffix;
    std::lock_guard lock(savingSnapshotMutex_);
    int ret = std::rename(oldSnapshotPath.c_str(), snapshotPath.c_str());
    if (ret == 0) {
        TLOGI(WmsLogTag::WMS_PATTERN, "Rename snapshot from %{public}s to %{public}s.",
            oldSnapshotPath.c_str(), snapshotPath.c_str());
    } else {
        TLOGW(WmsLogTag::WMS_PATTERN, "Failed to rename snapshot from %{public}s to %{public}s.",
            oldSnapshotPath.c_str(), snapshotPath.c_str());
    }
}

std::string ScenePersistence::GetSnapshotFilePath(SnapshotStatus& key, bool useKey, bool freeMultiWindow)
{
    if (freeMultiWindow) {
        return snapshotFreeMultiWindowPath_;
    }
    if (useKey || HasSnapshot(key, false)) {
        return snapshotPath_[key];
    }
    if (FindClosestFormSnapshot(key)) {
        return snapshotPath_[key];
    }
    TLOGW(WmsLogTag::WMS_PATTERN, "Failed");
    return snapshotPath_[SCREEN_UNKNOWN];
}

bool ScenePersistence::FindClosestFormSnapshot(SnapshotStatus& key)
{
    std::lock_guard lock(hasSnapshotMutex_);
    if (hasSnapshot_[key]) {
        return true;
    }
    bool isFolded = (key == SCREEN_FOLDED);
    if (isFolded) {
        for (int32_t screenStatus = SCREEN_EXPAND; screenStatus >= SCREEN_UNKNOWN; screenStatus--) {
            if (hasSnapshot_[screenStatus]) {
                key = screenStatus;
                return true;
            }
        }
        return false;
    }
    for (int32_t screenStatus = SCREEN_UNKNOWN; screenStatus < capacity_; screenStatus++) {
        if (hasSnapshot_[screenStatus]) {
            key = screenStatus;
            return true;
        }
    }
    return false;
}

void ScenePersistence::SaveUpdatedIcon(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    if (pixelMap == nullptr || updatedIconPath_.find('/') == std::string::npos) {
        return;
    }

    OHOS::Media::ImagePacker imagePacker;
    OHOS::Media::PackOption option;
    option.format = IMAGE_FORMAT;
    option.quality = IMAGE_QUALITY;
    option.numberHint = 1;
    if (pixelMap->GetWidth() > ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT ||
        pixelMap->GetHeight() > ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT) {
        // large image need scale
        double xScale = pixelMap->GetWidth() > ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT ?
            ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT / static_cast<double>(pixelMap->GetWidth()) : ICON_IMAGE_MAX_SCALE;
        double yScale = pixelMap->GetHeight() > ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT ?
            ICON_IMAGE_WIDTH_HEIGHT_SIZE_LIMIT / static_cast<double>(pixelMap->GetHeight()) : ICON_IMAGE_MAX_SCALE;
        pixelMap->scale(xScale, yScale, Media::AntiAliasingOption::MEDIUM);
    }
    if (remove(updatedIconPath_.c_str())) {
        TLOGD(WmsLogTag::DEFAULT, "Failed to delete old file");
    }
    if (imagePacker.StartPacking(GetUpdatedIconPath(), option)) {
        TLOGE(WmsLogTag::DEFAULT, "Save updated icon failed, starting packing error");
        return;
    }
    if (imagePacker.AddImage(*pixelMap)) {
        TLOGE(WmsLogTag::DEFAULT, "Save updated icon failed, adding image error");
        return;
    }
    int64_t packedSize = 0;
    if (imagePacker.FinalizePacking(packedSize)) {
        TLOGE(WmsLogTag::DEFAULT, "Save updated icon failed, finalizing packing error");
        return;
    }
    TLOGD(WmsLogTag::DEFAULT, "SaveUpdatedIcon finished");
}

std::string ScenePersistence::GetUpdatedIconPath() const
{
    return updatedIconPath_;
}

void ScenePersistence::SetSnapshotSize(SnapshotStatus key, bool freeMultiWindow, std::pair<uint32_t, uint32_t> size)
{
    std::lock_guard lock(snapshotSizeMutex_);
    if (freeMultiWindow) {
        snapshotFreeMultiWindowSize_ = size;
    } else {
        snapshotSize_[key] = size;
    }
}

std::pair<uint32_t, uint32_t> ScenePersistence::GetSnapshotSize(SnapshotStatus key, bool freeMultiWindow) const
{
    std::lock_guard lock(snapshotSizeMutex_);
    if (freeMultiWindow) {
        return snapshotFreeMultiWindowSize_;
    }
    return snapshotSize_[key];
}

void ScenePersistence::SetHasSnapshot(bool hasSnapshot, SnapshotStatus key)
{
    std::lock_guard lock(hasSnapshotMutex_);
    hasSnapshot_[key] = hasSnapshot;
}

void ScenePersistence::SetHasSnapshotFreeMultiWindow(bool hasSnapshot)
{
    std::lock_guard lock(hasSnapshotMutex_);
    hasSnapshotFreeMultiWindow_ = hasSnapshot;
}

bool ScenePersistence::HasSnapshot() const
{
    std::lock_guard lock(hasSnapshotMutex_);
    for (const auto& hasSnapshot : hasSnapshot_) {
        if (hasSnapshot) {
            return true;
        }
    }
    return hasSnapshotFreeMultiWindow_;
}

bool ScenePersistence::HasSnapshot(SnapshotStatus key, bool freeMultiWindow) const
{
    std::lock_guard lock(hasSnapshotMutex_);
    if (freeMultiWindow) {
        return hasSnapshotFreeMultiWindow_;
    }
    return hasSnapshot_[key];
}

void ScenePersistence::ClearSnapshot()
{
    std::lock_guard lock(hasSnapshotMutex_);
    for (auto& hasSnapshot : hasSnapshot_) {
        hasSnapshot = false;
    }
    hasSnapshotFreeMultiWindow_ = false;
}

bool ScenePersistence::IsSnapshotExisted(SnapshotStatus key)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "IsSnapshotExisted");
    struct stat buf;
    if (stat(snapshotPath_[key].c_str(), &buf)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "Snapshot file %{public}s does not exist",
            snapshotPath_[key].c_str());
        return false;
    }
    if (!S_ISREG(buf.st_mode)) {
        return false;
    }
    SetHasSnapshot(true, key);
    return true;
}

std::shared_ptr<Media::PixelMap> ScenePersistence::GetLocalSnapshotPixelMap(const float oriScale,
    const float newScale, SnapshotStatus key, bool freeMultiWindow)
{
    if (!IsSnapshotExisted(key)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "local snapshot pic is not existed");
        return nullptr;
    }

    uint32_t errorCode = 0;
    Media::SourceOptions sourceOpts;
    const char *astcImageFormat = this->isPcWindow_ ? ASTC_IMAGE_FORMAT_LOW : ASTC_IMAGE_FORMAT_HIGH;
    sourceOpts.formatHint = IsAstcEnabled() ? astcImageFormat : IMAGE_FORMAT;
    std::string path = GetSnapshotFilePath(key, true, freeMultiWindow);
    std::lock_guard lock(savingSnapshotMutex_);
    auto imageSource = Media::ImageSource::CreateImageSource(path, sourceOpts, errorCode);
    if (!imageSource) {
        TLOGE(WmsLogTag::WMS_PATTERN, "create image source fail, errCode: %{public}u", errorCode);
        return nullptr;
    }

    Media::ImageInfo info;
    int32_t decoderWidth = 0;
    int32_t decoderHeight = 0;
    errorCode = imageSource->GetImageInfo(info);
    if (errorCode == Rosen::SUCCESS) {
        decoderWidth = info.size.width;
        decoderHeight = info.size.height;
    }
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = Media::PixelFormat::RGBA_8888;
    if (oriScale != 0 && decoderWidth > 0 && decoderHeight > 0) {
        auto isNeedToScale = newScale < oriScale;
        decodeOpts.desiredSize.width = isNeedToScale ?
            static_cast<int>(decoderWidth * newScale / oriScale) : decoderWidth;
        decodeOpts.desiredSize.height = isNeedToScale ?
            static_cast<int>(decoderHeight * newScale / oriScale) : decoderHeight;
    }
    return imageSource->CreatePixelMap(decodeOpts, errorCode);
}
} // namespace OHOS::Rosen
