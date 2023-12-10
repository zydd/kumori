import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import kumori 0.1


AppbarWindow {
    id: root
    color: 'transparent'
    thickness: 36

    Component.onCompleted: {
        TrayService.init()
        WmService.init()
        root.show()
    }

    Rectangle {
        anchors.fill: parent
        color: '#ddd'
        opacity: 0.1
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ToolButton {
            palette.button: Qt.rgba(0, 0, 0, 0.4)
            palette.mid: Qt.rgba(0.5, 0.5, 0.5, 0.4)
            palette.buttonText: 'white'
            text: 'スタート'
            leftPadding: 15
            rightPadding: 15

            onClicked: Kumori.startMenu()
        }

        ListView {
            id: taskView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: contentWidth
            orientation: ListView.Horizontal
            model: WmService
            clip: true
//            boundsBehavior: Flickable.StopAtBounds

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
            Layout.fillHeight: true
            Layout.preferredWidth: contentWidth
            orientation: ListView.Horizontal
            layoutDirection: Qt.RightToLeft
            model: TrayService.proxy(TrayService.VisibleIcons)
            clip: true
            Layout.leftMargin: 5

            delegate: ToolButton {
                width: root.thickness
                height: root.thickness
//                hoverEnabled: true
                palette.button: 'transparent'

//                ToolTip.delay: 1000
//                ToolTip.timeout: 5000
//                ToolTip.visible: hovered && modelData.tooltip
//                ToolTip.text: modelData.tooltip

                TrayIcon {
                    anchors.centerIn: parent
                    liveIcon: model.trayIcon
                    width: 24
                    height: 24
                }
            }
        }

        ToolButton {
            id: actionCenter
            Layout.fillHeight: true
            Layout.leftMargin: 5

            palette.button: Qt.rgba(0, 0, 0, 0.4)
            palette.mid: Qt.rgba(0.5, 0.5, 0.5, 0.4)

            onClicked: Kumori.actionCenter()

            contentItem: Row {
                spacing: 5
                Repeater {
                    model: TrayService.proxy(TrayService.ActionCenterIcons)

                    delegate: TrayIcon {
                        acceptedButtons: Qt.RightButton
                        liveIcon: model.trayIcon
                        width: 24
                        height: 24
                    }
                }
            }
        }

        Label {
            id: time
            color: 'white'
            Layout.preferredWidth: 80
//            Layout.fillHeight: true
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 6
            font.pixelSize: root.thickness * 0.65
            font.weight: Font.Light
            font.family: 'Segoe UI'
            horizontalAlignment: Qt.AlignCenter
//            verticalAlignment: Qt.AlignCenter

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
