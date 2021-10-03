import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

ShaderEffect {
    id: circle

    property real ratio: width/height

    fragmentShader: Qt.resolvedUrl('circle.frag')

    property bool open: false
    property real radius: 0.0
    property point dir: Qt.point(0, 0);

    opacity: 0

    onRadiusChanged: {
        dir.x += Math.random() * 0.5 - 0.25
        dir.y += Math.random() * 0.5 - 0.25
    }

    states: [
        State {
            name: 'open'
            when: circle.open

            PropertyChanges { target: circle; radius: 4.0 }
            PropertyChanges { target: circle; opacity: 1 }
        },
        State {
            name: 'closed'

            PropertyChanges { target: circle; radius: 0 }
            PropertyChanges { target: circle; dir: Qt.point(0.0, 0.0) }
            PropertyChanges { target: circle; opacity: 0 }
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
}
