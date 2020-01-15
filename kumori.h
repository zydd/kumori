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

    static QString config(QString const& key);

    Q_INVOKABLE QQuickWindow *window();

    Q_INVOKABLE void ignoreAeroPeek(QQuickWindow *window);
    Q_INVOKABLE void drawOverDesktop(QQuickWindow *window);
    Q_INVOKABLE void drawUnderDesktop(QQuickWindow *window);

public slots:
    void clearComponentCache();
    void play_pause();

private:
    static Kumori *m_instance;

    QSettings m_config;
    QPointer<QQmlApplicationEngine> m_engine;
    QStringList m_args;

    void addConfig(QString key, QVariant const& defaultValue = {},
                   bool setDefault = false, bool ignoreArgs = false);

public: // QtCreator autocomplete
    enum Properties {
        appImportDir,
        userImportDir,
    };
    Q_ENUM(Properties)
};

#endif // KUMORI_H
