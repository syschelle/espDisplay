# espDisplay v0.1.13 build status

## Scope

v0.1.13 changes TM1637 temperature formatting and the timing semantics of alternate display mode.

## Changes verified in this environment

- External air temperature is rounded with `lroundf()` to a whole degree.
- The TM1637 reserves the final digit for a dedicated seven-segment degree symbol.
- Examples covered by tests: `22.49 -> 22°`, `22.50 -> 23°`, `-4.6 -> -5°`.
- Alternate mode starts with the clock.
- The configured alternate value now controls only the clock phase duration.
- The selected API metric is shown for exactly one second between clock phases.
- The existing clock colon blink remains independent of metric refresh timing.
- No persistent settings schema change is required.

## Checks run

- `python3 tools/check_project.py` — PASS
- `tools/test_display_format.cpp` compiled with `-Wall -Wextra -Werror -Wformat-truncation` — PASS
- `tools/test_validation.cpp` compiled with `-Wall -Wextra -Werror` — PASS
- Browser JavaScript syntax with `node --check` — PASS

## PlatformIO

The ESP8266 PlatformIO toolchain is not installed in this execution environment. Before tagging v0.1.13, run:

```bash
pio run -e d1_mini
```

and confirm the build completes without warnings or errors.
