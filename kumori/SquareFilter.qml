import QtQuick 2.0
import QtGraphicalEffects 1.12

ShaderEffect {
    anchors.fill: sourceItem

    property alias sourceItem: src.sourceItem
    property alias sourceRect: src.sourceRect
    property alias bgItem: bg.sourceItem
    property alias bgRect: bg.sourceRect

    property var bg: ShaderEffectSource {
        id: bg
        sourceRect: Qt.rect(sourceItem.x, sourceItem.y, sourceItem.width, sourceItem.height)
    }

    property var source: ShaderEffectSource {
        id: src
        hideSource: true
    }

    fragmentShader: Qt.resolvedUrl('square.frag')
}
