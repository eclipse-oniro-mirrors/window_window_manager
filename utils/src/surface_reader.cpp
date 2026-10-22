/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "surface_reader.h"
#include "sync_fence.h"
#include "window_manager_hilog.h"
#include "unique_fd.h"

#include <cstdint>
#include <securec.h>

using namespace OHOS::Media;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "SurfaceReader"};
} // namespace
const int BPP = 4; // bytes per pixel

SurfaceReader::SurfaceReader()
{
}

SurfaceReader::~SurfaceReader()
{
    if (csurface_ != nullptr) {
        csurface_->UnregisterConsumerListener();
    }
    psurface_ = nullptr;
    csurface_ = nullptr;
}

bool SurfaceReader::Init()
{
    csurface_ = IConsumerSurface::Create();
    if (csurface_ == nullptr) {
        return false;
    }

    auto producer = csurface_->GetProducer();
    psurface_ = Surface::CreateSurfaceAsProducer(producer);
    if (psurface_ == nullptr) {
        return false;
    }

    listener_ = new BufferListener(*this);
    SurfaceError ret = csurface_->RegisterConsumerListener(listener_);
    if (ret != SURFACE_ERROR_OK) {
        return false;
    }
    return true;
}

void SurfaceReader::OnVsync()
{
    WLOGI("SurfaceReader::OnVsync");

    sptr<SurfaceBuffer> cbuffer = nullptr;
    int32_t fence = -1;
    int64_t timestamp = 0;
    Rect damage;
    auto sret = csurface_->AcquireBuffer(cbuffer, fence, timestamp, damage);
    sptr<SyncFence> acquireFence = new SyncFence(fence);
    acquireFence->Wait(3000); // 3000ms
    if (cbuffer == nullptr || sret != OHOS::SURFACE_ERROR_OK) {
        WLOGFE("SurfaceReader::OnVsync: surface buffer is null");
        return;
    }

    if (!ProcessBuffer(cbuffer)) {
        WLOGFE("SurfaceReader::OnVsync: ProcessBuffer failed");
        return;
    }

    if (cbuffer != prevBuffer_) {
        if (prevBuffer_ != nullptr) {
            SurfaceError ret = csurface_->ReleaseBuffer(prevBuffer_, -1);
            if (ret != SURFACE_ERROR_OK) {
                WLOGFE("SurfaceReader::OnVsync: release buffer error");
                return;
            }
        }

        prevBuffer_ = cbuffer;
    }
}

sptr<Surface> SurfaceReader::GetSurface() const
{
    return psurface_;
}

void SurfaceReader::SetHandler(sptr<SurfaceReaderHandler> handler)
{
    handler_ = handler;
}

bool SurfaceReader::ProcessBuffer(const sptr<SurfaceBuffer>& buf)
{
    if (handler_ == nullptr) {
        WLOGFE("SurfaceReaderHandler not set");
        return false;
    }

    BufferInfo info;
    if (!GetBufferInfo(buf, info)) {
        return false;
    }

    size_t totalBytes = 0;
    uint8_t *data = AllocateAndCopyBuffer(info, totalBytes);
    if (data == nullptr) {
        return false;
    }

    sptr<Media::PixelMap> pixelMap = CreatePixelMap(info, data, totalBytes);
    if (pixelMap == nullptr) {
        free(data);
        return false;
    }

    handler_->OnImageAvailable(pixelMap);
    return true;
}

bool SurfaceReader::GetBufferInfo(const sptr<SurfaceBuffer>& buf, BufferInfo& info) const
{
    if (buf == nullptr) {
        WLOGFE("buffer is nullptr");
        return false;
    }
    BufferHandle *bufferHandle = buf->GetBufferHandle();
    if (bufferHandle == nullptr) {
        WLOGFE("bufferHandle nullptr");
        return false;
    }
    info.width = static_cast<uint32_t>(bufferHandle->width);
    info.height = static_cast<uint32_t>(bufferHandle->height);
    info.stride = static_cast<uint32_t>(bufferHandle->stride);
    info.addr = static_cast<uint8_t *>(buf->GetVirAddr());
    if (info.addr == nullptr) {
        WLOGFE("buffer virAddr is nullptr");
        return false;
    }
    if (info.width == 0 || info.height == 0 || info.stride == 0) {
        WLOGFE("invalid dimension w:%{public}u h:%{public}u stride:%{public}u", info.width, info.height, info.stride);
        return false;
    }
    return true;
}

uint8_t *SurfaceReader::AllocateAndCopyBuffer(const BufferInfo& info, size_t& totalBytes) const
{
    size_t rowBytes = static_cast<size_t>(info.width) * static_cast<size_t>(BPP);
    if (rowBytes > SIZE_MAX / info.height) {
        WLOGFE("buffer size overflow w:%{public}u h:%{public}u", info.width, info.height);
        return nullptr;
    }
    totalBytes = rowBytes * info.height;
    if (totalBytes > UINT32_MAX) {
        WLOGFE("buffer size too large w:%{public}u h:%{public}u", info.width, info.height);
        return nullptr;
    }
    if (info.stride < rowBytes) {
        WLOGFE("stride %{public}u smaller than rowBytes", info.stride);
        return nullptr;
    }
    auto data = static_cast<uint8_t *>(malloc(totalBytes));
    if (data == nullptr) {
        WLOGFE("data malloc failed");
        return nullptr;
    }
    for (uint32_t i = 0; i < info.height; i++) {
        size_t dstOff = rowBytes * i;
        size_t srcOff = static_cast<size_t>(info.stride) * i;
        errno_t ret = memcpy_s(data + dstOff, rowBytes, info.addr + srcOff, rowBytes);
        if (ret != EOK) {
            WLOGFE("memcpy failed");
            free(data);
            return nullptr;
        }
    }
    return data;
}

sptr<Media::PixelMap> SurfaceReader::CreatePixelMap(
    const BufferInfo& info, uint8_t *data, size_t totalBytes) const
{
    Media::InitializationOptions opts;
    opts.size.width = static_cast<int32_t>(info.width);
    opts.size.height = static_cast<int32_t>(info.height);
    std::unique_ptr<Media::PixelMap> pixelMapPtr = Media::PixelMap::Create(opts);
    sptr<Media::PixelMap> pixelMap(pixelMapPtr.release());
    if (pixelMap == nullptr) {
        WLOGFE("create pixelMap failed");
        return nullptr;
    }

    ImageInfo imageInfo;
    imageInfo.size.width = static_cast<int32_t>(info.width);
    imageInfo.size.height = static_cast<int32_t>(info.height);
    imageInfo.pixelFormat = OHOS::Media::PixelFormat::RGBA_8888;
    imageInfo.colorSpace = ColorSpace::SRGB;
    pixelMap->SetImageInfo(imageInfo);

    pixelMap->SetPixelsAddr(data, nullptr, static_cast<uint32_t>(totalBytes), AllocatorType::HEAP_ALLOC, nullptr);
    return pixelMap;
}
}
}
