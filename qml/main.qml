import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import kumori 0.1

Window {
    id: base
    color: 'black'
    flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint

    property var window
    property var error

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

            Kumori.clearComponentCache()

            window = Qt.createQmlObject('import QtQuick.Window 2.12; Window { color: "transparent";'
                                        + 'flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint | Qt.SubWindow }',
                                        base)

            var component = Qt.createComponent('file:///' + Kumori.userImportDir + '/Root.qml');
            if (component.status === Component.Ready) {
                component.createObject(window);
                window.x = Qt.binding(() => base.x)
                window.y = Qt.binding(() => base.y)
                window.width = Qt.binding(() => base.width)
                window.height = Qt.binding(() => base.height)
                window.showNormal()
            } else {
                showError(component.errorString())
            }
        }
    }

    function showError(msg) {
        console.log(msg)
        if (window) window.destroy()
        error = Qt.createQmlObject('import QtQuick 2.12; Text { color: "white"; text: "' + msg +
                                   '"; wrapMode: Text.Wrap; anchors.fill: parent; }', base)
        base.visible = true
    }

    function reload() { timer.restart() }
}
