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

#ifndef FOUNDATION_DMSERVER_ABSTRACT_SCREEN_H
#define FOUNDATION_DMSERVER_ABSTRACT_SCREEN_H


#include <vector>
#include <map>
#include <refbase.h>
#include <screen_manager/screen_types.h>
#include <ui/rs_display_node.h>
#include <ui/rs_surface_node.h>
#include <ui/rs_ui_context.h>
#include <ui/rs_ui_director.h>

#include "noncopyable.h"
#include "screen.h"
#include "screen_group.h"
#include "screen_group_info.h"
#include "screen_info.h"

namespace OHOS::Rosen {
class AbstractScreenGroup;
class AbstractScreenController;
class AbstractScreen : public RefBase {
public:
    AbstractScreen(sptr<AbstractScreenController>, const std::string& name, ScreenId dmsId, ScreenId rsId);
    AbstractScreen() = delete;
    WM_DISALLOW_COPY_AND_MOVE(AbstractScreen);
    ~AbstractScreen();
    sptr<SupportedScreenModes> GetActiveScreenMode() const;
    std::vector<sptr<SupportedScreenModes>> GetAbstractScreenModes() const;
    sptr<AbstractScreenGroup> GetGroup() const;
    sptr<ScreenInfo> ConvertToScreenInfo() const;
    bool SetOrientation(Orientation orientation);
    Rotation CalcRotation(Orientation orientation) const;
    bool SetVirtualPixelRatio(float virtualPixelRatio);
    float GetVirtualPixelRatio() const;
    bool SetSourceMode(ScreenSourceMode sourceMode);
    ScreenSourceMode GetSourceMode() const;

    void UpdateRSTree(std::shared_ptr<RSSurfaceNode>& surfaceNode, bool isAdd, bool needToUpdate = true);
    DMError AddSurfaceNode(std::shared_ptr<RSSurfaceNode>& surfaceNode, bool onTop, bool needToRecord = true);
    DMError RemoveSurfaceNode(std::shared_ptr<RSSurfaceNode>& surfaceNode);
    void UpdateDisplayGroupRSTree(
        std::shared_ptr<RSSurfaceNode>& surfaceNode, std::shared_ptr<RSDisplayNode>& parentNode, bool isAdd);
    void InitRSDisplayNode(const RSDisplayNodeConfig& config, const Point& startPoint);
    void InitRSDefaultDisplayNode(const RSDisplayNodeConfig& config, const Point& startPoint);
    void UpdateRSDisplayNode(Point startPoint, const sptr<AbstractScreen>& absScreen);
    ScreenId GetScreenGroupId() const;

    // colorspace, gamut
    DMError GetScreenSupportedColorGamuts(std::vector<ScreenColorGamut>& colorGamuts);
    DMError GetScreenColorGamut(ScreenColorGamut& colorGamut);
    DMError SetScreenColorGamut(int32_t colorGamutIdx);
    DMError GetScreenGamutMap(ScreenGamutMap& gamutMap);
    DMError SetScreenGamutMap(ScreenGamutMap gamutMap);
    DMError SetScreenColorTransform();
    const std::string& GetScreenName() const;
    void SetPhyWidth(uint32_t phyWidth);
    void SetPhyHeight(uint32_t phyHeight);
    uint32_t GetPhyWidth() const;
    uint32_t GetPhyHeight() const;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIContext> GetRSUIContext() const;

    const ScreenId dmsId_;
    const ScreenId rsId_;
    bool isScreenGroup_ { false };
    std::shared_ptr<RSDisplayNode> rsDisplayNode_;
    RSDisplayNodeConfig rSDisplayNodeConfig_;
    Point startPoint_{};
    ScreenId groupDmsId_ { SCREEN_ID_INVALID };
    ScreenId lastGroupDmsId_ { SCREEN_ID_INVALID };
    ScreenType type_ { ScreenType::REAL };
    int32_t activeIdx_ { 0 };
    std::vector<sptr<SupportedScreenModes>> modes_ = {};
    float virtualPixelRatio_ = { 1.0 };
    Orientation orientation_ { Orientation::UNSPECIFIED };
    Rotation rotation_ { Rotation::ROTATION_0 };
    Orientation screenRequestedOrientation_ { Orientation::UNSPECIFIED };
    // nativeSurfaceNodes_ means th node which is added/removed by interface of dms directly
    std::vector<std::shared_ptr<RSSurfaceNode>> nativeSurfaceNodes_;
    // appSurfaceNodes_ means th node which is added/removed by interface of wms
    std::vector<std::shared_ptr<RSSurfaceNode>> appSurfaceNodes_;

protected:
    void FillScreenInfo(sptr<ScreenInfo>) const;
    const sptr<AbstractScreenController> screenController_;

private:
    void SetPropertyForDisplayNode(const std::shared_ptr<RSDisplayNode>& rsDisplayNode,
        const RSDisplayNodeConfig& config, const Point& startPoint);
    std::string name_ { "UNKNOWN" };
    uint32_t phyWidth_ { UINT32_MAX };
    uint32_t phyHeight_ { UINT32_MAX };
    mutable std::recursive_mutex mutex_;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> rsUIDirector_;
};

class AbstractScreenGroup : public AbstractScreen {
public:
    AbstractScreenGroup(sptr<AbstractScreenController>, ScreenId dmsId, ScreenId rsId, std::string name,
        ScreenCombination combination);
    AbstractScreenGroup() = delete;
    WM_DISALLOW_COPY_AND_MOVE(AbstractScreenGroup);
    ~AbstractScreenGroup();

    bool AddChild(sptr<AbstractScreen>& dmsScreen, Point& startPoint);
    bool AddChildren(std::vector<sptr<AbstractScreen>>& dmsScreens, std::vector<Point>& startPoints);
    bool RemoveChild(sptr<AbstractScreen>& dmsScreen);
    bool RemoveDefaultScreen(const sptr<AbstractScreen>& dmsScreen);
    bool HasChild(ScreenId childScreen) const;
    std::vector<sptr<AbstractScreen>> GetChildren() const;
    std::vector<Point> GetChildrenPosition() const;
    Point GetChildPosition(ScreenId screenId) const;
    size_t GetChildCount() const;
    sptr<ScreenGroupInfo> ConvertToScreenGroupInfo() const;
    ScreenCombination GetScreenCombination() const;

    ScreenCombination combination_ { ScreenCombination::SCREEN_ALONE };
    ScreenId mirrorScreenId_ { SCREEN_ID_INVALID };
    ScreenId defaultScreenId_ { SCREEN_ID_INVALID };

private:
    bool GetRSDisplayNodeConfig(sptr<AbstractScreen>& dmsScreen, struct RSDisplayNodeConfig& config);

    std::map<ScreenId, sptr<AbstractScreen>> screenMap_;
    mutable std::mutex screenMapMutex_;
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DMSERVER_ABSTRACT_SCREEN_H
