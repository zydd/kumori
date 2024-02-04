#include "trayservice.h"

#include <unordered_map>

#include <qdebug.h>
#include <qpixmap.h>
#include <qscreen.h>
#include <qtimer.h>
#include <qwindow.h>
#include <qwinfunctions.h>

#include <Windows.h>
#include <CommCtrl.h>
#include <shellapi.h>

#include "trayicon.h"

static const QHash<QString, QUuid> defaultIconGuids = {
    {"hardware",    "7820ae78-23e3-4229-82c1-e41cb67d5b9c"},
    {"health",      "7820ae76-23e3-4229-82c1-e41cb67d5b9c"},
    {"location",    "7820ae77-23e3-4229-82c1-e41cb67d5b9c"},
    {"meetnow",     "7820ae83-23e3-4229-82c1-e41cb67d5b9c"},
    {"microphone",  "7820ae82-23e3-4229-82c1-e41cb67d5b9c"},
    {"network",     "7820ae74-23e3-4229-82c1-e41cb67d5b9c"},
    {"power",       "7820ae75-23e3-4229-82c1-e41cb67d5b9c"},
    {"update",      "7820ae81-23e3-4229-82c1-e41cb67d5b9c"},
    {"volume",      "7820ae73-23e3-4229-82c1-e41cb67d5b9c"},
};

static const QVector<QUuid> actionCenterGuids = {
    defaultIconGuids["network"],
    defaultIconGuids["volume"],
    defaultIconGuids["power"],
};

struct TrayServicePrivate {
    bool initialized = false;
    int timerId;

    HINSTANCE hInstance;
    HWND hwndTray;
    HWND hwndNotify;
    HWND hwndSystemTray;

    APPBARDATA systemTrayState;

    ushort registerWindowClass(LPCWSTR name);
    HWND registerNotifyWindow();
    HWND registerTrayWindow();

    void destroyNotifyWindow();
    void destroyTrayWindow();

