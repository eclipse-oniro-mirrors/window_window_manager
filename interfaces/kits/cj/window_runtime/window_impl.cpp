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

#include "window_impl.h"
#include <regex>
#include <sstream>
#include "permission.h"
#include "securec.h"
#include "ui_content.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_utils.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr Rect g_emptyRect = {0, 0, 0, 0};
}
static thread_local std::map<std::string, sptr<CJWindowImpl>> g_cjWindowMap;
std::recursive_mutex g_mutex;
constexpr int32_t MIN_DECOR_HEIGHT = 37;
constexpr int32_t MAX_DECOR_HEIGHT = 112;
constexpr int32_t MIN_GRAY_SCALE = 0.0;
constexpr int32_t MAX_GRAY_SCALE = 1.0;

const std::vector<MaximizePresentation> MAXIMIZE_PRESENTATION = {
    MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING,
    MaximizePresentation::EXIT_IMMERSIVE,
    MaximizePresentation::ENTER_IMMERSIVE};
const std::vector<SpecificSystemBar> SYSTEM_BAR = {
    SpecificSystemBar::STATUS, SpecificSystemBar::NAVIGATION,
    SpecificSystemBar::NAVIGATION_INDICATOR};
static int g_ctorCnt = 0;

sptr<CJWindowImpl> FindCjWindowObject(const std::string& windowName)
{
    TLOGD(WmsLogTag::WMS_DIALOG, "Try to find window %{public}s in g_cjWindowMap", windowName.c_str());
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    if (g_cjWindowMap.find(windowName) == g_cjWindowMap.end()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Can not find window %{public}s in g_cjWindowMap", windowName.c_str());
        return nullptr;
    }
    return g_cjWindowMap[windowName];
}

sptr<CJWindowImpl> CreateCjWindowObject(sptr<Window>& window)
{
    if (window == nullptr) {
        TLOGI(WmsLogTag::WMS_DIALOG, "Invalid input");
        return nullptr;
    }
    const std::string& windowName = window->GetWindowName();
    sptr<CJWindowImpl> windowImpl = FindCjWindowObject(windowName);
    if (windowImpl != nullptr) {
        TLOGI(WmsLogTag::WMS_DIALOG, "FindCjWindowObject %{public}s", windowName.c_str());
        return windowImpl;
    }

    windowImpl = FFI::FFIData::Create<CJWindowImpl>(window);
    if (windowImpl == nullptr) {
        TLOGI(WmsLogTag::WMS_DIALOG, "Failed to create window %{public}s", windowName.c_str());
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    g_cjWindowMap[windowName] = windowImpl;
    return windowImpl;
}

CJWindowImpl::CJWindowImpl(sptr<Window> ptr)
    : windowToken_(ptr), registerManager_(std::make_unique<CjWindowRegisterManager>())
{
    NotifyNativeWinDestroyFunc func = [](std::string windowName) {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        if (windowName.empty() || g_cjWindowMap.count(windowName) == 0) {
            TLOGE(WmsLogTag::WMS_DIALOG, "Can not find window %{public}s ", windowName.c_str());
            return;
        }
        g_cjWindowMap.erase(windowName);
        TLOGI(WmsLogTag::WMS_DIALOG, "Destroy window %{public}s in js window", windowName.c_str());
    };
    if (windowToken_ == nullptr) {
        TLOGI(WmsLogTag::WMS_DIALOG, "constructe failed");
        return;
    }
    windowToken_->RegisterWindowDestroyedListener(func);
    TLOGI(WmsLogTag::WMS_DIALOG, " constructorCnt: %{public}d", ++g_ctorCnt);
}

sptr<Window> CJWindowImpl::GetWindowToken()
{
    return windowToken_;
}

ResWindow CJWindowImpl::CheckWindow()
{
    ResWindow result;
    result.ret = static_cast<int32_t>(WmErrorCode::WM_OK);
    result.nativeWindow = nullptr;
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        result.ret = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return result;
    }
    result.nativeWindow = windowToken_;
    return result;
}

