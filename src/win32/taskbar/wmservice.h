#ifndef WMSERVICE_H
#define WMSERVICE_H

#include <qabstractitemmodel.h>

#include "nativewindow.h"

class QWindow;
class QQmlEngine;
class QJSEngine;

class WmService : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(NativeWindow *startMenu READ startMenu NOTIFY startMenuChanged)

public:
    struct WmServicePrivate;
    static QObject *instance(QQmlEngine *, QJSEngine *);

    Q_INVOKABLE void init();
    inline NativeWindow *startMenu() const { return _startMenu; }

private:
    enum ModelRoles {
        IdRole = Qt::UserRole + 1,
        NativeWindowRole,
    };

    WmServicePrivate *d = nullptr;
    QVector<NativeWindow *> _listedWindows;
    QHash<HWND, NativeWindow *> _nativeWindows;
    NativeWindow *_activeWindow = nullptr;
    NativeWindow *_startMenu = nullptr;

    explicit WmService(QObject *parent = nullptr);
    ~WmService();

    void enumerateWindows();
    NativeWindow *window(HWND hwnd);
    void destroyWindow(HWND hwnd);
    void list(NativeWindow *wnd);
    void unlist(NativeWindow *wnd);

signals:
    void startMenuChanged();

public:
    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // WMSERVICE_H
