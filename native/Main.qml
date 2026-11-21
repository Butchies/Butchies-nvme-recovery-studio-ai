import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ApplicationWindow {
    id: window
    width: 1120; height: 720
    minimumWidth: 780; minimumHeight: 520
    visible: true
    title: "NVMe Recovery Studio AI"
    color: "#101722"
    required property QtObject controller
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16
        Label {
            text: "NVMe Recovery Studio AI"
            font.pixelSize: 28; font.bold: true
            color: "#eef5ff"
        }
        Label {
            text: "Windows 10/11 · Nur externe USB-Laufwerke · Interne SSDs gesperrt"
            color: "#a5bed3"
        }
        RowLayout {
            Button { text: "USB-Geräte erkennen"; onClicked: controller.refresh() }
            Button { text: "Ziel prüfen"; onClicked: controller.inspectDestination(destination.text) }
            TextField {
                id: destination
                Layout.fillWidth: true
                placeholderText: "Zielordner für spätere Recovery"
            }
        }
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#192333"; radius: 12
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16
                Label { text: "Externe Laufwerke"; font.bold: true; font.pixelSize: 18; color: "white" }
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: controller.devices
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width; height: 76
                        color: index%2 ? "#1c2939" : "#243346"
                        radius: 6
                        Column {
                            anchors.fill: parent; anchors.margins: 12
                            spacing: 5
                            Label { text: modelData.name || "USB-Laufwerk"; color: "white"; font.bold: true }
                            Label {
                                text: "USB · NVMe hinter Bridge nicht bestätigt · Nur lesend"
                                color: "#a5bed3"
                            }
                        }
                    }
                }
            }
        }
        Label { text: controller.status; color: "#c6e2f2"; wrapMode: Text.WordWrap }
    }
}
