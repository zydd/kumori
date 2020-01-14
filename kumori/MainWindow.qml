import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import desktop 0.1 as Desktop

Window {
    id: window

    visible: true
    flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint // | Qt.WindowTransparentForInput
    color: 'transparent'

    Desktop.Root { anchors.fill: parent }
}
