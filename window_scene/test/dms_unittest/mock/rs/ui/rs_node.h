/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef RENDER_SERVICE_CLIENT_CORE_UI_RS_NODE_H
#define RENDER_SERVICE_CLIENT_CORE_UI_RS_NODE_H

#include <optional>
#include <unordered_map>
#include "common/rs_rect.h"

namespace OHOS {
namespace Rosen {
class RSUIContext;
class RSAnimationTimingProtocol {
public:
    RSAnimationTimingProtocol() = default;
    RSAnimationTimingProtocol(int duration) : duration_(duration) {}
    virtual ~RSAnimationTimingProtocol() = default;

    void SetDuration(int duration)
    {
        duration_ = duration;
    }

protected:
    int duration_{300};
};

class RSAnimationTimingCurve final {
public:
    static const RSAnimationTimingCurve DEFAULT;
    static const RSAnimationTimingCurve LINEAR;

    RSAnimationTimingCurve() = default;
    RSAnimationTimingCurve(const RSAnimationTimingCurve& timingCurve) = default;
};

using PropertyCallback = std::function<void()>;
using ExportTypeChangedCallback = std::function<void(bool)>;
using PropertyNodeChangeCallback = std::function<void()>;

class RSNode : public std::enable_shared_from_this<RSNode> {
public:
    virtual ~RSNode() {}
    void RemoveFromTree() {}
    uint64_t GetId() const
    {
        return 0;
    }
    static void OpenImplicitAnimation(const RSAnimationTimingProtocol& timingProtocol,
                                      const RSAnimationTimingCurve& timingCurve,
                                      const std::function<void()>& finishCallback = nullptr);
    virtual void SetBounds(const Vector4f& bounds)
    {
        return;
    }
    virtual void SetBounds(float positionX, float positionY, float width, float height) {}
    virtual void SetFrame(const Vector4f& frame)
    {
        frame_ = frame;
    }
    virtual void SetFrame(float positionX, float positionY, float width, float height)
    {
        frame_ = {positionX, positionY, width, height};
    }
    void SetPositionZ(float positionZ) {}
    void SetRotation(float degreeX, float degreeY, float degreeZ) {}
    void SetRotation(float degree) {}
    void SetTranslate(const Vector2f& translate) {}
    void SetTranslate(float translateX, float translateY, float translateZ) {}
    std::shared_ptr<RSUIContext> GetRSUIContext() const
    {
        return nullptr;
    }
    void SetRSUIContext(std::shared_ptr<RSUIContext> rsUIContext) {}
    void SetSkipCheckInMultiInstance(bool isSkipCheckInMultiInstance) {}
    void SetHDRBrightnessFactor(float factor) {}
    void SetScale(float scaleX, float scaleY) {}
    void SetPivot(float pivotX, float pivotY) {}

    Vector4f frame_;

    static void OpenImplicitAnimation(const std::shared_ptr<RSUIContext> rsUIContext,
                                      const RSAnimationTimingProtocol& timingProtocol,
                                      const RSAnimationTimingCurve& timingCurve,
                                      const std::function<void()>& finishCallback = nullptr)
    {
    }
    static void CloseImplicitAnimation(const std::shared_ptr<RSUIContext> rsUIContext) {}
};

using RSBaseNode = RSNode;

}  // namespace Rosen
}  // namespace OHOS

/** @} */
#endif  // RENDER_SERVICE_CLIENT_CORE_UI_RS_NODE_H
