#include <qdebug.h>
#include <qdir.h>
#include <qguiapplication.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qscreen.h>
#include <qwindow.h>

#include "kumori.h"
#include "launcher.h"
#include "wallpaper.h"

#ifdef Q_OS_WIN
#include "win.h"
#include "ohm.h"
#endif

QString userImportDir() {
    QSettings config;

    auto userImports = Kumori::instance()->userImportDir();

    QDir dir(userImports + "/desktop");
    dir.mkpath(dir.path());

    QFile root(dir.filePath("Root.qml"));
    if (! root.exists())
        QFile::copy(qApp->applicationDirPath() + "/kumori/Root.qml", root.fileName());

    QFile qmldir(dir.filePath("qmldir"));
    if (! qmldir.exists()) {
        qmldir.open(QFile::WriteOnly);

        qmldir.write("module desktop\n");
        qmldir.write("Root 0.1 Root.qml\n");
    }

    return userImports;
}

int main(int argc, char *argv[]) {
    qmlRegisterType<Wallpaper>("kumori", 0, 1, "Wallpaper");
    qmlRegisterSingletonType<Kumori>("kumori", 0, 1, "Kumori", &Kumori::instance);

#ifdef Q_OS_WIN
    qmlRegisterSingletonType<Ohm>("kumori", 0, 1, "Ohm", &Ohm::instance);
#endif

    QGuiApplication app(argc, argv);
    app.setApplicationName("kumori");

    auto userImports = userImportDir();

    QQmlApplicationEngine engine;
    engine.addImportPath(".");
    engine.addImportPath(userImports);

    qDebug() << engine.importPathList();

    auto const url = QUrl::fromLocalFile(app.applicationDirPath() + "/kumori/MainWindow.qml");
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

#ifdef Q_OS_WIN
//    exclude_from_peek(HWND(window->winId()));
//    ignore_show_desktop(HWND(window->winId()));
#endif

    window->setGeometry(window->screen()->availableGeometry());
    QObject::connect(window->screen(), &QScreen::availableGeometryChanged,
                     window, qOverload<QRect const&>(&QWindow::setGeometry));

    return app.exec();
}
