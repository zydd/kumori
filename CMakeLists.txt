cmake_minimum_required(VERSION 3.15)

project(kumori VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick qtwinextras)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick winextras REQUIRED)
find_package(Qt5 5.15 COMPONENTS Core Quick QmlImportScanner WinExtras REQUIRED)


FILE(GLOB_RECURSE SOURCES "src/*.cpp")
FILE(GLOB_RECURSE HEADERS "src/*.h")
FILE(GLOB_RECURSE QML "src/*.qml" "src/*.frag" "src/*.vert" "qmldir")
add_custom_target(headers SOURCES ${HEADERS})
add_custom_target(qml SOURCES ${QML})


if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(kumori MANUAL_FINALIZATION ${SOURCES})
else()
    add_executable(kumori ${SOURCES})
endif()

# Needed for static build
target_compile_options(kumori PRIVATE /Zi /Oy-)
target_link_options(kumori PRIVATE /MAP /debug /opt:ref)
set_property(TARGET kumori PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set(Qt5_USE_STATIC_LIBS ON)
#set(Qt5_USE_STATIC_RUNTIME ON)

target_compile_definitions(kumori PRIVATE
    UNICODE _UNICODE
    _USING_V110_SDK71_
    QT_DEPRECATED_WARNINGS
    QT_FORCE_ASSERTS
    QT_MESSAGELOGCONTEXT
)

target_include_directories(kumori PRIVATE ${Qt5WinExtras_INCLUDE_DIRS})

target_compile_definitions(kumori
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(kumori PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::WinExtras
    user32 dwmapi wbemuuid gdi32
)

qt5_import_qml_plugins(kumori)

set_target_properties(kumori PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_qml_module(kumori
        URI kumori
        VERSION 0.1
        QML_FILES ${QML_FILES}
    )

    qt_import_qml_plugins(kumori)
    qt_finalize_executable(kumori)
endif()
