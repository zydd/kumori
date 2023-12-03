#pragma once

#include "liveicon.h"

typedef struct HWND__ *HWND;

class TrayIcon : public LiveIcon {
    Q_OBJECT
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged)

public:
    inline TrayIcon(): LiveIcon() { }

    inline QString tooltip() { return _tooltip; }
    void setTooltip(QString tooltip);
    void forwardMouseEvent(unsigned event, unsigned x, unsigned y);

    struct Data {
        HWND hWnd;
        unsigned uID;
        unsigned uCallbackMessage;
        unsigned uVersion;
    } data;

private:
    QString _tooltip;

signals:
    void tooltipChanged();
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
};

