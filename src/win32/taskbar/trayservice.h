#ifndef TRAYSERVICE_H
#define TRAYSERVICE_H

#include <QObject>

class QWindow;
class QQmlEngine;
class QJSEngine;

class TrayService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> trayItems READ trayItems NOTIFY trayItemsChanged)

public:
    struct TrayServicePrivate;
    static QObject *instance(QQmlEngine *, QJSEngine *);

    QObjectList trayItems();
    Q_INVOKABLE void setTaskBar(QWindow *window);

private:
    explicit TrayService(QObject *parent = nullptr);
    ~TrayService();
    void init();

    TrayServicePrivate *d;

signals:
    void trayItemsChanged();

protected:
//    virtual void timerEvent(QTimerEvent *event) override;
};

#endif // TRAYSERVICE_H
