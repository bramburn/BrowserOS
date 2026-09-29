# `chrome/browser/ui/` — Browser, commands, actions, toolbar model

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The `chrome/browser/ui` layer: the `Browser` class and its window registry,
command dispatch, the app-menu action tree, the toolbar action model and
default-pinned list, keyboard accelerators, and the two BrowserOS UI feature
flags. This is the "shell" that the subdirectories below hang off.

## Contents

```
ui/
├── BUILD.gn                     ← adds webui/settings/browseros_metrics_handler.* and
│                                  (mac) webui/help/sparkle_version_updater_mac.* to
│                                  static_library("ui")
├── ui_features.{cc,h}           ← kThirdPartyLlmPanel, kClashOfGpts (both enabled by default)
├── browser.{cc,h}               ← CreateParams::hidden, is_hidden(), hidden_tab_pins_,
│                                  PinHiddenTabVisibility()
├── browser_list.{cc,h}          ← ShouldShowBrowserInUserInterface(), GetUserVisibleBrowsers()
├── browser_finder.cc            ← hidden Browsers excluded from find-any
├── browser_actions.cc           ← adds SidePanelAction entries for kThirdPartyLlm and
│                                  kClashOfGpts, each behind its FeatureList guard
├── browser_command_controller.cc← case arms for the 4 BrowserOS IDC_* commands
├── browser_commands.cc          ← Copy URL → chrome://browseros/* virtual URL
├── browser_ui_prefs.cc          ← hover-card pref off, home button on, split-tab pinned
├── browser_unittest.cc          ← IsHiddenReflectsCreateParams
├── accelerator_table.cc         ← Cmd/Ctrl+Shift+K, Cmd/Ctrl+Shift+L, Alt+A behind
│                                  features::kBrowserOsKeyboardShortcuts
└── (subdirs: actions/, browser_window/, cocoa/, extensions/, omnibox/,
    profiles/, side_panel/, startup/, tabs/, toolbar/, views/, webui/)
```

## Rules

**UI1 — Enumerate Browsers with `GetUserVisibleBrowsers()`.** Any new site
that lists windows (tab search, window switcher, drag-and-drop, extensions
API, automation) must use `BrowserList::GetUserVisibleBrowsers()` or
`ShouldShowBrowserInUserInterface(browser)` so hidden agent workspaces stay
out of the user's UI. `BrowserList::GetInstance()` includes them.

**UI2 — `Browser::is_hidden()` is decided at construction and never
changes.** `Browser::CreateParams::hidden` is copied into a `const bool
is_hidden_`. There is no setter; to change visibility you must create a new
Browser.

**UI3 — A new command ID needs a `case` here.** `browser_command_controller.cc`
is the single dispatch point for `IDC_*`. An ID added in
`chrome/app/chrome_command_ids.h` without a `case` here is silently inert.

**UI4 — Every panel registration is `FeatureList`-guarded.** Both
`browser_actions.cc` and `browser_window_features.cc` check
`base::FeatureList::IsEnabled(features::kThirdPartyLlmPanel)` /
`kClashOfGpts` before creating anything. Un-guarded registration leaves a
dangling coordinator when the flag is off.

**UI5 — `ui_features.cc` defaults are `FEATURE_ENABLED_BY_DEFAULT`.** Both
BrowserOS panels ship on. Turning one off is a flag flip, not a code
removal — keep the code path compiling with the flag off.

**UI6 — `browser_unittest.cc` must be updated when `Browser` changes.**
`IsHiddenReflectsCreateParams` constructs Browsers with
`DeprecatedCreateOwnedForTesting`; adding a field to `CreateParams` without
a test here leaves the new path uncovered.

**UI7 — Accelerators are duplicated per platform.** `accelerator_table.cc`
covers Windows/Linux; `browser/global_keyboard_shortcuts_mac.mm` covers
macOS. Both are behind `features::kBrowserOsKeyboardShortcuts`.

## Workflows

**Wiring a new toolbar/app-menu action**
1. Add the action ID in `actions/chrome_action_id.h`.
2. Add a side-panel entry ID in `side_panel/side_panel_entry_id.h` if it is
   a panel.
3. Register it in `browser_actions.cc` under a `FeatureList` guard.
4. Dispatch its `IDC_*` in `browser_command_controller.cc`.
5. Add it to the default pinned list in
   `toolbar/toolbar_pref_names.cc`.
6. Map it for the customize-toolbar UI in
   `webui/side_panel/customize_chrome/customize_toolbar/`.

**Adding an accelerator**
1. Add the binding in `accelerator_table.cc` (non-mac).
2. Add the equivalent chord in `browser/global_keyboard_shortcuts_mac.mm`.
3. Guard both with `features::kBrowserOsKeyboardShortcuts` (Alt+letter
   chords conflict with some keyboard layouts).

**Exposing a Browser to automation safely**
1. Confirm the code does not iterate `BrowserList::GetInstance()`.
2. If it must, filter with `ShouldShowBrowserInUserInterface`.
3. Add a test to `browser_unittest.cc` if you touched `Browser` itself.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/`.
- [`actions/AGENTS.md`](actions/AGENTS.md) — action ID registry.
- [`toolbar/AGENTS.md`](toolbar/AGENTS.md) — default pinned list and
  toolbar action model.
- [`side_panel/AGENTS.md`](side_panel/AGENTS.md) — side-panel entry IDs and
  prefs.
- [`views/AGENTS.md`](views/AGENTS.md) and
  [`webui/AGENTS.md`](webui/AGENTS.md) — the views and WebUI trees.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.
