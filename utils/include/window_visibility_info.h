/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_VISIBILITY_INFO_H
#define OHOS_ROSEN_WINDOW_VISIBILITY_INFO_H

#include "wm_common.h"

namespace OHOS::Rosen {
/**
 * @enum WindowLayerState
 *
 * @brief Layer state of a window
 */
enum WindowLayerState {
    WINDOW_ALL_VISIBLE = 0,
    WINDOW_SEMI_VISIBLE,
    WINDOW_IN_VISIBLE,
    WINDOW_LAYER_DRAWING,
    WINDOW_LAYER_NO_DRAWING,
    WINDOW_LAYER_UNKNOWN_STATE,
};

/**
 * @class WindowVisibilityInfo
 *
 * @brief Visibility info of window.
 */
class WindowVisibilityInfo : public Parcelable {
public:
    /**
     * @brief Default construct of WindowVisibilityInfo.
     */
    WindowVisibilityInfo() = default;

    /**
     * @brief Construct of WindowVisibilityInfo.
     *
     * @param winId Window id.
     * @param pid Process id.
     * @param uid User id.
     * @param visibility True means window is visible, false means the opposite.
     * @param winType Type of window.
     */
    WindowVisibilityInfo(uint32_t winId, int32_t pid, int32_t uid, WindowVisibilityState visibilityState,
        WindowType winType) : windowId_(winId), pid_(pid), uid_(uid), visibilityState_(visibilityState),
        windowType_(winType) {};

    WindowVisibilityInfo(uint32_t winId, int32_t pid, int32_t uid, WindowVisibilityState visibilityState,
        WindowType winType, WindowStatus windowStatus, const Rect& rect, const std::string& bundleName,
        const std::string& abilityName, bool isFocused) : windowId_(winId), pid_(pid), uid_(uid),
        visibilityState_(visibilityState), windowType_(winType), windowStatus_(windowStatus), rect_(rect),
        bundleName_(bundleName), abilityName_(abilityName), isFocused_(isFocused) {}

    /**
     * @brief Deconstruct of WindowVisibilityInfo.
     */
    ~WindowVisibilityInfo() = default;

    /**
     * @brief Marshalling WindowVisibilityInfo.
     *
     * @param parcel Package of WindowVisibilityInfo.
     * @return True means marshall success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel& parcel) const override;

    /**
     * @brief Unmarshalling WindowVisibilityInfo.
     *
     * @param parcel Package of WindowVisibilityInfo.
     * @return WindowVisibilityInfo object.
     */
    static WindowVisibilityInfo* Unmarshalling(Parcel& parcel);

    uint32_t GetWindowId() const { return windowId_; }

    const Rect& GetRect() const { return rect_; }

    const std::string& GetBundleName() const { return bundleName_; }

    void SetBundleName(const std::string& bundleName) { bundleName_ = bundleName; }

    const std::string& GetAbilityName() const { return abilityName_; }

    void SetAbilityName(const std::string& abilityName) { abilityName_ = abilityName; }

    WindowStatus GetWindowStatus() const { return windowStatus_; }

    WindowType GetWindowType() const { return windowType_; }

    WindowVisibilityState GetWindowVisibilityState() const { return visibilityState_; }

    bool IsFocused() const { return isFocused_; }

    int32_t GetAppIndex() const { return appIndex_; }

    void SetAppIndex(int32_t appIndex) { appIndex_ = appIndex; }

    bool IsSystem() const { return isSystem_; }

    void SetIsSystem(bool isSystem) { isSystem_ = isSystem; }

    uint32_t GetZOrder() const { return zOrder_; }

    void SetZOrder(uint32_t zOrder) { zOrder_ = zOrder; }

    void SetGlobalDisplayRect(const Rect& globalDisplayRect) { globalDisplayRect_ = globalDisplayRect; }

    Rect GetGlobalDisplayRect() const { return globalDisplayRect_; }

    int32_t GetCallingPid() const { return callingPid_; }

    void SetCallingPid(int32_t callingPid) { callingPid_ = callingPid; }

    int32_t GetCollaboratorType() const { return collaboratorType_; }

    void SetCollaboratorType(int32_t collaboratorType_) { collaboratorType_ = collaboratorType; }

    uint32_t windowId_ { INVALID_WINDOW_ID };
    int32_t pid_ { 0 };
    int32_t uid_ { 0 };
    WindowVisibilityState visibilityState_ = WINDOW_LAYER_STATE_MAX;
    WindowType windowType_ { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW };
    WindowStatus windowStatus_ = WindowStatus::WINDOW_STATUS_UNDEFINED;
    Rect rect_ = {0, 0, 0, 0};
    Rect globalDisplayRect_ { 0, 0, 0, 0 };
    std::string bundleName_;
    std::string abilityName_;
    bool isFocused_ = false;
    int32_t appIndex_ { 0 };
    bool isSystem_ = false;
    uint32_t zOrder_ { 0 };
    int32_t callingPid_ { -1 };
    int32_t collaboratorType_ = 0;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_VISIBILITY_INFO_H