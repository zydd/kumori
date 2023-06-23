#include <qdiriterator.h>
#include <qfilesystemwatcher.h>
#include <qguiapplication.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qscreen.h>
#include <qwindow.h>

#include "dirwatcher.h"
#include "qmltypes/kumori.h"
#include "qmltypes/wallpaper.h"

#ifdef Q_OS_WIN
#include "win32/ohm.h"
#include "win32/shelliconprovider.h"
#include "win32/taskbar/trayicon.h"
#include "win32/taskbar/trayservice.h"
#include "win32/taskbar/wmservice.h"
#endif

QString userImportDir() {
    QDir dir(Kumori::string("userImportDir"));
    dir.mkpath(dir.path());

    QFile root(dir.filePath("Root.qml"));
    if (! root.exists())
        QFile::copy(Kumori::string("appImportDir") + "/Root.qml", root.fileName());

    return dir.path();
}

int main(int argc, char *argv[]) {
    qSetMessagePattern("[%{time process} %{threadid} %{type} %{function}:%{line}] %{if-category}%{file}:%{line}: %{endif}%{message}");
    qDebug();

    qmlRegisterType<DirWatcher>("kumori", 0, 1, "DirWatcher");
    qmlRegisterType<Wallpaper>("kumori", 0, 1, "Wallpaper");

    qmlRegisterSingletonType<Kumori>("kumori", 0, 1, "Kumori", &Kumori::instance);
#ifdef Q_OS_WIN
    qmlRegisterType<TrayIconPainter>("kumori", 0, 1, "TrayIcon");

    qmlRegisterSingletonType<TrayService>("kumori", 0, 1, "TrayService", &TrayService::instance);
    qmlRegisterSingletonType<WmService>("kumori", 0, 1, "WmService", &WmService::instance);
    qmlRegisterSingletonType<Ohm>("kumori", 0, 1, "Ohm", &Ohm::instance);
#endif

//    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("zydd");
    app.setApplicationName("kumori");

    Kumori kumori(app.arguments());

    auto const userImports = userImportDir();
    auto const qmlDir = Kumori::string("appImportDir");
    auto const url = QUrl::fromLocalFile(qmlDir + "/main.qml");

    QQmlApplicationEngine engine;

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.addImportPath(qmlDir);
    engine.addImportPath(userImports);
    engine.addImageProvider("shellIcon", new ShellIconProvider);
    engine.load(url);

    if (engine.rootObjects().empty())
        QCoreApplication::exit(-1);

    Q_ASSERT(!engine.rootObjects().empty());
    auto window = qobject_cast<QWindow *>(engine.rootObjects()[0]);

    window->setGeometry(window->screen()->geometry());
    QObject::connect(window->screen(), &QScreen::geometryChanged,
                     window, qOverload<QRect const&>(&QWindow::setGeometry));

    auto watcher = new QFileSystemWatcher;
    auto const monitoredFileTypes = Kumori::config("monitoredFileTypes").toStringList();

    auto watchDir = [watcher, &monitoredFileTypes](QString dir) {
        watcher->addPath(dir);

        for (QDirIterator iter(dir, QDir::Dirs | QDir::NoDotAndDotDot,
                               QDirIterator::Subdirectories); iter.hasNext();) {
            auto dir = iter.next();

            watcher->addPath(dir);
            for (QFileInfo const& e : QDir(dir).entryInfoList(monitoredFileTypes, QDir::Files))
                watcher->addPath(e.filePath());
        }
    };

    QObject::connect(watcher, &QFileSystemWatcher::directoryChanged,
                     [watcher, &monitoredFileTypes](QString const& path){
        QDir dir(path);

        for (QFileInfo const& e : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
            watcher->addPath(e.filePath());
        for (QFileInfo const& e : dir.entryInfoList(monitoredFileTypes, QDir::Files))
            watcher->addPath(e.filePath());
    });
    QObject::connect(watcher, SIGNAL(fileChanged(QString)), window, SLOT(reload()));

    watchDir(userImports);
    watchDir(qmlDir);

    return app.exec();
}
