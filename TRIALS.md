# Nearby Buddy bench trials

## Lab stopping point · November 11, 2026

Live trials stopped at the owner's request after the v0.1.3 six-foot scan-window test. The requested 180-degree orientation trial was deferred. Both boards were left running v0.1.3 in fast mode. The student entry point and AI continuation prompt are in [README.md](README.md).

Sections below are dated experiment records. Requests for “next” placements describe the sequence at those checkpoints. The remaining experiments are summarized in [HANDOFF.md](HANDOFF.md). The readme graphics are illustrated synthetic demonstrations, and the measurements below came from the physical boards.


## v0.1.3 scan-window test at six feet, November 11, 2026

The user removed the added interference and explicitly confirmed a measured six-foot gap, flat and screens up. A fresh v0.1.2 comparison at this placement reproduced long fast/fast reception gaps of 6.368 s Blue and 6.000 s Green. A following Blue-fast/Green-eco segment reduced those maxima to 1.015 s and 1.489 s. Both segments lasted approximately 30 seconds. Private logs are `evidence/{blue,green}-6ft-timing-repeat.txt`.

v0.1.3 changes only fast mode's scan window from 60 ms to 100 ms per 100 ms interval, plus the visible version. Advertising remains at 100 ms. Eco remains 250 ms advertising and 45/240 ms scanning. Distance constants, stale timeout, mood thresholds and animation behavior retain their prior values. CLI build and existing host proximity tests passed. Each factory identity was verified immediately before its upload and all written image hashes verified. Both boards reported v0.1.3 build 2026-11-11T18:45:33 and successful advertising/scanning.

At the same six-foot position, 45-second captures produced:

| Receiver | Reports | Raw RSSI p10 / median / p90 | Filtered median | Largest gap within capture | Sad / sleep transitions |
| --- | ---: | --- | ---: | ---: | --- |
| Blue | 387 | -57 / -54 / -52 dBm | -54.2 dBm | 0.522 s | 0 / 0 |
| Green | 403 | -56 / -54 / -51 dBm | -54.1 dBm | 0.317 s | 0 / 0 |

No interval in either capture reached the 1.1-second stale timeout. No raw log sequence records were missing between captured reports. The captured happy/elated transitions reflect the existing uncalibrated thresholds. Green's capture started later because Arduino Serial Monitor had reopened its port; its 45-second run overlapped Blue's run. Both devices remained on and advertising throughout.

This trial supports keeping the full scan window for fast mode. It resolves the observed long gaps in this arrangement; orientation, repeated motion and battery operation still require verification. Increased scan duty may increase power consumption, which has not been measured. Private logs are `evidence/{blue,green}-6ft-v013-fullscan-1.txt`. v0.1.2 compiled images and source snapshots are retained privately under `build/v0.1.2-before-radio-trial/`.

Next physical action requested is rotating Greenface 180 degrees in place at six feet, screen up, while keeping Blueface unchanged.


## Obstructed setup and mixed radio profiles, November 11, 2026

The user clarified that USB cables reach only six feet. The setup initially described as nine feet used added interference to simulate distance. Its actual separation was not measured. It is excluded from distance calibration. Private files were renamed to `evidence/{blue,green}-obstructed-unmeasured-fast-1.txt`; original serial marks remain intact and the correction is in `evidence/obstructed-setup-note.txt`.

The approximately 30-second fast/fast capture had 144 Blue reports and 148 Green reports, raw medians of -51/-48 dBm, and largest within-capture gaps of 5.590/8.199 seconds. Those figures describe this obstructed setup only.

Two approximately 35-second mixed-profile trials followed in the same setup.

| Blue profile / Green profile | Blue reports / Green reports | Largest gap Blue / Green | Sad transitions Blue / Green | Sleep transitions |
| --- | --- | --- | --- | --- |
| fast / eco | 77 / 58 | 1.270 / 1.473 s | 1 / 2 | 0 / 0 |
| eco / fast | 58 / 72 | 1.475 / 1.266 s | 3 / 1 | 0 / 0 |

Mixed timings substantially reduced the longest gaps in both directions, although some gaps still exceeded the 1.1-second stale timeout and caused sad transitions. This supports a scan/advertising timing interaction as a hypothesis. It does not establish a root cause or a stable final profile. Both boards were restored to fast afterward.

During a 15-second laptop Bluetooth scan overlapping the first mixed trial, the Mac received 211 Blue callbacks with a maximum inter-callback gap of 0.324 seconds and 80 Green callbacks with a maximum gap of 1.024 seconds. These host callbacks are distinct from board-to-board reception. The observations show continued advertisements at the laptop's location.

