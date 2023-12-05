import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import kumori 0.1


ApplicationWindow {
    id: root
    color: 'transparent'
    flags: Qt.FramelessWindowHint

    x: 0
    y: Screen.height - height
    width: Screen.width
    height: 36

    font.pointSize: 8
    palette.button: 'transparent'

    Timer {
        running: true
        interval: 100
        onTriggered: {
            TrayService.setTaskBar(root)
            WmService.init()
            root.visible = true
        }

        Component.onDestruction: TrayService.restoreSystemTaskbar()
    }

    Rectangle {
        anchors.fill: parent
        color: '#ddd'
        opacity: 0.1
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ListView {
            id: taskView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: contentWidth
            orientation: ListView.Horizontal
            model: WmService
            clip: true
            //    boundsBehavior: Flickable.StopAtBounds

            delegate: ToolButton {
                width: 300
                height: taskView.height
                contentItem: Label {
                    text: model.nativeWindow.title
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    color: 'white'
                    elide: Text.ElideRight
                    leftPadding: winIcon.width + parent.padding

                    LiveIcon {
                        id: winIcon
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        liveIcon: model.nativeWindow.icon
                        width: 24
                        height: 24
                    }
                }

                background: Rectangle {
                    color: model.nativeWindow.active ? '#fff' : 'transparent'
                    opacity: 0.1
                }

                onClicked:
                    if (model.nativeWindow.active)
                        model.nativeWindow.minimize()
                    else
                        model.nativeWindow.toFront()
            }
        }

        ListView {
            id: trayView
            //    Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: contentWidth
            orientation: ListView.Horizontal
            layoutDirection: Qt.RightToLeft
            model: TrayService.trayItems
            clip: true

            delegate: ToolButton {
                width: trayView.height
                height: trayView.height
//                hoverEnabled: true

//                ToolTip.delay: 1000
//                ToolTip.timeout: 5000
//                ToolTip.visible: hovered && modelData.tooltip
//                ToolTip.text: modelData.tooltip

                contentItem:
                    TrayIcon {
                        anchors.centerIn: parent
                        liveIcon: modelData
                        width: 24
                        height: 24
                    }
            }
        }

        Label {
            id: time
            color: 'white'
            Layout.preferredWidth: 80
            font.pixelSize: parent.height * 0.65
            font.weight: Font.Light
            font.family: 'Segoe UI'
            horizontalAlignment: Qt.AlignCenter

            // disable subpixel antialiasing
            style: Text.Outline
            styleColor: 'transparent'

            //   layer.enabled: true
            //   layer.effect: DropShadow {
            //       color: 'black'
            //       radius: 8
            //       samples: 15
            //       spread: 0.2
            //   }
        }
    }

    Timer {
        running: true
        repeat: true
        interval: 100

        onTriggered: {
            var date = new Date()
            time.text = Qt.formatDateTime(date, 'hh:mm')
            interval = 60 * 1000 - (date.getSeconds() * 1000 + date.getMilliseconds())
        }
    }
}
