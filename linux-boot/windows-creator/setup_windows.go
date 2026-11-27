package main

import (
	"bufio"
	"bytes"
	"crypto/sha256"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"syscall"
	"time"
	"unsafe"
)

var shell = syscall.NewLazyDLL("shell32.dll")

func admin() bool { v, _, _ := shell.NewProc("IsUserAnAdmin").Call(); return v != 0 }
func elevate() error {
	path, e := os.Executable()
	if e != nil {
		return e
	}
	verb, _ := syscall.UTF16PtrFromString("runas")
	exe, e := syscall.UTF16PtrFromString(path)
	if e != nil {
		return e
	}
	r, _, e := shell.NewProc("ShellExecuteW").Call(0, uintptr(unsafe.Pointer(verb)), uintptr(unsafe.Pointer(exe)), 0, 0, 1)
	if r <= 32 {
		return fmt.Errorf("Administratorfreigabe fehlt: %v", e)
	}
	return nil
}

type progress struct {
	n, total int64
	last     time.Time
}

func (p *progress) Write(b []byte) (int, error) {
	p.n += int64(len(b))
	if time.Since(p.last) > time.Second {
		fmt.Printf("\r%.0f / %.0f MB ", float64(p.n)/1e6, float64(p.total)/1e6)
		p.last = time.Now()
	}
	return len(b), nil
}
func fetch(url, path, want string, max int64) error {
	client := http.Client{Timeout: time.Hour}
	r, e := client.Get(url)
	if e != nil {
		return e
	}
	defer r.Body.Close()
	if r.StatusCode != 200 {
		return fmt.Errorf("Download: %s", r.Status)
	}
	f, e := os.Create(path)
	if e != nil {
		return e
	}
	h := sha256.New()
	n, e := io.Copy(io.MultiWriter(f, h, &progress{total: r.ContentLength}), io.LimitReader(r.Body, max+1))
	ce := f.Close()
	fmt.Println()
	if e != nil {
		return e
	}
	if ce != nil {
		return ce
	}
	if n > max || fmt.Sprintf("%x", h.Sum(nil)) != want {
		return fmt.Errorf("Download weicht vom geprüften Release ab.")
	}
	return nil
}
func verify(img string) error {
	cmd := exec.Command(filepath.Join(os.Getenv("SystemRoot"), "System32", "WindowsPowerShell", "v1.0", "powershell.exe"), "-NoProfile", "-NonInteractive", "-Command", `$ErrorActionPreference='Stop'; [Console]::OutputEncoding=[System.Text.UTF8Encoding]::new($false); ConvertTo-Json -Compress -InputObject @(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and !$_.IsBoot -and !$_.IsSystem } | Select-Object Number,FriendlyName)`)
	data, e := cmd.Output()
	if e != nil {
		return e
	}
	var ds []struct {
		Number       int
		FriendlyName string
	}
	if e = json.Unmarshal(bytes.TrimPrefix(data, []byte{239, 187, 191}), &ds); e != nil {
		return e
	}
	source, e := os.Open(img)
	if e != nil {
		return e
	}
	defer source.Close()
	prefix := make([]byte, 65536)
	if _, e = io.ReadFull(source, prefix); e != nil {
		return e
	}
	for _, d := range ds {
		f, e := os.Open(fmt.Sprintf(`\\.\PhysicalDrive%d`, d.Number))
		if e != nil {
			continue
		}
		b := make([]byte, len(prefix))
		_, e = io.ReadFull(f, b)
		if e != nil || !bytes.Equal(b, prefix) {
			f.Close()
			continue
		}
		fmt.Println("Prüflesen:", d.FriendlyName, "Datenträger", d.Number)
		h := sha256.New()
		h.Write(b)
		_, e = io.CopyN(io.MultiWriter(h, &progress{n: int64(len(b)), total: size}), f, size-int64(len(b)))
		f.Close()
		fmt.Println()
		if e == nil && fmt.Sprintf("%x", h.Sum(nil)) == hash {
			fmt.Println("Vollständiges Abbild bestätigt auf", d.FriendlyName)
			return nil
		}
	}
	return fmt.Errorf("Keinen vollständig bestätigten USB-Stick gefunden. Stick eingesteckt lassen und ggf. DD-Abbild-Modus verwenden.")
}
func run() error {
	fmt.Println("Butchies Recovery – Manueller Modus, Hardware-Scan und Bootreport")
	tmp, e := os.MkdirTemp("", "Butchies-PCIe-")
	if e != nil {
		return e
	}
	defer os.RemoveAll(tmp)
	base := filepath.Join(tmp, "basis.iso")
	img := filepath.Join(tmp, "Butchies-PM981a.img")
	writer := filepath.Join(tmp, "rufus-4.16p.exe")
	fmt.Println("Linux-Basis laden. Die Dell-Pakete und der PCIe-Startdienst sind in diesem Setup enthalten.")
	if e = fetch("https://github.com/Butchies/Butchies-nvme-recovery-studio-ai/releases/download/aktuell/nvme-recovery-studio-ai-linux.iso", base, "71c36b5f6dc49d2446813f314c8fa25654ff9191b087c6ea142ed223851cb9a3", 1168670720); e != nil {
		return e
	}
	fmt.Println("Bootabbild erstellen und prüfen …")
	if e = assemble(base, img); e != nil {
		return e
	}
	os.Remove(base)
	if e = fetch("https://github.com/pbatard/rufus/releases/download/v4.16/rufus-4.16p.exe", writer, "8351a56c134aa6dd26a930d702cc3fd57f11ef90b17e55a8de960d5a73d46cfc", 8<<20); e != nil {
		return e
	}
	fmt.Println("In Rufus USB-Ziel kontrollieren und START wählen. Inhalt dieses Sticks wird gelöscht. Wenn gefragt: DD-Abbild-Modus. Nach FERTIG Rufus schließen und Stick eingesteckt lassen.")
	cmd := exec.Command(writer, "-g", "-l", "de-DE", "-i", img)
	cmd.Dir = tmp
	if e = cmd.Run(); e != nil {
		return e
	}
	return verify(img)
}
func main() {
	syscall.NewLazyDLL("kernel32.dll").NewProc("SetConsoleOutputCP").Call(65001)
	var e error
	if !admin() {
		e = elevate()
		if e == nil {
			return
		}
	} else {
		e = run()
	}
	if e != nil {
		fmt.Println("Hinweis:", e)
	}
	fmt.Print("Eingabetaste zum Schließen.")
	bufio.NewReader(os.Stdin).ReadString('\n')
	if e != nil {
		os.Exit(1)
	}
}
