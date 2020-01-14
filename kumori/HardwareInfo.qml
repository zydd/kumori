import QtQuick 2.0
import kumori 0.1

ListView {
    id: ohm
    interactive: false
    implicitHeight: contentHeight
    implicitWidth: 300
    visible: count > 0

    property int updateInterval: Ohm.updateInterval
    property bool autoUpdate: Ohm.autoUpdate
    property color textColor: 'white'

    Component.onCompleted: {
        Ohm.setUpdateInterval(updateInterval)
        Ohm.setAutoUpdate(autoUpdate)

        updateIntervalChanged.connect(() => Ohm.setUpdateInterval(updateInterval))
        autoUpdateChanged.connect(() => Ohm.setAutoUpdate(autoUpdate))
    }

    model: Ohm
    delegate: Item {
        implicitHeight: 20
        width: parent.width
        Text {
            id: txt1
            width: parent.width * 3/4
            text: path
            color: textColor
            font.pixelSize: 18
        }
        Text {
            id: txt2
            width: parent.width * 1/4
            anchors.right: parent.right
            text: Math.round(value * 100) / 100
            font: txt1.font
            color: textColor
            horizontalAlignment: Text.AlignRight
        }
    }
}
