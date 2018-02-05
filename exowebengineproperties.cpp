#include "exowebengineproperties.h"
#include <QGuiApplication>

ExoWebEngineProperties::ExoWebEngineProperties(QGuiApplication *application, QObject *parent) : QObject(parent)
{
    m_application = application;
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
    if (a != m_url) {
        m_title = a;
        emit titleChanged();
    }
}

QString ExoWebEngineProperties::title() const
{
    return m_title;
}

void ExoWebEngineProperties::exonicTerminate()
{
    m_application->exit();
}
