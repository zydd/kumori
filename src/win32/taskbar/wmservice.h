#ifndef WMSERVICE_H
#define WMSERVICE_H

#include <qobject.h>

class NativeWindow;
class QWindow;
class QQmlEngine;
class QJSEngine;

class WmService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> taskList READ taskList NOTIFY taskListChanged)

public:
    struct WmServicePrivate;
    static QObject *instance(QQmlEngine *, QJSEngine *);

    Q_INVOKABLE void init();
    Q_INVOKABLE QList<QObject*> taskList();

private:
    explicit WmService(QObject *parent = nullptr);
    ~WmService();

    WmServicePrivate *d;

signals:
    void taskListChanged();
};

#endif // WMSERVICE_H
