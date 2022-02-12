#include <csignal>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QQmlContext>
#include <qtwebengineglobal.h>
#include <QDebug>
#include <QRect>
#include <QScreen>
#include <QStyleHints>
#include <QFile>
#include <QTextStream>

#include "exoniccore.h"


int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication app(argc, argv);
    app.setApplicationName("exonic");
    app.setApplicationVersion("0.0.1");
    QCommandLineParser parser;

    parser.setApplicationDescription("Exonic helper");
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();
    QCommandLineOption urlOption("url", app.translate("main", "Application url <url>."), app.translate("main", "url"), "about:blank");
    QCommandLineOption appNameOption("application-name", app.translate("main", "Application name <name>."),
                                     app.translate("main", "application-name"), "Exonic");
    QCommandLineOption loadFromFile({"f", "from-file"}, app.translate("main", "Load application from local file."));
    QCommandLineOption loadQML({"q", "qml"}, app.translate("main", "Load QML."));
    QCommandLineOption appManagerHost("manager-host", app.translate("main", "Applications manager host <host>., default is localhost."),
                                      app.translate("main", "manager-host"), "localhost");
    QCommandLineOption appManagerPort("manager-port", app.translate("main", "Applications manager port <port>., default is 9090."),
                                      app.translate("main", "manager-port"), "9090");
    QCommandLineOption pidLocation("pid-location", app.translate("main", "Application PID path <path>., default is current directory."),
                                      app.translate("main", "pid-location"), ".");
    QCommandLineOption  disableSeccompFilterSandbox("disable-seccomp-filter-sandbox", app.translate("main", "QWebEngine param."));
    QCommandLineOption disableVirtualKeyboard("disable-virtual-keyboard", app.translate("main", "Disable virtual keyboard flag."));
    parser.addOption(urlOption);
    parser.addOption(appNameOption);
    parser.addOption(loadFromFile);
    parser.addOption(loadQML);
    parser.addOption(appManagerHost);
    parser.addOption(appManagerPort);
    parser.addOption(pidLocation);
    parser.addOption(disableSeccompFilterSandbox);
    parser.addOption(disableVirtualKeyboard);
    parser.process(app);

    qInfo() << app.arguments();

    if (!parser.parse(app.arguments())) {
        qCritical() << "Invalid command line";
        return 1;
    }

    if (parser.isSet(versionOption)) {
        qInfo() << versionOption.description();
        return 0;
    }

    if (parser.isSet(helpOption)) {
        qInfo() << helpOption.description();
        return 0;
    }

//    QFile file(parser.value(pidLocation) + QString("/") + QString::number(app.applicationPid()));
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//        qWarning() << "Could not write pid file";
//    else {
//        QTextStream out(&file);
//        out << app.applicationPid();
//    }
//    file.close();

    ExonicCore exonicCore(&app);

    int code = ExonicCore::setUnixSignalHandlers();
    if (code)
        qWarning() << "Cannot use all unix signals" << code;

    if (parser.isSet(loadFromFile))
        exonicCore.setUrl(QUrl::fromLocalFile(parser.value(urlOption)).toString());
    else
        exonicCore.setUrl(parser.value(urlOption));

    exonicCore.setTitle(parser.value(appNameOption));

    if (parser.isSet(disableVirtualKeyboard))
        exonicCore.setVirtualKeyboard(false);

    QtWebEngine::initialize();

    QQmlApplicationEngine engine;
    QQmlContext * context = engine.rootContext();
    context->setContextProperty("exonicCore", &exonicCore);

    QRect geometry = QGuiApplication::primaryScreen()->availableGeometry();
    if (!QGuiApplication::styleHints()->showIsFullScreen()) {
        const QSize size = geometry.size() * 3 / 5;
        const QSize offset = (geometry.size() - size) / 2;
        const QPoint pos = geometry.topLeft() + QPoint(offset.width(), offset.height());
        geometry = QRect(pos, size);
    }
    context->setContextProperty(QStringLiteral("initialX"), geometry.x());
    context->setContextProperty(QStringLiteral("initialY"), geometry.y());
    context->setContextProperty(QStringLiteral("initialWidth"), geometry.width());
    context->setContextProperty(QStringLiteral("initialHeight"), geometry.height());

    if (parser.isSet(loadQML))
        engine.load(exonicCore.url());
    else 
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
