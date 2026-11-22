#!/usr/bin/env python3
"""Vortraining des KI-Ursachenmodells mit typischen Fehlerbildern.

Erzeugt synthetische, aber an dokumentierten Fehlerbildern orientierte Fälle
(SMART-Grenzwerte, NVMe-Statuscodes, Kernelmeldungen, bekannte Firmwarefehler,
Bridge-Verhalten, Notmodus) und trainiert daraus die logistische Regression,
die native/case_model.h verwendet. Ergebnis: data/catalog/ki-startmodell.json
(Startwissen; echte Fälle des Nutzers verfeinern es danach weiter).
Merkmal- und Klassenreihenfolge MUSS case_model.h entsprechen.
"""
import json, numpy as np, pathlib, sys
rng=np.random.default_rng(20261122)
F=["Bias","Identify fehlt","SMART fehlt","Kein Medium","PCIe ohne NVMe","Warnbit Reserve","Warnbit Temperatur",
"Warnbit Zuverlässigkeit","Warnbit Nur-Lesen","Reserve niedrig","Verschleiß hoch","Medienfehler","Fehlerprotokoll",
"Temperatur hoch","Bekannter Firmwarefehler","Medienfehler steigen","Bridge: Zeitüberschreitungen","Bridge: Verbindungsabbrüche",
"Bridge reicht NVMe sonst durch","Letzte Rettung: defekte Sektoren","Letzte Rettung: Zeitüberschreitungen","Letzte Rettung unterbrochen",
"USB-Anschluss","PCIe-Anschluss","Familie Phison","Familie Silicon Motion","Familie Samsung","Familie WD/SanDisk",
"Familie Realtek/Maxio","Familie andere","NVMe-Fehler nicht wiederholbar","Selbsttest fehlgeschlagen","Cache nicht wirksam","Antwortzeiten instabil"]
C=["gesund","flash_verschleiss","firmware_fehler","bridge_problem","notmodus","controller_ausfall","cache_defekt"]
I={n:i for i,n in enumerate(F)}
def sq(v,s): return 0.0 if v<=0 else np.log1p(v)/np.log1p(s)
def base(usb):
    x=np.zeros(len(F)); x[0]=1
    x[I["USB-Anschluss"]]=usb; x[I["PCIe-Anschluss"]]=1-usb
    fam=rng.choice(["Familie Phison","Familie Silicon Motion","Familie Samsung","Familie WD/SanDisk","Familie Realtek/Maxio","Familie andere"],p=[.3,.2,.2,.12,.1,.08])
    x[I[fam]]=1
    x[I["Reserve niedrig"]]=rng.uniform(0,.05); x[I["Verschleiß hoch"]]=rng.uniform(0,.3)
    x[I["Temperatur hoch"]]=max(0,rng.normal(-.3,.2)); x[I["Fehlerprotokoll"]]=sq(rng.integers(0,30),5000)
    return x
def smart_missing(x,usb):
    if usb and rng.random()<.35:  # bridge without passthrough: no NVMe data at all
        for k in ["Reserve niedrig","Verschleiß hoch","Temperatur hoch","Fehlerprotokoll"]: x[I[k]]=0
        x[I["Identify fehlt"]]=1; x[I["SMART fehlt"]]=1
