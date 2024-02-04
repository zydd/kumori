/**************************************************************************
 *  appbarwindow.cpp
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

#include "appbarwindow.h"

#include <qscreen.h>

#include <Windows.h>

#define WM_USER_NEW_APPBAR (WM_USER + 110011)

struct AppbarWindowPrivate {
    APPBARDATA trayPos;
};


AppbarWindow::AppbarWindow(QWindow *parent)
    : QQuickWindow{parent}
{
    d = new AppbarWindowPrivate;
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}


AppbarWindow::~AppbarWindow() {
    qDebug();
    unregisterAppbar();
    delete d;
}


bool AppbarWindow::nativeEvent(const QByteArray &/*eventType*/, void *message, long *result) {
    auto msg = reinterpret_cast<MSG *>(message);

    switch (msg->message) {
    case WM_USER_NEW_APPBAR:
        switch (msg->wParam) {
        case ABN_STATECHANGE:
            qDebug() << "ABN_STATECHANGE" << (void *) msg->lParam;
            break;

        case ABN_POSCHANGED:
            qDebug() << "ABN_POSCHANGED" << (void *) msg->lParam;
            updateGeometry();
            break;

        case ABN_FULLSCREENAPP:
            qDebug() << "ABN_FULLSCREENAPP" << (void *) msg->lParam;

            if (msg->lParam) {
                setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
            } else {
                setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                //FIXME: Qt::WindowStaysOnTopHint not enough?
                SetWindowPos(
                    reinterpret_cast<HWND>(winId()),
                    HWND_TOPMOST,
                    0, 0, 0, 0,
                    SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
            }
            break;

        case ABN_WINDOWARRANGE:  // lParam == true -> hide
            qDebug() << "ABN_WINDOWARRANGE" << (void *) msg->lParam;
            break;

        default:
            qWarning() << "ABN_UNKNOWN" << (void *) msg->wParam << (void *) msg->lParam;
            break;
        }

        *result = 1;
        return true;
    }

    return false;
}


void AppbarWindow::showEvent(QShowEvent *event) {
    registerAppbar();
    QQuickWindow::showEvent(event);
}


void AppbarWindow::unregisterAppbar() {
    if (!_registered)
        return;

    qDebug();

    auto hwnd = reinterpret_cast<HWND>(winId());
    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hwnd;
        SHAppBarMessage(ABM_REMOVE, &abd);
    }

    _registered = false;
}


void AppbarWindow::updateGeometry() {
    qDebug() << _edge << _thickness;
    QRect rect = {};

    auto const screenRect = screen()->geometry();
    auto const availableRect = screen()->availableGeometry();
    qDebug() << "screen rect:" << screenRect;
    qDebug() << "available:" << availableRect;

    switch (_edge) {
    case Qt::TopEdge:
    case Qt::BottomEdge:
        rect.setSize({screenRect.width(), _thickness});
        break;

    case Qt::LeftEdge:
    case Qt::RightEdge:
        rect.setSize({_thickness, availableRect.height()});
        break;

    default:
        qCritical() << "undefined edge" << _edge;
        return;
    }

    switch (_edge) {
    case Qt::TopEdge:
    case Qt::LeftEdge:
        rect.moveTopLeft({0, 0});
        break;

    case Qt::BottomEdge:
     case Qt::RightEdge:
        rect.moveBottomRight(screenRect.bottomRight());
        break;
    }

    qDebug() << "geometry:" << rect;

    d->trayPos.rc.top = rect.top();
    d->trayPos.rc.bottom = rect.bottom() + 1;
    d->trayPos.rc.left = rect.left();
    d->trayPos.rc.right = rect.right() + 1;

    auto res = SHAppBarMessage(ABM_QUERYPOS, &d->trayPos);

    rect.setLeft(d->trayPos.rc.left);
    rect.setRight(d->trayPos.rc.right - 1);
    rect.setTop(d->trayPos.rc.top);
    rect.setBottom(d->trayPos.rc.bottom - 1);

    qDebug() << "QUERYPOS" << res << rect << rect.normalized();

    switch (_edge) {
        case Qt::LeftEdge:      d->trayPos.rc.right = d->trayPos.rc.left + _thickness; break;
        case Qt::RightEdge:     d->trayPos.rc.left = d->trayPos.rc.right - _thickness; break;
        case Qt::TopEdge:       d->trayPos.rc.bottom = d->trayPos.rc.top + _thickness; break;
        case Qt::BottomEdge:    d->trayPos.rc.top = d->trayPos.rc.bottom - _thickness; break;
    }

    // FIXME: For some reason the returned rect returned by QUERYPOS in Win11 leaves a gap above the taskbar
    // move bottom of appbar to edge of available work area
    RECT workArea = {};
    auto success = SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    switch (_edge) {
    case Qt::BottomEdge: {
        int offset = workArea.bottom + (_registered ? _thickness : 0) - d->trayPos.rc.bottom;
        if (success && workArea.bottom < screenRect.height() && offset > 0) {
            qWarning() << "QUERYPOS bottom offset" << offset;
            d->trayPos.rc.top += offset;
            d->trayPos.rc.bottom += offset;
        }
        break;
    }
    case Qt::LeftEdge:
    case Qt::RightEdge: {
        int offset = workArea.bottom - d->trayPos.rc.bottom;
        if (success && workArea.bottom < screenRect.height() && offset > 0) {
            qWarning() << "QUERYPOS bottom offset" << offset;
            d->trayPos.rc.bottom += offset;
        }
        break;
    }
    case Qt::TopEdge: break;
    }

    rect.setLeft(d->trayPos.rc.left);
    rect.setRight(d->trayPos.rc.right - 1);
    rect.setTop(d->trayPos.rc.top);
    rect.setBottom(d->trayPos.rc.bottom - 1);

    setGeometry(rect);

    qDebug() << "SETPOS" << rect;
    SHAppBarMessage(ABM_SETPOS, &d->trayPos);
}


void AppbarWindow::setEdge(Qt::Edge newEdge) {
    if (_edge == newEdge)
        return;
    _edge = newEdge;

//    updateGeometry();

    emit edgeChanged();
}


void AppbarWindow::setThickness(int newThickness) {
    qDebug() << newThickness;

    if (_thickness == newThickness)
        return;
    _thickness = newThickness;

//    updateGeometry();

    emit thicknessChanged();
}


void AppbarWindow::registerAppbar() {
    qDebug();

    if (_registered) {
        qDebug() << "already initialized";
        return;
    }

    auto hwnd = reinterpret_cast<HWND>(winId());

    unsigned uEdge = -1;
    switch (_edge) {
    case Qt::TopEdge:       uEdge = ABE_TOP;        break;
    case Qt::BottomEdge:    uEdge = ABE_BOTTOM;     break;
    case Qt::LeftEdge:      uEdge = ABE_LEFT;       break;
    case Qt::RightEdge:     uEdge = ABE_RIGHT;      break;

    default:
        qCritical() << "undefined edge" << _edge;
        return;
    }

    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hwnd;
        abd.uEdge = uEdge;
        abd.uCallbackMessage = WM_USER_NEW_APPBAR;
        SHAppBarMessage(ABM_NEW, &abd);
    }

    d->trayPos = {};  // needed by updateGeometry
    d->trayPos.cbSize = sizeof(APPBARDATA);
    d->trayPos.hWnd = hwnd;
    d->trayPos.uEdge = uEdge;

    connect(this, SIGNAL(closing(QQuickCloseEvent *)), this, SLOT(unregisterAppbar()));

    updateGeometry();
    _registered = true;
}

