#ifndef NATIVEWINDOW_H
#define NATIVEWINDOW_H

#include <qobject.h>

typedef struct HWND__ *HWND;

class NativeWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit NativeWindow(HWND hwnd, QObject *parent = nullptr);

    bool canAddToTaskbar();

    Q_INVOKABLE void toFront();
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void maximize();
    bool minimized();
    int showStyle();

    HWND hwnd() { return _hwnd; }
    QString title();
    bool active() const { return _active; }

    void setActive(bool newActive);

private:
    HWND _hwnd;
    bool _active = false;

    void makeForeground();

signals:
    void titleChanged();
    void activeChanged();
};

#endif // NATIVEWINDOW_H
