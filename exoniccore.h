#ifndef EXONICCORE_H
#define EXONICCORE_H

#include <QObject>
#include <QHash>
#include <QJSValue>
#include <QSocketNotifier>

#include "exoapishellprocess.h"


class QGuiApplication;
class QProcess;
class QFile;

class ExonicCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool virtualKeyboard READ virtualKeyboard WRITE setVirtualKeyboard NOTIFY virtualKeyboardChanged)
public:
    explicit ExonicCore(QGuiApplication *application, QObject *parent = nullptr);

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

    Q_INVOKABLE int openFile(const QString &filename, const QString &mode);
    Q_INVOKABLE bool closeFile(int header);
    Q_INVOKABLE QJSValue fileRead(int header, int count = 1);
    Q_INVOKABLE QJSValue fileReadLine(int header);
    Q_INVOKABLE QString fileReadAll(int header);
    Q_INVOKABLE bool fileExists(int header);
    Q_INVOKABLE int fileWrite(int header, const QString &data);

    void sendSignal(int sig);

    static void termSignalHandler(int unused);
    static void intSignalHandler(int unused);
    static void quitSignalHandler(int unused);
    static void hupSignalHandler(int unused);
    static void usr1SignalHandler(int unused);
    static void usr2SignalHandler(int unused);

    static int setUnixSignalHandlers();


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

public slots:
    void processIsDone(int processId, QJSValue &args);
    void processIsFailed(int processId, QJSValue &args);
    void onSigTerm();
    void onSigInt();
    void onSigQuit();
    void onSigHup();
    void onSigUsr1();
    void onSigUsr2();

private:
    QString m_url;
    QString m_title;
    bool m_vitualKeyboard;
    QGuiApplication *m_application;
    QHash<int, ExoApiShellProcess *> m_processes;
    QHash<int, QFile *> m_files;
    int m_lastProcessId;
    int m_lastFileId;
    unsigned char m_signalHandlersStarted;
    QSocketNotifier *m_termNotifier;
    QSocketNotifier *m_intNotifier;
    QSocketNotifier *m_quitNotifier;
    QSocketNotifier *m_hupNotifier;
    QSocketNotifier *m_usr1Notifier;
    QSocketNotifier *m_usr2Notifier;

    static int m_sigtermFd[2];
    static int m_sigintFd[2];
    static int m_sigquitFd[2];
    static int m_sighupFd[2];
    static int m_sigusr1Fd[2];
    static int m_sigusr2Fd[2];

    void checkSignalsHandlersState();
};

#endif // EXONICCORE_H
