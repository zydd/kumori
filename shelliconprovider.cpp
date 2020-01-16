#include "shelliconprovider.h"

#include <qdir.h>
#include <qfileinfo.h>

#ifdef Q_OS_WIN
#include <comdef.h>
#include <CommCtrl.h>
#include <commoncontrols.h>
#include <ShObjIdl.h>

#include <qwinfunctions.h>
#endif

ShellIconProvider::ShellIconProvider():
    QQuickImageProvider(QQuickImageProvider::Pixmap)
{ }

static QPixmap shellIcon(const QString &fileName, const QSize &size) {
#ifdef Q_OS_WIN
    const wchar_t *fileNameW = reinterpret_cast<const wchar_t *>(fileName.utf16());
    HRESULT hr;

    unsigned int flags = SHGFI_ICON | SHGFI_LARGEICON; // SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION

    if (!QFileInfo(fileName).isDir())
        flags |= SHGFI_USEFILEATTRIBUTES;

    SHFILEINFO info;
    ZeroMemory(&info, sizeof(SHFILEINFO));

    hr = HRESULT(SHGetFileInfo(fileNameW, 0, &info, sizeof(SHFILEINFO), flags));
    if (SUCCEEDED(hr) && info.hIcon) {
        for (auto const& iImageList : {SHIL_EXTRALARGE, SHIL_JUMBO}) {
            static const IID iID_IImageList = {0x46eb5926, 0x582e, 0x4017, {0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50}};

            IImageList *imageList = nullptr;
            if (FAILED(SHGetImageList(iImageList, iID_IImageList, reinterpret_cast<void **>(&imageList))))
                continue;

            HICON hIcon = nullptr;
            if (SUCCEEDED(imageList->GetIcon(info.iIcon, ILD_TRANSPARENT, &hIcon))) {
                auto pixmap = QtWin::fromHICON(hIcon);
                DestroyIcon(hIcon);
                return pixmap;
            }
        }

        auto pixmap = QtWin::fromHICON(info.hIcon);
        DestroyIcon(info.hIcon);
        if (! pixmap.isNull())
            return pixmap;
    }

    IShellItemImageFactory *pImageFactory;
    hr = SHCreateItemFromParsingName(fileNameW, nullptr, IID_PPV_ARGS(&pImageFactory));
    if (SUCCEEDED(hr)) {
        HBITMAP hbmp;
        hr = pImageFactory->GetImage({size.width(), size.height()}, SIIGBF_ICONONLY | SIIGBF_BIGGERSIZEOK | SIIGBF_SCALEUP, &hbmp);
        if (SUCCEEDED(hr)) {
            auto pixmap = QtWin::fromHBITMAP(hbmp, QtWin::HBitmapAlpha);
            DeleteObject(hbmp);
            return pixmap;
        }
    }
#endif

    return {};
}

QPixmap ShellIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
#ifdef Q_OS_WIN
    auto icon = shellIcon(QDir::toNativeSeparators(id), requestedSize);
    if (requestedSize.isValid())
        icon = icon.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    *size = icon.size();
    return icon;
#else
        return {};
#endif
}
