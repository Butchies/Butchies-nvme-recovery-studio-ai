"""Linux inventory CLI; no privileges or firmware commands required."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from core.device_inventory import inventory_json  # noqa: E402

if __name__ == "__main__":
    print(inventory_json())
