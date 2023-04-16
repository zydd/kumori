#ifndef WMSERVICE_H
#define WMSERVICE_H

#include <qabstractitemmodel.h>

#include "nativewindow.h"

class QWindow;
class QQmlEngine;
class QJSEngine;

class WmService : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> taskList READ taskList NOTIFY taskListChanged)

public:
    struct WmServicePrivate;
    static QObject *instance(QQmlEngine *, QJSEngine *);

    Q_INVOKABLE void init();
    Q_INVOKABLE QList<QObject*> taskList();

private:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NativeWindowRole,
    };

    explicit WmService(QObject *parent = nullptr);
    ~WmService();

    WmServicePrivate *d = nullptr;
    QHash<int, QByteArray> _roleNames;
    QVector<NativeWindow *> _windowList;
    QHash<HWND, int> _hwndIndex;
    NativeWindow *_activeWindow = nullptr;

    void enumerateWindows();

signals:
    void taskListChanged();

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // WMSERVICE_H
