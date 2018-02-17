import QtQuick 2.7
import QtQuick.Controls 1.1
import QtQuick.VirtualKeyboard 2.1
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

    Connections {
        target: appProperties
        onProcessResolve: {
            exonicAPI.processResolved(result)
        }
        onProcessReject: {
            exonicAPI.processRejected(error)
        }
    }

    QtObject {
        id: exonicAPI
        WebChannel.id: "exonicAPI"

        function terminateApplication() {
            appProperties.exonicTerminate();
        }

        function shell(command, returnStdOut, returnStdErr) {
            return appProperties.shell(command, returnStdOut, returnStdErr);
        }

        function startProcess(processId) {
            appProperties.startProcess(processId);
        }

        function terminateProcess(processId) {
            appProperties.terminateProcess(processId);
        }

        function killProcess(processId) {
            appProperties.killProcess(processId);
        }

        function getProcessState(processId) {
            return appProperties.processState(processId);
        }

        signal processResolved(variant object)
        signal processRejected(variant object)
    }

    WebEngineView {
        id: webEngineView
        focus: true
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: inputPanel.top
        url: appProperties.url
        webChannel: webChannel
        userScripts: [
            WebEngineScript {
                injectionPoint: WebEngineScript.DocumentCreation
                worldId: WebEngineScript.MainWorld
                name: "QtWebChannel"
                sourceUrl: "qrc:///qtwebchannel/qwebchannel.js"
            },
            WebEngineScript {
                injectionPoint: WebEngineScript.DocumentReady
                worldId: WebEngineScript.MainWorld
                name: "ExonicAPI"
                sourceUrl: "qrc:///exonicapi.js"
            }
        ]
    }

    WebChannel {
        id: webChannel
        registeredObjects: [exonicAPI]
    }

    InputPanel {
        id: inputPanel
        focus: false
        y: Qt.inputMethod.visible && appProperties.virtualKeyboard ? parent.height - inputPanel.height : parent.height
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
