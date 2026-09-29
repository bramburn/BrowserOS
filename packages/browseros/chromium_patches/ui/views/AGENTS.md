# `ui/views/` — views-layer headless window plumbing

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../ui/AGENTS.md`](../../ui/AGENTS.md).

## What's here

The views layer (`ui/views/`) of the headless-window feature. This is where the
`headless` flag is born (`Widget::InitParams`) and where it is dispatched to a
platform: macOS reads it directly, the Aura path routes it through
`PlatformWindowInitProperties`.

## Contents

```
views/
├── cocoa/
│   └── native_widget_mac_ns_window_host.mm   ← window_params->is_headless = params.headless
└── widget/
    ├── widget.h                       ← + bool headless in Widget::InitParams
    ├── widget_hwnd_utils.cc           ← WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE
    ├── widget_unittest.cc             ← + HeadlessInitParamDefaultsFalse
    └── desktop_aura/
        ├── desktop_window_tree_host_platform.cc  ← properties.headless = params.headless
        ├── desktop_window_tree_host_win.h        ← + bool is_headless_
        └── desktop_window_tree_host_win.cc       ← Show(): never ShowWindow()
```

Feature block: **none** (unregistered in `features.yaml`).

## Rules

**VWS1 — `Widget::InitParams::headless` is the public entry point.** Everything
else in this subtree is downstream. It defaults to `false` and is fixed at
construction.

**VWS2 — Two dispatch paths, two destinations.**
- Aura (Windows/Linux): `desktop_window_tree_host_platform.cc` copies it into
  `PlatformWindowInitProperties`, where X11 or the Win host reads it.
- macOS: `native_widget_mac_ns_window_host.mm` sets the mojom field
  `is_headless` directly, because the mac path does not go through
  `PlatformWindowInitProperties`.

Adding a platform means picking the right path, not wiring both.

**VWS3 — `widget_unittest.cc` is an upstream test file, not a new test dir.** The
added case is one function in the existing `WidgetTest` fixture. Keep it in
place; it guards the default.

**VWS4 — Windows suppression is two-part and both parts matter.**
`widget_hwnd_utils.cc` sets `WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE`; the
`desktop_aura` host never calls `ShowWindow`. Remove the styles and an
accidental surface gains taskbar presence; remove the early return and the
window is genuinely shown.

**VWS5 — `native_widget_mac_ns_window_host.mm` builds only on macOS.** No
Windows compile coverage; a mistake there ships silently from this host.

**VWS6 — Keep the `// BrowserOS:` / explanatory comments in the hunks.** They
are the only documentation of *why* a window can be created and never mapped,
and they are the first thing lost in a rebase.

## Workflows

**Creating a headless widget from BrowserOS code**
1. Set `params.headless = true` on the `Widget::InitParams` (or the
   `BrowserWindow` init params that produce it, under
   `chrome/browser/ui/views/frame/browser_native_widget_aura.cc` /
   `browser_native_widget_ash.cc`).
2. Build; the platform backend picks it up automatically.
3. Verify per platform (X11 / Windows / macOS checks in
   [`../ozone/platform/x11/AGENTS.md`](../ozone/platform/x11/AGENTS.md) and
   [`../AGENTS.md`](../AGENTS.md)).

**Adding a new platform**
1. Read `params.headless` in that platform's `NativeWidgetHost`.
2. Suppress the OS window surface (never map/show, plus platform-specific hints).
3. Ensure the compositor still runs — most platforms need a faked visibility
   notification, as X11 does.
4. Add the platform to the comment in
   `../platform_window/platform_window_init_properties.h`.

**Running the only test for the flag**
- `HeadlessInitParamDefaultsFalse` in
  `chromium_patches/ui/views/widget/widget_unittest.cc` — build the
  `//ui/views:widget_unittest` target and run it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/` subtree rules and the full flag chain.
- [`widget/AGENTS.md`](widget/AGENTS.md) — the Aura/Windows path.
- [`cocoa/AGENTS.md`](cocoa/AGENTS.md) — the macOS path.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
