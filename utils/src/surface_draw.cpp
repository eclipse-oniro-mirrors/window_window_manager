/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <hitrace_meter.h>
#include <surface.h>
#include "surface_draw.h"
#include <transaction/rs_interfaces.h>
#include <ui/rs_surface_extractor.h>
#include <ui/rs_ui_context.h>

#include "image/bitmap.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "render/rs_pixel_map_util.h"
#include "surface_capture_future.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SurfaceDraw"};
constexpr uint32_t IMAGE_BYTES_STRIDE = 4;
constexpr float CENTER_IN_RECT = 0.5;                     // Multiply by 0.5 to obtain the coordinates in center
constexpr float FIXED_BOTTOM_SAFE_AREA_HEIGHT_VP = 28.0;  // 28.0 indicates fixed bottom safe area height of windowRect
constexpr float FIXED_TOP_SAFE_AREA_HEIGHT_VP = 36.0;     // 36.0 indicates fixed top safe srea height off windowRect
constexpr float MAX_BRAND_CONTENT_WIDTH_VP = 400.0;       // 400.0 indicates max brand content width of windowRect
constexpr float MIN_BRAND_CONTENT_HEIGHT_VP = 80.0;       // 80.0 indicates min brand content height of windowRect
constexpr float MIN_RECT_HEIGHT_VP = 100.0;               // 100.0 indicates min rect height of windowRect
constexpr float SIDE_DISTANCE_VP = 16.0;                  // 16.0 indicates side distance of windowRect
constexpr float WIDTH_THRESHOLD_SMALL_VP = 320.0;         // Small threshold, use it when windowWidth <=320
constexpr float WIDTH_THRESHOLD_MEDIUM_VP = 600.0;        // Medium threshold, use it when windowWidth <=600
constexpr float WIDTH_THRESHOLD_LARGE_VP = 840.0;         // Large threshold, use it when windowWidth <=840
constexpr float ICON_SIZE_SMALL_VP = 128.0;               // 128.0 indicates small app icon sideLength
constexpr float ICON_SIZE_MEDIUM_VP = 192.0;              // 192.0 indicates medium app icon sideLength
constexpr float ICON_SIZE_LARGE_VP = 256.0;               // 256.0 indicates large app icon sideLength
constexpr float ASPECT_RATIO_1 = 9.0f / 7.2f;             // windowRect Aspect ratio threshold 9 : 7.2
constexpr float ASPECT_RATIO_2 = 9.0f / 10.8f;            // windwoRect Aspect ratio threshold 9 : 10.8
constexpr float EIGHTY_PERCENT = 0.8;                     // 0.8 indicates eighty percent
constexpr float FORTY_PERCENT = 0.4;                      // 0.4 indicates forty percent
constexpr float SEVENTY_PERCENT = 0.7;                    // 0.7 indicates seventy percent
constexpr float THIRTY_PERCENT = 0.3;                     // 0.3 indicates thirty percent
} // namespace

bool IsValidPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    if (pixelMap == nullptr) {
        TLOGD(WmsLogTag::WMS_PATTERN, "pixelMap is nullptr.");
        return false;
    }
    return pixelMap->GetHeight() > 0 && pixelMap->GetWidth() > 0;
}

template <typename T>
void TryDrawResource(const std::shared_ptr<Rosen::ResourceInfo>& resource, uint32_t frameIdx, T&& drawFunc,
    const char* resourceName)
{
    if (!resource || resource->pixelMaps.empty()) {
        TLOGD(WmsLogTag::WMS_PATTERN, "draw failed, null resource.");
        return;
    }
    uint32_t idx = (resource->pixelMaps.size() > 1) ? frameIdx : 0;
    const auto& map = resource->pixelMaps[idx];
    if (IsValidPixelMap(map)) {
        if (!drawFunc(map)) {
            TLOGD(WmsLogTag::WMS_PATTERN, "draw StartingWindow %{public}s failed.", resourceName);
        }
    }
}