int32_t CJWindowImpl::Hide()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(result.nativeWindow->Hide(0, false, false));
    TLOGI(WmsLogTag::WMS_DIALOG,
        "Window [%{public}u] hide end, ret=%{public}d", result.nativeWindow->GetWindowId(), ret);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::HideWithAnimation()
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (windowToken_ != nullptr) {
        auto winType = windowToken_->GetType();
        if (!WindowHelper::IsSystemWindow(winType)) {
            TLOGE(WmsLogTag::WMS_DIALOG, "window Type %{public}u is not supported", static_cast<uint32_t>(winType));
            errCode = WmErrorCode::WM_ERROR_INVALID_CALLING;
        }
    } else {
        errCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (errCode != WmErrorCode::WM_OK) {
        return static_cast<int32_t>(errCode);
    }
    errCode = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->Hide(0, true, false));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), errCode);
    return static_cast<int32_t>(errCode);
}

int32_t CJWindowImpl::ShowWindow()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    if (weakWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "window is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WMError ret = weakWindow->Show(0, false);
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] show with ret=%{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(ret));
}
        
int32_t CJWindowImpl::ShowWithAnimation()
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (windowToken_ == nullptr) {
        errCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    } else {
        auto winType = windowToken_->GetType();
        if (!WindowHelper::IsSystemWindow(winType)) {
            TLOGE(WmsLogTag::WMS_DIALOG, "window Type %{public}u is not supported", static_cast<uint32_t>(winType));
            errCode = WmErrorCode::WM_ERROR_INVALID_CALLING;
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        return static_cast<int32_t>(errCode);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->Show(0, true));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::DestroyWindow()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WMError ret = weakWindow->Destroy();
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] destroy end, ret=%{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    windowToken_ = nullptr;
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::MoveWindowTo(int32_t x, int32_t y)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->MoveTo(x, y));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] move end, ret=%{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) {
        TLOGE(WmsLogTag::WMS_DIALOG, "width or height should greater than 0!");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Resize(width, height));
    TLOGD(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] resize end, ret=%{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowMode(uint32_t mode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "set window mode permission denied!");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    WindowMode winMode = CJ_TO_NATIVE_WINDOW_MODE_MAP.at(static_cast<ApiWindowMode>(mode));
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetWindowMode(winMode));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] set type end, ret=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(ret);
}

CWindowProperties CJWindowImpl::GetWindowProperties(int32_t* errCode)
{
    CWindowProperties wp;
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        *errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return wp;
    }
    Rect drawableRect = g_emptyRect;
    auto uicontent = windowToken_->GetUIContent();
    if (uicontent == nullptr) {
        TLOGW(WmsLogTag::WMS_DIALOG, "uicontent is nullptr");
    } else {
        uicontent->GetAppPaintSize(drawableRect);
    }
    wp.drawableRect.posX = drawableRect.posX_;
    wp.drawableRect.posY = drawableRect.posY_;
    wp.drawableRect.height = drawableRect.height_;
    wp.drawableRect.width = drawableRect.width_;
    Rect rect = windowToken_->GetRect();
    wp.windowRect.posX = rect.posX_;
    wp.windowRect.posY = rect.posY_;
    wp.windowRect.height = rect.height_;
    wp.windowRect.width = rect.width_;
    WindowType type = windowToken_->GetType();
    if (type == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        wp.type = static_cast<uint32_t>(type);
    } else if (CJ_TO_WINDOW_TYPE_MAP.count(type) != 0) {
        wp.type = static_cast<uint32_t>(CJ_TO_WINDOW_TYPE_MAP.at(type));
    } else {
        wp.type = static_cast<uint32_t>(type);
    }
    wp.isFullScreen = windowToken_->IsFullScreen();
    wp.isLayoutFullScreen = windowToken_->IsLayoutFullScreen();
    wp.focusable = windowToken_->GetFocusable();
    wp.touchable = windowToken_->GetTouchable();
    wp.brightness = windowToken_->GetBrightness();
    wp.isKeepScreenOn = windowToken_->IsKeepScreenOn();
    wp.isPrivacyMode = windowToken_->IsPrivacyMode();
    wp.isRoundCorner = false;
    wp.isTransparent = windowToken_->IsTransparent();
    wp.id = windowToken_->GetWindowId();
    *errCode = 0;
    return wp;
}

