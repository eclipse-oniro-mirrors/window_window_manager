/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <unistd.h>
#include "surface_reader.h"
#include "surface_reader_handler_impl.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SurfaceReaderTest : public testing::Test {
public:
    SurfaceReaderTest() {}
    ~SurfaceReaderTest() {}
    static sptr<SurfaceBuffer> RequestTestBuffer(const sptr<Surface>& surface, uint32_t w, uint32_t h);
};

sptr<SurfaceBuffer> SurfaceReaderTest::RequestTestBuffer(const sptr<Surface>& surface, uint32_t w, uint32_t h)
{
    if (surface == nullptr) {
        return nullptr;
    }
    sptr<SurfaceBuffer> buffer;
    int32_t fence = 0;
    OHOS::BufferRequestConfig config = {
        .width = static_cast<int32_t>(w),
        .height = static_cast<int32_t>(h),
        .strideAlignment = 0x8,
        .format = GRAPHIC_PIXEL_FMT_RGBA_8888,
        .usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA,
    };
    OHOS::SurfaceError ret = surface->RequestBuffer(buffer, fence, config);
    if (ret != OHOS::SURFACE_ERROR_OK) {
        return nullptr;
    }
    return buffer;
}

namespace {
/**
 * @tc.name: Init
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, Init, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SurfaceReaderTest: Init start";
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    bool res = reader->Init();
    ASSERT_EQ(res, true);
    delete reader;
    GTEST_LOG_(INFO) << "SurfaceReaderTest: Init end";
}

/**
 * @tc.name: OnVsync
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, OnVsync, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    bool res = reader->Init();
    reader->OnVsync();
    delete reader;
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: GetSurface
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, GetSurface, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    reader->Init();
    sptr<Surface> surface = reader->GetSurface();
    ASSERT_NE(surface, nullptr);
    delete reader;
}

/**
 * @tc.name: SetHandler
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, SetHandler, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    reader->SetHandler(nullptr);
    ASSERT_TRUE(true);
    delete reader;
}

/**
 * @tc.name: ProcessBufferNullHandler
 * @tc.desc: ProcessBuffer returns false when handler is not set
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, ProcessBufferNullHandler, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    reader->Init();
    reader->SetHandler(nullptr);
    EXPECT_FALSE(reader->ProcessBuffer(nullptr));
    delete reader;
}

/**
 * @tc.name: ProcessBufferNullBuffer
 * @tc.desc: ProcessBuffer returns false when buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, ProcessBufferNullBuffer, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    reader->Init();
    sptr<SurfaceReaderHandlerImpl> handler = new SurfaceReaderHandlerImpl();
    reader->SetHandler(handler);
    EXPECT_FALSE(reader->ProcessBuffer(nullptr));
    delete reader;
}

/**
 * @tc.name: ProcessBufferNormalBuffer
 * @tc.desc: ProcessBuffer succeeds for a valid small buffer and delivers a PixelMap
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, ProcessBufferNormalBuffer, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    ASSERT_TRUE(reader->Init());
    sptr<SurfaceReaderHandlerImpl> handler = new SurfaceReaderHandlerImpl();
    reader->SetHandler(handler);
    sptr<SurfaceBuffer> buffer = RequestTestBuffer(reader->GetSurface(), 4, 4);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        GTEST_LOG_(INFO) << "SurfaceReaderTest: skip, surface buffer unavailable";
        delete reader;
        return;
    }
    EXPECT_TRUE(reader->ProcessBuffer(buffer));
    EXPECT_TRUE(handler->IsImageOk());
    sptr<Media::PixelMap> pixelMap = handler->GetPixelMap();
    ASSERT_NE(pixelMap, nullptr);
    EXPECT_EQ(pixelMap->GetWidth(), 4);
    EXPECT_EQ(pixelMap->GetHeight(), 4);
    delete reader;
}

/**
 * @tc.name: ProcessBufferOverflowDimension
 * @tc.desc: ProcessBuffer rejects dimensions whose row*height overflows size_t
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, ProcessBufferOverflowDimension, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    ASSERT_TRUE(reader->Init());
    sptr<SurfaceReaderHandlerImpl> handler = new SurfaceReaderHandlerImpl();
    reader->SetHandler(handler);
    sptr<SurfaceBuffer> buffer = RequestTestBuffer(reader->GetSurface(), 4, 4);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        GTEST_LOG_(INFO) << "SurfaceReaderTest: skip, surface buffer unavailable";
        delete reader;
        return;
    }
    BufferHandle* handle = buffer->GetBufferHandle();
    ASSERT_NE(handle, nullptr);
    auto origW = handle->width;
    auto origH = handle->height;
    handle->width = -1;  // 0xFFFFFFFF as uint32 -> rowBytes*height overflows size_t
    handle->height = -1;
    EXPECT_FALSE(reader->ProcessBuffer(buffer));
    handle->width = origW;
    handle->height = origH;
    delete reader;
}

/**
 * @tc.name: ProcessBufferTooLargeDimension
 * @tc.desc: ProcessBuffer rejects sizes that exceed UINT32_MAX (PixelMap size limit)
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, ProcessBufferTooLargeDimension, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    ASSERT_TRUE(reader->Init());
    sptr<SurfaceReaderHandlerImpl> handler = new SurfaceReaderHandlerImpl();
    reader->SetHandler(handler);
    sptr<SurfaceBuffer> buffer = RequestTestBuffer(reader->GetSurface(), 4, 4);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        GTEST_LOG_(INFO) << "SurfaceReaderTest: skip, surface buffer unavailable";
        delete reader;
        return;
    }
    BufferHandle* handle = buffer->GetBufferHandle();
    ASSERT_NE(handle, nullptr);
    auto origW = handle->width;
    auto origH = handle->height;
    handle->width = 0x10000;  // 65536x65536x4 = 16GB, exceeds UINT32_MAX
    handle->height = 0x10000;
    EXPECT_FALSE(reader->ProcessBuffer(buffer));
    handle->width = origW;
    handle->height = origH;
    delete reader;
}

/**
 * @tc.name: ProcessBufferZeroDimension
 * @tc.desc: ProcessBuffer rejects zero width/height/stride
 * @tc.type: FUNC
 */
HWTEST_F(SurfaceReaderTest, ProcessBufferZeroDimension, TestSize.Level1)
{
    SurfaceReader* reader = new (std::nothrow) SurfaceReader();
    ASSERT_TRUE(reader->Init());
    sptr<SurfaceReaderHandlerImpl> handler = new SurfaceReaderHandlerImpl();
    reader->SetHandler(handler);
    sptr<SurfaceBuffer> buffer = RequestTestBuffer(reader->GetSurface(), 4, 4);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        GTEST_LOG_(INFO) << "SurfaceReaderTest: skip, surface buffer unavailable";
        delete reader;
        return;
    }
    BufferHandle* handle = buffer->GetBufferHandle();
    ASSERT_NE(handle, nullptr);
    auto origW = handle->width;
    handle->width = 0;
    EXPECT_FALSE(reader->ProcessBuffer(buffer));
    handle->width = origW;
    delete reader;
}
} // namespace
} // namespace Rosen
} // namespace OHOS