bool IsValidRect(const Rect& targetRect, const Rect& winRect)
{
    bool valid = targetRect.posX_ > 0
              && targetRect.posY_ > 0
              && targetRect.width_ > 0
              && targetRect.height_ > 0
              && (targetRect.posX_ + targetRect.width_) < winRect.width_
              && (targetRect.posY_ + targetRect.height_) < winRect.height_;
    return valid;
}

Rect GetAppIconRect(const Rect& winRect, const float ratio)
{
    float width  = static_cast<float>(winRect.width_);
    float height = static_cast<float>(winRect.height_);
    float aspectRatio = width / height;
    float sideLen = ICON_SIZE_LARGE_VP * ratio;
    if (width <= WIDTH_THRESHOLD_SMALL_VP * ratio) {
        sideLen = ICON_SIZE_SMALL_VP * ratio;
    } else if (width <= WIDTH_THRESHOLD_MEDIUM_VP * ratio) {
        sideLen = MathHelper::GreatNotEqual(aspectRatio, ASPECT_RATIO_2) ?
            ICON_SIZE_SMALL_VP * ratio : ICON_SIZE_MEDIUM_VP * ratio;
    } else if (width <= WIDTH_THRESHOLD_LARGE_VP * ratio) {
        sideLen = MathHelper::GreatNotEqual(aspectRatio, ASPECT_RATIO_1) ?
            ICON_SIZE_MEDIUM_VP * ratio : ICON_SIZE_LARGE_VP * ratio;
    }
    float posX = (width - sideLen) * CENTER_IN_RECT;
    float posY = (SEVENTY_PERCENT * height - FIXED_TOP_SAFE_AREA_HEIGHT_VP * ratio - sideLen) *
        CENTER_IN_RECT + FIXED_TOP_SAFE_AREA_HEIGHT_VP * ratio;

    Rosen::Rect appIconRect {
        static_cast<int32_t>(posX),
        static_cast<int32_t>(posY),
        static_cast<uint32_t>(sideLen),
        static_cast<uint32_t>(sideLen)
    };
    return appIconRect;
}

Rect GetAboveRect(const Rect& winRect, const float ratio)
{
    float width  = static_cast<float>(winRect.width_);
    float height = static_cast<float>(winRect.height_);

    float axis = (height * CENTER_IN_RECT) / width;
    float sideLen = MathHelper::GreatNotEqual(axis, 1.0f) ?
        width * EIGHTY_PERCENT : (height * CENTER_IN_RECT) * EIGHTY_PERCENT;

    float posX = (width - sideLen) * CENTER_IN_RECT;
    float posY = (SEVENTY_PERCENT * height - FIXED_TOP_SAFE_AREA_HEIGHT_VP * ratio - sideLen) *
        CENTER_IN_RECT + FIXED_TOP_SAFE_AREA_HEIGHT_VP * ratio;
    Rosen::Rect aboveRect {
        static_cast<int32_t>(posX),
        static_cast<int32_t>(posY),
        static_cast<uint32_t>(sideLen),
        static_cast<uint32_t>(sideLen)
    };
    return aboveRect;
}

Rect GetBelowRect(const Rect& winRect, const float ratio)
{
    float width  = static_cast<float>(winRect.width_);
    float height = static_cast<float>(winRect.height_);

    float rectWidth = width - 2 * SIDE_DISTANCE_VP * ratio; // 2 indicates double
    float rectHeight = (height * THIRTY_PERCENT) * FORTY_PERCENT; // 40% height based on 30% of winRect height

    Rosen::Rect belowRect { 0, 0,
        static_cast<uint32_t>(MathHelper::Min(rectWidth, MAX_BRAND_CONTENT_WIDTH_VP * ratio)),
        static_cast<uint32_t>(MathHelper::Max(rectHeight, MIN_BRAND_CONTENT_HEIGHT_VP * ratio))
    };
    belowRect.posX_ = static_cast<int32_t>((width - belowRect.width_) * CENTER_IN_RECT);
    belowRect.posY_ = static_cast<int32_t>((height * THIRTY_PERCENT - FIXED_BOTTOM_SAFE_AREA_HEIGHT_VP * ratio -
        belowRect.height_) * CENTER_IN_RECT + height * SEVENTY_PERCENT);
    return belowRect;
}

