#pragma once

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
    } data;

private:
    QString _tooltip;
    QRect _rect;

signals:
    void tooltipChanged();
    void invalidated();
};


class TrayIconPainter : public LiveIconPainter {
    Q_OBJECT
public:
    explicit TrayIconPainter(QQuickItem *parent = nullptr);

    inline TrayIcon *trayIcon() { return qobject_cast<TrayIcon *>(liveIcon()); }

protected:
    // QQuickItem interface
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
//    virtual void mouseMoveEvent(QMouseEvent *event) override;

    // QQuickItem interface
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
};

