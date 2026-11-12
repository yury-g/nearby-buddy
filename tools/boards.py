#!/usr/bin/env python3
"""Discover Nearby Buddy boards by identity and use the installed Arduino CLI."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

IDENTITIES = {
    "blue": "b8:f8:62:d5:8b:8c",
    "green": "ac:27:6e:b1:cb:b0",
}
FQBN = "esp32:esp32:adafruit_feather_esp32s3_reversetft"
CHAPTER = Path(__file__).resolve().parents[1]


def discover():
    result = subprocess.run(
        ["arduino-cli", "board", "list", "--json"],
        check=True, capture_output=True, text=True,
    )
    return json.loads(result.stdout).get("detected_ports", [])


def resolve(color, detected):
    matches = []
    for record in detected:
        port = record.get("port", {})
        properties = port.get("properties", {})
        serial = properties.get("serialNumber", "").lower()
        vendor = properties.get("vid", "").lower()
        if serial == IDENTITIES[color] and vendor == "0x239a":
            matches.append(port["address"])
    if len(matches) != 1:
        raise RuntimeError(
            f"{color}: expected one USB board with serial {IDENTITIES[color]}, "
            f"found {len(matches)}. Check cabling and boot mode."
        )
    return matches[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="action", required=True)
    commands.add_parser("list")
    commands.add_parser("doctor")
    port = commands.add_parser("port")
    port.add_argument("color", choices=IDENTITIES)
    monitor = commands.add_parser("monitor")
    monitor.add_argument("color", choices=IDENTITIES)
    build = commands.add_parser("build")
    build.add_argument("sketch", type=Path)
    args = parser.parse_args()

    if not shutil.which("arduino-cli"):
        raise RuntimeError("arduino-cli is missing from PATH.")
    if args.action == "build":
        sketch = args.sketch.resolve()
        if not sketch.is_dir() or not (sketch / (sketch.name + ".ino")).is_file():
            raise RuntimeError("Pass an Arduino sketch directory containing its matching .ino file.")
        destination = CHAPTER / "build" / sketch.name
        destination.mkdir(parents=True, exist_ok=True)
        return subprocess.call([
            "arduino-cli", "compile", "--fqbn", FQBN,
            "--build-path", str(destination), str(sketch),
        ])

    detected = discover()
    if args.action in ("port", "monitor"):
        address = resolve(args.color, detected)
        if args.action == "port":
            print(address)
            return 0
        return subprocess.call([
            "arduino-cli", "monitor", "--port", address,
            "--config", "baudrate=115200",
        ])

    failed = False
    for color in IDENTITIES:
        try:
            address = resolve(color, detected)
            print(f"{color}: {address}  serial={IDENTITIES[color]}")
            if args.action == "doctor":
                descriptor = os.open(address, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
                os.close(descriptor)
                print("  Serial open succeeded; no commands or reset were sent.")
        except (RuntimeError, OSError) as error:
            failed = True
            print(f"  {error}", file=sys.stderr)
    if args.action == "doctor":
        subprocess.run(["arduino-cli", "version"], check=True)
        subprocess.run(["arduino-cli", "core", "list"], check=True)
        print("This check does not verify firmware behavior or upload ability.")
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (RuntimeError, subprocess.CalledProcessError, OSError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
