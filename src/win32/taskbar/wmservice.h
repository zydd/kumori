#ifndef WMSERVICE_H
#define WMSERVICE_H

#include <QObject>

class NativeWindow;

class WmService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> taskList READ taskList NOTIFY taskListChanged)

public:

    explicit WmService(QObject *parent = nullptr);

    Q_INVOKABLE QList<QObject*> getOpenWindows();

    inline const QList<QObject *> &taskList() { return _taskList; }

signals:

    void taskListChanged();
private:
    QList<QObject *> _taskList;
};

#endif // WMSERVICE_H
