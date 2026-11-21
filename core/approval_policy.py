"""Fail-closed recovery authorization data model. No executor is implemented."""
from __future__ import annotations
from dataclasses import dataclass
from enum import Enum
import re


class Platform(str, Enum):
    WINDOWS_USB = "windows_usb"
    LINUX_PCIE = "linux_pcie"


class Bus(str, Enum):
    USB = "usb"
    PCIE = "pcie"


@dataclass(frozen=True)
class Target:
    stable_fingerprint: str
    bus: Bus
    is_boot_device: bool
    is_system_device: bool
    identity_verified: bool


@dataclass(frozen=True)
class ProposedWrite:
    target_fingerprint: str
    image_sha256: str
    compatibility_verified: bool
    programmer_supported: bool
    firmware_provenance_verified: bool
    confirmation_token_valid: bool


def may_authorize(platform: Platform, target: Target, proposal: ProposedWrite) -> tuple[bool, str]:
    """Conservative planning gate, NOT permission to execute a flash."""
    if target.is_boot_device or target.is_system_device:
        return False, "Boot-/Systemlaufwerk geschützt"
    if not target.identity_verified or not target.stable_fingerprint:
        return False, "Geräteidentität nicht verifiziert"
    if platform == Platform.WINDOWS_USB and target.bus != Bus.USB:
        return False, "Windows-Edition erlaubt ausschließlich USB-Ziele"
    if platform == Platform.LINUX_PCIE and target.bus != Bus.PCIE:
        return False, "Linux-PCIe-Modus erwartet ein PCIe-Ziel"
    if proposal.target_fingerprint != target.stable_fingerprint:
        return False, "Zielgerät hat sich geändert"
    if not re.fullmatch(r"[a-fA-F0-9]{64}", proposal.image_sha256):
        return False, "Firmware-Hash fehlt oder ist ungültig"
    if not proposal.compatibility_verified:
        return False, "Kompatibilität nicht bestätigt"
    if not proposal.programmer_supported:
        return False, "Kein unterstützter Programmierweg"
    if not proposal.firmware_provenance_verified:
        return False, "Firmwareherkunft nicht verifiziert"
    if not proposal.confirmation_token_valid:
        return False, "Explizite Freigabe fehlt"
    return True, "Planungsprüfung bestanden; kein Flash-Executor vorhanden"
