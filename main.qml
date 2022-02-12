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
    title: exonicCore.title

    Connections {
        target: exonicCore
        onProcessResolve: {
            exonicAPI.processResolved(result)
        }
        onProcessReject: {
            exonicAPI.processRejected(error)
        }
        onSigterm: {
            exonicAPI.sigterm()
        }
        onSigint: {
            exonicAPI.sigint()
        }
        onSigquit: {
            exonicAPI.sigquit()
        }
        onSighup: {
            exonicAPI.sighup()
        }
        onSigusr1: {
            exonicAPI.sigusr1()
        }
        onSigusr2: {
            exonicAPI.sigusr2()
        }
    }

    QtObject {
        id: exonicAPI
        WebChannel.id: "exonicAPI"

        function terminateApplication() {
            exonicCore.exonicTerminate();
        }

        function shell(command, returnStdOut, returnStdErr) {
            return exonicCore.shell(command, returnStdOut, returnStdErr);
        }

        function startProcess(processId) {
            exonicCore.startProcess(processId);
        }

        function terminateProcess(processId) {
            exonicCore.terminateProcess(processId);
        }

        function killProcess(processId) {
            exonicCore.killProcess(processId);
        }

        function getProcessState(processId) {
            return exonicCore.processState(processId);
        }

        function signalHandled() {
            exonicCore.signalHandled();
        }

        function setTitle(title) {
            exonicCore.title = title;
        }


        function openFile(filename, mode) {
            return exonicCore.openFile(filename, mode);
        }

        function closeFile(header) {
            return exonicCore.closeFile(header);
        }

        function fileRead(header, count) {
            return exonicCore.fileRead(header, count);
        }

        function fileReadLine(header) {
            return exonicCore.fileReadLine(header);
        }

        function fileReadAll(header) {
            return exonicCore.fileReadAll(header);
        }

        function fileExists(header) {
            return exonicCore.fileExists(header);
        }

        function fileWrite(header, data) {
            return exonicCore.fileWrite(header, data);
        }

        signal processResolved(variant object)
        signal processRejected(variant object)
        signal sigterm()
        signal sigint()
        signal sigquit()
        signal sighup()
        signal sigusr1()
        signal sigusr2()
    }

    WebEngineView {
        id: webEngineView
        focus: true
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: inputPanel.top
        url: exonicCore.url
        webChannel: webChannel
        userScripts: [
            WebEngineScript {
                injectionPoint: WebEngineScript.DocumentCreation
                worldId: WebEngineScript.MainWorld
                name: "QtWebChannel"
                sourceUrl: "qrc:///qtwebchannel/qwebchannel.js"
            },
            WebEngineScript {
                injectionPoint: WebEngineScript.DocumentCreation
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
        y: Qt.inputMethod.visible && exonicCore.virtualKeyboard ? parent.height - inputPanel.height : parent.height
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
