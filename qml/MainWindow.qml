import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import kumori 0.1

Window {
    id: base
    color: 'transparent'
    flags: Qt.WindowStaysOnBottomHint | Qt.FramelessWindowHint | Qt.Popup | Qt.WindowTransparentForInput

    property var window

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
            console.log('Reload UI')

            if (window)
                window.destroy()

            window = wcomp.createObject(base)

            Kumori.clearComponentCache()

            var component = Qt.createComponent('file:///' + Kumori.userImportDir + '/Root.qml');
            if (component.status === Component.Ready)
                component.createObject(window);
            else
                print(component.errorString())

            window.showMaximized()
        }
    }

    function reload() {
        timer.restart()
    }
}
