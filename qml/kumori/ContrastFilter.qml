import QtQuick 2.0
import QtGraphicalEffects 1.12

ShaderEffect {
    id: root
    anchors.fill: sourceItem

    property alias sourceItem: src.sourceItem
    property alias sourceRect: src.sourceRect
    property alias backgroundItem: blurredBg.source

    GaussianBlur {
        id: blurredBg
        width: wallpaper.width/4
        height: wallpaper.height/4
        visible: false
        radius: 8
    }

    property var bg: ShaderEffectSource {
        id: bg
        sourceItem: blurredBg
        sourceRect: Qt.rect(root.sourceItem.x/4, root.sourceItem.y/4,
                            root.sourceItem.width/4, root.sourceItem.height/4)
    }

    property var source: ShaderEffectSource {
        id: src
        hideSource: true
    }

    fragmentShader: Qt.resolvedUrl('textcontrast.frag')
}
