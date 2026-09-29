# `resources/` — BrowserOS WebUI patches

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

A grouping directory with no files of its own. The only patched content is
`settings/`, the `chrome://settings` WebUI app: its route table, its page
export barrel, and six sub-pages that BrowserOS customises. This file exists
so the WebUI surface has a documented root.

## Contents

```
resources/
└── settings/               ← see settings/AGENTS.md
    ├── route.ts, router.ts, settings.ts, BUILD.gn
    ├── about_page/
    ├── browseros_prefs_page/
    ├── people_page/
    ├── reset_page/
    ├── settings_main/
    └── settings_menu/
```

## Rules

**RES0 — No sources directly in `resources/`.** A new WebUI surface goes in
its own subdirectory and is listed in that subdirectory's `BUILD.gn`
`build_webui("build")` sources.

**RES1 — WebUI changes are Lit/TypeScript + Polymer, not C++.** Anything that
needs browser-process support belongs in `../browseros/` or
`../extensions/api/browser_os/`, not here. The only exception is binding a pref
that is already registered and allowlisted.

**RES2 — A new page needs five edits.** (1) a `Route` in `settings/route.ts`,
(2) a field in the `SettingsRoutes` interface in `settings/router.ts`,
(3) a `view` slot in `settings/settings_main/settings_main.html`,
(4) an `import` in `settings/settings_main/settings_main.ts`, (5) an `export`
in `settings/settings.ts` and a `sources` entry in `settings/BUILD.gn`.
Missing any one of them produces a route that resolves to nothing.

**RES3 — Pages are under `chrome/browser/ui/AGENTS.md`'s sibling, not
`browser/ui/`.** `browser/ui/` (the C++ browser UI) is a different subtree with
its own `AGENTS.md`; do not conflate the two when adding a WebUI handler.

## Workflows

**Adding a `chrome://settings` subpage**
1. Create `resources/settings/<name>_page/` with a `.ts` and an `.html`.
2. Add the route and the `SettingsRoutes` field (RES2).
3. Add the `view` slot, the import, the export, and the `BUILD.gn` entry.
4. Extract every touched file and list them under one `features.yaml` block.

**Binding a BrowserOS pref in the WebUI**
1. Confirm the pref is registered — see `../prefs/AGENTS.md`.
2. Confirm it is allowlisted for `settingsPrivate` — see
   `../extensions/api/settings_private/AGENTS.md`.
3. Bind it with `prefs="{{prefs.browseros.x}}"`; do not add a second
   registration path from the WebUI.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`settings/AGENTS.md`](settings/AGENTS.md) — route/export wiring.
- [`../ui/AGENTS.md`](../ui/AGENTS.md) — the C++ browser UI (different subtree).
- [`../prefs/AGENTS.md`](../prefs/AGENTS.md) — pref registration.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
