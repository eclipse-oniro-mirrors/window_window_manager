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

#ifndef GTX_INPUT_EVENT_SENDER_H
#define GTX_INPUT_EVENT_SENDER_H

#include <mutex>
#include <atomic>
#include <algorithm>

#include "pointer_event.h"
#include "window.h"
#include "window_input_channel.h"
#include "window_input_redistribute_client.h"

namespace OHOS {
const uint32_t GTX_MAX_TOUCH_POINTS_NUMBER = 10;

struct GtxTouchPoint {
    int32_t id = -1;
    float screenX = .0f;
    float screenY = .0f;
    float x = .0f;
    float y = .0f;
    bool isPressed = false;
    double pressure = .0f;
};

struct GtxWindowExtent {
    int32_t offsetX = .0;
    int32_t offsetY = .0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct GtxTouchEventInfo {
    uint32_t windowId = 0;
    int32_t pointerId = -1;
    GtxWindowExtent extent = {};
    uint32_t numPoints = 0;
    GtxTouchPoint touchPoints[GTX_MAX_TOUCH_POINTS_NUMBER];
};

class GtxInputEventSender {
public:
    GtxInputEventSender() {}

    ~GtxInputEventSender() {}

    static GtxInputEventSender& GetInstance();

    void SetOpt(bool opt)
    {
        mIsEnable.store(opt);
    }

    void GetTouchEvent(GtxTouchEventInfo& touchEvent);
    void SetTouchEvent(Rosen::Rect rect, std::shared_ptr<MMI::PointerEvent> pointerEvent);

    void RegisterInputEventScale(int offsetX, int offsetY, float scaleX, float scaleY);
    void UnRegisterInputEventScale();

    class NapiAVSessionInputRedistributeCallback : public Rosen::IInputEventRecipientCallback {
    public:
        NapiAVSessionInputRedistributeCallback(int offsetX, int offsetY, float scaleX, float scaleY)
            : mOffsetX(offsetX), mOffsetY(offsetY), mScaleX(scaleX), mScaleY(scaleY) {}
        Rosen::InputAfterRedistributeBehavior OnInputEvent(const std::shared_ptr<MMI::KeyEvent>& KeyEvent) override;
        Rosen::InputAfterRedistributeBehavior
            OnInputEvent(const std::shared_ptr<MMI::PointerEvent>& PointerEvent) override;
        void SetTouchEventScale(int offsetX, int offsetY, float scaleX, float scaleY);

    private:
        int32_t mOffsetX = 0;
        int32_t mOffsetY = 0;
        float mScaleX = 1.0f;
        float mScaleY = 1.0f;
    };

private:
    std::atomic<bool> mIsEnable = false;
    std::shared_ptr<NapiAVSessionInputRedistributeCallback> mCallbackInstance;
    Rosen::IInputEventRecipientInfo mRecipientInfo;

    std::mutex mEventMutex;
    GtxTouchEventInfo mEvent = {};
};
} // namespace OHOS

#endif