/**************************************************************************
 *  ContrastFilter.qml
 *
 *  Copyright 2024 Gabriel Machado
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 **************************************************************************/

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
