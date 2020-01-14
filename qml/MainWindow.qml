import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import kumori 0.1

Window {
    id: window

    visible: true
    flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint // | Qt.WindowTransparentForInput
    color: 'transparent'

    property var root

    Timer {
        id: timer
        running: true
        repeat: false
        interval: 300
        onTriggered: {
            console.log('Reload UI')

            if (root)
                root.destroy()

            Kumori.clearComponentCache()

            var component = Qt.createComponent('file:///' + Kumori.userImportDir + '/Root.qml');
            if (component.status === Component.Ready)
                root = component.createObject(window);
            else
                print(component.errorString())
        }
    }

    function reload() {
        timer.restart()
    }
}
