#ifndef TRAYSERVICE_H
#define TRAYSERVICE_H

#include <qabstractitemmodel.h>

typedef struct HWND__ *HWND;

class QWindow;
class QQmlEngine;
class QJSEngine;

struct TrayServicePrivate;
class TrayIcon;

class TrayService : public QAbstractItemModel {
    Q_OBJECT
    friend struct TrayServicePrivate;

public:
    enum ModelRoles {
        IdRole = Qt::UserRole + 1,
        TrayIconRole
    };

    static QObject *instance(QQmlEngine *, QJSEngine *);

    Q_INVOKABLE void init();
    Q_INVOKABLE void restoreSystemTaskbar();

private:
    explicit TrayService(QObject *parent = nullptr);
    ~TrayService();

    TrayServicePrivate *d;
    QVector<TrayIcon *> _trayIcons;
    QHash<HWND, TrayIcon *> _iconData;

    void taskBarCreated();
    TrayIcon *icon(HWND hwnd);
    void removeIcon(HWND hwnd);

protected:
    // QObject interface
    virtual void timerEvent(QTimerEvent *event) override;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // TRAYSERVICE_H