int32_t CJWindowImpl::SetWindowLayoutFullScreen(bool isLayoutFullScreen)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetLayoutFullScreen(isLayoutFullScreen));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowBackgroundColor(const char* color)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBackgroundColor(color));
    if (ret == WmErrorCode::WM_OK) {
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowBrightness(float brightness)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetBrightness(brightness));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetBackdropBlurStyle(uint32_t blurStyle)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (blurStyle > static_cast<uint32_t>(WindowBlurStyle::WINDOW_BLUR_THICK)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Invalid window blur style");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret =
        WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBackdropBlurStyle(static_cast<WindowBlurStyle>(blurStyle)));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    } else {
        TLOGI(WmsLogTag::WMS_DIALOG,
            "Window [%{public}u, %{public}s] end, blurStyle=%{public}u",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), blurStyle);
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetPreferredOrientation(uint32_t orientation)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto winOrientation = static_cast<Orientation>(orientation);
    if (winOrientation < Orientation::UNSPECIFIED || winOrientation > Orientation::LOCKED) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Orientation %{public}u invalid!", orientation);
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    windowToken_->SetRequestedOrientation(winOrientation);
    TLOGI(WmsLogTag::WMS_DIALOG,
        "Window [%{public}u, %{public}s] end, orientation=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        static_cast<uint32_t>(winOrientation));
    return static_cast<int32_t>(WmErrorCode::WM_OK);
}
        
int32_t CJWindowImpl::SetWindowFocusable(bool focusable)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetFocusable(focusable));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowKeepScreenOn(bool keepScreenOn)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetKeepScreenOn(keepScreenOn));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::GetWindowAvoidArea(uint32_t areaType, CAvoidArea* retPtr)
{
    if (retPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Invalid input");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    AvoidAreaType avoidAreaType = static_cast<AvoidAreaType>(areaType);
    if (avoidAreaType >= AvoidAreaType::TYPE_END) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    retPtr->visible = avoidAreaType == AvoidAreaType::TYPE_CUTOUT ? false : true;
    if (ret != WmErrorCode::WM_OK) {
        retPtr->topRect = g_emptyRect;
        retPtr->leftRect = g_emptyRect;
        retPtr->rightRect = g_emptyRect;
        retPtr->bottomRect = g_emptyRect;
    } else {
        retPtr->topRect = avoidArea.topRect_;
        retPtr->leftRect = avoidArea.leftRect_;
        retPtr->rightRect = avoidArea.rightRect_;
        retPtr->bottomRect = avoidArea.bottomRect_;
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowPrivacyMode(bool isPrivacyMode)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetPrivacyMode(isPrivacyMode));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, mode=%{public}u",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), isPrivacyMode);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowTouchable(bool touchable)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetTouchable(touchable));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] set touchable end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetForbidSplitMove(bool isForbidSplitMove)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret;
    if (isForbidSplitMove) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            weakWindow->AddWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            weakWindow->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE));
    }
    return static_cast<int32_t>(ret);
}

bool CJWindowImpl::IsWindowSupportWideGamut(int32_t* errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        *errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    bool flag = windowToken_->IsSupportWideGamut();
    *errCode = 0;
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), flag);
    return flag;
}

bool CJWindowImpl::IsWindowShowing(int32_t* errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        *errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }
    bool state = (windowToken_->GetWindowState() == WindowState::STATE_SHOWN);
    *errCode = 0;
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] get show state end, state=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), state);
    return state;
}

