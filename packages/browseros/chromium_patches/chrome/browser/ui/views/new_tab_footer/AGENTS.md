# `chrome/browser/ui/views/new_tab_footer/` — footer suppression

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

One patch: `footer_controller.cc`. It makes
`ShouldShowExtensionFooter()` unconditionally return `false`, disabling
Chromium's new-tab-page footer entirely.

## Contents

```
new_tab_footer/
└── footer_controller.cc   ←
      - if (ShouldSkipForErrorPage()) return false;
      - return ntp_footer::IsExtensionNtp(...) && kNTPFooterExtensionAttributionEnabled
        && kNtpFooterVisible;
      + return false;
```

## Rules

**NF1 — The whole footer is off, not just the extension footer.** The
function short-circuits to `false` before any of Chromium's conditions
(including the error-page skip). Do not re-introduce a partial condition.

**NF2 — The pref default is separately forced off.** The companion patch in
[`../../webui/new_tab_footer/new_tab_footer_ui.cc`](../../webui/new_tab_footer/new_tab_footer_ui.cc)
changes `kNtpFooterVisible` from `true` to `false` at registration. Both
halves are needed: the code change covers existing profiles, the pref
change covers anything that reads the pref directly.

**NF3 — `UpdateFooterVisibilities()` is untouched** and still runs. Do not
delete it; other code observes the state it maintains.

**NF4 — Owned by the `chromium-ui-fixes` feature** in
`packages/browseros/build/features.yaml`, in the same block as the
`chrome/browser/ui/views/new_tab_footer/` and
`chrome/browser/ui/webui/new_tab_footer/` directory entries.

## Workflows

**Re-enabling the new-tab footer**
1. Restore the original body in `footer_controller.cc`.
2. Set `kNtpFooterVisible` back to `true` in
   `chrome/browser/ui/webui/new_tab_footer/new_tab_footer_ui.cc`.
3. Check `kNTPFooterExtensionAttributionEnabled` in
   `chrome/common/pref_names.h` is registered.
4. Both files belong to the same feature block — change them together.

**Debugging "footer still visible"**
1. Confirm the `return false` is the *only* remaining statement in
   `ShouldShowExtensionFooter()`.
2. Check whether a different footer path (`ContentsViewFooterController`)
   is drawing it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../../webui/new_tab_footer/AGENTS.md`](../../webui/new_tab_footer/AGENTS.md)
  — the pref-default half of this change.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
