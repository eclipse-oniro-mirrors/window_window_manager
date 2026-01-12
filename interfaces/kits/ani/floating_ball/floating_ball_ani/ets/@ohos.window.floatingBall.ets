/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

import type image from '@ohos.multimedia.image';
import type Want from '@ohos.app.ability.Want';
import type {Callback} from '@ohos.base';
import {BusinessError} from '@ohos.base';
import hilog from '@ohos.hilog';
import BaseContext from 'application.BaseContext';

export type CallbackVoid<T> = (data: T) => void;
export type CallbackState = (state: int) => void;

export default namespace floatingBall {
  static { loadLibrary('fbwindowani_kit.z'); }
  
  export native function isFloatingBallEnabled(): boolean;

  export interface FloatingBallController {
    startFloatingBall(params: FloatingBallParams): Promise<void>;
    updateFloatingBall(params: FloatingBallParams): Promise<void>;
    stopFloatingBall(): Promise<void>;
    getFloatingBallWindowInfo(): Promise<FloatingBallWindowInfo>;
    restoreMainWindow(want: Want): Promise<void>;
    onClick(callback: Callback<void>): void;
    onStateChange(callback: Callback<FloatingBallState>): void;
    offClick(callback?: Callback<void>): void;
    offStateChange(callback?: Callback<FloatingBallState>): void;
  }

  export interface FloatingBallWindowInfo {
    windowId: int;
  }

  export class FloatingBallWindowInfoInner implements FloatingBallWindowInfo {
    windowId: int = 0;
    constructor(windowId: int) {this.windowId = windowId;}
  }

  export interface FloatingBallConfiguration {
      context: BaseContext;
    }

  export enum FloatingBallTemplate {
    STATIC = 1,
    NORMAL = 2,
    EMPHATIC = 3,
    SIMPLE = 4,
  }

  export interface FloatingBallParams {
    template: FloatingBallTemplate;
    title: string;
    content?: string;
    backgroundColor?: string;
    icon?: image.PixelMap;
  }

  export enum FloatingBallState {
    STARTED = 1,
    STOPPED = 2,
  }

  function runOnClickEvent(cb: object): void {
    const func = cb as () => void;
    func();
  }

  function runOnStateChangeEvent(cb: object, cbArg: int): void {
    const func = cb as (cbArg: FloatingBallState) => void;
    func(cbArg as FloatingBallState);
  }

  // nativeObj : the address of AniFbController
  let nativeObj: long = 0;
  export function setNativeObj(nativeObject: long): void {
    nativeObj = nativeObject;
  }

  export native function createSync(nativeObj: long, config: FloatingBallConfiguration): FloatingBallController;
  function create(config: FloatingBallConfiguration): Promise<FloatingBallController> {
    return new Promise<FloatingBallController>((resolve: (value: FloatingBallController) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): FloatingBallController => {
            return createSync(nativeObj, config);
        }).then((ret: Any) => {
            resolve(ret as FloatingBallController);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
  }

  export native function fbControllerFinalizerCallback(nativeObj: long): void;
  let fbControllerFinalizerRegister = new FinalizationRegistry<long>(fbControllerFinalizerCallback);

  export class FbControllerInternal implements FloatingBallController {
    private nativeObj: long = 0;
    public setNativeObj(nativeObject: long): void {
      if (this.nativeObj) {
        fbControllerFinalizerRegister.unregister(this);
      }
      this.nativeObj = nativeObject;
      fbControllerFinalizerRegister.register(this, this.nativeObj, this);
    }

    public native startFloatingBallNative(nativeObj: long, params: FloatingBallParams): void;
    public startFloatingBall(params: FloatingBallParams): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.startFloatingBallNative(this.nativeObj, params);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public native updateFloatingBallNative(nativeObj: long, params: FloatingBallParams): void;
    public updateFloatingBall(params: FloatingBallParams): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.updateFloatingBallNative(this.nativeObj, params);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public native stopFloatingBallNative(nativeObj: long): void;
    public stopFloatingBall(): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.stopFloatingBallNative(this.nativeObj);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    native getFloatingBallWindowInfoNative(nativeObj: long): FloatingBallWindowInfo;
    public getFloatingBallWindowInfo(): Promise<FloatingBallWindowInfo> {
        return new Promise<FloatingBallWindowInfo>((resolve: (value: FloatingBallWindowInfo) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): FloatingBallWindowInfo => {
                let res = this.getFloatingBallWindowInfoNative(this.nativeObj) as FloatingBallWindowInfo;
                return res;
            }).then((ret: Any) :void => {
                resolve(ret as FloatingBallWindowInfo);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public native restoreMainWindowNative(nativeObj: long, want: Want): void;
    public restoreMainWindow(want: Want): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.restoreMainWindowNative(this.nativeObj, want);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    native onClickNative(nativeObj: long, callback: Object): void;
    public onClick(callback: Callback<void>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onClickNative(this.nativeObj, callback);
    }

    native onStateChangeNative(nativeObj: long, callback: Object): void;
    public onStateChange(callback: Callback<FloatingBallState>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onStateChangeNative(this.nativeObj, callback);
    }

    native offClickNative(nativeObj: long, callback?: Object): void;
    public offClick(callback?: Callback<void>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offClickNative(this.nativeObj, callback);
    }

    native offStateChangeNative(nativeObj: long, callback?: Object): void;
    public offStateChange(callback?: Callback<FloatingBallState>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offStateChangeNative(this.nativeObj, callback);
    }
  }  // class FbControllerInternal
}  // namespace floatingBall