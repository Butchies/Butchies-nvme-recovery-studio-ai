"""NVMe Recovery Studio AI - read-only desktop application."""
import json, platform, subprocess, sys, threading, tkinter as tk
from datetime import datetime
from pathlib import Path
from tkinter import filedialog, messagebox, ttk
import cases
import diagnostics
import analysis
import exporter

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
        return [{"name":str(x.get("Model") or "USB-Laufwerk"),"path":str(x.get("DeviceID") or ""),"serial":str(x.get("SerialNumber") or ""),"size":int(x.get("Size") or 0),"bus":"USB","nvme":False} for x in data]
    if sys.platform.startswith("linux"):
        try: data=json.loads(cmd(["lsblk","-J","-b","-d","-o","NAME,MODEL,SERIAL,SIZE,TRAN,TYPE,PATH"]))
        except (ValueError,TypeError): return []
        out=[]
        for x in data.get("blockdevices",[]):
            name=str(x.get("name") or "")
            if x.get("type")!="disk" or not(name.startswith("nvme") or x.get("tran")=="usb"):continue
            out.append({"name":str(x.get("model") or name),"path":str(x.get("path") or "/dev/"+name),"serial":str(x.get("serial") or ""),"size":int(x.get("size") or 0),"bus":"PCIe/NVMe" if name.startswith("nvme") else "USB","nvme":name.startswith("nvme")})
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
        self.label(top,("Windows 10/11 | Nur USB-Geräte" if sys.platform=="win32" else "Linux | NVMe und USB") +" | Nur lesende Inventarisierung",11).pack(anchor="w",pady=(6,0))
        bar=tk.Frame(self,bg="#101722",padx=22,pady=12);bar.pack(fill="x")
        buttons=tk.Frame(bar,bg="#101722");buttons.pack(fill="x")
        for name,cb in [("Geräte aktualisieren",self.refresh),("Rechneranalyse",self.host_info),("NVMe-Diagnose",self.diagnose),("Befund",self.assess),("Vergleich",self.compare_case),("Fallakte speichern",self.save_case),("Fallakten",self.open_cases),("Fallakte exportieren",self.export)]:
            tk.Button(buttons,text=name,command=cb,bg="#22566c",fg="white",relief="flat",padx=9,pady=7).pack(side="left",padx=(0,5))
        main=tk.PanedWindow(self,orient="horizontal",bg="#101722",sashwidth=6,bd=0);main.pack(fill="both",expand=True,padx=22,pady=12)
        left=tk.Frame(main,bg="#192333",padx=12,pady=12);right=tk.Frame(main,bg="#192333",padx=12,pady=12);main.add(left,minsize=350);main.add(right,minsize=280)
        self.label(left,"Erkannte Datenträger",15,True).pack(anchor="w")
        self.tree=ttk.Treeview(left,columns=("bus","size"),show="tree headings");self.tree.heading("#0",text="Gerät");self.tree.heading("bus",text="Anschluss");self.tree.heading("size",text="Größe");self.tree.column("#0",width=210);self.tree.column("bus",width=90);self.tree.column("size",width=100);self.tree.pack(fill="both",expand=True,pady=12);self.tree.bind("<<TreeviewSelect>>",self.choose)
        self.label(right,"Gerätedetails",15,True).pack(anchor="w")
        self.detail=tk.Text(right,bg="#192333",fg="#e8eff8",font=("Consolas",11),relief="flat",wrap="word",state="disabled");self.detail.pack(fill="both",expand=True,pady=12)
        self.status=self.label(self,"Keine Schreib-, Lösch- oder Firmwarefunktionen",10);self.status.pack(anchor="w",padx=22,pady=12)
        self.last_report=None
        self.case_for_compare=None
        self.refresh()
    def label(self,parent,text,size,bold=False):
        return tk.Label(parent,text=text,bg=parent["bg"],fg="#e8eff8",font=("Segoe UI",size,"bold" if bold else "normal"))
    def show(self,text):
        self.detail.config(state="normal");self.detail.delete("1.0","end");self.detail.insert("1.0",text);self.detail.config(state="disabled")
    def refresh(self):
        self.status.config(text="Geräte werden erfasst ...")
        threading.Thread(target=lambda:self.after(0,self.loaded,discover()),daemon=True).start()
    def loaded(self,devices):
        self.devices=devices;self.selected=None;self.last_report=None;self.tree.delete(*self.tree.get_children())
        for i,x in enumerate(devices):
            self.tree.insert("","end",iid=str(i),text=x["name"],values=(x["bus"],f'{x["size"]/1e9:.1f} GB'))
        self.status.config(text=f"{len(devices)} Datenträger erfasst. Keine Änderungen vorgenommen.")
        self.show("Gerät auswählen.\n\nUSB-Bridges können NVMe-Informationen verbergen.\nFirmware-Recovery und Datenrettung sind noch nicht implementiert.\nNVMe-Diagnose benötigt unter Linux nvme-cli und entsprechende Leseberechtigungen.")
    def choose(self,event=None):
        sel=self.tree.selection()
        if not sel:return
        x=self.devices[int(sel[0])];self.selected=x;self.last_report=None
        self.show(f'Modell: {x["name"]}\n\nGerätepfad: {x["path"]}\n\nSeriennummer: {x["serial"] or "Unbekannt"}\n\nGröße: {x["size"]/1e9:.1f} GB\n\nTransport: {x["bus"]}\n\nNVMe bestätigt: {"Ja" if x["nvme"] else "Nein, USB-Bridge nicht verifiziert"}\n\nNur lesende Inventarisierung. Keine Reparatur- oder Flashbefehle.')
    def host_info(self):
        self.last_report={"hardware":diagnostics.hardware()}
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="Rechneranalyse erfasst (nur lesend).")
    def diagnose(self):
        if not self.selected:
            messagebox.showinfo("NVMe-Diagnose","Bitte zuerst ein Laufwerk auswählen.");return
        device=dict(self.selected)
        self.status.config(text="Lesende NVMe-Diagnose läuft ...")
        def worker():
            report=diagnostics.nvme_report(device)
            self.after(0,lambda:self.diagnose_done(device,report))
        threading.Thread(target=worker,daemon=True).start()
    def diagnose_done(self,device,report):
        if self.selected is None or self.selected.get("path")!=device.get("path"):
            self.status.config(text="Diagnose abgeschlossen; Geräteauswahl geändert.");return
        self.last_report={"device":device,"diagnostics":report}
        self.last_report["assessment"]=analysis.assess(self.last_report)
        self.show(json.dumps(self.last_report,ensure_ascii=False,indent=2))
        self.status.config(text="NVMe-Diagnose abgeschlossen. Fehler und fehlende Berechtigungen werden angezeigt.")
    def assess(self):
        if not self.last_report:
            messagebox.showinfo("Befund","Zuerst NVMe-Diagnose durchführen.");return
        report=self.last_report.get("report",self.last_report)
        result=analysis.assess(report)
        self.show(json.dumps(result,ensure_ascii=False,indent=2))
        self.status.config(text="Regelbasierter Befund angezeigt; kein KI-Modell.")
    def compare_case(self):
        if not self.last_report:
            messagebox.showinfo("Vergleich","Zuerst aktuelle NVMe-Diagnose durchführen.");return
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
            old=cases.load(entries[selected[0]][0])
            old_report=(old or {}).get("report") or {}
            current=self.last_report.get("report",self.last_report)
            result=analysis.compare(old_report,current)
            self.show(json.dumps(result,ensure_ascii=False,indent=2))
            win.destroy()
        tk.Button(win,text="Vergleichen",command=choose_case).pack(pady=8)
    def save_case(self):
        if not self.selected and not self.last_report:
            messagebox.showinfo("Fallakte","Bitte zuerst ein Gerät oder die Rechneranalyse öffnen.");return
        item=self.selected or {}
        snapshot={"device":item,"report":self.last_report,"captured":datetime.now().astimezone().isoformat()}
        try:
            case_id=cases.save(item.get("name","Rechneranalyse"),item.get("path",""),snapshot)
            self.status.config(text=f"Fallakte #{case_id} lokal gespeichert.")
        except (OSError,ValueError) as exc:messagebox.showerror("Speicherfehler",str(exc))
    def open_cases(self):
        try: entries=cases.recent()
        except OSError as exc:messagebox.showerror("Datenbankfehler",str(exc));return
        window=tk.Toplevel(self);window.title("Gespeicherte Fallakten");window.geometry("680x400")
        listing=tk.Listbox(window,font=("Segoe UI",11));listing.pack(fill="both",expand=True,padx=14,pady=14)
        for item in entries:listing.insert("end",f"#{item[0]} | {item[1][:19]} | {item[2]} | {item[3]}")
        def open_selected():
            chosen=listing.curselection()
            if not chosen:return
            record=cases.load(entries[chosen[0]][0])
            self.show(json.dumps(record,ensure_ascii=False,indent=2))
            self.last_report=record
            window.destroy()
        tk.Button(window,text="Fallakte öffnen",command=open_selected).pack(pady=10)
    def export(self):
        path=filedialog.asksaveasfilename(defaultextension=".json",filetypes=[("JSON","*.json")])
        if not path:return
        try:
            Path(path).write_text(json.dumps({"app":"NVMe Recovery Studio AI","created":datetime.now().astimezone().isoformat(),"edition":platform.system(),"inventory":self.devices,"selected":self.selected,"report":self.last_report,"notice":"Inventarisierung, keine validierte Recovery-Diagnose."},indent=2,ensure_ascii=False),encoding="utf-8")
            self.status.config(text="Fallakte exportiert.")
        except OSError as e:messagebox.showerror("Fehler",str(e))

if __name__=="__main__":App().mainloop()
