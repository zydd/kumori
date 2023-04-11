#include "wmservice.h"
#include "nativewindow.h"

#include <qdebug.h>

#include "Windows.h"


WmService::WmService(QObject *parent)
    : QObject{parent}
{
    _taskList = getOpenWindows();
}

QList<QObject*> WmService::getOpenWindows() {
    QList<QObject*> ret;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto *ret = reinterpret_cast<QList<QObject*> *>(lParam);
        auto wnd = std::unique_ptr<NativeWindow>(new NativeWindow{hwnd});
        if (wnd->canAddToTaskbar()) {
            ret->append(wnd.release());
        }

        return true;
    }, LPARAM(&ret));

//    HWND hWndForeground = GetForegroundWindow();
    return ret;
}
