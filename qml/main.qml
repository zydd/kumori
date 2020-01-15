import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import kumori 0.1

Window {
    id: base
    color: 'black'
    flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint

    property var window
    property var error

    Component {
        id: wcomp

        Window {
            width: base.width
            height: base.height
            x: base.x
            y: base.y

            color: 'transparent'
            flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint | Qt.SubWindow
        }
    }

    Timer {
        id: timer
        running: true
        repeat: false
        interval: 300
        onTriggered: {
            console.log('loading UI')

            if (window) window.destroy()

            if (error) {
                base.visible = false
                error.destroy()
            }

            window = wcomp.createObject(base)

            Kumori.clearComponentCache()

            var component = Qt.createComponent('file:///' + Kumori.userImportDir + '/Root.qml');
            if (component.status === Component.Ready) {
                component.createObject(window);
                window.showMaximized()
            } else {
                showError(component.errorString())
            }
        }
    }

    function showError(msg) {
        if (window) window.destroy()
        error = Qt.createQmlObject('import QtQuick 2.12; Text { color: "white"; text: "' + msg +
                                   '"; wrapMode: Text.Wrap; anchors.fill: parent; }', base)
        base.visible = true
    }

    function reload() { timer.restart() }
}
