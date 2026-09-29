# `extensions/api/browser_os/` — the `browserOS.*` extension API

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The browser-side implementation of the `browserOS.*` extension API, used by
BrowserOS's own bundled extensions (and the Controller) to drive the browser
from JS. It covers page introspection (`getPageLoadStatus`), direct pref
access (`getPref` / `setPref` / `getAllPrefs`), telemetry (`logMetric`),
version reporting, script execution (`executeJavaScript`), synthetic input
(`clickCoordinates`, `typeAtCoordinates`) and a native file picker
(`choosePath`). `browser_os_api_helpers.{h,cc}` holds the low-level input
plumbing, `browser_os_api_utils.{h,cc}` the tab lookup shared with
`api/side_panel/`, and `browser_os_change_detector.{h,cc}` the
`WebContentsObserver` that decides whether an input action actually changed the
page. All eight files are `new file` patches.

## Contents

```
browser_os/
├── browser_os_api.h / .cc        ← ExtensionFunction subclasses:
│                                    browserOS.getPageLoadStatus, getPref, setPref,
│                                    getAllPrefs, logMetric, getVersionNumber,
│                                    getBrowserosVersionNumber, executeJavaScript,
│                                    clickCoordinates, typeAtCoordinates, choosePath
├── browser_os_api_helpers.h / .cc ← CssToWidgetScale(), PointClick(), NativeType(),
│                                    ClickCoordinatesWithDetection(),
│                                    TypeAtCoordinatesWithDetection()
├── browser_os_api_utils.h / .cc  ← TabInfo struct + GetTabFromOptionalId()
└── browser_os_change_detector.h / .cc ← ExecuteWithDetection() /
                                    ExecuteWithDetectionAsync(); default timeout 300ms
```

## Rules

**BAPI1 — Every function must be registered in two places.** The schema
(`chrome/common/extensions/api/`) drives the generated registry; anything not
hand-wired must be listed in `../../chrome_extensions_browser_api_provider.cc`
(that file explicitly registers
`registry->RegisterFunction<api::BrowserOSExecuteJavaScriptFunction>()`).
A function with a `Run()` but no registration is unreachable.

**BAPI2 — Coordinate maths is not optional.** `CssToWidgetScale()` reproduces
DevTools' `InputHandler::ScaleFactor()`: browser zoom × CSS zoom × page scale.
Device scale factor is deliberately *excluded* — the compositor handles it and
input wants widget DIPs. Any new coordinate-taking function must go through
`PointClick()` / `NativeType()` rather than synthesising its own events, or
clicks land in the wrong place at non-100% zoom.

**BAPI3 — Input actions report success only when the page changed.**
`ClickCoordinatesWithDetection()` and `TypeAtCoordinatesWithDetection()` wrap
the call in `BrowserOSChangeDetector`, which watches accessibility events,
navigation, DOMContentLoaded, focus changes and opened URLs, and times out at
300 ms. Returning `true` unconditionally makes the caller believe a click
landed. `TypeAtCoordinatesWithDetection` additionally falls back to a
JavaScript assignment when native IME typing produces no change — keep that
fallback.

**BAPI4 — `browser_os_api_utils.h` is shared; do not fork it.**
`GetTabFromOptionalId()` resolves an optional `tab_id` into a
`TabInfo{raw_ptr<WebContents>, int tab_id}` and fills an error string on
failure. `api/side_panel/side_panel_service.cc` uses the same helper; a second
tab-resolution implementation will disagree about incognito handling.

**BAPI5 — `browserOS.getPref` / `setPref` bypass the
`settingsPrivate` allowlist by design, and must stay narrow.** These read and
write the `browseros.*` profile prefs directly through `PrefService`, not
through the `settingsPrivate` surface described in
`../settings_private/AGENTS.md`. Widening them to arbitrary Chrome prefs is a
security change, not a feature.

**BAPI6 — `BrowserOSChoosePathFunction` is a `ui::SelectFileDialog::Listener`
and must be deleted-copy.** It holds a `scoped_refptr<ui::SelectFileDialog>`,
implements `FileSelected` / `FileSelectionCanceled`, and is explicitly
non-copyable. A new dialog-backed function must follow the same shape or the
dialog outlives the extension function.

**BAPI7 — All eight files are `new file mode` patches compiled by
`../../BUILD.gn`.** `//chrome/browser/extensions:extensions` lists them as
relative paths; there is no `BUILD.gn` here.

## Workflows

**Adding a `browserOS.*` function**
1. Add the schema entry in `chrome/common/extensions/api/` (JSON).
2. Declare the `ExtensionFunction` subclass in `browser_os_api.h` with
   `DECLARE_EXTENSION_FUNCTION`.
3. Implement `Run()` in `browser_os_api.cc`; resolve the target tab with
   `GetTabFromOptionalId()` (BAPI4).
4. If the generated registry does not pick it up, add an explicit
   `registry->RegisterFunction<…>()` in
   `../../chrome_extensions_browser_api_provider.cc`.
5. Extract and add the path to the same `features.yaml` block.

**Adding a new synthetic-input helper**
1. Put the low-level operation in `browser_os_api_helpers.{h,cc}` and derive
   its scale from `CssToWidgetScale()`.
2. If the operation's success is observable on the page, expose a
   `…WithDetection()` wrapper built on `BrowserOSChangeDetector` rather than
   returning a bare `void`.

**Tuning change detection**
1. Change the `timeout` default parameter in `browser_os_change_detector.h`
   (both static overloads), not the call sites — call sites that need a
   different window pass it explicitly.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/api/` group.
- [`../side_panel/AGENTS.md`](../side_panel/AGENTS.md) — the other consumer of
  `browser_os_api_utils.h`.
- [`../AGENTS.md`](../../AGENTS.md) — the provider that registers
  `browserOS.executeJavaScript`.
- [`../../../browseros/core/AGENTS.md`](../../../browseros/core/AGENTS.md) — the
  `browseros.*` prefs `getPref`/`setPref` read.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