    LRESULT forwardToSystemTray(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

static QPointer<TrayService> trayService = nullptr;

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
    : QAbstractItemModel{parent}
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
    {
        // FIXME: if explorer taskbar is set to auto-hide the area
        // of the custom appbar does not get registered correctly.

        // For now setting it to ABS_ALWAYSONTOP
        d->systemTrayState.cbSize = sizeof(APPBARDATA);
        d->systemTrayState.lParam = ABS_ALWAYSONTOP;
        d->systemTrayState.hWnd = d->hwndSystemTray;
        SHAppBarMessage(ABM_SETSTATE, &d->systemTrayState);
    }
    {
        d->systemTrayState.cbSize = sizeof(APPBARDATA);
        d->systemTrayState.lParam = ABS_AUTOHIDE;
        d->systemTrayState.hWnd = d->hwndSystemTray;
        SHAppBarMessage(ABM_SETSTATE, &d->systemTrayState);
    }

    d->registerTrayWindow();
    d->registerNotifyWindow();

    killTimer(d->timerId);
    d->timerId = startTimer(200);

    QTimer::singleShot(2000, [this]{
        taskBarCreated();
    });

    d->initialized = true;
    qDebug() << "done";
}


TrayService::~TrayService() {
    qDebug();
    restoreSystemTaskbar();
    d->destroyNotifyWindow();
    d->destroyTrayWindow();
    delete this->d;
}


void TrayService::timerEvent(QTimerEvent */*event*/) {
    if (IsWindowVisible(d->hwndSystemTray)) {
        qDebug() << "hide explorer taskbar";
        SHAppBarMessage(ABM_SETSTATE, &d->systemTrayState);
        ShowWindow(d->hwndSystemTray, SW_HIDE);
    }

    SetWindowPos(d->hwndTray,
                 HWND_TOPMOST,
                 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
}


QModelIndex TrayService::index(int row, int /*column*/, const QModelIndex &/*parent*/) const {
    return createIndex(row, 0);
}

QModelIndex TrayService::parent(const QModelIndex &child) const { return {}; }
int TrayService::columnCount(const QModelIndex &parent) const { return 1; }

int TrayService::rowCount(const QModelIndex &parent) const {
    return _trayIconMap.count();
}

QVariant TrayService::data(const QModelIndex &index, int role) const {
    Q_ASSERT(index.row() < _trayIconList.size());

    switch (role) {
    case IdRole:
        return intptr_t(_trayIconList[index.row()]->data.hWnd);
    case TrayIconRole:
        return QVariant::fromValue(_trayIconList[index.row()]);
    }

    Q_ASSERT(false);
    return {};
}

QHash<int, QByteArray> TrayService::roleNames() const {
    static const QHash<int, QByteArray> roles = {
        {ModelRoles::IdRole, "id"},
        {ModelRoles::TrayIconRole, "trayIcon"},
    };
    return roles;
}


void TrayService::taskBarCreated() {
    qDebug();
    int msg = RegisterWindowMessage(L"TaskbarCreated");
    if (!msg || !SUCCEEDED(SendNotifyMessage(HWND_BROADCAST, msg, NULL, NULL)))
        qWarning() << "could not send TaskbarCreated message";
}

QObject *TrayService::instance(QQmlEngine *, QJSEngine *) {
    qDebug() << ::trayService;

    if (!::trayService) {
        ::trayService = new TrayService();
        qDebug() << "new:" << ::trayService;
    }

    return ::trayService;
}


LRESULT TrayServicePrivate::forwardToSystemTray(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    qDebug();
    if (hwndSystemTray)
        return SendMessage(hwndSystemTray, msg, wParam, lParam);
    else
        return DefWindowProc(hWnd, msg, wParam, lParam);
}


LRESULT CALLBACK TrayServicePrivate::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    void* data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (msg) {
    case WM_COPYDATA: {
        if (lParam == 0) {
            qWarning() << "null WM_COPYDATA";
            break;
        }

        auto copyData = reinterpret_cast<PCOPYDATASTRUCT>(lParam);

        switch (copyData->dwData) {
        case 0: { // APPBAR
            break;
        }

        case 1: { // TRAYDATA
            auto trayData = reinterpret_cast<SHELLTRAYDATA *>(copyData->lpData);
            if (!trayData) {
                qWarning() << "invalid TRAYDATA";
                return false;
            }

            switch (trayData->dwMessage) {
            case NIM_ADD:
                qDebug() << "ADD" << trayData->nid.hWnd;
                goto case_nim_modify;

            case NIM_MODIFY: {
//                qDebug() << "MODIFY" << trayData->nid.hWnd;

                if (!::trayService->_trayIconMap.contains(trayData->nid.hWnd)) {
                    qWarning() << "trying to modify icon before add:" << trayData->nid.hWnd;
//                    return false;
                }

case_nim_modify:
                if (!IsWindow(trayData->nid.hWnd)) {
                    qWarning() << "invalid HWND" << trayData->nid.hWnd;
                    break;
                }

                auto trayIcon = ::trayService->icon(trayData->nid.hWnd);

                trayIcon->data.uID       = trayData->nid.uID;
                trayIcon->data.uVersion  = trayData->nid.uVersion;

                if (trayData->nid.uFlags & NIF_MESSAGE)
                    trayIcon->data.uCallbackMessage = trayData->nid.uCallbackMessage;

                if (trayData->nid.uFlags & NIF_GUID) {
                    trayIcon->data.guid = trayData->nid.guidItem;

                    // GUID affects sorting, emit dataChanged to notify the proxy model
                    auto row = ::trayService->_trayIconList.indexOf(trayIcon);
                    auto index = ::trayService->createIndex(row, 0);
                    emit ::trayService->dataChanged(index, index, {TrayService::TrayIconRole});
                }

                if (trayData->nid.uFlags & NIF_TIP)
                    trayIcon->setTooltip(QString::fromWCharArray(trayData->nid.szTip));

                if (trayData->nid.uFlags & NIF_ICON && trayData->nid.hIcon)
                    trayIcon->setIcon(QtWin::fromHICON(trayData->nid.hIcon));

                if (trayData->dwMessage == NIM_ADD) {
                    qDebug() << "added" << trayIcon->data.hWnd << trayIcon->tooltip();
                }

                return true;
            }

            case NIM_DELETE: {
                qDebug() << "DELETE" << trayData->nid.hWnd;

                // do not remove volume icon
                auto icon = ::trayService->_trayIconMap.value(trayData->nid.hWnd);
                if (icon && icon->data.guid == defaultIconGuids["volume"]) {
                    qDebug() << "DELETE volume ignored";
                    return false;
                }

                ::trayService->removeIcon(trayData->nid.hWnd);

                return true;
            }

            case NIM_SETFOCUS: {
                qDebug() << "SETFOCUS" << trayData->nid.hWnd;

                break;
            }

            case NIM_SETVERSION: {
                qDebug() << "SETVERSION" << trayData->nid.hWnd << "v:" << trayData->nid.uVersion;
                auto trayIcon = ::trayService->_trayIconMap.value(trayData->nid.hWnd, nullptr);
                if (!trayIcon)
                    return false;

                trayIcon->data.uVersion = trayData->nid.uVersion;
                return true;
            }
            }
            break;  // TRAYDATA
        }

        case 3: {  // NOTIFYICON
            auto notifyIcon = reinterpret_cast<WINNOTIFYICONIDENTIFIER *>(copyData->lpData);
            if (!notifyIcon) {
                qWarning() << "invalid WINNOTIFYICON";
                return false;
            }
            qDebug() << "WINNOTIFYICON" << notifyIcon->hWnd;

            auto trayIcon = ::trayService->_trayIconMap.value(notifyIcon->hWnd, nullptr);
            if (!trayIcon) {
                qWarning() << "TrayIcon not found" << notifyIcon->hWnd;
                return false;
            }

            switch (notifyIcon->dwMessage) {
            case 1:  // top-left
                return MAKELPARAM(trayIcon->rect().left(), trayIcon->rect().top());

            case 2:  // bottom-right
                return MAKELPARAM(trayIcon->rect().right(), trayIcon->rect().bottom());

            default:
                qWarning() << "unhandled message" << notifyIcon->dwMessage;
            }

            return false;
        }
        }
        break; // WM_COPYDATA
    }
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


TrayIcon *TrayService::icon(HWND hwnd) {
    auto icon = _trayIconMap.value(hwnd);
    if (icon)
        return icon;

    icon = new TrayIcon();
    icon->data.hWnd = hwnd;

    _trayIconMap.insert(hwnd, icon);
    QObject::connect(icon, &TrayIcon::invalidated, [this, hwnd]{ removeIcon(hwnd); });

    qDebug() << "add icon:" << hwnd;

    auto index = _trayIconList.size();
    beginInsertRows({}, index, index);
    _trayIconList.push_back(icon);
    endInsertRows();

    return icon;
}


void TrayService::removeIcon(HWND hwnd) {
    qDebug() << hwnd;

    auto icon = _trayIconMap.take(hwnd);
    if (!icon)
        return;

    auto index = std::distance(_trayIconList.begin(), std::find(_trayIconList.begin(), _trayIconList.end(), icon));
    if (index >= _trayIconList.size()) {
        qWarning() << "not found in list:" << hwnd;
        return;
    }

    beginRemoveRows({}, index, index);
    _trayIconList.remove(index);
    endRemoveRows();

    delete icon;
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


void TrayServicePrivate::destroyNotifyWindow() {
    qDebug();
    DestroyWindow(hwndNotify);
    UnregisterClass(L"TrayNotifyWnd", hInstance);
}


void TrayServicePrivate::destroyTrayWindow() {
    qDebug();
    DestroyWindow(hwndTray);
    UnregisterClass(L"Shell_TrayWnd", hInstance);
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
    // TODO: restore previous state iso. always setting to ABS_ALWAYSONTOP
    abd.lParam = ABS_ALWAYSONTOP;
    abd.hWnd = d->hwndSystemTray;
    SHAppBarMessage(ABM_SETSTATE, &abd);

    ShowWindow(d->hwndSystemTray, SW_SHOW);
}


TrayItemsProxy *TrayService::proxy(FilteringMode mode) {
    auto proxy = new TrayItemsProxy(mode);
    proxy->setSourceModel(this);
    proxy->sort(0);
    return proxy;
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


bool TrayItemsProxy::filterAcceptsRow(int source_row, const QModelIndex &/*source_parent*/) const {
    auto trayService = reinterpret_cast<TrayService *>(sourceModel());
    auto icon = trayService->iconAt(source_row);
    switch (_filterMode) {
    case TrayService::ActionCenterIcons:
        return actionCenterGuids.contains(icon->data.guid);

    case TrayService::VisibleIcons:
        return !actionCenterGuids.contains(icon->data.guid);

    case TrayService::HiddenIcons:
        return false;
    }

//    return true;
}


bool TrayItemsProxy::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
    auto trayService = reinterpret_cast<TrayService *>(sourceModel());
    auto left = trayService->iconAt(source_left.row());
    auto right = trayService->iconAt(source_right.row());

    auto left_guid_i = actionCenterGuids.indexOf(left->data.guid);
    auto right_guid_i = actionCenterGuids.indexOf(right->data.guid);

//    qDebug() << source_left.row() << source_right.row() << left_guid_i << right_guid_i << left->data.hWnd << right->data.hWnd;

    if (left_guid_i >= 0 && right_guid_i >= 0)
        return left_guid_i < right_guid_i;
    else if (right_guid_i >= 0)
        return true;
    else if (left_guid_i >= 0)
        return false;
    else
        return left->data.hWnd < right->data.hWnd;
}
