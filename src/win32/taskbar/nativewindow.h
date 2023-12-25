#ifndef NATIVEWINDOW_H
#define NATIVEWINDOW_H

#include <qobject.h>
#include <qpixmap.h>

typedef struct HWND__ *HWND;

class LiveIcon;

class NativeWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(LiveIcon *icon READ icon CONSTANT)

public:
    enum WindowRole {
        NormalWindow,
        ShellWindow,
        StartMenuWindow,
        ActionCenterWindow,
        NotificationCenterWindow,
    };
    explicit NativeWindow(HWND hwnd, QObject *parent = nullptr);
    ~NativeWindow();

    Q_INVOKABLE void toFront();
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void maximize();
    bool minimized();
    int showStyle();
    void loadIcon();
    bool canAddToTaskbar();
    bool cloaked();

    QString title();
    inline bool listed() const { return _listed; }
    inline HWND hwnd() const { return _hwnd; }
    inline bool active() const { return _active; }
    inline LiveIcon *icon() { return _icon; }
    inline QString const& windowClass() const { return _windowClass; }
    inline QString const& processName() const { return _processName; }
    inline WindowRole role() const { return _role; }

    void setActive(bool newActive);
    inline void setListed(bool listed) { _listed = listed; }

private:
    HWND _hwnd;
    bool _active = false;
    LiveIcon *_icon = nullptr;
    bool _listed = false;
    QString _windowClass;
    QString _processName;
    WindowRole _role = NormalWindow;

    void makeForeground();

signals:
    void titleChanged();
    void activeChanged();
};

#endif // NATIVEWINDOW_H
