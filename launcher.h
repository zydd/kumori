#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <qquickimageprovider.h>
#include <qvariant.h>

class Launcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList programs READ programs NOTIFY programsChanged)

public:
    class IconProvider : public QQuickImageProvider {
        friend class Launcher;
    public:
        QPixmap requestPixmap(const QString &id, QSize *size,
                              const QSize &requestedSize) override;
    private:
        IconProvider(Launcher *launcher);
        Launcher *launcher;
    };

    explicit Launcher(QObject *parent = nullptr);

    inline IconProvider *iconProvider() {
        return m_iconProvider; }

    inline QStringList programs() const {
        return m_programs; }

public slots:
    void launch(QString program);
    void reloadConfig();

signals:
    void programsChanged();

private:
    IconProvider *m_iconProvider;
    QMap<QString, QVariantMap> m_shortcuts;
    QStringList m_programs;
    QStringList m_desktop;
};

#endif // LAUNCHER_H
