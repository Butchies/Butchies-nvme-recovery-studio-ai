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
            text: controller.linuxEdition
                  ? "Linux-Boot-Edition · Direkter PCIe/NVMe-Zugriff · Boot-Stick und Systemlaufwerke geschützt"
                  : "Windows 10/11 · Nur externe USB-Laufwerke · Interne SSDs gesperrt"
            color: "#a5bed3"
        }
        RowLayout {
            Button { text: controller.linuxEdition ? "Geräte erkennen" : "USB-Geräte erkennen"; onClicked: controller.refresh() }
            Button { text: "Zielordner wählen"; enabled: !controller.recoveryRunning; onClicked: folderDialog.open() }
            Button { text: "Linux-Boot-Stick erstellen"; visible: !controller.linuxEdition; enabled: !controller.recoveryRunning; onClicked: stickDialog.open() }
            Button { text: "Ziel prüfen"; onClicked: controller.inspectDestination(destination.text) }
            CheckBox { id: longTestBox; text: "Langer Selbsttest" }
            CheckBox { id: fullSurfaceBox; visible: controller.linuxEdition; text: "Reparatur: ganze SSD prüfen" }
            CheckBox { id: writeTestBox; visible: controller.linuxEdition; text: "Cache-Schreibtest (überschreibt Daten!)" }
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
                var path = decodeURIComponent(selectedFolder.toString().replace(/^file:\/\//, ""))
                if (!controller.linuxEdition) path = path.replace(/^\//, "").replace(/\//g, "\\")
                destination.text = path
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
                Label {
                    visible: !!controller.aiReport.kiModell
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    text: controller.aiReport.kiModell
                          ? "KI-Modell: " + controller.aiReport.kiModell.text + " (" + Math.round(controller.aiReport.kiModell.wahrscheinlichkeit * 100) + " %)"
                            + (controller.aiReport.kiModell.gruende.length ? " – wegen: " + controller.aiReport.kiModell.gruende.join(", ") : "")
                          : ""
                    font.bold: true; color: "#ffe8a3"
                }
                RowLayout {
                    visible: !!controller.aiReport.kiModell
                    Label { text: "Tatsächliche Ursache bestätigen:"; color: "#c6e2f2" }
                    ComboBox { id: causeBox; model: controller.causeOptions(); textRole: "text"; valueRole: "id"; implicitWidth: 360 }
                    Button { text: "Bestätigen – KI lernt"; onClicked: controller.confirmCause(causeBox.currentValue) }
                }
                RowLayout {
                    Layout.fillWidth: true
                    visible: !!controller.cacheProgress.running
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: controller.cacheProgress.percent || 0 }
                    Label { text: (controller.cacheProgress.percent || 0) + " % · " + (controller.cacheProgress.phase || ""); color: "#c6e2f2" }
                }
                RowLayout {
                    Layout.fillWidth: true
                    visible: !!controller.repairProgress.phase
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: controller.repairProgress.percent || 0; visible: !!controller.repairProgress.running }
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; color: "#ffe8a3"
                            text: (controller.repairProgress.running ? (controller.repairProgress.percent || 0) + " % · " : "Reparatur: ") + (controller.repairProgress.phase || "") }
                }
                Repeater {
                    model: controller.repairProgress.result ? controller.repairProgress.result.schritte : []
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: (modelData.ok ? "✓ " : "✗ ") + modelData.schritt + ": " + modelData.antwort
                            color: modelData.ok ? "#b8f2c8" : "#ffb4a2" }
                }
                Label {
                    visible: !!controller.aiReport.platine
                    text: "Platine (schematisch): " + (controller.aiReport.platine ? controller.aiReport.platine.layoutName : "")
                    font.bold: true; color: "#ffffff"
                }
                RowLayout {
                    visible: !!controller.aiReport.platine
                    Label { text: "Bauform:"; color: "#c6e2f2" }
                    ComboBox {
                        id: layoutBox; implicitWidth: 420
                        model: controller.boardLayouts(); textRole: "name"; valueRole: "id"
                        currentIndex: controller.aiReport.platine ? indexOfValue(controller.aiReport.platine.layoutId) : -1
                        onActivated: controller.setBoardLayout(currentValue)
                    }
                }
                Row {
                    id: boardRow
                    visible: !!controller.aiReport.platine
                    spacing: 24
                    property var board: controller.aiReport.platine || ({})
                    property real s: 9
                    Repeater {
                        model: boardRow.board.beidseitig ? ["oben", "unten"] : ["oben"]
                        Column {
                            spacing: 4
                            property string side: modelData
                            Label { text: side === "oben" ? "Oberseite" : "Unterseite"; color: "#a5bed3"; font.pixelSize: 12 }
                            Rectangle {
                                width: (boardRow.board.laenge || 80) * boardRow.s; height: (boardRow.board.breite || 22) * boardRow.s
                                radius: 6; color: "#14532d"; border.color: "#2f7d4f"
                                Repeater {
                                    model: (boardRow.board.teile || []).filter(function(p){ return p.typ === "stecker" || p.seite === side })
                                    Rectangle {
                                        x: modelData.x * boardRow.s; y: modelData.y * boardRow.s
                                        width: modelData.w * boardRow.s; height: modelData.h * boardRow.s; radius: 3
                                        color: modelData.typ === "stecker" ? "#b8902f" : (modelData.typ === "nand" ? "#1f2937" : (modelData.typ === "dram" ? "#374151" : "#111827"))
                                        border.width: modelData.defekt ? 4 : 1
                                        border.color: modelData.defekt ? (modelData.schwere === "kritisch" ? "#ff3b30" : "#ff9f0a") : "#6b7280"
                                        Label {
                                            anchors.centerIn: parent; width: parent.width - 4; horizontalAlignment: Text.AlignHCenter
                                            wrapMode: Text.WordWrap; font.pixelSize: 11; color: "#f3f4f6"
                                            text: modelData.name + (modelData.defekt ? "\n⚠" : "")
                                        }
                                        HoverHandler { id: partHover }
                                        ToolTip.visible: partHover.hovered && modelData.defekt
                                        ToolTip.text: modelData.gruende.join("\n")
                                    }
                                }
                            }
                        }
                    }
                }
                Repeater {
                    model: controller.aiReport.platine ? controller.aiReport.platine.befunde : []
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap
                            text: "⚠ " + ({"controller":"Controller","dram":"Cache/DRAM","nand":"Flash-Speicher","pmic":"Spannungsversorgung","stecker":"Stecker/Verbindung"}[modelData.typ]) + ": " + modelData.grund
                            color: modelData.schwere === "kritisch" ? "#ffb4a2" : "#ffd59e" }
                }
                Label {
                    visible: !!controller.aiReport.controllerMeldungen
                    text: "Was der Controller meldet: " + (controller.aiReport.controllerMeldungen ? controller.aiReport.controllerMeldungen.zusammenfassung : "")
                    font.bold: true; color: "#ffffff"; Layout.fillWidth: true; wrapMode: Text.WordWrap
                }
                Repeater {
                    model: controller.aiReport.controllerMeldungen ? controller.aiReport.controllerMeldungen.eintraege : []
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 0
                        Label { Layout.fillWidth: true; wrapMode: Text.WrapAnywhere; text: modelData.roh; color: "#8fa5b8"; font.family: "monospace"; font.pixelSize: 12 }
                        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap
                                text: "→ " + modelData.uebersetzung + (modelData.detail ? " (" + modelData.detail + ")" : "") + (modelData.bedeutung ? ": " + modelData.bedeutung : "")
                                color: modelData.schwere === "kritisch" ? "#ffb4a2" : (modelData.schwere === "hoch" ? "#ffd59e" : "#c6e2f2") }
                    }
                }
                Label {
                    visible: !!controller.aiReport.cacheTest
                    Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true
                    text: controller.aiReport.cacheTest ? "Cache: " + controller.aiReport.cacheTest.urteil : ""
                    color: controller.aiReport.cacheTest && controller.aiReport.cacheTest.auffaellig ? "#ffb4a2" : "#b8f2c8"
                }
                Repeater {
                    model: controller.aiReport.cacheTest ? controller.aiReport.cacheTest.details : []
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "  " + modelData; color: "#b5d0e6" }
                }
                Label {
                    visible: (controller.aiReport.repairPaths || []).length > 0
                    text: "Reparaturwege (Vorlage: " + ((controller.aiReport.catalog && controller.aiReport.catalog.family) ? controller.aiReport.catalog.family.hersteller : "allgemein") + ")"
                    font.bold: true; color: "white"
                }
                Repeater {
                    model: controller.aiReport.repairPaths || []
                    Label {
                        Layout.fillWidth: true
                        text: "◆ " + modelData.typ + (modelData.datenverlust === "ja" ? " (Datenverlust!)" : "") + " – " + modelData.status
                        color: modelData.datenverlust === "ja" ? "#ffb4a2" : "#d8e8ff"
                        wrapMode: Text.WordWrap
                    }
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
                Label { text: controller.linuxEdition ? "NVMe-Geräte am PCIe-Bus" : "Externe Laufwerke"; font.bold: true; font.pixelSize: 18; color: "white" }
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: controller.devices
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width; height: 246
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
                                Button {
                                    text: "Cache testen"
                                    enabled: !controller.recoveryRunning
                                    onClicked: controller.runCacheTest(index, longTestBox.checked, writeTestBox.checked)
                                }
                                Button {
                                    text: "Reparieren"
                                    visible: controller.linuxEdition
                                    enabled: !controller.recoveryRunning
                                    onClicked: controller.startRepair(index, fullSurfaceBox.checked, "")
                                }
                                Button {
                                    text: "Firmware …"
                                    enabled: !controller.recoveryRunning
                                    onClicked: { firmwareFileDialog.deviceIndex = index; firmwareFileDialog.open() }
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
                                visible: !!modelData.protected || !!modelData.romModeSuspect
                                text: modelData.protected ? "GESCHÜTZT: " + modelData.protectedReason : "Notmodus-Verdacht: PCIe " + modelData.pciVendorId + ":" + modelData.pciDeviceId + " ohne NVMe-Funktion"
                                color: "#ffb4a2"; font.bold: true
                            }
                            Label {
                                text: modelData.connection === "PCIe"
                                      ? "PCIe " + modelData.pciAddress + " · " + modelData.pciVendorId + ":" + modelData.pciDeviceId + " · Treiber " + modelData.driver + " · " + (modelData.devicePath || "kein Namespace")
                                      : "USB · VID " + modelData.bridgeVendorId + " · PID " + modelData.bridgeProductId
                                color: "#a5bed3"
                            }
                            Label { visible: modelData.connection !== "PCIe"; text: "Geräterevision: " + modelData.bridgeFirmware; color: "#a5bed3" }
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
                          + (controller.knowledge.controllers || 0) + " Controller · "
                          + (controller.knowledge.proposals || 0) + " Vorlagen-Vorschläge\nVorlagen: "
                          + (controller.catalog.vendors || 0) + " Hersteller · " + (controller.catalog.devices || 0) + " Controller/Produkte · "
                          + (controller.catalog.firmwareIssues || 0) + " Firmwarefehler"
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
                Label { text: "Werkzeuge der KI"; font.bold: true; font.pixelSize: 16; color: "white" }
                RowLayout {
                    Layout.fillWidth: true
                    visible: !!controller.toolDownload.name
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: controller.toolDownload.percent || 0 }
                    Label {
                        text: (controller.toolDownload.percent || 0) + " % · " + (controller.toolDownload.phase || "")
                        color: "#cdd5ff"; font.pixelSize: 12
                    }
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 230
                    clip: true
                    spacing: 4
                    model: controller.tools
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: toolRow.implicitHeight + 10
                        radius: 6
                        color: modelData.installiert ? "#1f3a2c" : "#2a2440"
                        RowLayout {
                            id: toolRow
                            anchors.left: parent.left; anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 6
                            Label {
                                Layout.fillWidth: true
                                text: (modelData.installiert ? "✓ " : "○ ") + modelData.name
                                      + (modelData.nutzerBereitgestellt ? " (selbst bereitstellen)" : "")
                                color: modelData.installiert ? "#b8f2c8" : "#e0dcff"
                                font.pixelSize: 12; elide: Text.ElideRight
                                ToolTip.visible: toolHover.hovered
                                ToolTip.text: modelData.zweck + "\n" + (modelData.installiert ? modelData.pfad : (modelData.offizielleSeite || "Keine öffentliche Quelle"))
                                HoverHandler { id: toolHover }
                            }
                            Button {
                                visible: !modelData.installiert && !modelData.nutzerBereitgestellt
                                text: modelData.automatisch ? "Installieren" : "Seite öffnen"
                                enabled: !controller.toolDownload.running
                                onClicked: controller.installTool(modelData.id)
                            }
                            Button {
                                visible: !modelData.installiert && (modelData.nutzerBereitgestellt || modelData.plattform.indexOf("windows") >= 0)
                                text: "Registrieren"
                                onClicked: { toolFileDialog.toolId = modelData.id; toolFileDialog.open() }
                            }
                        }
                    }
                }
            }
        }
        }
        FileDialog {
            id: firmwareFileDialog
            property int deviceIndex: -1
            title: "Offizielle Firmwaredatei des Herstellers auswählen"
            nameFilters: ["Firmware (*.bin *.img *.fw *.enc *.fluf *.rom)", "Alle Dateien (*)"]
            onAccepted: {
                firmwareDialog.fileUrl = selectedFile.toString()
                firmwareDialog.deviceIndex = deviceIndex
                controller.prepareFirmware(deviceIndex, firmwareDialog.fileUrl, noBackupBox.checked)
                firmwareDialog.open()
            }
        }
        Dialog {
            id: firmwareDialog
            property string fileUrl: ""
            property int deviceIndex: -1
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: Math.min(760, window.width - 60)
            modal: true
            title: "Firmware-Manager"
            closePolicy: controller.firmwareProgress.running ? Popup.NoAutoClose : Popup.CloseOnEscape
            ColumnLayout {
                anchors.fill: parent
                spacing: 6
                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: controller.firmwarePlan.summary || "Prüfe …"; font.bold: true }
                Repeater {
                    model: controller.firmwarePlan.checks || []
                    Label {
                        Layout.fillWidth: true; wrapMode: Text.WordWrap
                        text: (modelData.ok ? "✓ " : (modelData.pflicht ? "✗ " : "! ")) + modelData.name + ": " + modelData.detail
                        color: modelData.ok ? "#2e7d32" : (modelData.pflicht ? "#c62828" : "#ef6c00")
                    }
                }
                Label { text: "Risiken"; font.bold: true }
                Repeater {
                    model: controller.firmwarePlan.risks || []
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "• " + modelData }
                }
                Label { Layout.fillWidth: true; wrapMode: Text.WrapAnywhere; text: "SHA-256: " + (controller.firmwarePlan.sha256 || ""); font.pixelSize: 11 }
                CheckBox {
                    id: noBackupBox
                    text: "Ich bestätige: Die Daten auf dieser SSD werden nicht benötigt (keine Sicherung nötig)"
                    onToggled: controller.prepareFirmware(firmwareDialog.deviceIndex, firmwareDialog.fileUrl, checked)
                }
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: approvalField
                        visible: !!controller.firmwarePlan.approvalCode
                        Layout.fillWidth: true
                        enabled: !!controller.firmwarePlan.allowed && !controller.firmwareProgress.running
                        placeholderText: "Bestätigungscode: letzte 4 Zeichen der Seriennummer"
                    }
                    Button {
                        text: "Jetzt flashen"
                        enabled: !!controller.firmwarePlan.allowed && !controller.firmwareProgress.running && (!!controller.firmwarePlan.expert || approvalField.text.length === 4)
                        onClicked: controller.flashFirmware(approvalField.text)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    visible: !!controller.firmwareProgress.phase
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: controller.firmwareProgress.percent || 0 }
                    Label { text: (controller.firmwareProgress.percent || 0) + " % · " + (controller.firmwareProgress.phase || "") }
                }
                Label {
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    visible: !!controller.firmwareProgress.result
                    text: controller.firmwareProgress.result ? controller.firmwareProgress.result.message : ""
                    font.bold: true
                }
            }
            standardButtons: controller.firmwareProgress.running ? Dialog.NoButton : Dialog.Close
        }
        FileDialog {
            id: stickImageDialog
            title: "Linux-Boot-Abbild auswählen"
            nameFilters: ["Boot-Abbild (*.iso *.img)"]
            onAccepted: stickImage.text = selectedFile.toString()
        }
        Dialog {
            id: stickDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: Math.min(700, window.width - 60)
            modal: true
            title: "Linux-Boot-Stick erstellen"
            closePolicy: controller.stickProgress.running ? Popup.NoAutoClose : Popup.CloseOnEscape
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                Label {
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    text: "Der USB-Stick wird vollständig überschrieben. Danach startet der Rechner vom Stick direkt in NVMe Recovery Studio AI (UEFI), mit direktem Zugriff auf interne M.2-SSDs. Beim ersten Start wird auf dem Stick ein dauerhafter Speicher für Fallakten und KI-Wissen angelegt."
                }
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: stickImage; Layout.fillWidth: true
                        placeholderText: "Boot-Abbild (.iso)"
                        Connections {
                            target: controller
                            function onStickChanged() { if (controller.stickProgress.image) stickImage.text = "file:///" + controller.stickProgress.image }
                        }
                    }
                    Button { text: "Datei wählen"; onClicked: stickImageDialog.open() }
                    Button { text: "Neueste laden"; enabled: !controller.stickProgress.running; onClicked: controller.downloadBootImage() }
                }
                ComboBox {
                    id: stickTarget
                    Layout.fillWidth: true
                    model: controller.devices
                    textRole: "name"
                    displayText: currentIndex >= 0 ? (controller.devices[currentIndex].name + " · " + controller.devices[currentIndex].devicePath) : "USB-Stick wählen"
                }
                TextField {
                    id: stickConfirm
                    Layout.fillWidth: true
                    placeholderText: "Zur Bestätigung STICK eingeben"
                }
                Button {
                    text: "Stick jetzt beschreiben"
                    enabled: !controller.stickProgress.running && stickTarget.currentIndex >= 0 && stickImage.text.length > 0 && stickConfirm.text.toUpperCase() === "STICK"
                    onClicked: controller.createBootStick(stickTarget.currentIndex, stickImage.text, stickConfirm.text)
                }
                RowLayout {
                    Layout.fillWidth: true
                    visible: !!controller.stickProgress.phase
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: controller.stickProgress.percent || 0 }
                    Label { text: (controller.stickProgress.percent || 0) + " %" }
                }
                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: controller.stickProgress.phase || "" }
            }
            standardButtons: controller.stickProgress.running ? Dialog.NoButton : Dialog.Close
        }
        FileDialog {
            id: toolFileDialog
            property string toolId: ""
            title: "Programmdatei des Werkzeugs auswählen"
            nameFilters: ["Programme (*.exe)", "Alle Dateien (*)"]
            onAccepted: controller.registerTool(toolId, selectedFile.toString())
        }
        Label { text: controller.status; color: "#c6e2f2"; wrapMode: Text.WordWrap }
    }
}
