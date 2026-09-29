# `chrome/browser/ui/profiles/` — profile error dialog suppression

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

One patch: `profile_error_dialog.cc`. BrowserOS comments out the
`chrome::ShowWarningMessageBox` call that Chromium uses for the
profile-in-use / profile-load-failed warning, replacing it with a
`FIXME: nikhil` note.

## Contents

```
profiles/
└── profile_error_dialog.cc   ← the
      // chrome::ShowWarningMessageBox(gfx::NativeWindow(), ..., message_id)
      call is commented out with a FIXME
```

## Rules

**PR1 — This is a deliberate product decision, not dead code.** The warning
box is suppressed; the dialog flow around it still runs. Do not "clean up"
the commented block by deleting it, and do not re-enable the call without a
product decision.

**PR2 — The FIXME is unresolved.** Whoever removes this suppression should
replace the native warning with an in-page or non-blocking notice rather
than restoring a modal `ShowWarningMessageBox`.

**PR3 — Changes here are `misc`-feature changes.** The file is listed under
the `misc` block in `packages/browseros/build/features.yaml`, so a
modification belongs in that feature's commit, not a UI one.

**PR4 — Check the sibling before editing.**
`chrome/browser/profiles/chrome_browser_main_extra_parts_profiles.cc` is
patched under the `metrics` feature and touches adjacent profile startup
code; do not merge concerns across the two.

## Workflows

**Restoring a profile warning**
1. Uncomment the `ShowWarningMessageBox` call, passing the localized
   `IDS_PROFILE_ERROR_DIALOG_TITLE` and the `message_id`.
2. Prefer a non-modal surface: the surrounding `ProfileErrorDialog` class
   already has the profile context available.
3. Remove the `FIXME` only once a replacement exists.

**Reviewing a diff in this directory**
Confirm the change is intentional UX suppression and is filed under the
`misc` feature in `build/features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../../profiles/chrome_browser_main_extra_parts_profiles.cc`](../../profiles/chrome_browser_main_extra_parts_profiles.cc)
  — adjacent profile startup patch.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — `packages/browseros/`.