void FitAndDraw(const std::shared_ptr<Media::PixelMap>& pixelMap, const Rect& targetRect,
    Drawing::Canvas& canvas, ImageFit fit)
{
    auto rsImage = std::make_shared<Rosen::RSImage>();
    if (rsImage == nullptr) {
        TLOGD(WmsLogTag::WMS_PATTERN, "rsImage is nullptr.");
        return;
    }
    rsImage->SetPixelMap(pixelMap);
    rsImage->SetImageFit(int(fit));
    canvas.Save();
    canvas.Translate(targetRect.posX_, targetRect.posY_);
    rsImage->CanvasDrawImage(
        canvas,
        Drawing::Rect(
            targetRect.posX_,
            targetRect.posY_,
            targetRect.posX_ + targetRect.width_,
            targetRect.posY_ + targetRect.height_),
        Drawing::SamplingOptions());
    canvas.Restore();
}

bool DoDrawIllustration(const std::shared_ptr<Media::PixelMap>& pixelMap, const Rect& winRect,
    Drawing::Canvas& canvas, const float ratio, ImageFit fit)
{
    auto aboveRect = GetAboveRect(winRect, ratio);
    if (!IsValidRect(aboveRect, winRect)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "aboveRect is invalid.");
        return false;
    }
    FitAndDraw(pixelMap, aboveRect, canvas, fit);
    return true;
}

bool DoDrawAppIcon(const std::shared_ptr<Media::PixelMap>& pixelMap, const Rect& winRect,
    Drawing::Canvas& canvas, const float ratio, ImageFit fit)
{
    auto appRect = GetAppIconRect(winRect, ratio);
    if (!IsValidRect(appRect, winRect)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "appRect is invalid.");
        return false;
    }
    FitAndDraw(pixelMap, appRect, canvas, fit);
    return true;
}

bool DoDrawBranding(const std::shared_ptr<Media::PixelMap>& pixelMap, const Rect& winRect,
    Drawing::Canvas& canvas, const float ratio, ImageFit fit)
{
    if ((winRect.height_ * THIRTY_PERCENT) < MIN_RECT_HEIGHT_VP * ratio) {
        TLOGD(WmsLogTag::WMS_PATTERN, "rect is invalid.");
        return false;
    }
    auto belowRect = GetBelowRect(winRect, ratio);
    if (!IsValidRect(belowRect, winRect)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "belowRect is invalid.");
        return false;
    }
    FitAndDraw(pixelMap, belowRect, canvas, fit);
    return true;
}

bool DoDrawBackgroundImage(const std::shared_ptr<Media::PixelMap>& pixelMap, const Rect& winRect,
    Drawing::Canvas& canvas, const std::string &fit)
{
    const std::unordered_map<std::string, ImageFit> drawStrategies = {
        { "Fill", ImageFit::FILL },
        { "None", ImageFit::NONE },
        { "Cover", ImageFit::COVER },
        { "Auto", ImageFit::FIT_WIDTH },
        { "Contain", ImageFit::CONTAIN },
        { "ScaleDown", ImageFit::SCALE_DOWN },
    };
    std::string effectiveFit = fit.empty() ? "Cover" : fit;
    auto it = drawStrategies.find(effectiveFit);
    if (it == drawStrategies.end()) {
        TLOGD(WmsLogTag::WMS_PATTERN, "Unsupported fit type: %{public}s", effectiveFit.c_str());
        return false;
    }
    FitAndDraw(pixelMap, winRect, canvas, it->second);
    return true;
}

bool SurfaceDraw::DrawImage(std::shared_ptr<RSSurfaceNode> surfaceNode, int32_t bufferWidth,
    int32_t bufferHeight, const std::string& imagePath)
{
    sptr<OHOS::Surface> layer = GetLayer(surfaceNode);
    if (layer == nullptr) {
        WLOGFE("layer is nullptr");
        return false;
    }
    sptr<OHOS::SurfaceBuffer> buffer = GetSurfaceBuffer(layer, bufferWidth, bufferHeight);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        return false;
    }
    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    if (!DoDraw(addr, buffer->GetWidth(), buffer->GetHeight(), imagePath)) {
        WLOGE("draw window pixel failed");
        return false;
    }
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .w = buffer->GetWidth(),
            .h = buffer->GetHeight(),
        },
    };
    OHOS::SurfaceError ret = layer->FlushBuffer(buffer, -1, flushConfig);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        WLOGFE("draw pointer FlushBuffer ret:%{public}s", SurfaceErrorStr(ret).c_str());
        return false;
    }
    return true;
}

