# `chrome/browser/ui/webui/new_tab_footer/` — footer pref default

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

One patch: `new_tab_footer_ui.cc`. It changes the default of
`prefs::kNtpFooterVisible` from `true` to `false` at pref registration.

## Contents

```
new_tab_footer/
└── new_tab_footer_ui.cc   ←
      - registry->RegisterBooleanPref(prefs::kNtpFooterVisible, true);
      + registry->RegisterBooleanPref(prefs::kNtpFooterVisible, false);
```

## Rules

**NFW1 — This is one half of a two-file change.** The other half is
[`../../views/new_tab_footer/footer_controller.cc`](../../views/new_tab_footer/footer_controller.cc),
which makes `ShouldShowExtensionFooter()` return `false` unconditionally.
The pref default covers new profiles; the code change covers existing ones
and any other caller of the pref.

**NFW2 — Changing the default here does not un-hide an existing profile.**
Chromium does not re-register a pref over a stored value. Existing profiles
keep whatever they had; only the controller change affects them.

**NFW3 — Owned by the `chromium-ui-fixes` feature** in
`packages/browseros/build/features.yaml`, which lists
`chrome/browser/ui/webui/new_tab_footer/` as a directory entry. The views
half is in the same block.

## Workflows

**Re-enabling the new-tab footer**
1. Restore `true` in this file.
2. Restore the original body in
   `chrome/browser/ui/views/new_tab_footer/footer_controller.cc`.
3. Both files are in the same feature block — change them in one commit.

**Debugging "footer shows for a fresh profile"**
Confirm the controller change is applied, not just the pref default; the
controller is what governs rendering.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/webui/`.
- [`../../views/new_tab_footer/AGENTS.md`](../../views/new_tab_footer/AGENTS.md)
  — the code half of this change.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
