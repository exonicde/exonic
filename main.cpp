#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QQmlContext>
#include <qtwebengineglobal.h>
#include <QDebug>
#include <QRect>
#include <QScreen>
#include <QStyleHints>

#include "exowebengineproperties.h"


int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("exonic");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCommandLineParser parser;

    parser.setApplicationDescription("Exonic helper");
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();
    QCommandLineOption urlOption(QStringList() << "url", QCoreApplication::translate("main", "Application url <url>."));
    parser.addOption(urlOption);
    QCommandLineOption appNameOption(QStringList() << "application-name", QCoreApplication::translate("main", "Application name <url>."));
    parser.addOption(appNameOption);
    parser.process(app);

    if (!parser.parse(QCoreApplication::arguments())) {
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

    ExoWebEngineProperties appProperties;
    appProperties.setUrl(parser.isSet(urlOption) ? parser.value(urlOption) : QString("https://www.google.com")); // QString("about:blank"));
    appProperties.setTitle(parser.isSet(appNameOption) ? parser.value(appNameOption) : QString("Exonic"));

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
