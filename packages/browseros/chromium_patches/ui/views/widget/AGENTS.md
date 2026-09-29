# `ui/views/widget/` — `Widget::InitParams::headless` and the Aura path

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The declaration and the Aura/Windows implementation of the headless-window
flag. `Widget::InitParams` gains a `headless` boolean; the Aura path copies it
into `PlatformWindowInitProperties`, and the Win host uses it to skip
`ShowWindow` entirely while the HWND carries `WS_EX_TOOLWINDOW`.

## Contents

```
widget/
├── widget.h                                 ← + bool headless = false  (in InitParams)
├── widget_hwnd_utils.cc                     ← + WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE
├── widget_unittest.cc                       ← + TEST HeadlessInitParamDefaultsFalse
└── desktop_aura/
    ├── desktop_window_tree_host_platform.cc ← + properties.headless = params.headless
    ├── desktop_window_tree_host_win.h       ← + bool is_headless_ = false
    └── desktop_window_tree_host_win.cc      ← Init(): capture; Show(): early return
```

Feature block: **none** (unregistered in `features.yaml`).

## Rules

**WID1 — `headless` belongs in `Widget::InitParams` and is set once.** The
comment states it is "Decided at construction; never transitions". Do not add
a setter or a runtime toggle.

**WID2 — `widget_unittest.cc` is an upstream test file.** The added
`HeadlessInitParamDefaultsFalse` case is the only automated guard on the flag's
default. It is cheap and it catches the worst possible regression (every window
in every test becoming invisible). Keep it.

**WID3 — `desktop_window_tree_host_platform.cc` is the only Aura→platform
bridge.** It is the single place `Widget::InitParams` becomes
`PlatformWindowInitProperties`. Adding a second copy would let the two
structs disagree.

**WID4 — The Win `Show()` early return must come after
`OnAcceleratedWidgetMadeVisible(true)` and before any `ShowWindow(SW_SHOW*)`
call.** That ordering is the whole mechanism: the Aura side becomes visible so
the compositor runs, the HWND does not.

**WID5 — The `WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE` styles are not redundant
with the early return.** They are the fallback for a window the OS compositor
surfaces anyway. Keep both parts.

**WID6 — `is_headless_` is captured in `Init()`, first statement.** It must be
read before the frame/style setup, because the styles derived in
`widget_hwnd_utils.cc` depend on it.

**WID7 — Windows files do compile on this host**, so a broken edit fails the
build — but only if a Windows build actually runs. `desktop_aura` is
`#if defined(OS_WIN)`-guarded upstream; a Linux-only build will skip it
entirely.

## Workflows

**Opening a headless window on Windows**
1. Set `params.headless = true` when constructing the `Widget` (usually from
   `chrome/browser/ui/views/frame/browser_native_widget_aura.cc`).
2. `IsWindowVisible(hwnd)` must be false after `Show()`.
3. The taskbar and Alt-Tab must show no entry.
4. A CDP screenshot of the tab must return real pixels.

**Adding a nuance (e.g. "also hide from the window animation host")**
1. Add a separate field to `Widget::InitParams` — do not overload `headless`.
2. Thread it through `desktop_window_tree_host_platform.cc`.
3. Implement in `desktop_window_tree_host_win.cc` and, for Linux, in
   `ui/ozone/platform/x11/x11_window.cc`.
4. Add a default-value test next to `HeadlessInitParamDefaultsFalse`.

**Running the flag's test**
- Build the `//ui/views:widget_unittest` target; run
  `HeadlessInitParamDefaultsFalse`. The rest of `WidgetTest` covers unrelated
  init-param behaviour.

**Rebasing `desktop_window_tree_host_win.cc`**
- Aura Win files change frequently. Reset to `BASE_COMMIT`, re-apply the
  `Init()` capture and the `Show()` early return, then extract.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/views/` rules and the two dispatch paths.
- [`../cocoa/AGENTS.md`](../cocoa/AGENTS.md) — the macOS path.
- [`../desktop_aura/AGENTS.md`](desktop_aura/AGENTS.md) — the Aura subfolder.
- [`../../platform_window/AGENTS.md`](../../platform_window/AGENTS.md) — where the flag is defined.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
