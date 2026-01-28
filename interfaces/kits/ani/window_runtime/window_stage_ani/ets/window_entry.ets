import window from '@ohos.window';
import {
  AsyncCallback,
  AsyncCallbackVoid,
  BaseContext,
  LocalStorage,
  BusinessError,
  UIContext,
} from '@ohos.window';

function main() {
    loadLibrary('windowstageani_module.z')
    let ws = window.CreateWindowStage(0);

    let callback:AsyncCallbackVoid = (err) => {
      if (err.code) {
        console.error(`Failed to load the content. Cause code: ${err.code}, message: ${err.message}`);
        return;
      }
      console.info('Succeeded in loading the content.');
    }

    ws.loadContent('ohos.test', callback);

    ws.getMainWindowSync();

    ws.getMainWindow().then((res:window.Window)=>{
      console.println('[ANI]getMainWindow primise');
    }).catch((e:Any)=>{
      console.println('[ANI]getMainWindow catch');
    });

    ws.getMainWindow(()=>{
      console.println('[ANI]getMainWindow callback');
    });

    let w = window.CreateWindow(0);

    w.isWindowSupportWideGamut().then((res:boolean)=>{
      console.println('[ANI]isWindowSupportWideGamut primise' + res);
    }).catch((e:Any)=>{
      console.println('[ANI]isWindowSupportWideGamut catch');
    });

    w.isWindowSupportWideGamut(()=>{
      console.println('[ANI]wideGamut callback');
    });

    w.setWindowLayoutFullScreen(true).then((res:int)=>{
      console.println('[ANI]setWindowLayoutFullScree primise' + res);
    }).catch((e:Any)=>{
      console.println('[ANI]setWindowLayoutFullScreen catch');
    });

    w.setWindowLayoutFullScreen(false, ()=>{
      console.println('[ANI]setWindowLayoutFullScreen callback');
    });

    let systemBarProperties = new window.SystemBarPropertiesInternal();
    w.setWindowSystemBarProperties(systemBarProperties).then((res:int)=>{
      console.println('[ANI]setWindowSystemBarProperties primise' + res);
    }).catch((e:Any)=>{
      console.println('[ANI]setWindowSystemBarProperties catch');
    });

    w.setWindowSystemBarProperties(systemBarProperties, ()=>{
      console.println('[ANI]setWindowSystemBarProperties callback');
    });

    w.setSpecificSystemBarEnabled('w', false, false).then((res:int)=>{
      console.println('[ANI]setSpecificSystemBarEnabled primise' + res);
    }).catch((e:Any)=>{
      console.println('[ANI]setSpecificSystemBarEnabled catch');
    });


    w.getWindowDecorHeight();
    w.setWindowBackgroundColor('0xFFFFFF');
    w.setWindowColorSpace(window.ColorSpace.DEFAULT).then((ret: int) => {
      console.println('[ANI] in ret:' + ret);
    }).catch((e: Any) => {
      let err = e as BusinessError;
      console.println('[ANI] in code:' + err?.code + ' err:' + err?.message);
    });
    w.setWindowColorSpace(window.ColorSpace.DEFAULT, (err) => {
      console.println('[ANI] setColorSpace err:' + err);
    });
    w.setPreferredOrientation(window.Orientation.UNSPECIFIED);
    w.setWindowPrivacyMode(true);
    w.recover();
    w.setUIContent('path');
    w.loadContent('path');
    w.loadContent('path', new LocalStorage());
    w.setWindowKeepScreenOn(true);
    let value: 'status' | 'navigation' = 'status';
    w.setWindowSystemBarEnable(Array.of(value));
    try {
      let ctx = w.getUIContext();
      console.println('[ANI] getUIContext ctx:'+ctx);
    } catch (e: Error) {
      console.println('[ANI] in e:' + e);
    }
    let avoidArea = w.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM);
    console.println('[ANI] avoidArea:' + JSON.stringify(avoidArea));
    w.setWaterMarkFlag(true);
    let cb = (data: window.WindowStatusType): void => {
      console.println(`onOffWindowStatusChangeTest status:${data as int}`);
    };
    w.on('windowEvent', cb);
    w.off('windowEvent', cb);
    let cb2 = (data: window.Size): void => {
      console.println(`windowSizeChange size:${JSON.stringify(data)}`);
    };
    w.on('windowSizeChange', cb2);
    w.off('windowSizeChange', cb2);
    let cb3 = (data: window.WindowEventType): void => {
      console.println(`windowEvent event:${data as int}`);
    };
    w.on('windowStatusChange', cb3);
    w.off('windowStatusChange', cb3);

    w.setImmersiveModeEnabledState(false);
    w.setWindowDecorVisible(false);
    w.setWindowDecorHeight(100);
    w.getWindowProperties();
    w.setWindowSystemBarProperties(systemBarProperties);
    w.setSpecificSystemBarEnabled('w', false, false);
    console.println('ok');
}