int32_t CJWindowImpl::SetWaterMarkFlag(bool enable)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WMError ret;
    if (enable) {
        ret = windowToken_->AddWindowFlag(WindowFlag::WINDOW_FLAG_WATER_MARK);
    } else {
        ret = windowToken_->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_WATER_MARK);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    } else {
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}d",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetShadowRadius(double radius)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "invalid radius");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowRadius(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetShadowColor(std::string color)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowColor(color));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetShadowOffsetX(double offsetX)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowOffsetX(offsetX));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetShadowOffsetY(double offsetY)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowOffsetY(offsetY));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetBackdropBlur(double radius)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "invalid radius");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBackdropBlur(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    } else {
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, radius=%{public}f",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetBlur(double radius)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "invalid radius");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBlur(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    } else {
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, radius=%{public}f",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetAspectRatio(double ratio)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not main window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (ratio <= 0.0) {
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WMError ret = windowToken_->SetAspectRatio(ratio);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    } else {
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}d",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::ResetAspectRatio()
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not main window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    WMError ret = windowToken_->ResetAspectRatio();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "failed");
    } else {
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] reset aspect ratio end, ret=%{public}d",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowColorSpace(uint32_t colorSpace)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    weakWindow->SetColorSpace(ColorSpace(colorSpace));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, colorSpace=%{public}u",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));
    return 0;
}

int32_t CJWindowImpl::Minimize()
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "subWindow hide");
        return Hide();
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->Minimize());
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, ret=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), ret);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetCornerRadius(float radius)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "set corner radius permission denied!");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "invalid radius");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetCornerRadius(radius));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, radius=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetResizeByDragEnabled(bool enable)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetResizeByDragEnabled(enable));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

/** @note @window.hierarchy */
int32_t CJWindowImpl::RaiseToAppTop()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->RaiseToAppTop());
    TLOGI(WmsLogTag::WMS_HIERARCHY, "Window [%{public}u, %{public}s] zorder raise success",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetSnapshotSkip(bool isSkip)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSnapshotSkip(isSkip));
    TLOGI(WmsLogTag::WMS_DIALOG, "[%{public}u, %{public}s] end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWakeUpScreen(bool wakeUp)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "permission denied!");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    windowToken_->SetTurnScreenOn(wakeUp);
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] screen %{public}d end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), wakeUp);
    return 0;
}

uint32_t CJWindowImpl::GetWindowColorSpace(int32_t* errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        *errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return 0;
    }
    ColorSpace colorSpace = windowToken_->GetColorSpace();
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, colorSpace=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));
    *errCode = 0;
    return static_cast<uint32_t>(colorSpace);
}

/** @note @window.hierarchy */
int32_t CJWindowImpl::SetRaiseByClickEnabled(bool enable)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetRaiseByClickEnabled(enable));
    TLOGI(WmsLogTag::WMS_HIERARCHY, "Window [%{public}u, %{public}s] end",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

/** @note @window.hierarchy */
int32_t CJWindowImpl::RaiseAboveTarget(int32_t windowId)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->RaiseAboveTarget(windowId));
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::Translate(double x, double y, double z)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    auto trans = windowToken_->GetTransform();
    trans.translateX_ = x;
    trans.translateY_ = y;
    trans.translateZ_ = z;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTransform(trans));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end, "
        "translateX=%{public}f, translateY=%{public}f, translateZ=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        trans.translateX_, trans.translateY_, trans.translateZ_);
    return static_cast<int32_t>(ret);
}

static bool IsPivotValid(double data)
{
    if (MathHelper::LessNotEqual(data, 0.0) || (MathHelper::GreatNotEqual(data, 1.0))) {
        return false;
    }
    return true;
}

int32_t CJWindowImpl::Rotate(double x, double y, double z, double pivotX, double pivotY)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (!IsPivotValid(pivotX) || !IsPivotValid(pivotY)) {
        TLOGE(WmsLogTag::WMS_DIALOG, " PivotX or PivotY should between 0.0 ~ 1.0");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    auto trans = windowToken_->GetTransform();
    trans.translateX_ = x;
    trans.translateY_ = y;
    trans.translateZ_ = z;
    trans.pivotX_ = pivotX;
    trans.pivotY_ = pivotY;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTransform(trans));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

static bool IsScaleValid(double data)
{
    if (!MathHelper::GreatNotEqual(data, 0.0)) {
        return false;
    }
    return true;
}

