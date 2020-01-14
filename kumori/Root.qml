import QtQuick 2.12
import kumori 0.1

Item {
    anchors.fill: parent

    Text {
        anchors.centerIn: parent
        text: 'kumori'
        color: 'white'
        style: Text.Outline
        styleColor: 'black'
        font.pixelSize: parent.height/6

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.openUrlExternally(Qt.resolvedUrl('.'))
        }
    }
}