Private mixed-profile logs are `evidence/{blue,green}-mixed-profile-{1,2}.txt`, with host data in `evidence/mac-mixed-profile-1.json`. Next requested physical action is removal of the added interference and return to a measured six-foot gap for radio-timing checks.


## Six-foot trial, November 11, 2026

The user explicitly confirmed six feet between the same facing edges. Both boards remained flat, screens up, USB connected, fast profile and demo disabled. Concurrent captures lasted about 30 seconds.

| Receiver | Reports | Raw RSSI p10 / median / p90 | Filtered median | Largest gap within capture | Sad / sleep transitions |
| --- | ---: | --- | ---: | ---: | --- |
| Blue | 144 | -56 / -55 / -53 dBm | -55.2 dBm | 6.576 s | 2 / 1 |
| Green | 119 | -56 / -55 / -55 dBm | -55.5 dBm | 5.352 s | 3 / 2 |

Fresh status distance medians were 1.30 ft on Blue and 1.335 ft on Green. Receiving behavior was generally happy, with an elated transition also logged on Blue. Both falsely slept during reception gaps. No raw log sequence records were missing between captured reports. Green's final USB-connected gauge reading was 10.00% and 3.731 V.

Private logs are `evidence/{blue,green}-6ft-flat-fast-1.txt`. Next requested placement is nine feet, subject to USB cable reach. The measured -55 dBm median supplies an initial six-foot reference for this orientation, pending repeats and body/orientation effects.


## Six-inch and three-foot trials, November 11, 2026

Both receivers were logged concurrently for approximately 30 seconds at each placement, USB connected, fast profile and demo disabled. The user explicitly confirmed six inches. For three feet, the user's "ok" was interpreted as confirmation of the requested placement and this interpretation was stated before capture. Both retained the flat, screens-up orientation.

| Gap | Receiver | Reports | Raw RSSI p10 / median / p90 | Filtered median | Largest within-capture gap | Sad / sleep transitions |
| --- | --- | ---: | --- | ---: | ---: | --- |
| 6 in | Blue | 249 | -81 / -39 / -38 dBm | -45.0 dBm | 1.712 s | 1 / 0 |
| 6 in | Green | 140 | -80.1 / -39 / -38 dBm | -42.0 dBm | 8.537 s | 1 / 2 |
| 3 ft | Blue | 176 | -47 / -46 / -46 dBm | -46.5 dBm | 4.449 s | 3 / 1 |
| 3 ft | Green | 190 | -48 / -47 / -46 dBm | -47.1 dBm | 3.644 s | 2 / 1 |

Fresh status distance medians were 0.315/0.355 ft at six inches and 0.52/0.54 ft at three feet for Blue/Green. Six-inch raw medians were close to the previous one-foot medians; three-foot medians were weaker. Current distance constants substantially underestimate the three-foot placement. Both danced at six inches. Both entered sleep at three feet during gaps. No raw log sequence records were missing between captured reports.

Private logs are `evidence/{blue,green}-6in-flat-fast-1.txt` and `evidence/{blue,green}-3ft-flat-fast-1.txt`. The final Green gauge reading was 9.74% and 3.732 V over USB. Next requested placement is six feet with matching orientation, subject to cable reach.


## One-inch gap, November 11, 2026

The user confirmed a one-inch gap between the same facing edges, retaining the flat, screens-up orientation. Both boards were USB connected in fast mode. A concurrent 30-second live capture produced the following results.

| Receiver | Reports | Raw RSSI p10 / median / p90 | Filtered median | Largest gap within capture | Sad / sleep transitions |
| --- | ---: | --- | ---: | ---: | --- |
| Blue | 182 | -79 / -27 / -25 dBm | -30.6 dBm | 5.343 s | 4 / 1 |
| Green | 69 | -68.8 / -28 / -26 dBm | -31.0 dBm | 7.541 s | 5 / 4 |

The raw medians are 14–15 dB weaker than the touching trial's -13 dBm medians. Both boards entered dance at one inch. This single orientation provides a candidate signal difference for calibration, but repeated trials and orientation/body effects are still needed to assess contact discrimination. Weak outliers and false separation remain present. No raw log sequence records were missing between captured reports. Green's final USB-connected gauge reading was 9.22% and 3.726 V.

Private logs are `evidence/{blue,green}-1in-flat-fast-1.txt`. Next requested placement is a six-inch gap between the same edges.


## Touching-edge trials, November 11, 2026

