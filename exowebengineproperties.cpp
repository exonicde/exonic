#include "exowebengineproperties.h"

#include <limits>
#include <QGuiApplication>
#include <QProcess>
#include <QDebug>


ExoWebEngineProperties::ExoWebEngineProperties(QGuiApplication *application, QObject *parent) : QObject(parent)
{
    m_application = application;
    m_lastProcessId = -1;
    m_vitualKeyboard = true;
}

void ExoWebEngineProperties::setUrl(const QString &a)
{
    if (a != m_url) {
        m_url = a;
        emit urlChanged();
    }
}

QString ExoWebEngineProperties::url() const
{
    return m_url;
}

void ExoWebEngineProperties::setTitle(const QString &a)
{
    if (a != m_title) {
        m_title = a;
        emit titleChanged();
    }
}

void ExoWebEngineProperties::setVirtualKeyboard(bool state)
{
    if (m_vitualKeyboard != state) {
        m_vitualKeyboard = state;
        emit virtualKeyboardChanged();
    }
}

QString ExoWebEngineProperties::title() const
{
    return m_title;
}

bool ExoWebEngineProperties::virtualKeyboard() const
{
    return m_vitualKeyboard;
}

void ExoWebEngineProperties::exonicTerminate()
{
    qInfo() << "Terminating...";
    m_application->exit();
}

int ExoWebEngineProperties::shell(const QString &command, bool returnStdOut, bool returnStdErr)
{
    qInfo() << "Creating process...";
    int imax = std::numeric_limits<int>::max();
    if (imax == m_processes.size())
        return -1;

    int nextPid = m_lastProcessId;
    do {
        if (imax > nextPid)
            ++nextPid;
        else
            nextPid = 0;
    } while (m_processes.contains(nextPid));

    auto exoProcess = new ExoApiShellProcess(command, this, returnStdOut, returnStdErr);
    exoProcess->setId(nextPid);
    m_processes.insert(nextPid, exoProcess);
    connect(exoProcess, SIGNAL(done(int,QJSValue&)), this, SLOT(processIsDone(int, QJSValue&)));
    connect(exoProcess, SIGNAL(fail(int,QJSValue&)), this, SLOT(processIsFailed(int,QJSValue&)));

    return exoProcess->id();
}


void ExoWebEngineProperties::startProcess(int processId)
{
    qInfo() << "Starting process...";
    auto exoProcess = m_processes[processId];
    exoProcess->start();
}

void ExoWebEngineProperties::terminateProcess(int processId)
{
    m_processes[processId]->process()->terminate();
}

void ExoWebEngineProperties::killProcess(int processId)
{
    m_processes[processId]->process()->kill();
}

QProcess::ProcessState ExoWebEngineProperties::processState(int processId)
{
    return m_processes.contains(processId) ? m_processes[processId]->state() : QProcess::NotRunning;
}

void ExoWebEngineProperties::log(const QString &text)
{
    qInfo() << text;
}

void ExoWebEngineProperties::processIsDone(int processId, QJSValue &args)
{
    qInfo() << "Process is done" << args.property("stdOut").toString();
    auto exoProcess = m_processes[processId];
    disconnect(exoProcess, SIGNAL(done(int,QJSValue&)), this, SLOT(processIsDone(int, QJSValue&)));
    disconnect(exoProcess, SIGNAL(fail(int,QJSValue&)), this, SLOT(processIsFailed(int,QJSValue&)));
    m_processes.remove(processId);
    emit processResolve(args);
}


void ExoWebEngineProperties::processIsFailed(int processId, QJSValue &args)
{
    qInfo() << "Process is failed";
    auto exoProcess = m_processes[processId];
    disconnect(exoProcess, SIGNAL(done(int,QJSValue&)), this, SLOT(processIsDone(int, QJSValue&)));
    disconnect(exoProcess, SIGNAL(fail(int,QJSValue&)), this, SLOT(processIsFailed(int,QJSValue&)));
    m_processes.remove(processId);
    emit processReject(args);
}
