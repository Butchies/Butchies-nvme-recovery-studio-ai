#pragma once
// Offline-Firmware-Archiv.
// Einmal mit Internet füllen (offizielle Herstellerserver), danach offline nutzen.
// Ablage: <AppData>/firmware/  –  index.json + Dateien.
// Beim Erstellen des Linux-Boot-Sticks wird das Archiv (und die geladenen
// Werkzeuge) auf die Datenpartition NVMEDATEN des Sticks kopiert; die
// Linux-Edition übernimmt es beim Start automatisch.
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <functional>

namespace FirmwareArchive {

inline QString appDataDir() {
    const QString d=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(d);
    return d;
}
inline QString dir() { const QString d=QDir(appDataDir()).filePath(QStringLiteral("firmware")); QDir().mkpath(d); return d; }
inline QString indexPath() { return QDir(dir()).filePath(QStringLiteral("index.json")); }

// Vergleichbare Form eines Modellnamens: nur Großbuchstaben und Ziffern.
inline QString norm(QString s) {
    s=s.toUpper();
    s.remove(QRegularExpression(QStringLiteral("[^A-Z0-9]")));
    s.remove(QStringLiteral("SAMSUNGSSD")); s.remove(QStringLiteral("SAMSUNG"));
    s.remove(QStringLiteral("WDBLACK")).remove(QStringLiteral("WDCWDS"));
    return s;
}
// Kapazitätsangaben (1TB, 500GB …) entfernen – Firmware gilt meist für die ganze Serie.
inline QString series(const QString &model) {
    QString s=model.toUpper();
    s.remove(QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\s*(TB|GB)\\b")));
    s.remove(QRegularExpression(QStringLiteral("WITH HEATSINK|HEATSINK")));
    return norm(s);
}

inline QJsonArray load() {
    QFile f(indexPath());
    if(!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).object().value("eintraege").toArray();
}
inline bool save(const QJsonArray &entries) {
    QSaveFile f(indexPath());
    if(!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(QJsonObject{{"version",1},{"stand",QDateTime::currentDateTime().toString(Qt::ISODate)},{"eintraege",entries}}).toJson());
    return f.commit();
}
inline bool hasSource(const QJsonArray &entries, const QString &url) {
    for(const auto &e:entries) if(e.toObject().value("quelle").toString()==url && QFile::exists(QDir(dir()).filePath(e.toObject().value("datei").toString()))) return true;
    return false;
}
inline QString sha256(const QString &path) {
    QFile f(path); if(!f.open(QIODevice::ReadOnly)) return {};
    QCryptographicHash h(QCryptographicHash::Sha256); h.addData(&f); return QString::fromLatin1(h.result().toHex());
}

// Datei ins Archiv übernehmen (Download oder Import).
inline QJsonObject add(const QString &sourceFile, QJsonObject meta) {
    const QString hersteller=meta.value("hersteller").toString(QStringLiteral("Unbekannt"));
    const QString sub=QString(hersteller).replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9]+")),QStringLiteral("_"));
    QDir(dir()).mkpath(sub);
    QString rel=sub+QLatin1Char('/')+(meta.value("modellOrdner").toString().isEmpty() ? QString() : meta.value("modellOrdner").toString()+QLatin1Char('_'))
              +QFileInfo(sourceFile).fileName().remove(QStringLiteral(".part"));
    if(meta.contains("dateiname")) rel=sub+QLatin1Char('/')+meta.value("dateiname").toString();
    const QString target=QDir(dir()).filePath(rel);
    if(QFileInfo(sourceFile).absoluteFilePath()!=QFileInfo(target).absoluteFilePath()) {
        QFile::remove(target);
        if(!QFile::copy(sourceFile,target)) return {};
    }
    meta["datei"]=rel;
    meta["groesse"]=QFileInfo(target).size();
    meta["sha256"]=sha256(target);
    meta["geladen"]=QDateTime::currentDateTime().toString(Qt::ISODate);
    meta.remove("dateiname");
    auto entries=load();
    for(int i=entries.size()-1;i>=0;--i) if(entries.at(i).toObject().value("datei").toString()==rel) entries.removeAt(i);
    entries.append(meta);
    save(entries);
    return meta;
}

// Passende Firmware für ein Gerät: gleiche Modellreihe, nach Version absteigend.
inline QVariantList match(const QVariantMap &device) {
    const QString model=device.value("nvmeControllerModel").toString().isEmpty() ? device.value("name").toString() : device.value("nvmeControllerModel").toString();
    const QString exact=norm(model), ser=series(model);
    const QString current=device.value("nvmeFirmware").toString().trimmed();
    QVariantList out;
    if(exact.isEmpty()) return out;
    for(const auto &v:load()) {
        const auto e=v.toObject();
        int score=0;
        for(const auto &m:e.value("modelle").toArray()) {
            const QString n=norm(m.toString()), s=series(m.toString());
            if(n==exact) score=qMax(score,3);
            else if(!s.isEmpty() && (s==ser || ser.contains(s) || s.contains(ser))) score=qMax(score,2);
        }
        if(!score) continue;
        auto m=e.toVariantMap();
        m["pfad"]=QDir(dir()).filePath(e.value("datei").toString());
        m["vorhanden"]=QFile::exists(m.value("pfad").toString());
        m["treffer"]=score==3 ? QStringLiteral("genau dieses Modell") : QStringLiteral("gleiche Modellreihe");
        m["istAktuell"]=!current.isEmpty() && e.value("version").toString()==current;
        m["score"]=score;
        out.append(m);
    }
    std::sort(out.begin(),out.end(),[](const QVariant &a,const QVariant &b){
        const auto x=a.toMap(), y=b.toMap();
        if(x.value("score").toInt()!=y.value("score").toInt()) return x.value("score").toInt()>y.value("score").toInt();
        return x.value("version").toString()>y.value("version").toString();
    });
    return out;
}

inline QVariantMap stats() {
    const auto entries=load();
    QVariantMap perVendor; qint64 bytes=0; int files=0;
    for(const auto &v:entries) {
        const auto e=v.toObject();
        if(!QFile::exists(QDir(dir()).filePath(e.value("datei").toString()))) continue;
        ++files; bytes+=e.value("groesse").toVariant().toLongLong();
        perVendor[e.value("hersteller").toString()]=perVendor.value(e.value("hersteller").toString()).toInt()+1;
    }
    QFile f(indexPath()); QString stand;
    if(f.open(QIODevice::ReadOnly)) stand=QJsonDocument::fromJson(f.readAll()).object().value("stand").toString();
    return {{"dateien",files},{"megabyte",int(bytes/1048576)},{"hersteller",perVendor},{"stand",stand},{"ordner",QDir::toNativeSeparators(dir())}};
}

inline void copyTree(const QString &from, const QString &to, bool overwrite, int &copied) {
    QDirIterator it(from,QDir::Files|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);
    while(it.hasNext()) {
        const QString src=it.next();
        const QString rel=QDir(from).relativeFilePath(src);
        if(rel==QStringLiteral("index.json")) continue;
        const QString dst=QDir(to).filePath(rel);
        if(QFile::exists(dst)) { if(!overwrite || QFileInfo(dst).size()==QFileInfo(src).size()) continue; QFile::remove(dst); }
        QDir().mkpath(QFileInfo(dst).path());
        if(QFile::copy(src,dst)) ++copied;
    }
}

// Archiv aus einem Ordner übernehmen (z. B. NVMEDATEN-Partition eines Sticks).
// Erkennt Archiv-Ordner mit index.json; sonst werden alle Firmwaredateien
// einzeln dem angegebenen Gerät (Modell) zugeordnet.
inline QVariantMap importFolder(const QString &folder, const QVariantMap &forDevice={}) {
    QString root=folder;
    for(const auto &cand:{folder,QDir(folder).filePath(QStringLiteral("firmware")),QDir(folder).filePath(QStringLiteral("nvme-recovery-studio/firmware"))})
        if(QFile::exists(QDir(cand).filePath(QStringLiteral("index.json")))) { root=cand; break; }
    int copied=0, added=0;
    QFile idx(QDir(root).filePath(QStringLiteral("index.json")));
    if(idx.open(QIODevice::ReadOnly)) {
        if(QDir(root).absolutePath()==QDir(dir()).absolutePath()) return {{"ok",true},{"message",QStringLiteral("Das ist bereits das eigene Archiv.")}};
        const auto foreign=QJsonDocument::fromJson(idx.readAll()).object().value("eintraege").toArray();
        copyTree(root,dir(),false,copied);
        auto mine=load();
        QSet<QString> known; for(const auto &e:mine) known.insert(e.toObject().value("datei").toString());
        for(const auto &e:foreign) if(!known.contains(e.toObject().value("datei").toString())) { mine.append(e); ++added; }
        save(mine);
        return {{"ok",true},{"message",QStringLiteral("Archiv übernommen: %1 neue Einträge, %2 Dateien kopiert.").arg(added).arg(copied)}};
    }
    const QString model=forDevice.value("nvmeControllerModel").toString();
    if(model.isEmpty()) return {{"ok",false},{"message",QStringLiteral("Kein Archiv im Ordner gefunden. Für lose Firmwaredateien zuerst die SSD auswählen, zu der sie gehören.")}};
    static const QStringList ext={QStringLiteral("bin"),QStringLiteral("fw"),QStringLiteral("img"),QStringLiteral("rom"),QStringLiteral("enc"),QStringLiteral("fluf"),QStringLiteral("iso"),QStringLiteral("ufw")};
    QDirIterator it(folder,QDir::Files,QDirIterator::Subdirectories);
    while(it.hasNext()) {
        const QFileInfo fi(it.next());
        if(!ext.contains(fi.suffix().toLower())) continue;
        const auto r=add(fi.absoluteFilePath(),{{"hersteller",forDevice.value("catalogVendor",QStringLiteral("Eigene Dateien")).toString()},
            {"modelle",QJsonArray{model}},{"version",fi.completeBaseName()},{"art",fi.suffix().toLower()==QStringLiteral("iso")?QStringLiteral("samsung-iso"):QStringLiteral("nvme-bild")},
            {"quelle",QStringLiteral("import:")+fi.fileName()},{"hinweis",QStringLiteral("Vom Benutzer übernommen")}});
        if(!r.isEmpty()) ++added;
    }
    return {{"ok",added>0},{"message",added ? QStringLiteral("%1 Firmwaredateien für %2 übernommen.").arg(added).arg(model)
                                            : QStringLiteral("Keine Firmwaredateien (.bin, .fw, .img, .fluf, .iso …) gefunden.")}};
}

// Eigene Daten (Firmware, Werkzeuge, KI-Wissen) in einen Ordner kopieren.
inline QVariantMap exportTo(const QString &target) {
    const QString base=QDir(target).filePath(QStringLiteral("nvme-recovery-studio"));
    int copied=0;
    copyTree(appDataDir(),base,true,copied);
    QFile::remove(QDir(base).filePath(QStringLiteral("firmware/index.json")));
    QFile::copy(indexPath(),QDir(base).filePath(QStringLiteral("firmware/index.json")));
    return {{"ok",true},{"dateien",copied},{"message",QStringLiteral("%1 Dateien nach %2 kopiert.").arg(copied).arg(QDir::toNativeSeparators(base))}};
}

// Datenpartition des Boot-Sticks finden (Bezeichnung NVMEDATEN).
inline QString findStickData() {
#ifndef Q_OS_WIN
    if(QFile::exists(QStringLiteral("/mnt/nvmedaten/nvme-recovery-studio"))) return QStringLiteral("/mnt/nvmedaten");
#endif
    for(const auto &v:QStorageInfo::mountedVolumes())
        if(v.isValid() && v.isReady() && (v.name().compare(QStringLiteral("NVMEDATEN"),Qt::CaseInsensitive)==0
           || QFile::exists(QDir(v.rootPath()).filePath(QStringLiteral("nvme-recovery-studio/firmware/index.json")))))
            return v.rootPath();
    return {};
}

// Beim Start: Daten vom Stick übernehmen (fehlende Dateien, Wissen nur wenn noch keins da ist).
inline QString seedFromStick() {
    const QString root=findStickData();
    if(root.isEmpty()) return {};
    const QString base=QDir(root).filePath(QStringLiteral("nvme-recovery-studio"));
    if(!QDir(base).exists()) return {};
    int copied=0;
    copyTree(base,appDataDir(),false,copied);
    importFolder(QDir(base).filePath(QStringLiteral("firmware")));
    return copied ? QStringLiteral("Vom Boot-Stick übernommen: %1 Dateien (Firmware-Archiv, Werkzeuge, KI-Wissen).").arg(copied) : QString();
}

// Samsung liefert Firmware als bootfähiges ISO. Die Linux-Edition packt es aus
// und startet Samsungs eigenes Update-Programm (fumagician) direkt.
inline QVariantMap runSamsungIso(const QString &iso, const std::function<void(const QString&)> &log) {
#ifdef Q_OS_WIN
    Q_UNUSED(iso); Q_UNUSED(log);
    return {{"ok",false},{"message",QStringLiteral("Samsung-Firmware-ISOs laufen nur in der Linux-Boot-Edition.")}};
#else
    QTemporaryDir tmp;
    if(!tmp.isValid()) return {{"ok",false},{"message",QStringLiteral("Kein temporärer Ordner")}};
    auto sh=[&](const QString &cmd,int timeout=120000)->QString {
        QProcess p; p.setProcessChannelMode(QProcess::MergedChannels); p.setWorkingDirectory(tmp.path());
        p.start(QStringLiteral("/bin/sh"),{QStringLiteral("-c"),cmd}); p.waitForFinished(timeout);
        return QString::fromLocal8Bit(p.readAll());
    };
    log(QStringLiteral("Samsung-ISO wird ausgepackt …"));
    sh(QStringLiteral("bsdtar -xf '%1' && for i in $(find . -iname 'initrd*'); do mkdir -p ird && (cd ird && (gzip -dc ../$i 2>/dev/null || xz -dc ../$i 2>/dev/null || cat ../$i) | cpio -idm 2>/dev/null); done").arg(QString(iso).replace(QLatin1Char('\''),QStringLiteral("'\\''"))));
    QString tool;
    QDirIterator it(tmp.path(),{QStringLiteral("fumagician")},QDir::Files,QDirIterator::Subdirectories);
    if(it.hasNext()) tool=it.next();
    if(tool.isEmpty()) return {{"ok",false},{"message",QStringLiteral("Im ISO wurde kein Samsung-Update-Programm (fumagician) gefunden.")}};
    QFile::setPermissions(tool,QFile::permissions(tool)|QFileDevice::ExeOwner);
    log(QStringLiteral("Starte Samsung-Firmware-Update (fumagician) – Stromversorgung nicht unterbrechen."));
    QProcess p; p.setProcessChannelMode(QProcess::MergedChannels); p.setWorkingDirectory(QFileInfo(tool).path());
    p.start(tool,{});
    if(!p.waitForStarted()) return {{"ok",false},{"message",QStringLiteral("fumagician ließ sich nicht starten.")}};
    QString all;
    while(p.state()!=QProcess::NotRunning) {
        p.waitForReadyRead(1000);
        const QString chunk=QString::fromLocal8Bit(p.readAll());
        if(chunk.isEmpty()) continue;
        all+=chunk;
        for(const auto &l:chunk.split(QLatin1Char('\n'),Qt::SkipEmptyParts)) log(QStringLiteral("Samsung: ")+l.trimmed());
        if(chunk.contains(QRegularExpression(QStringLiteral("\\[?[Yy]/[Nn]\\]?|\\(y/n\\)|continue\\?"),QRegularExpression::CaseInsensitiveOption))) p.write("y\n");
    }
    const bool ok=all.contains(QStringLiteral("success"),Qt::CaseInsensitive) || all.contains(QStringLiteral("completed"),Qt::CaseInsensitive);
    return {{"ok",ok},{"message",ok ? QStringLiteral("Samsung-Firmware übertragen. Rechner ausschalten (nicht nur neu starten), damit die neue Firmware aktiv wird.")
                                    : QStringLiteral("Samsung-Update ohne Erfolgsmeldung beendet – Ausgabe im Verlauf prüfen.")}};
#endif
}

} // namespace FirmwareArchive