int32_t CJWindowImpl::Scale(double x, double y, double pivotX, double pivotY)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (!IsPivotValid(pivotX) || !IsPivotValid(pivotY) || !IsScaleValid(x) || !IsScaleValid(y)) {
        TLOGE(WmsLogTag::WMS_DIALOG, " PivotX or PivotY should between 0.0 ~ 1.0, scale should greater than 0.0");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    auto trans = windowToken_->GetTransform();
    trans.pivotX_ = pivotX;
    trans.pivotY_ = pivotY;
    trans.scaleX_ = x;
    trans.scaleY_ = y;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTransform(trans));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::Opacity(double opacity)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "not allowed since window is not system window");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (!IsPivotValid(opacity)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "opacity should greater than 0 or smaller than 1.0");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetAlpha(opacity));
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] Opacity end, alpha=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), opacity);
    return static_cast<int32_t>(ret);
}

std::shared_ptr<Media::PixelMap> CJWindowImpl::Snapshot(int32_t* errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        *errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = windowToken_->Snapshot();
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "get pixelmap is null");
        *errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return nullptr;
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] OnSnapshot, WxH=%{public}dx%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        pixelMap->GetWidth(), pixelMap->GetHeight());
    *errCode = 0;
    return pixelMap;
}

static uint32_t GetColorFromJs(const std::string& colorStr, uint32_t defaultColor, bool& flag)
{
    std::regex pattern("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{8})$");
    if (!std::regex_match(colorStr, pattern)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Invalid color input");
        return defaultColor;
    }
    std::string color = colorStr.substr(1);
    if (color.length() == RGB_LENGTH) {
        color = "FF" + color; // ARGB
    }
    flag = true;
    std::stringstream ss;
    uint32_t hexColor;
    ss << std::hex << color;
    ss >> hexColor;
    TLOGI(WmsLogTag::WMS_DIALOG, "Origin %{public}s, process %{public}s, final %{public}x",
        colorStr.c_str(), color.c_str(), hexColor);
    return hexColor;
}

static void UpdateSystemBarProperties(std::map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags, sptr<Window> weakToken)
{
    for (auto it : systemBarPropertyFlags) {
        WindowType type = it.first;
        SystemBarPropertyFlag flag = it.second;
        auto property = weakToken->GetSystemBarPropertyByType(type);
        if (flag.enableFlag == false) {
            systemBarProperties[type].enable_ = property.enable_;
        } else {
            systemBarProperties[type].settingFlag_ = static_cast<SystemBarSettingFlag>(
                static_cast<uint32_t>(systemBarProperties[type].settingFlag_) |
                static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING));
        }
        if (flag.backgroundColorFlag == false) {
            systemBarProperties[type].backgroundColor_ = property.backgroundColor_;
        }
        if (flag.contentColorFlag == false) {
            systemBarProperties[type].contentColor_ = property.contentColor_;
        }
        if (flag.backgroundColorFlag || flag.contentColorFlag) {
            systemBarProperties[type].settingFlag_ = static_cast<SystemBarSettingFlag>(
                static_cast<uint32_t>(systemBarProperties[type].settingFlag_) |
                static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
        }
    }
}

void SetBarPropertyMap(
    std::map<WindowType, SystemBarProperty>& properties,
    std::map<WindowType, SystemBarPropertyFlag>& propertyFlags,
    const CBarProperties& cProperties,
    sptr<Window> nativeWindow)
{
    auto statusProperty = nativeWindow->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto navProperty = nativeWindow->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    properties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
    properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
    propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = SystemBarPropertyFlag();
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_ = GetColorFromJs(cProperties.statusBarColor,
        statusProperty.backgroundColor_, propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColorFlag);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_ =
        GetColorFromJs(cProperties.navigationBarColor, navProperty.backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColorFlag);
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ = cProperties.enableStatusBarAnimation;
    properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ = cProperties.enableNavigationBarAnimation;
    if (!cProperties.statusBarContentColor.empty()) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ =
            GetColorFromJs(cProperties.statusBarContentColor, statusProperty.contentColor_,
                propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag);
    } else {
        if (cProperties.isStatusBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag = true;
    }
    if (!cProperties.navigationBarContentColor.empty()) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ =
            GetColorFromJs(cProperties.navigationBarContentColor, navProperty.contentColor_,
                propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag);
    } else {
        if (cProperties.isNavigationBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag = true;
    }
}