After the user confirmed contact, an initial capture was made. The user then said "try now", and a fresh 30-second capture was started. Use the second capture for the current placement comparison. Both boards were flat, screens up, USB connected, fast profile, demo disabled.

| Receiver | Reports | Raw RSSI p10 / median / p90 | Filtered RSSI median | Largest gap within capture | Sad / sleep transitions |
| --- | ---: | --- | ---: | ---: | --- |
| Blue | 191 | -80 / -13 / -13 dBm | -17.5 dBm | 6.507 s | 2 / 2 |
| Green | 167 | -79 / -13 / -13 dBm | -17.3 dBm | 4.393 s | 3 / 2 |

The strong median signal separates this touching arrangement from the earlier one-foot medians near -40 dBm. Weak outliers and multi-second reception gaps persist at contact. Both boards falsely entered sleep. Contact versus a small air gap remains untested. No raw log sequence gaps were observed between captured records. Private logs are `evidence/{blue,green}-touch-flat-fast-2.txt`; the first capture is retained separately as `*-1.txt`.


## Measured one-foot trials, November 11, 2026

The user confirmed both boards flat, screens up, approximately one foot apart, USB connected, with the requested screen information visible. Both receivers were logged concurrently for about 35 seconds per profile. These were live radio trials with demo disabled.

| Profile | Receiver | Reports | Median raw RSSI | Largest gap within capture including edges | Sad / sleep transitions |
| --- | --- | ---: | ---: | ---: | --- |
| fast | Blue | 252 | -41 dBm | 2.929 s | 1 / 1 |
| fast | Green | 108 | -40 dBm | 9.676 s | 3 / 3 |
| eco | Blue | 26 | -41 dBm | 4.080 s | 6 / 6 |
| eco | Green | 31 | -40 dBm | 3.811 s | 6 / 6 |

Fresh fast-profile status estimates had medians of 0.305 ft on Blue and 0.325 ft on Green. Both entered dance at one foot. The current distance constants underestimate this arrangement, and dance does not establish physical contact. Green's raw p10 was -80 dBm in both profiles despite a median near -40 dBm. No raw log sequence gaps appeared between captured records.

Both profiles produced false separation behavior while the boards were stationary. Eco produced more frequent sad/sleep transitions with the present 1.1-second stale timeout. The cause of long reception gaps remains unconfirmed. Both boards were returned to fast mode. Private logs are `evidence/{blue,green}-1ft-flat-{fast,eco}-1.txt`. A touching-edge comparison follows with the same flat orientation.


## First paired reception, November 11, 2026

Blue's pre-replacement recovery image was verified with two matching 4 MiB ROM reads. Blue then booted Nearby Buddy v0.1.2 built at 17:45:05, with all upload hashes verified. Green ran its existing v0.1.2 build at 17:36:14. The following observations used actual BLE traffic with `demo=0`, USB attached, fast profile and unmeasured placement.

- Both receivers matched the enrolled partner. Blue reached 113 peer reports by uptime 30.025 s. Green increased from 198 to 352 peer reports over about 19.7 seconds.
- Blue had a reception gap exceeding ten seconds, followed by fresh reports, then another multi-second gap. Its display state changed among elated, sad and sleep. This requires a stationary repeat with placement controlled before diagnosing the cause.
- Green's reported filtered signal was usually around -42 to -44 dBm, with a brief -55.3 dBm reading. Provisional distance telemetry was commonly 0.33–0.39 ft. Physical separation was unmeasured, so these values provide no calibration evidence.
- Blue gauge telemetry was 4.209 V and 103.20% raw SOC. Its display code clamps percentage to 100%. Green reported 3.651 V and about 3.62% SOC while USB connected.
- The laptop independently received both identity payloads through Bluetooth in a ten-second Bleak scan. Blue produced 29 host reports at -50 to -40 dBm; Green produced 32 at -36 to -34 dBm. These are host callback counts. The receiver positions differ from the board-to-board path.
- Raw USB logs remain local under `evidence/`. The existing host tests passed. Visual confirmation of Blue's screen, controlled one-foot trial, full distance sweep and off-USB trials are pending.


## Baseline, November 11, 2026

Greenface, USB powered on the Narwhal 26 Mac, firmware `NB1`. Arduino IDE verified upload data hashes on the enrolled Greenface. The CLI compiled with Arduino-ESP32 3.3.8 and the Reverse TFT Feather board profile. The host proximity test passed with synthetic RSSI input.

