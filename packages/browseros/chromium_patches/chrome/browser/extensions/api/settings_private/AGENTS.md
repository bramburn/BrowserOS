# `extensions/api/settings_private/` — the pref allowlist

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

`prefs_util.cc` builds `PrefsUtil::GetAllowlistedKeys()` — the map that decides
which Chrome preferences `chrome.settingsPrivate` may read and write. Two
BrowserOS additions: five `browseros::prefs::k*` keys are exposed to the WebUI,
and the ChromeOS-only import dialog gets `kImportDialogExtensions` and
`kImportDialogCookies` rows. Everything else in this diff is unchanged.

## Contents

```
settings_private/
└── prefs_util.cc   ← 5 browseros::prefs::k* rows (kString/kBoolean) + 2
                      import-dialog rows inside #if BUILDFLAG(IS_CHROMEOS)
```

## Rules

**SP1 — This map is a security boundary, not a convenience list.** Adding a
key here makes that preference readable *and writable* by any extension with
the `settingsPrivate` permission. Only add a pref the WebUI genuinely needs.

**SP2 — The pref type must match the registration.** Each row is
`(*s_allowlist)[<key>] = settings_api::PrefType::k<T>;`. Getting `kString` for
a boolean pref produces a runtime cast failure in the WebUI, not a compile
error. The current BrowserOS rows are `kProviders`/`kCustomProviders` as
`kString` (both JSON-in-a-string) and `kShowToolbarLabels` / `kShowLLMChat` /
`kShowLLMHub` as `kBoolean`.

**SP3 — The pref must already exist in
`../../browseros/core/browseros_prefs.h`.** This file only maps; the constant
and its registration live there. A row referencing a key that is not
registered is a no-op that looks like it works.

**SP4 — The two import-dialog rows are ChromeOS-only.** They sit inside
`#if BUILDFLAG(IS_CHROMEOS)` next to `kImportDialogSearchEngine`. Moving them
out of the guard changes behaviour on every other platform.

**SP5 — Include ordering follows Chromium's clang-format groups.** The patch
adds `chrome/browser/browseros/core/browseros_prefs.h` in the `chrome/`
include block and `chrome/browser/browseros/metrics/browseros_metrics_prefs.h`
in the `components/` block (as upstream has it). Do not "tidy" these into
alphabetical order across the whole file.

## Workflows

**Exposing a new `browseros.*` pref to the WebUI**
1. Register the pref in `../../browseros/core/browseros_prefs.{h,cc}`.
2. Add the `(*s_allowlist)[browseros::prefs::kX] = settings_api::PrefType::…;`
   row in `GetAllowlistedKeys()` next to the other BrowserOS rows.
3. If the WebUI page needs it, add the binding in
   `../../resources/settings/browseros_prefs_page/`.
4. Extract and update the same `features.yaml` block.

**Verifying a pref is actually reachable**
1. Confirm the constant exists in `browseros_prefs.h`.
2. Confirm the allowlist row's `PrefType` matches the registered type.
3. Confirm the WebUI element binds `pref="{{prefs.browseros.x}}"`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/api/` group.
- [`../../../browseros/core/AGENTS.md`](../../../browseros/core/AGENTS.md) — where
  the `browseros::prefs::k*` constants are declared and registered.
- [`../../../resources/settings/browseros_prefs_page/AGENTS.md`](../../../resources/settings/browseros_prefs_page/AGENTS.md)
  — the WebUI that consumes the exposed prefs.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
