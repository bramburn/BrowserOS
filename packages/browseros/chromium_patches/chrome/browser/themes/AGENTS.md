# `themes/` — light default and the BrowserOS first-run theme

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

Two diffs, one behaviour change. `theme_service.cc` flips the default browser
colour scheme from `kSystem` to `kLight` for both `kBrowserColorScheme` and the
deprecated `kDeprecatedBrowserColorSchemeDoNotUse`, and calls
`browseros::SyncDefaultTheme(profile_->GetPrefs())` as the first statement of
`ThemeService::Init()` — which applies the BrowserOS blue tonal-spot theme on
first run if the user has not customised anything. `BUILD.gn` adds
`//chrome/browser/browseros/core:prefs` to the `themes` target's deps.

## Contents

```
themes/
├── theme_service.cc   ← RegisterProfilePrefs(): kDeprecatedBrowserColorSchemeDoNotUse
│                         kSystem → kLight (SYNCABLE_PREF)
│                       kBrowserColorScheme: kSystem → kLight
│                       Init(): browseros::SyncDefaultTheme(profile_->GetPrefs());
└── BUILD.gn           ← "themes" target deps += //chrome/browser/browseros/core:prefs
```

## Rules

**TH1 — `SyncDefaultTheme()` is a no-op once the user has customised the
theme.** That is the whole contract; it is why it can be called unconditionally
from `Init()` rather than from first-run code. Do not "fix" the placement —
`Init()` is early enough that the very first paint uses the BrowserOS theme
and late enough that user prefs are already loaded.

**TH2 — Both colour-scheme prefs must stay in step.**
`kBrowserColorScheme` and the deprecated
`kDeprecatedBrowserColorSchemeDoNotUse` are both flipped to `kLight`, and the
deprecated one is `SYNCABLE_PREF`. Changing only one leaves upgraded profiles
resolving the colour scheme from the deprecated value and getting `kSystem`
back.

**TH3 — `kLight` is a fork-wide default, not a per-profile choice.** It is a
registered default; an existing user value still wins. Do not add code that
overrides a stored value.

**TH4 — The `BUILD.gn` dep is `:prefs`, not `:core`.** `theme_service.cc`
includes `chrome/browser/browseros/core/browseros_prefs.h`, whose
implementation lives in the `prefs` target. The `action_utils` target is not
needed here.

**TH5 — `SyncVerticalTabsPref()` is a sibling one-shot in the same header.**
It is *not* called from this file. Do not move either call site into the other
directory; they have different lifetimes (theme: once at `Init()`; vertical
tabs: at controller init, per the header comment).

## Workflows

**Changing the default colour scheme**
1. Edit both `RegisterIntegerPref` values in
   `ThemeService::RegisterProfilePrefs()` in `theme_service.cc`.
2. Keep the deprecated pref's `SYNCABLE_PREF` flag.
3. Do not touch `SyncDefaultTheme()` — that is the BrowserOS brand theme, a
   separate concern from the light/dark/system mode.

**Changing the BrowserOS first-run theme**
1. Edit `SyncDefaultTheme()` in
   `../browseros/core/browseros_prefs.cc`; do not edit the call site.
2. Keep the "user has not customised it" guard, otherwise a user's explicit
   theme choice is overwritten on every start.
3. Re-extract both the header and `.cc` diffs if either changed.

**Verifying the theme applies on a fresh profile**
1. Delete the profile's `Preferences`.
2. Start BrowserOS; `ThemeService::Init()` runs `SyncDefaultTheme()` before
   `InitFromPrefs()`.
3. The first rendered window should be the BrowserOS tonal-spot theme in light
   mode.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../browseros/core/AGENTS.md`](../browseros/core/AGENTS.md) —
  `SyncDefaultTheme()` and `SyncVerticalTabsPref()`.
- [`../prefs/AGENTS.md`](../prefs/AGENTS.md) — profile pref registration.
- [`../../app/AGENTS.md`](../../app/AGENTS.md) — `BRANDING.*` under
  `chromium_files/chrome/app/theme/`, the other half of the branding.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
