#include "trayicon.h"

#include <Windows.h>


void TrayIcon::setTooltip(QString const& tooltip) {
//    qDebug() << data.hWnd << tooltip;

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

//    qDebug() << "********"
//             << "hWnd"      << data.hWnd
//             << "uCallbackMessage" << data.uCallbackMessage
//             << "uID"       << data.uID
//             << "uVersion"  << data.uVersion;

    qDebug() << data.hWnd << "v:" << data.uVersion << "wParam:" << (void *) wParam << "lParam:" << (void *) lParam;

    SendNotifyMessage(data.hWnd, data.uCallbackMessage, wParam, lParam);
}


bool TrayIcon::valid() {
    bool valid = IsWindow(data.hWnd);
    if (!valid) {
        qDebug() << valid;
        emit invalidated();
    }
    return valid;
}


TrayIconPainter::TrayIconPainter(QQuickItem *parent)
    : LiveIconPainter{parent}
{
//    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    connect(this, &LiveIconPainter::liveIconChanged, [this]{
        auto geometry = QRectF{position(), size()};
        geometryChanged(geometry, geometry);
    });
}


void TrayIconPainter::mousePressEvent(QMouseEvent *event) {
    qDebug() << event;

    if (!trayIcon()->valid()) return;

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
//        trayIcon()->forwardMouseEvent(WM_LBUTTONDOWN, event->globalX(), event->globalY());
        trayIcon()->forwardMouseEvent(WM_LBUTTONUP, event->globalX(), event->globalY());

        if (trayIcon()->data.uVersion >= NOTIFYICON_VERSION)
            trayIcon()->forwardMouseEvent(NIN_SELECT, event->globalX(), event->globalY());

        break;

    case Qt::RightButton:
//        trayIcon()->forwardMouseEvent(WM_RBUTTONDOWN, event->globalX(), event->globalY());
        trayIcon()->forwardMouseEvent(WM_RBUTTONUP, event->globalX(), event->globalY());

        if (trayIcon()->data.uVersion >= NOTIFYICON_VERSION)
            trayIcon()->forwardMouseEvent(WM_CONTEXTMENU, event->globalX(), event->globalY());

        break;

    case Qt::MiddleButton:
//        trayIcon()->forwardMouseEvent(WM_MBUTTONDOWN, event->globalX(), event->globalY());
        trayIcon()->forwardMouseEvent(WM_MBUTTONUP, event->globalX(), event->globalY());
        break;

    default:
        qWarning() << "unexpected event";
        QQuickPaintedItem::mouseReleaseEvent(event);
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
        QQuickPaintedItem::mouseDoubleClickEvent(event);
        break;
    }
}

void TrayIconPainter::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) {
    if (trayIcon()) {
        auto topLeft = mapToGlobal(newGeometry.topLeft());
        trayIcon()->setRect(QRect(topLeft.toPoint(), newGeometry.size().toSize()));
    }
    LiveIconPainter::geometryChanged(newGeometry, oldGeometry);
}

//void TrayIconPainter::mouseMoveEvent(QMouseEvent *event) {
//    qDebug() << event;
//    QQuickPaintedItem::mouseMoveEvent(event);
//}
