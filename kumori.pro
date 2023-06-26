QT += qml quick winextras opengl
#CONFIG += lrelease embed_translations
CONFIG += c++17
LIBS += -luser32 -ldwmapi -lwbemuuid -lgdi32

DEFINES += QT_DEPRECATED_WARNINGS QT_FORCE_ASSERTS QT_MESSAGELOGCONTEXT
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

msvc* {
#    QMAKE_CXXFLAGS += /WX  # treat warnings as errors
#    DEFINES += NTDDI_WIN7=0x06010000 _WIN32_WINNT_WIN7=0x0601 WINVER=0x0601
    DEFINES += _USING_V110_SDK71_
    RC_INCLUDEPATH += $$(INCLUDEPATH)

    QMAKE_CXXFLAGS_RELEASE += /Zi /Oy-
    QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref
}

SOURCES += \
    src/dirwatcher.cpp \
    src/main.cpp \
    src/qmltypes/kumori.cpp \
    src/qmltypes/shaderpipeline/pipelinerenderer.cpp \
    src/qmltypes/shaderpipeline/shaderpipeline.cpp \
    src/qmltypes/wallpaper.cpp \
    src/win32/ohm.cpp \
    src/win32/shelliconprovider.cpp \
    src/win32/taskbar/nativewindow.cpp \
    src/win32/taskbar/trayicon.cpp \
    src/win32/taskbar/trayservice.cpp \
    src/win32/taskbar/wmservice.cpp \
    src/win32/winsearch.cpp

HEADERS += \
    src/dirwatcher.h \
    src/qmltypes/kumori.h \
    src/qmltypes/shaderpipeline/pipelinerenderer.h \
    src/qmltypes/shaderpipeline/shaderpipeline.h \
    src/qmltypes/wallpaper.h \
    src/win32/ohm.h \
    src/win32/shelliconprovider.h \
    src/win32/taskbar/nativewindow.h \
    src/win32/taskbar/trayicon.h \
    src/win32/taskbar/trayservice.h \
    src/win32/taskbar/wmservice.h \
    src/win32/winsearch.h

RESOURCES +=

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


qml.path = /tmp/$${TARGET}/bin/kumori
qml.files += \
    src/qml/*.frag \
    src/qml/*.qml \
    src/qml/fluidsim/*.frag \
    src/qml/kumori/*.frag \
    src/qml/kumori/*.qml

INSTALLS += qml

DISTFILES += \
    src/qml/kumori/qmldir
