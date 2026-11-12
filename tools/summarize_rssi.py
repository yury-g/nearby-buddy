#!/usr/bin/env python3
"""Summarize raw Nearby Buddy USB telemetry for one receiver at a time."""
import argparse
from collections import Counter
from pathlib import Path
import re
import statistics

RECORD = re.compile(
    r"NB1 (?:MARK ms=(?P<mark_ms>\d+) (?P<label>[^\r\n]+)"
    r"|RSSI n=(?P<count>\d+) ms=(?P<ms>\d+) dbm=(?P<raw>-?\d+) filtered=(?P<filtered>-?[\d.]+)"
    r"|STATE ms=\d+ (?P<mood>\w+) demo=(?P<demo>[01]))"
)


def percentiles(values):
    if not values:
        return "—"
    if len(values) == 1:
        return f"{values[0]:g}"
    points = statistics.quantiles(values, n=10, method="inclusive")
    return f"{points[0]:.1f} / {points[4]:.1f} / {points[8]:.1f}"


def segments(data):
    label, start, samples, states = "unmarked", None, [], Counter()
    for line in data:
        match = RECORD.search(line)
        if not match:
            continue
        if match["label"]:
            if start is not None or samples:
                yield label, start, int(match["mark_ms"]), samples, states
            label, start = match["label"].strip(), int(match["mark_ms"])
            samples, states = [], Counter()
        elif match["count"]:
            samples.append((
                int(match["count"]), int(match["ms"]), int(match["raw"]), float(match["filtered"])
            ))
        elif match["demo"] == "0":
            states[match["mood"]] += 1
    if start is not None or samples:
        yield label, start, None, samples, states


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path, help="Text copied from a single receiver's Serial Monitor")
    args = parser.parse_args()
    with args.log.open(errors="replace") as log:
        for label, start, end, samples, states in segments(log):
            print(f"\n{label}: {len(samples)} matching reports")
            print("  State changes:", ", ".join(f"{m}={n}" for m, n in states.items()) or "—")
            if not samples:
                continue
            count, time, raw, filtered = zip(*samples)
            gaps = [b - a for a, b in zip(time, time[1:]) if b >= a]
            missing = sum(max(0, b - a - 1) for a, b in zip(count, count[1:]))
            duration = end - start if start is not None and end is not None else time[-1] - time[0]
            print("  Raw dBm p10 / median / p90:", percentiles(raw))
            print("  Filtered dBm p10 / median / p90:", percentiles(filtered))
            print("  Report gaps ms p10 / median / p90:", percentiles(gaps))
            print("  Missing raw log records between visible sequence numbers:", missing)
            if duration > 0:
                print(f"  Captured reports per second: {len(samples) * 1000 / duration:.2f}")


if __name__ == "__main__":
    main()
