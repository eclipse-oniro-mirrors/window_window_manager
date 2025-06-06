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

#ifndef OHOS_WINDOW_PROXY_H
#define OHOS_WINDOW_PROXY_H

#include "window_interface.h"
#include "iremote_proxy.h"
#include "wm_common.h"
#include <transaction/rs_transaction.h>

namespace OHOS {
namespace Rosen {
class WindowProxy : public IRemoteProxy<IWindow> {
public:
    explicit WindowProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IWindow>(impl) {};

    ~WindowProxy() {};

    WMError UpdateWindowRect(const struct Rect& rect, bool decoStatus, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    WMError UpdateWindowMode(WindowMode mode) override;
    WMError UpdateWindowModeSupportType(uint32_t windowModeSupportType) override;
    WMError UpdateFocusStatus(bool focused) override;
    WMError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;
    WMError UpdateWindowState(WindowState state) override;
    WMError UpdateWindowDragInfo(const PointInfo& point, DragEvent event) override;
    WMError UpdateDisplayId(DisplayId from, DisplayId to) override;
    WMError UpdateOccupiedAreaChangeInfo(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    WMError UpdateOccupiedAreaAndRect(const sptr<OccupiedAreaChangeInfo>& info, const Rect& rect,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    WMError UpdateActiveStatus(bool isActive) override;
    sptr<WindowProperty> GetWindowProperty() override;
    WMError NotifyTouchOutside() override;
    WMError NotifyScreenshot() override;
    WMError NotifyScreenshotAppEvent(ScreenshotEventType type) override;
    WMError DumpInfo(const std::vector<std::string>& params) override;
    WMError NotifyDestroy(void) override;
    WMError NotifyForeground(void) override;
    WMError NotifyBackground(void) override;
    WMError NotifyWindowClientPointUp(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    WMError UpdateZoomTransform(const Transform& trans, bool isDisplayZoomOn) override;
    WMError RestoreSplitWindowMode(uint32_t mode) override;
    void ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent> event) override;
    void NotifyForegroundInteractiveStatus(bool interactive) override;
    void NotifyMMIServiceOnline(uint32_t winId) override;

private:
    static inline BrokerDelegator<WindowProxy> delegator_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_PROXY_H
