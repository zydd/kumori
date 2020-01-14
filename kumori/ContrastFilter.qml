import QtQuick 2.0
import QtGraphicalEffects 1.12

ShaderEffect {
    anchors.fill: sourceItem

    property alias sourceItem: src.sourceItem
    property alias sourceRect: src.sourceRect
    property alias bgItem: bg.sourceItem
    property alias bgRect: bg.sourceRect

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
        sourceRect: Qt.rect(sourceItem.x/4, sourceItem.y/4, sourceItem.width/4, sourceItem.height/4)
    }

    property var source: ShaderEffectSource {
        id: src
        hideSource: true
    }

    fragmentShader: Qt.resolvedUrl('textcontrast.frag')
}
