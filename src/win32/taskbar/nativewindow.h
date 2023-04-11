#ifndef NATIVEWINDOW_H
#define NATIVEWINDOW_H

#include <QObject>

typedef struct HWND__ *HWND;

class NativeWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

public:
    explicit NativeWindow(HWND hwnd, QObject *parent = nullptr);

    bool canAddToTaskbar();
    QString title();

private:
    HWND hwnd;

signals:
    void titleChanged();
};

#endif // NATIVEWINDOW_H
