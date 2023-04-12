#ifndef WMSERVICE_H
#define WMSERVICE_H

#include <QObject>

class NativeWindow;

class WmService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> taskList READ taskList NOTIFY taskListChanged)

public:
    explicit WmService(QObject *parent = nullptr);

    Q_INVOKABLE QList<QObject*> taskList();

signals:
    void taskListChanged();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
};

#endif // WMSERVICE_H
