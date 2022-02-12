(function () {
    'use strict';

    /**
     * Promise.prototype.finally
     *
     * Pulled from https://github.com/domenic/promises-unwrapping/issues/18#issuecomment-57801572
     * @author @stefanpenner, @matthew-andrews
     */

    (function() {
        // in programs even if there's ever a native finally.
        window.Promise.prototype.finally = window.Promise.prototype.finally || function (callback) {
            let constructor = this.constructor;

            return this.then(value => constructor.resolve(callback()).then(() => value))
                .catch(reason => constructor.resolve(callback()).then(() => { throw reason; }));
        };
    }());

    class UnixSignalsHandlers {
        constructor(api) {
            this._handlers = {
                term: [],
                int: [],
                quit: [],
                hup: [],
                usr1: [],
                usr2: []
            };
            for (let sig in this._handlers) {
                if (!this._handlers.hasOwnProperty(sig) || !api[`sig${sig}`]) continue;
                api[`sig${sig}`].connect(this._handle.bind(this, sig));
            }
            this._api = api;
        }

        addHandler(type, handler) {
            let handlers = this._handlers[type];
            if (handlers.indexOf(handler) === -1)
                handlers.push(handler);
        }

        removeHandler(type, handler) {
            let handlers = this._handlers[type];
            let index = handlers.indexOf(handler);
            if (index !== -1)
                handlers.splice(index, 1);
        }

        containsHandler(type, handler) {
            return this._handlers[type].indexOf(handler) !== -1;
        }

        availableSignals() {
            let signals = [];
            for (let sig in this._handlers) {
                if (!this._handlers.hasOwnProperty(sig)) continue;
                signals.push(`sig${sig}`);
            }

            return signals;
        }

        async _handle(type) {
            let handlers = this._handlers[type];
            for (let i = 0; i < handlers.length; ++i) {
                try {
                    await handlers[i]();
                }
                catch (e) {}
            }
            this._api.signalHandled();
        }
    }

    class ExonicFile {
        constructor(api, header) {
            this._api = api;
            this._header;
        }

        close() {
            return new Promise((resolve, reject) => {
                this._api.closeFile(this._header, result => Boolean(result) ? resolve(true) : reject(false))
            });
        }

        read(count = 1) {
            return new Promise((resolve, reject) => {
                this._api.fileRead(this._header, count, result => typeof(result) === "string" ? resolve(result) : reject(result));
            });
        }

        readLine() {
            return new Promise((resolve, reject) => {
                this._api.fileReadLine(this._header, result => typeof(result) === "string" ? resolve(result) : reject(result));
            });
        }

        readAll() {
            return new Promise((resolve, reject) => {
                this._api.fileReadAll(this._header, result => typeof(result) === "string" ? resolve(result) : reject(result));
            });
        }

        write(data) {
            return new Promise((resolve, reject) => {
                this._api.fileWrite(this._header, data, result => typeof(result) === "number" ? resolve(result) : reject(result));
            });
        }
    }

    class ExonicFileAPI {
        constructor(api) {
            this._api = api;
        }

        open(filename, mode) {
            return new Promise((resolve, reject) => {
                this._api.openFile(filename, mode, result => typeof(result) === 'number' ? resolve(result) : reject(result));
            }).then(header => new ExonicFile(this._api, header));
        }
    }

    function debugLog(text, erase = false) {
        let out = document.getElementById('output');
        if (erase) 
            out.innerText = text + "<br>";
        else
            out.innerText += text + "<br>";
        }

    function createExonicAPIObject(channel) {
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
            terminateApplication: channel.objects.exonicAPI.terminateApplication,
            signals: new UnixSignalsHandlers(channel.objects.exonicAPI),
            setTitle: channel.objects.exonicAPI.setTitle,
            file: new ExonicFileAPI(channel.objects.exonicAPI)
        };
    }

    function waitForQWebChannel() {
        if (window.hasOwnProperty('QWebChannel')) {
            new QWebChannel(qt.webChannelTransport, createExonicAPIObject);
        }
        else {
            setTimeout(waitForQWebChannel, 20);
        }
    }

    waitForQWebChannel();
})();
