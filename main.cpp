#include <qdiriterator.h>
#include <qfilesystemwatcher.h>
#include <qguiapplication.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qscreen.h>
#include <qwindow.h>

#include "kumori.h"
#include "launcher.h"
#include "wallpaper.h"

#ifdef Q_OS_WIN
#include "ohm.h"
#endif

void makeUserImportDir() {
    QDir dir(Kumori::instance()->config("userImportDir"));
    dir.mkpath(dir.path());

    QFile root(dir.filePath("Root.qml"));
    if (! root.exists())
        QFile::copy(Kumori::config("appImportDir") + "/Root.qml", root.fileName());
}

int main(int argc, char *argv[]) {
    qmlRegisterType<Wallpaper>("kumori", 0, 1, "Wallpaper");
    qmlRegisterSingletonType<Kumori>("kumori", 0, 1, "Kumori", &Kumori::instance);

#ifdef Q_OS_WIN
    qmlRegisterSingletonType<Ohm>("kumori", 0, 1, "Ohm", &Ohm::instance);
#endif

    QGuiApplication app(argc, argv);
    app.setOrganizationName("zydd");
    app.setApplicationName("kumori");

    Kumori kumori(app.arguments());

    makeUserImportDir();
    auto userImports = Kumori::config("userImportDir");
    auto qmlDir = Kumori::config("appImportDir");

    QQmlApplicationEngine engine;
    engine.addImportPath(qmlDir);
    engine.addImportPath(userImports);

    auto const url = QUrl::fromLocalFile(qmlDir + "/main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    auto launcher = new Launcher;
    engine.rootContext()->setContextProperty("launcher", launcher);
    engine.addImageProvider("licon", launcher->iconProvider());

    engine.load(url);

    if (engine.rootObjects().empty())
        return EXIT_FAILURE;

    auto window = qobject_cast<QWindow *>(engine.rootObjects()[0]);
    Q_ASSERT(window);

    window->setGeometry(window->screen()->availableGeometry());
    QObject::connect(window->screen(), &QScreen::availableGeometryChanged,
                     window, qOverload<QRect const&>(&QWindow::setGeometry));

    auto watcher = new QFileSystemWatcher;

    auto watchDir = [watcher](QString dir) {
        watcher->addPath(dir);

        for (QDirIterator iter(dir, QDir::Dirs | QDir::NoDotAndDotDot,
                               QDirIterator::Subdirectories); iter.hasNext();) {
            auto dir = iter.next();

            watcher->addPath(dir);
            for (QFileInfo const& e : QDir(dir).entryInfoList({"*.qml"}, QDir::Files))
                watcher->addPath(e.filePath());
        }
    };

    watchDir(userImports);
    watchDir(qmlDir);

    QObject::connect(watcher, &QFileSystemWatcher::directoryChanged, [watcher](QString const& path){
        QDir dir(path);

        for (QFileInfo const& e : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
            watcher->addPath(e.filePath());
        for (QFileInfo const& e : dir.entryInfoList({"*.qml"}, QDir::Files))
            watcher->addPath(e.filePath());
    });
    QObject::connect(watcher, SIGNAL(fileChanged(QString)), window, SLOT(reload()));

    return app.exec();
}
