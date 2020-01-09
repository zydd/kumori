QT += qml quick winextras
CONFIG += lrelease embed_translations
CONFIG += c++17
LIBS += -luser32 -ldwmapi
DEFINES += NTDDI_WIN7=0x06010000 _WIN32_WINNT_WIN7=0x0601 WINVER=0x0601

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        iconextractor.cpp \
        launcher.cpp \
        main.cpp \
        wallpaper.cpp \
        win.cpp

RESOURCES +=

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    iconextractor.h \
    launcher.h \
    wallpaper.h \
    win.h

qml.path = /tmp/$${TARGET}/bin/qml
qml.files += \
    qml/main.qml \
    qml/Launcher.qml \
    qml/Youbi.qml \
    qml/youbi.frag \
    qml/Circle.qml \
    qml/circle.frag \
    qml/CanvasMenu.qml

INSTALLS += qml
