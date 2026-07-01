/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import pip from '@ohos.pip';
import { NodeController, FrameNode, UIContext, typeNode } from '@kit.ArkUI';

const TAG: string = 'PiPContent';
const ABOUT_TO_STOP = 3;

class XCNodeController extends NodeController {
  private mXComponent: typeNode.XComponent;
  private node: FrameNode | null = null;

  constructor(xComponent: typeNode.XComponent) {
    super();
    this.mXComponent = xComponent;
  }

  makeNode(uiContext: UIContext): FrameNode | null {
    this.node = new FrameNode(uiContext);
    try {
        this.node.appendChild(this.mXComponent);
    } catch (err) {
        console.error('appendChild failed');
    }
    return this.node;
  }

  replaceNode(newNode: typeNode.XComponent): void {
    this.node?.removeChild(this.mXComponent);
    this.mXComponent = newNode;
    this.node?.appendChild(this.mXComponent);
  }

  removeNode() {
    this.node?.removeChild(this.mXComponent);
  }
  aboutToDisappear() {
    this.node?.clearChildren();
  }
}

@Entry
@Component
struct PiPContent {
  private xComponentController: XComponentController = new XComponentController();
  private nodeController: NodeController | null = null;
  private mXCNodeController: XCNodeController | null = null;
  @State useNode: boolean = false;
  @State nodeChange: boolean = false;
  private xComponent: typeNode.XComponent | null = null;
  xComponentId: string = 'pipContent';
  xComponentType: string = 'surface';

  validateNode(node: typeNode.XComponent | null): boolean {
    if (node === null || node === undefined) {
      console.error(TAG, `validateNode node is null`);
      return false;
    }
    let type: string = node.getNodeType();
    if (type !== 'XComponent') {
      console.error(TAG, `node type mismatch: ${type}`);
      return false;
    }
    return true;
  }

  private registerUpdateNodeListener() {
    pip.on('nodeUpdate', (newNode: typeNode.XComponent) => {
      console.info(TAG, `nodeUpdate`);
      if (!this.validateNode(newNode)) {
        return;
      }
      if (this.useNode) {
        pip.setPipNodeType(this.xComponent, false);
        this.updatePipNodeType(newNode);
        this.mXCNodeController?.replaceNode(newNode);
        this.nodeChange = true;
      } else {
        this.updatePipNodeType(newNode);
        this.mXCNodeController = new XCNodeController(newNode);
        console.info(TAG, 'update to Node Controller');
        this.registerStateChangeListener();
        this.useNode = true;
      }
    });
  }

  private updatePipNodeType(newNode: typeNode.XComponent) {
    let parent: FrameNode | null = newNode.getParent();
    if (parent === null || parent === undefined) {
      pip.setPipNodeType(newNode, false);
    } else {
      pip.setPipNodeType(newNode, true);
      parent.removeChild(newNode);
    }
  }

  private registerStateChangeListener() {
    pip.on('stateChange', (state: number) => {
      console.info(TAG, `stateChange state:${state}`);
      if (state === ABOUT_TO_STOP) {
        this.mXCNodeController?.removeNode();
      }
    })
  }

  aboutToAppear(): void {
    try {
      this.nodeController = pip.getCustomUIController();
      this.registerUpdateNodeListener();
      this.xComponent = pip.getTypeNode();
      if (!this.validateNode(this.xComponent)) {
        return;
      }
      if (this.xComponent === null) {
        console.error(TAG, `validateNode node is null`);
        return;
      }
      this.useNode = true;
      this.updatePipNodeType(this.xComponent);
      pip.setTypeNodeEnabled();
      this.mXCNodeController = new XCNodeController(this.xComponent);
      console.info(TAG, 'use Node Controller');
      this.registerStateChangeListener();
    } catch (err) {
      console.log(`aboutToAppear failed`);
    }
  }

  aboutToDisappear(): void {
    try {
      pip.off('stateChange');
      pip.off('nodeUpdate');
    } catch (err) {
      console.log(`aboutToDisappear failed`);
    }
  }

  build() {
    Stack() {
      if (this.useNode || this.nodeChange) {
        this.buildNode();
      } else {
        this.buildXComponent();
      }
      if (this.nodeController !== null) {
        this.buildCustomUI();
      }
    }
    .size({ width: '100%', height: '100%' });
  }

  @Builder
  buildCustomUI() {
    NodeContainer(this.nodeController)
      .size({ width: '100%', height: '100%' });
  }

  @Builder
  buildXComponent() {
    XComponent({ id: this.xComponentId, type: this.xComponentType, controller: this.xComponentController })
      .onLoad(() => {
        pip.initXComponentController(this.xComponentController);
        console.debug(TAG, 'XComponent onLoad done');
      })
      .size({ width: '100%', height: '100%' })
      .backgroundColor(Color.Transparent);
  }

  @Builder
  buildNode() {
    NodeContainer(this.mXCNodeController)
      .size({ width: '100%', height: '100%' })
  }
}