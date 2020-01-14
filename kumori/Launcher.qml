import QtQuick 2.0

Item {
    anchors.fill: parent

    function showAt(x, y) {
        circle.x = x - circle.width/2
        circle.y = y - circle.height/2
        focus = true
        circle.open = true
    }

    function hide() {
        circle.open = false
    }

    onFocusChanged: if (!focus) hide()

    Connections {
        target: window
        onActiveChanged: if (!active) hide()
    }

    Circle {
        id: circle

        x: parent.width/2 - width/2
        y: parent.height/2 - height/2

        width: 400
        height: 400
    }

    Item {
        id: icons

        anchors.fill: circle
        opacity: circle.opacity
        visible: circle.visible

        property real radius: circle.width/2 * circle.radius/6

        Component.onCompleted: {
            launcher.programsChanged.connect(reload)
            reload()
        }

        function reload() {
            for (var c in children)
                children[c].destroy()

            for (var i = 0; i < launcher.programs.length; ++i) {
                icon.createObject(icons, { p: i / launcher.programs.length,
                                           source: 'image://licon/' + launcher.programs[i] })
            }
        }

        Component {
            id: icon
            Image {
                width: 48
                height: width
                x: icons.width/2 + Math.cos(2 * Math.PI * p) * icons.radius - width/2
                y: icons.height/2 + Math.sin(2 * Math.PI * p) * icons.radius - height/2
                z: 2
                property real p
                cache: false
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MidButton | Qt.RightButton
        hoverEnabled: circle.visible

        property var presStart
        property point last: Qt.point(-1, -1)

        function polar(p) {
            var dx = (p.x - circle.x) / circle.width * 2 - 1
            var dy = (p.y - circle.y) / circle.width * 2 - 1

            var r = Math.sqrt(dx*dx + dy*dy)
            var th = Math.atan2(dy, dx) / Math.PI / 2
            if (th < 0) th += 1
            return Qt.point(r, th)
        }

        onPressed: {
            if (circle.open) {
                var p = polar(mouse)
                if (p.x < 3/6 || p.x > 5/6) {
                    hide()
                    mouse.accepted = false
                }
            } else if (mouse.button === Qt.RightButton) {
                circle.open = true
                presStart = mouse

                circle.x = mouseX - circle.width/2
                circle.y = mouseY - circle.height/2

                last.x =  (mouse.x - circle.x) / circle.width * 2 - 1
                last.y = -(mouse.y - circle.y) / circle.width * 2 + 1
            } else {
                mouse.accepted = false
            }
        }

        onPositionChanged: {
            // animate when dragging right-click
            if (circle.visible) {
                var x =  (mouse.x - circle.x) / circle.width * 2 - 1
                var y = -(mouse.y - circle.y) / circle.width * 2 + 1

                var dx = x - last.x
                var dy = y - last.y
                last.x = x
                last.y = y

                var r = Math.sqrt(x*x + y*y)
                var m = Math.min(Math.sqrt(dx*dx + dy*dy) * 2, 1) / (1 + Math.pow(0.5 * r, 4))

                if (m > 0.05) {
                    circle.dir.x = x * m / r
                    circle.dir.y = y * m / r
                }
            }
        }

        onReleased: {
            var p = polar(mouse)
            var s = p.y * launcher.programs.length + 0.5

            if (presStart) {
                var dx = mouseX - presStart.x
                var dy = mouseY - presStart.y

                if (p.x > 3/6 && p.x < 2) {
                    hide()
                    launcher.launch(launcher.programs[Math.floor(s % launcher.programs.length)])
                }

                presStart = null
            } else {
                if (circle.open && mouse.button === Qt.LeftButton) {
                    hide()

                    if (p.x > 3/6 && p.x < 5/6) {
                        launcher.launch(launcher.programs[Math.floor(s % launcher.programs.length)])
                    }
                }
            }
        }
    }
}



