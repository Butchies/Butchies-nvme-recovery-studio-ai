"""Read-only Linux NVMe inventory. No raw writes, resets or firmware commands."""
from __future__ import annotations
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import re
from typing import Optional

SYS_CLASS_NVME = Path("/sys/class/nvme")
SYS_BLOCK = Path("/sys/block")
NAME_RE = re.compile(r"^nvme[0-9]+$")


def read_text(path: Path) -> Optional[str]:
    try:
        return path.read_text(encoding="utf-8", errors="replace").strip() or None
    except (OSError, UnicodeError):
        return None


def pci_attribute(controller: Path, attribute: str) -> Optional[str]:
    try:
        device = (controller / "device").resolve(strict=True)
        if not str(device).startswith("/sys/"):
            return None
        return read_text(device / attribute)
    except OSError:
        return None


@dataclass(frozen=True)
class NvmeController:
    name: str
    serial: Optional[str]
    model: Optional[str]
    firmware_rev: Optional[str]
    transport: Optional[str]
    state: Optional[str]
    pci_vendor: Optional[str]
    pci_device: Optional[str]
    pci_subsystem_vendor: Optional[str]
    pci_subsystem_device: Optional[str]
    namespaces: tuple[str, ...]


def discover_controllers(root: Path = SYS_CLASS_NVME, block_root: Path = SYS_BLOCK) -> list[NvmeController]:
    """Enumerate sysfs only. Missing attributes are reported as null."""
    if not root.is_dir():
        return []
    results: list[NvmeController] = []
    for entry in sorted(root.iterdir(), key=lambda p: p.name):
        if not NAME_RE.fullmatch(entry.name):
            continue
        try:
            namespaces = tuple(sorted(
                p.name for p in block_root.glob(entry.name + "n*")
                if re.fullmatch(re.escape(entry.name) + r"n[0-9]+", p.name)
            ))
            results.append(NvmeController(
                name=entry.name,
                serial=read_text(entry / "serial"),
                model=read_text(entry / "model"),
                firmware_rev=read_text(entry / "firmware_rev"),
                transport=read_text(entry / "transport"),
                state=read_text(entry / "state"),
                pci_vendor=pci_attribute(entry, "vendor"),
                pci_device=pci_attribute(entry, "device"),
                pci_subsystem_vendor=pci_attribute(entry, "subsystem_vendor"),
                pci_subsystem_device=pci_attribute(entry, "subsystem_device"),
                namespaces=namespaces,
            ))
        except OSError:
            continue
    return results


def inventory_json() -> str:
    return json.dumps({
        "schema_version": 1,
        "platform": "linux",
        "read_only": True,
        "controllers": [asdict(x) for x in discover_controllers()],
    }, ensure_ascii=False, indent=2)
