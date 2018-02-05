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

#include "exowebengineproperties.h"


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
    QCommandLineOption appManagerHost("manager-host", app.translate("main", "Applications manager host <host>., default is localhost."),
                                      app.translate("main", "manager-host"), "localhost");
    QCommandLineOption appManagerPort("manager-port", app.translate("main", "Applications manager port <port>., default is 9090."),
                                      app.translate("main", "manager-port"), "9090");
    QCommandLineOption pidLocation("pid-location", app.translate("main", "Application PID path <path>., default is current directory."),
                                      app.translate("main", "pid-location"), ".");
    parser.addOption(urlOption);
    parser.addOption(appNameOption);
    parser.addOption(loadFromFile);
    parser.addOption(appManagerHost);
    parser.addOption(appManagerPort);
    parser.addOption(pidLocation);
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

    QFile file(parser.value(pidLocation) + QString("/") + QString::number(app.applicationPid()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        qWarning() << "Could not write pid file";
    else {
        QTextStream out(&file);
        out << app.applicationPid();
    }
    file.close();

    ExoWebEngineProperties appProperties(&app);
    if (parser.isSet(loadFromFile))
        appProperties.setUrl(QUrl::fromLocalFile(parser.value(urlOption)).toString());
    else
        appProperties.setUrl(parser.value(urlOption));

    appProperties.setTitle(parser.value(appNameOption));

    QtWebEngine::initialize();

    QQmlApplicationEngine engine;
    QQmlContext * context = engine.rootContext();
    context->setContextProperty("appProperties", &appProperties);

    QRect geometry = QGuiApplication::primaryScreen()->availableGeometry();
    if (!QGuiApplication::styleHints()->showIsFullScreen()) {
        const QSize size = geometry.size() * 4 / 5;
        const QSize offset = (geometry.size() - size) / 2;
        const QPoint pos = geometry.topLeft() + QPoint(offset.width(), offset.height());
        geometry = QRect(pos, size);
    }
    context->setContextProperty(QStringLiteral("initialX"), geometry.x());
    context->setContextProperty(QStringLiteral("initialY"), geometry.y());
    context->setContextProperty(QStringLiteral("initialWidth"), geometry.width());
    context->setContextProperty(QStringLiteral("initialHeight"), geometry.height());

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
