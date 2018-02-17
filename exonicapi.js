(function () {
    'use strict';

    new QWebChannel(qt.webChannelTransport, function (channel) {
        function debugLog(text, erase = false) {
            let out = document.getElementById('output');
            if (erase) 
                out.innerText = text + "<br>";
            else
                out.innerText += text + "<br>";
        }

        window.exonicAPI = {
            shell(command, returnStdOut = false, returnStdErr = false) {
                return new Promise(resolve => {
                    channel.objects.exonicAPI.shell(command, returnStdOut, returnStdErr, resolve);
                }).then(processId => {
                    return {
                        id: processId,
                        state() {
                            return new Promise(resolve => channel.objects.exonicAPI.getProcessState(processId, resolve));
                        },
                        kill() {
                            channel.objects.exonicAPI.killProcess(processId);
                        },
                        terminate() {
                            channel.objects.exonicAPI.terminateProcess(processId);
                        },
                        promise: new Promise((resolve, reject) => {
                            let onResolve = result => {
                                if (!result || result.id !== processId) return;
                                delete result.id;
                                channel.objects.exonicAPI.processResolved.disconnect(onResolve);
                                resolve(result);
                            };
                            let onReject = error => {
                                if (!error || error.id !== processId) return;
                                delete error.id;
                                channel.objects.exonicAPI.processRejected.disconnect(onReject);
                                reject(error);
                            };
                            channel.objects.exonicAPI.processResolved.connect(onResolve);
                            channel.objects.exonicAPI.processRejected.connect(onReject);
                            channel.objects.exonicAPI.startProcess(processId);
                        })
                    };
                });
            },
            terminateApplication: channel.objects.exonicAPI.terminateApplication
        };
    });
})();
