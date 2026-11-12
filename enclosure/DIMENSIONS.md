# Dimension basis and allowances

All dimensions are millimeters. Checked November 11, 2026. The enclosure is designed from vendor CAD and documented dimensions, not estimated from a screenshot.

## Sources

- [Adafruit 5691 downloads](https://learn.adafruit.com/esp32-s3-reverse-tft-feather/downloads) links the [Eagle layout](https://github.com/adafruit/Adafruit-ESP32-S3-Reverse-TFT-Feather-PCB) and [official STEP/STL model](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/5691%20Feather%20ESP32%20S3%20Reverse%20TFT). The bundled STL is unchanged and carries [Adafruit's MIT license](reference/ADAFRUIT-LICENSE.txt). Exact source URL and hashes are in [SOURCES.json](reference/SOURCES.json).
- [Adafruit 3898 product page](https://www.adafruit.com/product/3898) gives approximate 36 × 17 × 7.8 dimensions. We instead use the larger maxima on page 8 of the [PKCELL LP801735 datasheet](https://cdn-shop.adafruit.com/product-files/3898/3898_specsheet_LP801735_400mAh_3.7V_20161129.pdf): **37 × 17.5 × 8.2**, with **25 ± 5 mm leads**. The battery model is a conservative rectangular envelope, not detailed pouch geometry.

## Vendor geometry

Eagle coordinates have USB at x=0. The model is rigidly rotated 180° about X, so the front-facing enclosure uses y = 22.86 − Eagle y and z = 27 − vendor z. This avoids mirroring the physical board.

| Feature | Documented/model value |
| --- | --- |
| PCB | 50.8 × 22.86; corner radius 2.54; model thickness 1.57 |
| Vendor assembled model Z | −1.9 to +6.37 relative to display-side PCB plane |
| Display body model XY | x 10.736…42.397; Eagle y 2.562…20.114 |
| D0/BOOT, D1, D2 centers | Eagle x 7.62; y 4.445, 11.43, 18.415 |
| Front RESET | Eagle (44.45, 11.43), SW4 |
| Rear RESET | Eagle (10.795, 5.334), SW2; duplicates front RESET |
| Left mounting centers | Eagle (2.54,2.54), (2.54,20.32) |
| Right mounting centers | Eagle (48.26,1.8415), (48.26,20.955) |
| JST connector origin | Eagle (10.795,17.653); transformed y 5.207 |
| USB model bounds | x −1.213…6.687; Eagle y 6.96…15.9 |

## Design choices, not vendor dimensions

| Choice | Value / purpose |
| --- | --- |
| Main body | 77 × 47 × 29; integral strap eyelet extends overall shell length to 86 |
| Battery bay | 52 × 27 × 12 at x14…66, y−2…25, z4.5…16.5 |
| Larger target battery | Up to 50 × 25 × 10, leaving nominal assembly clearance; fit alone does not establish electrical suitability |
| Board edge clearance | 0.3 per side; front-plane z27; solid pads support mounting annuli with 0.15 vertical clearance |
| Internal insulating deck | z17…18.5, 1.5 thick; wire opening x4…17, y−8…7 |
| Small battery position | x14.5…51.5, y0…17.5, z8.3…16.5 |
| Battery retention | Removable battery-safe foam adhesive, nominal 0.5 thick between pouch and deck; never clamp the pouch |
| Mating plug allowance | 9 × 7 × 6 reserve at x6.3, y−6.8, z20.8; a design allowance pending actual plug measurement |
| Rear RESET access | 5 mm aligned hole through lid and carrier; use a blunt nonconductive tool, front RESET remains directly accessible |
| Sliding joint | 0.25 clearance per side; 45° shoulders; removable diamond-section lock pin |
| Lock key | 2.6 square section, 49 long plus 2-thick head; 0.25 per-side hole allowance; tether hole in head |

The battery blocks exclude leads and connector. The small-pack conceptual wire centerline is about 20 mm before allowing bend slack. A lead at the datasheet minimum of 20 mm may require a compatible short extension; confirm polarity at both ends. Actual lead bend, insulation and plug insertion must be checked with the real pack. A larger battery may need different lead length. Headers, FeatherWings and attached sensor boards are outside this fit model.
