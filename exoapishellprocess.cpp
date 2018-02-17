#include "exoapishellprocess.h"
#include <QProcess>
#include <QJSEngine>
#include <QDebug>


ExoApiShellProcess::ExoApiShellProcess(const QString &command, QObject *parent, bool returnStdOut, bool returnStdErr) : QObject(parent)
{
    m_id = 0;
    m_process = new QProcess(this);
    m_returnStdErr = returnStdErr;
    m_returnStdOut = returnStdOut;
    m_command = command;
}

ExoApiShellProcess::~ExoApiShellProcess()
{
    delete m_process;
}

int ExoApiShellProcess::id() const
{
    return m_id;
}

QProcess::ProcessState ExoApiShellProcess::state() const
{
    return m_process->state();
}

QProcess *ExoApiShellProcess::process() const
{
    return m_process;
}

bool ExoApiShellProcess::returnStdOut() const
{
    return m_returnStdOut;
}

bool ExoApiShellProcess::returnStdErr() const
{
    return m_returnStdErr;
}

void ExoApiShellProcess::setId(int id)
{
    m_id = id;
}

void ExoApiShellProcess::start()
{
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    m_process->start(m_command);
}


void ExoApiShellProcess::disconnectFinishHandlers()
{
    disconnect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
    disconnect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
}

void ExoApiShellProcess::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    disconnectFinishHandlers();
    QJSEngine js;
    auto result = js.newObject();
    result.setProperty("id", m_id);
    result.setProperty("exitCode", exitCode);
    result.setProperty("exitStatus", (int)exitStatus);

    // ToDo: Add QByteArray normal conversion to ArrayBuffer
    if (m_returnStdOut) {
        auto stdOut = m_process->readAllStandardOutput();
        result.setProperty("stdOut", QString(stdOut));
    }

    if (m_returnStdErr) {
        auto stdErr = m_process->readAllStandardError();
        result.setProperty("stdErr", QString(stdErr));
    }

    emit done(m_id, result);
}

void ExoApiShellProcess::processError(QProcess::ProcessError error)
{
    disconnectFinishHandlers();

    QJSEngine js;
    auto result = js.newObject();
    result.setProperty("id", m_id);
    result.setProperty("error", (int)error);

    // ToDo: Add QByteArray normal conversion to ArrayBuffer
    if (m_returnStdOut) {
        auto stdOut = m_process->readAllStandardOutput();
        result.setProperty("stdOut", QString(stdOut));
    }

    if (m_returnStdErr) {
        auto stdErr = m_process->readAllStandardError();
        result.setProperty("stdErr", QString(stdErr));
    }

    emit fail(m_id, result);
}
