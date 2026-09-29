# `resources/settings/reset_page/` — reset feedback opt-in

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

One line of one file. `reset_profile_dialog.html` removes the `checked`
attribute from the `<cr-checkbox id="sendSettings">` in the dialog's `footer`
slot, so "Send feedback" is now opt-in rather than pre-checked. The label
(`$i18nRaw{resetPageFeedback}`) and the rest of the dialog are unchanged.

## Contents

```
reset_page/
└── reset_profile_dialog.html   ← <cr-checkbox id="sendSettings" checked>
                                 → <cr-checkbox id="sendSettings">
```

## Rules

**RS1 — This is a default flip, not a removal.**
The `<cr-checkbox>` element still exists and still carries
`id="sendSettings"`; only the `checked` attribute is gone. Deleting the element
would break `reset_profile_banner` / the reset handler that reads the checked
state.

**RS2 — The element `id` is a contract.**
`sendSettings` is referenced by the C++ reset-profile handler and by
`reset_profile_banner`. Renaming it desynchronises the dialog from the handler
with no compile error.

**RS3 — Keep the hunk to one line.** A one-attribute diff against an upstream
file is the cheapest thing to re-apply on the next Chromium bump. Do not
"improve" the surrounding markup in the same change.

**RS4 — The label is `$i18nRaw{}`, not `$i18n{}`.** That is upstream's choice
(the string contains markup) and this patch does not change it. Do not
"normalise" it.

**RS5 — No `BUILD.gn` entry is needed.** `reset_profile_dialog.html` is an
existing upstream template included by
`../settings_main/settings_main.ts` (`import '../reset_page/reset_profile_banner.js';`
and the dialog's own import). The fork only edits it in place.

## Workflows

**Changing what is pre-selected in the reset dialog**
1. Edit the attribute on `<cr-checkbox id="sendSettings">` in
   `reset_profile_dialog.html`.
2. Keep the `id` (RS2).
3. Re-extract; the patch should be a single changed line.

**Verifying the behaviour**
1. Open `chrome://settings/clearBrowserData` → "Reset settings".
2. Confirm "Send feedback" is unticked by default.
3. Ticking it and confirming must still reach the reset handler; if it does
   not, the `id` was changed.

**Checking that the reset still completes**
1. Reset with the checkbox unticked.
2. Confirm the profile is reset and no feedback is submitted.
3. This patch does not affect the reset itself — only the feedback opt-in.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `settings/` app, routes and exports.
- [`../settings_main/AGENTS.md`](../settings_main/AGENTS.md) — the container
  that imports the reset banner.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
