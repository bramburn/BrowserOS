# `npm/scripts/` — postinstall downloader

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. One file: the `postinstall` hook of the npm package.

## What's here

`postinstall.js` downloads the released `browseros-cli` binary for the
current platform from the GitHub release, verifies it against
`checksums.txt`, extracts it into `../.binary/`, and chmods it. It is
the npm path to the exact same artefacts that `../../Makefile release`
builds and `../../update/manifest.go` downloads at runtime.

## Rules

### SCR1 — The npm version is the release version
`VERSION` is read from `../package.json`, and the URL is
`releases/download/browseros-cli-v${VERSION}/...`. Version skew means a
404 at install time for every user. `make npm-version VERSION=x` in
`../../Makefile` is what keeps the two in sync.

### SCR2 — Checksum verification is mandatory when `checksums.txt` is reachable
The archive and the checksum file are fetched in parallel. If the
checksum file is present, a mismatch is a hard `process.exit(1)`. If it
can't be fetched at all, the script **warns and continues** — that is a
deliberate liveness trade-off, so don't tighten it into a hard failure
without checking the CDN.

### SCR3 — CI opt-out, forced opt-in
`process.env.CI && !process.env.BROWSEROS_NPM_FORCE` exits 0 before any
network call. `../bin/browseros-cli.js` sets `BROWSEROS_NPM_FORCE=1`
when it needs to self-heal. Both halves are required.

### SCR4 — Extraction is a shell-out on purpose
Windows uses `powershell -Command "Expand-Archive ..."`, everything else
`tar -xzf`. There is no Node zip/tar dependency, by design (rule BIN1 in
[`../bin/AGENTS.md`](../bin/AGENTS.md) is zero-dependency). The temp
archive is deleted after extraction, and the binary's presence is
re-checked before success is reported.

### SCR5 — Platform maps Node names to Go names
`PLATFORM_MAP` (`win32 → windows`) and `ARCH_MAP` (`x64 → amd64`). These
must produce the exact same key as `PlatformKey` in
`../../update/manifest.go` and the `PLATFORMS` list in
`../../Makefile`. Change all three together.

### SCR6 — Extraction of `.binary/` is disposable
`../.binary/` is gitignored by the repo and excluded by `../.npmignore`.
Never write anything outside it, and never expect it to survive a
`npm ci`.

## Workflows

- **Running it manually:** `BROWSEROS_NPM_FORCE=1 node scripts/postinstall.js`
  (with `CI` unset). It prints which version it fetched and whether the
  checksum verified.
- **A user reports a 404 on install:** the `package.json` version has no
  matching `browseros-cli-v<version>` GitHub release. Re-run
  `make npm-version` and `make npm-publish`, or the release.
- **Adding a platform:** extend `PLATFORM_MAP` / `ARCH_MAP` here, add the
  pair to `PLATFORMS` in `../../Makefile`, and to `PlatformKey` in
  `../../update/manifest.go`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the npm package.
- [`../bin/AGENTS.md`](../bin/AGENTS.md) — the launcher that re-runs this script.
- [`../../update/AGENTS.md`](../../update/AGENTS.md) — the in-binary update path for the same assets.
- [`../../scripts/AGENTS.md`](../../scripts/AGENTS.md) — the curl/PowerShell installer alternative.
