#include <qdiriterator.h>
#include <qfilesystemwatcher.h>
#include <qguiapplication.h>
#include <qopenglcontext.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qscreen.h>
#include <qwindow.h>

#include "dirwatcher.h"
#include "qmltypes/kumori.h"
#include "qmltypes/shaderpipeline/shaderpipeline.h"
#include "qmltypes/wallpaper.h"

#ifdef Q_OS_WIN
#include "win32/ohm.h"
#include "win32/shelliconprovider.h"
#include "win32/taskbar/liveicon.h"
#include "win32/taskbar/trayservice.h"
#include "win32/taskbar/trayicon.h"
#include "win32/taskbar/wmservice.h"
#endif

QString createUserImportDir() {
    QDir dir(Kumori::string("userImportDir"));
    dir.mkpath(dir.path());

    QFile root(dir.filePath("main.qml"));
    if (! root.exists())
        QFile::copy(Kumori::string("appImportDir") + "/main.qml", root.fileName());

    return dir.path();
}

QQmlApplicationEngine *loadQml(QUrl const& url) {
    qDebug() << url;

    QQmlApplicationEngine *engine = new QQmlApplicationEngine();

    engine->addImageProvider("shellIcon", new ShellIconProvider);
    engine->addImportPath(Kumori::string("appImportDir"));
    engine->load(url);

//    auto window = qobject_cast<QWindow *>(engine->rootObjects().first());

    return engine;
}

int main(int argc, char *argv[]) {
    qSetMessagePattern("[%{time process} %{threadid} %{type} %{function}:%{line}] %{if-category}%{file}:%{line}: %{endif}%{message}");
    qDebug();

    qmlRegisterType<DirWatcher>("kumori", 0, 1, "DirWatcher");
    qmlRegisterType<ShaderPipeline>("kumori", 0, 1, "ShaderPipeline");
    qmlRegisterType<Wallpaper>("kumori", 0, 1, "Wallpaper");

    qmlRegisterSingletonType<Kumori>("kumori", 0, 1, "Kumori", &Kumori::instance);
#ifdef Q_OS_WIN
    qRegisterMetaType<LiveIcon *>();
    qmlRegisterType<LiveIconPainter>("kumori", 0, 1, "LiveIcon");
    qmlRegisterType<TrayIconPainter>("kumori", 0, 1, "TrayIcon");

    qmlRegisterSingletonType<TrayService>("kumori", 0, 1, "TrayService", &TrayService::instance);
    qmlRegisterSingletonType<WmService>("kumori", 0, 1, "WmService", &WmService::instance);
    qmlRegisterSingletonType<Ohm>("kumori", 0, 1, "Ohm", &Ohm::instance);
#endif

//    QSurfaceFormat::setDefaultFormat([]{
//        QSurfaceFormat format;
//        format.setRenderableType(QSurfaceFormat::OpenGLES);
//        format.setProfile(QSurfaceFormat::CoreProfile);
//        format.setVersion(3, 0);
//        return format;
//    }());

//    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("zydd");
    app.setApplicationName("kumori");

    Kumori kumori(app.arguments());

    auto const userImportDir = createUserImportDir();
    auto const appImportDir = Kumori::string("appImportDir");
    qDebug() << "appImportDir:" << appImportDir;
    qDebug() << "userImportDir:" << userImportDir;
    auto const url = QUrl::fromLocalFile(userImportDir + "/main.qml");

    QPointer<QQmlApplicationEngine> engine = loadQml(url);

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

    // Reload application when files are changed
    QObject::connect(watcher, &QFileSystemWatcher::fileChanged, [&]{
        QObject::connect(engine, &QQmlApplicationEngine::destroyed, [&]{
            engine = loadQml(url);
        });
        engine->deleteLater();
    });

    watchDir(userImportDir);
    watchDir(appImportDir);

    QObject::connect(&app, &QGuiApplication::aboutToQuit, [&]{
        engine->deleteLater();
    });

    return app.exec();
}
