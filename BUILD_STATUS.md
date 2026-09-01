# Build / validation status — espDisplay v0.1.21

## Completed in the artifact environment

- project static/API/web checks: PASS
- JavaScript syntax (`node --check`): PASS
- browser SHA-256 regression vector: PASS (part of project checks)
- display formatter C++ tests with `-Wall -Wextra -Werror -Wformat-truncation`: PASS
- host validation C++ tests with `-Wall -Wextra -Werror`: PASS
- compact API server status presentation guard: PASS
- canonical release-document hygiene guard: PASS
- release workflow canonical `RELEASE_NOTES.md` guard: PASS

## Repository release-document policy

Only the current source version keeps these two release documents in the repository root:

- `BUILD_STATUS.md`
- `RELEASE_NOTES.md`

Historical `BUILD_STATUS_v*.md` and `RELEASE_NOTES_v*.md` files are intentionally removed. Published GitHub Releases are the historical release record.

## Additional validation

- GitHub Actions workflow YAML parse: PASS
- packaged ZIP re-validation: performed after packaging

## Not available in this environment

PlatformIO (`pio`) is not installed, so the real ESP8266 firmware build was not executed here.

Before tagging the release run:

```text
pio run -e d1_mini
```

The PlatformIO build and GitHub Actions result are the authoritative ESP8266 compile/link checks.
