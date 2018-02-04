#ifndef EXOWEBENGINEPROPERTIES_H
#define EXOWEBENGINEPROPERTIES_H

#include <QObject>

class ExoWebEngineProperties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
public:
    explicit ExoWebEngineProperties(QObject *parent = nullptr);

    void setUrl(const QString &a);
    void setTitle(const QString &a);
    QString url() const;
    QString title() const;

signals:
    void urlChanged();
    void titleChanged();
public slots:

signals:

private:
    QString m_url;
    QString m_title;
};

#endif // EXOWEBENGINEPROPERTIES_H