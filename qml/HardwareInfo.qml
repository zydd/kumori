import QtQuick 2.0
import QtGraphicalEffects 1.12
import desktop 0.1

ListView {
    id: ohm
    interactive: false

    visible: count > 0

    property int updateInterval: Ohm.updateInterval
    property bool autoUpdate: Ohm.autoUpdate

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
            font.pixelSize: 18
        }
        Text {
            id: txt2
            width: parent.width * 1/4
            anchors.right: parent.right
            text: Math.round(value * 100) / 100
            font: txt1.font
            horizontalAlignment: Text.AlignRight
        }
    }

    GaussianBlur {
        id: wallpaper_blurred
        source: wallpaper
        width: wallpaper.width/4
        height: wallpaper.height/4
        visible: false
        radius: 8
    }

    layer.enabled: true
    layer.effect: ShaderEffect {
        property var bg: ShaderEffectSource {
            sourceItem: wallpaper_blurred
            sourceRect: Qt.rect(ohm.x/4, ohm.y/4, ohm.width/4, ohm.height/4)
        }
        fragmentShader: Qt.resolvedUrl('textcontrast.frag')
    }
}
