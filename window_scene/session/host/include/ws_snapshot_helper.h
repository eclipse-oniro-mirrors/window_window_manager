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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SNAPSHOT_HELPER_H
#define OHOS_ROSEN_WINDOW_SCENE_SNAPSHOT_HELPER_H

#include <string>

#include <dm_common.h>

namespace OHOS::Rosen {
namespace {
constexpr int32_t SCREEN_UNKNOWN = 0;
constexpr int32_t SCREEN_EXPAND = 1;
constexpr int32_t SCREEN_FOLDED = 2;
constexpr int32_t SCREEN_COUNT = 3;
constexpr uint32_t SNAPSHOT_PORTRAIT = 0;
constexpr uint32_t SNAPSHOT_LANDSCAPE = 1;
constexpr uint32_t ORIENTATION_COUNT = 2;
constexpr uint32_t PORTRAIT_ANGLE = 0;
constexpr uint32_t LANDSCAPE_ANGLE = 90;
constexpr uint32_t PORTRAIT_INVERTED_ANGLE = 180;
constexpr uint32_t LANDSCAPE_INVERTED_ANGLE = 270;
constexpr uint32_t ROTATION_ANGLE = 360;
constexpr uint32_t ROTATION_COUNT = 4;
constexpr uint32_t SECONDARY_EXPAND_OFFSET = 1;
}
using SnapshotStatus = int32_t;
constexpr SnapshotStatus defaultStatus = SCREEN_UNKNOWN;
constexpr SnapshotStatus defaultCapacity = SCREEN_EXPAND;
constexpr SnapshotStatus maxCapacity = SCREEN_COUNT;

class WSSnapshotHelper {
public:
    static WSSnapshotHelper* GetInstance();
    int32_t GetScreenStatus();
    static int32_t GetScreenStatus(FoldStatus foldStatus);
    static DisplayOrientation GetDisplayOrientation(int32_t rotation);
    void SetWindowScreenStatus(int32_t screenStatus);
    void SetWindowScreenStatus(FoldStatus foldStatus);
    void SetWindowOrientationStatus(Rotation rotation);
    uint32_t GetWindowRotation() const;

private:
    WSSnapshotHelper() = default;
    ~WSSnapshotHelper() = default;
    SnapshotStatus screenStatus_;
    Rotation screenRotation_;
    mutable std::mutex statusMutex_;
    mutable std::mutex rotationMutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SNAPSHOT_HELPER_H