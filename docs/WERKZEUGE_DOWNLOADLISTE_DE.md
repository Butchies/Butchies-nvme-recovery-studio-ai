# Werkzeuge für die Recovery-KI – Downloadliste

Diese Liste wird aus `data/catalog/tools.json` erzeugt. Das Programm zeigt dieselben Einträge im Bereich „Werkzeuge“, erkennt installierte Werkzeuge automatisch und nutzt sie selbst.

| Werkzeug | Zweck | Wie die KI es nutzt | Plattform | Offizielle Quelle | Installation |
|---|---|---|---|---|---|
| smartmontools (smartctl) | NVMe-Identify, SMART/Health und Fehlerprotokoll auch hinter USB-Bridges (JMicron, ASMedia, Realtek) auslesen. | automatisch bei jeder Diagnose, wenn Windows selbst keine NVMe-Daten über USB liefert | windows, linux | https://www.smartmontools.org/wiki/Download | automatisch durch das Programm |
| nvme-cli | Vollständiger NVMe-Zugriff unter Linux: Identify, Logs, Firmware-Slots, Firmware-Download/Commit. | Linux-Boot-Edition (Firmware-Manager, Notmodus-Prüfung) | linux | https://github.com/linux-nvme/nvme-cli | offizielle Seite öffnen, Installation wird erkannt |
| GNU ddrescue | Alternative Rettungsabbild-Erstellung unter Linux; liest dieselbe Lesekarte (abbild.map) wie dieses Programm. | Linux-Boot-Edition; Fortsetzen einer unter Windows begonnenen Rettung | linux | https://www.gnu.org/software/ddrescue/ | offizielle Seite öffnen, Installation wird erkannt |
| Samsung Magician / Samsung Firmware-ISO | Offizielle Firmware-Updates und Diagnose für Samsung-SSDs. | empfohlen bei bekanntem Samsung-Firmwarefehler; Firmware nur aus dieser Quelle | windows | https://semiconductor.samsung.com/consumer-storage/support/tools/ | offizielle Seite öffnen, Installation wird erkannt |
| Western Digital / SanDisk Dashboard | Offizielle Firmware-Updates und Diagnose für WD- und SanDisk-SSDs. | empfohlen bei bekanntem WD/SanDisk-Firmwarefehler | windows | https://support.wdc.com/knowledgebase/answer.aspx?ID=14217 | offizielle Seite öffnen, Installation wird erkannt |
| Crucial Storage Executive | Offizielle Firmware-Updates für Crucial-SSDs (P-, T-, MX-, BX-Serie); Firmware-ISO für Updates außerhalb von Windows. | empfohlen bei Crucial-SSDs | windows | https://www.crucial.com/support/storage-executive | offizielle Seite öffnen, Installation wird erkannt |
| Kingston SSD Manager | Offizielle Firmware-Updates und Diagnose für Kingston-SSDs. | empfohlen bei Kingston-SSDs (nur interner Anschluss, keine USB-Gehäuse) | windows | https://www.kingston.com/de/support/technical/ssdmanager | offizielle Seite öffnen, Installation wird erkannt |
| SK hynix Drive Manager Easy Kit | Offizielle Firmware-Updates und Diagnose für SK-hynix-SSDs. | empfohlen bei SK-hynix-SSDs | windows | https://ssd.skhynix.com/ | offizielle Seite öffnen, Installation wird erkannt |
| Solidigm Storage Tool (SST) / Synergy Toolkit | Offizielle Firmware-Updates für Solidigm- und Intel-NAND-SSDs; CLI auch unter Linux. | empfohlen bei Solidigm/Intel-SSDs; CLI in der Linux-Boot-Edition nutzbar | windows, linux | https://www.solidigm.com/support-page/drivers-downloads/ka-00085.html | offizielle Seite öffnen, Installation wird erkannt |
| KIOXIA SSD Utility | Offizielle Firmware-Updates für KIOXIA-SSDs (EXCERIA-Serien). | empfohlen bei KIOXIA-SSDs | windows | https://europe.kioxia.com/en-mea/personal/ssd/ssd-utility.html | offizielle Seite öffnen, Installation wird erkannt |
| Seagate SeaTools / Firmware-Downloads | Diagnose und Firmware (per Seriennummer) für Seagate-SSDs (FireCuda/BarraCuda, Phison-basiert). | empfohlen bei Seagate-SSDs | windows, linux | https://www.seagate.com/support/downloads/seatools/ | offizielle Seite öffnen, Installation wird erkannt |
| Phison MPTool (Notmodus-Neuinitialisierung) | Controller im ROM-/Safe-Mode neu laden. Datenverlust! | nur Linux-Boot-Edition bzw. direkter PCIe-Anschluss, nur nach ausdrücklicher Freigabe | windows, linux | – (keine öffentliche Quelle) | vom Nutzer bereitstellen und im Programm registrieren |
| Silicon Motion MPTool | SMI-Controller im ROM-Modus neu laden. Datenverlust! | nur Linux-Boot-Edition bzw. direkter PCIe-Anschluss, nur nach ausdrücklicher Freigabe | windows | – (keine öffentliche Quelle) | vom Nutzer bereitstellen und im Programm registrieren |
| Realtek SSD MPTool | Realtek-SSD-Controller neu initialisieren. Datenverlust! | nur direkter PCIe-Anschluss, nur nach ausdrücklicher Freigabe | windows | – (keine öffentliche Quelle) | vom Nutzer bereitstellen und im Programm registrieren |
| Maxio MPTool | Maxio-Controller neu initialisieren. Datenverlust! | nur direkter PCIe-Anschluss, nur nach ausdrücklicher Freigabe | windows | – (keine öffentliche Quelle) | vom Nutzer bereitstellen und im Programm registrieren |

**Hinweise**

- Firmwaredateien werden ausschließlich aus der offiziellen Quelle des SSD-Herstellers bezogen, nie aus Foren oder Spiegelservern.
- Notmodus-Werkzeuge (MPTools) setzen die SSD in der Regel vollständig zurück: Nutzerdaten gehen verloren. Sie werden nur nach Datensicherung und ausdrücklicher Freigabe verwendet.
- Die Herstellertools von Samsung, Kingston u. a. unterstützen meist keine fremden USB-Gehäuse; für Firmware-Updates die SSD intern anschließen.
