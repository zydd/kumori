#include "wmservice.h"
#include "nativewindow.h"

#include <qdebug.h>
#include <qlibrary.h>
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

    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

static WmService *wmService = nullptr;

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
    delete this->d;
}

QModelIndex WmService::index(int row, int column, const QModelIndex &parent) const {
    return createIndex(row, column);
}

QModelIndex WmService::parent(const QModelIndex &child) const {
    return {};
}

int WmService::rowCount(const QModelIndex &parent) const {
    return _windowList.size();
}

int WmService::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant WmService::data(const QModelIndex &index, int role) const {
    switch (role) {
    case IdRole:    return 0;
    case NativeWindowRole: return QVariant::fromValue(_windowList[index.row()]);
    }

    Q_ASSERT(false);
    return {};
}

QHash<int, QByteArray> WmService::roleNames() const {
    return _roleNames;
}


QObject *WmService::instance(QQmlEngine *, QJSEngine *) {
    qDebug();

    if (!::wmService)
        ::wmService = new WmService();

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

    // FIXME: WinRT apps that are opened after WmService is started are not listed

    if (msg == staticData.WM_SHELLHOOKMESSAGE) {
        switch (wParam) {
        case HSHELL_GETMINRECT:
            qDebug() << "GETMINRECT" << hwndParam;
            break;
        case HSHELL_WINDOWACTIVATED:
            break;
        case HSHELL_RUDEAPPACTIVATED: {
            qDebug() << "RUDEAPPACTIVATED" << hwndParam;
            auto itr = wmService->_hwndIndex.find(hwndParam);
            if (itr != wmService->_hwndIndex.end() && itr.value() >= 0) {
                auto wnd = wmService->_windowList[itr.value()];

                if (wmService->_activeWindow)
                    wmService->_activeWindow->setActive(false);

                wmService->_activeWindow = wnd;
                wnd->setActive(true);
            }
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
            auto wnd = new NativeWindow{hwndParam};
            if (wnd->canAddToTaskbar()) {
                auto index = wmService->_windowList.size();
                wmService->beginInsertRows({}, index, index);
                wmService->_hwndIndex[hwndParam] = index;
                wmService->_windowList.push_back(wnd);
                wmService->endInsertRows();
            } else {
                delete wnd;
                wmService->_hwndIndex[hwndParam] = -1;
            }
            break;
        }
        case HSHELL_WINDOWDESTROYED: {
            qDebug() << "WINDOWDESTROYED" << hwndParam;
            auto itr = wmService->_hwndIndex.find(hwndParam);
            if (itr != wmService->_hwndIndex.end()) {
                if (itr.value() >= 0) {
                    auto index = itr.value();
                    wmService->beginRemoveRows({}, index, index);
                    auto wnd = wmService->_windowList[index];

                    wmService->_windowList.remove(index);

                    if (wmService->_activeWindow == wnd)
                        wmService->_activeWindow = nullptr;

                    delete wnd;
                    wmService->endRemoveRows();
                }

                wmService->_hwndIndex.remove(hwndParam);
            }
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
            auto itr = wmService->_hwndIndex.find(hwndParam);
            if (itr != wmService->_hwndIndex.end() && itr.value() >= 0) {
                auto wnd = wmService->_windowList[itr.value()];
                emit wnd->titleChanged();
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


void WmService::enumerateWindows() {
    Q_ASSERT(_windowList.size() == 0);

    struct WindowList {
        QVector<NativeWindow *> windowList;
        QHash<HWND, int> hwndIndex;
    };

    WindowList windows;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto *data = reinterpret_cast<WindowList *>(lParam);
        auto wnd = new NativeWindow{hwnd};
        if (wnd->canAddToTaskbar()) {
            qDebug() << data->windowList.size() << hwnd << wnd->title();
            data->hwndIndex.insert(hwnd, data->windowList.size());
            data->windowList.push_back(wnd);
        } else {
            delete wnd;
            data->hwndIndex.insert(hwnd, -1);
        }
        return true;
    }, LPARAM(&windows));

    beginInsertRows({}, 0, windows.windowList.size() - 1);
    _windowList = std::move(windows.windowList);
    _hwndIndex = std::move(windows.hwndIndex);
    endInsertRows();

    auto foreground = GetForegroundWindow();
    foreach (auto window, _windowList) {
        if (window->hwnd() == foreground) {
            window->setActive(true);
            _activeWindow = window;
            break;
        }
    }
}
