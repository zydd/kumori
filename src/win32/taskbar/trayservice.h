/**************************************************************************
 *  trayservice.h
 *
 *  Copyright 2024 Gabriel Machado
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 **************************************************************************/

#ifndef TRAYSERVICE_H
#define TRAYSERVICE_H

#include <qabstractitemmodel.h>
#include <qsortfilterproxymodel.h>

typedef struct HWND__ *HWND;

class QWindow;
class QQmlEngine;
class QJSEngine;

struct TrayServicePrivate;
class TrayIcon;
class TrayItemsProxy;


class TrayService : public QAbstractItemModel {
    Q_OBJECT
    friend struct TrayServicePrivate;

public:
    enum ModelRoles {
        IdRole = Qt::UserRole + 1,
        TrayIconRole
    };
    enum FilteringMode {
        VisibleIcons,
        HiddenIcons,
        ActionCenterIcons,
    };
    Q_ENUM(FilteringMode)

    static QObject *instance(QQmlEngine *, QJSEngine *);
    inline TrayIcon *iconAt(unsigned i) { return _trayIconList[i]; }

    Q_INVOKABLE void init();
    Q_INVOKABLE void restoreSystemTaskbar();
    Q_INVOKABLE TrayItemsProxy *proxy(FilteringMode mode);

private:
    explicit TrayService(QObject *parent = nullptr);
    ~TrayService();

    TrayServicePrivate *d;
    QVector<TrayIcon *> _trayIconList;
    QHash<HWND, TrayIcon *> _trayIconMap;

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

class TrayItemsProxy : public QSortFilterProxyModel {
    Q_OBJECT
public:
    inline TrayItemsProxy(TrayService::FilteringMode mode):
        _filterMode{mode} { }

private:
    TrayService::FilteringMode _filterMode;

protected:
    // QSortFilterProxyModel interface
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};


#endif // TRAYSERVICE_H
