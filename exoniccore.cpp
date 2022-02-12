#include "exoniccore.h"

#include <limits>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <QGuiApplication>
#include <QProcess>
#include <QFile>
#include <QDebug>


int ExonicCore::m_sigtermFd[2];
int ExonicCore::m_sigintFd[2];
int ExonicCore::m_sigquitFd[2];
int ExonicCore::m_sighupFd[2];
int ExonicCore::m_sigusr1Fd[2];
int ExonicCore::m_sigusr2Fd[2];

ExonicCore::ExonicCore(QGuiApplication *application, QObject *parent) : QObject(parent)
{
    m_application = application;
    m_lastProcessId = m_lastFileId = -1;
    m_vitualKeyboard = true;
    m_signalHandlersStarted = 0;

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigtermFd))
       qFatal("Couldn't create TERM socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigintFd))
       qFatal("Couldn't create INT socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigquitFd))
       qFatal("Couldn't create QUIT socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sighupFd))
       qFatal("Couldn't create HUP socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigusr1Fd))
       qFatal("Couldn't create USR1 socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigusr2Fd))
       qFatal("Couldn't create USR2 socketpair");

    m_termNotifier = new QSocketNotifier(m_sigtermFd[1], QSocketNotifier::Read, this);
    connect(m_termNotifier, SIGNAL(activated(int)), this, SLOT(onSigTerm()));

    m_intNotifier = new QSocketNotifier(m_sigintFd[1], QSocketNotifier::Read, this);
    connect(m_intNotifier, SIGNAL(activated(int)), this, SLOT(onSigInt()));

    m_quitNotifier = new QSocketNotifier(m_sigquitFd[1], QSocketNotifier::Read, this);
    connect(m_quitNotifier, SIGNAL(activated(int)), this, SLOT(onSigQuit()));

    m_hupNotifier = new QSocketNotifier(m_sighupFd[1], QSocketNotifier::Read, this);
    connect(m_hupNotifier, SIGNAL(activated(int)), this, SLOT(onSigHup()));

    m_usr1Notifier = new QSocketNotifier(m_sigusr1Fd[1], QSocketNotifier::Read, this);
    connect(m_usr1Notifier, SIGNAL(activated(int)), this, SLOT(onSigUsr1()));

    m_usr2Notifier = new QSocketNotifier(m_sigusr2Fd[1], QSocketNotifier::Read, this);
    connect(m_usr2Notifier, SIGNAL(activated(int)), this, SLOT(onSigUsr2()));
}

void ExonicCore::setUrl(const QString &a)
{
    if (a != m_url) {
        m_url = a;
        emit urlChanged();
    }
}

QString ExonicCore::url() const
{
    return m_url;
}

void ExonicCore::setTitle(const QString &a)
{
    if (a != m_title) {
        m_title = a;
        emit titleChanged();
    }
}

void ExonicCore::setVirtualKeyboard(bool state)
{
    if (m_vitualKeyboard != state) {
        m_vitualKeyboard = state;
        emit virtualKeyboardChanged();
    }
}

QString ExonicCore::title() const
{
    return m_title;
}

bool ExonicCore::virtualKeyboard() const
{
    return m_vitualKeyboard;
}

void ExonicCore::exonicTerminate()
{
    qInfo() << "Terminating...";
    m_application->exit();
}

int ExonicCore::shell(const QString &command, bool returnStdOut, bool returnStdErr)
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


void ExonicCore::startProcess(int processId)
{
    qInfo() << "Starting process...";
    auto exoProcess = m_processes[processId];
    exoProcess->start();
}

void ExonicCore::terminateProcess(int processId)
{
    m_processes[processId]->process()->terminate();
}

void ExonicCore::killProcess(int processId)
{
    m_processes[processId]->process()->kill();
}

QProcess::ProcessState ExonicCore::processState(int processId)
{
    return m_processes.contains(processId) ? m_processes[processId]->state() : QProcess::NotRunning;
}

void ExonicCore::signalHandled()
{
    --m_signalHandlersStarted;
    checkSignalsHandlersState();
}

int ExonicCore::openFile(const QString &filename, const QString &mode)
{
    qInfo() << "Opening file" << filename;
    int imax = std::numeric_limits<int>::max();
    if (imax == m_files.size())
        return -1;

    int nextId = m_lastFileId;
    do {
        if (imax > nextId)
            ++nextId;
        else
            nextId = 0;
    } while (m_files.contains(nextId));

    QIODevice::OpenMode flags = 0x0;

    if (!mode.contains('b'))
        flags |= QIODevice::Text;

    if (mode.contains('u'))
        flags |= QIODevice::Unbuffered;

    if (mode.contains('+')) {
        if (mode.contains('w'))
            flags |= QIODevice::ReadWrite | QIODevice::Truncate;
        else if (mode.contains('a'))
            flags |= QIODevice::ReadWrite | QIODevice::Append;
        else if (mode.contains('r'))
            flags |= QIODevice::ReadWrite;
    }
    else {
        if (mode.contains('w'))
            flags |= QIODevice::WriteOnly | QIODevice::Truncate;
        else if (mode.contains('a'))
            flags |= QIODevice::WriteOnly | QIODevice::Append;
        else if (mode.contains('r'))
            flags |= QIODevice::ReadOnly;
    }

    auto file = new QFile(filename, this);

    if (!file->open(flags)) {
        delete file;
        return -1;
    }

    m_files.insert(nextId, file);
    qInfo() << "GGG";

    return nextId;
}

QString ExonicCore::fileReadAll(int header)
{
    qInfo() << "read all";
    if (!m_files.contains(header))
        return QString("");//QJSValue::NullValue;

    auto file = m_files[header];
    // ToDo: add binary format support
//    QJSValue result(QString(file->readAll()));

    return QString(file->readAll());
}

bool ExonicCore::fileExists(int header)
{
    if (!m_files.contains(header))
        return false;
    return m_files[header]->exists();
}

int ExonicCore::fileWrite(int header, const QString &data)
{
    qInfo() << "Before check";
    if (!m_files.contains(header))
        return 0;

    qInfo() << "Before convert";
    QByteArray d = data.toUtf8();
    qInfo() << "Before write";
    int result = static_cast<int>(m_files[header]->write(data.toUtf8()));
    qInfo() << "Bytes written" << result;
    return result;
}

bool ExonicCore::closeFile(int header)
{
    if (!m_files.contains(header))
        return false;

    auto file = m_files[header];
    file->close();
    m_files.remove(header);
    delete file;

    return true;
}

QJSValue ExonicCore::fileRead(int header, int count)
{
    if (!m_files.contains(header))
        return QJSValue::NullValue;

    return QJSValue(QString(m_files[header]->read(count)));
}

QJSValue ExonicCore::fileReadLine(int header)
{
    if (!m_files.contains(header))
        return QJSValue::NullValue;

    return QJSValue(QString(m_files[header]->readLine()));
}

void ExonicCore::sendSignal(int sig)
{
    qInfo() << "sendSignal(" << sig << ")";
    ++m_signalHandlersStarted;
    switch (sig) {
    case SIGTERM:
        emit sigterm();
        break;
    case SIGINT:
        emit sigint();
        break;
    case SIGQUIT:
        emit sigquit();
        break;
    case SIGHUP:
        emit sighup();
        break;
    case SIGUSR1:
        emit sigusr1();
        break;
    case SIGUSR2:
        emit sigusr2();
        break;
    }
}

void ExonicCore::termSignalHandler(int unused)
{
    char a = 1;
    ::write(m_sigtermFd[0], &a, sizeof(a));
}

void ExonicCore::intSignalHandler(int unused)
{
    char a = 1;
    ::write(m_sigintFd[0], &a, sizeof(a));
}

void ExonicCore::quitSignalHandler(int unused)
{
    char a = 1;
    ::write(m_sigquitFd[0], &a, sizeof(a));
}

void ExonicCore::hupSignalHandler(int unused)
{
    char a = 1;
    ::write(m_sighupFd[0], &a, sizeof(a));
}

void ExonicCore::usr1SignalHandler(int unused)
{
    char a = 1;
    ::write(m_sigusr1Fd[0], &a, sizeof(a));
}

void ExonicCore::usr2SignalHandler(int unused)
{
    char a = 1;
    ::write(m_sigusr2Fd[0], &a, sizeof(a));
}

int ExonicCore::setUnixSignalHandlers()
{
    struct sigaction sTerm, sInt, sQuit, sHup, sUsr1, sUsr2;
    sTerm.sa_handler = ExonicCore::termSignalHandler;
    sigemptyset(&sTerm.sa_mask);
    sTerm.sa_flags = 0;
    sTerm.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &sTerm, 0))
        return 1;

    sInt.sa_handler = ExonicCore::intSignalHandler;
    sigemptyset(&sInt.sa_mask);
    sInt.sa_flags = 0;
    sInt.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &sInt, 0))
        return 2;

    sQuit.sa_handler = ExonicCore::quitSignalHandler;
    sigemptyset(&sQuit.sa_mask);
    sQuit.sa_flags = 0;
    sQuit.sa_flags |= SA_RESTART;

    if (sigaction(SIGQUIT, &sQuit, 0))
        return 3;

    sHup.sa_handler = ExonicCore::hupSignalHandler;
    sigemptyset(&sHup.sa_mask);
    sHup.sa_flags = 0;
    sHup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &sHup, 0))
        return 4;

    sUsr1.sa_handler = ExonicCore::usr1SignalHandler;
    sigemptyset(&sUsr1.sa_mask);
    sUsr1.sa_flags = 0;
    sUsr1.sa_flags |= SA_RESTART;

    if (sigaction(SIGUSR1, &sUsr1, 0))
        return 5;

    sUsr2.sa_handler = ExonicCore::usr2SignalHandler;
    sigemptyset(&sUsr2.sa_mask);
    sUsr2.sa_flags = 0;
    sUsr2.sa_flags |= SA_RESTART;

    if (sigaction(SIGUSR2, &sUsr2, 0))
        return 5;

    return 0;
}

