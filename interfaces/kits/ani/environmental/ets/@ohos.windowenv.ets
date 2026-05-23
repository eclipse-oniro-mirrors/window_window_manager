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

import hilog from '@ohos.hilog';
import window from '@ohos.window';
import uiExtension from '@ohos.arkui.uiExtension';

type Callback<T> = (value: T) => void;

const ATTR_DOMAIN = 0x04217;
const ATTR_TAG = 'WMSAttribute';
const FOCUS_DOMAIN = 0x04207;
const FOCUS_TAG = 'WMSFocus';

namespace env_native {
    export native function findWindowById(instanceId: long): Object;
    export native function getDisplayId(instanceId: long): long;
}

export class WindowStaticEnv {
    static { loadLibrary('window_static_env_module.z'); }

    private instanceId: long;
    private window: window.Window | undefined = undefined;
    private windowProxy: uiExtension.WindowProxy | undefined = undefined;
    private context: 
    private windowEventCb: Callback<window.WindowEventType> | undefined;

    constructor(instanceId: long) {
        this.instanceId = instanceId;
        try {
            const win: Object = env_native.findWindowById(instanceId);
            if (win === undefined) {
                hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] window is undefined, instanceId: ${instanceId}`);
            } else if (win instanceof window.Window) {
                this.window = win as window.Window;
            } else if (win instanceof uiExtension.WindowProxy) {
                this.windowProxy = win as uiExtension.WindowProxy;
            }
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] env constructor failed, ${error.message}`);
        }
    }

    registerDisplayIdEnv(cb: Callback<long>): void {
        try {
            cb(env_native.getDisplayId(this.instanceId));
            this.window?.onDisplayIdChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] DisplayIdEnv register failed, ${error.message}`);
        }
    }

    unregisterDisplayIdEnv(cb: Callback<long>): void {
        try {
            this.window?.offDisplayIdChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] DisplayIdEnv unregister failed, ${error.message}`);
        }
    }

    registerSystemDensityEnv(cb: Callback<double>): void {
        try {
            const windowDensityInfo: window.WindowDensityInfo = this.window!.getWindowDensityInfo();
            cb(windowDensityInfo.systemDensity);
            this.window?.onSystemDensityChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] SystemDensityEnv register failed, ${error.message}`);
        }
    }

    unregisterSystemDensityEnv(cb: Callback<double>): void {
        try {
            this.window?.offSystemDensityChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] SystemDensityEnv unregister failed, ${error.message}`);
        }
    }

    registerFocusEnv(cb: Callback<boolean>): void {
        try {
            if (!this.windowEventCb) {
                this.windowEventCb = (windowEventType: window.WindowEventType) => {
                    cb(windowEventType === window.WindowEventType.WINDOW_ACTIVE);
                }
                this.window?.onWindowEvent(this.windowEventCb!);
            }
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG, `[env] FocusEnv register failed, ${error.message}`);
        }
    }

    unregisterFocusEnv(cb: Callback<boolean>): void {
        try {
            if (this.windowEventCb) {
                this.window?.offWindowEvent(this.windowEventCb!);
                this.windowEventCb = undefined;
            }
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG, `[env] FocusEnv unregister failed, ${error.message}`);
        }
    }

    registerHighlightedEnv(cb: Callback<boolean>): void {
        try {
            this.window?.onWindowHighlightChange(cb);
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG, `[env] HighlightedEnv register failed, ${error.message}`);
        }
    }

    unregisterHighlightedEnv(cb: Callback<boolean>): void {
        try {
            this.window?.offWindowHighlightChange(cb);
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG , `[env] HighlightedEnv unregister failed, ${error.message}`);
        }
    }
}