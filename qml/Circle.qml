import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

ShaderEffectSource {
    id: root

    sourceRect: Qt.rect(x, y, width, height)
    live: false

    property bool open: false
    property real radius: 0.0
    property point dir: Qt.point(0, 0);
//    property alias mouseArea: ma

    opacity: 0
    visible: opacity

    onRadiusChanged: {
        dir.x += Math.random() * 0.5 - 0.25
        dir.y += Math.random() * 0.5 - 0.25
    }

    states: [
        State {
            name: 'open'
            when: root.open

            PropertyChanges { target: root; radius: 4.0 }
            PropertyChanges { target: root; opacity: 1 }
        },
        State {
            name: 'closed'

            PropertyChanges { target: root; radius: 0 }
            PropertyChanges { target: root; dir: Qt.point(0.0, 0.0) }
            PropertyChanges { target: root; opacity: 0 }
        }
    ]

    transitions: [
        Transition {
            from: 'open'

            NumberAnimation {
                property: 'radius'
                easing.type: Easing.InCubic
                duration: 200
            }
            NumberAnimation {
                property: 'opacity'
                duration: 100
            }
        },
        Transition {
            NumberAnimation {
                property: 'radius'
                easing.type: Easing.OutCubic
                duration: 100
            }
            NumberAnimation {
                property: 'opacity'
                easing.type: Easing.InCubic
                duration: 100
            }
        }
    ]

    layer.enabled: true
    layer.effect: ShaderEffect {
        property real ratio: root.width/root.height
        property point dir: root.dir
        property real radius: root.radius

        fragmentShader: Qt.resolvedUrl('circle.frag')
    }

    Timer {
        running: dir.x || dir.y
        interval: 16
        onTriggered: {
            dir.x = dir.x * 0.5
            dir.y = dir.y * 0.5

            if (Math.abs(dir.x) < 0.05)
                dir.x = 0
            if (Math.abs(dir.y) < 0.05)
                dir.y = 0
        }
    }

//    MouseArea {
//        id: ma
//        anchors.fill: parent
//        hoverEnabled: true

//        property point dir: Qt.point(0, 0);
//        property point last: Qt.point(0, 0);



//        onPositionChanged: {
//            dir.x += (mouse.x - last.x) / width * 2
//            last.x = mouse.x

//            dir.y += (last.y - mouse.y) / height * 2
//            last.y = mouse.y
//        }

//        onEntered: {
//            last.x = mouseX
//            last.y = mouseY
//        }

//        onClicked: {
//            var dx = mouseX/width * 2 - 1
//            var dy = mouseY/height * 2 - 1
//            var r = Math.sqrt(dx*dx + dy*dy)
//            var th = Math.atan2(dy, dx)
//            if (th < 0)
//                th += 2 * Math.PI;

//            root.clicked(r, th)
//        }
//    }
}
