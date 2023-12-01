import QtQuick 2.0
import QtGraphicalEffects 1.12

ShaderEffect {
    id: root

    property alias backgroundItem: bg.sourceItem
    property alias backgroundRect: bg.sourceRect

    property var bg: ShaderEffectSource {
        id: bg
    }

    fragmentShader: Qt.resolvedUrl('square.frag')
}
