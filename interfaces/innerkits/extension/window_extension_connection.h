/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef WINDOW_EXTENSION_CONNECTION_H
#define WINDOW_EXTENSION_CONNECTION_H

#include <string>
#include <element_name.h>
#include <i_input_event_consumer.h>
#include <input_manager.h>
#include <key_event.h>
#include <pointer_event.h>
#include <memory>

#include "wm_common.h"

namespace OHOS {
namespace Rosen {
extern const std::string RECT_FORM_KEY_POS_X;
extern const std::string RECT_FORM_KEY_POS_Y;
extern const std::string RECT_FORM_KEY_HEIGHT;
extern const std::string RECT_FORM_KEY_WIDTH;
extern const std::string WINDOW_ID;

class RSSurfaceNode;
class ExtensionSession;
/**
 * @class IWindowExtensionCallback
 *
 * @brief Callback of window extension.
 */
class IWindowExtensionCallback : virtual public RefBase {
public:
    /**
     * @brief Window ready to connect.
     *
     * @param rsSurfaceNode Surface node from RS.
     */
    virtual void OnWindowReady(const std::shared_ptr<RSSurfaceNode>& rsSurfaceNode) = 0;

    /**
     * @brief Callback when window extension disconnected.
     */
    virtual void OnExtensionDisconnected() = 0;

    /**
     * @brief Callback when receive key event.
     *
     * @param event Keyboard event.
     */
    virtual void OnKeyEvent(const std::shared_ptr<MMI::KeyEvent>& event) = 0;

    /**
     * @brief Callback when receive pointer event.
     *
     * @param event Pointer event.
     */
    virtual void OnPointerEvent(const std::shared_ptr<MMI::PointerEvent>& event) = 0;

    /**
     * @brief Callback when press back button.
     */
    virtual void OnBackPress() = 0;
};

/**
 * @class WindowExtensionConnection
 *
 * @brief Connection for window extension.
 */
class WindowExtensionConnection : public RefBase {
public:
    /**
     * @brief Constructor for WindowExtensionConnection.
     */
    WindowExtensionConnection();

    /**
     * @brief Deconstructor for WindowExtensionConnection.
     */
    ~WindowExtensionConnection();

    /**
     * @brief Deconstructor for WindowExtensionConnection.
     *
     * @param element Element name.
     * @param rect Rect of window extension.
     * @param uid User id.
     * @param windowId Window id.
     * @param callback Callback for window extension.
     * @return ERR_OK means connect success, others means connect failed.
     */
    int ConnectExtension(const AppExecFwk::ElementName& element, const Rect& rect,
        uint32_t uid, uint32_t windowId, const sptr<IWindowExtensionCallback>& callback) const;

    /**
     * @brief Deconstructor for WindowExtensionConnection.
     *
     * @param element Element name.
     * @param rect Rect of window extension.
     * @param uid User id.
     * @param windowId Window id.
     * @param callback Callback for window extension.
     * @param extensionSession session for extension
     * @return ERR_OK means connect success, others means connect failed.
     */
    int ConnectExtension(const AppExecFwk::ElementName& element, const Rect& rect,
        uint32_t uid, uint32_t windowId, const sptr<IWindowExtensionCallback>& callback,
        const sptr<ExtensionSession>& extensionSession) const;

    /**
     * @brief Disconnect window extension.
     */
    void DisconnectExtension() const;

    /**
     * @brief Show window extension.
     */
    void Show() const;

    /**
     * @brief Hide window extension.
     */
    void Hide() const;

    /**
     * @brief Request focus of window extension.
     */
    void RequestFocus() const;

    /**
     * @brief Set bounds of window extension.
     *
     * @param rect Window extension rect.
     */
    void SetBounds(const Rect& rect) const;
private:
    class Impl;
    sptr<Impl> pImpl_;
};
} // namespace Rosen
} // namespace OHOS
#endif // WINDOW_EXTENSION_CONNECTION_H