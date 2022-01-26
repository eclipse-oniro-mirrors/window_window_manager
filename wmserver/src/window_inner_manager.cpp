/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "window_inner_manager.h"

#include "include/core/SkCanvas.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "transaction/rs_transaction.h"
#include "ui/rs_surface_extractor.h"

#include "window_life_cycle_interface.h"
#include "window_manager_hilog.h"
#include "window_option.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowInnerManager"};
}

WM_IMPLEMENT_SINGLE_INSTANCE(WindowInnerManager)

bool WindowInnerManager::DecodeImageFile(const char* filename, SkBitmap& bitmap)
{
    sk_sp<SkData> data(SkData::MakeFromFileName(filename));
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(data));
    if (codec == nullptr) {
        return false;
    }
    SkColorType colorType = kN32_SkColorType;
    SkImageInfo info = codec->getInfo().makeColorType(colorType);
    if (!bitmap.tryAllocPixels(info)) {
        return false;
    }
    return SkCodec::kSuccess == codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());
}

void WindowInnerManager::DrawSurface(const sptr<Window>& window)
{
    auto surfaceNode = window->GetSurfaceNode();
    auto winRect = window->GetRect();
    WLOGFI("SurfaceWindowDraw winRect, x : %{public}d, y : %{public}d, width: %{public}d, height: %{public}d",
        winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);

    auto width = winRect.width_;
    auto height = winRect.height_;

    std::shared_ptr<RSSurface> rsSurface = RSSurfaceExtractor::ExtractRSSurface(surfaceNode);
    if (rsSurface == nullptr) {
        WLOGFE("RSSurface is nullptr");
        return;
    }
#ifdef ACE_ENABLE_GL
    rsSurface->SetRenderContext(rc_);
#endif
    auto frame = rsSurface->RequestFrame(width, height);
    if (frame == nullptr) {
        WLOGFE("DrawSurface frameptr is nullptr");
        return;
    }
    auto canvas = frame->GetCanvas();
    if (canvas == nullptr) {
        WLOGFE("DrawSurface canvas is nullptr");
        return;
    }
    if (!dividerBitmap_.isNull() && dividerBitmap_.width() != 0 && dividerBitmap_.height() != 0) {
        SkPaint paint;
        SkMatrix matrix;
        SkRect paintRect;
        if (width < height) {
            // rotate when devider is hozizon
            matrix.setScale(static_cast<float>(height) / dividerBitmap_.width(),
                static_cast<float>(width) / dividerBitmap_.height());
            matrix.postRotate(-90.0f); // devider shader rotate -90.0
        } else {
            matrix.setScale(static_cast<float>(width) / dividerBitmap_.width(),
                static_cast<float>(height) / dividerBitmap_.height());
        }
        paint.setShader(dividerBitmap_.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
        if (paint.getShader() != nullptr) {
            paint.setShader(paint.getShader()->makeWithLocalMatrix(matrix));
            paintRect.set(0, 0, static_cast<int>(width), static_cast<int>(height));
            canvas->drawRect(paintRect, paint);
        } else {
            WLOGFE("Paint's shader is nullptr");
        }
    }
    frame->SetDamageRegion(0, 0, width, height);
    rsSurface->FlushFrame(frame);
}


sptr<Window> WindowInnerManager::GetDividerWindow(uint32_t displayId) const
{
    auto iter = dividerMap_.find(displayId);
    if (iter == dividerMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

sptr<Window> WindowInnerManager::CreateWindow(uint32_t displayId, const WindowType& type, const Rect& rect)
{
    sptr<Window> window = GetDividerWindow(displayId);
    if (window == nullptr) {
        sptr<WindowOption> divWindowOp = new WindowOption();
        divWindowOp->SetWindowRect(rect);
        divWindowOp->SetWindowType(type);
        divWindowOp->SetFocusable(false);
        divWindowOp->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        window = Window::Create("divider" + std::to_string(displayId), divWindowOp);
        if (window == nullptr) {
            WLOGFE("Window is nullptr");
            return nullptr;
        }
        dividerMap_.insert(std::make_pair(displayId, window));
        WLOGFI("CreateWindow success");
    }
    return window;
}

void WindowInnerManager::CreateAndShowDivider(std::unique_ptr<WindowMessage> msg)
{
    auto window = CreateWindow(msg->displayId, WindowType::WINDOW_TYPE_DOCK_SLICE, msg->dividerRect);
    if (window == nullptr) {
        return;
    }

    WMError res = window->Show();
    if (res != WMError::WM_OK) {
        WLOGFE("Show window failed");
        return;
    }

#ifdef ACE_ENABLE_GL
    WLOGFI("Draw divider on gpu");
    // init render context
    static bool hasInitRC = false;
    if (!hasInitRC) {
        rc_ = RenderContextFactory::GetInstance().CreateEngine();
        if (rc_) {
            rc_->InitializeEglContext();
            hasInitRC = true;
        } else {
            WLOGFE("InitilizeEglContext failed");
            return;
        }
    }
#endif
    if (!isDividerImageLoaded_) {
        isDividerImageLoaded_ = DecodeImageFile(splitIconPath_, dividerBitmap_);
        if (!isDividerImageLoaded_) {
            WLOGFE("Decode devider image failed");
        }
    }
    DrawSurface(window);
    WLOGFI("CreateAndShowDivider success");
}

void WindowInnerManager::HideAndDestroyDivider(std::unique_ptr<WindowMessage> msg)
{
    sptr<Window> window = GetDividerWindow(msg->displayId);
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return;
    }
    WMError res = window->Destroy();
    if (res != WMError::WM_OK) {
        WLOGFE("Destroy window failed");
        return;
    }
    dividerMap_.erase(msg->displayId);
    WLOGFI("HideAndDestroyDivider success");
}

void WindowInnerManager::DestroyThread(std::unique_ptr<WindowMessage> msg)
{
    hasInitThread_ = false;
    needDestroyThread_ = true;
    isDividerImageLoaded_ = false;
    WLOGFI("DestroyThread success");
}

void WindowInnerManager::HandleMessage()
{
    WLOGFI("HandleMessage");
    std::vector<std::unique_ptr<WindowMessage>> handleMsgs;
    while (!needDestroyThread_) {
        // lock to store massages
        {
            std::unique_lock<std::mutex> lk(mutex_);
            conVar_.wait(lk, [this] { return ready_; });
            for (auto& iter: messages_) {
                handleMsgs.push_back(std::move(iter));
            }
            messages_.clear();
            ready_ = false;
        }
        // loop to handle massages
        for (auto& msg : handleMsgs) {
            auto cmdType = msg->cmdType;
            using Func_t = void(WindowInnerManager::*)(std::unique_ptr<WindowMessage> winMsg);
            static const std::map<InnerWMCmd, Func_t> funcMap = {
                std::make_pair(INNER_WM_CREATE_DIVIDER,  &WindowInnerManager::CreateAndShowDivider),
                std::make_pair(INNER_WM_DESTROY_DIVIDER, &WindowInnerManager::HideAndDestroyDivider),
                std::make_pair(INNER_WM_DESTROY_THREAD,  &WindowInnerManager::DestroyThread)};
            auto it = funcMap.find(cmdType);
            if (it != funcMap.end()) {
                (this->*(it->second))(std::move(msg));
            }
        }
        handleMsgs.clear();
    }
}

void WindowInnerManager::SendMessage(InnerWMCmd cmdType, uint32_t displayId)
{
    std::unique_lock<std::mutex> lk(mutex_);
    if (!hasInitThread_) {
        WLOGFI("Inner window manager thread has not been created");
        return;
    }
    std::unique_ptr<WindowMessage> winMsg = std::make_unique<WindowMessage>();
    winMsg->cmdType = cmdType;
    winMsg->displayId = displayId;
    WLOGFI("SendMessage : displayId = %{public}d,  type = %{public}d",
        winMsg->displayId, static_cast<uint32_t>(cmdType));
    messages_.push_back(std::move(winMsg));
    ready_ = true;
    conVar_.notify_one();
}

void WindowInnerManager::SendMessage(InnerWMCmd cmdType, uint32_t displayId, const Rect& dividerRect)
{
    std::unique_lock<std::mutex> lk(mutex_);
    if (!hasInitThread_) {
        WLOGFI("Inner window manager thread has not been created");
        return;
    }
    std::unique_ptr<WindowMessage> winMsg = std::make_unique<WindowMessage>();
    winMsg->cmdType = cmdType;
    winMsg->displayId = displayId;
    winMsg->dividerRect = dividerRect;
    WLOGFI("SendMessage : displayId = %{public}d,  type = %{public}d" \
        " Rect = [%{public}d %{public}d %{public}d %{public}d]",
        winMsg->displayId, static_cast<uint32_t>(cmdType),
        winMsg->dividerRect.posX_, winMsg->dividerRect.posY_,
        winMsg->dividerRect.width_, winMsg->dividerRect.height_);
    messages_.push_back(std::move(winMsg));
    ready_ = true;
    conVar_.notify_one();
}

void WindowInnerManager::Init()
{
    std::unique_lock<std::mutex> lk(mutex_);
    needDestroyThread_ = false;
    // create inner thread
    std::thread innerWMThread(&WindowInnerManager::HandleMessage, this);
    innerWMThread.detach();
    hasInitThread_ = true;
    WLOGFI("Inner window manager thread create success");
}
}
}