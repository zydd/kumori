/**************************************************************************
 *  HardwareInfo.qml
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
import kumori 0.1

ListView {
    id: ohm
    interactive: false
    implicitHeight: contentHeight
    implicitWidth: 300
    visible: count > 0

    property color textColor: 'white'

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