bool SurfaceDraw::DrawImage(std::shared_ptr<RSSurfaceNode> surfaceNode, int32_t bufferWidth,
    int32_t bufferHeight, std::shared_ptr<Media::PixelMap> pixelMap)
{
    sptr<OHOS::Surface> layer = GetLayer(surfaceNode);
    if (layer == nullptr) {
        WLOGFE("layer is nullptr");
        return false;
    }
    sptr<OHOS::SurfaceBuffer> buffer = GetSurfaceBuffer(layer, bufferWidth, bufferHeight);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        return false;
    }
    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    if (!DoDraw(addr, buffer->GetWidth(), buffer->GetHeight(), pixelMap)) {
        WLOGE("draw window pixel failed");
        return false;
    }
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .w = buffer->GetWidth(),
            .h = buffer->GetHeight(),
        },
    };
    OHOS::SurfaceError ret = layer->FlushBuffer(buffer, -1, flushConfig);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        WLOGFE("draw pointer FlushBuffer ret:%{public}s", SurfaceErrorStr(ret).c_str());
        return false;
    }
    return true;
}

bool SurfaceDraw::DrawColor(std::shared_ptr<RSSurfaceNode> surfaceNode, int32_t bufferWidth,
    int32_t bufferHeight, uint32_t color)
{
    sptr<OHOS::Surface> layer = GetLayer(surfaceNode);
    if (layer == nullptr) {
        WLOGFE("layer is nullptr");
        return false;
    }
    sptr<OHOS::SurfaceBuffer> buffer = GetSurfaceBuffer(layer, bufferWidth, bufferHeight);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        return false;
    }
    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    if (!DoDraw(addr, buffer->GetWidth(), buffer->GetHeight(), color)) {
        WLOGE("draw window color failed");
        return false;
    }
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .w = buffer->GetWidth(),
            .h = buffer->GetHeight(),
        },
    };
    OHOS::SurfaceError ret = layer->FlushBuffer(buffer, -1, flushConfig);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        WLOGFE("draw pointer FlushBuffer ret:%{public}s", SurfaceErrorStr(ret).c_str());
        return false;
    }
    return true;
}

sptr<OHOS::Surface> SurfaceDraw::GetLayer(std::shared_ptr<RSSurfaceNode> surfaceNode)
{
    if (surfaceNode == nullptr) {
        return nullptr;
    }
    return surfaceNode->GetSurface();
}

sptr<OHOS::SurfaceBuffer> SurfaceDraw::GetSurfaceBuffer(sptr<OHOS::Surface> layer,
    int32_t bufferWidth, int32_t bufferHeight)
{
    sptr<OHOS::SurfaceBuffer> buffer;
    int32_t releaseFence = 0;
    OHOS::BufferRequestConfig config = {
        .width = bufferWidth,
        .height = bufferHeight,
        .strideAlignment = 0x8,
        .format = GRAPHIC_PIXEL_FMT_RGBA_8888,
        .usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA,
    };

    OHOS::SurfaceError ret = layer->RequestBuffer(buffer, releaseFence, config);
    if (ret != OHOS::SURFACE_ERROR_OK) {
        WLOGFE("request buffer ret:%{public}s", SurfaceErrorStr(ret).c_str());
        return nullptr;
    }
    return buffer;
}

std::unique_ptr<OHOS::Media::PixelMap> SurfaceDraw::DecodeImageToPixelMap(const std::string& imagePath)
{
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/png";
    uint32_t ret = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(imagePath, opts, ret);
    if (imageSource == nullptr) {
        WLOGFE("invalid image path.");
        return nullptr;
    }
    std::set<std::string> formats;
    ret = imageSource->GetSupportedFormats(formats);
    WLOGFD("get supported format ret:%{public}u", ret);

    OHOS::Media::DecodeOptions decodeOpts;
    std::unique_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, ret);
    if (pixelMap == nullptr) {
        WLOGFE("pixelMap is nullptr");
    }
    return pixelMap;
}

