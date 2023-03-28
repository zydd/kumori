import QtQuick 2.12
import kumori 0.1

Item {
    id: root

    property int multisample: 6
    property bool low_spec: false

    property string header: `
        #define MULTISAMPLE_W ${timer.running ? 1 : multisample}
        #define MULTISAMPLE_H ${timer.running ? 1 : multisample}
        #define SHOW_MAP ${map_size > 0 ? 1 : 0}
        #define fractal ducks
    `
    property string shader

    property point c: Qt.point(-0.6267530000000018,4.615967999525822)
    property point center: Qt.point(-2.9958174002672897,0)
    property real zoom: 0.554781
    property int iter: 157
    property real esc: 10
    property int pre: 0
    property real col: 0.98136
    property real col_shift: 0
    property real map_size: 0
    property real map_zoom: 0.3
    property real rotation: 0

    ShaderEffect {
        id: fractal
        anchors.fill: parent

        property point c: root.c
        property int iter: root.iter
        property real esc: root.esc
        property int pre: root.pre
        property real col: root.col
        property real col_shift: root.col_shift
        property real zoom: root.zoom
        property real map_zoom: root.map_zoom
        property real map_size: root.map_size
        property real ratio: width / height
        property point center: root.center
        property real rotation: root.rotation
        property var pixel: Qt.size(1 / width, 1 / height);

        fragmentShader: root.shader ? root.header + root.shader : ''

        layer.enabled: (timer.running | ma.pressed) && root.low_spec
        layer.textureSize: Qt.size(width/2, height/2)
    }

    property point center_v
    property point center_a
    property point v
    property point a
    property real zoom_a
    property real zoom_v
    property real map_zoom_a
    property real map_zoom_v
    property real rot_a
    property real rot_v

    Timer {
        id: timer
        running:
            Math.abs(v.x) + Math.abs(v.y) +
            Math.abs(a.x) + Math.abs(a.y) +
            Math.abs(zoom_v) + Math.abs(zoom_a) +
            Math.abs(zoom_v) + Math.abs(map_zoom_a) +
            Math.abs(rot_v) + Math.abs(rot_a) +
            Math.abs(center_v.x) + Math.abs(center_v.y) +
            Math.abs(center_a.x) + Math.abs(center_a.y) > 1e-20
        interval: 16
        repeat: true

        onRunningChanged:
            if (running)
                print('c:', c, 'zoom:', zoom, 'center:', center, 'col', col, 'col_shift', col_shift, 'iter', iter)
            else if (!root.low_spec)
                root.multisample = 4

        onTriggered: {
            zoom_v += zoom_a * 0.005
            zoom *= 1 + zoom_v
            zoom_v *= 0.8

            map_zoom_v += map_zoom_a * 0.005
            map_zoom *= 1 + map_zoom_v
            map_zoom_v *= 0.8

            rot_v += rot_a * 0.008
            rotation += rot_v
            rot_v *= 0.8

            v.x += a.x * 0.0005
            v.y += a.y * 0.0005

            c.x += v.x * map_zoom
            c.y += v.y * map_zoom

            v.x *= 0.8
            v.y *= 0.8

            center_v.x += center_a.x * 0.005 * zoom
            center_v.y += center_a.y * 0.005 * zoom

            translate(center_v.x , center_v.y)

            center_v.x *= 0.8
            center_v.y *= 0.8
        }
    }

    function translate(dx, dy) {
        center.x += Math.cos(rotation) * dx - Math.sin(rotation) * dy
        center.y += Math.sin(rotation) * dx + Math.cos(rotation) * dy
    }

    function http_get(url, callback) {
        var doc = new XMLHttpRequest()
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE)
                callback(doc.response)
        }
        doc.open('GET', url, true)
        doc.send()
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        property point clickStart

        onClicked: root.focus = true
        
        onPressed: clickStart = Qt.point(mouse.x, mouse.y)

        onPositionChanged: {
            var dx = (mouse.x - clickStart.x) / width * 2 * width / height
            var dy = -(mouse.y - clickStart.y) / height * 2

            translate(-dx * zoom, -dy * zoom)

            clickStart = Qt.point(mouse.x, mouse.y)
            multisample = 1
        }

        onWheel: {
            multisample = 1
            if (wheel.angleDelta.y > 0)
                zoomAt(1/(1 + wheel.angleDelta.y/360), wheel)
            else
                zoomAt(1-wheel.angleDelta.y/360, wheel)
        }

        function pos2coord(pos) {
            var c = Qt.point(pos.x / width * 2 - 1, -pos.y / height * 2 + 1)

            c.x *= width / height * zoom
            c.y *= zoom

            var d = Qt.point(Math.cos(root.rotation) * c.x - Math.sin(root.rotation) * c.y,
                             Math.sin(root.rotation) * c.x + Math.cos(root.rotation) * c.y)

            d.x += center.x
            d.y += center.y

            return d
        }

        function zoomAt(factor, pos) {
            var coord = pos2coord(pos)

            center.x = center.x * factor + coord.x * (1-factor)
            center.y = center.y * factor + coord.y * (1-factor)

            zoom *= factor
        }
    }

    Keys.onPressed: {
        switch(event.key) {
            case Qt.Key_Up:     a.y +=  1; break
            case Qt.Key_Down:   a.y += -1; break
            case Qt.Key_Left:   a.x += -1; break
            case Qt.Key_Right:  a.x +=  1; break
            case Qt.Key_Z:      zoom_a +=  1; break
            case Qt.Key_X:      zoom_a += -1; break
            case Qt.Key_V:      map_zoom_a +=  1; break
            case Qt.Key_B:      map_zoom_a += -1; break
            case Qt.Key_I:      center_a.y +=  1; break
            case Qt.Key_K:      center_a.y += -1; break
            case Qt.Key_J:      center_a.x += -1; break
            case Qt.Key_L:      center_a.x +=  1; break
            case Qt.Key_R:      rot_a += -1; break
            case Qt.Key_T:      rot_a +=  1; break

            // case Qt.Key_Escape: Qt.quit()

            case Qt.Key_M:      return
            case Qt.Key_N:      return
            default:            return
        }
        multisample = 1
    }

    Keys.onReleased: {
        switch(event.key) {
            case Qt.Key_H:
                map_size = map_size == 0 ? 0.3 : 0
                return

            case Qt.Key_G:
                if (root.low_spec) {
                    if (multisample < 4)
                        multisample = 4
                } else if (multisample < 6) {
                    multisample = 6
                }

                map_size = 0
                var filename = `screenshots/irz,${c.x},${c.y},${center.x},${center.y},${zoom}.png`
                console.log(`Screenshot: ${filename}`)
                fractal.grabToImage((img) => img.saveToFile(filename))
                return

            case Qt.Key_C:
                center.x = 0
                center.y = 0
                center_v.x = 0
                center_v.y = 0
                return

            case Qt.Key_Y:
                rot_a = 0
                rot_v = 0
                rotation = 0
                return

            case Qt.Key_M:
                map_size = 0
                multisample = 4
                return

            case Qt.Key_N:
                multisample = 6
                map_size = 0
                return

            case Qt.Key_P:
                preset()
                return
        }

        switch(event.key) {
            case Qt.Key_Up:     a.y -=  1; break
            case Qt.Key_Down:   a.y -= -1; break
            case Qt.Key_Left:   a.x -= -1; break
            case Qt.Key_Right:  a.x -=  1; break
            case Qt.Key_Z:      zoom_a -=  1; break
            case Qt.Key_X:      zoom_a -= -1; break
            case Qt.Key_V:      map_zoom_a -=  1; break
            case Qt.Key_B:      map_zoom_a -= -1; break
            case Qt.Key_I:      center_a.y -=  1; break
            case Qt.Key_K:      center_a.y -= -1; break
            case Qt.Key_J:      center_a.x -= -1; break
            case Qt.Key_L:      center_a.x -=  1; break
            case Qt.Key_R:      rot_a -= -1; break
            case Qt.Key_T:      rot_a -=  1; break
            case Qt.Key_1:      iter = iter > 0 ? iter - 1 : iter; break
            case Qt.Key_2:      iter += 1; break
            case Qt.Key_3:      pre = pre > 0 ? pre - 1 : pre; break
            case Qt.Key_4:      pre += 1; break
            case Qt.Key_Q:      col -= 1e-3; break
            case Qt.Key_W:      col += 1e-3; break
            case Qt.Key_A:      col_shift -= 1e-2; break
            case Qt.Key_S:      col_shift += 1e-2; break
            case Qt.Key_O:      rotation += Math.PI/2; break
            case Qt.Key_F:      root.low_spec = !root.low_spec; break
            default:            return
        }
        multisample = 0
    }

    function preset() {
        c = Qt.point(-0.623048, 4.61666)
        center = Qt.point(-2.61087, 0.40818)
        zoom = 0.12224574942781402
        col = 0.98236
        iter = 181
        col_shift = 0.15999999999999998
        rotation = 0
        multisample = 6
    }

    Component.onCompleted: {
        http_get(Qt.resolvedUrl('ducks.frag'), (str) => root.shader = str)
    }
}
