#include "wmservice.h"
#include "nativewindow.h"

#include <qdebug.h>
#include <qlibrary.h>
#include <qpointer.h>
#include <qwinfunctions.h>

#include <Windows.h>
#include <WinUser.h>


using WmServicePrivate = WmService::WmServicePrivate;

struct WmService::WmServicePrivate {
    bool initialized = false;

    HINSTANCE hInstance;
    HWND hwndTaskm;
    HWND hwndTray;
    HWND hwndSystemTray;

    static struct {
        int WM_SHELLHOOKMESSAGE;
        int WM_TASKBARCREATEDMESSAGE;
        int TASKBARBUTTONCREATEDMESSAGE;
    } staticData;

    ushort registerWindowClass(LPCWSTR name);
    HWND createTaskmanWindow();
    void destroyTaskmanWindow();

    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

static QPointer<WmService> wmService = nullptr;

decltype(WmServicePrivate::staticData) WmServicePrivate::staticData;


WmService::WmService(QObject *parent)
    : QAbstractItemModel{parent}
{
    qDebug();
    d = new WmServicePrivate();

    _roleNames[Roles::IdRole] = "id";
    _roleNames[Roles::NativeWindowRole] = "nativeWindow";
}


WmService::~WmService() {
    qDebug();
    d->destroyTaskmanWindow();
    delete this->d;
}

QModelIndex WmService::index(int row, int column, const QModelIndex &parent) const {
    return createIndex(row, column);
}

QModelIndex WmService::parent(const QModelIndex &child) const {
    return {};
}

int WmService::rowCount(const QModelIndex &parent) const {
    return _listedWindows.size();
}

int WmService::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant WmService::data(const QModelIndex &index, int role) const {
    Q_ASSERT(index.row() < _listedWindows.size());

    switch (role) {
    case IdRole:
        return 0;
    case NativeWindowRole:
        return QVariant::fromValue(_listedWindows[index.row()]);
    }

    Q_ASSERT(false);
    return {};
}

QHash<int, QByteArray> WmService::roleNames() const {
    return _roleNames;
}


QObject *WmService::instance(QQmlEngine *, QJSEngine *) {
    qDebug() << ::wmService;

    if (!::wmService) {
        ::wmService = new WmService();
        qDebug() << "new:" << ::wmService;
    }

    return ::wmService;
}


void WmService::init() {
    qDebug();

    if (d->initialized) {
        qDebug() << "already initialized";
        return;
    }

    d->hInstance = GetModuleHandle(nullptr);

    // TODO: check if there are multiple instances of Shell_TrayWnd and fail
    d->hwndTray = FindWindow(L"Shell_TrayWnd", NULL);
    d->hwndSystemTray = FindWindowEx(NULL, d->hwndTray, L"Shell_TrayWnd", NULL);

    if (!d->hwndTray || !d->hwndSystemTray) {
        qWarning() << "could not get tray handles";
        return;
    }

    d->staticData.WM_SHELLHOOKMESSAGE = RegisterWindowMessage(L"SHELLHOOK");
    d->staticData.WM_TASKBARCREATEDMESSAGE = RegisterWindowMessage(L"TaskbarCreated");
    d->staticData.TASKBARBUTTONCREATEDMESSAGE = RegisterWindowMessage(L"TaskbarButtonCreated");

    auto hwndTaskm = d->createTaskmanWindow();

    {
        QLibrary lib("user32");
        lib.load();

        BOOL (*SetTaskmanWindow)(HWND);
        SetTaskmanWindow = (decltype(SetTaskmanWindow)) lib.resolve("SetTaskmanWindow");
        if (SetTaskmanWindow)
            SetTaskmanWindow(hwndTaskm);
        else
            qWarning() << "could not get pointer to SetTaskmanWindow";
    }

    RegisterShellHookWindow(hwndTaskm);

    RemoveProp(d->hwndSystemTray, L"TaskbandHWND");
    SetProp(d->hwndTray, L"TaskbandHWND", hwndTaskm);

    enumerateWindows();

    d->initialized = true;
    qDebug() << "done";
}


HWND WmServicePrivate::createTaskmanWindow() {
    qDebug();

    ushort trayNotifyClassReg = registerWindowClass(L"KumoriTaskmanWindow");
    if (trayNotifyClassReg == 0) {
        auto err = GetLastError();
        qCritical() << "could not register Shell_TrayWnd class:"
                    << err
                    << QtWin::errorStringFromHresult(err)
                    << QtWin::stringFromHresult(err);
        return nullptr;
    }
    hwndTaskm = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // extended window style
        L"KumoriTaskmanWindow", // window class name
        L"", // window title
        WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // window style
        0, // x position
        0, // y position
        0, // width
        0, // height
        NULL, // parent window handle
        NULL, // menu handle
        hInstance, // application instance handle
        NULL); // creation parameters

    if (hwndTaskm == nullptr) {
        auto err = GetLastError();
        qCritical() << "could not create KumoriTaskmanWindow window:"
                    << err
                    << QtWin::errorStringFromHresult(err).toUtf8()
                     << QtWin::stringFromHresult(err).toUtf8();
        return nullptr;
    }

    return hwndTaskm;
}


void WmServicePrivate::destroyTaskmanWindow() {
    qDebug();
    DestroyWindow(hwndTaskm);
    UnregisterClass(L"KumoriTaskmanWindow", hInstance);
}


ushort WmServicePrivate::registerWindowClass(LPCWSTR name) {
    qDebug() << QString::fromWCharArray(name);

    WNDCLASS newClass = {};
    newClass.lpszClassName = name;
    newClass.hInstance = hInstance;
    newClass.style = 0x8;
    newClass.lpfnWndProc = WmServicePrivate::wndProc;

    return RegisterClass(&newClass);
}