| Trial | Observation |
| --- | --- |
| Fast profile | BLE advertising and scan returned 0. The controller reported 478 to 868 surrounding advertisements between uptime 8.020 s and 14.021 s, about 65 reports/s in this environment. Zero matched the enrolled peer. |
| Eco profile | BLE advertising and scan returned 0. The controller reported 2,208 to 2,293 surrounding advertisements between uptime 78.073 s and 82.073 s, about 21 reports/s. Zero matched the enrolled peer. |
| Synthetic expression tour | Serial demo ran for 18 s and returned to sleep. Dance advanced from 140 to 199 cumulative TFT writes over about two seconds. Longest measured render/write call was 28.207 ms. After sleep settled, the cumulative count stayed at 243. |
| Fuel gauge | The MAX17048 answered over I²C. Early readings on USB were about 3.59 V and 2.05–2.08% state of charge. The charge-rate estimate was positive. |

Surrounding reports are uncontrolled background traffic and show only that the scan received over-the-air reports in both configurations. TFT write timing and state transitions came from firmware telemetry. The user confirmed seeing the green face on the TFT.

### Display and estimate update

Greenface booted v0.1.2 after an IDE upload hash verification. The serial monitor confirmed build time 17:36:14 EST, successful fast-profile starts, a 3.600 V and 2.25% gauge reading on USB, and no matching peer reports. The host test checked the distance formula with synthetic dBm values. The v0.1.2 distance field showed no estimated feet in telemetry because Blueface still had its prior firmware. The live estimate and both-board battery-only readings await the pair test.

## Run the two-board test

1. Verify both identities and the Blueface backup. Upload the same sketch to both boards. Use 115200 baud and New Line. On both boards, enter `status` and check that the correct face name appears, `adv=0 scan=0`, and `peer` rises with the two boards on. Begin in `fast`. Make sure the batteries can sustain a separate unplugged test.
2. For one receiving board at a time, send `raw on`. Enter a mark such as `mark 6ft_flat_1` after arranging each position. Hold each position for 20–30 s and record touching, 1 in, 6 in, 1 ft, 3 ft, 6 ft and 9 ft. Repeat with antennas aligned, boards rotated, both in hands and one body between them. Keep both antenna-to-antenna spacing and case contact in the notes. Mark the last endpoint and send `raw off`. Copy the log locally under `evidence/`.
3. Repeat on the other receiver. Compare both directions with `python3 tools/summarize_rssi.py evidence/blue-fast.txt`. Compare the displayed distance with the tape measure at each position. The RSSI bands are useful if they separate across repeated setups. Report the overlap, false dances without contact, missed dances at contact, and false wakes near six feet. Mark any demo as synthetic. Keep the original RSSI logs.
4. From stationary positions, do at least five approaches and separations. Film the two screens and the motion in one shot at 60 fps or faster. Count frames from crossing the marked line or physical contact to the first visible change on each board. Repeat in `eco`. For an initial feel target, look for a visible response in under 0.75 s on approach and sleepy eyes within about 3 s of sustained separation. Report median and slowest observations. Raw report gaps measure reception cadence. The two board uptime clocks start independently.
5. Compare power with USB disconnected, the same battery on the same board, and comparable charge levels and display state. Test both profiles at fixed separation, then repeat a sleeping-versus-awake run to expose backlight cost. Use a series current logger at the battery if one is available. With the onboard gauge alone, compare state-of-charge change over equal, long battery-only runs and report percentage points per hour as an estimate. Log start/end voltage, state of charge, duration and whether USB was attached. The gauge's rate reading is an estimate of change in state of charge. It provides no direct mA reading.

### Test interpretation

The firmware uses one independent estimate per board. The 100 ms fast advertising interval and 60% scan duty are starting points. The eco profile uses a 250 ms advertising interval and 18.75% scan duty. Its effective report gaps may exceed the first build's 1.1 s stale timeout. Compare packet gaps before calling eco a usable profile. The logs count receiver callbacks and queue/log drops where available. An advertisement has no transmitted sequence number, so missing RF packets and physical-motion latency require separate measurements.

Start by finding the fast profile's useful signal bands in the actual room. If contact and an air gap overlap, use the measured very-close behavior as the dance trigger and document the gap. ESP-NOW RSSI shares the signal-strength ambiguity. Wi-Fi FTM is another ESP32-S3 experiment if distance repeatability warrants it. The installed Arduino 3.3.8 configuration enables the FTM initiator and responder; actual ranging remains untested. BLE Channel Sounding requires newer controller capabilities than the Bluetooth 5 controller in these Feathers.
