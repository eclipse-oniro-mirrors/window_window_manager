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

const windowenv = requireInternal('windowenv');
const hilog = requireInternal('hilog');
const HILOG_DOMAIN = 0x04217;
const HILOG_TAG = 'WMSAttribute';

@ObservedV2
class DisplayIdEnv implements IEnvironmentValue<window.DisplayId> {
  @Trace public displayId: number = -1;
  #win: window.Window | uiExtension.WindowProxy;

  get value(): window.DisplayId {
    return this;
  }

  constructor(context: UIContext) {
    try {
      this.#win = windowenv.findWindowById(context.getId());
      this.displayId = windowenv.getDisplayId(context.getId());
      this.#win.on('displayIdChange', this.#displayIdChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] displayId env constructor failed, ${error.message}`);
    }
  }

  #displayIdChangeCallback = (displayId: number): void => {
    this.displayId = displayId;
  };

  update(): void {}

  destroy(): void {
    try {
      this.#win.off('displayIdChange', this.#displayIdChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] px env destroy failed, ${error.message}`);
    }
  }
}

export default {
  DisplayIdEnv
}
