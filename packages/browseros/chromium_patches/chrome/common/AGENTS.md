# `chrome/common/` — shared paths, prefs, WebUI URLs, executable name

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

The cross-process layer: brand constants used by both the browser and
utility processes, `PathService` directory keys, pref-name strings, and
`chrome://` URL constants. Everything here is header- or constants-only
except `chrome_constants.cc` / `chrome_paths*.cc`.

## Contents

```
common/
├── chrome_constants.cc         ← kBrowserProcessExecutableName/Path = "browseros"
├── chrome_paths.{cc,h}         ← new DIR_BROWSEROS_BUNDLED_EXTENSIONS key; mac
│                                 FrameworkBundlePath()/Resources/browseros_extensions,
│                                 other platforms DIR_MODULE/browseros_extensions
├── chrome_paths_linux.cc       ← data dir basename "browser-os"
├── pref_names.h                ← import-dialog prefs + browseros.metrics_* ids;
│                                 note that other BrowserOS prefs live in
│                                 chrome/browser/browseros/core/browseros_prefs.h
├── webui_url_constants.{cc,h} ← kBrowserOSFirstRun, kChromeUIClashOfGptsHost/URL
└── (subdirs: extensions/, importer/)   ← see their own AGENTS.md
```

## Rules

**CM1 — Do not add BrowserOS prefs to `pref_names.h`.** The file itself
carries a note: other BrowserOS prefs moved to
`chrome/browser/browseros/core/browseros_prefs.h`. Only the import-dialog
and metrics IDs still live here. Adding new ones re-splits ownership.

**CM2 — The executable is `browseros`, not `chrome`.** `chrome_constants.cc`
sets the browser process, browser process path, and helper process path to
`FPL("browseros")`. Anything that assumes `chrome` on disk will break on
Windows.

**CM3 — New `PathService` directory keys need an entry in *both*
`chrome_paths.h` (the enum) and `chrome_paths.cc` (the `switch` arm).** A
key without a `Get` arm returns `false` at runtime, not compile time.

**CM4 — `DIR_BROWSEROS_BUNDLED_EXTENSIONS` is a two-platform switch.**
macOS resolves it inside the framework bundle; everything else resolves it
next to `DIR_MODULE`. Adding a third platform means adding a third branch
here.

**CM5 — A `chrome://` host needs a constant *and* a WebUI config.** Adding
to `webui_url_constants.h` is not enough — register a
`content::WebUIConfig` in
[`../browser/ui/webui/chrome_web_ui_configs.cc`](../browser/ui/webui/chrome_web_ui_configs.cc).

**CM6 — `chrome_paths_linux.cc` uses `browser-os` (hyphen) while the
executable is `browseros`.** The hyphen is intentional for the Linux data
directory; do not "fix" the inconsistency.

## Workflows

**Adding a `chrome://` page**
1. Add host + URL constants to `webui_url_constants.h`.
2. Write the `content::WebUIConfig` / `WebUIController` under
   `../browser/ui/webui/`.
3. Register it in `../browser/ui/webui/chrome_web_ui_configs.cc`.
4. Add the sources to `../browser/ui/BUILD.gn` (or the local `BUILD.gn`).

**Adding a new pref**
1. Profile-local: declare in `pref_names.h` (only for the exceptions above)
   or `browseros_prefs.h`; register in the owning `RegisterProfilePrefs`.
2. Syncable: add to the register call in
   `../browser/prefs/browser_prefs.cc`.
3. Add the `k<Name>` string to `browseros_prefs.h` if it is BrowserOS-scoped.

**Adding a `PathService` directory**
1. Add the enumerator to `chrome_paths.h` with a comment.
2. Add the resolution arm in `chrome_paths.cc` (`#if BUILDFLAG(IS_MAC)` split
   if macOS differs).
3. Pass the key to `PathService::Get` from the consumer.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`../browser/browseros/core/AGENTS.md`](../browser/browseros/core/AGENTS.md) —
  where BrowserOS prefs, switches, and constants now live.
- [`../browser/ui/webui/AGENTS.md`](../browser/ui/webui/AGENTS.md) — WebUI
  registration.
- [`extensions/AGENTS.md`](extensions/AGENTS.md) and
  [`importer/AGENTS.md`](importer/AGENTS.md) — subtrees.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
