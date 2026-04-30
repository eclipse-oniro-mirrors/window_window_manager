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

const window = requireInternal('window');
const hilog = requireInternal('hilog');
const HILOG_DOMAIN = 0x04207;
const HILOG_TAG = 'WMSFocus';

interface UIEnvFocus {
  isFocused: boolean;
}

interface UIEnvHighlight {
  isHighlighted: boolean;
}

@ObservedV2
class WindowFocusedEnv implements IEnvironmentValue<UIEnvFocus> {
  @Trace isFocused: boolean = false;
  #win: window.Window;

  get value(): UIEnvFocus {
    return {
      isFocused: this.isFocused,
    };
  }

  constructor(context: UIContext) {
    try {
      this.#win = window.findWindow(context.getWindowName());
      this.#win.on('windowEvent', this.#focusChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] focus env constructor failed, ${error.message}`);
      this.isFocused = false;
    }
  }

  #focusChangeCallback = (windowEventType: window.WindowEventType): void => {
    this.isFocused = windowEventType === window.WindowEventType.WINDOW_ACTIVE;
    hilog.debug(HILOG_DOMAIN, HILOG_TAG, `[env] focusChangeCallback , ${this.isFocused}`);
  };

  update() : void {}

  destroy(): void {
    try {
      this.#win.off('windowEvent', this.#focusChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] focus env destroy failed, ${error.message}`);
    }
  }
}

@ObservedV2
class WindowHighlightedEnv implements IEnvironmentValue<UIEnvHighlight> {
  @Trace isHighlighted: boolean = false;
  #win: window.Window;

  get value() : UIEnvHighlight {
    return {
      isHighlighted: this.isHighlighted
    };
  }

  constructor(context: UIContext) {
    try {
      this.#win = window.findWindow(context.getWindowName());
      this.#win.on('windowHighlightChange', this.#highlightChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] highlight env constructor failed, ${error.message}`);
      this.isHighlighted = false;
    }
  }

  #highlightChangeCallback = (highlighted: boolean): void => {
    this.isHighlighted = highlighted;
    hilog.debug(HILOG_DOMAIN, HILOG_TAG, `[env] highlightChangeCallback , ${this.isHighlighted}`);
  };

  update() : void {}

  destroy(): void {
    try {
      this.#win.off('windowHighlightChange', this.#highlightChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] highlight env destroy failed, ${error.message}`);
    }
  }
}

export default {
  WindowFocusedEnv,
  WindowHighlightedEnv
}
