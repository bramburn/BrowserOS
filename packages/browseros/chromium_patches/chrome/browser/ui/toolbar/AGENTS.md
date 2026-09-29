# `chrome/browser/ui/toolbar/` — default pinned actions, toolbar model

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

The model layer under the toolbar: the pref names for the pinned-action
migration, the default-pinned-action list, the pinned-extension model, and
the macOS app-menu update indicator severity.

## Contents

```
toolbar/
├── toolbar_pref_names.{cc,h}   ← default_pinned_actions list: Chrome Labs removed;
│                                 kActionSidePanelShowThirdPartyLlm appended when
│                                 kThirdPartyLlmPanel, kActionSidePanelShowClashOfGpts
│                                 when kClashOfGpts.  .h adds:
│                                   toolbar.pinned_third_party_llm_migration_complete
│                                   toolbar.pinned_clash_of_gpts_migration_complete
├── toolbar_actions_model.cc    ← IsBrowserOSPinnedExtension() short-circuits the
│                                 pinned check; BrowserOS extensions that are marked
│                                 pinned are appended to the force-pinned list
├── app_menu_icon_controller.cc  ← macOS: update indicator severity raised to
│                                  Severity::kMedium ("show update indicator sooner")
└── pinned_toolbar/             ← see pinned_toolbar/AGENTS.md
```

## Rules

**TB1 — The default pinned list is a feature-guarded sequence, not a
set.** `kThirdPartyLlmPanel` and `kClashOfGpts` blocks append to
`default_pinned_actions` in that order. Reordering changes the toolbar
layout for every new profile.

**TB2 — Migration prefs are one-shot and must not be deleted.**
`kPinnedThirdPartyLlmMigrationComplete` and
`kPinnedClashOfGptsMigrationComplete` record that a profile has been moved
to the new pinned-toolbar container. Deleting them re-runs the migration on
existing profiles.

**TB3 — Chrome Labs is not pinned by default.** That is a BrowserOS product
decision, changed here *and* in
`pinned_toolbar/pinned_toolbar_actions_model.cc`. Both sites must agree or
new and migrated profiles get different toolbars.

**TB4 — `IsBrowserOSPinnedExtension(action_id)` short-circuits before the
normal pinned check.** This is what lets a BrowserOS extension be pinned
by its ID rather than by a user action. Keep the early return first.

**TB5 — Extension pinning is additive and idempotent.** The loop over
`browseros::GetBrowserOSExtensionIds()` appends only IDs not already in
`pinned`, gated on `IsBrowserOSPinnedExtension`. Do not replace `pinned`
wholesale; other code holds references to it.

**TB6 — `app_menu_icon_controller.cc` is macOS-only behaviour** even
though the file is cross-platform; the change is inside a macOS path and
`enable_sparkle`-adjacent. Confirm platform guards before editing.

## Workflows

**Adding a new default-pinned BrowserOS action**
1. Add the action ID in `../actions/chrome_action_id.h`.
2. Append it in `toolbar_pref_names.cc` inside a `FeatureList` guard, after
   the existing two.
3. Add a migration pref in `toolbar_pref_names.h` if the action needs to be
   moved for existing profiles.
4. Add the coordinator + entry so the button has something to show.

**Pinning a BrowserOS extension by default**
1. Mark it in the BrowserOS extension set
   (`chrome/browser/browseros/core/browseros_constants.h`).
2. `toolbar_actions_model.cc` picks it up via
   `GetBrowserOSExtensionIds()` + `IsBrowserOSPinnedExtension()`.
3. Add a migration pref so existing profiles get the button too.

**Raising the update indicator frequency (macOS)**
Edit the severity return in `app_menu_icon_controller.cc`. The
`mac-sparkle-updater` feature in `build/features.yaml` owns this file.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`pinned_toolbar/AGENTS.md`](pinned_toolbar/AGENTS.md) — the reactive
  model that mirrors these prefs.
- [`../views/toolbar/AGENTS.md`](../views/toolbar/AGENTS.md) — the view-side
  button and label rendering.
- [`../side_panel/AGENTS.md`](../side_panel/AGENTS.md) — the panel entry IDs
  appended here.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