void SurfaceDraw::DrawPixelmap(Drawing::Canvas& canvas, const std::string& imagePath)
{
    std::unique_ptr<OHOS::Media::PixelMap> pixelmap = DecodeImageToPixelMap(imagePath);
    if (pixelmap == nullptr) {
        WLOGFE("drawing pixel map is nullptr");
        return;
    }
    Drawing::Pen pen;
    pen.SetAntiAlias(true);
    pen.SetColor(Drawing::Color::COLOR_BLUE);
    Drawing::scalar penWidth = 1;
    pen.SetWidth(penWidth);
    canvas.AttachPen(pen);
    RSPixelMapUtil::DrawPixelMap(canvas, *pixelmap, 0, 0);
}

bool SurfaceDraw::DoDraw(uint8_t* addr, uint32_t width, uint32_t height, const std::string& imagePath)
{
    Drawing::Bitmap bitmap;
    Drawing::BitmapFormat format { Drawing::ColorType::COLORTYPE_RGBA_8888,
        Drawing::AlphaType::ALPHATYPE_OPAQUE };
    bitmap.Build(width, height, format);
    Drawing::Canvas canvas;
    canvas.Bind(bitmap);
    canvas.Clear(Drawing::Color::COLOR_TRANSPARENT);
    DrawPixelmap(canvas, imagePath);
    uint32_t addrSize = width * height * IMAGE_BYTES_STRIDE;
    errno_t ret = memcpy_s(addr, addrSize, bitmap.GetPixels(), addrSize);
    if (ret != EOK) {
        WLOGFE("draw failed");
        return false;
    }
    return true;
}

bool SurfaceDraw::DoDraw(uint8_t* addr, uint32_t width, uint32_t height, std::shared_ptr<Media::PixelMap> pixelMap)
{
    Drawing::Bitmap bitmap;
    Drawing::Canvas canvas;
    Drawing::BitmapFormat format { Drawing::ColorType::COLORTYPE_RGBA_8888, Drawing::AlphaType::ALPHATYPE_OPAQUE };
    bitmap.Build(width, height, format);
    canvas.Bind(bitmap);
    canvas.Clear(Drawing::Color::COLOR_TRANSPARENT);

    Drawing::Image image;
    Drawing::Bitmap imageBitmap;
    Drawing::SamplingOptions sampling = Drawing::SamplingOptions(Drawing::FilterMode::NEAREST,
        Drawing::MipmapMode::NEAREST);
    imageBitmap.Build(pixelMap->GetWidth(), pixelMap->GetHeight(), format);
    imageBitmap.SetPixels(const_cast<uint8_t*>(pixelMap->GetPixels()));
    image.BuildFromBitmap(imageBitmap);

    Drawing::Rect dst(0, 0, width, height);
    Drawing::Rect src(0, 0, pixelMap->GetWidth(), pixelMap->GetHeight());
    canvas.DrawImageRect(image, src, dst, sampling);

    uint32_t addrSize = width * height * IMAGE_BYTES_STRIDE;
    errno_t ret = memcpy_s(addr, addrSize, bitmap.GetPixels(), addrSize);
    if (ret != EOK) {
        WLOGFE("draw failed");
        return false;
    }
    return true;
}

bool SurfaceDraw::DoDraw(uint8_t* addr, uint32_t width, uint32_t height, uint32_t color)
{
    Drawing::Bitmap bitmap;
    Drawing::BitmapFormat format { Drawing::ColorType::COLORTYPE_RGBA_8888,
        Drawing::AlphaType::ALPHATYPE_OPAQUE };
    bitmap.Build(width, height, format);
    Drawing::Canvas canvas;
    canvas.Bind(bitmap);
    canvas.Clear(color);

    uint32_t addrSize = width * height * IMAGE_BYTES_STRIDE;
    errno_t ret = memcpy_s(addr, addrSize, bitmap.GetPixels(), addrSize);
    if (ret != EOK) {
        WLOGFE("draw failed");
        return false;
    }
    return true;
}

