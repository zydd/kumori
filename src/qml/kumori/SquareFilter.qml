import QtQuick 2.0
import QtGraphicalEffects 1.12

ShaderEffect {
    id: root
    anchors.fill: sourceItem

    property alias sourceItem: src.sourceItem
    property alias sourceRect: src.sourceRect
    property alias backgroundItem: bg.sourceItem
    property alias backgroundRect: bg.sourceRect

    property var bg: ShaderEffectSource {
        id: bg
        sourceRect: Qt.rect(root.sourceItem.x, root.sourceItem.y,
                            root.sourceItem.width, root.sourceItem.height)
    }

    property var source: ShaderEffectSource {
        id: src
        hideSource: true
    }

    fragmentShader: Qt.resolvedUrl('square.frag')
}
