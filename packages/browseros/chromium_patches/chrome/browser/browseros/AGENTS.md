# `browseros/` — BrowserOS-owned browser-process code

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

This is the fork's own C++ — the only subtree under `chrome/browser/` where
BrowserOS authors whole files rather than nudging Chromium ones. It holds the
shared constants/prefs/switches vocabulary (`core/`), the bundled-extension
installer stack (`extensions/`), PostHog analytics (`metrics/`), the
`browseros_server.exe` process manager with its health check, OTA updater and
MCP-port IPC proxy (`server/`), the separate browser OTA service
(`update/`), and the CRX payload copied into the build output
(`bundled_extensions/`). There are no `.cc`/`.h` files directly in
this directory — only `BUILD.gn`, which groups the subdirectories.

Everything here is **new-file** patch content (`new file mode 100644` in the
diff header), i.e. Chromium has no counterpart for any of it.

## Contents

```
browseros/
├── BUILD.gn                  ← group("browseros") deps on :core, :metrics,
│                               :server, :browseros_update; plus
│                               group("browseros_bundled_extensions")
├── core/                     ← browseros_constants.h (extension IDs, virtual
│                               chrome://browseros/* routes, update URLs),
│                               browseros_switches.h, browseros_prefs.{h,cc},
│                               browseros_action_utils.h  (see core/AGENTS.md)
├── extensions/               ← loader / installer / maintainer for the bundled
│                               Agent, Bug Reporter and Controller CRX files
├── metrics/                  ← BrowserOSMetrics (static Log API) + the
│                               per-profile PostHog KeyedService
├── server/                   ← BrowserOSServerManager singleton: launches,
│                               health-checks, restarts and OTA-updates the
│                               bundled sidecar binary (see server/AGENTS.md)
│   └── test/                 ← gmock fakes for the manager's injected interfaces
├── update/                   ← 25 files: the OTA check/download/verify/install
│                               service (checker, downloader, manifest, proxy,
│                               service, verifier + win/stub variants). Has no
│                               AGENTS.md of its own yet.
└── bundled_extensions/       ← BUILD.gn that copies the .crx payload into the
                                output dir (CRX/JSON are build-fetched, not
                                committed)
```

## Rules

**BOS1 — This is the only place new BrowserOS C++ lives.** If a change needs a
brand-new file rather than a modification to a Chromium file, it belongs under
`chrome/browser/browseros/`. Changes to Chromium-owned files stay in their own
directory (e.g. `../extensions/`, `../themes/`).

**BOS2 — `group("browseros")` is the only aggregate.** Its deps are
`//chrome/browser/browseros/core`, `//chrome/browser/browseros/metrics`,
`//chrome/browser/browseros/server` and
`//chrome/browser/browseros/update:browseros_update`. Note `extensions/` and
`bundled_extensions/` are *not* in that group — the latter gets its own
`group("browseros_bundled_extensions")`. Adding a new subdirectory means
adding it to its `source_set` *and* to an aggregate here; a `source_set` that
nothing depends on is never compiled and will rot.

**BOS3 — Extension identity is centralised in `core/browseros_constants.h`.**
`kBrowserOSExtensions[]` (id + flags), `kAgentExtensionId`,
`kBugReporterExtensionId`, `kControllerExtensionId`, `kBrowserOSConfigUrl`,
`kBrowserOSUpdateUrl`, `kBrowserOSAlphaConfigUrl` and
`kBrowserOSAlphaUpdateUrl` all live there. Never hard-code an extension ID or a
BrowserOS CDN URL in another directory — add it to that table and include the
header.

**BOS4 — `.crx` and `bundled_extensions.json` are build inputs, never
committed here.** `bundled_extensions/.gitignore` ignores `*.crx` and `*.json`;
the `copy()`/`bundle_data()` target in that directory's `BUILD.gn` runs at GN
time. Editing the CRX list means editing the `_bundled_extensions_sources` list
in that `BUILD.gn`, which names the CRX by extension ID.

**BOS5 — Everything is a `new file mode` patch.** Do not turn one of these
into a diff against a Chromium file; if a same-path file starts existing
upstream, that is a Chromium-version bump problem to raise, not silently
rebase.

**BOS6 — Pref registration is centralised.** Profile prefs go through
`core/browseros_prefs.h:RegisterProfilePrefs` (called from
`../prefs/browser_prefs.cc:RegisterBrowserOSPrefs`); metrics prefs through
`metrics/browseros_metrics_prefs.h`; server ports through
`server/browseros_server_prefs.h:RegisterLocalStatePrefs`. A BrowserOS pref
registered anywhere else is invisible to the WebUI `settingsPrivate` allowlist.

## Workflows

**Adding a new bundled BrowserOS extension**
1. Add the CRX to the build inputs in `bundled_extensions/BUILD.gn`
   (`_bundled_extensions_sources`) with the extension ID as the filename.
2. Add its ID (and flags: pinned / labelled / contextual side panel) to
   `kBrowserOSExtensions[]` in `core/browseros_constants.h`.
3. If it needs a virtual `chrome://browseros/*` route, add a row to
   `kBrowserOSURLRoutes[]` in the same header.
4. Re-run `browseros dev extract` for each touched path and add them under the
   right feature block in `../../../../build/features.yaml`.

**Adding a BrowserOS command-line switch**
1. Add `inline constexpr char kXxx[] = "browseros-xxx";` to the relevant
   section of `core/browseros_switches.h`.
2. Read it with `base::CommandLine::ForCurrentProcess()->HasSwitch(kXxx)`.
3. Add a `#if` guard only if the switch can only be honoured on one platform
   (the Sparkle switches are macOS-only but are defined unconditionally).

**Changing server behaviour**
1. Edit under `server/`, run `browseros dev extract
   chrome/browser/browseros/server/<file>`.
2. `server/BUILD.gn` lists sources explicitly — a new file must be added to
   `source_set("server")` there.
3. Add the path to the `server` feature block in `../../../../build/features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`core/AGENTS.md`](core/AGENTS.md) — constants, switches, prefs, action utils.
- [`server/AGENTS.md`](server/AGENTS.md) — sidecar process manager, updater, proxy.
- [`extensions/AGENTS.md`](extensions/AGENTS.md) — bundled-extension loader.
- [`metrics/AGENTS.md`](metrics/AGENTS.md) — PostHog analytics service.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.
