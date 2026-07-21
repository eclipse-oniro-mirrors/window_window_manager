/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_MOVE_DRAG_BOUNDS_APPLIER_H
#define OHOS_ROSEN_MOVE_DRAG_BOUNDS_APPLIER_H

#include <cstdint>
#include <memory>
#include <optional>

#include <refbase.h>

#include "ws_common.h"

namespace OHOS::Rosen {
class RSSurfaceNode;
class RSUIContext;
class SceneSession;

template <typename T> class Vector4;
typedef Vector4<float> Vector4f;

/**
 * @brief Applies move-drag bounds to original and shadow surface nodes.
 *
 * During move-drag, bounds may need to be submitted immediately by SSM thread.
 * This applier uses shadow nodes for immediate BOUNDS / FRAME submission through
 * the independent RSUIContext, while keeping original nodes in sync for the normal
 * transaction path.
 */
class MoveDragBoundsApplier : public std::enable_shared_from_this<MoveDragBoundsApplier> {
public:
    /**
     * @brief Creates an applier bound to the owner SceneSession.
     *
     * @param session Owner SceneSession.
     */
    explicit MoveDragBoundsApplier(wptr<SceneSession> session);

    /**
     * @brief Applies move-drag bounds to RS surface nodes.
     *
     * When needFlush is true, bounds are first applied through shadow nodes for
     * immediate rendering in the current frame. The same bounds are then applied
     * to original nodes to prevent later normal transactions from committing
     * stale geometry.
     *
     * @param rect Target window rect.
     * @param isGlobal Whether rect is in global coordinates.
     * @param needFlush Whether to submit the current frame immediately.
     */
    void Apply(const WSRect& rect, bool isGlobal, bool needFlush);

    /**
     * @brief Gets the active shadow node used as the move-drag target.
     *
     * If a leash shadow node exists, it is preferred because the leash node owns
     * positioned window bounds. Otherwise, the surface shadow node is used.
     *
     * @return Active shadow node, original node as fallback, or nullptr.
     */
    std::shared_ptr<RSSurfaceNode> GetTargetShadowSurfaceNode();

private:
    /**
     * @brief Gets or creates the surface shadow node.
     *
     * The shadow node is cached by source surface node id. When the source node
     * changes, the cached shadow is recreated to avoid updating an obsolete RS node.
     * If shadow node creation fails, the original surface node is returned.
     *
     * @return Surface shadow node, original surface node as fallback, or nullptr.
     */
    std::shared_ptr<RSSurfaceNode> GetShadowSurfaceNode();

    /**
     * @brief Gets or creates the leash window shadow node.
     *
     * When a leash node exists, it carries the positioned window bounds and is
     * therefore preferred as the move-drag target. The shadow node is cached by
     * source leash node id and recreated when the source node changes. If shadow
     * node creation fails, the original leash node is returned.
     *
     * @return Leash shadow node, original leash node as fallback, or nullptr.
     */
    std::shared_ptr<RSSurfaceNode> GetLeashWinShadowSurfaceNode();

    /**
     * @brief Handles bounds changes reported by a move-drag shadow node.
     *
     * Updates SceneSession bounds and rect state so server-side layout data stays
     * consistent with the RS node changed during move-drag.
     *
     * @param rsUIContext RSUIContext used to create the sync transaction.
     * @param bounds Bounds reported by RS.
     */
    void OnBoundsChanged(std::shared_ptr<RSUIContext> rsUIContext, const Vector4f& bounds);

    /**
     * @brief Applies bounds to the original surface tree.
     *
     * This method is used for normal transaction commit together with ArkUI relayout.
     * It does not create an AutoRSTransaction by itself.
     *
     * @param rect Target window rect.
     * @param isGlobal Whether rect is in global coordinates.
     */
    void ApplyToOriginalNode(const WSRect& rect, bool isGlobal);

    /**
     * @brief Applies bounds through shadow nodes for immediate move-drag rendering.
     *
     * Shadow-node updates are committed independently to avoid flushing unrelated
     * pending SurfaceNode updates in the current transaction.
     *
     * @param rect Target window rect.
     * @param isGlobal Whether rect is in global coordinates.
     */
    void ApplyToShadowNode(const WSRect& rect, bool isGlobal);

    /**
     * @brief Applies positioned window bounds to a surface node.
     *
     * Both Bounds and Frame are updated because RS rendering and hit-test geometry
     * depend on them staying consistent during move-drag.
     *
     * @param node Target surface node.
     * @param rect Target window rect.
     * @param isGlobal Whether rect is in global coordinates.
     * @return True if bounds are applied; false otherwise.
     */
    static bool ApplyWindowBounds(const std::shared_ptr<RSSurfaceNode>& node, const WSRect& rect, bool isGlobal);

    /**
     * @brief Applies content-local bounds to a surface node hosted by a leash node.
     *
     * When a leash node owns the window position, the surface node keeps only the
     * local content size at origin (0, 0).
     *
     * @param node Target surface node.
     * @param rect Target window rect whose width and height are used.
     * @return True if bounds are applied; false otherwise.
     */
    static bool ApplyContentBounds(const std::shared_ptr<RSSurfaceNode>& node, const WSRect& rect);

    /**
     * @brief Owner SceneSession.
     */
    wptr<SceneSession> session_;

    /**
     * @brief Cached shadow node for the original surface node.
     *
     * This node is lazily initialized when the window is dragged for the first time.
     * It copies the necessary RSBoundsModifier and RSFrameModifier from the original
     * surface node to support drag updates.
     */
    std::shared_ptr<RSSurfaceNode> shadowSurfaceNode_;

    /**
     * @brief Source node id corresponding to the original surfaceNode.
     */
    std::optional<uint64_t> shadowSourceNodeId_;

    /**
     * @brief Cached shadow node for the leash window surface node.
     *
     * This node is lazily initialized when the window is dragged for the first time.
     * Initialization occurs only if `leashWinSurfaceNode_` exists, and copies the
     * necessary RSBoundsModifier and RSFrameModifier from it to support drag updates.
     */
    std::shared_ptr<RSSurfaceNode> leashWinShadowSurfaceNode_;

    /**
     * @brief Source node id corresponding to the original leashWinSurfaceNode.
     */
    std::optional<uint64_t> leashWinShadowSourceNodeId_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_MOVE_DRAG_BOUNDS_APPLIER_H