void ExonicCore::processIsDone(int processId, QJSValue &args)
{
    qInfo() << "Process is done" << args.property("stdOut").toString();
    auto exoProcess = m_processes[processId];
    disconnect(exoProcess, SIGNAL(done(int,QJSValue&)), this, SLOT(processIsDone(int, QJSValue&)));
    disconnect(exoProcess, SIGNAL(fail(int,QJSValue&)), this, SLOT(processIsFailed(int,QJSValue&)));
    m_processes.remove(processId);
    emit processResolve(args);
}


void ExonicCore::processIsFailed(int processId, QJSValue &args)
{
    qInfo() << "Process is failed";
    auto exoProcess = m_processes[processId];
    disconnect(exoProcess, SIGNAL(done(int,QJSValue&)), this, SLOT(processIsDone(int, QJSValue&)));
    disconnect(exoProcess, SIGNAL(fail(int,QJSValue&)), this, SLOT(processIsFailed(int,QJSValue&)));
    m_processes.remove(processId);
    emit processReject(args);
}

void ExonicCore::onSigTerm()
{
    m_termNotifier->setEnabled(false);
    char tmp;
    ::read(m_sigtermFd[1], &tmp, sizeof(tmp));

    sendSignal(SIGTERM);

    m_termNotifier->setEnabled(true);
}

