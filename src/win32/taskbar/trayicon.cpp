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
    qDebug() << data.hWnd << "v:" << data.uVersion << "event:" << (void *) event;

    LPARAM lParam_h = 0;
    WPARAM wParam = data.uID;

    if (data.uVersion > 3) {
        lParam_h = data.uID;
        wParam = MAKEWPARAM(x, y);
    }

    {
        // Allow process to take focus so popups close when losing focus
        DWORD procId = 0;
        GetWindowThreadProcessId(data.hWnd, &procId);
        AllowSetForegroundWindow(procId);
    }

    SendNotifyMessage(data.hWnd, data.uCallbackMessage, wParam, MAKELPARAM(event, lParam_h));
}


TrayIconPainter::TrayIconPainter(QQuickItem *parent)
    : LiveIconPainter{parent}
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
}


void TrayIconPainter::mousePressEvent(QMouseEvent *event) {
    qDebug() << event;

    switch (event->button()) {
    case Qt::LeftButton:
        return trayIcon()->forwardMouseEvent(WM_LBUTTONDOWN, event->x(), event->y());
    case Qt::RightButton:
        return trayIcon()->forwardMouseEvent(WM_RBUTTONDOWN, event->x(), event->y());
    case Qt::MiddleButton:
        return trayIcon()->forwardMouseEvent(WM_MBUTTONDOWN, event->x(), event->y());
    default:
        qWarning() << "unexpected event";
        break;
    }

    QQuickPaintedItem::mousePressEvent(event);
}


void TrayIconPainter::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << event;

    switch (event->button()) {
    case Qt::LeftButton:
        trayIcon()->forwardMouseEvent(WM_LBUTTONUP, event->x(), event->y());

        if (trayIcon()->data.uVersion >= 3)
            trayIcon()->forwardMouseEvent(NIN_SELECT, event->x(), event->y());

        break;

    case Qt::RightButton:
        trayIcon()->forwardMouseEvent(WM_RBUTTONUP, event->x(), event->y());

        if (trayIcon()->data.uVersion >= 3)
            trayIcon()->forwardMouseEvent(WM_CONTEXTMENU, event->x(), event->y());

        break;

    case Qt::MiddleButton:
        return trayIcon()->forwardMouseEvent(WM_MBUTTONUP, event->x(), event->y());
    default:
        qWarning() << "unexpected event";
        break;
    }

    QQuickPaintedItem::mousePressEvent(event);
}


void TrayIconPainter::mouseDoubleClickEvent(QMouseEvent *event) {
    qDebug() << event;

    switch (event->button()) {
    case Qt::LeftButton:
        return trayIcon()->forwardMouseEvent(WM_LBUTTONDBLCLK, event->x(), event->y());
    case Qt::RightButton:
        return trayIcon()->forwardMouseEvent(WM_RBUTTONDBLCLK, event->x(), event->y());
    case Qt::MiddleButton:
        return trayIcon()->forwardMouseEvent(WM_MBUTTONDBLCLK, event->x(), event->y());
    default:
        qWarning() << "unexpected event";
        break;
    }

    QQuickPaintedItem::mousePressEvent(event);
}
