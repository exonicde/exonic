import QtQuick 2.9
import QtQuick.Controls 1.1
import QtQuick.VirtualKeyboard 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2
import QtWebEngine 1.1
import QtWebChannel 1.0

ApplicationWindow {
    id: window
    visible: true
    x: initialX
    y: initialY
    width: initialWidth
    height: initialHeight
    title: appProperties.title

    QtObject{
        id: exonicAPI
        WebChannel.id: "exonicAPI"

        function terminateApplication() {
            appProperties.exonicTerminate();
        }
    }

    WebEngineView {
        anchors.fill: parent
        url: appProperties.url
        webChannel: webChannel
    }

    WebChannel {
        id: webChannel
        registeredObjects: [exonicAPI]
    }
}
