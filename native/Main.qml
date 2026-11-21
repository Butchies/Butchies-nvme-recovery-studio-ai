import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
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
            Button { text: "Zielordner wählen"; enabled: !controller.recoveryRunning; onClicked: folderDialog.open() }
            Button { text: "Ziel prüfen"; onClicked: controller.inspectDestination(destination.text) }
            TextField {
                id: destination
                Layout.fillWidth: true
                enabled: !controller.recoveryRunning
                placeholderText: "Zielordner für das Rettungsabbild (anderes Laufwerk als die defekte SSD)"
            }
        }
        FolderDialog {
            id: folderDialog
            title: "Zielordner für das Rettungsabbild"
            onAccepted: {
                var path = decodeURIComponent(selectedFolder.toString().replace(/^file:\/{3}/, ""))
                destination.text = path.replace(/\//g, "\\")
            }
        }
        Rectangle {
            id: recoveryPanel
            Layout.fillWidth: true
            visible: controller.recoveryRunning || !!controller.recoveryResult.state
            implicitHeight: recoveryColumn.implicitHeight + 24
            color: "#1d3a2f"; radius: 10
            ColumnLayout {
                id: recoveryColumn
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top; anchors.margins: 12
                spacing: 6
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: controller.recoveryRunning ? "Automatische Rettung läuft" : "Rettung: " + (controller.recoveryResult.rating || "")
                        font.bold: true; color: "white"; font.pixelSize: 17
                    }
                    Button {
                        text: "Anhalten"
                        visible: controller.recoveryRunning
                        onClicked: controller.cancelRecovery()
                    }
                }
                Label {
                    Layout.fillWidth: true
                    visible: controller.recoveryRunning
                    text: controller.recoveryProgress.phase || "Vorbereitung …"
                    color: "#cfeedd"; wrapMode: Text.WordWrap
                }
                RowLayout {
                    Layout.fillWidth: true
                    visible: controller.recoveryRunning
                    ProgressBar {
                        Layout.fillWidth: true
                        from: 0; to: 100
                        value: controller.recoveryProgress.percent || 0
                    }
                    Label {
                        text: (controller.recoveryProgress.percent || 0).toFixed(1) + " %"
                        color: "white"; font.bold: true
                    }
                }
                Label {
                    Layout.fillWidth: true
                    visible: controller.recoveryRunning && !!controller.recoveryProgress.rescued
                    text: "Gesichert: " + controller.recoveryProgress.rescued
                          + " (" + (controller.recoveryProgress.rescuedPercent || 0).toFixed(2) + " %)"
                          + " · Nicht lesbar: " + controller.recoveryProgress.bad
                          + " · Offen: " + controller.recoveryProgress.pending
                          + " · " + controller.recoveryProgress.rate
                          + " · Restzeit Durchgang: " + controller.recoveryProgress.eta
                    color: "#cfeedd"; wrapMode: Text.WordWrap
                }
                Label {
                    Layout.fillWidth: true
                    visible: !controller.recoveryRunning && !!controller.recoveryResult.summary
                    text: controller.recoveryResult.summary || ""
                    color: "#cfeedd"; wrapMode: Text.WordWrap
                }
                Repeater {
                    model: controller.recoveryRunning ? [] : (controller.recoveryResult.nextSteps || [])
                    Label { Layout.fillWidth: true; text: "→ " + modelData; color: "#b8e3cc"; wrapMode: Text.WordWrap }
                }
                Label {
                    Layout.fillWidth: true
                    visible: !controller.recoveryRunning && !!controller.recoveryResult.reportPath
                    text: "Bericht: " + (controller.recoveryResult.reportPath || "")
                    color: "#9fcfb6"; wrapMode: Text.WrapAnywhere
                }
                ListView {
                    id: logView
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    clip: true
                    model: controller.recoveryLog
                    delegate: Label { width: ListView.view.width; text: modelData; color: "#8fb8a3"; font.pixelSize: 12; elide: Text.ElideRight }
                    onCountChanged: positionViewAtEnd()
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: reportColumn.implicitHeight + 24
            color: "#203247"; radius: 10
            ColumnLayout {
                id: reportColumn
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top; anchors.margins: 12
                spacing: 5
                Label { text: "Automatische Diagnose"; font.bold: true; color: "white"; font.pixelSize: 17 }
                Label {
                    Layout.fillWidth: true
                    text: controller.aiReport.diagnosis || "Gerät auswählen und automatische Diagnose starten."
                    color: "#c6e2f2"; wrapMode: Text.WordWrap
                }
                Label {
                    Layout.fillWidth: true
                    visible: !!controller.aiReport.severity
                    text: "Bewertung: " + (controller.aiReport.severity || "") + " · Sicherheit: " + (controller.aiReport.confidence || "")
                    color: "#c6e2f2"; wrapMode: Text.WordWrap
                }
                Repeater {
                    model: controller.aiReport.actions || []
                    Label { Layout.fillWidth: true; text: "• " + modelData; color: "#b5d0e6"; wrapMode: Text.WordWrap }
                }
            }
        }
        RowLayout {
        Layout.fillWidth: true; Layout.fillHeight: true
        spacing: 16
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
                        width: ListView.view.width; height: 222
                        color: index%2 ? "#1c2939" : "#243346"
                        radius: 6
                        MouseArea {
                            anchors.fill: parent
                            onClicked: controller.selectDevice(index)
                        }
                        Column {
                            anchors.fill: parent; anchors.margins: 12
                            spacing: 5
                            Row {
                                spacing: 14
                                Label { text: modelData.name || "USB-Laufwerk"; color: "white"; font.bold: true }
                                Button {
                                    text: "Automatisch diagnostizieren"
                                    enabled: !controller.recoveryRunning
                                    onClicked: controller.autoDiagnose(index)
                                }
                                Button {
                                    text: "Automatisch retten"
                                    enabled: !controller.recoveryRunning
                                    onClicked: controller.startAutoRecovery(index, destination.text)
                                }
                            }
                            Label {
                                visible: !!modelData.aiDiagnosis
                                width: parent.width
                                text: "KI: " + (modelData.aiDiagnosis || "") + " [" + (modelData.aiSeverity || "") + "]"
                                color: modelData.aiSeverity === "Kritisch" ? "#ffb4a2" : "#b8f2c8"
                                elide: Text.ElideRight
                            }
                            Label {
                                text: "USB · VID " + modelData.bridgeVendorId + " · PID " + modelData.bridgeProductId
                                color: "#a5bed3"
                            }
                            Label { text: "Geräterevision: " + modelData.bridgeFirmware; color: "#a5bed3" }
                            Label {
                                text: "NVMe-Passthrough: " + modelData.nvmePassthrough + " · Nur lesend"
                                color: "#a5bed3"
                            }
                            Label {
                                text: modelData.nvmeIdentifyAvailable
                                      ? "Controller: " + modelData.nvmeControllerModel + " · Firmware: " + modelData.nvmeFirmware
                                      : "NVMe-Controllerdaten nicht verfügbar"
                                color: "#a5bed3"
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Label {
                                text: modelData.smartAvailable
                                      ? "SMART: " + modelData.temperatureCelsius + " °C · Reserve " + modelData.availableSparePercent + "% · Verschleiß " + modelData.percentageUsed + "% · Medienfehler " + modelData.mediaErrors
                                      : "SMART/Health über diese Verbindung nicht verfügbar"
                                color: "#a5bed3"
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Label {
                                text: modelData.errorLogAvailable
                                      ? "NVMe-Fehlerprotokoll: " + modelData.recentNvmeErrors.length + " Einträge (maximal 4 gelesen)"
                                      : "NVMe-Fehlerprotokoll über diese Verbindung nicht verfügbar"
                                color: "#a5bed3"
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            Layout.preferredWidth: 420; Layout.fillHeight: true
            color: "#1a1f36"; radius: 12
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 14
                spacing: 6
                Label { text: "KI-Live-Monitor"; font.bold: true; font.pixelSize: 18; color: "white" }
                Label {
                    Layout.fillWidth: true
                    text: "Wissensbasis: " + (controller.knowledge.cases || 0) + " Einträge · "
                          + (controller.knowledge.devices || 0) + " Geräte · "
                          + (controller.knowledge.bridges || 0) + " Bridges · "
                          + (controller.knowledge.controllers || 0) + " Controller"
                    color: "#a9b4e6"; font.pixelSize: 12; wrapMode: Text.WordWrap
                }
                ListView {
                    id: aiView
                    Layout.fillWidth: true; Layout.fillHeight: true
                    clip: true
                    spacing: 3
                    model: controller.aiMonitor
                    delegate: Label {
                        width: ListView.view.width
                        text: modelData
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                        color: modelData.indexOf("Warnung:") >= 0 ? "#ffb4a2"
                             : modelData.indexOf("Gelernt") >= 0 || modelData.indexOf("Erfahrung") >= 0 ? "#b8f2c8"
                             : modelData.indexOf("Entscheidung:") >= 0 ? "#ffe8a3" : "#cdd5ff"
                    }
                    onCountChanged: positionViewAtEnd()
                }
            }
        }
        }
        Label { text: controller.status; color: "#c6e2f2"; wrapMode: Text.WordWrap }
    }
}