bool SurfaceDraw::DrawImageRect(std::shared_ptr<RSSurfaceNode> surfaceNode, Rect rect,
    std::shared_ptr<Media::PixelMap> pixelMap, uint32_t color, bool fillWindow)
{
    int32_t winHeight = static_cast<int32_t>(rect.height_);
    int32_t winWidth = static_cast<int32_t>(rect.width_);
    sptr<OHOS::Surface> layer = GetLayer(surfaceNode);
    if (layer == nullptr) {
        WLOGFE("layer is nullptr");
        return false;
    }
    sptr<OHOS::SurfaceBuffer> buffer = GetSurfaceBuffer(layer, winWidth, winHeight);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        return false;
    }
    if (!DoDrawImageRect(buffer, rect, pixelMap, color, fillWindow)) {
        WLOGE("draw image rect failed.");
        return false;
    }
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .w = buffer->GetWidth(),
            .h = buffer->GetHeight(),
        },
    };
    OHOS::SurfaceError surfaceRet = layer->FlushBuffer(buffer, -1, flushConfig);
    if (surfaceRet != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        WLOGFE("draw pointer FlushBuffer ret:%{public}s", SurfaceErrorStr(surfaceRet).c_str());
        return false;
    }
    return true;
}

bool SurfaceDraw::DoDrawImageRect(sptr<OHOS::SurfaceBuffer> buffer, const Rect& rect,
    std::shared_ptr<Media::PixelMap> pixelMap, uint32_t color, bool fillWindow)
{
    int32_t winWidth = static_cast<int32_t>(rect.width_);
    int32_t winHeight = static_cast<int32_t>(rect.height_);
    // actual width of the surface buffer after alignment
    auto bufferStride = buffer->GetStride();
    int32_t alignWidth = bufferStride / static_cast<int32_t>(IMAGE_BYTES_STRIDE);
    if (pixelMap == nullptr) {
        WLOGFE("drawing pixel map failed, because pixel map is nullptr.");
        return false;
    }
    if (pixelMap->GetHeight() <= 0 || pixelMap->GetWidth() <= 0 || winWidth <= 0 || winHeight <= 0) {
        WLOGFE("drawing pixel map failed, because width or height is invalid.");
        return false;
    }
    Drawing::Bitmap bitmap;
    Drawing::BitmapFormat format { Drawing::ColorType::COLORTYPE_RGBA_8888,
        Drawing::AlphaType::ALPHATYPE_OPAQUE };
    bitmap.Build(alignWidth, winHeight, format);
    Drawing::Canvas canvas;
    canvas.Bind(bitmap);
    canvas.Clear(color);
    float xAxis = static_cast<float>(winWidth) / pixelMap->GetWidth();
    float yAxis = static_cast<float>(winHeight) / pixelMap->GetHeight();
    float axis = std::min(xAxis, yAxis);
    int scaledPixelMapW = pixelMap->GetWidth();
    int scaledPixelMapH = pixelMap->GetHeight();
    if (axis < 1.0) {
        canvas.Scale(axis, axis);
        scaledPixelMapW = scaledPixelMapW * axis;
        scaledPixelMapH = scaledPixelMapH * axis;
    } else if (fillWindow) {
        // scale snapshot to whole window
        canvas.Scale(xAxis, yAxis);
        scaledPixelMapW = winWidth;
        scaledPixelMapH = winHeight;
    }
    int left = (winWidth - scaledPixelMapW) / 2; // 2 is the left and right boundaries of the win
    int top = (winHeight - scaledPixelMapH) / 2; // 2 is the top and bottom boundaries of the win
    WLOGFD("pixelMap width: %{public}d win height: %{public}d left:%{public}d top:%{public}d.",
        pixelMap->GetWidth(), pixelMap->GetHeight(), left, top);
    RSPixelMapUtil::DrawPixelMap(canvas, *pixelMap, left, top);
    // bufferSize is actual size of the surface buffer after alignment
    int32_t bufferSize = bufferStride * winHeight;
    uint8_t* bitmapAddr = static_cast<uint8_t*>(bitmap.GetPixels());
    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    errno_t ret = memcpy_s(addr, bufferSize, bitmapAddr, bufferSize);
    if (ret != EOK) {
        WLOGFE("draw image rect failed, because copy bitmap to buffer failed.");
        return false;
    }
    return true;
}

