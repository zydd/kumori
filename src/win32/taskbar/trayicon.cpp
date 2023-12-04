#include "trayicon.h"

#include <Windows.h>


void TrayIcon::setTooltip(QString tooltip) {
    qDebug();

    if (_tooltip == tooltip)
        return;

    _tooltip = tooltip;
    emit tooltipChanged();
}

void TrayIcon::forwardMouseEvent(unsigned event, unsigned x, unsigned y) {
    LPARAM lParam_h = 0;
    WPARAM wParam = data.uID;

    if (data.uVersion >= NOTIFYICON_VERSION) {
        lParam_h = data.uID;
        wParam = MAKEWPARAM(x, y);
    }

    LPARAM lParam = MAKELPARAM(event, lParam_h);

    qDebug() << data.hWnd << "v:" << data.uVersion << "wParam:" << (void *) wParam << "lParam:" << (void *) lParam;

    SendNotifyMessage(data.hWnd, data.uCallbackMessage, wParam, lParam);
}


TrayIconPainter::TrayIconPainter(QQuickItem *parent)
    : LiveIconPainter{parent}
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
}


void TrayIconPainter::mousePressEvent(QMouseEvent *event) {
    qDebug() << event;

    trayIcon()->forwardMouseEvent(WM_MOUSEMOVE, event->globalX(), event->globalY());

    {
        // Allow process to take focus so popups close when losing focus
        DWORD procId = 0;
        GetWindowThreadProcessId(trayIcon()->data.hWnd, &procId);
        AllowSetForegroundWindow(procId);
    }

    switch (event->button()) {
    case Qt::LeftButton:
        trayIcon()->forwardMouseEvent(WM_LBUTTONDOWN, event->globalX(), event->globalY());
        break;

    case Qt::RightButton:
        trayIcon()->forwardMouseEvent(WM_RBUTTONDOWN, event->globalX(), event->globalY());
        break;

    case Qt::MiddleButton:
        trayIcon()->forwardMouseEvent(WM_MBUTTONDOWN, event->globalX(), event->globalY());
        break;

    default:
        qWarning() << "unexpected event";
        QQuickPaintedItem::mousePressEvent(event);
        break;
    }
}


void TrayIconPainter::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << event;

    switch (event->button()) {
    case Qt::LeftButton:
        trayIcon()->forwardMouseEvent(WM_LBUTTONUP, event->globalX(), event->globalY());

        if (trayIcon()->data.uVersion >= NOTIFYICON_VERSION)
            trayIcon()->forwardMouseEvent(NIN_SELECT, event->globalX(), event->globalY());

        break;

    case Qt::RightButton:
        trayIcon()->forwardMouseEvent(WM_RBUTTONUP, event->globalX(), event->globalY());

        if (trayIcon()->data.uVersion >= NOTIFYICON_VERSION)
            trayIcon()->forwardMouseEvent(WM_CONTEXTMENU, event->globalX(), event->globalY());

        break;

    case Qt::MiddleButton:
        trayIcon()->forwardMouseEvent(WM_MBUTTONUP, event->globalX(), event->globalY());
        break;

    default:
        qWarning() << "unexpected event";
        QQuickPaintedItem::mousePressEvent(event);
        break;
    }
}


void TrayIconPainter::mouseDoubleClickEvent(QMouseEvent *event) {
    qDebug() << event;

    switch (event->button()) {
    case Qt::LeftButton:
        trayIcon()->forwardMouseEvent(WM_LBUTTONDBLCLK, event->globalX(), event->globalY());
        break;
    case Qt::RightButton:
        trayIcon()->forwardMouseEvent(WM_RBUTTONDBLCLK, event->globalX(), event->globalY());
        break;
    case Qt::MiddleButton:
        trayIcon()->forwardMouseEvent(WM_MBUTTONDBLCLK, event->globalX(), event->globalY());
        break;
    default:
        qWarning() << "unexpected event";
        QQuickPaintedItem::mousePressEvent(event);
        break;
    }
}