void ExonicCore::onSigInt()
{
    m_intNotifier->setEnabled(false);
    char tmp;
    ::read(m_sigintFd[1], &tmp, sizeof(tmp));

    sendSignal(SIGINT);

    m_intNotifier->setEnabled(true);
}

void ExonicCore::onSigQuit()
{
    m_quitNotifier->setEnabled(false);
    char tmp;
    ::read(m_sigquitFd[1], &tmp, sizeof(tmp));

    sendSignal(SIGQUIT);

    m_quitNotifier->setEnabled(true);
}

void ExonicCore::onSigHup()
{
    m_hupNotifier->setEnabled(false);
    char tmp;
    ::read(m_sighupFd[1], &tmp, sizeof(tmp));

    sendSignal(SIGHUP);

    m_hupNotifier->setEnabled(true);
}

void ExonicCore::onSigUsr1()
{
    m_usr1Notifier->setEnabled(false);
    char tmp;
    ::read(m_sigusr1Fd[1], &tmp, sizeof(tmp));

    sendSignal(SIGUSR1);

    m_usr1Notifier->setEnabled(true);
}

void ExonicCore::onSigUsr2()
{
    m_usr2Notifier->setEnabled(false);
    char tmp;
    ::read(m_sigusr2Fd[1], &tmp, sizeof(tmp));

    sendSignal(SIGUSR2);

    m_usr2Notifier->setEnabled(true);
}

void ExonicCore::checkSignalsHandlersState()
{
    qInfo() << "Check signals handlers state";
    if (m_signalHandlersStarted < 1)
        m_application->exit(0);
}