int32_t CJWindowImpl::SetWindowSystemBarProperties(const CBarProperties& cProperties)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::map<WindowType, SystemBarProperty> properties;
    std::map<WindowType, SystemBarPropertyFlag> propertyFlags;
    SetBarPropertyMap(properties, propertyFlags, cProperties, windowToken_);
    UpdateSystemBarProperties(properties, propertyFlags, windowToken_);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, properties.at(WindowType::WINDOW_TYPE_STATUS_BAR)));
    if (ret != WmErrorCode::WM_OK) {
        return static_cast<int32_t>(ret);
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR,
            properties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR)));
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowSystemBarEnable(char** arr, uint32_t size)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    std::map<WindowType, SystemBarProperty> systemBarProperties;
    std::map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    auto statusProperty = windowToken_->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto navProperty = windowToken_->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    statusProperty.enable_ = false;
    navProperty.enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = SystemBarPropertyFlag();
    for (uint32_t i = 0; i < size; i++) {
        std::string name = arr[i];
        if (name.compare("status") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
        } else if (name.compare("navigation") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
        }
    }
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableFlag = true;
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableFlag = true;
    UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags, windowToken_);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR)));
    if (ret != WmErrorCode::WM_OK) {
        return static_cast<int32_t>(ret);
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR,
        systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR)));
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::OnRegisterWindowCallback(const std::string& type, int64_t funcId, int64_t parameter)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WmErrorCode ret = registerManager_->RegisterListener(windowToken_, type, CaseType::CASE_WINDOW, funcId, parameter);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Register failed, window [%{public}u, %{public}s] type: %{public}s",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), type.c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::UnregisterWindowCallback(const std::string& type, int64_t funcId)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    WmErrorCode ret = registerManager_->UnregisterListener(windowToken_, type, CaseType::CASE_WINDOW, funcId);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Unregister failed, window [%{public}u, %{public}s] type: %{public}s",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), type.c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetSubWindowModal(bool isModal)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (!WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "Invalid Window Type");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    ModalityType modalityType = ModalityType::WINDOW_MODALITY;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->SetSubWindowModal(isModal, modalityType));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "Set subWindow modal failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowLimits(const CWindowLimits& cWindowLimits,
                                      CWindowLimits& retPtr)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;

    WindowLimits windowLimits;
    windowLimits.maxWidth_ = cWindowLimits.maxWidth;
    windowLimits.maxHeight_ = cWindowLimits.maxHeight;
    windowLimits.minWidth_ = cWindowLimits.minWidth;
    windowLimits.minHeight_ = cWindowLimits.minHeight;

    if (windowLimits.maxWidth_ < 0 || windowLimits.maxHeight_ < 0 ||
        windowLimits.minWidth_ < 0 || windowLimits.minHeight_ < 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Width or height should be greatr or equal to 0");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WindowLimits sizeLimits(windowLimits);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetWindowLimits(sizeLimits));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Set window limits failed, window [%{public}u, %{public}s]",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    retPtr.maxWidth = sizeLimits.maxWidth_;
    retPtr.maxHeight = sizeLimits.maxHeight_;
    retPtr.minWidth = sizeLimits.minWidth_;
    retPtr.maxHeight = sizeLimits.minHeight_;
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::GetWindowLimits(CWindowLimits& retPtr)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    WindowLimits windowLimits;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowLimits(windowLimits));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Get Window limits failed, window [%{public}u, %{public}s]",
              windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    retPtr.maxWidth = windowLimits.maxWidth_;
    retPtr.maxHeight = windowLimits.maxHeight_;
    retPtr.minWidth = windowLimits.minWidth_;
    retPtr.maxHeight = windowLimits.minHeight_;
    return static_cast<int32_t>(ret);
}

bool CJWindowImpl::GetImmersiveModeEnabledState(int32_t& errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "WindowToken_ is nullptr");
        errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return false;
    }

    bool ret = windowToken_->GetImmersiveModeEnabledState();
    errCode = 0;
    return ret;
}

