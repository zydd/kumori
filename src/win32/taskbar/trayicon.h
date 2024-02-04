/**************************************************************************
 *  trayicon.h
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

#pragma once

#include <array>

#include "quuid.h"

#include "liveicon.h"

typedef struct HWND__ *HWND;

class TrayIcon : public LiveIcon {
    Q_OBJECT
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged)

public:
    inline TrayIcon(): LiveIcon() { }

    inline QString tooltip() { return _tooltip; }
    inline QRect rect() { return _rect; }
    void setRect(QRect const& rect) {  _rect = rect; }
    void setTooltip(QString const& tooltip);
    void forwardMouseEvent(unsigned event, unsigned x, unsigned y);
    bool valid();

    struct Data {
        HWND hWnd;
        unsigned uID;
        unsigned uCallbackMessage;
        unsigned uVersion;
        QUuid guid;
    } data = {};

private:
    QString _tooltip;
    QRect _rect;

signals:
    void tooltipChanged();
    void invalidated();
};


class TrayIconPainter : public LiveIconPainter {
    Q_OBJECT
    Q_PROPERTY(Qt::MouseButtons acceptedButtons MEMBER _acceptedButtons NOTIFY acceptedButtonsChanged)

public:
    explicit TrayIconPainter(QQuickItem *parent = nullptr);

    inline TrayIcon *trayIcon() { return qobject_cast<TrayIcon *>(liveIcon()); }

private:
    Qt::MouseButtons _acceptedButtons = Qt::AllButtons;

protected:
    // QQuickItem interface
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
//    virtual void mouseMoveEvent(QMouseEvent *event) override;

    // QQuickItem interface
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

signals:
    void acceptedButtonsChanged();
};

