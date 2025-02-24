/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// WARNING! THIS FILE IS AUTO-GENERATED, DO NOT MAKE CHANGES, THEY WILL BE LOST ON NEXT GENERATION!

export enum display_FoldDisplayMode {
    FOLD_DISPLAY_MODE_UNKNOWN = 0,
    FOLD_DISPLAY_MODE_FULL = 1,
    FOLD_DISPLAY_MODE_MAIN = 2,
    FOLD_DISPLAY_MODE_SUB = 3,
    FOLD_DISPLAY_MODE_COORDINATION = 4
}
export enum display_FoldStatus {
    FOLD_STATUS_UNKNOWN = 0,
    FOLD_STATUS_EXPANDED = 1,
    FOLD_STATUS_FOLDED = 2,
    FOLD_STATUS_HALF_FOLDED = 3
}
export namespace display {
    export interface FoldCreaseRegion {
        readonly displayId: number
        readonly creaseRects: Array<display.Rect>
    }
}
export namespace display {
    export interface Rect {
        left: number
        top: number
        width: number
        height: number
    }
}
export namespace display {
    export interface Display {
    }
}
