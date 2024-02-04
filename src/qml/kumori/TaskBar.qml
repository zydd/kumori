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
        Kumori.blurWindowBackground(root)
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
            id: startBtn
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 6

            text: 'スタート'

            font.pixelSize: root.thickness * 0.5
            font.weight: Font.Light

            palette.button: Qt.rgba(0, 0, 0, 0.4)
            palette.mid: Qt.rgba(0.5, 0.5, 0.5, 0.4)
            palette.buttonText: 'white'

            background: Rectangle {
                color: (!WmService.startMenu || !WmService.startMenu.active) ?
                           parent.palette.button : parent.palette.mid
            }

            onClicked:
                if (!WmService.startMenu || !WmService.startMenu.active)
                    Kumori.startMenu()
                else WmService.startMenu.active = false
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
                height: root.thickness
                padding: 5

                font.pixelSize: root.thickness * 0.4

                palette.button: Qt.rgba(0, 0, 0, 0.4)
                palette.mid: Qt.rgba(0.5, 0.5, 0.5, 0.4)
                palette.buttonText: 'white'

                contentItem: RowLayout {
                    LiveIcon {
                        id: winIcon
                        liveIcon: model.nativeWindow.icon
                        width: 24
                        height: 24
                    }

                    Label {
                        Layout.fillWidth: true
                        text: model.nativeWindow.title
                        color: palette.buttonText
                        elide: Qt.ElideRight
                    }
                }

                background: Rectangle {
                    color: model.nativeWindow.active ? '#fff' : 'transparent'
                    opacity: 0.1
                }

                Connections {
                    target: model.nativeWindow

                    function onActiveChanged() {
                        if (model.nativeWindow.active)
                            taskView.positionViewAtIndex(model.row, ListView.Contain)
                    }
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
            visible: actionCenterIcons.count

            onClicked: Kumori.actionCenter()

            contentItem: Row {
                spacing: 5
                Repeater {
                    id: actionCenterIcons
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

        ToolButton {
            id: clock
            Layout.preferredWidth: 80
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 6
            font.pixelSize: root.thickness * 0.65
            font.weight: Font.Light

            palette.button: Qt.rgba(0, 0, 0, 0.4)
            palette.mid: Qt.rgba(0.5, 0.5, 0.5, 0.4)
            palette.buttonText: 'white'

            onClicked: Kumori.notificationArea()
        }

        Timer {
            running: true
            repeat: true
            interval: 100

            onTriggered: {
                var date = new Date()
                clock.text = Qt.formatDateTime(date, 'hh:mm')
                interval = 60 * 1000 - (date.getSeconds() * 1000 + date.getMilliseconds())
            }
        }
    }
}
