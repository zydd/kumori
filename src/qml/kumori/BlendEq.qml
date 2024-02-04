/**************************************************************************
 *  BlendEq.qml
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

import QtQuick 2.15
import kumori 0.1

ShaderEffect {
    id: root

    property var foregroundSource

    property string formula: `#define BLEND(bg, fg) vec4(bg.rgb * mix(vec3(1.0), bg.rgb, fg.a), 1.0)`

    fragmentShader: formula + Kumori.readFile(Qt.resolvedUrl('blend.frag'))
}
