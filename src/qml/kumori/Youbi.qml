/**************************************************************************
 *  Youbi.qml
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

import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0


Text {
    id: txt

    font.pixelSize: height * 0.9
    font.family: 'Dejima,衡山毛筆フォント行書,YOzFontK,MS Mincho'
    font.weight: Font.Black

    horizontalAlignment: Qt.AlignCenter

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
