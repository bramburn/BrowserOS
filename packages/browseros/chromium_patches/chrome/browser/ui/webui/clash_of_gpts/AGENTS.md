# `chrome/browser/ui/webui/clash_of_gpts/` — `chrome://clash-of-gpts/`

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The WebUI host for the Clash of GPTs ("Council") feature. It is a thin
shell: the config/controller pair that serves the page, plus the glue that
routes the page's lifecycle events to the per-window
`ClashOfGptsCoordinator`.

## Contents

```
clash_of_gpts/
├── clash_of_gpts_ui.h   ← NEW: class ClashOfGptsUIConfig : public
│                           content::WebUIConfig, with CreateWebUIController()
└── clash_of_gpts_ui.cc  ← NEW: ClashOfGptsUIConfig ctor (adds the host to the
                            WebUI map), the WebUIController (CSP, data source),
                            and access to
                            browser_window_features()->clash_of_gpts_coordinator()
```

## Rules

**CGW1 — Two new files, registered in three places.** Both files must be
listed in `../BUILD.gn` (`source_set("webui")`), the config registered in
`../chrome_web_ui_configs.cc`, and the host declared in
`chrome/common/webui_url_constants.h` (`kChromeUIClashOfGptsHost` =
`"clash-of-gpts"`, `kChromeUIClashOfGptsURL` = `"chrome://clash-of-gpts/"`).

**CGW2 — The WebUI reaches state through
`browser_window_features()`.** It includes
`chrome/browser/ui/browser_window/public/browser_window_features.h` and calls
`clash_of_gpts_coordinator()`, which **returns null when
`features::kClashOfGpts` is off**. Null-check, or gate on the feature.

**CGW3 — It finds the right window with `BrowserFinder`.** The controller
resolves the active `Browser` before touching the coordinator; the
coordinator is per-`BrowserWindow`.

**CGW4 — The panel implementation lives elsewhere.** The view, window, and
state machine are in
[`../../../views/side_panel/clash_of_gpts/`](../../views/side_panel/clash_of_gpts/AGENTS.md).
This directory is only the `chrome://` surface.

**CGW5 — Do not add a second source of truth for pane state.** Pane
providers, last URLs, and pane count are prefs registered in
`chrome/browser/ui/side_panel/side_panel_prefs.cc`.

## Workflows

**Changing the page's CSP or resources**
Edit the controller in `clash_of_gpts_ui.cc`. The `WebUIConfig` constructor
is where the host string must match
`kChromeUIClashOfGptsHost`.

**Adding a WebUI ↔ native message**
Register the message callback in the controller and expose the method on
`ClashOfGptsCoordinator` in
`chrome/browser/ui/views/side_panel/clash_of_gpts/`.

**Debugging the page 404s**
1. Is the config registered in `../chrome_web_ui_configs.cc`?
2. Do the host constants match the `WebUIConfig` host string exactly?
3. Are both files in `../BUILD.gn`?

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/webui/`.
- [`../../views/side_panel/clash_of_gpts/AGENTS.md`](../../views/side_panel/clash_of_gpts/AGENTS.md)
  — the panel implementation.
- [`../../browser_window/public/AGENTS.md`](../../browser_window/public/AGENTS.md)
  — `clash_of_gpts_coordinator()`.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