int32_t CJWindowImpl::SetImmersiveModeEnabledState(bool enabled)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetImmersiveModeEnabledState(enabled));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "Set immersive mode failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::KeepKeyboardOnFocus(bool keepKeyboardFlag)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (!WindowHelper::IsSystemWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD,
            "KeepkeyboardOnFocus is not allowed since window is not system window or app subwindow");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    WmErrorCode ret = windowToken_->KeepKeyboardOnFocus(keepKeyboardFlag);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Keep keyboard on focus failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::GetWindowDecorHeight(int32_t& height)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->GetDecorHeight(height));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Get window decor height failed, window [%{public}u, %{public}s]",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowDecorHeight(int32_t height)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (height < MIN_DECOR_HEIGHT || height > MAX_DECOR_HEIGHT) {
        TLOGE(WmsLogTag::WMS_DECOR, "height should be greater than 37 or smaller than 112");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDecorHeight(height));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Set window decor height failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::Recover()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Recover());
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Recover failed, window [%{public}u, %{public}s]",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowDecorVisible(bool isVisible)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDecorVisible(isVisible));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Set window decor visible failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::GetTitleButtonRect(CTitleButtonRect& retPtr)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    TitleButtonRect titleButtonRect;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetTitleButtonArea(titleButtonRect));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Get window title button rect failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    retPtr.right = titleButtonRect.posX_;
    retPtr.top = titleButtonRect.posY_;
    retPtr.width = titleButtonRect.width_;
    retPtr.height = titleButtonRect.height_;
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetDialogBackGestureEnabled(bool enabled)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDialogBackGestureEnabled(enabled));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Set dialog backgesture failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::DisableLandscapeMultiWindow()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetLandscapeMultiWindow(false));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Disable landscape multi window failed, window [%{public}u, %{public}s]",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::EnableLandscapeMultiWindow()
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetLandscapeMultiWindow(true));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Enable landscape multi window failed, window [%{public}u, %{public}s]",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::SetWindowGrayScale(float grayScale)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    constexpr float eps = 1e-6;
    if (grayScale < (MIN_GRAY_SCALE - eps) || grayScale > (MAX_GRAY_SCALE + eps)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE,
            "graysclae should be greater than or equal to 0.0 or smaller than or equal to 1.0");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetGrayScale(grayScale));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Set window grayscale failed, window [%{public}u, %{public}s]",
              weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

uint32_t CJWindowImpl::GetPreferredOrientation(int32_t& errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "WindowToken_ is nullptr");
        errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return 0;
    }

    Orientation orientation = windowToken_->GetRequestedOrientation();
    if (orientation < Orientation::UNSPECIFIED || orientation > Orientation::LOCKED) {
        TLOGE(WmsLogTag::DEFAULT, "Orientation is invalid");
        errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
        return static_cast<uint32_t>(orientation);
    }

    return static_cast<uint32_t>(orientation);
}

int32_t CJWindowImpl::GetWindowStatus(int32_t& errCode)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "WindowToken_ is nullptr");
        errCode = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return 0;
    }

    WindowStatus windowStatus;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowStatus(windowStatus));
    errCode = static_cast<int32_t>(ret);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "Get window status failed, window [%{public}u, %{public}s]",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }

    return static_cast<int32_t>(windowStatus);
}

static std::string GetHexColor(uint32_t color)
{
    const int32_t rgbaLength = 8;

    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = rgbaLength - static_cast<int>(temp.length());
    std::string finalColor("#");
    std::string tmpColor(count, '0');
    tmpColor += temp;
    finalColor += tmpColor;

    return finalColor;
}

static char* CopyToHeap(const std::string& str)
{
    char* heapStr = new char[str.length() + 1];
    errno_t err = strcpy_s(heapStr, str.length() + 1, str.c_str());
    if (err != EOK) {
        delete[] heapStr;
        return nullptr;
    }
    return heapStr;
}

