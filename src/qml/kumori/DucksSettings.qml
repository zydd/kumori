import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0

Control {
    id: root

    property QtObject param: QtObject {
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
        property bool flip: false
        property alias col_shift_r: colShiftFieldR.value
        property alias col_shift_g: colShiftFieldG.value
        property alias col_shift_b: colShiftFieldB.value
    }

    Settings {
        id: settings
        category: 'ducks'
        property bool saved: false

        Component.onCompleted: {
            if (saved) {
                for (var key in param) {
                    if (typeof param[key] != 'function')
                        param[key] = settings.value(key)
                }
            }
        }

        function save() {
            console.log('save')
            for (var key in param) {
                if (typeof param[key] != 'function')
                    settings.setValue(key, param[key])
            }
            settings.saved = true
        }
    }

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
                '',
                'ducks',
                'mandelbrot',
                'nautilus',
            ]
            property string text: 'ducks'
            onCurrentTextChanged: {
                text = currentText
                if (currentText) {
                    root['preset_' + currentText]()
                    root.save()
                }
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
                'combined2',
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
        Slider {
            id: colShiftField
            Layout.columnSpan: 3
            Layout.fillWidth: true
            value: 0.0
            from: 0
            to: 2*Math.PI
        }
        Item { width: 1 }
        Label { text: 'R:' }
        Slider {
            id: colShiftFieldR
            Layout.columnSpan: 3
            Layout.fillWidth: true
            value: 0.0
            from: 0
            to: 2*Math.PI
        }
        Item { width: 1 }
        Label { text: 'G:' }
        Slider {
            id: colShiftFieldG
            Layout.columnSpan: 3
            Layout.fillWidth: true
            value: 0.0
            from: 0
            to: 2*Math.PI
        }
        Item { width: 1 }
        Label { text: 'B:' }
        Slider {
            id: colShiftFieldB
            Layout.columnSpan: 3
            Layout.fillWidth: true
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
            text: ''
            Keys.onReturnPressed: {
                eval(text)
            }
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
        param.multisample = 2
        param.main_image = 'julia'
        param.mini_image = 'fractal'
        param.formula = 'clog(vec2(z.x, abs(z.y))) + c'
        param.color_method = 'cyclic_log_log_yb'
        param.flip = false
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
        param.flip = false
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
            "col_variation": 5,
            "flip": true,
        }
        Object.assign(param, config)
    }

    component FloatField : TextField {
        property real value
        onEditingFinished: value = parseFloat(text)
        onValueChanged: text = value
        selectByMouse: true
        validator: DoubleValidator { }
    }

    function save() {
        settings.save()
    }
}
