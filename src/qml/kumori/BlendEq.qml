import QtQuick 2.15
import kumori 0.1

ShaderEffect {
    id: root

    property alias foregroundSource: fg.sourceItem

    property var fg: ShaderEffectSource {
        id: fg
    }

    property string formula: `#define BLEND(bg, fg) vec4(bg.rgb * mix(vec3(1.0), bg.rgb, fg.a), 1.0)`

    fragmentShader: formula + Kumori.readFile(Qt.resolvedUrl('blend.frag'))
}
