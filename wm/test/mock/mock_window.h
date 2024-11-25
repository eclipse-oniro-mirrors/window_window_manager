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

#ifndef MOCK_WINDOW_H
#define MOCK_WINDOW_H

#include "window.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class MockWindowChangeListener : public IWindowChangeListener {
public:
    MOCK_METHOD3(OnSizeChange, void(Rect rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction));
    MOCK_METHOD2(OnModeChange, void(WindowMode, bool));
};

class MockWindowLifeCycleListener : public IWindowLifeCycle {
public:
    MOCK_METHOD0(AfterForeground, void(void));
    MOCK_METHOD0(AfterBackground, void(void));
    MOCK_METHOD0(AfterFocused, void(void));
    MOCK_METHOD0(AfterUnfocused, void(void));
    MOCK_METHOD1(ForegroundFailed, void(int32_t));
    MOCK_METHOD1(BackgroundFailed, void(int32_t));
    MOCK_METHOD0(AfterActive, void(void));
    MOCK_METHOD0(AfterInactive, void(void));
    MOCK_METHOD0(AfterResumed, void(void));
    MOCK_METHOD0(AfterPaused, void(void));
    MOCK_METHOD0(AfterDestroyed, void(void));
};

class MockAvoidAreaChangedListener : public IAvoidAreaChangedListener {
public:
    MOCK_METHOD2(OnAvoidAreaChanged, void(const AvoidArea avoidArea, AvoidAreaType type));
};

class MockTouchOutsideListener : public ITouchOutsideListener {
public:
    MOCK_CONST_METHOD0(OnTouchOutside, void());
};

class MockWindowStatusChangeListener : public IWindowStatusChangeListener {
public:
    MOCK_METHOD1(OnWindowStatusChange, void(WindowStatus status));
};

class MockIOccupiedAreaChangeListener : public IOccupiedAreaChangeListener {
public:
    MOCK_METHOD2(OnSizeChange, void(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction));
};

class MockInputEventConsumer : public IInputEventConsumer {
public:
    MOCK_CONST_METHOD1(OnInputEvent, bool(const std::shared_ptr<MMI::KeyEvent>&));
    MOCK_CONST_METHOD1(OnInputEvent, bool(const std::shared_ptr<MMI::PointerEvent>&));
    MOCK_CONST_METHOD1(OnInputEvent, bool(const std::shared_ptr<MMI::AxisEvent>&));
};

class MockWindowNoInteractionListener : public IWindowNoInteractionListener {
public:
    MOCK_METHOD0(OnWindowNoInteractionCallback, void());
    MOCK_METHOD1(SetTimeout, void(int64_t));
    MOCK_CONST_METHOD0(GetTimeout, int64_t());
};

class MockWindowTitleButtonRectChangedListener : public IWindowTitleButtonRectChangedListener {
public:
    MOCK_METHOD1(OnWindowTitleButtonRectChanged, void(const TitleButtonRect&));
};

class MockWindowRectChangeListener : public IWindowRectChangeListener {
public:
    MOCK_METHOD2(OnRectChange, void(Rect rect, WindowSizeChangeReason reason));
};

class MockIAceAbilityHandler : public IAceAbilityHandler {
public:
    MOCK_METHOD1(SetBackgroundColor, void(uint32_t color));
    MOCK_METHOD0(GetBackgroundColor, uint32_t());
};

class MockIDisplayMoveListener : public IDisplayMoveListener {
public:
    MOCK_METHOD2(OnDisplayMove, void(DisplayId from, DisplayId to));
};

class MockIWindowNoInteractionListener : public IWindowNoInteractionListener {
public:
    MOCK_METHOD0(OnWindowNoInteractionCallback, void());
};

class MockIDialogDeathRecipientListener : public IDialogDeathRecipientListener {
public:
    MOCK_CONST_METHOD0(OnDialogDeathRecipient, void());
};

class MockIDialogTargetTouchListener : public IDialogTargetTouchListener {
public:
    MOCK_CONST_METHOD0(OnDialogTargetTouch, void());
};

class MockIScreenshotListener : public IScreenshotListener {
public:
    MOCK_METHOD0(OnScreenshot, void());
};

class MockISubWindowCloseListener : public ISubWindowCloseListener {
public:
    MOCK_METHOD1(OnSubWindowClose, void(bool&));
};

class MockIMainWindowCloseListener : public IMainWindowCloseListener {
public:
    MOCK_METHOD1(OnMainWindowClose, void(bool&));
};

class MockISwitchFreeMultiWindowListener : public ISwitchFreeMultiWindowListener {
public:
    MOCK_METHOD1(OnSwitchFreeMultiWindow, void(bool));
};
} // Rosen
} // OHOS

#endif