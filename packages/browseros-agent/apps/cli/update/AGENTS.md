# `update/` — self-update

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. Implements `browseros-cli update` and the silent
> background check that runs alongside every other command.

## What's here

The self-update subsystem: fetch a release manifest, pick the asset for
the running `GOOS`/`GOARCH`, download it, verify its SHA-256, extract
the single binary from the archive, and replace `os.Executable()`.
`Manager` is the only stateful type; everything else is a pure function
over `Manifest` / `Asset` / `State`. `cmd/root.go` constructs a Manager
around *every* command invocation and `cmd/update.go` drives the
interactive `update` command.

## Contents

```
update/
├── manager.go     ← Options, Manager, NewManager, CachedNotice,
│                    AutomaticEnabled, ShouldCheck,
│                    StartBackgroundCheck, CheckNow, Apply,
│                    FormatNotice, recordError, saveAppliedState
├── manifest.go    ← Manifest, Asset, FetchManifest, Validate,
│                    NormalizeVersion, IsReleaseVersion,
│                    CompareVersions, PlatformKey, SelectAsset
├── archive.go     ← DownloadAsset, ExtractBinary, tar.gz + zip readers
├── apply.go       ← CheckPermissions, VerifyChecksum, ApplyBinary
├── state.go       ← State, StatePath, LoadState, SaveState, IsStale
└── *_test.go      ← one per source file, all with httptest + t.TempDir
```

## Rules

### UPD1 — `Options` is fully injectable; never read globals in this package
`Options` carries `ManifestURL`, `CheckTTL`, `HTTPTimeout`,
`DownloadTimeout`, `JSONOutput`, `Debug`, `Automatic`, `HTTPClient`, and
`Now func() time.Time`. `NewManager` fills defaults for each zero value.
Tests depend on this — when you add a knob, give it a zero-value default
in `NewManager` and inject it in the test rather than adding a package
global or calling `time.Now()` directly.

### UPD2 — Every download is size-limited and every archive must hold exactly one file
`maxManifestSize` (1 MiB) is applied with `io.LimitReader` in
`FetchManifest`; `maxAssetSize` (64 MiB) and `maxBinarySize` (256 MiB) in
`archive.go`. Both tar.gz and zip readers error on a second regular
file and on a missing one. Keep these limits — a compromised or
half-written CDN must not be able to exhaust the CLI's memory.

### UPD3 — The binary is replaced, never the archive
`Apply` runs `DownloadAsset → VerifyChecksum → ExtractBinary →
CheckPermissions(os.Executable()) → ApplyBinary`. `CheckPermissions`
runs *before* the write so a read-only install gets "reinstall with the
installer script" instead of a half-applied update. Don't reorder these.

### UPD4 — Non-release builds never self-update
`IsReleaseVersion` is a `semver.Canonical` check on the ldflags-injected
`version`. `AutomaticEnabled` and `CheckNow` both refuse anything that
isn't a real semver, so the default `"dev"` from `main.go` disables the
whole subsystem. Don't add a path that bypasses it.

### UPD5 — Automatic checks are opt-out and near-silent
`AutomaticEnabled` requires `Automatic: true`, **not** `JSONOutput`, no
`BROWSEROS_SKIP_UPDATE_CHECK`, an install method other than
npm/brew/homebrew (`BROWSEROS_INSTALL_METHOD`), and a release version.
`StartBackgroundCheck` returns a `<-chan struct{}` that
`cmd/root.go` drains for at most 150 ms
(`automaticUpdateDrainTimeout`) after the command finishes. A check
that blocks longer than that will be abandoned — keep it fast.

### UPD6 — State is written atomically to `config.Dir()/update-state.json`
`SaveState` writes to a `os.CreateTemp` file in the same directory and
`os.Rename`s it into place, so a crash mid-write can't truncate the
state. Use `StatePath()`; don't build the path yourself. Note the
`State` is a value copy on write (`*state = *m.state`) so a failed check
preserves `LatestVersion` and only sets `CheckError`.

### UPD7 — Errors are wrapped with `%w` and surfaced verbatim
`Apply`'s permission failure wraps the underlying error and appends the
recovery instruction. `FetchManifest` reports the HTTP status.
Users see these strings directly — keep them actionable.

## Workflows

### "Adding a new platform to the release matrix"
1. `PlatformKey` in `manifest.go` allow-lists `{darwin,linux,windows} ×
   {amd64,arm64}`; extend the switch and add the key to the manifest
   `assets` map contract.
2. Add the pair to `PLATFORMS` in `../Makefile` so `make release`
   builds it, and bump `HOST_EXT` handling for Windows `.exe`.
3. Add a `PlatformKey` case to `manifest_test.go`.
4. Mirror the change in `../npm/scripts/postinstall.js` (`PLATFORM_MAP`
   / `ARCH_MAP`) and in both `../scripts/install.*`.

### "Changing when we nag about updates"
1. `CachedNotice` is read *before* the command runs; `StartBackgroundCheck`
   is started alongside it. Both live in `Manager`.
2. Any new condition belongs in `AutomaticEnabled` so `update --check`,
   the JSON path, and the background check all agree.
3. The notice text is `FormatNotice` — it branches on
   `BROWSEROS_INSTALL_METHOD` to print the right upgrade command.

### "Testing this package"
1. One test file per source file, `package update`, table-driven where
   there's a variant (see `manifest_test.go`).
2. Serve manifests and archives with `httptest.NewServer` and point
   `Options.ManifestURL` at it.
3. Redirect `config.Dir()` with `t.Setenv("XDG_CONFIG_HOME", t.TempDir())`
   so `SaveState` doesn't touch the real config directory.
4. Freeze time with `Options.Now` rather than sleeping.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `update` command and the background hook in `Execute()`.
- [`../config/AGENTS.md`](../config/AGENTS.md) — `Dir()`, where `update-state.json` lives.
- [`../Makefile`](../Makefile) — the release matrix this manifest must match.
- [`../npm/scripts/AGENTS.md`](../npm/scripts/AGENTS.md) — the npm download path, same assets.
- [`../scripts/AGENTS.md`](../scripts/AGENTS.md) — CDN installer scripts.