LRESULT CALLBACK WmServicePrivate::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto hwndParam = reinterpret_cast<HWND>(lParam);

    if (msg == staticData.WM_SHELLHOOKMESSAGE) {
        switch (wParam) {
        case HSHELL_GETMINRECT:
            qDebug() << "GETMINRECT" << hwndParam;
            break;
        case HSHELL_WINDOWACTIVATED:
            qDebug() << "WINDOWACTIVATED" << hwndParam;
            break;
        case HSHELL_RUDEAPPACTIVATED: {
            qDebug() << "RUDEAPPACTIVATED" << hwndParam;
            auto wnd = wmService->window(hwndParam);

//            if (!wnd->listed() && wnd->canAddToTaskbar())
//                wmService->list(wnd);

            if (wmService->_activeWindow)
                wmService->_activeWindow->setActive(false);

            wmService->_activeWindow = wnd;
            wnd->setActive(true);

            break;
        }
        case HSHELL_WINDOWREPLACING:
            qDebug() << "WINDOWREPLACING" << hwndParam;
            break;
        case HSHELL_WINDOWREPLACED:
            qDebug() << "WINDOWREPLACED" << hwndParam;
            break;
        case HSHELL_WINDOWCREATED: {
            qDebug() << "WINDOWCREATED" << hwndParam;
            wmService->window(hwndParam);
            break;
        }
        case HSHELL_WINDOWDESTROYED: {
            qDebug() << "WINDOWDESTROYED" << hwndParam;
            wmService->destroyWindow(hwndParam);
            break;
        }
        case HSHELL_ACTIVATESHELLWINDOW:
            qDebug() << "ACTIVATESHELLWINDOW" << hwndParam;
            break;
        case HSHELL_TASKMAN:
            qDebug() << "TASKMAN" << hwndParam;
            break;
        case HSHELL_REDRAW: {
            qDebug() << "REDRAW" << hwndParam;
            auto wnd = wmService->window(hwndParam);

            emit wnd->titleChanged();

            qDebug() << "listed:" << wnd->listed();

            if (wnd->listed() != wnd->canAddToTaskbar()) {
                qDebug() << "list status changed:" << wnd->listed();
                if (wnd->listed()) {
                    wmService->unlist(wnd);
                } else {
                    wmService->list(wnd);
                }
            }
        }
            break;
        case HSHELL_FLASH:
            qDebug() << "FLASH" << hwndParam;
            break;
        case HSHELL_ENDTASK:
            qDebug() << "ENDTASK" << hwndParam;
            break;
        case HSHELL_APPCOMMAND:
            qDebug() << "APPCOMMAND" << hwndParam;
            break;
        case HSHELL_MONITORCHANGED :
            qDebug() << "MONITORCHANGED " << hwndParam;
            break;

        default:
            qWarning() << "unhandled event" << wParam << lParam;
            break;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}


NativeWindow *WmService::window(HWND hwnd) {
    auto wnd = _nativeWindows.value(hwnd);
    if (!wnd) {
        wnd = new NativeWindow{hwnd};
        _nativeWindows[hwnd] = wnd;

//        if (wnd->canAddToTaskbar())
//            list(wnd);
    }
    return wnd;
}


void WmService::destroyWindow(HWND hwnd) {
    auto wnd = _nativeWindows.take(hwnd);

    if (wnd && wnd->listed())
        unlist(wnd);

    if (_activeWindow == wnd)
        _activeWindow = nullptr;

    delete wnd;
}

void WmService::list(NativeWindow *wnd) {
    qDebug() << wnd;
    Q_ASSERT(!wnd->listed());

    auto index = _listedWindows.size();
    beginInsertRows({}, index, index);
    _listedWindows.push_back(wnd);
    endInsertRows();

    wnd->setListed(true);
}

void WmService::unlist(NativeWindow *wnd) {
    qDebug() << wnd;
    Q_ASSERT(wnd->listed());

    auto index = std::distance(_listedWindows.begin(), std::find(_listedWindows.begin(), _listedWindows.end(), wnd));
    if (index >= _listedWindows.size()) {
        qWarning() << "not found in list:" << wnd;
        return;
    }

    beginRemoveRows({}, index, index);
    _listedWindows.remove(index);
    endRemoveRows();

    wnd->setListed(false);
}


void WmService::enumerateWindows() {
    qDebug();
    Q_ASSERT(_listedWindows.size() == 0);

    struct WindowList {
        QVector<NativeWindow *> listedWindows;
        QHash<HWND, NativeWindow *> nativeWindows;
    };

    WindowList windows;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto *data = reinterpret_cast<WindowList *>(lParam);
        auto wnd = new NativeWindow{hwnd};
        data->nativeWindows[hwnd] = wnd;

        if (wnd->canAddToTaskbar() && !wnd->cloaked()) {
            qDebug() << data->listedWindows.size() << hwnd << wnd->title();
            wnd->setListed(true);
            data->listedWindows.push_back(wnd);
        }
        return true;
    }, LPARAM(&windows));

    beginInsertRows({}, 0, windows.listedWindows.size() - 1);
    _listedWindows = std::move(windows.listedWindows);
    _nativeWindows = std::move(windows.nativeWindows);
    endInsertRows();

    auto foreground = GetForegroundWindow();
    foreach (auto window, _listedWindows) {
        if (window->hwnd() == foreground) {
            window->setActive(true);
            _activeWindow = window;
            break;
        }
    }
}

