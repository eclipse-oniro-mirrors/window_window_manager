/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base';
import hilog from '@ohos.hilog';
import window from '@ohos.window';
import { Callback } from '@ohos.base';

/**
 * uiExtensionHost.
 *
 * @namespace uiExtensionHost
 * @syscap SystemCapability.ArkUI.ArkUI.Full
 * @systemapi
 * @since 11
 */
namespace uiExtensionHost {
  export native function createExtensionWindow(win: long, hostId: int): UIExtensionHostWindowProxy;

  // UIExtension WmsLogTag
  const WMS_UIEXT: int = 0xC0420D;
  const WMS_UIEXT_TAG: string = "WMSUiext";
  const WINDOW_SIZE_CHANGE_CB: string = "windowSizeChange";
  const AVOID_AREA_CHANGE_CB: string = "avoidAreaChange";
  const enum WmErrorCode {
    WM_OK = 0,
    WM_ERROR_INVALID_PARAM = 401
  }

  // internal class and function
  export type CallbackSize = (size: window.Size) => void;
  export type CallbackArea = (area: window.AvoidAreaOptions) => void;

  function sizeChangeCallback(fn: object, fnArg: object): void {
    let f = fn as (d: object) => void;
    f(fnArg);
  }

  function keyboardHeightChangeCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: number) => void;
    func(cbArg as number);
  }

  class UIERect implements window.Rect {
    left: int;
    top: int;
    width: int;
    height: int;
  }

  class UIEAvoidArea implements window.AvoidArea {
    constructor() {
      this.leftRect = new UIERect();
      this.topRect = new UIERect();
      this.rightRect = new UIERect();
      this.bottomRect = new UIERect();
    }

    visible: boolean;
    leftRect: window.Rect;
    topRect: window.Rect;
    rightRect: window.Rect;
    bottomRect: window.Rect;
  }

  class UIEProperties implements UIExtensionHostWindowProxyProperties {
    constructor() {
      this.uiExtensionHostWindowProxyRect = new UIERect();
    }

    uiExtensionHostWindowProxyRect: window.Rect;
  }

  export native function destroyWindowProxy(nativeObj: long): void;
  let windowProxyRegister = new FinalizationRegistry<long>(destroyWindowProxy);

  // API implementation
  export class UIExtensionHostInternal implements UIExtensionHostWindowProxy {
    static { loadLibrary("embeddablewindowstageani_module.z") }
    private nativeObj: long = 0;

    private setNativeObj(nativeObj: long): void {
      this.nativeObj = nativeObj;
      windowProxyRegister.register(this, nativeObj);
    }

    private setAreaVisible(obj: window.AvoidArea, visible: boolean) {
      obj.visible = visible;
    }

    public native getProperties(nativeObj: long, property: object): int;
    public native setProperties(nativeObj: long, property: object): int;
    public native getWindowAvoidArea(nativeObj: long, type: int, area: object): int;
    public native setWaterMarkFlag(nativeObj: long, enable: boolean): int;
    public native hidePrivacyContentForHost(nativeObj: long, shouldHide: boolean): int;
    public native onSync(nativeObj: long, type: string, callback: object): void;
    public native offSync(nativeObj: long, type: string, callback?: object): void;
    public native createSubWindowWithOptions(nativeObj: long, name: string,
      subWindowOptions: window.SubWindowOptions, followCreatorLifecycle: boolean): window.Window;
    public native hideNonSecureWindows(nativeObj: long, shouldHide: boolean): int;

    get properties(): UIExtensionHostWindowProxyProperties {
      let property = new UIEProperties();
      let ret = this.getProperties(this.nativeObj, property as object);
      if (ret !== WmErrorCode.WM_OK) {
        hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `getProperties failed, ret: ${ret}`);
      }
      return property;
    }

    set properties(properties: UIExtensionHostWindowProxyProperties) {
      this.setProperties(this.nativeObj, properties as object);
    }

    public on<T>(type: string, callback: Callback<T>): void {
      this.onSync(this.nativeObj, type, callback);
    }

    public off<T>(type: string, callback?: Callback<T>): void {
      this.offSync(this.nativeObj, type, callback);
    }

    public onAvoidAreaChange<T>(callback: Callback<T>): void {
      this.onSync(this.nativeObj, AVOID_AREA_CHANGE_CB, callback);
    }

    public offAvoidAreaChange<T>(callback?: Callback<T>): void {
      this.offSync(this.nativeObj, AVOID_AREA_CHANGE_CB, callback);
    }

    public onWindowSizeChange(callback: Callback<window.Size>): void {
      this.onSync(this.nativeObj, WINDOW_SIZE_CHANGE_CB, callback);
    }

    public offWindowSizeChange(callback?: Callback<window.Size>): void {
      this.offSync(this.nativeObj, WINDOW_SIZE_CHANGE_CB, callback);
    }

    public getWindowAvoidArea(type: window.AvoidAreaType): window.AvoidArea {
      let area: window.AvoidArea = new UIEAvoidArea();
      let ret = this.getWindowAvoidArea(this.nativeObj, type as int, area as object);
      if (ret !== WmErrorCode.WM_OK) {
        hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `getWindowAvoidArea failed, ret: ${ret}`);
      }
      return area;
    }

    public setWaterMarkFlag(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          let ret = this.setWaterMarkFlag(this.nativeObj, enable);
          if (ret !== WmErrorCode.WM_OK) {
            hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `setWaterMarkFlag failed, ret: ${ret}`);
          }
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    public hidePrivacyContentForHost(shouldHide: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          let ret = this.hidePrivacyContentForHost(this.nativeObj, shouldHide);
          if (ret !== WmErrorCode.WM_OK) {
            hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `hidePrivacyContentForHost failed, ret: ${ret}`);
          }
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    public hideNonSecureWindows(shouldHide: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          let ret = this.hideNonSecureWindows(this.nativeObj, shouldHide);
          if (ret !== WmErrorCode.WM_OK) {
            hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `hideNonSecureWindows failed, ret: ${ret}`);
          }
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public createSubWindowWithOptions(name: string, subWindowOptions: window.SubWindowOptions): Promise<window.Window> {
      return new Promise<window.Window>((resolve: (value: window.Window) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): window.Window => {
          return this.createSubWindowWithOptions(this.nativeObj, name, subWindowOptions, false);
        }).then((ret: Any) => {
            resolve(ret as window.Window);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
      });
    }

    public createSubWindowWithOptions(name: string, subWindowConfig: window.SubWindowOptions,
      followCreatorLifecycle: boolean): Promise<window.Window> {
      return new Promise<window.Window>((resolve: (value: window.Window) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): window.Window => {
          return this.createSubWindowWithOptions(this.nativeObj, name, subWindowConfig, followCreatorLifecycle);
        }).then((ret: Any) => {
            resolve(ret as window.Window);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
      });
    }
  }

  /**
   * Transition Controller
   *
   * @interface UIExtensionHostWindowProxy
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @systemapi
   * @since 11
   */
  export interface UIExtensionHostWindowProxy {
    getWindowAvoidArea(type: window.AvoidAreaType): window.AvoidArea;
    setWaterMarkFlag(enable: boolean): Promise<void>;
    hidePrivacyContentForHost(shouldHide: boolean): Promise<void>;
    hideNonSecureWindows(shouldHide: boolean): Promise<void>;
    createSubWindowWithOptions(name: string, subWindowOptions: window.SubWindowOptions): Promise<window.Window>;
    createSubWindowWithOptions(name: string, subWindowConfig: window.SubWindowOptions,
      followCreatorLifecycle: boolean): Promise<window.Window>;
    on<T>(type: string, callback: Callback<T>): void;
    off<T>(type: string, callback?: Callback<T>): void;
    onAvoidAreaChange<T>(callback: Callback<T>): void;
    offAvoidAreaChange<T>(callback?: Callback<T>): void;
    onWindowSizeChange(callback: Callback<window.Size>): void;
    offWindowSizeChange(callback?: Callback<window.Size>): void;

    properties: UIExtensionHostWindowProxyProperties;
  }

  /**
   * Properties of UIExtension window
   *
   * @interface UIExtensionHostWindowProxyProperties
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @systemapi
   * @since 11
   */
  interface UIExtensionHostWindowProxyProperties {
    /**
     * The position and size of the UIExtension window
     *
     * @type { window.Rect }
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @systemapi
     * @since 11
     */
    uiExtensionHostWindowProxyRect: window.Rect;
  }
}

export default uiExtensionHost;