bool SurfaceDraw::GetSurfaceSnapshot(const std::shared_ptr<RSSurfaceNode> surfaceNode,
    std::shared_ptr<Media::PixelMap>&pixelMap, int32_t timeoutMs, float scaleW, float scaleH)
{
    if (surfaceNode == nullptr) {
        WLOGFE("surfaceNode or rsUIContext is null");
        return false;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SurfaceDraw:GetSurfaceSnapshot(%llu)", surfaceNode->GetId());
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleW,
        .scaleY = scaleH,
    };
    auto rsUICtx = surfaceNode->GetRSUIContext();
    if (rsUICtx == nullptr || rsUICtx->GetRSRenderInterface() == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "rsUIContext is null");
        return false;
    }
    if (rsUICtx->GetRSRenderInterface()->TakeSurfaceCapture(surfaceNode, callback, config)) {
        pixelMap = callback->GetResult(timeoutMs); // get pixelmap time out ms
    }
    if (pixelMap == nullptr) {
        WLOGE("get surface snapshot timeout.");
        return false;
    }
    return true;
}

bool SurfaceDraw::DrawMasking(std::shared_ptr<RSSurfaceNode> surfaceNode, Rect screenRect,
    Rect transparentRect)
{
    int32_t screenHeight = static_cast<int32_t>(screenRect.height_);
    int32_t screenWidth = static_cast<int32_t>(screenRect.width_);
    int32_t transparentHeight = static_cast<int32_t>(transparentRect.height_);
    int32_t transparentWidth = static_cast<int32_t>(transparentRect.width_);
    sptr<OHOS::Surface> layer = GetLayer(surfaceNode);
    if (layer == nullptr) {
        WLOGFE("layer is nullptr");
        return false;
    }
    sptr<OHOS::SurfaceBuffer> buffer = GetSurfaceBuffer(layer, screenWidth, screenHeight);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        return false;
    }
    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    Drawing::Bitmap fullbitmap;
    Drawing::BitmapFormat fullBitmapFormat { Drawing::ColorType::COLORTYPE_RGBA_8888,
        Drawing::AlphaType::ALPHATYPE_OPAQUE };
    fullbitmap.Build(screenWidth, screenHeight, fullBitmapFormat);
    Drawing::Canvas canvas;
    canvas.Bind(fullbitmap);
    canvas.Clear(0xFF000000);
    Drawing::Bitmap transBitmap;
    Drawing::BitmapFormat transBitmapFormat { Drawing::ColorType::COLORTYPE_RGBA_8888,
        Drawing::AlphaType::ALPHATYPE_OPAQUE };
    transBitmap.Build(transparentWidth, transparentHeight, transBitmapFormat);
    transBitmap.ClearWithColor(0);
    canvas.DrawBitmap(transBitmap, static_cast<Drawing::scalar>(transparentRect.posX_),
        static_cast<Drawing::scalar>(transparentRect.posY_));

    uint32_t addrSize = static_cast<uint32_t>(screenWidth * screenHeight * IMAGE_BYTES_STRIDE);
    errno_t ret = memcpy_s(addr, addrSize, fullbitmap.GetPixels(), addrSize);
    if (ret != EOK) {
        WLOGFE("draw failed");
        return false;
    }
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .w = buffer->GetWidth(),
            .h = buffer->GetHeight(),
        },
    };
    OHOS::SurfaceError surfaceRet = layer->FlushBuffer(buffer, -1, flushConfig);
    if (surfaceRet != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        WLOGFE("draw masking FlushBuffer ret:%{public}s", SurfaceErrorStr(surfaceRet).c_str());
        return false;
    }
    return true;
}

