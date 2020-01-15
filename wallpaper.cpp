#include "wallpaper.h"

#include <qdir.h>
#include <qfilesystemwatcher.h>
#include <qpainter.h>
#include <qquickwindow.h>
#include <qscreen.h>
#include <qtimer.h>

static const char *img_path = "/AppData/Roaming/Microsoft/Windows/Themes/TranscodedWallpaper";

Wallpaper::Wallpaper() {
    m_path = QDir::homePath() + img_path;
    m_image = QImage(m_path, "JPEG");

    auto watcher = new QFileSystemWatcher(this);
    watcher->addPath(m_path);

    connect(watcher, &QFileSystemWatcher::fileChanged, this, &QQuickItem::update);

    connect(this, &QQuickItem::windowChanged, [this](QQuickWindow *window){
        if (window) {
            connect(window, &QQuickWindow::xChanged, this, &QQuickItem::update);
            connect(window, &QQuickWindow::yChanged, this, &QQuickItem::update);
        }
    });

    m_timer.setSingleShot(true);
    m_timer.setInterval(4000);
    connect(&m_timer, &QTimer::timeout, [this](){
        if (! m_image.isNull())
            m_image = {};
    });
}

void Wallpaper::paint(QPainter *painter) {
    qDebug() << "Wallpaper::paint()";

    if (m_image.isNull()) {
        m_image = QImage(m_path, "JPEG");

        if (m_image.isNull())
            return;
    }

    QMetaObject::invokeMethod(this, [this](){ m_timer.start(); }, Qt::QueuedConnection);

    QSize size(painter->device()->width(), painter->device()->height());

    painter->fillRect(QRect{{0, 0}, size}, Qt::black);

    QRectF available = {window()->x() + x(), window()->y() + y(), width(), height()};
    QSize screen = window()->screen()->size();

    if (m_image.width() < screen.width() || m_image.height() < screen.height() ||
            (m_image.width() > screen.width() && m_image.height() > screen.height())) {
        m_image = m_image.scaled(screen, Qt::KeepAspectRatioByExpanding,
                                 Qt::SmoothTransformation);
    }

    QRectF target({0, 0}, size);
    QRectF source({0, 0}, screen);

    source.moveCenter({m_image.width()/2., m_image.height()/2.});

    auto centered_top = source.top();

    source.setTop(source.top() + available.top());
        source.setLeft(source.left() + available.left());
    source.setWidth(available.width());
    source.setHeight(available.height());

    if (centered_top > 20)
        source.moveTop(source.top() - 20);

    painter->drawImage(target, m_image, source);
}
