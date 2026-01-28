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

const window = requireInternal('window');
const hilog = requireInternal('hilog');
const HILOG_DOMAIN = 0x04209;
const HILOG_TAG = 'WMSImms';

interface UIEnvAvoidArea {
  statusBar: window.AvoidArea;
  cutout: window.AvoidArea;
  keyboard: window.AvoidArea;
  navigationIndicator: window.AvoidArea;
}

enum AvoidAreaType {
  TYPE_SYSTEM = 0,
  TYPE_CUTOUT = 1,
  TYPE_SYSTEM_GESTURE = 2,
  TYPE_KEYBOARD = 3,
  TYPE_NAVIGATION_INDICATOR = 4,
}

function createEmptyRect(): window.Rect {
  return { left: 0, top: 0, width: 0, height: 0 };
}

function createEmptyAvoidArea(): window.AvoidArea {
  return {
    visible: false,
    leftRect: createEmptyRect(),
    topRect: createEmptyRect(),
    rightRect: createEmptyRect(),
    bottomRect: createEmptyRect(),
  }
}

@ObservedV2
class WindowAvoidAreaPxEnv implements IEnvironmentValue<UIEnvAvoidArea> {
  @Trace statusBar: window.AvoidArea;
  @Trace cutout: window.AvoidArea;
  @Trace keyboard: window.AvoidArea;
  @Trace navigationIndicator: window.AvoidArea;

  #win: window.Window;
  
  get value() {
    return this;
  }

  constructor(context: UIContext) {
    try {
      this.#win = window.findWindow(context.getWindowName());
      this.statusBar = this.#win.getWindowAvoidArea(AvoidAreaType.TYPE_SYSTEM);
      this.cutout = this.#win.getWindowAvoidArea(AvoidAreaType.TYPE_CUTOUT);
      this.keyboard = this.#win.getWindowAvoidArea(AvoidAreaType.TYPE_KEYBOARD);
      this.navigationIndicator = this.#win.getWindowAvoidArea(AvoidAreaType.TYPE_NAVIGATION_INDICATOR);
      this.#win.on('avoidAreaChange', this.#avoidAreaChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] px env constructor failed, ${error.message}`);
    }
  }

  #avoidAreaChangeCallback = ({ type, area }: window.AvoidAreaOptions) => {
    switch (type) {
      case AvoidAreaType.TYPE_SYSTEM:
        this.statusBar = area;
        break;
      case AvoidAreaType.TYPE_CUTOUT:
        this.cutout = area;
        break;
      case AvoidAreaType.TYPE_KEYBOARD:
        this.keyboard = area;
        break;
      case AvoidAreaType.TYPE_NAVIGATION_INDICATOR:
        this.navigationIndicator = area;
        break;
    }
  };

  update() {}

  destroy(): void {
    try {
      this.#win.off('avoidAreaChange', this.#avoidAreaChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] px env destroy failed, ${error.message}`);
    }
  }
}

@ObservedV2
class WindowAvoidAreaVpEnv implements IEnvironmentValue<UIEnvAvoidArea> {
  @Trace statusBar: window.AvoidArea;
  @Trace cutout: window.AvoidArea;
  @Trace keyboard: window.AvoidArea;
  @Trace navigationIndicator: window.AvoidArea;

  #win: window.Window;
  #context: UIContext;
  #areaInPx: Map<AvoidAreaType, window.AvoidArea> = new Map();
  
  get value() {
    return this;
  }

  constructor(context: UIContext) {
    try {
      this.#context = context;
      this.#win = window.findWindow(this.#context.getWindowName());
      for (const type of [AvoidAreaType.TYPE_SYSTEM, AvoidAreaType.TYPE_CUTOUT, AvoidAreaType.TYPE_KEYBOARD, AvoidAreaType.TYPE_NAVIGATION_INDICATOR]) {
        this.#areaInPx.set(type, this.#win.getWindowAvoidArea(type));
      }
      this.update();
      this.#win.on('avoidAreaChange', this.#avoidAreaChangeCallback);
      this.#context.getUIObserver().on('densityUpdate', this.#densityUpdateCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] vp env constructor failed, ${error.message}`);
    }
  }

  #translateRectToVp(rect: window.Rect): window.Rect {
    const vpRect = createEmptyRect();
    vpRect.left = this.#context?.px2vp(rect.left) ?? rect.left;
    vpRect.top = this.#context?.px2vp(rect.top) ?? rect.top;
    vpRect.width = this.#context?.px2vp(rect.width) ?? rect.width;
    vpRect.height = this.#context?.px2vp(rect.height) ?? rect.height;
    return vpRect;
  }

  #translateAvoidAreaToVp(area: window.AvoidArea): window.AvoidArea {
    const vpArea = createEmptyAvoidArea();
    vpArea.visible = area.visible;
    vpArea.leftRect = this.#translateRectToVp(area.leftRect);
    vpArea.topRect = this.#translateRectToVp(area.topRect);
    vpArea.rightRect = this.#translateRectToVp(area.rightRect);
    vpArea.bottomRect = this.#translateRectToVp(area.bottomRect);
    return vpArea;
  }

  #avoidAreaChangeCallback = ({ type, area }: window.AvoidAreaOptions) => {
    this.#areaInPx.set(type, area);
    switch (type) {
      case AvoidAreaType.TYPE_SYSTEM:
        this.statusBar = this.#translateAvoidAreaToVp(area);
        break;
      case AvoidAreaType.TYPE_CUTOUT:
        this.cutout = this.#translateAvoidAreaToVp(area);
        break;
      case AvoidAreaType.TYPE_KEYBOARD:
        this.keyboard = this.#translateAvoidAreaToVp(area);
        break;
      case AvoidAreaType.TYPE_NAVIGATION_INDICATOR:
        this.navigationIndicator = this.#translateAvoidAreaToVp(area);
        break;
    }
  };

  #densityUpdateCallback = () => {
    this.update();
  }

  update() {
    if (!this.#context) {
      return;
    }

    if (this.#areaInPx.has(AvoidAreaType.TYPE_SYSTEM)) {
      this.statusBar = this.#translateAvoidAreaToVp(this.#areaInPx.get(AvoidAreaType.TYPE_SYSTEM));
    }
    if (this.#areaInPx.has(AvoidAreaType.TYPE_CUTOUT)) {
      this.cutout = this.#translateAvoidAreaToVp(this.#areaInPx.get(AvoidAreaType.TYPE_CUTOUT));
    }
    if (this.#areaInPx.has(AvoidAreaType.TYPE_KEYBOARD)) {
      this.keyboard = this.#translateAvoidAreaToVp(this.#areaInPx.get(AvoidAreaType.TYPE_KEYBOARD));
    }
    if (this.#areaInPx.has(AvoidAreaType.TYPE_NAVIGATION_INDICATOR)) {
      this.navigationIndicator = this.#translateAvoidAreaToVp(
        this.#areaInPx.get(AvoidAreaType.TYPE_NAVIGATION_INDICATOR)
      );
    }
  }

  destroy(): void {
    try {
      this.#areaInPx.clear();
      this.#context.getUIObserver().off('densityUpdate', this.#densityUpdateCallback);
      this.#win.off('avoidAreaChange', this.#avoidAreaChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] vp env destroy failed, ${error.message}`);
    }
  }
}

export default {
  WindowAvoidAreaPxEnv,
  WindowAvoidAreaVpEnv
}