def case(c):
    usb=int(rng.random()<.5); x=base(usb)
    if c=="gesund":
        smart_missing(x,usb)
    elif c=="flash_verschleiss":
        x[I["Medienfehler"]]=sq(rng.integers(5,5000),1000); x[I["Reserve niedrig"]]=rng.uniform(.1,.95)
        x[I["Verschleiß hoch"]]=rng.uniform(.3,1.0); x[I["Fehlerprotokoll"]]=sq(rng.integers(50,20000),5000)
        x[I["Warnbit Reserve"]]=rng.random()<.5; x[I["Warnbit Zuverlässigkeit"]]=rng.random()<.6; x[I["Warnbit Nur-Lesen"]]=rng.random()<.25
        x[I["Medienfehler steigen"]]=rng.random()<.6; x[I["Letzte Rettung: defekte Sektoren"]]=sq(rng.integers(0,300),200)*(rng.random()<.5)
        x[I["NVMe-Fehler nicht wiederholbar"]]=rng.random()<.6; x[I["Selbsttest fehlgeschlagen"]]=rng.random()<.35
        x[I["Letzte Rettung: Zeitüberschreitungen"]]=sq(rng.integers(0,6),20)*(rng.random()<.3)
    elif c=="firmware_fehler":
        x[I["Bekannter Firmwarefehler"]]=rng.random()<.7
        x[I["Fehlerprotokoll"]]=sq(rng.integers(100,60000),5000)
        x[I["Verschleiß hoch"]]=rng.uniform(0,.6) if rng.random()<.5 else x[I["Verschleiß hoch"]]
        x[I["Warnbit Zuverlässigkeit"]]=rng.random()<.2; x[I["Letzte Rettung unterbrochen"]]=rng.random()<.2
        if not x[I["Bekannter Firmwarefehler"]]: x[I["Antwortzeiten instabil"]]=rng.random()<.4; x[I["Fehlerprotokoll"]]=sq(rng.integers(2000,80000),5000)
        if x[I["Familie Samsung"]]==0 and x[I["Familie WD/SanDisk"]]==0 and rng.random()<.5: x[I["Bekannter Firmwarefehler"]]=0
    elif c=="bridge_problem":
        usb=1; x[I["USB-Anschluss"]]=1; x[I["PCIe-Anschluss"]]=0
        x[I["Bridge: Zeitüberschreitungen"]]=sq(rng.integers(3,60),50); x[I["Bridge: Verbindungsabbrüche"]]=sq(rng.integers(1,12),10)
        x[I["Letzte Rettung unterbrochen"]]=rng.random()<.7; x[I["Letzte Rettung: Zeitüberschreitungen"]]=sq(rng.integers(0,8),20)
        if rng.random()<.4: x[I["Identify fehlt"]]=1; x[I["SMART fehlt"]]=1
    elif c=="notmodus":
        x[I["Identify fehlt"]]=1; x[I["SMART fehlt"]]=1; x[I["Kein Medium"]]=1
        for k in ["Reserve niedrig","Verschleiß hoch","Temperatur hoch","Fehlerprotokoll"]: x[I[k]]=0
        if not usb: x[I["PCIe ohne NVMe"]]=rng.random()<.9
    elif c=="controller_ausfall":
        mode=rng.integers(0,3)
        if mode==0:  # verschwindet / hängt
            x[I["Letzte Rettung: Zeitüberschreitungen"]]=sq(rng.integers(3,20),20); x[I["Letzte Rettung unterbrochen"]]=1
            x[I["Antwortzeiten instabil"]]=rng.random()<.6
        elif mode==1:  # antwortet nicht mehr
            x[I["Identify fehlt"]]=1; x[I["SMART fehlt"]]=1
            for k in ["Reserve niedrig","Verschleiß hoch","Temperatur hoch","Fehlerprotokoll"]: x[I[k]]=0
            if usb: x[I["Bridge reicht NVMe sonst durch"]]=1
            else: x[I["Kein Medium"]]=rng.random()<.5
        else:
            x[I["Selbsttest fehlgeschlagen"]]=1; x[I["Warnbit Zuverlässigkeit"]]=rng.random()<.5; x[I["Temperatur hoch"]]=rng.uniform(.3,1)
            x[I["Warnbit Temperatur"]]=rng.random()<.4
    elif c=="cache_defekt":
        x[I["Cache nicht wirksam"]]=rng.random()<.85; x[I["Antwortzeiten instabil"]]=rng.random()<.6
        x[I["Selbsttest fehlgeschlagen"]]=rng.random()<.3; x[I["Fehlerprotokoll"]]=sq(rng.integers(10,3000),5000)
        if not x[I["Cache nicht wirksam"]]: x[I["Antwortzeiten instabil"]]=1
    return np.clip(x.astype(float),0,1)
N=4000
X=[];Y=[]
for ci,c in enumerate(C):
    for _ in range(N): X.append(case(c)); Y.append(ci)
X=np.array(X); Y=np.array(Y)
idx=rng.permutation(len(Y)); X,Y=X[idx],Y[idx]
cut=int(.85*len(Y)); Xt,Yt,Xv,Yv=X[:cut],Y[:cut],X[cut:],Y[cut:]
W=np.zeros((len(C),len(F)))
oh=np.eye(len(C))[Yt]
for ep in range(800):
    z=Xt@W.T; z-=z.max(1,keepdims=True); p=np.exp(z); p/=p.sum(1,keepdims=True)
    g=(p-oh).T@Xt/len(Yt)+1e-3*W
    W-=1.0*g
pred=(Xv@W.T).argmax(1); acc=(pred==Yv).mean()
conf=np.zeros((len(C),len(C)),int)
for a,b in zip(Yv,pred): conf[a,b]+=1
print(f"Genauigkeit auf Testfällen: {acc*100:.1f} %")
for i,c in enumerate(C): print(f"  {c:20s} richtig {conf[i,i]/conf[i].sum()*100:5.1f} %")
out={"features":len(F),"klassen":len(C),"gewichte":W.round(5).tolist(),"trainiert":0,"rettungen":[],
     "vortraining":{"faelle":int(len(Y)),"genauigkeitTest":round(float(acc),4),"quelle":"tools/pretrain_model.py – synthetische Fälle aus dokumentierten Fehlerbildern"}}
p=pathlib.Path(__file__).resolve().parent.parent/"data"/"catalog"/"ki-startmodell.json"
p.write_text(json.dumps(out,ensure_ascii=False))
print("geschrieben:",p)
