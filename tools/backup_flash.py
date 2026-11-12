#!/usr/bin/env python3
"""Make two matching, private full-flash reads from an enrolled Feather."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import subprocess
import tempfile

from boards import CHAPTER, IDENTITIES, discover

FLASH_BYTES = 4 * 1024 * 1024
ESPTOOL = (
    Path.home()
    / "Library/Arduino15/packages/esp32/tools/esptool_py/5.2.0/esptool"
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("color", choices=IDENTITIES)
    parser.add_argument("--port", help="ROM download serial port")
    args = parser.parse_args()
    port = args.port
    if port is None:
        candidates = []
        for record in discover():
            candidate = record.get("port", {})
            properties = candidate.get("properties", {})
            usb_id = (properties.get("vid", "").lower(), properties.get("pid", "").lower())
            serial = properties.get("serialNumber", "").lower()
            if usb_id == ("0x303a", "0x1001") and serial == IDENTITIES[args.color]:
                candidates.append(candidate["address"])
        if len(candidates) != 1:
            parser.error(f"Put {args.color} in ROM mode or supply its ROM port with --port.")
        port = candidates[0]
    os.umask(0o077)
    command = [
        str(ESPTOOL), "--chip", "esp32s3", "--port", port,
        "--before", "no-reset", "--after", "no-reset",
    ]
    identity = subprocess.check_output(
        [*command, "--no-stub", "read-mac"], stderr=subprocess.STDOUT, text=True,
    )
    print(identity, end="")
    found = re.findall(r"(?im)^\s*MAC:\s*((?:[0-9a-f]{2}:){5}[0-9a-f]{2})\s*$", identity)
    if not found or any(mac.lower() != IDENTITIES[args.color] for mac in found):
        parser.exit(1, "Factory MAC did not match. No flash image was read.\n")

    recovery = CHAPTER / "build" / "private-recovery"
    recovery.mkdir(parents=True, exist_ok=True)
    destination = Path(tempfile.mkdtemp(prefix=f"{args.color}-", dir=recovery))
    first = destination / "flash.bin"
    with tempfile.TemporaryDirectory(prefix="verify-", dir=destination) as temp:
        second = Path(temp) / "verify.bin"
        for output in (first, second):
            subprocess.run(
                [*command, "--baud", "460800", "read-flash", "0", "0x400000", str(output)],
                check=True,
            )
        original = first.read_bytes()
        if len(original) != FLASH_BYTES or original != second.read_bytes():
            parser.exit(1, f"Flash reads did not verify. Unverified first read: {first}\n")
    digest = hashlib.sha256(original).hexdigest()
    (destination / "sha256.txt").write_text(f"{digest}  flash.bin\n")
    print(f"Verified two matching {FLASH_BYTES}-byte reads. SHA-256 {digest}")
    print(f"Private recovery image: {first}")
    print("Press RESET to exit ROM download mode.")


if __name__ == "__main__":
    main()
