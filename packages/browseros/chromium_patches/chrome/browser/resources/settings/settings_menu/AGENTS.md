# `resources/settings/settings_menu/` — the settings left nav

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The left-hand navigation list of `chrome://settings`. The fork's only change
is a six-line HTML comment: a complete `<a role="menuitem"
id="browseros-prefs-menu" href="/browseros-settings" class="cr-nav-menu-item">`
block linking to the BrowserOS Settings page, wrapped in `<!-- … -->`. The
route and the page exist; the nav entry is present but inert.

## Contents

```
settings_menu/
└── settings_menu.html   ← + commented-out <a id="browseros-prefs-menu"
                              href="/browseros-settings"> block, inserted after
                              the People link and before the Autofill link
```

## Rules

**SMN1 — The entry is commented out on purpose.** The page is reachable at
`chrome://settings/browseros-settings`; it simply is not advertised in the nav.
Un-commenting is a one-change edit, but it is a product decision, not a cleanup.

**SMN2 — Un-commenting must keep all five pieces.** The `role="menuitem"`
attribute, the `id="browseros-prefs-menu"`, the `href="/browseros-settings"`
(which must match the `Route` in `../route.ts`), the
`class="cr-nav-menu-item"` (which the menu's CSS selects on), and the
`<cr-ripple></cr-ripple>` child (which supplies the hover/active indicator).
Removing the ripple makes the item look broken.

**SMN3 — Position is between People and Autofill.** The block is inserted
after the `peoplePageTitle` link and before the `autofill` link. The nav order
is a product surface; inserting elsewhere changes the information architecture
of the settings app.

**SMN4 — There is no `.ts` change here.** The menu is template-only; the
`onClick` handlers for the *existing* items (e.g. `onAutofillClick_`) live in
the upstream `settings_menu.ts`, which this fork does not patch. A new item
that needs logic requires adding a handler there.

**SMN5 — Icons come from `cr:`, not `settings:`.** The commented block uses
`<cr-icon icon="settings:build">`; if it is enabled, verify that icon id
exists in the `cr_icon` set available on this page, or the item renders a
blank square.

## Workflows

**Adding a nav entry for an existing route**
1. Insert an `<a role="menuitem" id="…" href="/<path>"
   class="cr-nav-menu-item">` block in the desired position.
2. Include a `<cr-icon icon="…">` and a `<cr-ripple></cr-ripple>`.
3. Confirm the `href` matches the `Route` in `../route.ts` exactly, including
   the leading slash.
4. Re-extract; keep the diff to the inserted block.

**Deciding whether to enable the BrowserOS entry**
1. Check `../browseros_prefs_page/` still works at the direct URL.
2. Un-comment the block (SM2) and verify the item appears between People and
   Autofill.
3. Verify the icon renders (SM5) and the ripple animates.
4. Re-extract `settings_menu.html` into the same `features.yaml` block as the
   route and page.

**Debugging a nav item that does not respond**
1. Check the `href` against `../route.ts` (a missing leading slash silently
   fails).
2. Check `class="cr-nav-menu-item"` is present — without it the item is
   unstyled and may have no hit area.
3. Check `<cr-ripple>` is a direct child.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `settings/` app, routes and exports.
- [`../route.ts`](../route.ts) — the `BROWSEROS_PREFS` route this href targets.
- [`../browseros_prefs_page/AGENTS.md`](../browseros_prefs_page/AGENTS.md) —
  the page this entry would open.
- [`../settings_main/AGENTS.md`](../settings_main/AGENTS.md) — the view
  container the route resolves to.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
