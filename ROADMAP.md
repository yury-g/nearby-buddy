# Prototype roadmap

Firmware v0.1.3 and enclosure v0.1 are educational prototypes. No production readiness is claimed. Priorities are ordered below; OTA and low-power work are not implemented promises.

| Priority | Work | Evidence needed before calling it done |
| --- | --- | --- |
| 1 · Distance | Calibrate RSSI and moods across both receivers, repeated placements, orientations, hands, bodies and environments. Separate obstruction from actual distance. | Held-out measured trials; error distribution; false near/sleep rates; approach/separation latency; contact versus small-gap confusion matrix. Never use the obstructed “9 ft” trial as distance data. |
| 2 · Small batteries | Log actual current, tune display/NeoPixel/radio duty and measure useful runtime. Evaluate smaller battery sizes as well as larger packs. | Battery-only measurements with stated capacity, voltage, brightness, profile and temperature; peak-current and charge-current checks; repeatable runtime; no unsupported mAh-to-hours claims. Eco currently remains experimental. |
| 3 · OTA | Introduce a suitable flash partition scheme, authenticated image delivery, integrity checks, rollback and USB recovery. Decide how updates are initiated and how credentials are provisioned. | Successful and deliberately interrupted updates on both roles; invalid-image rejection; rollback/recovery demonstration. Current TinyUF2 4 MB No OTA layout must be reconsidered before implementation. |
| 4 · Production | Harden pairing, input/state handling, errors, reconnect behavior, update trust and reproducible builds. Validate battery/charging, thermal behavior, enclosure retention, impact and wear. | Documented requirements, failure tests and release process appropriate to the actual shipped product. Prototype enrollment by MAC is not authentication. |

## Mechanical follow-up

Print the rail/key coupon first, then an empty shell/carrier/lid/key. Check all buttons, USB insertion, plug clearance, battery lead length, seams and attachment retention without a battery installed. Fit the specified battery only after the empty checks pass. Repeat the radio calibration with the final enclosure and its intended battery; packaging and body placement can change RSSI. Record actual dimensions, printer/material/settings and any changes.

## First student iteration

Connect two boards by USB, identify them, build the unchanged baseline, preserve their old firmware, enroll the pair and run the demonstration. Then change one visible behavior, such as an expression timing, and compare it with the baseline. Leave live radio calibration and power optimization for separate measured experiments.
