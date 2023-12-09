#ifndef TRAYSERVICE_H
#define TRAYSERVICE_H

#include <QObject>

class QWindow;
class QQmlEngine;
class QJSEngine;

struct TrayServicePrivate;

class TrayService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> trayItems READ trayItems NOTIFY trayItemsChanged)
    friend struct TrayServicePrivate;

public:
    static QObject *instance(QQmlEngine *, QJSEngine *);

    Q_INVOKABLE void init();
    QObjectList trayItems();
    Q_INVOKABLE void restoreSystemTaskbar();

private:
    explicit TrayService(QObject *parent = nullptr);
    ~TrayService();

    TrayServicePrivate *d;

    void taskBarCreated();

signals:
    void trayItemsChanged();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
};

#endif // TRAYSERVICE_H
