# `resources/settings/` — the `chrome://settings` WebUI app

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Chromium's settings WebUI, and the five entry points the fork has to touch
whenever a page is added. `router.ts` declares the `SettingsRoutes` interface
(one field per route), `route.ts` constructs the `Route` objects (one line per
route, title from `loadTimeData.getString(...)`), `settings.ts` is the public
barrel that re-exports every page element to the WebUI bundle,
`settings_main/settings_main.html` provides one `<div slot="view">` per
sub-page, and `settings_main/settings_main.ts` imports the page module. The
patch adds exactly one route — `BROWSEROS_PREFS` at `/browseros-settings` —
and the `SettingsBrowserOSPrefsPageElement` export.

## Contents

```
settings/
├── router.ts     ← interface SettingsRoutes { … BROWSEROS_PREFS: Route; … }
├── route.ts      ← r.BROWSEROS_PREFS = new Route('/browseros-settings',
│                   'BrowserOS Settings');
│                   (r.BASIC.createSection for the rest)
├── settings.ts   ← export {SettingsBrowserOSPrefsPageElement} from
│                   './browseros_prefs_page/browseros_prefs_page.js';
├── BUILD.gn      ← build_webui("build") sources +=
│                   "browseros_prefs_page/browseros_prefs_page.ts"
├── about_page/          ← the Chrome about: page, de-Googled
├── browseros_prefs_page/← the new BrowserOS Settings page (ts + html)
├── people_page/         ← import dialog: cookies + extensions checkboxes
├── reset_page/          ← reset-profile feedback checkbox default
├── settings_main/       ← the view container (html + ts)
└── settings_menu/       ← the left nav (BrowserOS entry is commented out)
```

## Rules

**SET1 — A new page is five coordinated edits, not one.** `router.ts` (interface
field), `route.ts` (`new Route(...)`), `settings.ts` (export),
`settings_main/settings_main.html` (`<div slot="view">` with a
`dom-if` on `renderPlugin_(routes_.X, lastRoute_, inSearchMode_)`), and
`settings_main/settings_main.ts` (side-effect `import`). A route with no view
slot renders a blank page, not an error.

**SET2 — The `build_webui("build")` `sources` list is explicit.**
`BUILD.gn` enumerates every `.ts` entry point, including
`browseros_prefs_page/browseros_prefs_page.ts`. A `.ts` file that is not
listed is never bundled and its element is never defined.

**SET3 — `BROWSEROS_PREFS` is a top-level route, not a section of another
page.** It is created from `r.BASIC` directly, not via
`r.X.createSection(...)`, so it gets its own URL
(`/browseros-settings`) rather than a nested path. Follow that pattern for
any future top-level BrowserOS page.

**SET4 — The nav entry in `settings_menu/settings_menu.html` is intentionally
commented out.** The page is reachable at `/browseros-settings` but has no
left-nav link. Un-commenting the `<a id="browseros-prefs-menu">` block is a
one-line change, but it must stay in sync with the route existing.

**SET5 — Do not add a `nxtscape_page/` here.** The fork's
`features.yaml` lists `chrome/browser/resources/settings/nxtscape_page/*`, but
no such directory exists in `chromium_patches/` and no file in this tree
references it. Treat the manifest entry as stale unless a corresponding
Chromium-side change lands.

**SET6 — Page elements are exported through `settings.ts`, not imported
directly by consumers.** `chrome_web_ui.cc`-side templates and other pages
consume the barrel. Adding a second import path bypasses the tree-shaking the
barrel exists to provide.

## Workflows

**Adding a `chrome://settings` subpage**
1. Create `<name>_page/<name>_page.ts` and `.html` (see
   `browseros_prefs_page/` for the pattern).
2. Add the `SettingsRoutes` field in `router.ts`.
3. Add `r.<NAME> = new Route('/<path>', 'Title');` in `route.ts`.
4. Add the `view` slot in `settings_main/settings_main.html` and the import in
   `settings_main/settings_main.ts`.
5. Add the export in `settings.ts` and the `.ts` in `BUILD.gn`.
6. Extract all six diffs into one `features.yaml` block.

**Adding a nav entry for an existing route**
1. Un-comment / add the `<a role="menuitem" id="…" href="/<path>">` block in
   `settings_menu/settings_menu.html`.
2. Keep the `cr-nav-menu-item` class and the `<cr-ripple>` child; the menu
   styles depend on both.
3. The `id` must match the sub-page's own anchor id for scroll targeting.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `resources/` parent.
- [`browseros_prefs_page/AGENTS.md`](browseros_prefs_page/AGENTS.md) — the
  page this route renders.
- [`settings_main/AGENTS.md`](settings_main/AGENTS.md) — the view container.
- [`settings_menu/AGENTS.md`](settings_menu/AGENTS.md) — the left nav.
- [`../../prefs/AGENTS.md`](../../prefs/AGENTS.md) — pref registration.
- [`../../extensions/api/settings_private/AGENTS.md`](../../extensions/api/settings_private/AGENTS.md)
  — the allowlist the WebUI reads through.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.
