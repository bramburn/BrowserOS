# `chrome/browser/ui/webui/` — WebUI host layer

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

The host side of BrowserOS's custom `chrome://` pages: the WebUI config
registry, the first-run welcome page, and the `chrome://clash-of-gpts/`
page. Everything here is what `content::WebUIConfigMap` needs in order for
a virtual URL to resolve.

## Contents

```
webui/
├── chrome_web_ui_configs.cc     ← registers BrowserOSWelcomeUIConfig and
│                                   ClashOfGptsUIConfig in the WebUI map
├── browseros_welcome.h         ← NEW: full BrowserOSWelcomeUIConfig +
│                                   BrowserOSUIDataSource + controller for
│                                   chrome://browseros-welcome
├── BUILD.gn                    ← adds clash_of_gpts/clash_of_gpts_ui.* to
│                                   source_set("webui")
├── clash_of_gpts/              ← chrome://clash-of-gpts/ (see its AGENTS.md)
├── help/                       ← macOS Sparkle VersionUpdater (see its AGENTS.md)
├── new_tab_footer/             ← pref default change (see its AGENTS.md)
├── settings/                   ← BrowserOS metrics handler + import handler
└── side_panel/                 ← customize-chrome mojom (see its AGENTS.md)
```

## Rules

**WU1 — A `chrome://` host needs four things.** A host constant in
`chrome/common/webui_url_constants.h`, a `content::WebUIConfig` subclass, a
`AddWebUIConfig(...)` line in `chrome_web_ui_configs.cc`, and the sources in
`BUILD.gn`. Missing any one is a runtime 404 or a link error, never a
compile error in the page itself.

**WU2 — `browseros_welcome.h` is a new file.** It carries the config class,
the data source, and the controller for `chrome://browseros-welcome` in a
single header. When adding more first-run content, extend this file rather
than creating a parallel one, unless a separate host is genuinely needed.

**WU3 — `chrome_web_ui_configs.cc` is gated by build flags.** The
registration lines sit inside the desktop-only block; verify the guard when
adding a config, or an Android build fails.

**WU4 — `BUILD.gn` here only covers the `webui` source_set.** The settings
handlers are added to `chrome/browser/ui/BUILD.gn` instead
(`webui/settings/browseros_metrics_handler.*`,
`webui/help/sparkle_version_updater_mac.*`). Check which target owns a path
before editing a GN file.

**WU5 — WebUI pages are untrusted-ish surfaces.** Each `WebUIConfig` sets
its own CSP. A new page must define one rather than inheriting a permissive
default.

## Workflows

**Adding a new `chrome://` page**
1. Write the `WebUIConfig` + `WebUIController` (+ data source if it serves
   resources) in a subdirectory here.
2. Add the host/URL constants in
   `chrome/common/webui_url_constants.h`.
3. Register the config in `chrome_web_ui_configs.cc`.
4. Add the sources to this `BUILD.gn` (or the owning `chrome/browser/ui/BUILD.gn`).
5. List the files under a feature block in
   `packages/browseros/build/features.yaml` (`first-run` for the welcome
   page, `llm-hub` for Clash of GPTs).

**Changing the first-run page**
`chrome://browseros-welcome` is opened from
`chrome/browser/chrome_browser_main.cc`
(`AddFirstRunTabs({GURL("chrome://browseros-welcome")})`). Both must change
together.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`settings/AGENTS.md`](settings/AGENTS.md) — the largest child; metrics
  and import handlers.
- [`clash_of_gpts/AGENTS.md`](clash_of_gpts/AGENTS.md) — `chrome://clash-of-gpts/`.
- [`help/AGENTS.md`](help/AGENTS.md) — Sparkle `VersionUpdater`.
- [`side_panel/AGENTS.md`](side_panel/AGENTS.md) — customize-toolbar mojom.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
