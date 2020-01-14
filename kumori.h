#ifndef KUMORI_H
#define KUMORI_H

#include <qpointer.h>
#include <qqmlengine.h>
#include <qsettings.h>

class Kumori : public QObject {
    Q_OBJECT
public:
    static Kumori *instance() {
        if (! m_instance)
            m_instance = new Kumori;
        return m_instance;
    }
    static QObject *instance(QQmlEngine *engine, QJSEngine *) {
        instance()->m_engine = engine;
        return instance();
    }

    Q_INVOKABLE QString userImportDir();

public slots:
    void clearComponentCache();

private:
    QSettings m_config;
    QPointer<QQmlEngine> m_engine;

    static Kumori *m_instance;

    Kumori() { }
};

#endif // KUMORI_H
