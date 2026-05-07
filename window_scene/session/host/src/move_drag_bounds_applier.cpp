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

#include "move_drag_bounds_applier.h"

#include <cmath>

#include <transaction/rs_sync_transaction_controller.h>
#include <ui/rs_surface_node.h>

#include "rs_adapter.h"
#include "scene_session.h"
#include "session_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
MoveDragBoundsApplier::MoveDragBoundsApplier(wptr<SceneSession> session) : session_(session) {}

void MoveDragBoundsApplier::Apply(const WSRect& rect, bool isGlobal, bool needFlush)
{
    // Submit through shadow nodes when the current move-drag frame must be
    // rendered immediately. Directly flushing the original node may also commit
    // unrelated pending SurfaceNode changes recorded in the current transaction.
    // Shadow nodes isolate the bounds/frame update from those pending changes.
    if (needFlush) {
        ApplyToShadowNode(rect, isGlobal);
    }

    // Always stage the same bounds on original nodes for the normal transaction
    // path, so later ArkUI relayout or RS commits do not overwrite the immediate
    // shadow-node result with stale geometry.
    ApplyToOriginalNode(rect, isGlobal);
}

std::shared_ptr<RSSurfaceNode> MoveDragBoundsApplier::GetTargetShadowSurfaceNode()
{
    if (auto leashWinShadowSurfaceNode = GetLeashWinShadowSurfaceNode()) {
        return leashWinShadowSurfaceNode;
    }
    return GetShadowSurfaceNode();
}

std::shared_ptr<RSSurfaceNode> MoveDragBoundsApplier::GetShadowSurfaceNode()
{
    auto session = session_.promote();
    if (!session) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
        return nullptr;
    }
    auto surfaceNode = session->GetSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "surfaceNode is null");
        return nullptr;
    }
    auto sourceNodeId = static_cast<uint64_t>(surfaceNode->GetId());
    if (shadowSurfaceNode_ && shadowSourceNodeId_ == sourceNodeId) {
        return shadowSurfaceNode_;
    }

    // Recreate the shadow node when the source surface node changes.
    //
    // Move-drag updates Bounds and Frame, so the shadow node must synchronize
    // RSBoundsModifier and RSFrameModifier from the source node. If the source node
    // has never initialized these modifiers, shadow creation may fail and the
    // original node is used as fallback.
    shadowSurfaceNode_ = surfaceNode->CreateShadowSurfaceNode(
        { ShadowPropertyType::BOUNDS, ShadowPropertyType::FRAME });
    shadowSourceNodeId_ = sourceNodeId;
    if (!shadowSurfaceNode_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create shadow surface node, fallback to surfaceNode");
        return surfaceNode;
    }

    if (!GetLeashWinShadowSurfaceNode()) {
        // Without a leash shadow node, the surface shadow is the move-drag target.
        shadowSurfaceNode_->SetBoundsChangedCallback(
            [weakThis = weak_from_this(),
             rsUIContext = shadowSurfaceNode_->GetRSUIContext()](const Vector4f& bounds) {
                if (auto applier = weakThis.lock()) {
                    applier->OnBoundsChanged(rsUIContext, bounds);
                }
            });
    }
    return shadowSurfaceNode_;
}

std::shared_ptr<RSSurfaceNode> MoveDragBoundsApplier::GetLeashWinShadowSurfaceNode()
{
    auto session = session_.promote();
    if (!session) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
        return nullptr;
    }
    auto leashWinSurfaceNode = session->GetLeashWinSurfaceNode();
    if (!leashWinSurfaceNode) {
        return nullptr;
    }
    auto sourceNodeId = static_cast<uint64_t>(leashWinSurfaceNode->GetId());
    if (leashWinShadowSurfaceNode_ && leashWinShadowSourceNodeId_ == sourceNodeId) {
        return leashWinShadowSurfaceNode_;
    }

    // Recreate the leash shadow node when the source leash node changes.
    //
    // Move-drag updates Bounds and Frame, so the shadow node must synchronize
    // RSBoundsModifier and RSFrameModifier from the source leash node. If the source
    // node has never initialized these modifiers, shadow creation may fail and the
    // original leash node is used as fallback.
    leashWinShadowSurfaceNode_ =
        leashWinSurfaceNode->CreateShadowSurfaceNode({ ShadowPropertyType::BOUNDS, ShadowPropertyType::FRAME });
    leashWinShadowSourceNodeId_ = sourceNodeId;
    if (!leashWinShadowSurfaceNode_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create shadow surface node, fallback to leashWinSurfaceNode");
        return leashWinSurfaceNode;
    }

    // A leash shadow owns positioned window bounds, so it is the move-drag target when available.
    leashWinShadowSurfaceNode_->SetBoundsChangedCallback(
        [weakThis = weak_from_this(),
         rsUIContext = leashWinShadowSurfaceNode_->GetRSUIContext()](const Vector4f& bounds) {
            if (auto applier = weakThis.lock()) {
                applier->OnBoundsChanged(rsUIContext, bounds);
            }
        });
    return leashWinShadowSurfaceNode_;
}

