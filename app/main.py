"""NVMe Recovery Studio AI - read-only desktop application."""
import json, platform, queue, re, sqlite3, subprocess, sys, threading, tkinter as tk
from datetime import datetime
from pathlib import Path
from tkinter import filedialog, messagebox, ttk
import cases
import diagnostics
import ata_diagnostics
import ata_analysis
import analysis
import exporter
import safety
import firmware
import firmware_catalog
import triage
import imaging_plan
import controller_identity

def cmd(args):
    try:
        p=subprocess.run(args,capture_output=True,text=True,timeout=12,creationflags=getattr(subprocess,"CREATE_NO_WINDOW",0))
        return p.stdout if p.returncode==0 else ""
    except (OSError,subprocess.TimeoutExpired): return ""

def discover():
    if sys.platform=="win32":
        script="$ErrorActionPreference='SilentlyContinue'; Get-CimInstance Win32_DiskDrive | Where-Object { $_.InterfaceType -eq 'USB' -or $_.PNPDeviceID -match '^USBSTOR|^USB\\' } | Select-Object Model,SerialNumber,Size,DeviceID,InterfaceType | ConvertTo-Json -Compress"
        try: data=json.loads(cmd(["powershell.exe","-NoProfile","-NonInteractive","-Command",script]))
        except (ValueError,TypeError): return []
        if isinstance(data,dict): data=[data]
        if not isinstance(data,list):return []
        out=[]
        prefix=chr(92)*2+"."+chr(92)+"PHYSICALDRIVE"
        for x in data:
            if not isinstance(x,dict):continue
            path=str(x.get("DeviceID") or "")
            if not path.upper().startswith(prefix) or not path[len(prefix):].isdigit():continue
            try: size=int(x.get("Size") or 0)
            except (ValueError,TypeError,OverflowError):continue
            if size<=0:continue
            out.append({"name":str(x.get("Model") or "USB-Laufwerk"),
                        "path":path,"serial":str(x.get("SerialNumber") or ""),
                        "size":size,"bus":"USB","nvme":False,
                        "connection":"extern (USB)","m2_form_factor_verified":False})
        return out
    if sys.platform.startswith("linux"):
        try: data=json.loads(cmd(["lsblk","-J","-b","-d","-o","NAME,MODEL,SERIAL,SIZE,TRAN,TYPE,PATH,ROTA"]))
        except (ValueError,TypeError): return []
        if not isinstance(data,dict) or not isinstance(data.get("blockdevices"),list):return []
        out=[]
        for x in data["blockdevices"]:
            if not isinstance(x,dict) or x.get("type")!="disk":continue
            name=str(x.get("name") or "")
            transport=str(x.get("tran") or "").lower()
            is_nvme=bool(re.fullmatch(r"nvme[0-9]+n[0-9]+",name))
            is_usb=transport=="usb"
            bus="PCIe/NVMe" if is_nvme and not is_usb else "USB" if is_usb else transport.upper() if transport else "Unbekannt"
            try: size=int(x.get("size") or 0)
            except (ValueError,TypeError,OverflowError):continue
            path=str(x.get("path") or "/dev/"+name)
            if size<=0 or not re.fullmatch(r"/dev/(?:nvme[0-9]+n[0-9]+|sd[a-z]+|hd[a-z]+|vd[a-z]+|xvd[a-z]+|mmcblk[0-9]+)",path):
                continue
            # SATA M.2 and SATA 2.5-inch drives are not reliably distinguishable via lsblk.
            out.append({"name":str(x.get("model") or name),"path":path,
                        "serial":str(x.get("serial") or ""),"size":size,
                        "bus":bus,"nvme":bool(is_nvme and not is_usb),
                        "m2_form_factor_verified":False,
                        "connection":"extern (USB)" if is_usb else "intern/direkt oder nicht eindeutig"})
        return out
    return []

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("NVMe Recovery Studio AI");self.geometry("1050x670");self.minsize(760,500)
        self.configure(bg="#101722");self.devices=[];self.selected=None
        s=ttk.Style();s.theme_use("clam");s.configure("Treeview",background="#192333",fieldbackground="#192333",foreground="#e8eff8",rowheight=31);s.map("Treeview",background=[("selected","#20556b")]);s.configure("Treeview.Heading",background="#26364a",foreground="white")
        top=tk.Frame(self,bg="#131e2c",padx=22,pady=20);top.pack(fill="x")
        self.label(top,"NVMe Recovery Studio AI",22,True).pack(anchor="w")
        self.label(top,("Windows 10/11 | Nur USB-Geräte" if sys.platform=="win32" else "Linux | Alle physischen Laufwerke (intern und extern)") +" | Nur lesende Inventarisierung",11).pack(anchor="w",pady=(6,0))
        bar=tk.Frame(self,bg="#101722",padx=22,pady=12);bar.pack(fill="x")
        buttons=tk.Frame(bar,bg="#101722");buttons.pack(fill="x")
        actions=[("Geräte aktualisieren",self.refresh),("Rechneranalyse",self.host_info),("SSD-Diagnose",self.diagnose),("NVMe-Controller",self.controller_info),("USB-Verbindung",self.usb_diagnose),("Schutzprüfung",self.safety_check),("Firmwaredatei prüfen",self.inspect_firmware),("Firmwarekatalog",self.catalog_lookup),("Recovery-Leitfaden",self.triage_plan),("Abbild planen",self.plan_image),("Befund",self.assess),("Vergleich",self.compare_case),("Fallakte speichern",self.save_case),("Fallakten",self.open_cases),("Fallakte exportieren",self.export)]
        for i,(name,cb) in enumerate(actions):
            tk.Button(buttons,text=name,command=cb,bg="#22566c",fg="white",relief="flat",padx=9,pady=7).grid(row=i//5,column=i%5,sticky="ew",padx=4,pady=4)
        for i in range(5): buttons.grid_columnconfigure(i,weight=1)
        main=tk.PanedWindow(self,orient="horizontal",bg="#101722",sashwidth=6,bd=0);main.pack(fill="both",expand=True,padx=22,pady=12)
        left=tk.Frame(main,bg="#192333",padx=12,pady=12);right=tk.Frame(main,bg="#192333",padx=12,pady=12);main.add(left,minsize=350);main.add(right,minsize=280)
        self.label(left,"Erkannte Datenträger",15,True).pack(anchor="w")
        self.tree=ttk.Treeview(left,columns=("bus","connection","size"),show="tree headings");self.tree.heading("#0",text="Gerät");self.tree.heading("bus",text="Bus");self.tree.heading("connection",text="Verbindung");self.tree.heading("size",text="Größe");self.tree.column("#0",width=210);self.tree.column("bus",width=85);self.tree.column("connection",width=150);self.tree.column("size",width=100);self.tree.pack(fill="both",expand=True,pady=12);self.tree.bind("<<TreeviewSelect>>",self.choose)
        self.label(right,"Gerätedetails",15,True).pack(anchor="w")
        self.detail=tk.Text(right,bg="#192333",fg="#e8eff8",font=("Consolas",11),relief="flat",wrap="word",state="disabled");self.detail.pack(fill="both",expand=True,pady=12)
        self.status=self.label(self,"Keine Schreib-, Lösch- oder Firmwarefunktionen",10);self.status.pack(anchor="w",padx=22,pady=12)
        self.last_report=None
        self.case_for_compare=None
        self.inventory_epoch=0
        self.refresh_request_id=0
        self.ui_queue=queue.Queue()
        self.after(75,self.process_ui_queue)
        self.refresh()
    def post_ui(self,callback):
        """Workers enqueue callbacks; only the Tk main thread touches widgets."""
        self.ui_queue.put(callback)
    def process_ui_queue(self):
        for _ in range(50):
            try: callback=self.ui_queue.get_nowait()
            except queue.Empty: break
            try: callback()
            except Exception as exc:
                self.status.config(text=f"UI-Rückmeldung fehlgeschlagen: {type(exc).__name__}")
        self.after(75,self.process_ui_queue)
    def label(self,parent,text,size,bold=False):
        return tk.Label(parent,text=text,bg=parent["bg"],fg="#e8eff8",font=("Segoe UI",size,"bold" if bold else "normal"))
    def show(self,text):
        self.detail.config(state="normal");self.detail.delete("1.0","end");self.detail.insert("1.0",text);self.detail.config(state="disabled")
    def refresh(self):
        self.refresh_request_id+=1
        request_id=self.refresh_request_id
        self.inventory_epoch+=1
        self.status.config(text="Geräte werden erfasst ...")
        def worker():
            devices=discover()
            self.post_ui(lambda:self.loaded(devices,request_id))
        threading.Thread(target=worker,daemon=True).start()
    def loaded(self,devices,request_id):
        if request_id!=self.refresh_request_id:return
        self.devices=devices;self.selected=None;self.last_report=None;self.inventory_epoch+=1;self.tree.delete(*self.tree.get_children())
        for i,x in enumerate(devices):
            self.tree.insert("","end",iid=str(i),text=x["name"],values=(x["bus"],x.get("connection","extern (USB)" if sys.platform=="win32" else "Unbekannt"),f'{x["size"]/1e9:.1f} GB'))
        self.status.config(text=f"{len(devices)} Datenträger erfasst. Keine Änderungen vorgenommen.")
        self.show("Gerät auswählen.\n\nUnter Linux werden auch SATA-Laufwerke angezeigt, da M.2-SATA nicht immer eindeutig erkennbar ist.\nUSB-Bridges können NVMe-Informationen verbergen.\nFirmware-Recovery und Datenrettung sind noch nicht implementiert.\nNVMe-Diagnose benötigt unter Linux nvme-cli und entsprechende Leseberechtigungen.")
    def choose(self,event=None):
        sel=self.tree.selection()
        if not sel:return
        x=self.devices[int(sel[0])];self.inventory_epoch+=1;self.selected=x;self.last_report=None
        self.show(f'Modell: {x["name"]}\n\nGerätepfad: {x["path"]}\n\nSeriennummer: {x["serial"] or "Unbekannt"}\n\nGröße: {x["size"]/1e9:.1f} GB\n\nTransport: {x["bus"]}\n\nNVMe bestätigt: {"Ja" if x["nvme"] else "Nein / bei USB nicht verifiziert"}\n\nNur lesende Inventarisierung. Keine Reparatur- oder Flashbefehle.')
    def host_info(self):
        epoch=self.inventory_epoch
        self.status.config(text="Rechneranalyse läuft ...")
        def worker():
            report={"hardware":diagnostics.hardware()}
            self.post_ui(lambda:self.host_done(report,epoch))
        threading.Thread(target=worker,daemon=True).start()
    def host_done(self,report,epoch):
        if epoch!=self.inventory_epoch:
            self.status.config(text="Geräteauswahl geändert; Rechnerbericht verworfen.");return
        self.last_report=report
        self.show(json.dumps(report,ensure_ascii=False,indent=2))
        self.status.config(text="Rechneranalyse erfasst (nur lesend).")
    def controller_info(self):
        if not self.selected:
            messagebox.showinfo("NVMe-Controller","Bitte zuerst ein Laufwerk auswählen.");return
        device=dict(self.selected)
        epoch=self.inventory_epoch
        self.status.config(text="PCIe-Controller wird über Linux-sysfs ermittelt ...")
        def worker():
            report=controller_identity.report(device)
            self.post_ui(lambda:self.controller_done(device,report,epoch))
        threading.Thread(target=worker,daemon=True).start()
    def controller_done(self,device,report,epoch):
        if not self.selection_is_current(device,epoch):
            self.status.config(text="Controllerbericht verworfen: Geräteauswahl geändert.");return
        self.merge_device_report(device,"controller_identity",report)
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Controlleridentität lesend erfasst. Keine Firmwarefreigabe.")
    def diagnose(self):
        if not self.selected:
            messagebox.showinfo("SSD-Diagnose","Bitte zuerst ein Laufwerk auswählen.");return
        device=dict(self.selected)
        epoch=self.inventory_epoch
        self.status.config(text="Lesende SSD-Diagnose läuft ...")
        def worker():
            report=(ata_diagnostics.report(device) if sys.platform.startswith("linux") and device.get("bus") in ("SATA","ATA") else diagnostics.nvme_report(device))
            self.post_ui(lambda:self.diagnose_done(device,report,epoch))
        threading.Thread(target=worker,daemon=True).start()
    def plan_image(self):
        if not self.selected:
            messagebox.showinfo("Abbild planen","Zuerst einen Datenträger auswählen.");return
        destination=filedialog.askdirectory(title="Zielverzeichnis für spätere Datensicherung auswählen")
        if not destination:return
        device=dict(self.selected)
        epoch=self.inventory_epoch
        self.status.config(text="Ziel-Mount und Speicherkapazität werden lesend geprüft ...")
        def worker():
            try:
                result=imaging_plan.plan(device,destination)
            except (OSError,ValueError) as exc:
                result={"error":str(exc),"ready_to_image":False,"device_writes_allowed":False}
            self.post_ui(lambda:self.plan_image_done(device,result,epoch))
        threading.Thread(target=worker,daemon=True).start()
    def plan_image_done(self,device,result,epoch):
        if not self.selection_is_current(device,epoch):
            self.status.config(text="Geräteauswahl geändert; Abbildplanung verworfen.");return
        self.merge_device_report(device,"imaging_plan",result)
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Abbildplanung beendet. Kein Gerät geöffnet, kein Abbild erstellt.")
    def merge_device_report(self,device,key,value):
        """Retain evidence for the same selection without mixing different disks."""
        previous=self.current_diagnostic()
        merged=dict(previous) if previous.get("device")==device else {"device":device}
        merged[key]=value
        self.last_report=merged
        return merged
    def selection_is_current(self,device,epoch):
        return epoch==self.inventory_epoch and self.selected==device
    def current_diagnostic(self):
        report=self.last_report if isinstance(self.last_report,dict) else {}
        if isinstance(report.get("report"),dict):
            report=report["report"]
        return report
    def triage_plan(self):
        if not self.last_report:
            messagebox.showinfo("Recovery-Leitfaden","Zuerst eine Diagnose oder Schutzprüfung durchführen.");return
        report=self.current_diagnostic()
        result=triage.plan(report)
        self.show(json.dumps(result,ensure_ascii=False,indent=2))
        self.status.config(text="Lesender Recovery-Leitfaden angezeigt. Keine Geräteaktion.")
    def catalog_lookup(self):
        report=(self.last_report or {}).get("firmware_file") or {}
        if not report.get("sha256"):
            messagebox.showinfo("Firmwarekatalog","Zuerst eine Firmwaredatei prüfen.");return
        path=filedialog.askopenfilename(title="Lokalen Firmwarekatalog auswählen",filetypes=[("JSON","*.json")])
        if not path:return
        self.status.config(text="Lokaler Katalog wird verglichen ...")
        def worker():
            catalog=firmware_catalog.load(path)
            result=firmware_catalog.lookup(catalog,report)
            self.post_ui(lambda:self.catalog_done(report,result))
        threading.Thread(target=worker,daemon=True).start()
    def catalog_done(self,firmware_report,result):
        self.last_report={"firmware_file":firmware_report,"catalog_lookup":result}
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Hash-Abgleich abgeschlossen. Keine Firmwarefreigabe.")
    def inspect_firmware(self):
        path=filedialog.askopenfilename(title="Firmwaredatei nur lesend untersuchen")
        if not path:return
        self.status.config(text="Firmwaredatei wird offline gelesen und gehasht ...")
        def worker():
            try: report=firmware.inspect(path)
            except (OSError,ValueError) as exc: report={"error":str(exc)}
            self.post_ui(lambda:self.firmware_done(report))
        threading.Thread(target=worker,daemon=True).start()
    def firmware_done(self,report):
        self.last_report={"firmware_file":report}
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Dateianalyse abgeschlossen. Keine Kompatibilitätsfreigabe, kein Flash.")
    def safety_check(self):
        if not self.selected:
            messagebox.showinfo("Schutzprüfung","Bitte zuerst einen Datenträger auswählen.");return
        device=dict(self.selected)
        epoch=self.inventory_epoch
        self.status.config(text="Schutzprüfung läuft ...")
        def worker():
            report=safety.classify(device)
            self.post_ui(lambda:self.safety_done(device,report,epoch))
        threading.Thread(target=worker,daemon=True).start()
    def safety_done(self,device,report,epoch):
        if not self.selection_is_current(device,epoch):
            self.status.config(text="Auswahl geändert; Schutzprüfung verworfen.");return
        self.merge_device_report(device,"safety",report)
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Schutzprüfung abgeschlossen: Schreibzugriffe gesperrt.")
    def usb_diagnose(self):
        if not self.selected:
            messagebox.showinfo("USB-Verbindung","Bitte zuerst ein USB-Gerät auswählen.");return
        device=dict(self.selected)
        epoch=self.inventory_epoch
        self.status.config(text="USB-Verbindung wird lesend analysiert ...")
        def worker():
            report=diagnostics.usb_report(device)
            self.post_ui(lambda:self.usb_done(device,report,epoch))
        threading.Thread(target=worker,daemon=True).start()
    def usb_done(self,device,report,epoch):
        if not self.selection_is_current(device,epoch):
            self.status.config(text="Geräteauswahl geändert; Bericht verworfen.");return
        self.merge_device_report(device,"usb_diagnostics",report)
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="USB-Bericht abgeschlossen. NVMe-Passthrough nicht bestätigt.")
    def diagnose_done(self,device,report,epoch):
        if not self.selection_is_current(device,epoch):
            self.status.config(text="Diagnose abgeschlossen; Geräteauswahl geändert.");return
        self.merge_device_report(device,"diagnostics",report)
        if "smart" in report:
            self.last_report["assessment"]=analysis.assess(self.last_report)
        elif "smartctl" in report:
            self.last_report["assessment"]=ata_analysis.assess(self.last_report)
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Lesende SSD-Diagnose abgeschlossen. Fehlende SMART-Daten und Berechtigungen werden angezeigt.")
    def assess(self):
        if not self.last_report:
            messagebox.showinfo("Befund","Zuerst NVMe-Diagnose durchführen.");return
        report=self.current_diagnostic()
        if "diagnostics" not in report:
            messagebox.showinfo("Befund","Diese Fallakte enthält keine SSD-SMART-Diagnose.");return
        result=(ata_analysis.assess(report) if "smartctl" in report.get("diagnostics",{}) else analysis.assess(report))
        self.show(json.dumps(result,ensure_ascii=False,indent=2))
        self.status.config(text="Regelbasierter Befund angezeigt; kein KI-Modell.")
    def compare_case(self):
        current=self.current_diagnostic()
        if "diagnostics" not in current:
            messagebox.showinfo("Vergleich","Zuerst eine aktuelle SSD-Diagnose durchführen.");return
        try: entries=cases.recent()
        except Exception as exc:messagebox.showerror("Fallakten",str(exc));return
        if not entries:
            messagebox.showinfo("Vergleich","Keine gespeicherten Fallakten vorhanden.");return
        win=tk.Toplevel(self);win.title("Fallakte für Vergleich auswählen");win.geometry("670x360")
        lb=tk.Listbox(win);lb.pack(fill="both",expand=True,padx=12,pady=12)
        for item in entries:lb.insert("end",f"#{item[0]} | {item[1][:19]} | {item[2]}")
        def choose_case():
            selected=lb.curselection()
            if not selected:return
            try: old=cases.load(entries[selected[0]][0])
            except (OSError,ValueError,sqlite3.Error) as exc:
                messagebox.showerror("Fallakten",str(exc));return
            old_report=(old or {}).get("report") or {}
            if isinstance(old_report,dict) and isinstance(old_report.get("report"),dict):
                old_report=old_report["report"]
            if not isinstance(old_report,dict) or "diagnostics" not in old_report:
                messagebox.showinfo("Vergleich","Gespeicherte Fallakte enthält keine SSD-Diagnose.");return
            old_diag=old_report.get("diagnostics") or {}
            new_diag=current.get("diagnostics") or {}
            if not isinstance(old_diag,dict) or not isinstance(new_diag,dict):
                result={"error":"Ungültige Diagnoseformate"}
            elif "smartctl" in old_diag and "smartctl" in new_diag:
                result=ata_analysis.compare(old_report,current)
            elif "smart" in old_diag and "smart" in new_diag:
                result=analysis.compare(old_report,current)
            else:
                result={"error":"Vergleich gesperrt: unterschiedliche Diagnoseverfahren"}
            self.show(json.dumps(result,ensure_ascii=False,indent=2))
            win.destroy()
        tk.Button(win,text="Vergleichen",command=choose_case).pack(pady=8)
    def save_case(self):
        if not self.last_report:
            messagebox.showinfo("Fallakte","Bitte zuerst eine Diagnose, Schutzprüfung oder Rechneranalyse durchführen.");return
        item=(self.last_report or {}).get("device") or {}
        snapshot={"device":item,"report":self.last_report,"captured":datetime.now().astimezone().isoformat()}
        try:
            case_id=cases.save(item.get("name","Rechneranalyse"),item.get("path",""),snapshot)
            self.status.config(text=f"Fallakte #{case_id} lokal gespeichert.")
        except (OSError,ValueError,sqlite3.Error) as exc:messagebox.showerror("Speicherfehler",str(exc))
    def open_cases(self):
        try: entries=cases.recent()
        except Exception as exc:messagebox.showerror("Datenbankfehler",str(exc));return
        window=tk.Toplevel(self);window.title("Gespeicherte Fallakten");window.geometry("680x400")
        listing=tk.Listbox(window,font=("Segoe UI",11));listing.pack(fill="both",expand=True,padx=14,pady=14)
        for item in entries:listing.insert("end",f"#{item[0]} | {item[1][:19]} | {item[2]} | {item[3]}")
        def open_selected():
            chosen=listing.curselection()
            if not chosen:return
            try: record=cases.load(entries[chosen[0]][0])
            except (OSError,ValueError,sqlite3.Error) as exc:
                messagebox.showerror("Fallakten",str(exc));return
            self.show(json.dumps(record,ensure_ascii=False,indent=2))
            self.last_report=record
            window.destroy()
        tk.Button(window,text="Fallakte öffnen",command=open_selected).pack(pady=10)
    def export(self):
        path=filedialog.asksaveasfilename(defaultextension=".json",filetypes=[("JSON","*.json")])
        if not path:return
        anonymize=messagebox.askyesno("Datenschutz","Bekannte Gerätekennungen und rohe Diagnoseinformationen ausblenden? (Empfohlen; vollständige Anonymität nicht garantiert)")
        try:
            exporter.export_json(path,{"app":"NVMe Recovery Studio AI","created":datetime.now().astimezone().isoformat(),"edition":platform.system(),"inventory":self.devices,"selected":self.selected,"report":self.last_report,"notice":"Keine validierte Recovery-Diagnose."},anonymize=anonymize)
            self.status.config(text="Fallakte exportiert.")
        except OSError as e:messagebox.showerror("Fehler",str(e))

if __name__=="__main__":App().mainloop()
