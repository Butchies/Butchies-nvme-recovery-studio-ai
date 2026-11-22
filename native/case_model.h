#pragma once
// Small learning AI of the recovery agent (no language model).
//
// 1) Cause classifier: multinomial logistic regression over ~30 features
//    from SMART, error log, connection, controller family and history.
//    Starts from rule-derived knowledge (prior weights) and learns online
//    from every confirmed case (user confirmation or rescue outcome).
//    Explains each prediction by its strongest feature contributions.
// 2) Rescue tuner: nearest-neighbour search over earlier rescues; reuses
//    the settings of the most similar successful one.
// Model file: <AppData>/ki-modell.json (a few KB).
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <cmath>
#include <vector>
#include <algorithm>

namespace CaseModel {

inline const QStringList &causes() {
    static const QStringList c{
        QStringLiteral("gesund"), QStringLiteral("flash_verschleiss"), QStringLiteral("firmware_fehler"),
        QStringLiteral("bridge_problem"), QStringLiteral("notmodus"), QStringLiteral("controller_ausfall")};
    return c;
}
inline QString causeText(const QString &c) {
    if(c==QStringLiteral("gesund")) return QStringLiteral("kein Defekt erkennbar");
    if(c==QStringLiteral("flash_verschleiss")) return QStringLiteral("Flash-Verschleiß / defekte Speicherbereiche");
    if(c==QStringLiteral("firmware_fehler")) return QStringLiteral("Firmwarefehler");
    if(c==QStringLiteral("bridge_problem")) return QStringLiteral("Problem mit USB-Gehäuse/Bridge oder Verbindung");
    if(c==QStringLiteral("notmodus")) return QStringLiteral("Controller im Notmodus");
    return QStringLiteral("Controller antwortet nicht stabil / Elektronikdefekt");
}

inline const QStringList &featureNames() {
    static const QStringList f{
        QStringLiteral("Bias"), QStringLiteral("Identify fehlt"), QStringLiteral("SMART fehlt"), QStringLiteral("Kein Medium"),
        QStringLiteral("PCIe ohne NVMe"), QStringLiteral("Warnbit Reserve"), QStringLiteral("Warnbit Temperatur"),
        QStringLiteral("Warnbit Zuverlässigkeit"), QStringLiteral("Warnbit Nur-Lesen"), QStringLiteral("Reserve niedrig"),
        QStringLiteral("Verschleiß hoch"), QStringLiteral("Medienfehler"), QStringLiteral("Fehlerprotokoll"),
        QStringLiteral("Temperatur hoch"), QStringLiteral("Bekannter Firmwarefehler"), QStringLiteral("Medienfehler steigen"),
        QStringLiteral("Bridge: Zeitüberschreitungen"), QStringLiteral("Bridge: Verbindungsabbrüche"),
        QStringLiteral("Bridge reicht NVMe sonst durch"), QStringLiteral("Letzte Rettung: defekte Sektoren"),
        QStringLiteral("Letzte Rettung: Zeitüberschreitungen"), QStringLiteral("Letzte Rettung unterbrochen"),
        QStringLiteral("USB-Anschluss"), QStringLiteral("PCIe-Anschluss"),
        QStringLiteral("Familie Phison"), QStringLiteral("Familie Silicon Motion"), QStringLiteral("Familie Samsung"),
        QStringLiteral("Familie WD/SanDisk"), QStringLiteral("Familie Realtek/Maxio"), QStringLiteral("Familie andere"),
        QStringLiteral("NVMe-Fehler nicht wiederholbar")};
    return f;
}

inline double num(const QVariant &v) { bool ok=false; const double d=v.toString().toDouble(&ok); return ok ? d : 0.0; }
inline double squash(double v, double scale) { return v<=0 ? 0.0 : std::log1p(v)/std::log1p(scale); }

// Feature vector from device data, diagnosis and experience.
inline std::vector<double> features(const QVariantMap &d, const QVariantMap &report, const QVariantMap &exp) {
    std::vector<double> x(size_t(featureNames().size()),0.0);
    int i=0;
    const auto set=[&](double v){ x[size_t(i++)]=std::clamp(v,0.0,1.0); };
    const bool smart=d.value("smartAvailable").toBool();
    const unsigned warn=d.value("criticalWarning").toString().toUInt(nullptr,16);
    const auto prevSmart=exp.value("device").toMap().value("letzterSmart").toMap();
    const auto bridge=exp.value("bridge").toMap();
    const auto last=exp.value("device").toMap().value("letzteRettung").toMap();
    const QString fam=report.value("catalog").toMap().value("family").toMap().value("id").toString();
    const QString size=d.value("sizeBytes").toString().trimmed();
    bool dnr=false;
    for(const auto &e:d.value("recentNvmeErrors").toList()) if(e.toMap().value("doNotRetry").toBool()) dnr=true;
    set(1.0);
    set(d.value("nvmeIdentifyAvailable").toBool() ? 0 : 1);
    set(smart ? 0 : 1);
    set(size.isEmpty() || size==QStringLiteral("0") ? 1 : 0);
    set(d.value("romModeSuspect").toBool() ? 1 : 0);
    set(smart && (warn&0x01) ? 1 : 0);
    set(smart && (warn&0x02) ? 1 : 0);
    set(smart && (warn&0x04) ? 1 : 0);
    set(smart && (warn&0x08) ? 1 : 0);
    set(smart ? (100.0-num(d.value("availableSparePercent")))/100.0 : 0);
    set(smart ? num(d.value("percentageUsed"))/100.0 : 0);
    set(smart ? squash(num(d.value("mediaErrors")),1000) : 0);
    set(smart ? squash(num(d.value("errorLogEntries")),5000) : 0);
    set(smart ? (num(d.value("temperatureCelsius"))-50.0)/30.0 : 0);
    set(report.value("firmwareIssues").toList().isEmpty() ? 0 : 1);
    set(!prevSmart.isEmpty() && num(d.value("mediaErrors"))>num(prevSmart.value("medienfehler")) ? 1 : 0);
    set(squash(bridge.value("zeitueberschreitungen").toDouble(),50));
    set(squash(bridge.value("verbindungsverluste").toDouble(),10));
    set(bridge.value("identifyOk").toInt()>0 && !d.value("nvmeIdentifyAvailable").toBool() ? 1 : 0);
    set(squash(last.value("defektBereiche").toDouble(),200));
    set(squash(last.value("zeitueberschreitungen").toDouble(),20));
    set(last.value("status").toString()==QStringLiteral("unterbrochen") ? 1 : 0);
    set(d.value("connection").toString()==QStringLiteral("USB") ? 1 : 0);
    set(d.value("connection").toString()==QStringLiteral("PCIe") ? 1 : 0);
    set(fam==QStringLiteral("phison") ? 1 : 0);
    set(fam==QStringLiteral("siliconmotion") ? 1 : 0);
    set(fam==QStringLiteral("samsung") ? 1 : 0);
    set(fam==QStringLiteral("wd_sandisk") ? 1 : 0);
    set(fam==QStringLiteral("realtek") || fam==QStringLiteral("maxio") ? 1 : 0);
    set(fam.isEmpty() || (fam!=QStringLiteral("phison") && fam!=QStringLiteral("siliconmotion") && fam!=QStringLiteral("samsung")
        && fam!=QStringLiteral("wd_sandisk") && fam!=QStringLiteral("realtek") && fam!=QStringLiteral("maxio")) ? 1 : 0);
    set(dnr ? 1 : 0);
    return x;
}

class Model {
public:
    Model() {
        const QString dir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_path=QDir(dir).filePath(QStringLiteral("ki-modell.json"));
        const int nc=int(causes().size()), nf=int(featureNames().size());
        m_w.assign(size_t(nc),std::vector<double>(size_t(nf),0.0));
        seedPriors();
        QFile f(m_path);
        if(f.open(QIODevice::ReadOnly)) {
            const auto o=QJsonDocument::fromJson(f.readAll()).object();
            if(o.value("features").toInt()==nf && o.value("klassen").toInt()==nc) {
                const auto w=o.value("gewichte").toArray();
                for(int c=0;c<nc && c<w.size();++c) for(int j=0;j<nf;++j) m_w[size_t(c)][size_t(j)]=w[c].toArray().at(j).toDouble();
                m_trained=o.value("trainiert").toInt();
                m_rescues=o.value("rettungen").toArray();
            }
        }
    }
    int trainedCases() const { return m_trained; }
    int rescueCases() const { return int(m_rescues.size()); }

