#ifndef EXOAPISHELLPROCESS_H
#define EXOAPISHELLPROCESS_H

#include <QObject>
#include <QJSValue>
#include <QProcess>


class ExoApiShellProcess : public QObject {
    Q_OBJECT

public:
    explicit ExoApiShellProcess(const QString &command, QObject *parent = nullptr, bool returnStdOut = false, bool returnStdErr = false);
    virtual ~ExoApiShellProcess();

    int id() const;
    QProcess::ProcessState state() const;
    QProcess *process() const;
    bool returnStdOut() const;
    bool returnStdErr() const;
    void setId(int id);
    void start();

signals:
    void done(int processId, QJSValue &args);
    void fail(int processId, QJSValue &args);

protected:
    int m_id;
    QString m_command;
    QProcess *m_process;
    bool m_returnStdOut;
    bool m_returnStdErr;

    void disconnectFinishHandlers();

protected slots:
    void processFinished(int exitCode, QProcess::ExitStatus);
    void processError(QProcess::ProcessError error);
};

#endif // EXOAPISHELLPROCESS_H
