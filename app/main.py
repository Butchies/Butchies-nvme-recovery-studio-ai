"""NVMe Recovery Studio AI - read-only desktop application."""
import json, platform, subprocess, sys, threading, tkinter as tk
from datetime import datetime
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

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
        for name,cb in [("Geräte aktualisieren",self.refresh),("Fallakte exportieren",self.export)]:
            tk.Button(bar,text=name,command=cb,bg="#22566c",fg="white",relief="flat",padx=14,pady=8).pack(side="left",padx=(0,10))
        main=tk.PanedWindow(self,orient="horizontal",bg="#101722",sashwidth=6,bd=0);main.pack(fill="both",expand=True,padx=22,pady=12)
        left=tk.Frame(main,bg="#192333",padx=12,pady=12);right=tk.Frame(main,bg="#192333",padx=12,pady=12);main.add(left,minsize=350);main.add(right,minsize=280)
        self.label(left,"Erkannte Datenträger",15,True).pack(anchor="w")
        self.tree=ttk.Treeview(left,columns=("bus","size"),show="tree headings");self.tree.heading("#0",text="Gerät");self.tree.heading("bus",text="Anschluss");self.tree.heading("size",text="Größe");self.tree.column("#0",width=210);self.tree.column("bus",width=90);self.tree.column("size",width=100);self.tree.pack(fill="both",expand=True,pady=12);self.tree.bind("<<TreeviewSelect>>",self.choose)
        self.label(right,"Gerätedetails",15,True).pack(anchor="w")
        self.detail=tk.Text(right,bg="#192333",fg="#e8eff8",font=("Consolas",11),relief="flat",wrap="word",state="disabled");self.detail.pack(fill="both",expand=True,pady=12)
        self.status=self.label(self,"Keine Schreib-, Lösch- oder Firmwarefunktionen",10);self.status.pack(anchor="w",padx=22,pady=12)
        self.refresh()
    def label(self,parent,text,size,bold=False):
        return tk.Label(parent,text=text,bg=parent["bg"],fg="#e8eff8",font=("Segoe UI",size,"bold" if bold else "normal"))
    def show(self,text):
        self.detail.config(state="normal");self.detail.delete("1.0","end");self.detail.insert("1.0",text);self.detail.config(state="disabled")
    def refresh(self):
        self.status.config(text="Geräte werden erfasst ...")
        threading.Thread(target=lambda:self.after(0,self.loaded,discover()),daemon=True).start()
    def loaded(self,devices):
        self.devices=devices;self.selected=None;self.tree.delete(*self.tree.get_children())
        for i,x in enumerate(devices):
            self.tree.insert("","end",iid=str(i),text=x["name"],values=(x["bus"],f'{x["size"]/1e9:.1f} GB'))
        self.status.config(text=f"{len(devices)} Datenträger erfasst. Keine Änderungen vorgenommen.")
        self.show("Gerät auswählen.\n\nUSB-Bridges können NVMe-Informationen verbergen.\nKI, Firmware-Recovery und Datenrettung sind noch nicht implementiert.")
    def choose(self,event=None):
        sel=self.tree.selection()
        if not sel:return
        x=self.devices[int(sel[0])];self.selected=x
        self.show(f'Modell: {x["name"]}\n\nGerätepfad: {x["path"]}\n\nSeriennummer: {x["serial"] or "Unbekannt"}\n\nGröße: {x["size"]/1e9:.1f} GB\n\nTransport: {x["bus"]}\n\nNVMe bestätigt: {"Ja" if x["nvme"] else "Nein, USB-Bridge nicht verifiziert"}\n\nNur lesende Inventarisierung. Keine Reparatur- oder Flashbefehle.')
    def export(self):
        path=filedialog.asksaveasfilename(defaultextension=".json",filetypes=[("JSON","*.json")])
        if not path:return
        try:
            Path(path).write_text(json.dumps({"app":"NVMe Recovery Studio AI","created":datetime.now().astimezone().isoformat(),"edition":platform.system(),"inventory":self.devices,"selected":self.selected,"notice":"Inventarisierung, keine validierte Recovery-Diagnose."},indent=2,ensure_ascii=False),encoding="utf-8")
            self.status.config(text="Fallakte exportiert.")
        except OSError as e:messagebox.showerror("Fehler",str(e))

if __name__=="__main__":App().mainloop()
