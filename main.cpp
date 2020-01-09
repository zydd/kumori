#include <qdebug.h>
#include <qguiapplication.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qscreen.h>
#include <qwindow.h>

#include "wallpaper.h"
#include "launcher.h"

#ifdef Q_OS_WIN
#include "win.h"
#endif

int main(int argc, char *argv[]) {
    qmlRegisterType<Wallpaper>("desktop", 0, 1, "Wallpaper");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("./qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    auto launcher = new Launcher;
    engine.rootContext()->setContextProperty("launcher", launcher);
    engine.addImageProvider("licon", launcher->iconProvider());

    engine.load(url);

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
