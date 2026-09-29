# `scripts/` — CDN installer scripts

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. Two files: the copy-paste installers served from
> `https://cdn.browseros.com/cli/`.

## What's here

`install.sh` (bash, macOS/Linux) and `install.ps1` (PowerShell,
Windows) — the `curl … | bash` / `irm … | iex` install path for
`browseros-cli` itself. They download a **released** archive from the
CDN, extract the single binary, and put it somewhere on `PATH`. They
are not build scripts; the Go build is `../Makefile`. The two differ
in two places worth knowing before you edit them: only `install.sh`
verifies SHA-256, and only `install.ps1` edits the user `PATH` (see
SCR5 and SCR6).

## Contents

```
scripts/
├── install.sh    ← bash, default $HOME/.browseros/bin,
│                   --version / --dir flags, env-var equivalents
└── install.ps1   ← PowerShell 5.1+, default %LOCALAPPDATA%\browseros-cli\bin,
                    -Version / -Dir params, BROWSEROS_VERSION / BROWSEROS_DIR env
```

## Rules

### SCR1 — They must survive `| bash` and `| iex` execution
Both are written for the stdin-piped case. `install.sh` uses `set -euo
pipefail` and manual `while [[ $# -gt 0 ]]` parsing because `$0` and
`$1` don't survive the pipe. `install.ps1` reads its options from
`$env:BROWSEROS_VERSION` / `$env:BROWSEROS_DIR` *in addition to* its
`param()` block, because `param()` is ignored under `irm | iex`. Keep
both paths working; the env-var branch is the one most users hit.

### SCR2 — Version must be validated before interpolation into a URL
Both reject anything not matching `^\d+\.\d+\.\d+(-…)?$` before building
`$CdnBase/v$VERSION/...`. Don't relax it — the value is interpolated
into a download path.

### SCR3 — Windows must force TLS 1.2 and detect ARM64 honestly
`install.ps1` sets `[Net.ServicePointManager]::SecurityProtocol = Tls12`
(PS 5.1 defaults to TLS 1.0) and prefers
`[System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture`
over `$env:PROCESSOR_ARCHITECTURE`, which lies under x64 emulation on
ARM64. It also rejects 32-bit Windows outright.

### SCR4 — Artefact names must match `../Makefile` and `../npm/`
`browseros-cli_${VERSION}_${platform}_${arch}.tar.gz` / `.zip`,
platforms `darwin|linux|windows`, arches `amd64|arm64`, served under
`$CDN_BASE/v$VERSION/`. `../Makefile`'s `release` target and
[`../npm/scripts/postinstall.js`](../npm/scripts/postinstall.js) build
the exact same names. Change all three together.

### SCR5 — `install.sh` verifies the checksum; `install.ps1` does not
This asymmetry is real, not an oversight in the docs. `install.sh`
fetches `${CDN_BASE}/v$VERSION/checksums.txt`, matches on the archive
filename, and compares against `sha256sum` or `shasum`; a missing
`checksums.txt`, a missing entry, or a missing hash tool degrades to a
warning. `install.ps1` downloads the zip and `Expand-Archive`s it
without any checksum step. If you close the gap, do it in
`install.ps1` first — it is the one that is missing the check. Keep
verification ahead of the move into the install dir in both scripts;
both extract to a temp dir (`mktemp -d` / `[Path]::GetTempPath()` with
a `finally` cleanup) and only then move the binary into place.

### SCR6 — PATH handling differs by platform; keep it that way
`install.ps1` **mutates** the user `Path` registry value via
`[Environment]::SetEnvironmentVariable("Path", ..., "User")` and tells
the user to restart the terminal. `install.sh` deliberately does not
edit `~/.zshrc` or `~/.bashrc`; it prints the `export PATH=…` line to
add. Adding a mutation to the bash script changes behaviour for macOS
and Linux users — check before mirroring.

### SCR7 — Keep them dependency-free
`curl` + `tar` on POSIX; `Invoke-WebRequest` + `Expand-Archive` on
Windows. No jq, no Python, no bootstrapping.

## Workflows

- **Adding a new release artefact:** update the name template in both
  scripts, `PLATFORMS` in `../Makefile`, and the maps in
  [`../npm/scripts/postinstall.js`](../npm/scripts/postinstall.js).
- **A user's installer fails silently:** check the TLS 1.2 line first on
  Windows PowerShell 5.1 — that's the most common cause, and the script
  sets it explicitly for that reason.
- **Testing without touching the filesystem or the network:**
  `install.sh --version abc` and `install.ps1 -Version "abc"` both fail
  the semver format check in SCR2 and exit 1 immediately. `install.sh
  --help` prints usage and exits 0. Anything with a valid-looking
  version proceeds to a real CDN download, so don't use one as a smoke
  test in CI.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../Makefile`](../Makefile) — builds the archives these scripts install.
- [`../update/AGENTS.md`](../update/AGENTS.md) — the in-binary update path, same CDN.
- [`../npm/scripts/AGENTS.md`](../npm/scripts/AGENTS.md) — the npm install path.
- [`../README.md`](../README.md) — the user-facing install section that quotes these URLs.
