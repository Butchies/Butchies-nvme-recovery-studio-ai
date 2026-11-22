import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

// NVMe Recovery Studio AI – helle Oberfläche (Entwurf vom 22.11.2026)
// Aufbau: Kopfzeile · Navigation links · Arbeitsfläche · KI-Assistent rechts
ApplicationWindow {
    id: window
    width: 1480; height: 920
    minimumWidth: 1100; minimumHeight: 680
    visible: true
    title: "NVMe Recovery Studio AI"
    color: theme.bg
    required property QtObject controller

    // ---------------------------------------------------------------- Theme
    QtObject {
        id: theme
        readonly property color bg: "#f4f6fa"
        readonly property color card: "#ffffff"
        readonly property color line: "#e5e9f0"
        readonly property color text: "#1f2937"
        readonly property color strong: "#0f172a"
        readonly property color muted: "#667085"
        readonly property color accent: "#2563eb"
        readonly property color accentSoft: "#e8f0fe"
        readonly property color accentText: "#1d4ed8"
        readonly property color good: "#12b76a"
        readonly property color goodSoft: "#e7f6ec"
        readonly property color goodText: "#166534"
        readonly property color warn: "#f79009"
        readonly property color warnSoft: "#fff4e0"
        readonly property color warnText: "#9a5b00"
        readonly property color bad: "#d92d20"
        readonly property color badSoft: "#fdecec"
        readonly property color badText: "#b42318"
        readonly property string mono: Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono"
    }
    font.family: Qt.platform.os === "windows" ? "Segoe UI" : "DejaVu Sans"
    font.pixelSize: 14
    palette.window: theme.bg
    palette.base: "#ffffff"
    palette.button: "#ffffff"
    palette.buttonText: "#344054"
    palette.text: theme.text
    palette.windowText: theme.text
    palette.highlight: theme.accent

    // ------------------------------------------------------- Shared state
    property int page: 0
    readonly property var report: controller.aiReport || ({})
    readonly property var dev: report.geraet || ({})
    readonly property int selected: report.index !== undefined ? report.index : -1
    readonly property bool hasDevice: selected >= 0

    function severityLevel(s) {
        if (s === "Kritisch" || s === "Gesperrt") return "bad"
        if (s === "Auffällig" || s === "Diagnose eingeschränkt") return "warn"
        if (s) return "good"
        return "grey"
    }
    function score(s) {
        return ({ "Keine akute SMART-Warnung": 92, "Auffällig": 65, "Diagnose eingeschränkt": 50, "Kritisch": 18, "Gesperrt": 0 })[s] ?? 0
    }
    function levelColor(l) { return l === "bad" ? theme.bad : l === "warn" ? theme.warn : l === "good" ? theme.good : "#98a2b3" }
    function levelSoft(l) { return l === "bad" ? theme.badSoft : l === "warn" ? theme.warnSoft : l === "good" ? theme.goodSoft : "#eef1f5" }
    function levelText(l) { return l === "bad" ? theme.badText : l === "warn" ? theme.warnText : l === "good" ? theme.goodText : "#475467" }
    function icon(name, color) { return "icons/" + name + "-" + (color || "grey") + ".svg" }
    function fileUrlToPath(u) {
        var p = decodeURIComponent(u.toString().replace(/^file:\/\//, ""))
        if (!controller.linuxEdition) p = p.replace(/^\//, "").replace(/\//g, "\\")
        return p
    }

    // ------------------------------------------------- Reusable components
    component Card: Rectangle {
        default property alias content: inner.data
        property alias spacing: inner.spacing
        color: theme.card; radius: 12; border.color: theme.line
        implicitHeight: inner.implicitHeight + 36
        ColumnLayout { id: inner; anchors.fill: parent; anchors.margins: 18; spacing: 10 }
    }
    component Heading: RowLayout {
        property string text
        property string iconName: ""
        spacing: 8
        Image { visible: iconName !== ""; source: iconName ? window.icon(iconName) : ""; sourceSize: Qt.size(18, 18) }
        Label { text: parent.text; font.pixelSize: 15; font.bold: true; color: theme.strong }
    }
    component Pill: Rectangle {
        property string text
        property string level: "grey"
        radius: 999; color: window.levelSoft(level)
        implicitWidth: pillLabel.implicitWidth + 20; implicitHeight: 24
        Label { id: pillLabel; anchors.centerIn: parent; text: parent.text; font.pixelSize: 12; font.bold: true; color: window.levelText(parent.level) }
    }
    component ActionButton: Button {
        id: ab
        property string iconName: ""
        property bool primary: false
        property bool danger: false
        implicitHeight: 40
        leftPadding: 16; rightPadding: 16
        contentItem: RowLayout {
            spacing: 8
            Image { visible: ab.iconName !== ""; source: ab.iconName ? window.icon(ab.iconName, ab.primary ? "white" : (ab.danger ? "red" : "grey")) : ""; sourceSize: Qt.size(16, 16) }
            Label { text: ab.text; font.bold: true; color: ab.primary ? "#ffffff" : (ab.danger ? theme.badText : "#344054"); opacity: ab.enabled ? 1 : 0.5 }
        }
        background: Rectangle {
            radius: 8
            color: ab.primary ? (ab.down ? "#1e40af" : theme.accent) : (ab.down ? "#f2f4f7" : "#ffffff")
            border.color: ab.primary ? theme.accent : (ab.danger ? "#f4b4ae" : "#d0d5dd")
            opacity: ab.enabled ? 1 : 0.6
        }
    }
    component Progress: Rectangle {
        property real value: 0
        property color barColor: theme.accent
        implicitHeight: 10; radius: 5; color: "#eef1f5"
        Rectangle { width: parent.width * Math.max(0, Math.min(1, parent.value / 100)); height: parent.height; radius: 5; color: parent.barColor }
    }
    component Tile: Rectangle {
        property string label
        property string value
        property string note
        property string level: "grey"
        Layout.fillWidth: true
        implicitHeight: 96; radius: 12; color: theme.card; border.color: theme.line
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 14; spacing: 4
            Label { text: label; color: theme.muted }
            Label { text: value; font.pixelSize: 24; font.bold: true; color: theme.strong }
            Pill { text: note; level: parent.parent.level; visible: note !== "" }
        }
    }

    // ------------------------------------------------------------- Dialogs
    FolderDialog {
        id: folderDialog
        title: "Zielordner für Rettungsabbild"
        onAccepted: destination.text = window.fileUrlToPath(selectedFolder)
    }
    FileDialog {
        id: firmwareFileDialog
        title: "Offizielle Firmwaredatei auswählen"
        nameFilters: ["Firmware (*.bin *.img *.fw *.enc *.fluf *.rom)", "Alle Dateien (*)"]
        onAccepted: { firmwareDialog.fileUrl = selectedFile.toString(); controller.prepareFirmware(window.selected, firmwareDialog.fileUrl, noBackupBox.checked); firmwareDialog.open() }
    }
    FileDialog {
        id: toolFileDialog
        property string toolId: ""
        title: "Programmdatei des Werkzeugs auswählen"
        nameFilters: ["Programme (*.exe)", "Alle Dateien (*)"]
        onAccepted: controller.registerTool(toolId, selectedFile.toString())
    }
    FileDialog {
        id: stickImageDialog
        title: "Linux-Boot-Abbild auswählen"
        nameFilters: ["Boot-Abbild (*.iso *.img)"]
        onAccepted: stickImage.text = selectedFile.toString()
    }
    FolderDialog {
        id: archiveImportDialog
        title: "Ordner oder Stick mit Firmware wählen"
        onAccepted: controller.importArchive(selectedFolder.toString())
    }
    FolderDialog {
        id: archiveExportDialog
        title: "Ziel für das Offline-Paket wählen (z. B. USB-Stick)"
        onAccepted: controller.exportArchive(selectedFolder.toString())
    }
    Dialog {
        id: firmwareDialog
        property string fileUrl: ""
        parent: Overlay.overlay; anchors.centerIn: parent
        width: Math.min(760, window.width - 80); modal: true
        title: "Firmware-Manager"
        closePolicy: controller.firmwareProgress.running ? Popup.NoAutoClose : Popup.CloseOnEscape
        standardButtons: controller.firmwareProgress.running ? Dialog.NoButton : Dialog.Close
        ColumnLayout {
            anchors.fill: parent; spacing: 8
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; text: controller.firmwarePlan.summary || "Prüfe …" }
            Repeater {
                model: controller.firmwarePlan.checks || []
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Image { source: window.icon(modelData.ok ? "check" : (modelData.pflicht ? "x" : "warn"), modelData.ok ? "green" : (modelData.pflicht ? "red" : "amber")); sourceSize: Qt.size(16, 16) }
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "<b>" + modelData.name + ":</b> " + modelData.detail; textFormat: Text.StyledText }
                }
            }
            Label { text: "Risiken"; font.bold: true; Layout.topMargin: 6 }
            Repeater { model: controller.firmwarePlan.risks || []; Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "• " + modelData; color: theme.muted } }
            CheckBox { id: noBackupBox; text: "Daten auf dieser SSD werden nicht benötigt"; onToggled: controller.prepareFirmware(window.selected, firmwareDialog.fileUrl, checked) }
            RowLayout {
                Layout.fillWidth: true
                TextField { id: approvalField; Layout.fillWidth: true; visible: !!controller.firmwarePlan.approvalCode; placeholderText: "Bestätigungscode: letzte 4 Zeichen der Seriennummer" }
                ActionButton {
                    text: "Jetzt flashen"; danger: true; iconName: "chip"
                    enabled: !!controller.firmwarePlan.allowed && !controller.firmwareProgress.running && (!!controller.firmwarePlan.expert || approvalField.text.length === 4)
                    onClicked: controller.flashFirmware(approvalField.text)
                }
            }
            Progress { Layout.fillWidth: true; visible: !!controller.firmwareProgress.phase; value: controller.firmwareProgress.percent || 0 }
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: controller.firmwareProgress.result ? controller.firmwareProgress.result.message : (controller.firmwareProgress.phase || "") }
        }
    }

    // ------------------------------------------------------------ Top bar
    header: Rectangle {
        height: 64; color: "#ffffff"
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: theme.line }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 24; anchors.rightMargin: 24; spacing: 16
            Rectangle {
                width: 34; height: 34; radius: 9; color: theme.accent
                Image { anchors.centerIn: parent; source: window.icon("chip", "white"); sourceSize: Qt.size(20, 20) }
            }
            Label { text: "NVMe Recovery Studio AI"; font.pixelSize: 17; font.bold: true; color: theme.strong }
            Rectangle {
                radius: 999; color: theme.accentSoft; implicitHeight: 24; implicitWidth: edLabel.implicitWidth + 20
                Label { id: edLabel; anchors.centerIn: parent; font.pixelSize: 12; font.bold: true; color: theme.accentText
                        text: controller.linuxEdition ? "Linux-Boot-Stick · direkter PCIe-Zugriff" : "Windows · nur externe USB-SSDs" }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                radius: 999; color: theme.goodSoft; implicitHeight: 24; implicitWidth: kiLabel.implicitWidth + 36
                RowLayout {
                    anchors.centerIn: parent; spacing: 6
                    Image { source: window.icon("spark", "green"); sourceSize: Qt.size(13, 13) }
                    Label { id: kiLabel; font.pixelSize: 12; font.bold: true; color: theme.goodText
                            text: "KI aktiv · " + (controller.knowledge.cases || 0) + " Einträge" }
                }
            }
            Rectangle {
                implicitWidth: 340; implicitHeight: 36; radius: 8; border.color: "#d6dce5"; color: "#ffffff"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 4; spacing: 6
                    Image { source: window.icon("folder"); sourceSize: Qt.size(16, 16) }
                    TextField {
                        id: destination; Layout.fillWidth: true; background: null
                        enabled: !controller.recoveryRunning
                        placeholderText: "Zielordner für Rettungen (anderes Laufwerk)"
                    }
                    ToolButton { text: "…"; onClicked: folderDialog.open(); ToolTip.visible: hovered; ToolTip.text: "Zielordner wählen" }
                }
            }
        }
    }

    // ------------------------------------------------------------ Layout
    RowLayout {
        anchors.fill: parent; spacing: 0

        // --- Navigation
        Rectangle {
            Layout.preferredWidth: 220; Layout.fillHeight: true; color: "#ffffff"
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: theme.line }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 2
                Label { text: "ARBEITSBEREICH"; font.pixelSize: 11; font.letterSpacing: 0.8; color: theme.muted; Layout.margins: 12; Layout.bottomMargin: 6 }
                Repeater {
                    model: [
                        { name: "Übersicht", icon: "home", page: 0, show: true },
                        { name: "Rettung", icon: "save", page: 1, show: true },
                        { name: "Reparatur", icon: "tool", page: 2, show: controller.linuxEdition },
                        { name: "Firmware", icon: "chip", page: 3, show: true },
                        { name: "Cache-Test", icon: "cache", page: 4, show: true },
                        { name: "Werkzeuge", icon: "box", page: 5, show: true },
                        { name: "Linux-Boot-Stick", icon: "usb", page: 6, show: !controller.linuxEdition },
                        { name: "Verlauf", icon: "clock", page: 7, show: true }
                    ]
                    delegate: ItemDelegate {
                        required property var modelData
                        visible: modelData.show
                        Layout.fillWidth: true; implicitHeight: 42
                        background: Rectangle { radius: 8; color: window.page === modelData.page ? theme.accentSoft : (parent.hovered ? "#f5f7fa" : "transparent") }
                        contentItem: RowLayout {
                            spacing: 10
                            Image { source: window.icon(modelData.icon, window.page === modelData.page ? "blue" : "grey"); sourceSize: Qt.size(18, 18) }
                            Label { text: modelData.name; font.bold: window.page === modelData.page; color: window.page === modelData.page ? theme.accentText : "#475467" }
                        }
                        onClicked: window.page = modelData.page
                    }
                }
                Item { Layout.fillHeight: true }
                ActionButton { Layout.fillWidth: true; text: controller.linuxEdition ? "Geräte erkennen" : "USB-Geräte erkennen"; iconName: "refresh"; enabled: !controller.recoveryRunning; onClicked: controller.refresh() }
            }
        }

        // --- Work area
        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: availableWidth
            ColumnLayout {
                width: parent.width
                spacing: 18
                Item { implicitHeight: 4 }

                // Device cards (all pages)
                GridLayout {
                    Layout.fillWidth: true; Layout.leftMargin: 24; Layout.rightMargin: 24
                    columns: Math.max(1, Math.floor(width / 360)); columnSpacing: 12; rowSpacing: 12
                    Repeater {
                        model: controller.devices
                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            Layout.fillWidth: true; implicitHeight: 64; radius: 10; color: "#ffffff"
                            border.width: window.selected === index ? 2 : 1
                            border.color: window.selected === index ? theme.accent : theme.line
                            readonly property string lvl: modelData.protected ? "grey" : (modelData.romModeSuspect ? "bad" : window.severityLevel(modelData.aiSeverity))
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 12
                                Rectangle { width: 38; height: 38; radius: 8; color: "#f2f4f7"; Image { anchors.centerIn: parent; source: window.icon("chip"); sourceSize: Qt.size(20, 20) } }
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    Label { text: modelData.name; font.bold: true; color: theme.strong; elide: Text.ElideRight; Layout.fillWidth: true }
                                    Label { color: theme.muted; font.pixelSize: 12; elide: Text.ElideRight; Layout.fillWidth: true
                                            text: modelData.connection === "PCIe" ? "PCIe " + modelData.pciAddress + " · " + modelData.pciVendorId + ":" + modelData.pciDeviceId
                                                                                   : "USB · VID " + modelData.bridgeVendorId + " · PID " + modelData.bridgeProductId }
                                }
                                Pill { level: lvl; text: modelData.protected ? "Geschützt" : (modelData.romModeSuspect ? "Notmodus" : (modelData.aiSeverity === "Keine akute SMART-Warnung" ? "Gesund" : (modelData.aiSeverity || "neu"))) }
                            }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; enabled: !controller.recoveryRunning; onClicked: controller.autoDiagnose(index) }
                        }
                    }
                    Label {
                        visible: controller.devices.length === 0
                        text: controller.linuxEdition ? "Keine NVMe-SSD am PCIe-Bus gefunden. SSD in den M.2-Steckplatz setzen und „Geräte erkennen“ wählen."
                                                      : "Keine externe USB-SSD angeschlossen. SSD im USB-Gehäuse anschließen – die KI erkennt sie automatisch."
                        color: theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                }

                StackLayout {
                    Layout.fillWidth: true; Layout.leftMargin: 24; Layout.rightMargin: 24
                    currentIndex: window.page

                    // ===== 0 Übersicht
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true; visible: window.hasDevice
                            RowLayout {
                                Layout.fillWidth: true; spacing: 18
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 6
                                    Label { text: window.dev.nvmeControllerModel && window.dev.nvmeControllerModel !== "Nicht ermittelt" ? window.dev.nvmeControllerModel : (window.dev.name || ""); font.pixelSize: 22; font.bold: true; color: theme.strong }
                                    Label { color: theme.muted; Layout.fillWidth: true; elide: Text.ElideRight
                                            text: "Firmware " + (window.dev.nvmeFirmware || "–") + " · " + (window.report.catalog && window.report.catalog.controllerName ? window.report.catalog.controllerName : (window.report.catalog && window.report.catalog.vendorName ? window.report.catalog.vendorName : "Controller unbekannt")) + " · " + (window.dev.nvmePassthrough || "") }
                                    Rectangle {
                                        Layout.fillWidth: true; radius: 10; color: "#f8fafc"; border.color: theme.line
                                        implicitHeight: verdict.implicitHeight + 24
                                        Label { id: verdict; anchors.fill: parent; anchors.margins: 12; wrapMode: Text.WordWrap; textFormat: Text.StyledText
                                                text: "<b>KI-Diagnose:</b> " + (window.report.diagnosis || "") + (window.report.kiModell ? " <b>Wahrscheinliche Ursache: " + window.report.kiModell.text + " (" + Math.round(window.report.kiModell.wahrscheinlichkeit * 100) + " %).</b>" : "") }
                                    }
                                    Flow {
                                        Layout.fillWidth: true; spacing: 8; Layout.topMargin: 6
                                        ActionButton { text: "Automatisch retten"; iconName: "save"; primary: !controller.linuxEdition || window.severityLevel(window.report.severity) !== "bad"; enabled: !controller.recoveryRunning; onClicked: { window.page = 1; controller.startAutoRecovery(window.selected, destination.text) } }
                                        ActionButton { visible: controller.linuxEdition; text: "Reparieren"; iconName: "tool"; primary: window.severityLevel(window.report.severity) === "bad"; enabled: !controller.recoveryRunning; onClicked: { window.page = 2; controller.startRepair(window.selected, fullSurfaceBox.checked, "") } }
                                        ActionButton { text: "Cache testen"; iconName: "cache"; enabled: !controller.recoveryRunning; onClicked: { window.page = 4; controller.runCacheTest(window.selected, longTestBox.checked, writeTestBox.checked) } }
                                        ActionButton { text: "Firmware …"; iconName: "chip"; enabled: !controller.recoveryRunning; onClicked: firmwareFileDialog.open() }
                                    }
                                }
                                // Health ring
                                Item {
                                    implicitWidth: 132; implicitHeight: 132
                                    readonly property string lvl: window.severityLevel(window.report.severity)
                                    Canvas {
                                        anchors.fill: parent
                                        property real val: window.score(window.report.severity)
                                        property color col: window.levelColor(parent.lvl)
                                        onValChanged: requestPaint(); onColChanged: requestPaint()
                                        onPaint: {
                                            var c = getContext("2d"); c.reset(); c.lineWidth = 14; c.lineCap = "round"
                                            c.strokeStyle = "#eef1f5"; c.beginPath(); c.arc(66, 66, 56, 0, 2 * Math.PI); c.stroke()
                                            c.strokeStyle = col; c.beginPath(); c.arc(66, 66, 56, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * val / 100); c.stroke()
                                        }
                                    }
                                    ColumnLayout {
                                        anchors.centerIn: parent; spacing: 0
                                        Label { Layout.alignment: Qt.AlignHCenter; text: window.score(window.report.severity); font.pixelSize: 30; font.bold: true; color: theme.strong }
                                        Label { Layout.alignment: Qt.AlignHCenter; text: window.report.severity === "Keine akute SMART-Warnung" ? "Gesund" : (window.report.severity || ""); color: theme.muted; font.pixelSize: 12 }
                                    }
                                }
                            }
                        }
                        // Tiles
                        RowLayout {
                            Layout.fillWidth: true; spacing: 12; visible: window.hasDevice && !!window.dev.smartAvailable
                            Tile { label: "Temperatur"; value: (window.dev.temperatureCelsius || "–") + " °C"
                                   level: +window.dev.temperatureCelsius >= 70 ? "bad" : (+window.dev.temperatureCelsius >= 55 ? "warn" : "good")
                                   note: +window.dev.temperatureCelsius >= 70 ? "zu heiß" : (+window.dev.temperatureCelsius >= 55 ? "erhöht" : "normal") }
                            Tile { label: "Reserve"; value: (window.dev.availableSparePercent || "–") + " %"
                                   level: +window.dev.availableSparePercent < 20 ? "bad" : (+window.dev.availableSparePercent < 90 ? "warn" : "good")
                                   note: +window.dev.availableSparePercent < 20 ? "erschöpft" : (+window.dev.availableSparePercent < 90 ? "sinkt" : "voll") }
                            Tile { label: "Verschleiß"; value: (window.dev.percentageUsed || "–") + " %"
                                   level: +window.dev.percentageUsed >= 90 ? "bad" : (+window.dev.percentageUsed >= 30 ? "warn" : "good")
                                   note: +window.dev.percentageUsed >= 90 ? "verbraucht" : (+window.dev.percentageUsed >= 30 ? "mittel" : "gering") }
                            Tile { label: "Medienfehler"; value: window.dev.mediaErrors || "–"
                                   level: +window.dev.mediaErrors > 0 ? "bad" : "good"; note: +window.dev.mediaErrors > 0 ? "vorhanden" : "keine" }
                        }
                        // Controller messages + board
                        RowLayout {
                            Layout.fillWidth: true; spacing: 18; visible: window.hasDevice
                            Card {
                                Layout.fillWidth: true; Layout.preferredWidth: 1; Layout.alignment: Qt.AlignTop
                                Heading { text: "Was der Controller meldet"; iconName: "warn" }
                                Label { visible: !window.report.controllerMeldungen || window.report.controllerMeldungen.eintraege.length === 0; text: "Der Controller meldet keine Fehler."; color: theme.goodText }
                                Repeater {
                                    model: window.report.controllerMeldungen ? window.report.controllerMeldungen.eintraege : []
                                    ColumnLayout {
                                        Layout.fillWidth: true; spacing: 4
                                        Rectangle { Layout.fillWidth: true; height: 1; color: "#f0f2f5"; visible: index > 0 }
                                        RowLayout {
                                            spacing: 8; Layout.fillWidth: true
                                            Pill { text: modelData.schwere === "kritisch" ? "kritisch" : (modelData.schwere === "hoch" ? "auffällig" : "Hinweis"); level: modelData.schwere === "kritisch" ? "bad" : (modelData.schwere === "hoch" ? "warn" : "grey") }
                                            Rectangle {
                                                Layout.fillWidth: true; radius: 6; color: "#f8fafc"; implicitHeight: rawL.implicitHeight + 8
                                                Label { id: rawL; anchors.fill: parent; anchors.margins: 4; anchors.leftMargin: 8; text: modelData.roh; font.family: theme.mono; font.pixelSize: 12; color: theme.muted; elide: Text.ElideRight }
                                            }
                                        }
                                        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; textFormat: Text.StyledText
                                                text: "<b>" + modelData.uebersetzung + (modelData.detail ? " (" + modelData.detail + ")" : "") + ":</b> " + (modelData.bedeutung || "") }
                                    }
                                }
                            }
                            Card {
                                id: boardCard
                                Layout.fillWidth: true; Layout.preferredWidth: 1; Layout.alignment: Qt.AlignTop
                                readonly property var board: window.report.platine || ({})
                                RowLayout {
                                    Layout.fillWidth: true
                                    Heading { text: "Platine"; iconName: "chip" }
                                    Label { text: "schematisch"; color: theme.muted; font.pixelSize: 12 }
                                    Item { Layout.fillWidth: true }
                                    ComboBox {
                                        implicitWidth: 260; model: controller.boardLayouts(); textRole: "name"; valueRole: "id"
                                        currentIndex: boardCard.board.layoutId ? indexOfValue(boardCard.board.layoutId) : -1
                                        onActivated: controller.setBoardLayout(currentValue)
                                    }
                                }
                                Repeater {
                                    model: boardCard.board.beidseitig ? ["oben", "unten"] : ["oben"]
                                    ColumnLayout {
                                        Layout.fillWidth: true; spacing: 4
                                        property string side: modelData
                                        Label { text: side === "oben" ? "Oberseite" : "Unterseite"; color: theme.muted; font.pixelSize: 12 }
                                        Rectangle {
                                            id: pcb
                                            Layout.fillWidth: true
                                            implicitHeight: width * (boardCard.board.breite || 22) / (boardCard.board.laenge || 80)
                                            radius: 10; color: "#e8f3ec"; border.color: "#b9dcc6"
                                            readonly property real s: width / (boardCard.board.laenge || 80)
                                            Repeater {
                                                model: (boardCard.board.teile || []).filter(function (p) { return p.typ === "stecker" || p.seite === side })
                                                Rectangle {
                                                    x: modelData.x * pcb.s; y: modelData.y * pcb.s; width: modelData.w * pcb.s; height: modelData.h * pcb.s; radius: 5
                                                    color: modelData.defekt ? (modelData.schwere === "kritisch" ? theme.badSoft : theme.warnSoft) : (modelData.typ === "stecker" ? "#f5e6b8" : (modelData.typ === "nand" ? "#f2f4f7" : "#ffffff"))
                                                    border.width: modelData.defekt ? 2 : 1
                                                    border.color: modelData.defekt ? (modelData.schwere === "kritisch" ? theme.bad : theme.warn) : (modelData.typ === "stecker" ? "#d9b75a" : "#c7ced8")
                                                    Label {
                                                        anchors.centerIn: parent; width: parent.width - 4; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap
                                                        font.pixelSize: 11; font.bold: true; text: modelData.name + (modelData.defekt ? " ⚠" : "")
                                                        color: modelData.defekt ? (modelData.schwere === "kritisch" ? theme.badText : theme.warnText) : "#344054"
                                                    }
                                                    HoverHandler { id: ph }
                                                    ToolTip.visible: ph.hovered && modelData.defekt
                                                    ToolTip.text: modelData.gruende.join("\n")
                                                }
                                            }
                                        }
                                    }
                                }
                                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.goodText; visible: (boardCard.board.befunde || []).length === 0; text: "Keine Bauteile auffällig." }
                                Repeater {
                                    model: boardCard.board.befunde || []
                                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; textFormat: Text.StyledText
                                            color: modelData.schwere === "kritisch" ? theme.badText : theme.warnText
                                            text: "<b>" + ({ controller: "Controller", dram: "Cache/DRAM", nand: "Flash-Speicher", pmic: "Spannungsversorgung", stecker: "Stecker/Verbindung" })[modelData.typ] + ":</b> " + modelData.grund }
                                }
                            }
                        }
                        // Reparaturwege
                        Card {
                            Layout.fillWidth: true; visible: window.hasDevice && (window.report.repairPaths || []).length > 0
                            Heading { text: "Reparaturwege laut Vorlage" + (window.report.catalog && window.report.catalog.family ? " · " + window.report.catalog.family.hersteller : ""); iconName: "tool" }
                            Repeater {
                                model: window.report.repairPaths || []
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 10
                                    Pill { text: ({ datenrettung: "Datenrettung", nvme_firmware_update: "Firmware-Update", notmodus_hersteller_werkzeug: "Notmodus-Werkzeug", labor: "Labor", hardware_pruefung: "Hardware prüfen", firmware_slot_aktivieren: "Firmware-Slot" })[modelData.typ] || modelData.typ
                                           level: modelData.datenverlust === "ja" ? "bad" : "grey" }
                                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: modelData.status; color: theme.text }
                                }
                            }
                        }
                        Card {
                            Layout.fillWidth: true; visible: !window.hasDevice
                            Heading { text: "Willkommen"; iconName: "spark" }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Die KI überwacht die Anschlüsse und untersucht jede SSD automatisch. Wähle oben eine SSD, um ihre Diagnose zu sehen." }
                        }
                    }

                    // ===== 1 Rettung
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            RowLayout { Layout.fillWidth: true
                                Heading { text: "Automatische Rettung"; iconName: "save" }
                                Item { Layout.fillWidth: true }
                                Pill { visible: controller.recoveryRunning || !!controller.recoveryResult.state
                                       text: controller.recoveryRunning ? "läuft" : (controller.recoveryResult.rating || ""); level: controller.recoveryRunning ? "grey" : (controller.recoveryResult.rating === "Vollständig gerettet" ? "good" : "warn") }
                            }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Die SSD wird nur gelesen. Das Abbild landet im Zielordner oben rechts; nach Abbruch setzt die Rettung an derselben Stelle fort." }
                            RowLayout {
                                ActionButton { text: "Rettung starten"; primary: true; iconName: "save"; enabled: window.hasDevice && !controller.recoveryRunning; onClicked: controller.startAutoRecovery(window.selected, destination.text) }
                                ActionButton { text: "Anhalten"; visible: controller.recoveryRunning; onClicked: controller.cancelRecovery() }
                                ActionButton { text: "Ziel prüfen"; iconName: "folder"; onClicked: controller.inspectDestination(destination.text) }
                            }
                            RowLayout {
                                Layout.fillWidth: true; visible: controller.recoveryRunning
                                Label { text: controller.recoveryProgress.phase || "Vorbereitung …"; Layout.fillWidth: true }
                                Label { text: (controller.recoveryProgress.percent || 0).toFixed(1) + " %"; font.bold: true }
                            }
                            Progress { Layout.fillWidth: true; visible: controller.recoveryRunning; value: controller.recoveryProgress.percent || 0 }
                            RowLayout {
                                Layout.fillWidth: true; visible: controller.recoveryRunning && !!controller.recoveryProgress.rescued; spacing: 12
                                Tile { label: "Gesichert"; value: controller.recoveryProgress.rescued || "–"; note: (controller.recoveryProgress.rescuedPercent || 0).toFixed(2) + " %"; level: "good" }
                                Tile { label: "Nicht lesbar"; value: controller.recoveryProgress.bad || "–"; note: (controller.recoveryProgress.badRegions || 0) + " Bereiche"; level: (controller.recoveryProgress.badRegions || 0) > 0 ? "bad" : "good" }
                                Tile { label: "Tempo"; value: controller.recoveryProgress.rate || "–"; note: "" }
                                Tile { label: "Restzeit"; value: controller.recoveryProgress.eta || "–"; note: "" }
                            }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; visible: !controller.recoveryRunning && !!controller.recoveryResult.summary; font.bold: true; text: controller.recoveryResult.summary || "" }
                            Repeater { model: controller.recoveryRunning ? [] : (controller.recoveryResult.nextSteps || []); Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "→ " + modelData } }
                            Label { visible: !!controller.recoveryResult.reportPath; text: "Bericht: " + (controller.recoveryResult.reportPath || ""); color: theme.muted; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                        }
                        Card {
                            Layout.fillWidth: true; visible: controller.recoveryLog.length > 0
                            Heading { text: "Protokoll"; iconName: "clock" }
                            Repeater { model: controller.recoveryLog.slice(-30); Label { Layout.fillWidth: true; text: modelData; font.family: theme.mono; font.pixelSize: 12; color: theme.muted; elide: Text.ElideRight } }
                        }
                    }

                    // ===== 2 Reparatur
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            RowLayout { Layout.fillWidth: true
                                Heading { text: "Automatische Reparaturkette"; iconName: "tool" }
                                Item { Layout.fillWidth: true }
                                Pill { visible: !!controller.repairProgress.result && !controller.repairProgress.running
                                       text: controller.repairProgress.result && controller.repairProgress.result.ok ? "repariert" : "nicht repariert"
                                       level: controller.repairProgress.result && controller.repairProgress.result.ok ? "good" : "bad" }
                            }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.badText
                                    text: "Löscht alle Daten der SSD. Ablauf: PCIe-Reset → Controller-Reset → anderer Firmware-Slot → Format NVM → Sanitize → Firmware neu; nach Erfolg Oberflächen- und Nullfüll-Test." }
                            RowLayout {
                                CheckBox { id: fullSurfaceBox; text: "Abschlusstest über die ganze SSD" }
                                Item { Layout.fillWidth: true }
                                ActionButton { text: "Reparatur starten"; primary: true; iconName: "tool"; enabled: window.hasDevice && !controller.recoveryRunning; onClicked: controller.startRepair(window.selected, fullSurfaceBox.checked, "") }
                            }
                            RowLayout {
                                Layout.fillWidth: true; visible: !!controller.repairProgress.running
                                Label { text: controller.repairProgress.phase || ""; Layout.fillWidth: true }
                                Label { text: (controller.repairProgress.percent || 0) + " %"; font.bold: true }
                            }
                            Progress { Layout.fillWidth: true; visible: !!controller.repairProgress.running; value: controller.repairProgress.percent || 0 }
                            Repeater {
                                model: controller.repairProgress.result ? controller.repairProgress.result.schritte : []
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 10
                                    Rectangle { width: 24; height: 24; radius: 12; color: modelData.ok ? theme.good : theme.bad
                                                Image { anchors.centerIn: parent; source: window.icon(modelData.ok ? "check" : "x", "white"); sourceSize: Qt.size(14, 14) } }
                                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; textFormat: Text.StyledText; text: "<b>" + modelData.schritt + ":</b> " + modelData.antwort }
                                }
                            }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; visible: !!controller.repairProgress.result && !controller.repairProgress.running
                                    text: controller.repairProgress.result ? controller.repairProgress.result.message : "" }
                        }
                    }

                    // ===== 3 Firmware
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            Heading { text: "Firmware-Manager"; iconName: "chip" }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: controller.linuxEdition ? "Standard-NVMe-Update: Prüfungen, Übertragung mit Fortschritt, Aktivierung und Kontrolle. Nur offizielle Herstellerdateien verwenden."
                                                                  : "Über USB-Gehäuse sind Firmware-Updates meist nicht möglich. Für das Flashen den Linux-Boot-Stick mit der SSD im M.2-Steckplatz verwenden." }
                            Repeater {
                                model: window.report.firmwareIssues || []
                                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.warnText; textFormat: Text.StyledText
                                        text: "<b>Bekannter Firmwarefehler:</b> " + modelData.aktuelleFirmware + " → Update auf " + modelData.korrigierteFirmware + " (" + modelData.quelleFirmware + ")" }
                            }
                            Label { visible: window.hasDevice; text: "Aktuell: " + (window.dev.nvmeFirmware || "–") + (window.dev.firmwareSlotRevisions ? " · Slots: " + window.dev.firmwareSlotRevisions.join(", ") : ""); color: theme.text }
                            ActionButton { text: "Firmwaredatei wählen …"; primary: true; iconName: "chip"; enabled: window.hasDevice && !controller.recoveryRunning; onClicked: firmwareFileDialog.open() }
                        }
                        // Offline-Firmware-Archiv
                        Card {
                            Layout.fillWidth: true
                            RowLayout { Layout.fillWidth: true
                                Heading { text: "Offline-Firmware-Archiv"; iconName: "save" }
                                Item { Layout.fillWidth: true }
                                Pill { text: (controller.archive.dateien || 0) + " Dateien · " + (controller.archive.megabyte || 0) + " MB"; level: (controller.archive.dateien || 0) > 0 ? "good" : "grey" }
                            }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Läuft von allein: Sobald Internet da ist (LAN oder WLAN), lädt die KI neue Firmware und fehlende Werkzeuge selbst nach – beim Start und danach täglich. Ohne Internet wird das Archiv offline genutzt. Beim Erstellen des Linux-Boot-Sticks kommen Archiv, Werkzeuge und KI-Wissen automatisch mit auf den Stick (Partition NVMEDATEN)." + (controller.archive.stand ? " Stand: " + controller.archive.stand.substring(0, 10) : "") }
                            Flow {
                                Layout.fillWidth: true; spacing: 8
                                ActionButton { text: controller.archive.laeuft ? "Aktualisiert …" : "Jetzt aktualisieren"; primary: true; iconName: "refresh"; enabled: !controller.archive.laeuft; onClicked: controller.fillArchive() }
                                ActionButton { text: "Anhalten"; visible: !!controller.archive.laeuft; onClicked: controller.cancelArchive() }
                                ActionButton { text: "Ordner / Stick übernehmen"; iconName: "folder"; onClicked: archiveImportDialog.open() }
                                ActionButton { text: "Archiv exportieren"; iconName: "usb"; onClicked: archiveExportDialog.open() }
                            }
                            RowLayout {
                                Layout.fillWidth: true; visible: !!controller.archive.laeuft
                                Label { text: controller.archive.phase || ""; Layout.fillWidth: true; elide: Text.ElideRight }
                                Label { text: (controller.archive.fortschritt || "") + " · " + (controller.archive.prozent || 0) + " %"; font.bold: true }
                            }
                            Progress { Layout.fillWidth: true; visible: !!controller.archive.laeuft; value: controller.archive.prozent || 0 }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; visible: !controller.archive.laeuft && !!controller.archive.phase; text: controller.archive.phase || ""; color: theme.goodText }
                            Label { text: "Passend für die gewählte SSD"; font.bold: true; visible: window.hasDevice; Layout.topMargin: 4 }
                            Label { visible: window.hasDevice && controller.archiveMatches.length === 0; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Für dieses Modell liegt noch keine Firmware im Archiv. Die KI sucht bei der nächsten Aktualisierung automatisch danach." }
                            Repeater {
                                model: controller.archiveMatches
                                Rectangle {
                                    Layout.fillWidth: true; radius: 8; border.color: theme.line; implicitHeight: mRow.implicitHeight + 16
                                    RowLayout {
                                        id: mRow; anchors.fill: parent; anchors.margins: 8; anchors.leftMargin: 12; spacing: 10
                                        ColumnLayout {
                                            Layout.fillWidth: true; spacing: 2
                                            Label { text: modelData.hersteller + " · Version " + modelData.version; font.bold: true; color: theme.strong }
                                            Label { Layout.fillWidth: true; elide: Text.ElideRight; font.pixelSize: 12; color: theme.muted
                                                    text: modelData.treffer + (modelData.voraussetzungen && modelData.voraussetzungen.length ? " · setzt voraus: " + modelData.voraussetzungen.join(", ") : "") + (modelData.art === "samsung-iso" ? " · Samsung-ISO" : (modelData.art === "lvfs-cab" ? " · LVFS-Paket" : "")) }
                                        }
                                        Pill { text: modelData.istAktuell ? "bereits installiert" : (modelData.vorhanden ? "offline verfügbar" : "Datei fehlt"); level: modelData.istAktuell ? "grey" : (modelData.vorhanden ? "good" : "bad") }
                                        ActionButton {
                                            text: "Verwenden"; primary: !modelData.istAktuell; iconName: "chip"
                                            enabled: modelData.vorhanden && !controller.recoveryRunning && ((modelData.art !== "samsung-iso" && modelData.art !== "lvfs-cab") || controller.linuxEdition)
                                            onClicked: {
                                                if (modelData.art === "samsung-iso" || modelData.art === "lvfs-cab") controller.runArchived(modelData.pfad, modelData.art)
                                                else { firmwareDialog.fileUrl = "file:///" + modelData.pfad.replace(/^\/+/, ""); controller.prepareFirmware(window.selected, firmwareDialog.fileUrl, noBackupBox.checked); firmwareDialog.open() }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Card {
                            Layout.fillWidth: true
                            Heading { text: "Firmware-Quellen der Hersteller"; iconName: "box" }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Die KI durchsucht alle Herstellerseiten selbstständig nach Firmware. Anbieter, die ihre Dateien nur in eigenen Programmen ausliefern, werden trotzdem bei jeder Aktualisierung neu geprüft." }
                            Repeater {
                                model: controller.firmwareVendors()
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 10
                                    Label { text: modelData.hersteller; font.bold: true; Layout.preferredWidth: 200; elide: Text.ElideRight }
                                    Label { text: modelData.modelle; color: theme.muted; Layout.fillWidth: true; elide: Text.ElideRight; font.pixelSize: 12 }
                                    Label { text: modelData.dateien + " Dateien"; color: theme.muted; font.pixelSize: 12 }
                                    Pill { text: modelData.status; level: modelData.dateien > 0 ? "good" : (modelData.status === "Server nicht erreichbar" ? "warn" : "grey") }
                                    ToolButton { text: "Seite"; onClicked: controller.openUrl(modelData.seite) }
                                }
                            }
                        }
                    }

                    // ===== 4 Cache-Test
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            Heading { text: "Cache-Test"; iconName: "cache" }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Prüft Cache-Ausstattung (DRAM / Host-Speicher), startet den Controller-Selbsttest und misst Lesezugriffe. Der Schreibtest überschreibt Daten (nur Linux)." }
                            RowLayout {
                                CheckBox { id: longTestBox; text: "Langer Selbsttest" }
                                CheckBox { id: writeTestBox; visible: controller.linuxEdition; text: "Schreibtest (überschreibt Daten)" }
                                Item { Layout.fillWidth: true }
                                ActionButton { text: "Cache testen"; primary: true; iconName: "cache"; enabled: window.hasDevice && !controller.recoveryRunning; onClicked: controller.runCacheTest(window.selected, longTestBox.checked, writeTestBox.checked) }
                            }
                            RowLayout { Layout.fillWidth: true; visible: !!controller.cacheProgress.running
                                Label { text: controller.cacheProgress.phase || ""; Layout.fillWidth: true }
                                Label { text: (controller.cacheProgress.percent || 0) + " %"; font.bold: true } }
                            Progress { Layout.fillWidth: true; visible: !!controller.cacheProgress.running; value: controller.cacheProgress.percent || 0 }
                            Label { visible: !!window.report.cacheTest; Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true
                                    color: window.report.cacheTest && window.report.cacheTest.auffaellig ? theme.badText : theme.goodText
                                    text: window.report.cacheTest ? window.report.cacheTest.urteil : "" }
                            Repeater { model: window.report.cacheTest ? window.report.cacheTest.details : []; Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "• " + modelData; color: theme.text } }
                        }
                    }

                    // ===== 5 Werkzeuge
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            Heading { text: "Werkzeuge der KI"; iconName: "box" }
                            RowLayout { Layout.fillWidth: true; visible: !!controller.toolDownload.name
                                Progress { Layout.fillWidth: true; value: controller.toolDownload.percent || 0 }
                                Label { text: (controller.toolDownload.percent || 0) + " % · " + (controller.toolDownload.phase || ""); color: theme.muted } }
                            Repeater {
                                model: controller.tools
                                Rectangle {
                                    Layout.fillWidth: true; radius: 8; border.color: theme.line; implicitHeight: toolRow.implicitHeight + 16
                                    RowLayout {
                                        id: toolRow; anchors.fill: parent; anchors.margins: 8; anchors.leftMargin: 12; spacing: 10
                                        ColumnLayout {
                                            Layout.fillWidth: true; spacing: 2
                                            Label { text: modelData.name; font.bold: true; color: theme.strong }
                                            Label { text: modelData.zweck; color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                        }
                                        Pill { text: modelData.installiert ? "installiert" : (modelData.nutzerBereitgestellt ? "selbst bereitstellen" : "fehlt"); level: modelData.installiert ? "good" : "grey" }
                                        ActionButton { visible: !modelData.installiert && !modelData.nutzerBereitgestellt; text: modelData.automatisch ? "Installieren" : "Seite öffnen"; enabled: !controller.toolDownload.running; onClicked: controller.installTool(modelData.id) }
                                        ActionButton { visible: !modelData.installiert; text: "Registrieren"; onClicked: { toolFileDialog.toolId = modelData.id; toolFileDialog.open() } }
                                    }
                                }
                            }
                        }
                    }

                    // ===== 6 Linux-Boot-Stick
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            Heading { text: "Linux-Boot-Stick erstellen"; iconName: "usb" }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted
                                    text: "Der Stick wird vollständig überschrieben. Danach startet der Rechner vom Stick direkt in NVMe Recovery Studio AI mit direktem Zugriff auf interne M.2-SSDs." }
                            Rectangle {
                                Layout.fillWidth: true; radius: 8; color: theme.accentSoft; implicitHeight: offl.implicitHeight + 20
                                Label { id: offl; anchors.fill: parent; anchors.margins: 10; wrapMode: Text.WordWrap; color: theme.accentText; textFormat: Text.StyledText
                                        text: "<b>Offline-Paket kommt mit auf den Stick:</b> " + (controller.archive.dateien || 0) + " Firmwaredateien (" + (controller.archive.megabyte || 0) + " MB), "
                                              + controller.tools.filter(function (t) { return t.installiert }).length + " Werkzeuge und das KI-Wissen – in der Partition NVMEDATEN. Am Rechner mit der defekten SSD wird kein Internet gebraucht."
                                              + ((controller.archive.dateien || 0) === 0 ? " Das Archiv füllt sich automatisch, sobald dieser PC Internet hat." : "") }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                TextField { id: stickImage; Layout.fillWidth: true; placeholderText: controller.stickProgress.image ? "Geladen: " + controller.stickProgress.image : "Boot-Abbild (.iso)" }
                                ActionButton { text: "Datei wählen"; onClicked: stickImageDialog.open() }
                                ActionButton { text: "Neueste laden"; enabled: !controller.stickProgress.running; onClicked: controller.downloadBootImage() }
                            }
                            ComboBox { id: stickTarget; Layout.fillWidth: true; model: controller.devices; textRole: "name"
                                       displayText: currentIndex >= 0 ? controller.devices[currentIndex].name + " · " + controller.devices[currentIndex].devicePath : "USB-Stick wählen" }
                            RowLayout {
                                Layout.fillWidth: true
                                TextField { id: stickConfirm; Layout.fillWidth: true; placeholderText: "Zur Bestätigung STICK eingeben" }
                                ActionButton { text: "Stick beschreiben"; danger: true; iconName: "usb"
                                    enabled: !controller.stickProgress.running && stickTarget.currentIndex >= 0 && stickImage.text.length > 0 && stickConfirm.text.toUpperCase() === "STICK"
                                    onClicked: controller.createBootStick(stickTarget.currentIndex, stickImage.text, stickConfirm.text) }
                            }
                            Progress { Layout.fillWidth: true; visible: !!controller.stickProgress.phase; value: controller.stickProgress.percent || 0 }
                            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: (controller.stickProgress.phase ? (controller.stickProgress.percent || 0) + " % · " : "") + (controller.stickProgress.phase || "") }
                        }
                    }

                    // ===== 7 Verlauf
                    ColumnLayout {
                        spacing: 18
                        Card {
                            Layout.fillWidth: true
                            Heading { text: "Verlauf der KI"; iconName: "clock" }
                            Repeater {
                                model: controller.aiMonitor.slice().reverse().slice(0, 200)
                                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.pixelSize: 13
                                        color: modelData.indexOf("Warnung") >= 0 ? theme.badText : (modelData.indexOf("Gelernt") >= 0 || modelData.indexOf("gelernt") >= 0 ? theme.goodText : (modelData.indexOf("Entscheidung") >= 0 ? theme.warnText : theme.text))
                                        text: modelData }
                            }
                        }
                    }
                }
                Label { Layout.leftMargin: 24; Layout.rightMargin: 24; Layout.bottomMargin: 18; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted; text: controller.status }
            }
        }

        // --- KI-Assistent
        Rectangle {
            Layout.preferredWidth: 340; Layout.fillHeight: true; color: "#ffffff"
            Rectangle { anchors.left: parent.left; width: 1; height: parent.height; color: theme.line }
            ScrollView {
                anchors.fill: parent; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width - 36; x: 18; spacing: 16
                    Item { implicitHeight: 4 }
                    Heading { text: "KI-Assistent"; iconName: "spark" }
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 10; visible: !!window.report.kiModell
                        Label { text: "Wahrscheinliche Ursache"; color: theme.muted; font.pixelSize: 12 }
                        Repeater {
                            model: window.report.kiModell ? window.report.kiModell.rangliste.slice(0, 3) : []
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 4
                                RowLayout { Layout.fillWidth: true
                                    Label { text: modelData.text; Layout.fillWidth: true; elide: Text.ElideRight; font.pixelSize: 13 }
                                    Label { text: Math.round(modelData.wahrscheinlichkeit * 100) + " %"; font.bold: true; font.pixelSize: 13 } }
                                Progress { Layout.fillWidth: true; implicitHeight: 8; value: modelData.wahrscheinlichkeit * 100
                                           barColor: index === 0 ? (modelData.ursache === "gesund" ? theme.good : theme.bad) : (index === 1 ? theme.warn : theme.accent) }
                            }
                        }
                        Label { visible: window.report.kiModell && window.report.kiModell.gruende.length > 0; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted; font.pixelSize: 12
                                text: window.report.kiModell ? "Gründe: " + window.report.kiModell.gruende.join(", ") : "" }
                        ComboBox { id: causeBox; Layout.fillWidth: true; model: controller.causeOptions(); textRole: "text"; valueRole: "id" }
                        ActionButton { Layout.fillWidth: true; text: "Ursache bestätigen – KI lernt"; onClicked: controller.confirmCause(causeBox.currentValue) }
                    }
                    Label { text: "Verlauf"; color: theme.muted; font.pixelSize: 12 }
                    Repeater {
                        model: controller.aiMonitor.slice(-8).reverse()
                        RowLayout {
                            Layout.fillWidth: true; spacing: 10
                            readonly property string kind: modelData.indexOf("Warnung") >= 0 || modelData.indexOf("✗") >= 0 ? "bad"
                                                         : (modelData.indexOf("Gelernt") >= 0 || modelData.indexOf("✓") >= 0 || modelData.indexOf("gelernt") >= 0 ? "good" : "blue")
                            Rectangle { Layout.alignment: Qt.AlignTop; width: 22; height: 22; radius: 11; color: kind === "bad" ? theme.bad : (kind === "good" ? theme.good : theme.accent)
                                        Image { anchors.centerIn: parent; source: window.icon(kind === "bad" ? "warn" : (kind === "good" ? "check" : "spark"), "white"); sourceSize: Qt.size(12, 12) } }
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 0
                                Label { text: modelData.substring(0, 8); color: "#98a2b3"; font.pixelSize: 11 }
                                Label { text: modelData.substring(10); Layout.fillWidth: true; wrapMode: Text.WordWrap; font.pixelSize: 13; maximumLineCount: 3; elide: Text.ElideRight }
                            }
                        }
                    }
                    ActionButton { Layout.fillWidth: true; text: "Ganzen Verlauf zeigen"; iconName: "clock"; onClicked: window.page = 7 }
                    Label { text: "Werkzeuge"; color: theme.muted; font.pixelSize: 12 }
                    Repeater {
                        model: controller.tools.filter(function (t) { return t.installiert || t.id === "smartctl" }).slice(0, 4)
                        Rectangle {
                            Layout.fillWidth: true; implicitHeight: 38; radius: 8; border.color: theme.line
                            RowLayout { anchors.fill: parent; anchors.margins: 8
                                Label { text: modelData.name; Layout.fillWidth: true; elide: Text.ElideRight; font.pixelSize: 13 }
                                Pill { text: modelData.installiert ? "installiert" : "fehlt"; level: modelData.installiert ? "good" : "grey" } }
                        }
                    }
                    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted; font.pixelSize: 12
                            text: "Wissensbasis: " + (controller.knowledge.devices || 0) + " Geräte · " + (controller.knowledge.bridges || 0) + " Bridges · Vorlagen: " + (controller.catalog.vendors || 0) + " Hersteller, " + (controller.catalog.devices || 0) + " Controller" }
                    Item { implicitHeight: 12 }
                }
            }
        }
    }
}
