#ifndef NATIVEWINDOW_H
#define NATIVEWINDOW_H

#include <qobject.h>
#include <qpixmap.h>

typedef struct HWND__ *HWND;

class TrayIcon;

class NativeWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(TrayIcon *icon READ icon CONSTANT)

public:
    explicit NativeWindow(HWND hwnd, QObject *parent = nullptr);
    ~NativeWindow();

    bool canAddToTaskbar();

    Q_INVOKABLE void toFront();
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void maximize();
    bool minimized();
    int showStyle();
    void loadIcon();

    HWND hwnd() { return _hwnd; }
    QString title();
    bool active() const { return _active; }
    TrayIcon *icon() { return _icon; }

    void setActive(bool newActive);

private:
    HWND _hwnd;
    bool _active = false;
    TrayIcon *_icon = nullptr;

    void makeForeground();

signals:
    void titleChanged();
    void activeChanged();
};

#endif // NATIVEWINDOW_H
