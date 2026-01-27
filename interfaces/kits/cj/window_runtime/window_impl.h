/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef WINDOW_IMPL_H
#define WINDOW_IMPL_H

#include "ffi_remote_data.h"
#include "pixel_map.h"
#include "window.h"
#include "window_utils.h"
#include "window_register_manager.h"

namespace OHOS {
namespace Rosen {
struct CAvoidArea {
    bool visible;
    Rect leftRect { 0, 0, 0, 0 };
    Rect topRect { 0, 0, 0, 0 };
    Rect rightRect { 0, 0, 0, 0 };
    Rect bottomRect { 0, 0, 0, 0 };
};

struct ResWindow {
    int32_t ret;
    sptr<Window> nativeWindow;
};

struct CBarProperties {
    std::string statusBarColor;
    bool isStatusBarLightIcon;
    std::string statusBarContentColor;
    std::string navigationBarColor;
    bool isNavigationBarLightIcon;
    std::string navigationBarContentColor;
    bool enableStatusBarAnimation;
    bool enableNavigationBarAnimation;
};

struct CTitleButtonRect {
    int32_t right;
    int32_t top;
    uint32_t width;
    uint32_t height;
};

struct CWindowLimits {
    uint32_t maxWidth = INT32_MAX;
    uint32_t maxHeight = INT32_MAX;
    uint32_t minWidth = 1;
    uint32_t minHeight = 1;
};

class CJWindowImpl : public OHOS::FFI::FFIData {
public:
    explicit CJWindowImpl(sptr<Window> ptr);
    sptr<Window> GetWindowToken();
    int32_t Hide();
    int32_t HideWithAnimation();
    int32_t ShowWindow();
    int32_t ShowWithAnimation();
    int32_t DestroyWindow();
    int32_t MoveWindowTo(int32_t x, int32_t y);
    int32_t Resize(uint32_t width, uint32_t height);
    int32_t SetWindowMode(uint32_t mode);
    CWindowProperties GetWindowProperties(int32_t* errCode);
    int32_t SetWindowLayoutFullScreen(bool isLayoutFullScreen);
    int32_t SetWindowBackgroundColor(const char* color);
    int32_t SetWindowBrightness(float brightness);
    int32_t SetWindowFocusable(bool focusable);
    int32_t SetWindowKeepScreenOn(bool keepScreenOn);
    int32_t SetWindowPrivacyMode(bool isPrivacyMode);
    int32_t SetWindowTouchable(bool touchable);
    int32_t SetForbidSplitMove(bool isForbidSplitMove);
    bool IsWindowSupportWideGamut(int32_t* errCode);
    int32_t SetWindowColorSpace(uint32_t colorSpace);
    bool IsWindowShowing(int32_t* errCode);
    int32_t SetBackdropBlurStyle(uint32_t blurStyle);
    int32_t SetPreferredOrientation(uint32_t orientation);
    int32_t GetWindowAvoidArea(uint32_t areaType, CAvoidArea* retPtr);
    int32_t SetShadowRadius(double radius);
    int32_t SetShadowColor(std::string color);
    int32_t SetShadowOffsetX(double offsetX);
    int32_t SetShadowOffsetY(double offsetY);
    int32_t SetBackdropBlur(double radius);
    int32_t SetBlur(double radius);
    int32_t SetWaterMarkFlag(bool enable);
    int32_t SetAspectRatio(double ratio);
    int32_t ResetAspectRatio();
    int32_t Minimize();
    int32_t SetCornerRadius(float radius);
    int32_t SetResizeByDragEnabled(bool enable);
    int32_t RaiseToAppTop();
    int32_t SetSnapshotSkip(bool isSkip);
    int32_t SetWakeUpScreen(bool wakeUp);

    int32_t SetRaiseByClickEnabled(bool enable);
    uint32_t GetWindowColorSpace(int32_t* errCode);
    int32_t RaiseAboveTarget(int32_t windowId);
    int32_t Translate(double x, double y, double z);
    int32_t Rotate(double x, double y, double z, double pivotX, double pivotY);
    int32_t Scale(double x, double y, double pivotX, double pivotY);
    int32_t Opacity(double opacity);
    std::shared_ptr<Media::PixelMap> Snapshot(int32_t* errCode);
    int32_t SetWindowSystemBarEnable(char** arr, uint32_t size);
    int32_t OnRegisterWindowCallback(const std::string& type, int64_t funcId, int64_t parameter = 0);
    int32_t UnregisterWindowCallback(const std::string& type, int64_t funcId);
    int32_t SetWindowSystemBarProperties(const CBarProperties& cProperties);
    ResWindow CheckWindow();
    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }
    int32_t SetSubWindowModal(bool isModal);
    int32_t SetWindowLimits(const CWindowLimits& cWindowLimits, CWindowLimits& retPtr);
    int32_t GetWindowLimits(CWindowLimits& retPtr);
    bool GetImmersiveModeEnabledState(int32_t& errCode);
    int32_t SetImmersiveModeEnabledState(bool enabled);
    int32_t KeepKeyboardOnFocus(bool keepKeyboardFlag);
    int32_t GetWindowDecorHeight(int32_t& height);
    int32_t SetWindowDecorHeight(int32_t height);
    int32_t Recover();
    int32_t SetWindowDecorVisible(bool isVisible);
    int32_t GetTitleButtonRect(CTitleButtonRect& retPtr);
    int32_t SetDialogBackGestureEnabled(bool enabled);
    int32_t DisableLandscapeMultiWindow();
    int32_t EnableLandscapeMultiWindow();
    int32_t SetWindowGrayScale(float grayScale);
    uint32_t GetPreferredOrientation(int32_t& errCode);
    int32_t GetWindowStatus(int32_t& errCode);
    int32_t GetWindowSystemBarProperties(CJBarProperties& retPtr);
    int32_t SpecificSystemBarEnabled(int32_t name, bool enable, bool enableAnimation);
    int32_t Maximize(int32_t presentation);
    int32_t CreateSubWindowWithOptions(std::string name, int64_t &windowId,
                                       CSubWindowOptions option);
private:
    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType =
            OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("CJWindowImpl");
        return &runtimeType;
    }
    sptr<Window> windowToken_;
    std::unique_ptr<CjWindowRegisterManager> registerManager_ = nullptr;
};

sptr<CJWindowImpl> CreateCjWindowObject(sptr<Window>& window);
sptr<CJWindowImpl> FindCjWindowObject(const std::string& windowName);

}
}

#endif