bool SurfaceDraw::DrawCustomStartingWindow(const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    const Rect& rect, const std::shared_ptr<Rosen::StartingWindowPageDrawInfo>& info,
    float ratio, const std::array<uint32_t, size_t(StartWindowResType::Count)>& frameIndex)
{
    int32_t winHeight = static_cast<int32_t>(rect.height_);
    int32_t winWidth = static_cast<int32_t>(rect.width_);
    sptr<OHOS::Surface> layer = GetLayer(surfaceNode);
    if (layer == nullptr) {
        TLOGD(WmsLogTag::WMS_PATTERN, "layer is nullptr");
        return false;
    }
    sptr<OHOS::SurfaceBuffer> buffer = GetSurfaceBuffer(layer, winWidth, winHeight);
    if (buffer == nullptr || buffer->GetVirAddr() == nullptr) {
        TLOGD(WmsLogTag::WMS_PATTERN, "buffer or virAddr is nullptr");
        return false;
    }
    if (!DoDrawCustomStartingWindow(buffer, rect, info, ratio, frameIndex)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "draw Custom startingwindow page failed");
        return false;
    }
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .w = buffer->GetWidth(),
            .h = buffer->GetHeight(),
        },
    };
    OHOS::SurfaceError surfaceRet = layer->FlushBuffer(buffer, -1, flushConfig);
    if (surfaceRet != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        TLOGD(WmsLogTag::WMS_PATTERN, "draw pointer FlushBuffer ret:%{public}s", SurfaceErrorStr(surfaceRet).c_str());
        return false;
    }
    return true;
}

bool SurfaceDraw::DoDrawCustomStartingWindow(const sptr<OHOS::SurfaceBuffer>& buffer, const Rect& rect,
    const std::shared_ptr<Rosen::StartingWindowPageDrawInfo>& info, const float ratio,
    const std::array<uint32_t, size_t(StartWindowResType::Count)>& frameIndex)
{
    if (info == nullptr || rect.width_ <= 0 || rect.height_ <= 0 || ratio  <= 0) {
        TLOGD(WmsLogTag::WMS_PATTERN, "info is nullptr or invalid rect and ratio");
        return false;
    }
    auto bufferStride = buffer->GetStride();
    int32_t alignWidth = bufferStride / static_cast<int32_t>(IMAGE_BYTES_STRIDE);
    Drawing::Bitmap customDrawBitmap;
    Drawing::BitmapFormat format { Drawing::ColorType::COLORTYPE_RGBA_8888, Drawing::AlphaType::ALPHATYPE_OPAQUE };
    customDrawBitmap.Build(alignWidth, rect.height_, format);
    Drawing::Canvas canvas;
    canvas.Bind(customDrawBitmap);
    canvas.Clear(info->bgColor);
    TryDrawResource(info->bgImage, frameIndex[(size_t)StartWindowResType::BgImage],
        [&](const auto& map) { return DoDrawBackgroundImage(map, rect, canvas, info->startWindowBackgroundImageFit); },
        "background image");

    TryDrawResource(info->branding, frameIndex[(size_t)StartWindowResType::Branding],
        [&](const auto& map) { return DoDrawBranding(map, rect, canvas, ratio, ImageFit::SCALE_DOWN); },
        "branding image");

    TryDrawResource(info->illustration, frameIndex[(size_t)StartWindowResType::Illustration],
        [&](const auto& map) { return DoDrawIllustration(map, rect, canvas, ratio, ImageFit::SCALE_DOWN); },
        "illustration image");

    TryDrawResource(info->appIcon, frameIndex[(size_t)StartWindowResType::AppIcon],
        [&](const auto& map) { return DoDrawAppIcon(map, rect, canvas, ratio, ImageFit::CONTAIN); },
        "app icon image");
    int32_t bufferSize = bufferStride * rect.height_;
    uint8_t* bitmapAddr = static_cast<uint8_t*>(customDrawBitmap.GetPixels());
    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    errno_t ret = memcpy_s(addr, bufferSize, bitmapAddr, bufferSize);
    if (ret != EOK) {
        TLOGD(WmsLogTag::WMS_PATTERN, "draw image failed, because copy customDrawBitmap to buffer failed.");
        return false;
    }
    return true;
}
} // Rosen
} // OHOS