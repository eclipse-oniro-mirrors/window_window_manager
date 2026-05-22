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

#include "gtx_input_event_sender.h"
#include "window_manager_hilog.h"

namespace OHOS {
using Rosen::g_domainContents;
using Rosen::HILOG_DOMAIN_WINDOW;
using Rosen::WmsLogTag;

GtxInputEventSender& GtxInputEventSender::GetInstance()
{
    static GtxInputEventSender instance;
    return instance;
}

void GtxInputEventSender::GetTouchEvent(GtxTouchEventInfo& touchEvent)
{
    std::unique_lock<std::mutex> lock(mEventMutex);
    touchEvent = mEvent;
}

void GtxInputEventSender::SetTouchEvent(Rosen::Rect rect, std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    if (!mIsEnable.load()) {
        return;
    }
    if (pointerEvent == nullptr) {
        return;
    }

    std::unique_lock<std::mutex> lock(mEventMutex);
    mEvent = {};
    mEvent.windowId = static_cast<uint32_t>(pointerEvent->GetAgentWindowId());
    mEvent.pointerId = pointerEvent->GetPointerId();
    mEvent.extent = { rect.posX_, rect.posY_, rect.width_, rect.height_ };

    std::vector<int32_t> pointIndex = pointerEvent->GetPointerIds();
    mEvent.numPoints = std::min(pointIndex.size(), static_cast<size_t>(GTX_MAX_TOUCH_POINTS_NUMBER));

    for (uint32_t i = 0; i < mEvent.numPoints; i++) {
        GtxTouchPoint& dstTouchPoint = mEvent.touchPoints[i];
        MMI::PointerEvent::PointerItem srcTouchPoint;
        pointerEvent->GetPointerItem(pointIndex[i], srcTouchPoint);

        dstTouchPoint.id = pointIndex[i];
        dstTouchPoint.screenX = srcTouchPoint.GetDisplayX();
        dstTouchPoint.screenY = srcTouchPoint.GetDisplayY();
        dstTouchPoint.x = srcTouchPoint.GetWindowX();
        dstTouchPoint.y = srcTouchPoint.GetWindowY();
        dstTouchPoint.isPressed = srcTouchPoint.IsPressed();
        dstTouchPoint.pressure = srcTouchPoint.GetPressure();
    }
}

Rosen::InputAfterRedistributeBehavior GtxInputEventSender::NapiAVSessionInputRedistributeCallback::OnInputEvent(
    const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    (void)keyEvent;
    return Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL;
}

Rosen::InputAfterRedistributeBehavior GtxInputEventSender::NapiAVSessionInputRedistributeCallback::OnInputEvent(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        return Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL;
    }
    for (auto& item : pointerEvent->GetAllPointerItems()) {
        int32_t displayX = item.GetDisplayX();
        int32_t WindowX = item.GetWindowX();
        double DisplayXPos = item.GetDisplayXPos();
        double WindowXPos = item.GetWindowXPos();
        double globalX = item.GetGlobalX();

        int32_t displayY = item.GetDisplayY();
        int32_t WindowY = item.GetWindowY();
        double DisplayYPos = item.GetDisplayYPos();
        double WindowYPos = item.GetWindowYPos();
        double globalY = item.GetGlobalY();

        item.SetDisplayX(displayX * mScaleX + mOffsetX);
        item.SetWindowX(WindowX * mScaleX + mOffsetX);
        item.SetDisplayXPos(DisplayXPos * mScaleX + mOffsetX);
        item.SetWindowXPos(WindowXPos * mScaleX + mOffsetX);
        item.SetGlobalX(globalX * mScaleX + mOffsetX);

        item.SetDisplayY(displayY * mScaleY + mOffsetY);
        item.SetWindowY(WindowY * mScaleY + mOffsetY);
        item.SetDisplayYPos(DisplayYPos * mScaleY + mOffsetY);
        item.SetWindowYPos(WindowYPos * mScaleY + mOffsetY);
        item.SetGlobalY(globalY * mScaleY + mOffsetY);

        pointerEvent->UpdatePointerItem(item.GetPointerId(), item);
    }
    return Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL;
}

void GtxInputEventSender::RegisterInputEventScale(int offsetX, int offsetY, float scaleX, float scaleY)
{
    if (mCallbackInstance != nullptr) {
        Rosen::WindowInputRedistributeClient::UnRegisterInputEventRedistribute(mRecipientInfo);
    }
    mCallbackInstance = std::make_shared<NapiAVSessionInputRedistributeCallback>(offsetX, offsetY, scaleX, scaleY);
    mRecipientInfo.identity = Rosen::InputRedistributeIdentity::IDENTITY_CONTROL_DISPLAY_SEPARATION;
    mRecipientInfo.timing = Rosen::InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT;
    mRecipientInfo.type = Rosen::InputEventType::POINTER_EVENT;
    mRecipientInfo.priority = Rosen::DEFAULT_REDISTRIBUTE_PRIORITY;
    mRecipientInfo.recipient = mCallbackInstance;
    Rosen::WindowInputRedistributeClient::RegisterInputEventRedistribute(mRecipientInfo);
}

void GtxInputEventSender::UnRegisterInputEventScale()
{
    Rosen::WindowInputRedistributeClient::UnRegisterInputEventRedistribute(mRecipientInfo);
    mCallbackInstance.reset();
}

extern "C" {
__attribute__((visibility("default"))) void SetGtxTouchEventStatus(bool isEnable)
{
    OHOS::GtxInputEventSender::GetInstance().SetOpt(isEnable);
}

__attribute__((visibility("default"))) void GetGtxTouchEvent(OHOS::GtxTouchEventInfo& touchEvent)
{
    OHOS::GtxInputEventSender::GetInstance().GetTouchEvent(touchEvent);
}

__attribute__((visibility("default"))) void RegisterInputEventScale(int offsetX, int offsetY,
    float scaleX, float scaleY)
{
    OHOS::GtxInputEventSender::GetInstance().RegisterInputEventScale(offsetX, offsetY, scaleX, scaleY);
}

__attribute__((visibility("default"))) void UnRegisterInputEventScale()
{
    OHOS::GtxInputEventSender::GetInstance().UnRegisterInputEventScale();
}
}

} // namespace OHOS
