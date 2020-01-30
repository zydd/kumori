import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

Item {
    implicitWidth: txt.implicitWidth
    height: 250

    Text {
        id: txt

        font.pixelSize: parent.height
        font.family: 'Dejima,衡山毛筆フォント行書,YOzFontK,MS Mincho'
        font.weight: Font.Black

        Timer {
            running: true
            interval: 1000
            repeat: true
            triggeredOnStart: true

            onTriggered: {
                var date = new Date()

                parent.text = date.toLocaleString(Qt.locale('ja'), 'ddd')

                interval = 24 * 3600 * 1000 -
                        ((date.getHours() * 60
                          + date.getMinutes()) * 60
                         + date.getSeconds()) * 1000
            }
        }
    }
}
