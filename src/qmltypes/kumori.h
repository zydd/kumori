#ifndef KUMORI_H
#define KUMORI_H

#include <qpointer.h>
#include <qqmlapplicationengine.h>
#include <qqmlpropertymap.h>
#include <qsettings.h>

class QQuickWindow;

class Kumori : public QQmlPropertyMap {
    Q_OBJECT

public:
    struct Group : public QQmlPropertyMap {
        QString m_prefix;
        QSettings m_config;
        Group(QString const& prefix):
            QQmlPropertyMap(this, nullptr),
            m_prefix(prefix)
        { }
        QVariant updateValue(const QString &key, const QVariant &input) override;
    };

    Kumori(QStringList args);

    static Kumori *instance() {
        Q_ASSERT(m_instance);
        return m_instance;
    }
    static QObject *instance(QQmlEngine *engine, QJSEngine *) {
        instance()->m_engine = qobject_cast<QQmlApplicationEngine *>(engine);
        engine->setObjectOwnership(instance(), QQmlEngine::CppOwnership);
        return instance();
    }

    static QVariant config(QString const& key);
    static QString string(QString const& key) {
        return config(key).toString(); }

    Q_INVOKABLE void ignoreAeroPeek(QQuickWindow *window);
    Q_INVOKABLE void drawOverDesktop(QQuickWindow *window);
    Q_INVOKABLE void drawUnderDesktop(QQuickWindow *window);
    Q_INVOKABLE void blurWindowBackground(QQuickWindow *window);

    Q_INVOKABLE void addConfig(QString const& key, QVariant const& defaultValue) {
        addConfig(key, defaultValue, false, false); }

    Q_INVOKABLE QByteArray readFile(QUrl url);
    Q_INVOKABLE bool appendLog(QUrl url, QString text);

    Q_INVOKABLE void hideTaskbar();
    Q_INVOKABLE void showTaskbar();
    Q_INVOKABLE void actionCenter();
    Q_INVOKABLE void notificationArea();
    Q_INVOKABLE void startMenu();

public slots:
    void clearComponentCache();
    void playPause();

private:
    static Kumori *m_instance;

    QSettings m_config;
    QPointer<QQmlApplicationEngine> m_engine;
    QStringList m_args;

    void addConfig(QString const& key, QVariant const& defaultValue,
                   bool setDefault, bool ignoreArgs = false);

protected:
    QVariant updateValue(const QString &key, const QVariant &input) override;

public: // QtCreator autocomplete
    enum Properties {
        appImportDir,
        userImportDir,
    };
    Q_ENUM(Properties)
};

#endif // KUMORI_H
