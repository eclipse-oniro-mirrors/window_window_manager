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

declare function requireInternal(k: string): any;

type ConstructorV2 = {
  new (...args: any[]): any;
};

declare function ObservedV2<T extends ConstructorV2>(BaseClass: T): T;

interface IEnvironmentValue<T> {
  value: T;
  update(newValue: T): void;
  destroy(): void;
}

declare const Trace: (target: Object, propertyKey: string) => void;

interface Callback<T> {
  (data: T): void;
}

declare namespace window {
  interface Size {
    width: number;
    height: number;
  }

  interface SizeInVp {
    widthVp: number;
    heightVp: number;
  }

  interface Rect {
    left: number;
    top: number;
    width: number;
    height: number;
  }

  interface AvoidArea {
    visible: boolean;
    leftRect: Rect;
    topRect: Rect;
    rightRect: Rect;
    bottomRect: Rect;
  }

  interface AvoidAreaOptions {
    type: number;
    area: AvoidArea;
  }

  interface Window {
    getWindowProperties(): { windowRect: Size };
    getWindowAvoidArea(type: number): AvoidArea;
    on(type: 'windowSizeChange', callback: Callback<Size>): void;
    off(type: 'windowSizeChange', callback?: Callback<Size>): void;
    on(type: 'avoidAreaChange', callback: Callback<AvoidAreaOptions>): void;
    off(type: 'avoidAreaChange', callback?: Callback<AvoidAreaOptions>): void;
  }
}

declare class UIContext {
  getWindowName(): string;
  getUIObserver(): UIObserver;
  px2vp(value: number): number;
}

declare namespace uiObserver {
  class DensityInfo {
    context: UIContext;
    density: number;
  }
}

declare class UIObserver {
  on(type: 'densityUpdate', callback: Callback<uiObserver.DensityInfo>): void;
  off(type: 'densityUpdate', callback?: Callback<uiObserver.DensityInfo>): void;
}