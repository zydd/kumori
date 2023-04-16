#ifndef NATIVEWINDOW_H
#define NATIVEWINDOW_H

#include <qobject.h>

typedef struct HWND__ *HWND;

class NativeWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

public:
    explicit NativeWindow(HWND hwnd, QObject *parent = nullptr);

    bool canAddToTaskbar();
    QString title();

    Q_INVOKABLE void toFront();
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void maximize();
    bool minimized();
    int showStyle();

private:
    HWND hwnd;

    void makeForeground();

signals:
    void titleChanged();
};

#endif // NATIVEWINDOW_H
