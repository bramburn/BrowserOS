# `chrome/browser/ui/startup/` — startup infobar removal

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

One patch: `infobar_utils.cc`. It deletes the Google-API-keys infobar
("Google API keys are missing") from `AddInfoBarsIfNecessary`, leaving the
other startup infobars (obsolete-system warning) untouched.

## Contents

```
startup/
└── infobar_utils.cc   ← removes:
      if (!google_apis::HasAPIKeyConfigured()) {
        GoogleApiKeysInfoBarDelegate::Create(infobar_manager);
      }
```

## Rules

**SU1 — This is a deletion patch; there is nothing to add here.** The
BrowserOS intent is "no API-key infobar, ever". The equivalent
`google_api_keys_infobar_delegate.cc` file is also listed under
`chromium-ui-fixes` in `build/features.yaml` and carries further changes.

**SU2 — Do not remove the surrounding infobars.** The
`ObsoleteSystem::IsObsoleteNowOrSoon()` block that follows is upstream
Chromium behaviour and is intentionally retained.

**SU3 — `infobar_manager` is still used by later blocks in the function.**
Deleting the local `infobar_manager` declaration along with the removed
`if` breaks the build; the patch only removes the `if` body.

**SU4 — Owned by the `misc` feature block** in
`packages/browseros/build/features.yaml` together with
`browser/ui/profiles/profile_error_dialog.cc` and
`chrome/installer/mini_installer/chrome.release`. Keep related
"silence the browser" changes together.

## Workflows

**Suppressing another startup infobar**
1. Delete its `if` block from `AddInfoBarsIfNecessary` in this file.
2. Check whether the delegate's own `.cc` also needs a patch (e.g.
   `google_api_keys_infobar_delegate.cc` under `chromium-ui-fixes`).
3. File both under the same feature block in `build/features.yaml`.

**Verifying infobar layout after a change**
The infobar's height is set in
`chrome/browser/ui/views/chrome_layout_provider.cc`
(`DISTANCE_..._INFOBAR`), which BrowserOS also patches (non-refresh
infobars use `2 * 3` padding). See
[`../views/AGENTS.md`](../views/AGENTS.md).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../views/AGENTS.md`](../views/AGENTS.md) — infobar height/padding.
- [`../../chrome_browser_main.cc`](../../chrome_browser_main.cc) — the other
  startup behaviour (first-run tab, iCloud manifest).
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
