#ifndef EXOWEBENGINEPROPERTIES_H
#define EXOWEBENGINEPROPERTIES_H

#include <csignal>
#include <QObject>
#include <QHash>
#include <QJSValue>

#include "exoapishellprocess.h"


class QGuiApplication;
class QProcess;

class ExoWebEngineProperties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool virtualKeyboard READ virtualKeyboard WRITE setVirtualKeyboard NOTIFY virtualKeyboardChanged)
public:
    explicit ExoWebEngineProperties(QGuiApplication *application, QObject *parent = nullptr);

    void setUrl(const QString &a);
    void setTitle(const QString &a);
    void setVirtualKeyboard(bool state);
    QString url() const;
    QString title() const;
    bool virtualKeyboard() const;
    Q_INVOKABLE void exonicTerminate();
    Q_INVOKABLE int shell(const QString &command, bool returnStdOut = false, bool returnStdErr = false);
    Q_INVOKABLE void startProcess(int processId);
    Q_INVOKABLE void terminateProcess(int processId);
    Q_INVOKABLE void killProcess(int processId);
    Q_INVOKABLE QProcess::ProcessState processState(int processId);
    Q_INVOKABLE void signalHandled();
    void sendSignal(int sig);

signals:
    void urlChanged();
    void titleChanged();
    void virtualKeyboardChanged();
    void processResolve(QJSValue result);
    void processReject(QJSValue error);
    void sigterm();
    void sigint();
    void sigquit();
    void sighup();
    void sigusr1();
    void sigusr2();
    void sigstop();

public slots:
    void processIsDone(int processId, QJSValue &args);
    void processIsFailed(int processId, QJSValue &args);

private:
    QString m_url;
    QString m_title;
    bool m_vitualKeyboard;
    QGuiApplication *m_application;
    QHash<int, ExoApiShellProcess *> m_processes;
    int m_lastProcessId;
    unsigned char m_signalHandlersStarted;

    static void onSigTerm(int sig);
    void checkSignalsHandlersState();
};

#endif // EXOWEBENGINEPROPERTIES_H
