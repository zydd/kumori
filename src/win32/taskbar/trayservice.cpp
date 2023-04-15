#include "trayservice.h"

#include <unordered_map>

#include <qdebug.h>
#include <qpixmap.h>
#include <qscreen.h>
#include <qwindow.h>
#include <qwinfunctions.h>

#include <Windows.h>
#include <CommCtrl.h>

#include "trayicon.h"


using TrayServicePrivate = TrayService::TrayServicePrivate;

struct TrayService::TrayServicePrivate {
    bool initialized = false;
    int timerId;

    HINSTANCE hInstance;
    HWND hwndTray;
    HWND hwndNotify;
    HWND hwndSystemTray;

    APPBARDATA trayPos;

    std::unordered_map<HWND, TrayIcon *> iconData;

    ushort registerWindowClass(LPCWSTR name);
    HWND registerNotifyWindow();
    HWND registerTrayWindow();

    LRESULT forwardToSystemTray(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

static TrayService *trayService = nullptr;

#pragma pack(push, 1)
struct SHELLTRAYDATA {
    int dwUnknown;
    uint dwMessage;
    NOTIFYICONDATA nid;
};

struct APPBARMSGDATA_ext {
    static_assert(sizeof(APPBARDATA) == 36);
    APPBARDATA abd;
    int dwMessage;
    int dwPadding1;
    long hSharedMemory;
    int dwSourceProcessId;
    int dwPadding3;
};

struct WINNOTIFYICONIDENTIFIER {
    int dwMagic;
    int dwMessage;
    int cbSize;
    int dwPadding;
    HWND hWnd;
    uint uID;
    GUID guidItem;
};
#pragma pack(pop)


TrayService::TrayService(QObject *parent)
    : QObject{parent}
{
    qDebug();
    d = new TrayServicePrivate();
}


void TrayService::init() {
    qDebug();

    if (d->initialized) {
        qDebug() << "already initialized";
        return;
    }

    d->hInstance = GetModuleHandle(nullptr);

    // TODO: check if there are multiple instances of Shell_TrayWnd and fail
    d->hwndSystemTray = FindWindow(L"Shell_TrayWnd", NULL);

    d->registerTrayWindow();
    d->registerNotifyWindow();

    qDebug() << "broadcast TaskbarCreated";
    int msg = RegisterWindowMessage(L"TaskbarCreated");
    if (msg > 0)
        SendNotifyMessage(HWND_BROADCAST, msg, NULL, NULL);
    else
        qWarning() << "could not create TaskbarCreated message";

    d->initialized = true;
    qDebug() << "done";
}

TrayService::~TrayService() {
    qDebug();
    restoreSystemTaskbar();
    delete this->d;
}

QObject *TrayService::instance(QQmlEngine *, QJSEngine *) {
    qDebug();

    if (!::trayService)
        ::trayService = new TrayService();

    return ::trayService;
}

QObjectList TrayService::trayItems() {
    QObjectList ret;

    for (auto itr = d->iconData.begin(); itr != d->iconData.end(); ++itr) {
        ret.push_back(itr->second);
    }

    return ret;
}


LRESULT TrayServicePrivate::forwardToSystemTray(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (hwndSystemTray)
        return SendMessage(hwndSystemTray, msg, wParam, lParam);
    else
        return DefWindowProc(hWnd, msg, wParam, lParam);
}

#define WM_USER_NEW_APPBAR (WM_USER + 110011)

LRESULT CALLBACK TrayServicePrivate::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    void* data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (msg) {
    case WM_COPYDATA: {
        if (lParam == 0) {
            qDebug() << "null WM_COPYDATA";
            break;
        }

        auto copyData = reinterpret_cast<PCOPYDATASTRUCT>(lParam);

        switch (copyData->dwData) {
        case 0: {
//            qDebug() << "AppBar message";
            // AppBar message
            if (sizeof(APPBARMSGDATA_ext) == copyData->cbData) {
                auto *amd = reinterpret_cast<APPBARMSGDATA_ext *>(copyData->lpData);

                if (sizeof(APPBARDATA) != amd->abd.cbSize) {
                    qWarning() << "unexpected size";
                    break;
                }
            } else {
//                qWarning() << "unexpected AppBar message size";
            }
            break;
        }
        case 1: {
            auto trayData = reinterpret_cast<SHELLTRAYDATA *>(copyData->lpData);

            auto itr = ::trayService->d->iconData.find(trayData->nid.hWnd);
            if (itr == ::trayService->d->iconData.end()) {
                itr = ::trayService->d->iconData.insert({trayData->nid.hWnd, new TrayIcon()}).first;

                qDebug() << "add icon:" << QString::fromWCharArray(trayData->nid.szTip);

                emit ::trayService->trayItemsChanged();
            }

            itr->second->setTooltip(QString::fromWCharArray(trayData->nid.szTip));

            if (trayData->nid.hIcon)
                itr->second->setIcon(QtWin::fromHICON(trayData->nid.hIcon));
            emit itr->second->iconChanged();

            break;
        }
        case 3: {
            auto notifyData = reinterpret_cast<WINNOTIFYICONIDENTIFIER *>(copyData->lpData);
            qDebug() << "notify" << notifyData->hWnd << notifyData->cbSize;

            break;
        }
        }

        break;
    }
    case WM_USER_NEW_APPBAR:
        qDebug() << "new appbar";
        SHAppBarMessage(ABM_SETPOS, &trayService->d->trayPos);
        return 0;
    }

    if  (  msg == WM_COPYDATA
        || msg == WM_ACTIVATEAPP
        || msg == WM_COMMAND
        || msg >= WM_USER
        )
        return ::trayService->d->forwardToSystemTray(hWnd, msg, wParam, lParam);
    else
        return DefWindowProc(hWnd, msg, wParam, lParam);
}


ushort TrayServicePrivate::registerWindowClass(LPCWSTR name) {
    qDebug() << QString::fromWCharArray(name);

    WNDCLASS newClass = {};
    newClass.lpszClassName = name;
    newClass.hInstance = hInstance;
    newClass.style = 0x8;
    newClass.lpfnWndProc = TrayServicePrivate::wndProc;

    return RegisterClass(&newClass);
}


HWND TrayServicePrivate::registerTrayWindow() {
    qDebug();

    ushort trayNotifyClassReg = registerWindowClass(L"Shell_TrayWnd");
    if (trayNotifyClassReg == 0) {
        auto err = GetLastError();
        qCritical() << "could not register Shell_TrayWnd class:"
                    << err
                    << QtWin::errorStringFromHresult(err)
                    << QtWin::stringFromHresult(err);
        return nullptr;
    }
    hwndTray = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // extended window style
        L"Shell_TrayWnd", // window class name
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

    if (hwndTray == nullptr) {
        auto err = GetLastError();
        qCritical() << "could not create Shell_TrayWnd window:"
                    << err
                    << QtWin::errorStringFromHresult(err).toUtf8()
                     << QtWin::stringFromHresult(err).toUtf8();
        return nullptr;
    }

    return hwndTray;
}


HWND TrayServicePrivate::registerNotifyWindow() {
    qDebug();

    ushort trayNotifyClassReg = registerWindowClass(L"TrayNotifyWnd");
    if (trayNotifyClassReg == 0) {
        auto err = GetLastError();
        qCritical() << "could not create TrayNotifyWnd class:"
                    << err
                    << QtWin::errorStringFromHresult(err)
                    << QtWin::stringFromHresult(err);
        return nullptr;
    }
    hwndNotify = CreateWindowEx(
        0, // extended window style
        L"TrayNotifyWnd", // window class name
        NULL, // window title
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // window style
        0, // x position
        0, // y position
        0, // width
        0, // height
        hwndTray, // parent window handle
        NULL, // menu handle
        hInstance, // application instance handle
        NULL); // creation parameters

    if (hwndNotify == nullptr) {
        auto err = GetLastError();
        qCritical() << "could not create TrayNotifyWnd window:"
                    << err
                    << QtWin::errorStringFromHresult(err).toUtf8()
                    << QtWin::stringFromHresult(err).toUtf8();
        return nullptr;
    }

    return hwndNotify;
}


void TrayService::setTaskBar(QWindow *window) {
    qDebug() << window << window->geometry();

    init();

    window->setParent(nullptr);

    auto edge =
            window->y() > window->screen()->geometry().y()/2 ? ABE_BOTTOM :
            window->x() > window->screen()->geometry().x()/2 ? ABE_RIGHT :
            window->x() > 0 || window->height() > window->width() ? ABE_LEFT :
                                                                    ABE_TOP;
    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = d->hwndTray;
        abd.uEdge = edge;
        abd.uCallbackMessage = WM_USER_NEW_APPBAR;
        SHAppBarMessage(ABM_NEW, &abd);
    }

    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.lParam = ABS_AUTOHIDE;
        abd.hWnd = d->hwndSystemTray;
        SHAppBarMessage(ABM_SETSTATE, &abd);
    }

    {
        d->trayPos = {};
        d->trayPos.cbSize = sizeof(APPBARDATA);
        d->trayPos.hWnd = d->hwndTray;
        d->trayPos.uEdge = edge;
        d->trayPos.rc.top = window->y();
        d->trayPos.rc.bottom = window->y() + window->height();
        d->trayPos.rc.left = window->x();
        d->trayPos.rc.right = window->x() + window->width();
        qDebug() << "set initial appbar pos";
        SHAppBarMessage(ABM_SETPOS, &d->trayPos);
    }

    killTimer(d->timerId);
    d->timerId = startTimer(200);

