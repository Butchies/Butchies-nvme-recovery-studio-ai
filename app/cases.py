"""Local SQLite case history; never opens a block device for writing."""
import json
import sqlite3
from contextlib import closing
from datetime import datetime, timezone
from pathlib import Path

def db_path():
    import os, sys
    if sys.platform=="win32": root=Path(os.environ.get("LOCALAPPDATA",str(Path.home())))/"NVMeRecoveryStudioAI"
    else: root=Path(os.environ.get("XDG_DATA_HOME",str(Path.home()/".local/share")))/"nvme-recovery-studio-ai"
    root.mkdir(parents=True,exist_ok=True)
    return root/"cases.sqlite3"

def connect():
    conn=sqlite3.connect(db_path())
    conn.execute("""CREATE TABLE IF NOT EXISTS cases(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        created TEXT NOT NULL,
        title TEXT NOT NULL,
        device_path TEXT NOT NULL,
        snapshot TEXT NOT NULL
    )""")
    conn.commit()
    return conn

def save(title, device_path, snapshot):
    with closing(connect()) as conn, conn:
        cursor=conn.execute("INSERT INTO cases(created,title,device_path,snapshot) VALUES (?,?,?,?)",
             (datetime.now(timezone.utc).isoformat(),title,device_path,json.dumps(snapshot,ensure_ascii=False)))
        return cursor.lastrowid

def recent(limit=50):
    with closing(connect()) as conn:
        return conn.execute("SELECT id,created,title,device_path FROM cases ORDER BY id DESC LIMIT ?",(limit,)).fetchall()

def load(case_id):
    with closing(connect()) as conn:
        row=conn.execute("SELECT snapshot FROM cases WHERE id=?",(case_id,)).fetchone()
    return json.loads(row[0]) if row else None
