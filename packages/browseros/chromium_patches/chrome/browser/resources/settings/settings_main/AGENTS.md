# `resources/settings/settings_main/` — the settings view container

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The `<cr-view-manager>` that hosts every `chrome://settings` sub-page, one
`<div slot="view">` per route. The fork adds one such div for
`BROWSEROS_PREFS` and one side-effect import. `settings_main.html` appends the
new `<div slot="view" id="browserosPrefs">` containing a
`<template is="dom-if" if="[[renderPlugin_(routes_.BROWSEROS_PREFS,
lastRoute_, inSearchMode_)]]">` that instantiates
`<settings-browseros-prefs-page>`; `settings_main.ts` adds
`import '../browseros_prefs_page/browseros_prefs_page.js';` and removes one
stray blank line.

## Contents

```
settings_main/
├── settings_main.html   ← + <div slot="view" id="browserosPrefs">
│                            <template is="dom-if"
│                              if="[[renderPlugin_(routes_.BROWSEROS_PREFS,
│                                                 lastRoute_, inSearchMode_)]]">
│                              <settings-browseros-prefs-page role="main" …>
├── settings_main.ts     ← + import '../browseros_prefs_page/browseros_prefs_page.js';
│                          - one blank line after the SettingsPlugin import
```

## Rules

**SM1 — The `dom-if` predicate is the routing contract.**
`renderPlugin_(routes_.X, lastRoute_, inSearchMode_)` decides whether the view
is instantiated. It must name the same `SettingsRoutes` key that
`../router.ts` declares and `../route.ts` constructs; a mismatch yields a view
that never renders, with no console error.

**SM2 — The `slot="view"` div and the `id` both matter.** The view manager
addresses children by slot, and the `id` (`browserosPrefs`) is the anchor the
nav and search use for scroll targeting. A new view needs both.

**SM3 — The `.ts` change is an import, not a binding.**
`import '../browseros_prefs_page/browseros_prefs_page.js';` is a side-effect
import; the element is consumed as a custom element tag inside the template,
not as a value. Do not rewrite it as a named import expecting to use the
class.

**SM4 — New views are appended, not inserted.**
The `browserosPrefs` div is added after the last existing `</div>` and before
`</cr-view-manager>`. Chromium's ordering assumptions about view order (first
view = default route) make mid-file insertion risky.

**SM5 — The removed blank line is diff noise, not a change.** The `.ts` patch
deletes one empty line after `import type {SettingsPlugin} …`. Do not restore
it or reformat the surrounding import block; every extra changed line is a
merge cost.

**SM6 — `role="main"` and `class="cr-centered-card-container"` are the layout
contract** for a settings view. A new view that omits them renders full-bleed
and breaks the page's spacing.

## Workflows

**Adding a sub-page view**
1. Create the element (see `../browseros_prefs_page/`).
2. Declare the route in `../router.ts` and construct it in `../route.ts`.
3. Append a `<div slot="view" id="…">` with the `dom-if` predicate to
   `settings_main.html`, after the last existing view div.
4. Add the side-effect import to `settings_main.ts`.
5. Export the element from `../settings.ts` and list the `.ts` in
   `../BUILD.gn`.
6. Extract all four diffs into one `features.yaml` block.

**Debugging a route that renders nothing**
1. Check `routes_.<NAME>` resolves in `../route.ts` (SM1).
2. Check the view div's `dom-if` names the same key.
3. Check the element module is imported here (SM3) and defined with
   `customElements.define` in its own file.
4. Check the element's `.ts` is in `../BUILD.gn`'s `sources` (the bundler will
   not include it otherwise).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `settings/` app, routes and exports.
- [`../router.ts`](../router.ts) / [`../route.ts`](../route.ts) — the
  `SettingsRoutes` key this view keys off.
- [`../browseros_prefs_page/AGENTS.md`](../browseros_prefs_page/AGENTS.md) —
  the element rendered here.
- [`../settings_menu/AGENTS.md`](../settings_menu/AGENTS.md) — the nav entry
  that links to this view.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
