# `extensions/` — BrowserOS hooks into Chromium's extension system

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

Seven small diffs that make the stock Chromium extension system behave for
BrowserOS: install the bundled CRX files as external components, protect them
from being disabled or uninstalled, preserve their `chrome.storage.local` across
transient uninstalls, route them through the alpha update manifest on the alpha
channel, allow their `chrome://browseros/*` overrides, and hide their entry in
the extensions context menu. `BUILD.gn` is what makes the fork's own
`browseros/extensions/` and `api/browser_os/` files part of
`//chrome/browser/extensions:extensions`.

## Contents

```
extensions/
├── BUILD.gn                            ← adds the six
│                                         //chrome/browser/browseros/extensions/*.cc
│                                         paths, the eight api/browser_os/* files,
│                                         and deps on //chrome/browser/browseros/core
│                                         and //chrome/browser/browseros/metrics
├── external_provider_impl.cc           ← constructs the BrowserOSExtensionLoader
│                                         provider (kExternalComponent for both CRX
│                                         and download location, auto-acknowledge,
│                                         allow-updates, install-immediately)
├── chrome_extension_registrar_delegate.cc ← skips DataDeleter for BrowserOS
│                                         extensions; CanDisableExtension() returns false
├── extension_management.cc             ← GetEffectiveUpdateURL() → alpha appcast
│                                         when kBrowserOsAlphaFeatures is on;
│                                         GetForcePinnedList() adds pinned BrowserOS IDs
├── extension_context_menu_model.cc     ← drops the "Remove" item for BrowserOS extensions
├── extension_url_overrides_registrar.cc ← only registers chrome:// URL overrides for
│                                         BrowserOS extensions (everything else returns early)
└── chrome_extensions_browser_api_provider.cc ← registers api::BrowserOSExecuteJavaScriptFunction()
```

Subdirectories: `api/browser_os/`, `api/debugger/`, `api/settings_private/`,
`api/side_panel/`, `updater/` — each has its own `AGENTS.md`.

## Rules

**EX1 — Every "is this a BrowserOS extension?" decision goes through
`../browseros/core/browseros_constants.h`.** `IsBrowserOSExtension()`,
`IsBrowserOSPinnedExtension()` and `IsBrowserOSLabelledExtension()` are the only
sources. Four files in this directory include that header; a fifth that
compares IDs literally is a bug.

**EX2 — Protection is spread across three call sites and must stay in sync.**
`CanDisableExtension()` (registrar delegate) blocks disabling,
`ExtensionContextMenuModel::InitMenuWithFeature` hides "Remove", and
`GetForcePinnedList()` keeps them pinned. Removing one without the others
leaves a BrowserOS extension that is still uninstallable through the UI.

**EX3 — The `DataDeleter` bypass in `PostUninstallExtension` is deliberate and
load-bearing.** The comment explains it: BrowserOS extensions are transiently
uninstalled during update cycles (when both the bundled CRX and the remote
config fail on startup), and user configuration in `chrome.storage.local` must
survive. Do not "fix" this by removing the branch; the extension is
uninstalled precisely so it can be reinstalled cleanly.

**EX4 — The alpha update URL override belongs in
`GetEffectiveUpdateURL()`, not in `manifest.json`.** The comment states the
reason: a mid-session channel flip must take effect on the next update check
without uninstalling the extension. Putting it in the manifest makes it
startup-only.

**EX5 — `extension_url_overrides_registrar.cc` narrows, it does not add.**
The patch makes non-BrowserOS extensions return early from
`OnExtensionLoaded`, so their `chrome://` overrides are not registered at all.
Adding a new override for a non-BrowserOS extension requires removing that
early return, not working around it.

**EX6 — `chrome_extensions_browser_api_provider.cc` is the registration point
for the `browserOS.*` API.** Only
`api::BrowserOSExecuteJavaScriptFunction()` is registered here by hand; the
other `api/browser_os/browser_os_api.h` functions are registered through the
generated `api_registration`/JSON path. A new function that does not appear in
either place is invisible to extensions.

**EX7 — `BUILD.gn` uses absolute GN paths for the BrowserOS files.** The
`sources` list entries are
`"//chrome/browser/browseros/extensions/browseros_extension_*.cc"`, not relative
names, because those files live outside this directory. Keep that style when
adding one.

## Workflows

**Adding a protection rule for BrowserOS extensions**
1. Add the predicate to `../browseros/core/browseros_constants.h` if it is not
   an existing one (`IsBrowserOSPinnedExtension`, `IsBrowserOSLabelledExtension`,
   `UsesContextualSidePanelToggle`).
2. Apply it at every relevant call site (EX2) — disabling, context menu,
   pinning, uninstall.
3. Extract each touched diff and list them under the same `features.yaml`
   block.

**Changing how bundled CRX files are installed**
1. Edit `external_provider_impl.cc` only for provider-level flags (location,
   `set_auto_acknowledge`, `set_install_immediately`).
2. Behaviour inside the install (bundled-then-remote ordering, delays) belongs
   in `../browseros/extensions/browseros_extension_installer.cc`.

**Adding a `browserOS.*` extension API function**
1. Declare the `ExtensionFunction` subclass in `api/browser_os/browser_os_api.h`
   with `DECLARE_EXTENSION_FUNCTION("<name>", <ENUM>)`.
2. Implement `Run()` in `api/browser_os/browser_os_api.cc`.
3. Add the `"name"` string to the API's JSON schema
   (`chrome/common/extensions/api/`) so the generated registry picks it up, or
   register it by hand in `chrome_extensions_browser_api_provider.cc`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../browseros/core/AGENTS.md`](../browseros/core/AGENTS.md) — extension ID
  table and predicates.
- [`../browseros/extensions/AGENTS.md`](../browseros/extensions/AGENTS.md) —
  loader / installer / maintainer.
- [`api/browser_os/AGENTS.md`](api/browser_os/AGENTS.md) — the `browserOS.*` API.
- [`api/side_panel/AGENTS.md`](api/side_panel/AGENTS.md) — the
  `sidePanel.browseros*` functions.
- [`updater/AGENTS.md`](updater/AGENTS.md) — `InstallPendingNow()`.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
