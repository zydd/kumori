#ifndef KUMORI_H
#define KUMORI_H

#include <qpointer.h>
#include <qqmlengine.h>
#include <qqmlpropertymap.h>
#include <qsettings.h>

class Kumori : public QQmlPropertyMap {
    Q_OBJECT

public:
    Kumori(QStringList args);

    static Kumori *instance() {
        Q_ASSERT(m_instance);
        return m_instance;
    }
    static QObject *instance(QQmlEngine *engine, QJSEngine *) {
        instance()->m_engine = engine;
        return instance();
    }

    static QString config(QString const& key);

public slots:
    void clearComponentCache();

private:
    static Kumori *m_instance;

    QSettings m_config;
    QPointer<QQmlEngine> m_engine;
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
