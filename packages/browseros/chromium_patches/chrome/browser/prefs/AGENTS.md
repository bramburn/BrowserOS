# `prefs/` — where BrowserOS prefs get registered

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

Chromium's central pref-registration entry points, and the four lines that
wire every BrowserOS preference into them. `browser_prefs.h` gains
`RegisterBrowserOSPrefs(user_prefs::PrefRegistrySyncable*)`;
`browser_prefs.cc` implements it as a one-line delegation to
`browseros::RegisterProfilePrefs()`, calls it from
`RegisterProfilePrefs()`, and additionally calls
`browseros_server::RegisterLocalStatePrefs(registry)` and
`browseros_metrics::RegisterLocalStatePrefs(registry)` from
`RegisterLocalState()`. `browser_prefs.cc` also gains
`browseros_metrics::RegisterProfilePrefs(registry)` inside
`RegisterProfilePrefs()`. `BUILD.gn` adds
`//chrome/browser/browseros/core:prefs` to the `impl` target's deps.

## Contents

```
prefs/
├── browser_prefs.h   ← +void RegisterBrowserOSPrefs(user_prefs::PrefRegistrySyncable*);
├── browser_prefs.cc  ← +#include browseros/core/browseros_prefs.h
│                       +#include browseros/server/browseros_server_prefs.h
│                       +#include browseros/metrics/browseros_metrics_prefs.h
│                       RegisterLocalState():        + browseros_server::RegisterLocalStatePrefs
│                                                     + browseros_metrics::RegisterLocalStatePrefs
│                       RegisterProfilePrefs():      + browseros_metrics::RegisterProfilePrefs
│                                                     + RegisterBrowserOSPrefs
│                       +void RegisterBrowserOSPrefs() { browseros::RegisterProfilePrefs(registry); }
└── BUILD.gn          ← "impl" target deps += //chrome/browser/browseros/core:prefs
```

## Rules

**PREF1 — A BrowserOS pref that is not registered here does not exist.**
There are exactly four registration calls, and they are the only path from a
`browseros::prefs::k*` constant to a `PrefService`. Adding a constant without
its registration call produces a pref that reads back as the default and
silently ignores writes.

**PREF2 — Profile prefs and Local State prefs are different registries.**
`RegisterBrowserOSPrefs()` and `browseros_metrics::RegisterProfilePrefs()` take
`user_prefs::PrefRegistrySyncable*` (per-profile, potentially syncable);
`browseros_server::RegisterLocalStatePrefs()` and
`browseros_metrics::RegisterLocalStatePrefs()` take `PrefRegistrySimple*`
(machine-wide). Putting a per-machine port in the profile registry — or a
per-profile UI pref in Local State — is a real bug, not a style issue.

**PREF3 — `RegisterBrowserOSPrefs()` is a delegation shim, not a
re-implementation.** Its whole body is
`browseros::RegisterProfilePrefs(registry);`. Keep it that way: the pref
definitions belong in `../browseros/core/browseros_prefs.cc`, and this file
should not learn what they are.

**PREF4 — The new `#include`s are in upstream's include groups.**
`chrome/browser/browseros/core/browseros_prefs.h` and
`.../server/browseros_server_prefs.h` sit in the `chrome/` block;
`.../metrics/browseros_metrics_prefs.h` sits in the `components/` block where
Chromium already has it out of order. Do not re-sort the file.

**PREF5 — The `BUILD.gn` dep is on `:prefs`, not `:core`.**
`source_set("impl")` needs `//chrome/browser/browseros/core:prefs` because that
is the target holding `browseros_prefs.cc`. The server and metrics pref
targets are reached transitively through the callers; adding them here without
need widens the dependency edge.

## Workflows

**Registering a new BrowserOS profile pref**
1. Add the constant in `../browseros/core/browseros_prefs.h` and register it
   in `../browseros/core/browseros_prefs.cc` (`browseros::RegisterProfilePrefs`).
2. Nothing further is needed here — `RegisterBrowserOSPrefs()` already
   forwards (PREF3).
3. If the WebUI must see it, add a `settingsPrivate` allowlist row in
   `../extensions/api/settings_private/prefs_util.cc`.

**Registering a new BrowserOS Local State pref**
1. Add the registration function beside the existing one in
   `../browseros/server/browseros_server_prefs.{h,cc}` or
   `../browseros/metrics/browseros_metrics_prefs.{h,cc}`.
2. Call it from `RegisterLocalState()` in `browser_prefs.cc`, grouped with the
   two existing BrowserOS calls.
3. Add the include to the matching group (PREF4).
4. Extract the two diffs and list them under one `features.yaml` block.

**Confirming a pref is actually live**
1. `chrome://version` → profile path; check `Local State` / `Preferences`.
2. Or set it via `--enable-features`-free path:
   `chrome://settings` for allowlisted prefs, `browserOS.setPref` for
   `browseros.*` (see `../extensions/api/browser_os/AGENTS.md`).
3. If neither shows it, the registration call is missing (PREF1).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../browseros/core/AGENTS.md`](../browseros/core/AGENTS.md) — the
  `browseros::prefs::k*` constants and `RegisterProfilePrefs`.
- [`../browseros/server/AGENTS.md`](../browseros/server/AGENTS.md) —
  `RegisterLocalStatePrefs()` for the server ports.
- [`../browseros/metrics/AGENTS.md`](../browseros/metrics/AGENTS.md) —
  both metrics registration functions.
- [`../extensions/api/settings_private/AGENTS.md`](../extensions/api/settings_private/AGENTS.md)
  — exposing registered prefs to the WebUI.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
