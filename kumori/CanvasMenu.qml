import QtQuick 2.0

Item {
    id: root

    width: 300
    height: 300

    Canvas {
        id: canvas

        anchors.fill: parent

        property real radius: icon
        property real icon: 48 * 1.5
        property bool open: false

        opacity: 0

        onRadiusChanged: requestPaint()

        renderTarget: Canvas.FramebufferObject
        renderStrategy: Canvas.Cooperative

        Component.onCompleted: open = true

        states: [
            State {
                when: canvas.open
                PropertyChanges { target: canvas; radius: root.width/2 }
                PropertyChanges { target: canvas; opacity: 1 }
            },
            State {
                PropertyChanges { target: canvas; radius: icon }
                PropertyChanges { target: canvas; opacity: 0 }
            }
        ]

        transitions: Transition {
            NumberAnimation {
                property: 'radius'
                easing.type: Easing.OutCubic
                duration: 100
            }
            NumberAnimation {
                property: 'opacity'
//                easing.type: Easing.InCubic
                duration: 100
            }
        }

        onPaint: {
            console.log('shadows', ! transitions[0].running)
            var ctx = getContext("2d")
            ctx.save()

            ctx.clearRect(0, 0, width, height)

            ctx.fillStyle = '#4E4E4E'

            ctx.beginPath()
            ctx.arc(width/2, height/2, radius, 0, 2 * Math.PI, true);
            ctx.fill();

//            if (! transitions[0].running) {
//                ctx.shadowColor = '#000000';
//                ctx.shadowBlur = 5;
//                ctx.shadowOffsetX = 0;
//                ctx.shadowOffsetY = 2;
//                ctx.lineWidth = 4;

//                ctx.save()

//                ctx.beginPath()
//                ctx.arc(width/2, height/2, radius, 0, 2 * Math.PI, true);
//                ctx.closePath()
//                ctx.clip();

//                // outer shadow
//                ctx.beginPath()
//                ctx.arc(width/2, height/2, radius + ctx.lineWidth/2, 0, 2 * Math.PI, true);
//                ctx.closePath()
//                ctx.stroke();

//                ctx.restore()
//            }

            if (radius > icon + ctx.lineWidth/2) {
//                 inner shadow
//                if (! transitions[0].running) {
//                    ctx.beginPath()
//                    ctx.arc(width/2, height/2, radius - icon - ctx.lineWidth/2, 0, 2 * Math.PI, false);
//                    ctx.closePath()
//                    ctx.stroke();
//                }

                ctx.beginPath()
                ctx.arc(width/2, height/2, radius - icon, 0, 2 * Math.PI, true);
                ctx.clip()

                ctx.clearRect(0, 0, width, height);
            }

            ctx.restore()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: canvas.open = !canvas.open
    }
}
