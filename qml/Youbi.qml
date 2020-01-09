import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

Item {
    id: youbi

    implicitWidth: txt.implicitWidth
    implicitHeight: txt.implicitHeight

    Text {
        id: txt

        font.pixelSize: root.height / 3
        font.family: '衡山毛筆フォント行書,Dejima,YOzFontK'
        font.weight: Font.Black

        Timer {
            running: true
            interval: 1000
            repeat: true
            triggeredOnStart: true

            onTriggered: {
                var date = new Date()

                parent.text = date.toLocaleString(Qt.locale('ja'), 'ddd')

                interval = 24 * 3600 * 1000 -
                        ((date.getHours() * 60
                          + date.getMinutes()) * 60
                         + date.getSeconds()) * 1000
            }
        }
    }

    ShaderEffectSource {
        id: eff

        anchors.fill: txt

        sourceItem: txt
        live: true
        hideSource: true

        layer.enabled: true
        layer.effect: ShaderEffect {
            property real ratio: eff.width / eff.height
            property var bg: ShaderEffectSource {
                sourceItem: wallpaper
                sourceRect: Qt.rect(youbi.x, youbi.y, youbi.width, youbi.height)
            }

            fragmentShader: Qt.resolvedUrl('youbi.frag')
        }
    }

    MouseArea {
        id: ma
        anchors.fill: youbi
        hoverEnabled: true
        drag.target: youbi
    }
}