int32_t CJWindowImpl::GetWindowSystemBarProperties(CJBarProperties& retPtr)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;

    SystemBarProperty status = weakWindow->GetSystemBarPropertyByType(
        WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty navi = weakWindow->GetSystemBarPropertyByType(
        WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    
    retPtr.statusBarColor = CopyToHeap(GetHexColor(status.backgroundColor_));
    retPtr.statusBarContentColor = CopyToHeap(GetHexColor(status.contentColor_));
    retPtr.isStatusBarLightIcon = status.contentColor_ == SYSTEM_COLOR_WHITE;
    retPtr.navigationBarColor = CopyToHeap(GetHexColor(navi.backgroundColor_));
    retPtr.navigationBarContentColor = CopyToHeap(GetHexColor(navi.contentColor_));
    retPtr.isNavigationBarLightIcon = navi.contentColor_ == SYSTEM_COLOR_WHITE;
    retPtr.enableStatusBarAnimation = status.enableAnimation_;
    retPtr.enableNavigationBarAnimation = navi.enableAnimation_;
    return result.ret;
}

int32_t CJWindowImpl::SpecificSystemBarEnabled(int32_t name, bool enable,
                                               bool enableAnimation)
{
    ResWindow result = CheckWindow();
    if (result.ret != 0) {
        return result.ret;
    }
    sptr<Window> weakWindow = result.nativeWindow;
    std::map<WindowType, SystemBarProperty> systemBarProperties;
    if (name < 0 || name >= static_cast<int32_t>(SYSTEM_BAR.size())) {
        TLOGE(WmsLogTag::WMS_IMMS, "Invalid input");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    SpecificSystemBar barName = SYSTEM_BAR[name];
    if (barName == SpecificSystemBar::STATUS) {
        systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = enable;
        systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ = enableAnimation;
    } else if (barName == SpecificSystemBar::NAVIGATION) {
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = enable;
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ = enableAnimation;
    } else if (barName == SpecificSystemBar::NAVIGATION_INDICATOR) {
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = enable;
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enableAnimation_ = enableAnimation;
    }

    WmErrorCode ret = WmErrorCode::WM_OK;
    if (barName == SpecificSystemBar::STATUS) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSpecificBarProperty(
            WindowType::WINDOW_TYPE_STATUS_BAR,
            systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR)));
    } else if (barName == SpecificSystemBar::NAVIGATION) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSpecificBarProperty(
            WindowType::WINDOW_TYPE_NAVIGATION_BAR,
            systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR)));
    } else if (barName == SpecificSystemBar::NAVIGATION_INDICATOR) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSpecificBarProperty(
            WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR,
            systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR)));
    }

    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "Get window status failed, window [%{public}u, %{public}s]",
              weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    }

    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::Maximize(int32_t presentation)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (!WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "subwindow hide");
        Hide();
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (presentation < 0 || presentation >= static_cast<int32_t>(MAXIMIZE_PRESENTATION.size())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Invalid input");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->Maximize(MAXIMIZE_PRESENTATION[presentation]));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "maximize window failed, window [%{public}u, %{public}s] ",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    }
    return static_cast<int32_t>(ret);
}

int32_t CJWindowImpl::CreateSubWindowWithOptions(std::string name,
                                                 int64_t &windowId,
                                                 CSubWindowOptions option)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "WindowToken_ is nullptr");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    if (!windowToken_->IsPcOrFreeMultiWindowCapabilityEnabled()) {
        TLOGE(WmsLogTag::WMS_SUB, "Device not supported");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    }

    if (!WindowHelper::IsSystemWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "This is not subWindow or mainWindow");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    sptr<WindowOption> windowOption = new WindowOption();
    windowOption->SetSubWindowTitle(option.title);
    windowOption->SetSubWindowDecorEnable(option.decorEnabled);

    if ((windowOption->GetWindowFlags() &
            static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL))) {
        TLOGE(WmsLogTag::WMS_SUB, "Device not supported");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    }

    if ((windowOption->GetWindowTopmost() && !Permission::IsSystemCalling() &&
            !Permission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    windowOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowOption->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowOption->SetOnlySupportSceneBoard(true);
    windowOption->SetParentId(windowToken_->GetWindowId());
    windowOption->SetWindowTag(WindowTag::SUB_WINDOW);
    auto window = Window::Create(name, windowOption, windowToken_->GetContext());
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Create sub window failed");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    sptr<CJWindowImpl> windowImpl = CreateCjWindowObject(window);
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "[createSubWindow] windowImpl is null");
        return static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    windowId = windowImpl->GetID();
    return static_cast<int32_t>(WmErrorCode::WM_OK);
}
}
}