void MoveDragBoundsApplier::OnBoundsChanged(
    std::shared_ptr<RSUIContext> rsUIContext, const Vector4f& bounds)
{
    auto session = session_.promote();
    if (!session) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
        return;
    }
    session->SetBounds({ bounds.x_, bounds.y_, bounds.z_, bounds.w_ });
    WSRect rect;
    rect.posX_ = std::round(bounds.x_ + session->GetOffsetX());
    rect.posY_ = std::round(bounds.y_ + session->GetOffsetY());
    rect.width_ = std::round(bounds.z_);
    rect.height_ = std::round(bounds.w_);
    auto transaction = session->GetSessionRect() != rect ?
        RSSyncTransactionAdapter::GetRSTransaction(rsUIContext) : nullptr;
    auto ret = session->UpdateRect(rect, SizeChangeReason::UNDEFINED, __func__, transaction);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
            "Failed to update rect for move drag shadow surface node, id: %{public}d, error: %{public}d",
            session->GetPersistentId(), static_cast<int32_t>(ret));
    }
}

void MoveDragBoundsApplier::ApplyToOriginalNode(const WSRect& rect, bool isGlobal)
{
    auto session = session_.promote();
    if (!session) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
        return;
    }
    auto surfaceNode = session->GetSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "surfaceNode is null");
        return;
    }

    if (auto leashWinSurfaceNode = session->GetLeashWinSurfaceNode()) {
        // With a leash node, the surface node keeps content-local bounds,
        // while the leash node carries positioned window bounds.
        ApplyContentBounds(surfaceNode, rect);
        ApplyWindowBounds(leashWinSurfaceNode, rect, isGlobal);
    } else {
        ApplyWindowBounds(surfaceNode, rect, isGlobal);
    }
}

void MoveDragBoundsApplier::ApplyToShadowNode(const WSRect& rect, bool isGlobal)
{
    auto shadowSurfaceNode = GetShadowSurfaceNode();
    if (!shadowSurfaceNode) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "shadowSurfaceNode is null");
        return;
    }

    if (auto leashWinShadowSurfaceNode = GetLeashWinShadowSurfaceNode()) {
        // Mirror the original leash layout on shadow nodes to keep immediate rendering
        // and normal commits consistent.
        {
            AutoRSTransaction trans(shadowSurfaceNode);
            ApplyContentBounds(shadowSurfaceNode, rect);
        }
        {
            AutoRSTransaction trans(leashWinShadowSurfaceNode);
            ApplyWindowBounds(leashWinShadowSurfaceNode, rect, isGlobal);
        }
    } else {
        AutoRSTransaction trans(shadowSurfaceNode);
        ApplyWindowBounds(shadowSurfaceNode, rect, isGlobal);
    }
}

bool MoveDragBoundsApplier::ApplyWindowBounds(
    const std::shared_ptr<RSSurfaceNode>& node, const WSRect& rect, bool isGlobal)
{
    if (!node) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "surface node is null");
        return false;
    }
    node->SetGlobalPositionEnabled(isGlobal);
    node->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
    node->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    return true;
}

bool MoveDragBoundsApplier::ApplyContentBounds(
    const std::shared_ptr<RSSurfaceNode>& node, const WSRect& rect)
{
    if (!node) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "surface node is null");
        return false;
    }
    node->SetBounds(0.0f, 0.0f, rect.width_, rect.height_);
    node->SetFrame(0.0f, 0.0f, rect.width_, rect.height_);
    return true;
}
} // namespace OHOS::Rosen
