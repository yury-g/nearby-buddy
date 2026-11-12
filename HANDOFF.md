# Nearby Buddy handoff

## Saved stopping point · November 11, 2026

The owner stopped live testing to prepare a student-lab handoff. Both original boards run **v0.1.3**, compiled `2026-11-11T18:45:33`. Both were left in fast mode. The proposed 180-degree orientation trial was deferred.

Fast uses 100 ms advertising and a 100/100 ms scan window. Eco uses 250 ms advertising and 45/240 ms scanning. In the latest measured six-foot trial, each board completed 45 seconds with no sad or sleep transitions. Maximum report gaps were 0.522 s Blue and 0.317 s Green. The build, host tests, upload hashes and live version checks passed.

Read [README.md](README.md) for the student entry point and AI prompt and [TRIALS.md](TRIALS.md) for measurements. The visual tour uses synthetic sequencing and firmware-rendered faces.

## Original hardware

| Role | Factory MAC | Last normal USB port |
| --- | --- | --- |
| Blueface | b8:f8:62:d5:8b:8c | /dev/cu.usbmodem1101 |
| Greenface | ac:27:6e:b1:cb:b0 | /dev/cu.usbmodem101 |

Both are Adafruit Feather ESP32-S3 Reverse TFT boards with 135×240 screens and MAX17048 gauges. Ports may change. Normal VID/PID is 239a:8123. ROM USB is 303a:1001, with uppercase factory MAC observed in its USB serial. The retired Green b8:f8:62:d5:80:14 is different hardware.

Blue was verified live as BatteryFaces-v27-seesaw-gamepad before replacement. Green already ran Nearby Buddy when this continuation began. The owner confirmed the colored faces and requested screen information on both boards.

## Recovery and upload notes

Blue's original 4 MiB flash was read twice and both images matched SHA-256
`12a0d70a8cdc55fd013c0723e65fc4a0a9a6b9d169e683d33800e0f137a0f16d`.
The original backup and manifest remain privately with the owner and are not distributed. Restore has not been bench-tested. Create your own identity-verified backup before replacing firmware on different boards.

The original `tools/backup_flash.py` used a RAM flasher stub and failed during a read. Later stub reads stalled. Treat that helper as needing repair before reuse.

The successful backup used esptool 5.2.0's Python API in a local virtual environment. A 1200-baud open/close entered ROM mode. The working sequence was `ESP32S3ROM(port, 115200)`, `connect('usb-reset', attempts=2)`, factory verification through `read_mac()`, `cmds.attach_flash()`, and two `cmds.read_flash(esp, 0, 0x400000, output, flash_size='4MB', no_progress=True)` calls on one connection. It used the ROM loader with no stub. Both files were checked for 4 MiB length and equal SHA-256 before writing a manifest. Explicit flash size was necessary.

The same ROM-only Python API wrote the normal Arduino components after rechecking factory identity. Each written image hash verified. Watchdog reset returned normal USB. Application USB remained USB-OTG with CDC enabled, TinyUF2 4 MB No OTA and full-chip erase disabled. Uploads included bootloader, partitions, boot_app0, application and TinyUF2. Those components replaced their previous flash contents.

Arduino IDE also uploaded successfully with UART0 / Hardware CDC upload mode when the board was in ROM mode. Its Serial Monitor sometimes reopened Green's port after resets. Close the competing monitor before script access. Native serial reads and uploads worked in this continuation; earlier permission failures are historical.

Ignored `build/serial-venv/` contains esptool 5.2.0, pyserial and Bleak. Previous v0.1.2 binaries and source snapshots remain privately in `build/v0.1.2-before-radio-trial/`.

## Evidence and open work

- v0.1.2 stationary trials had long reception gaps and false sleep. Mixed fast/eco profiles improved gaps. v0.1.3 isolated the fast scan-window change and removed false sleep in the latest six-foot test.
- Distance and mood thresholds still need calibration. The formula underestimates measured distances. Earlier firmware danced at one foot.
- The measured sweep covered touching, one inch, six inches, one foot, three feet and six feet. Three feet was inferred from the user's “ok”, and that assumption was stated before capture.
- There was no measured nine-foot trial. The user added interference because USB cables reached six feet. Those logs were reclassified and excluded from distance calibration.
- Laptop Bluetooth received both enrolled advertisements. Packets are non-connectable; USB carries commands.
- Both gauges responded. Green's captured reading near the final upload was about 11.81% and 3.742 V on USB. No current-in-mA measurement or battery-only runtime test occurred.
- Resume orientation, body/hand, repeated approach/separation, contact/small-gap and battery-only experiments when the lab is ready.

## Public handoff

Canonical student repository: https://github.com/yury-g/nearby-buddy

Firmware was extracted from the Nearby Buddy chapter of OpenToy at checkpoint `0c180b040405f25727e124708ffced5c0bc79efb`. The MIT-licensed renderer is included locally. No access to another private repository is needed.

The November 11 documentation continuation added the prototype roadmap and source-based enclosure. Firmware behavior was unchanged, and no new hardware trial or upload was performed. CAD fit is digitally checked; physical print and fit are pending. See [enclosure/README.md](enclosure/README.md) and [ROADMAP.md](ROADMAP.md).
