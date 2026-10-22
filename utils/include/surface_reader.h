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

#ifndef SURFACE_READER_H
#define SURFACE_READER_H

#include "refbase.h"

#include <surface.h>
#include "iconsumer_surface.h"
#include "surface_reader_handler.h"

namespace OHOS {
namespace Rosen {
class SurfaceReader {
public:
    SurfaceReader();
    virtual ~SurfaceReader();

    bool Init();
    void DeInit();

    sptr<Surface> GetSurface() const;
    void SetHandler(sptr<SurfaceReaderHandler> handler);
private:
    class BufferListener : public IBufferConsumerListener {
    public:
        explicit BufferListener(SurfaceReader& surfaceReader): surfaceReader_(surfaceReader)
        {
        }
        ~BufferListener() noexcept override = default;
        void OnBufferAvailable() override
        {
            surfaceReader_.OnVsync();
        }

    private:
        SurfaceReader &surfaceReader_;
    };
    friend class BufferListener;

    void OnVsync();
    bool ProcessBuffer(const sptr<SurfaceBuffer>& buf);

    struct BufferInfo {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t stride = 0;
        uint8_t *addr = nullptr;
    };
    bool GetBufferInfo(const sptr<SurfaceBuffer>& buf, BufferInfo& info) const;
    uint8_t *AllocateAndCopyBuffer(const BufferInfo& info, size_t& totalBytes) const;
    sptr<Media::PixelMap> CreatePixelMap(const BufferInfo& info, uint8_t *data, size_t totalBytes) const;

    sptr<IBufferConsumerListener> listener_ = nullptr;
    sptr<IConsumerSurface> csurface_ = nullptr; // cosumer surface
    sptr<Surface> psurface_ = nullptr; // producer surface
    sptr<SurfaceBuffer> prevBuffer_ = nullptr;
    sptr<SurfaceReaderHandler> handler_ = nullptr;
};
}
}

#endif // SURFACE_READER_H
