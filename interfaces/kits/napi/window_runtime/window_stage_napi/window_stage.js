/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

class WindowStage {
  constructor(obj) {
    this.windowStage_ = obj;
  }

  setUIContent(context, url, storage) {
    return this.windowStage_.setUIContent(context, url, storage);
  }

  loadContent(url, storage, asyncCallback) {
    return this.windowStage_.loadContent(url, storage, asyncCallback);
  }

  loadContentByName(name, storage, asyncCallback) {
    return this.windowStage_.loadContentByName(name, storage, asyncCallback);
  }

  releaseUIContent() {
    return this.windowStage_.releaseUIContent();
  }

  getWindowMode(asyncCallback) {
    return this.windowStage_.getWindowMode(asyncCallback);
  }

  getMainWindow(asyncCallback) {
    return this.windowStage_.getMainWindow(asyncCallback);
  }

  getMainWindowSync() {
    return this.windowStage_.getMainWindowSync();
  }

  createSubWindow(windowName, asyncCallback) {
    return this.windowStage_.createSubWindow(windowName, asyncCallback);
  }

  createSubWindowWithOptions(windowName, options, asyncCallback) {
    return this.windowStage_.createSubWindowWithOptions(windowName, options, asyncCallback);
  }

  getSubWindow(asyncCallback) {
    return this.windowStage_.getSubWindow(asyncCallback);
  }

  setWindowModal(isModal) {
    return this.windowStage_.setWindowModal(isModal);
  }

  on(type, callback) {
    return this.windowStage_.on(type, callback);
  }

  off(type, callback) {
    return this.windowStage_.off(type, callback);
  }

  setShowOnLockScreen(showOnLockScreen) {
    return this.windowStage_.setShowOnLockScreen(showOnLockScreen);
  }

  disableWindowDecor() {
    return this.windowStage_.disableWindowDecor();
  }

  setDefaultDensityEnabled(enabled) {
    return this.windowStage_.setDefaultDensityEnabled(enabled);
  }

  setCustomDensity(density, applyToSubWindow) {
    return this.windowStage_.setCustomDensity(density, applyToSubWindow);
  }

  removeStartingWindow() {
    return this.windowStage_.removeStartingWindow();
  }

  setWindowRectAutoSave(enabled, isSaveBySpecifiedFlag) {
    return this.windowStage_.setWindowRectAutoSave(enabled, isSaveBySpecifiedFlag);
  }

  isWindowRectAutoSave() {
    return this.windowStage_.isWindowRectAutoSave();
  }

  setSupportedWindowModes(supportedWindowModes, grayOutMaximizeButton) {
    if (grayOutMaximizeButton === undefined) {
      return this.windowStage_.setSupportedWindowModes(supportedWindowModes);
    }
    return this.windowStage_.setSupportedWindowModes(supportedWindowModes, grayOutMaximizeButton);
  }

  setImageForRecent(imgResourceId, value) {
    return this.windowStage_.setImageForRecent(imgResourceId, value);
  }

  removeImageForRecent() {
    return this.windowStage_.removeImageForRecent();
  }
}

export default WindowStage;