    // Probabilities per cause, sorted, with explanation of the top cause.
    QVariantMap predict(const std::vector<double> &x) const {
        const auto p=softmax(x);
        std::vector<int> idx(p.size()); for(size_t i=0;i<idx.size();++i) idx[i]=int(i);
        std::sort(idx.begin(),idx.end(),[&](int a,int b){ return p[size_t(a)]>p[size_t(b)]; });
        QVariantList ranking;
        for(int c:idx) ranking.append(QVariantMap{{"ursache",causes().at(c)},{"text",causeText(causes().at(c))},{"wahrscheinlichkeit",p[size_t(c)]}});
        const int top=idx.front();
        std::vector<std::pair<double,int>> contrib;
        for(size_t j=1;j<x.size();++j) if(x[j]>0) contrib.push_back({m_w[size_t(top)][j]*x[j],int(j)});
        std::sort(contrib.begin(),contrib.end(),[](auto a,auto b){ return a.first>b.first; });
        QStringList why;
        for(size_t k=0;k<contrib.size() && k<3;++k) if(contrib[k].first>0.05) why<<featureNames().at(contrib[k].second);
        return {{"ursache",causes().at(top)},{"text",causeText(causes().at(top))},{"wahrscheinlichkeit",p[size_t(top)]},
                {"gruende",why},{"rangliste",ranking},{"trainiert",m_trained}};
    }
    // Online learning step (stochastic gradient descent, L2).
    void learn(const std::vector<double> &x, const QString &cause, double weight=1.0) {
        const int y=int(causes().indexOf(cause));
        if(y<0) return;
        const double lr=0.3*weight/std::sqrt(1.0+m_trained*0.05);
        const auto p=softmax(x);
        for(size_t c=0;c<m_w.size();++c)
            for(size_t j=0;j<x.size();++j)
                m_w[c][j]-=lr*((p[c]-(int(c)==y ? 1.0 : 0.0))*x[j]+0.001*m_w[c][j]);
        ++m_trained;
        save();
    }
    // Remember a rescue outcome for the rescue tuner.
    void rememberRescue(const std::vector<double> &x, const QVariantMap &plan, const QVariantMap &result) {
        const double score=result.value("rescuedPercent").toDouble()/100.0
            -0.02*result.value("timeouts").toInt()-0.05*result.value("deviceLost").toInt()
            +(result.value("state").toString()==QStringLiteral("abgeschlossen") ? 0.2 : 0.0);
        QJsonArray xs; for(double v:x) xs.append(v);
        m_rescues.append(QJsonObject{{"x",xs},{"score",score},{"blockBytes",plan.value("chunkBytes").toLongLong()},
            {"zeitlimitMs",plan.value("readTimeoutMs").toInt()},{"wiederholungen",plan.value("retryPasses").toInt()},
            {"maxTimeouts",plan.value("maxConsecutiveTimeouts").toInt()},{"zeit",QDateTime::currentDateTime().toString(Qt::ISODate)}});
        while(m_rescues.size()>500) m_rescues.removeFirst();
        save();
    }
    // Settings of the most similar successful rescue (cosine similarity).
    QVariantMap suggestRescue(const std::vector<double> &x) const {
        double best=0; QJsonObject hit;
        for(const auto &r:m_rescues) {
            const auto o=r.toObject();
            if(o.value("score").toDouble()<0.9) continue;
            const auto xs=o.value("x").toArray();
            double dot=0,na=0,nb=0;
            for(size_t j=1;j<x.size() && int(j)<xs.size();++j) { const double b=xs[int(j)].toDouble(); dot+=x[j]*b; na+=x[j]*x[j]; nb+=b*b; }
            const double sim=(na>0 && nb>0) ? dot/std::sqrt(na*nb) : 0;
            if(sim>best) { best=sim; hit=o; }
        }
        if(best<0.85) return {};
        return {{"aehnlichkeit",best},{"chunkBytes",hit.value("blockBytes").toVariant()},{"readTimeoutMs",hit.value("zeitlimitMs").toInt()},
                {"retryPasses",hit.value("wiederholungen").toInt()},{"maxConsecutiveTimeouts",hit.value("maxTimeouts").toInt()},
                {"zeit",hit.value("zeit").toString()}};
    }
    // Cause derived from a rescue outcome (weak label).
    static QString causeFromRescue(const QVariantMap &result, const QVariantMap &report) {
        if(result.value("state").toString()==QStringLiteral("fehler")) return {};
        if(!report.value("firmwareIssues").toList().isEmpty()) return QStringLiteral("firmware_fehler");
        if(result.value("deviceLost").toInt()>0) return QStringLiteral("bridge_problem");
        if(result.value("badBytes").toULongLong()>0) return QStringLiteral("flash_verschleiss");
        if(result.value("timeouts").toInt()>2) return QStringLiteral("controller_ausfall");
        if(result.value("state").toString()==QStringLiteral("abgeschlossen")) return QStringLiteral("gesund");
        return {};
    }
private:
    std::vector<double> softmax(const std::vector<double> &x) const {
        std::vector<double> z(m_w.size(),0.0);
        for(size_t c=0;c<m_w.size();++c) for(size_t j=0;j<x.size();++j) z[c]+=m_w[c][j]*x[j];
        const double m=*std::max_element(z.begin(),z.end());
        double s=0; for(auto &v:z) { v=std::exp(v-m); s+=v; }
        for(auto &v:z) v/=s;
        return z;
    }
    void prior(const QString &cause, int feature, double w) { m_w[size_t(causes().indexOf(cause))][size_t(feature)]=w; }
    // Initial knowledge = today's rules, so the model is useful from case 1.
    void seedPriors() {
        prior(QStringLiteral("gesund"),0,1.5);
        prior(QStringLiteral("notmodus"),3,2.0); prior(QStringLiteral("notmodus"),4,4.0);
        prior(QStringLiteral("controller_ausfall"),1,1.5); prior(QStringLiteral("controller_ausfall"),3,1.5);
        prior(QStringLiteral("controller_ausfall"),20,2.0); prior(QStringLiteral("controller_ausfall"),30,1.5);
        prior(QStringLiteral("flash_verschleiss"),5,3.0); prior(QStringLiteral("flash_verschleiss"),7,2.5);
        prior(QStringLiteral("flash_verschleiss"),8,3.0); prior(QStringLiteral("flash_verschleiss"),9,3.0);
        prior(QStringLiteral("flash_verschleiss"),10,2.5); prior(QStringLiteral("flash_verschleiss"),11,4.0);
        prior(QStringLiteral("flash_verschleiss"),15,3.0); prior(QStringLiteral("flash_verschleiss"),19,3.0);
        prior(QStringLiteral("firmware_fehler"),14,5.0); prior(QStringLiteral("firmware_fehler"),12,1.0);
        prior(QStringLiteral("bridge_problem"),16,2.5); prior(QStringLiteral("bridge_problem"),17,3.0);
        prior(QStringLiteral("bridge_problem"),2,1.0); prior(QStringLiteral("bridge_problem"),22,0.5);
        prior(QStringLiteral("controller_ausfall"),18,2.5);
        prior(QStringLiteral("gesund"),6,-1.0); prior(QStringLiteral("gesund"),11,-3.0); prior(QStringLiteral("gesund"),3,-3.0);
    }
    void save() const {
        QJsonArray w;
        for(const auto &row:m_w) { QJsonArray r; for(double v:row) r.append(v); w.append(r); }
        QSaveFile f(m_path);
        if(f.open(QIODevice::WriteOnly)) {
            f.write(QJsonDocument(QJsonObject{{"features",int(featureNames().size())},{"klassen",int(causes().size())},
                {"gewichte",w},{"trainiert",m_trained},{"rettungen",m_rescues}}).toJson(QJsonDocument::Compact));
            f.commit();
        }
    }
    QString m_path;
    std::vector<std::vector<double>> m_w;
    int m_trained=0;
    QJsonArray m_rescues;
};

} // namespace CaseModel
