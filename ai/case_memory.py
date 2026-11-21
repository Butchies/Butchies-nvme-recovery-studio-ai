"""Local SQLite case notebook. Stores evidence and conclusions, not hidden model thoughts."""
from __future__ import annotations
import json
import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA = """
PRAGMA foreign_keys=ON;
CREATE TABLE IF NOT EXISTS cases (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    created_at TEXT NOT NULL,
    device_fingerprint TEXT NOT NULL,
    status TEXT NOT NULL DEFAULT 'open'
);
CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    case_id INTEGER NOT NULL REFERENCES cases(id),
    created_at TEXT NOT NULL,
    event_type TEXT NOT NULL,
    evidence_json TEXT NOT NULL,
    explanation TEXT NOT NULL
);
"""


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


class CaseMemory:
    def __init__(self, database: str | Path):
        self.database = str(database)
        with self.connect() as conn:
            conn.executescript(SCHEMA)

    def connect(self) -> sqlite3.Connection:
        return sqlite3.connect(self.database)

    def new_case(self, device_fingerprint: str) -> int:
        if not device_fingerprint.strip():
            raise ValueError("Device fingerprint required")
        with self.connect() as conn:
            cur = conn.execute(
                "INSERT INTO cases (created_at, device_fingerprint) VALUES (?, ?)",
                (utc_now(), device_fingerprint),
            )
            return int(cur.lastrowid)

    def record(self, case_id: int, event_type: str, evidence: dict[str, Any], explanation: str) -> None:
        if not event_type.strip():
            raise ValueError("Event type required")
        with self.connect() as conn:
            conn.execute(
                "INSERT INTO events (case_id, created_at, event_type, evidence_json, explanation) VALUES (?, ?, ?, ?, ?)",
                (case_id, utc_now(), event_type, json.dumps(evidence, ensure_ascii=False), explanation),
            )

    def events(self, case_id: int) -> list[dict[str, Any]]:
        with self.connect() as conn:
            rows = conn.execute(
                "SELECT created_at, event_type, evidence_json, explanation FROM events WHERE case_id=? ORDER BY id",
                (case_id,),
            ).fetchall()
        return [
            {"created_at": ts, "event_type": typ, "evidence": json.loads(payload), "explanation": why}
            for ts, typ, payload, why in rows
        ]
