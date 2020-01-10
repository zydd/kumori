import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import desktop 0.1

Window {
    id: window

    visible: true
    flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint // | Qt.WindowTransparentForInput
    color: 'transparent'

    Item {
        id: root
        anchors.fill: parent

        Wallpaper {
            id: wallpaper
            anchors.fill: parent
        }

        Launcher {
            anchors.fill: parent
        }

        Youbi {
            x: root.width - width
            y: root.height - height
        }
    }
}
