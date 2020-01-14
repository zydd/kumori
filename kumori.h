#ifndef KUMORI_H
#define KUMORI_H

#include <qsettings.h>

class QQmlEngine;
class QJSEngine;

class Kumori : public QObject {
    Q_OBJECT
public:
    static Kumori *instance() {
        if (! m_instance)
            m_instance = new Kumori;
        return m_instance;
    }
    static QObject *instance(QQmlEngine *, QJSEngine *) {
        return instance(); }

    Q_INVOKABLE QString userImportDir();

private:
    QSettings m_config;

    static Kumori *m_instance;

    Kumori() { }
};

#endif // KUMORI_H
