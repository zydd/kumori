import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import kumori 0.1

Item {
    id: root

    property string header: `
        #define MULTISAMPLE_W ${timer.running ? 1 : param.multisample}
        #define MULTISAMPLE_H ${timer.running ? 1 : param.multisample}
        #define SHOW_MAP ${param.map_size > 0 ? 1 : 0}
        #define main_image ${param.main_image}
        #define mini_image ${param.mini_image}
        #define fractal_fn(z, c) ${param.formula}
        #define get_color color_${param.color_method}
    `
    property string shader

    QtObject {
        id: param
        objectName: 'fractal'
        property alias multisample: multisampleSlider.value
        property alias low_spec: lowSpecCheckBox.checked
        property alias c_x: cXField.value
        property alias c_y: cYField.value
        property alias center_x: centerXField.value
        property alias center_y: centerYField.value
        property alias zoom: zoomField.value
        property int iter: 157
        property real esc: 10
        property int pre: 0
        property alias col: colField.value
        property alias col_shift: colShiftField.value
        property alias col_variation: colVariationField.value
        property real map_size: 0
        property real map_zoom: 0.3
        property real rotation: 0
        property string main_image: 'julia'
        property string mini_image: 'fractal'
        property string formula: 'clog(vec2(z.x, abs(z.y))) + c'
        property string color_method: 'combined'
    }

    QtObject {
        id: ctrl
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
    }

    Settings {
        property alias c_x: param.c_x
        property alias c_y: param.c_y
        property alias center_x: param.center_x
        property alias center_y: param.center_y
        property alias zoom: param.zoom
        property alias iter: param.iter
        property alias esc: param.esc
        property alias pre: param.pre
        property alias col: param.col
        property alias col_shift: param.col_shift
        property alias col_variation: param.col_variation
        property alias map_size: param.map_size
        property alias map_zoom: param.map_zoom
        property alias rotation: param.rotation
        property alias main_image: param.main_image
        property alias mini_image: param.mini_image
        property alias formula: param.formula
        property alias color_method: param.color_method

        property alias paramWindow_visible: paramWindow.visible
        property alias paramWindow_x: paramWindow.x
        property alias paramWindow_y: paramWindow.y
    }

    ShaderEffect {
        id: fractal
        anchors.fill: parent

        property point c: Qt.point(param.c_x, param.c_y)
        property int iter: param.iter
        property real esc: param.esc
        property int pre: param.pre
        property real col: param.col
        property real col_shift: param.col_shift
        property real col_variation: param.col_variation
        property real zoom: param.zoom
        property real map_zoom: param.map_zoom
        property real map_size: param.map_size
        property real ratio: width / height
        property point center: Qt.point(param.center_x, param.center_y)
        property real rotation: param.rotation
        property size pixel: Qt.size(1 / width, 1 / height);

        fragmentShader: root.shader ? root.header + root.shader : ''

        layer.enabled: (timer.running | ma.pressed) && param.low_spec
        layer.textureSize: Qt.size(width/2, height/2)
    }

    Timer {
        id: mstimer
        interval: 800
        onTriggered: param.multisample = 4
    }

    Timer {
        id: timer
        running:
            Math.abs(ctrl.a.x) + Math.abs(ctrl.a.y) + Math.abs(ctrl.zoom_a)
            + Math.abs(ctrl.map_zoom_a) + Math.abs(ctrl.rot_a)
            + Math.abs(ctrl.center_a.x) + Math.abs(ctrl.center_a.y) > 0.5
        interval: 16
        repeat: true

        onRunningChanged:
            if (running) {
                mstimer.stop()
            } else {
                console.log(JSON.stringify(param))
                if (!param.low_spec)
                    mstimer.restart()
            }

        onTriggered: {
            ctrl.zoom_v += ctrl.zoom_a * 0.0001
            ctrl.zoom_v = clamp(ctrl.zoom_v, -0.03, 0.03)
            param.zoom *= 1 + ctrl.zoom_v

            ctrl.map_zoom_v += ctrl.map_zoom_a * 0.0001
            ctrl.map_zoom_v = clamp(ctrl.map_zoom_v, -0.03, 0.03)
            param.map_zoom *= 1 + ctrl.map_zoom_v

            ctrl.rot_v += ctrl.rot_a * 0.0001
            ctrl.rot_v = clamp(ctrl.rot_v, -0.01, 0.01)
            param.rotation += ctrl.rot_v

            ctrl.v.x += ctrl.a.x * 0.0001
            ctrl.v.y += ctrl.a.y * 0.0001
            ctrl.v.x = clamp(ctrl.v.x, -0.005, 0.005)
            ctrl.v.y = clamp(ctrl.v.y, -0.005, 0.005)

            param.c_x += ctrl.v.x * param.map_zoom
            param.c_y += ctrl.v.y * param.map_zoom

            ctrl.center_v.x += ctrl.center_a.x * 0.0001
            ctrl.center_v.y += ctrl.center_a.y * 0.0001
            ctrl.center_v.x = clamp(ctrl.center_v.x, -0.02, 0.02)
            ctrl.center_v.y = clamp(ctrl.center_v.y, -0.02, 0.02)

            translate(ctrl.center_v.x * param.zoom, ctrl.center_v.y * param.zoom)
        }

        function clamp(value, min, max) {
          return Math.min(Math.max(value, min), max);
        }
    }

    function translate(dx, dy) {
        param.center_x += Math.cos(param.rotation) * dx - Math.sin(param.rotation) * dy
        param.center_y += Math.sin(param.rotation) * dx + Math.cos(param.rotation) * dy
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
            param.multisample = 1
            var dx = (mouse.x - clickStart.x) / width * 2 * width / height
            var dy = -(mouse.y - clickStart.y) / height * 2

            translate(-dx * param.zoom, -dy * param.zoom)

            clickStart = Qt.point(mouse.x, mouse.y)
        }

        onWheel: {
            param.multisample = 1
            if (wheel.angleDelta.y > 0)
                zoomAt(1/(1 + wheel.angleDelta.y/360), wheel)
            else
                zoomAt(1-wheel.angleDelta.y/360, wheel)
        }

        function pos2coord(pos) {
            var c = Qt.point(pos.x / width * 2 - 1, -pos.y / height * 2 + 1)

            c.x *= width / height * param.zoom
            c.y *= param.zoom

            var d = Qt.point(Math.cos(param.rotation) * c.x - Math.sin(param.rotation) * c.y,
                             Math.sin(param.rotation) * c.x + Math.cos(param.rotation) * c.y)

            d.x += param.center_x
            d.y += param.center_y

            return d
        }

        function zoomAt(factor, pos) {
            var coord = pos2coord(pos)

            param.center_x = param.center_x * factor + coord.x * (1-factor)
            param.center_y = param.center_y * factor + coord.y * (1-factor)

            param.zoom *= factor
        }
    }

    Keys.onPressed: {
        // No repeat, no redraw
        if (!event.isAutoRepeat) switch (event.key) {
        case Qt.Key_H:
            param.map_size = param.map_size == 0 ? 0.3 : 0
            return

        case Qt.Key_G:
            param.multisample = 4
            // param.map_size = 0

            let size = Qt.size(fractal.width * 2, fractal.height * 2)
            let timestamp = new Date().getTime()
            var screenshotpath = `${Kumori.currentDir}/screenshots`
            var filename = `${screenshotpath}/kd-${timestamp}.jpg`

            var logline = `kd-${timestamp}.jpg: `
            logline += JSON.stringify(param)
            logline += '\n'
            Kumori.appendLog(`file:///${screenshotpath}/params.txt`, logline)

            fractal.grabToImage((img) => {
                                    console.log(`screenshot: ${filename}`)
                                    img.saveToFile(filename)
                                }, size)
            return

        case Qt.Key_P:
            preset_ducks()
            return

        case Qt.Key_Tab:
            paramWindow.visible = !paramWindow.visible
            return

        case Qt.Key_F:      param.low_spec = !param.low_spec; return
        }

        param.multisample = 1
        if (!param.low_spec)
            mstimer.restart()

        // Redraw

        switch (event.key) {
        case Qt.Key_1:      param.iter = Math.max(1, param.iter - 1); return
        case Qt.Key_2:      param.iter += 1; return
        case Qt.Key_3:      param.pre = Math.max(1, param.pre - 1); return
        case Qt.Key_4:      param.pre += 1; return
        case Qt.Key_Q:      param.col -= 2 * Math.PI; return
        case Qt.Key_W:      param.col += 2 * Math.PI; return
        case Qt.Key_A:
            param.col_shift = (param.col_shift - 0.05 < 0) ?
                        param.col_shift - 0.05 + 2*Math.PI :
                        param.col_shift - 0.05
            return
        case Qt.Key_S:
            param.col_shift = (param.col_shift + 0.05 > 2*Math.PI) ?
                        param.col_shift + 0.05 - 2*Math.PI :
                        param.col_shift + 0.05
            return
        }

        if (!event.isAutoRepeat) switch (event.key) {
            case Qt.Key_Up:     ctrl.a.y =  1; break
            case Qt.Key_Down:   ctrl.a.y = -1; break
            case Qt.Key_Left:   ctrl.a.x = -1; break
            case Qt.Key_Right:  ctrl.a.x =  1; break
            case Qt.Key_Z:      ctrl.zoom_a =  1; break
            case Qt.Key_X:      ctrl.zoom_a = -1; break
            case Qt.Key_V:      ctrl.map_zoom_a =  1; break
            case Qt.Key_B:      ctrl.map_zoom_a = -1; break
            case Qt.Key_I:      ctrl.center_a.y =  1; break
            case Qt.Key_K:      ctrl.center_a.y = -1; break
            case Qt.Key_J:      ctrl.center_a.x = -1; break
            case Qt.Key_L:      ctrl.center_a.x =  1; break
            case Qt.Key_R:      ctrl.rot_a = -1; break
            case Qt.Key_T:      ctrl.rot_a =  1; break
            case Qt.Key_O:      param.rotation += Math.PI/2; break

            case Qt.Key_Period:
                if (param.main_image === 'fractal') {
                    param.main_image = 'julia'
                    param.mini_image = 'fractal'
                } else {
                    param.main_image = 'fractal'
                    param.mini_image = 'julia'
                }
                var t
                t = param.center_x; param.center_x = param.c_x; param.c_x = t
                t = param.center_y; param.center_y = param.c_y; param.c_y = t
                t = param.zoom; param.zoom = param.map_zoom; param.map_zoom = t
                break
        }
    }

    Keys.onReleased: {
        if (event.isAutoRepeat)
            return

        switch (event.key) {
            case Qt.Key_C:
                param.center_x = 0
                param.center_y = 0
                ctrl.center_v.x = 0
                ctrl.center_v.y = 0
                break

            case Qt.Key_Y:
                ctrl.rot_a = 0
                ctrl.rot_v = 0
                param.rotation = 0
                break

            case Qt.Key_M:
                param.map_size = 0
                param.multisample = 4
                break

            case Qt.Key_N:
                param.map_size = 0
                param.multisample = 6
                break

            case Qt.Key_Up:
            case Qt.Key_Down:   ctrl.a.y = 0; ctrl.v.y = 0; break
            case Qt.Key_Left:
            case Qt.Key_Right:  ctrl.a.x = 0; ctrl.v.x = 0; break
            case Qt.Key_Z:
            case Qt.Key_X:      ctrl.zoom_a = 0; ctrl.zoom_v = 0; break
            case Qt.Key_V:
            case Qt.Key_B:      ctrl.map_zoom_a = 0; ctrl.map_zoom_v = 0; break
            case Qt.Key_I:
            case Qt.Key_K:      ctrl.center_a.y = 0; ctrl.center_v.y = 0; break
            case Qt.Key_J:
            case Qt.Key_L:      ctrl.center_a.x = 0; ctrl.center_v.x = 0; break
            case Qt.Key_R:
            case Qt.Key_T:      ctrl.rot_a = 0; ctrl.rot_v = 0; break
        }
    }

    function preset_ducks() {
        param.c_x = -0.623048
        param.c_y = 4.61666
        param.center_x = -2.61087
        param.center_y = 0.40818
        param.zoom = 0.12224574942781402
        param.iter = 181
        param.pre = 0
        param.col = 400.0
        param.col_shift = 3.6776
        param.rotation = 0
        param.map_zoom = 0.12
        param.multisample = 4
        param.main_image = 'julia'
        param.mini_image = 'fractal'
        param.formula = 'clog(vec2(z.x, abs(z.y))) + c'
        param.color_method = 'cyclic_log_log_yb'
    }

    function preset_mandelbrot() {
        param.c_x = -0.4
        param.c_y = 0.6
        param.center_x = 0.0
        param.center_y = 0.0
        param.zoom = 2.0
        param.iter = 64
        param.pre = 0
        param.col = 1.0
        param.col_shift = 0.45
        param.rotation = 0
        param.map_zoom = 0.12
        param.multisample = 4
        param.main_image = 'julia'
        param.mini_image = 'fractal'
        param.formula = 'cmul(z, z) + c'
        param.color_method = 'iteration'
    }

    function preset_nautilus() {
        var config = {
            "objectName": "fractal",
            "iter": 64,
            "esc": 10,
            "pre": 0,
            "map_size": 0,
            "map_zoom": 0.06820460072736786,
            "rotation": 1.028299999999999,
            "main_image": "julia",
            "mini_image": "fractal",
            "formula": "cmul(z, abs(z)) + c",
            "color_method": "combined",
            "multisample": 4,
            "low_spec": false,
            "c_x": 0.38854011109653114,
            "c_y": 0.11681436824954756,
            "center_x": -0.19674004488140456,
            "center_y": 0.45870767596535933,
            "zoom": 0.9935743920840994,
            "col": 22.430370614359212,
            "col_shift": 3.4500791190303746,
            "col_variation": 5
        }
        Object.assign(param, config)
    }

    Component.onCompleted: {
        http_get(Qt.resolvedUrl('ducks.frag'), (str) => root.shader = str)
    }

    Window {
        id: paramWindow
        visible: false

        width: settingsLayout.implicitWidth
        height: settingsLayout.implicitHeight

        property int iter: 157
        property real esc: 10
        property int pre: 0
        property real map_size: 0
        property real map_zoom: 0.3
        property real rotation: 0
        property string main_image: 'julia'
        property string mini_image: 'fractal'

        onClosing: {
            close.accepted = false
            visible = false
        }

        component FloatField : TextField {
            property real value
            onEditingFinished: value = parseFloat(text)
            onValueChanged: text = value
            selectByMouse: true
            validator: DoubleValidator { }
        }

        Control {
            anchors.fill: parent
            id: settingsLayout
            padding: 10

            contentItem:
            GridLayout {
                columns: 5

                Label { text: 'Preset:'; Layout.columnSpan: 2 }
                ComboBox {
                    Layout.columnSpan: 3
                    Layout.fillWidth: true
                    id: presetInput
                    wheelEnabled: true
                    model: [
                        'ducks',
                        'mandelbrot',
                        'nautilus',
                    ]
                    property string text: 'ducks'
                    onCurrentTextChanged: {
                        text = currentText
                        root['preset_' + currentText]()
                    }
                    onTextChanged: currentIndex = find(text, Qt.MatchExactly)
                }

                Label { text: 'Formula:'; Layout.columnSpan: 2 }
                TextField {
                    Layout.columnSpan: 3
                    Layout.fillWidth: true
                    id: formulaInput
                    text: param.formula
                    selectByMouse: true

                    onEditingFinished: param.formula = text
                    Connections {
                        target: param
                        function onFormulaChanged() { formulaInput.text = param.formula }
                    }
                }

                Label { text: 'Coloring:'; Layout.columnSpan: 2 }
                ComboBox {
                    Layout.columnSpan: 3
                    Layout.fillWidth: true
                    id: coloringInput
                    wheelEnabled: true
                    model: [
                        'combined',
                        'norm_itr_count',
                        'iteration',
                        'clamp_cyclic_log',
                        'cyclic_log',
                        'cyclic_log_log_yb',
                        'cyclic_log_log_yb2',
                        'cyclic_log_log_pg',
                    ]
                    onCurrentTextChanged: param.color_method = currentText
                    Connections {
                        target: param
                        function onColor_methodChanged() {
                            coloringInput.currentIndex = coloringInput.find(param.color_method, Qt.MatchExactly)
                        }
                    }
                }

                Label { text: 'Low Spec:'; Layout.columnSpan: 2 }
                CheckBox {
                    Layout.columnSpan: 3
                    id: lowSpecCheckBox
                }

                Label { text: 'Multisample:'; Layout.columnSpan: 2 }
                RowLayout {
                    Layout.columnSpan: 3
                    Slider {
                        id: multisampleSlider
                        snapMode: Slider.SnapAlways
                        stepSize: 1
                        from: 1
                        to: param.low_spec ? 4 : 6
                    }
                    Label {
                        text: multisampleSlider.value ** 2
                    }
                }

                Label { text: 'c:' }
                Label { text: 'r:' }
                FloatField {
                    id: cXField
                    value: -0.626753
                }

                Label { text: 'i:' }
                FloatField {
                    id: cYField
                    value: 4.615968
                }

                Label { text: 'Center:' }
                Label { text: 'x:' }
                FloatField {
                    id: centerXField
                    value: -2.9958174
                }

                Label { text: 'y:' }
                FloatField {
                    id: centerYField
                    value: 0.0
                }

                Label { text: 'Zoom:'; Layout.columnSpan: 2 }
                FloatField {
                    Layout.columnSpan: 3
                    id: zoomField
                    value: 0.554781
                }

                Label { text: 'Color:' }


                Label { text: 'variation:' }
                FloatField {
                    id: colVariationField
                    value: 6.0
                }

                Label { text: 'factor:' }
                FloatField {
                    id: colField
                    value: 400.0
                }

                Item { width: 1 }
                Label { text: 'shift:' }
//                FloatField {
//                    id: colShiftField
//                    value: 0.0
//                }
                Slider {
                    Layout.columnSpan: 3
                    Layout.fillWidth: true
                    id: colShiftField
                    value: 0.0
                    from: 0
                    to: 2*Math.PI
                }

                Label { text: 'Params:' }
                TextField {
                    Layout.columnSpan: 5
                    Layout.fillWidth: true
                    onFocusChanged: focus && selectAll()
                    selectByMouse: true
                    property string paramstr: JSON.stringify(param)
                    onParamstrChanged: text = paramstr
                    onAccepted: {
                        var obj = JSON.parse(text)
                        Object.assign(param, obj)
                    }
                }

                Label { text: 'Eval:' }
                TextField {
                    Layout.columnSpan: 5
                    Layout.fillWidth: true
                    onFocusChanged: focus && selectAll()
                    selectByMouse: true
                    text: 'Kumori.getOpenWindows()'
                    Keys.onReturnPressed: {
                        eval(text)
                    }
                }
            }
        }
    }
}
