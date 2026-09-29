# `resources/settings/about_page/` — the de-Googled about page

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The `chrome://settings/help` page, with Google-specific surface removed. The
`about_page.html` diff deletes the product-title line, the two "learn more"
links to `support.google.com/chrome`, the `onProductLogoClick_` handler, and
the product-logo click binding; it adds a `BrowserOS - $i18n{aboutBrowserOSVersion}`
line above the Chromium version line and loosens `.info-sections` line-height
to 1.8. `about_page.ts` changes one handler: `onHelpClick_()` now does
`window.open('http://docs.browseros.com/')` instead of
`this.aboutBrowserProxy_.openHelpPage()`.

## Contents

```
about_page/
├── about_page.ts    ← onHelpClick_(): aboutBrowserProxy_.openHelpPage()
│                       → window.open('http://docs.browseros.com/')
└── about_page.html  ← - product title div, - 2 support.google.com links,
                       - onProductLogoClick_ binding, + "BrowserOS - {version}",
                       .info-sections line-height 1.8, .info-section rule dropped
```

## Rules

**AB1 — The docs URL is a literal in `about_page.ts`.** There is no pref, no
build arg, no `loadTimeData` key. Changing the docs destination means editing
that one line; do not introduce a constant elsewhere and expect it to be used.

**AB2 — The URL is `http://`, not `https://`, in this patch.** If that is
upgraded, it is a deliberate change with a review, not a drive-by.

**AB3 — `aboutBrowserOSVersion` is a load-time string, not a computed
version.** `$i18n{aboutBrowserOSVersion}` resolves from the generated
`strings` module. The actual version number comes from the build; the label
does not.

**AB4 — The removed logo click binding and its handler go together.**
`onProductLogoClick_` is a known-about-flags affordance. The `.html` binding
and the intent are both removed upstream-side; if a future change restores one,
restore both.

**AB5 — Deleting a `$i18n{}` reference does not delete the string.**
`aboutLearnMoreUpdatingErrors`, `aboutLearnMoreUpdating` and `aboutProductTitle`
remain in the strings template; unused entries are harmless. Do not edit
`chrome/app/chromium_strings.grd` to "clean them up" as part of a page change.

**AB6 — The layout tweak (line-height 1.8, removal of the `.info-section`
margin rule) is cosmetic but load-bearing for the shortened page.** With the
title and links removed, the remaining two-line rows need the extra leading to
fill the space. Reverting it leaves a visibly cramped page.

## Workflows

**Changing the help link**
1. Edit `onHelpClick_()` in `about_page.ts`.
2. Keep it a direct `window.open(...)`; the `aboutBrowserProxy_` path goes to
   Google's help centre.
3. Re-extract; the `.ts` patch should stay one line.

**Adding a version line to the about page**
1. Add the `<div class="secondary">…</div>` in `about_page.html` alongside the
   existing `$i18n{aboutBrowserVersion}` line.
2. Use a `$i18n{}` key that already exists in the strings template, or add the
   key there in the same change.
3. Do not reintroduce `$i18n{aboutProductTitle}` (AB5).

**Checking the page after a change**
1. Open `chrome://settings/help`.
2. Confirm no `support.google.com` link remains anywhere on the page.
3. Confirm both the `BrowserOS -` and Chromium version lines render.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `settings/` app, routes and exports.
- [`../../../../app/AGENTS.md`](../../../../app/AGENTS.md) — `BRANDING.*` files that
  feed `chrome://theme/current-channel-logo`.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
