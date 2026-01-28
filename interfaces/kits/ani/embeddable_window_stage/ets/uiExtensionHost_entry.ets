import uiExtensionHost from '@ohos.uiExtensionHost';
import window from '@ohos.window';

function main() {
    loadLibrary('embeddablewindowstageani_module.z')
    let win = uiExtensionHost.createExtentionWindow(0, 0);
    console.println('test <set> uiExtensionHostWindowProxyRect');

    console.println('test <get> uiExtensionHostWindowProxyRect' + win.properties);

    console.println('test getWindowAvoidArea CUT ' + win.getWindowAvoidArea(window.AvoidAreaType.TYPE_CUTOUT).visible);
    console.println('test getWindowAvoidArea SYS ' + win.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM).visible);
    console.println('test onWindowSizeChange ');

    let cb = (size: window.Size) => {
      console.info(`The avoid area of the host window is: ${JSON.stringify(size)}.`);
    }
    win.on('windowSizeChange', cb);
    console.println('test offWindowSizeChange ');
    win.off('windowSizeChange');

    console.println('test onAvoidAreaChange ');
    cb = (size: window.Size) => {
      console.info(`The avoid area of the host window is: ${JSON.stringify(size)}.`);
    }
    win.on('avoidAreaChange', cb)
    console.println('test offAvoidAreaChange ');
    win.off('avoidAreaChange');
}
