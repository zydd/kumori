#include <qdiriterator.h>
#include <qfilesystemwatcher.h>
#include <qguiapplication.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qscreen.h>
#include <qwindow.h>

#include "kumori.h"
#include "dirwatcher.h"
#include "shelliconprovider.h"
#include "wallpaper.h"

#ifdef Q_OS_WIN
#include "ohm.h"
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
    qmlRegisterType<DirWatcher>("kumori", 0, 1, "DirWatcher");
    qmlRegisterType<Wallpaper>("kumori", 0, 1, "Wallpaper");

    qmlRegisterSingletonType<Kumori>("kumori", 0, 1, "Kumori", &Kumori::instance);
#ifdef Q_OS_WIN
    qmlRegisterSingletonType<Ohm>("kumori", 0, 1, "Ohm", &Ohm::instance);
#endif

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

    auto window = qobject_cast<QWindow *>(engine.rootObjects()[0]);
    Q_ASSERT(window);

    window->setGeometry(window->screen()->availableGeometry());
    QObject::connect(window->screen(), &QScreen::availableGeometryChanged,
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
