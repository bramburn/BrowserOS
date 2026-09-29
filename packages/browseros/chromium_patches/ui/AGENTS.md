# `ui/` — headless window support

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md).

## What's here

Ten patch files implementing one feature across Chromium's `ui/` layer: a
**headless window** that renders and paints but never appears in the OS's window
list. The BrowserOS agent uses it to open real, navigable, screenshot-able
windows that the user never sees — no taskbar entry, no Mission Control, no
Alt-Tab.

The feature is a single flag (`headless`) threaded down through four platform
layers.

## Contents

```
ui/
├── platform_window/
│   └── platform_window_init_properties.h   ← + bool headless  (the origin)
├── views/
│   ├── cocoa/native_widget_mac_ns_window_host.mm  ← mac: → mojom is_headless
│   └── widget/
│       ├── widget.h                    ← + bool headless in Widget::InitParams
│       ├── widget_hwnd_utils.cc        ← win: WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE
│       ├── widget_unittest.cc          ← + HeadlessInitParamDefaultsFalse test
│       └── desktop_aura/
│           ├── desktop_window_tree_host_platform.cc  ← InitParams → InitProperties
│           ├── desktop_window_tree_host_win.h        ← + bool is_headless_
│           └── desktop_window_tree_host_win.cc       ← Show(): never ShowWindow()
└── ozone/platform/x11/
    ├── x11_window.h   ← + bool is_headless_
    └── x11_window.cc  ← SKIP_TASKBAR + SKIP_PAGER; Show() unmapped
```

Feature ownership in `features.yaml`: **none of these are listed.** The whole
`ui/` subtree is unregistered; see UI4.

## Rules

**UI1 — The flag is a chain across all ten files here plus the two
`components/remote_cocoa/` files, and it must stay intact.**

```
Widget::InitParams::headless                (ui/views/widget/widget.h)
  └─ mac  → window_params->is_headless      (ui/views/cocoa/…ns_window_host.mm)
            └─ mojom                       (components/remote_cocoa/common/…mojom)
              └─ shim                      (components/remote_cocoa/app_shim/…bridge.mm)
  └─ aura → PlatformWindowInitProperties::headless  (ui/platform_window/…h)
            └─ generic                     (…desktop_window_tree_host_platform.cc)
              ├─ win → WS_EX_TOOLWINDOW + never ShowWindow  (…win.cc/.h, widget_hwnd_utils.cc)
              └─ x11 → SKIP_TASKBAR/PAGER + XUnmapWindow    (ui/ozone/platform/x11/…)
```

Breaking any link silently disables the feature on that platform rather than
failing to compile.

**UI2 — `headless` is decided at construction and never transitions.** The
comment in `widget.h` says so explicitly. Don't add code that toggles it after
creation — `is_headless_` is captured in `Init()` / `Initialize()` and the
window's OS-level attributes are set from it once.

**UI3 — "Headless" here means invisible, not `headless_display`.** The compositor
still runs so pages render and screenshots work; only the OS window surface is
suppressed. Do not route this through Chromium's existing headless-display
machinery, and do not disable the compositor to achieve it.

**UI4 — None of these files are in `features.yaml`.** They apply by path but
never appear in an annotated commit. When you touch one, add it to a feature
block — the whole `ui/` subtree is a documentation gap in the manifest.

**UI5 — Windows and X11 are compiled on this host; macOS and Wayland are not.**
`ui/ozone/platform/x11/` and `ui/views/widget/desktop_aura/` build here.
`ui/views/cocoa/` builds only on mac. Wayland has **no** headless support yet —
the `platform_window_init_properties.h` comment says so, and adding a partial
Wayland implementation is worse than none.

**UI6 — `widget_unittest.cc` is a real upstream test file.** The
`HeadlessInitParamDefaultsFalse` case is the only test for the flag. Keep it
passing; it is a cheap guard against accidentally defaulting `headless` to true,
which would hide every window in every test.

**UI7 — `WS_EX_TOOLWINDOW` is belt-and-suspenders on Windows.** The HWND is
never shown at all; the styles exist so an accidental surface has no
taskbar/Alt-Tab presence. Do not remove the style flags thinking they are
redundant.

**UI8 — X11 `Show()` fakes the delegate notification.**
`window_mapped_in_client_ = true` plus an explicit
`OnWindowStateChanged(kUnknown, kNormal)` keeps the content compositor running
while the XWindow stays unmapped. Replacing this with a real `Map()` breaks the
feature; removing the notification stalls rendering.

## Workflows

**Opening a headless window (what this feature is for)**
1. Construct the `Widget` with `params.headless = true`.
2. For a `BrowserWindow`, that comes from
   `chrome/browser/ui/views/frame/browser_native_widget_aura.cc` /
   `browser_native_widget_ash.cc` (under `chrome/`, not here).
3. On X11, verify the window has no taskbar entry and is not in the pager.
4. Confirm a CDP screenshot of the tab returns real pixels.

**Adding headless support to Wayland**
1. Read the "not yet supported" note in
   `platform_window/platform_window_init_properties.h`.
2. Implement in `ui/ozone/platform/wayland/` — that file is **not** in this
   overlay, so it is a new patch requiring a new `features.yaml` entry.
3. Mirror the X11 approach: never `xdg_surface`/`commit` the map, or set
   `XDG_TOPLEVEL_STATE` equivalents where the protocol allows.
4. Verify compositor output still reaches the surface; the X11 trick of faking
   the delegate callback is the pattern to copy.

**Adding a nuance to headless behaviour (e.g. "also skip the window list")**
1. Extend `PlatformWindowInitProperties` with a separate field — do not overload
   `headless`.
2. Thread it through `desktop_window_tree_host_platform.cc`.
3. Implement per platform: X11 atoms, Windows styles, macOS swizzler.
4. Update the chain diagram above.

**Debugging "headless windows are visible"**
| Platform | Check |
|---|---|
| X11 | `xwininfo -root -tree` — is the XWindow mapped? Are the SKIP_TASKBAR atoms set? |
| Windows | Is `ShowWindow(SW_SHOW*)` reached in `desktop_window_tree_host_win.cc`? |
| macOS | Did `params->is_headless` reach the app shim? Check both remote_cocoa files. |
| All | Is `params.headless` actually set on the `Widget::InitParams` (chrome-side)? |

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../components/remote_cocoa/AGENTS.md`](../components/remote_cocoa/AGENTS.md) — the macOS mojom + shim half of the chain.
- [`../components/AGENTS.md`](../components/AGENTS.md) — the other platform shims.