//    ShowWindow(d->hwndSystemTray, SW_HIDE);
}


void TrayService::timerEvent(QTimerEvent */*event*/) {
    if (IsWindowVisible(d->hwndSystemTray)) {
        static APPBARDATA abd = []{
            APPBARDATA abd;
            abd.cbSize = sizeof(APPBARDATA);
            abd.lParam = ABS_AUTOHIDE;
            return abd;
        }();
        abd.hWnd = d->hwndSystemTray;
        SHAppBarMessage(ABM_SETSTATE, &abd);
//        SHAppBarMessage(ABM_SETPOS, &d->trayPos);

//        SetWindowPos(d->hwndSystemTray, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        ShowWindow(d->hwndSystemTray, SW_HIDE);
        qDebug() << "hide explorer taskbar";
    }

    SetWindowPos(d->hwndTray, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
}


void TrayService::restoreSystemTaskbar() {
    qDebug();

    killTimer(d->timerId);
    d->timerId = 0;

    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = d->hwndTray;
        SHAppBarMessage(ABM_REMOVE, &abd);
    }

//    ShowWindow(d->hwndSystemTray, SW_SHOW);

    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.lParam = ABS_ALWAYSONTOP;
    abd.hWnd = d->hwndSystemTray;
    SHAppBarMessage(ABM_SETSTATE, &abd);

    ShowWindow(d->hwndSystemTray, SW_SHOW);
//    SetWindowPos(d->hwndSystemTray, HWND_BOTTOM, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}


// Undocumented API ITrayNotify
// https://gist.github.com/ysc3839/25e8ed113c4e975b6781c9759ed4ee87


enum NOTIFYITEM_PREFERENCE {
    // In Windows UI: "Only show notifications."
    PREFERENCE_SHOW_WHEN_ACTIVE = 0,
    // In Windows UI: "Hide icon and notifications."
    PREFERENCE_SHOW_NEVER = 1,
    // In Windows UI: "Show icon and notifications."
    PREFERENCE_SHOW_ALWAYS = 2
};

#pragma pack(push, 1)
typedef struct tagNOTIFYITEM {
    PWSTR pszExeName;
    PWSTR pszTip;
    HICON hIcon;
    HWND hWnd;
    NOTIFYITEM_PREFERENCE dwPreference;
    UINT uID;
    GUID guidItem;
} NOTIFYITEM, *PNOTIFYITEM;
#pragma pack(pop)


MIDL_INTERFACE("D782CCBA-AFB0-43F1-94DB-FDA3779EACCB")
INotificationCB : public IUnknown {
public:
    BEGIN_INTERFACE
    virtual HRESULT STDMETHODCALLTYPE Notify(ULONG, NOTIFYITEM *) = 0;
    END_INTERFACE
};

MIDL_INTERFACE("FB852B2C-6BAD-4605-9551-F15F87830935")
ITrayNotify : public IUnknown {
public:
    BEGIN_INTERFACE
    virtual HRESULT STDMETHODCALLTYPE RegisterCallback(INotificationCB* callback) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPreference(const NOTIFYITEM* notify_item) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnableAutoTray(BOOL enabled) = 0;
    END_INTERFACE
};

MIDL_INTERFACE("D133CE13-3537-48BA-93A7-AFCD5D2053B4")
ITrayNotifyWin8 : public IUnknown {
public:
    BEGIN_INTERFACE
    virtual HRESULT STDMETHODCALLTYPE RegisterCallback(INotificationCB* callback, ULONG*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterCallback(ULONG*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPreference(NOTIFYITEM const*) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnableAutoTray(BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE DoAction(BOOL) = 0;
    END_INTERFACE
};

const CLSID CLSID_TrayNotify = {0x25DEAD04, 0x1EAC, 0x4911, {
                                    0x9E, 0x3A, 0xAD, 0x0A, 0x4A, 0xB5, 0x60, 0xFD }};

template <class T>
class NotificationMgr : public INotificationCB {
public:
    NotificationMgr(T *pTrayNotify) : m_pTrayNotify(pTrayNotify) {}

    IFACEMETHODIMP QueryInterface(REFIID riid, PVOID *ppv)
    {
        if (ppv == nullptr)
            return E_POINTER;

        if (riid == IID_IUnknown)
            *ppv = static_cast<IUnknown *>(this);
        else if (riid == __uuidof (INotificationCB))
            *ppv = static_cast<INotificationCB *>(this);
        else
            return E_NOINTERFACE;

        reinterpret_cast<IUnknown*>(*ppv)->AddRef();

        return S_OK;
    }

    IFACEMETHODIMP_(ULONG) AddRef() { return 1; }

    IFACEMETHODIMP_(ULONG) Release() { return 1; }

    IFACEMETHODIMP Notify(ULONG Event, NOTIFYITEM *NotifyItem) {
        wchar_t *copy = _wcsdup(NotifyItem->pszExeName);
        if (copy)
        {
            _wcslwr_s(copy, wcslen(NotifyItem->pszExeName) + 1);
            if (wcsstr(copy, L"telegram.exe"))
            {
                if (m_pTrayNotify)
                {
                    NotifyItem->dwPreference = PREFERENCE_SHOW_ALWAYS;
                    m_pTrayNotify->SetPreference(NotifyItem);
                }
            }
            free(copy);
        }
        return S_OK;
    }
private:
    T *m_pTrayNotify;
};


int getItemCount() {
    HWND hShellTrayWnd = FindWindow(L"Shell_TrayWnd", NULL);
    HWND hNotifyWnd = FindWindowEx(hShellTrayWnd, NULL, L"TrayNotifyWnd", NULL);
    HWND hSysPager = FindWindowEx(hNotifyWnd, NULL, L"SysPager", NULL);
    HWND hToolbar = FindWindowEx(hSysPager, NULL, L"ToolbarWindow32", NULL);

    qDebug() << "htoolbar" << hToolbar;

    int nButtons = SendMessage(hToolbar, TB_BUTTONCOUNT, 0, 0);

//    for (int i = 0; i < nButtons; i++) {
//        TBBUTTON button;
//        SendMessage(hToolbar, TB_GETBUTTON, i, (LPARAM)&button);

//        qDebug() << "btn" << button.iString;
//    }

    auto hWnd = FindWindow(L"NotifyIconOverflowWindow", NULL);
    auto hTray = FindWindowEx(hWnd, NULL, L"ToolbarWindow32", NULL);

    nButtons += SendMessage(hTray, TB_BUTTONCOUNT, 0, 0);
    return nButtons;
}


int getItemCountW8() {
    CoInitialize(nullptr);
    IUnknown *pIUnk;
    HRESULT hr = CoCreateInstance(CLSID_TrayNotify, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pIUnk));
    if (!SUCCEEDED(hr)) {
        qDebug() << "COM init failed";
        return 0;
    }

    ITrayNotifyWin8 *pTrayNotifyW8;
    hr = pIUnk->QueryInterface(&pTrayNotifyW8);
    if (!SUCCEEDED(hr)) {
        qDebug() << "COM query failed";
        return 0;
    }

    hr = pTrayNotifyW8->EnableAutoTray(false);
    if (!SUCCEEDED(hr)) {
        qDebug() << "COM EnableAutoTray false failed";
        return 0;
    }

    int count = getItemCount();

    hr = pTrayNotifyW8->EnableAutoTray(true);
    if (!SUCCEEDED(hr)) {
        qDebug() << "COM EnableAutoTray true failed";
        return count;
    }

    pTrayNotifyW8->Release();

    return count;
}
