#ifndef TRAYICON_H
#define TRAYICON_H

#include <qpixmap.h>
#include <qquickpainteditem.h>

#include "trayservice.h"


class TrayIcon : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY iconChanged)

public:
    inline TrayIcon() { }

    inline QString tooltip() { return _tooltip; }
    inline void setTooltip(QString tooltip) { this->_tooltip = tooltip; }

    inline QPixmap icon() { return _icon; }
    inline void setIcon(QPixmap const& icon) { this->_icon = icon; }

private:
    QString _tooltip;
    QPixmap _icon;

signals:
    void iconChanged();
};


class TrayIconPainter : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QObject *trayIcon WRITE setTrayIconObject)

public:
    inline TrayIconPainter(QQuickItem *parent = nullptr) :
        QQuickPaintedItem(parent)
    { }

    void setTrayIconObject(QObject *newTrayIconObject);
    virtual void paint(QPainter *painter) override;

private:
    TrayIcon *_trayIcon;
};

#endif // TRAYICON_H
