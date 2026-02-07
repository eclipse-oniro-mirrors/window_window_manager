'use static'

import {Entry, Text, Column, Row, Component, Button, ClickEvent,
  Context, Builder, Stack, Color, XComponent, NodeContainer
} from '@ohos.arkui.component';
import { XComponentController, XComponentOptions } from 'arkui.component.xcomponent';
import { XComponentType } from 'arkui.component.enums';
import { State } from '@ohos.arkui.stateManagement';
import { FrameNode, NodeController } from '@ohos.arkui.node'
import { typeNode } from 'arkui.FrameNode';
import { UIContext } from '@ohos.arkui.UIContext';
import hilog from '@ohos.hilog';
import { PiPManager } from './@ohos.PiPWindow';

const DOMAIN = 0x420E;
const TAG: string = '[ANI][PiPContent]';
const ABOUT_TO_STOP = 3;

export type CallbackVoid<T> = (data: T) => void;
export type CallbackState = (state: int) => void;
export type OnCallback = CallbackVoid<typeNode.XComponent> | CallbackState;

export class XCNodeController extends NodeController {
  private mXComponent: typeNode.XComponent;
  private node: FrameNode | null = null;

  constructor(xComponent: typeNode.XComponent) {
    super();
    this.mXComponent = xComponent;
  }

  makeNode(uiContext: UIContext): FrameNode | null {
    this.node = new FrameNode(uiContext);
    this.node?.appendChild(this.mXComponent);
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

  validateNode(node: typeNode.XComponent | null): boolean {
    if (node === null || node === undefined) {
      return false;
    }
    let type: string | undefined = node?.getNodeType();
    if (type !== 'XComponent') {
      hilog.error(DOMAIN, TAG, `node type mismatch: ${type}`);
      return false;
    }
    return true;
  }

  UpdateNodeCallback(newNode: typeNode.XComponent):void {
    hilog.info(DOMAIN, TAG, 'runUpdateNodeCallback start');
    if (!this.validateNode(newNode)) {
      return;
    }
    if (this.useNode) {
      PiPManager.setPipNodeType(this.xComponent!, false);
      this.updatePipNodeType(newNode);
      this.mXCNodeController?.replaceNode(newNode);
      this.nodeChange = true;
    } else {
      this.updatePipNodeType(newNode);
      this.mXCNodeController = new XCNodeController(newNode);
      this.registerStateChangeListener();
      this.useNode = true;
    }
  }

  private registerUpdateNodeListener() {
    PiPManager.on('nodeUpdate', this.UpdateNodeCallback);
  }

  private updatePipNodeType(newNode: typeNode.XComponent) {
    hilog.info(DOMAIN, TAG, 'updatePipNodeType start');
    let parent: FrameNode | null = newNode.getParent();
    if (parent === null || parent === undefined) {
      PiPManager.setPipNodeType(newNode, false);
    } else {
      PiPManager.setPipNodeType(newNode, true);
      parent?.removeChild(newNode);
    }
  }

  stateChangeCallback(state: int):void {
    if (state === ABOUT_TO_STOP) {
      this.mXCNodeController?.removeNode();
    }
  }

  private registerStateChangeListener() {
    PiPManager.on('stateChange', this.stateChangeCallback);
  }

  aboutToAppear(): void {
    try {
      hilog.info(DOMAIN, TAG, 'aboutToAppear start');
      this.nodeController = PiPManager.getCustomUIController();
      this.registerUpdateNodeListener();
      this.xComponent = PiPManager.getTypeNode();
      if (!this.validateNode(this.xComponent)) {
        return;
      }
      if (this.xComponent === null) {
        return;
      }
      this.useNode = true;
      this.updatePipNodeType(this.xComponent!);
      PiPManager.setTypeNodeEnabled();
      this.mXCNodeController = new XCNodeController(this.xComponent!);
      this.registerStateChangeListener();
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'aboutToAppear error');
    }
  }

  aboutToDisappear(): void {
    try {
      PiPManager.off('stateChange');
      PiPManager.off('nodeUpdate');
    } catch (err) {
      hilog.info(DOMAIN, TAG, 'aboutToDisappear failed');
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
    NodeContainer(this.nodeController!)
      .size({ width: '100%', height: '100%' });
  }

  @Builder
  buildXComponent() {
    XComponent({ type: XComponentType.SURFACE, controller: this.xComponentController } as XComponentOptions)
      .onLoad(() => {
        PiPManager.initXComponentController(this.xComponentController);
        hilog.info(DOMAIN, TAG, 'XComponent onLoad done');

      })
      .size({ width: '100%', height: '100%' })
      .backgroundColor(Color.Transparent);
  }

  @Builder
  buildNode() {
    NodeContainer(this.mXCNodeController!)
      .size({ width: '100%', height: '100%' })
  }
}