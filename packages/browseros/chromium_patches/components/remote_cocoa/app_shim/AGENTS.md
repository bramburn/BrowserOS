# `components/remote_cocoa/app_shim/` — honouring `is_headless` in the shim

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-line-condition diff in Chromium's macOS app shim. When a
`NativeWidgetNSWindowInitParams` arrives with `is_headless` set, the shim calls
`[window_ setIsHeadless:YES]` so the NSWindow is excluded from Dock, Mission
Control, and Cmd-Tab — the macOS half of the agent's hidden-window feature.

Feature block: **none** (unregistered in `features.yaml`).

## Contents

```
app_shim/
└── native_widget_ns_window_bridge.mm   ← @@ -555,7 +555,7 @@
```

```objc
- if (display::Screen::Get()->IsHeadless()) {
+ if (params->is_headless || display::Screen::Get()->IsHeadless()) {
    [window_ setIsHeadless:YES];
  }
```

## Rules

**RCA1 — OR with the existing check; never replace it.** `--headless` mode is
unrelated to the per-window agent flag. Dropping
`display::Screen::Get()->IsHeadless()` breaks headless Chromium outright.

**RCA2 — The mojom field must exist first.** This file will not compile unless
`../common/native_widget_ns_window.mojom` declares `is_headless`.

**RCA3 — Objective-C++, macOS only.** The app shim process is built on macOS
and nowhere else. There is no Windows compile coverage; a syntax error here
ships undetected on this host.

**RCA4 — Do not "simplify" the condition into a ternary or hoist
`display::Screen::Get()`.** `Screen::Get()` is not cheap and the shim runs on
the main thread; the short-circuit order in the original is deliberate.

**RCA5 — Cross-repo counterpart.** The Windows equivalent is
`ui/views/widget/desktop_aura/desktop_window_tree_host_win.cc` (never call
`ShowWindow`) plus `WS_EX_TOOLWINDOW` in `widget_hwnd_utils.cc`; the X11
equivalent is `ui/ozone/platform/x11/x11_window.cc`. All three implement the
same flag from different angles — change one, review the others.

## Workflows

**Adding a nuance to headless-window detection**
1. Extend the mojom in `../common/native_widget_ns_window.mojom` first.
2. Edit the `setIsHeadless:` branch here.
3. Build on a macOS host and verify with a windowed agent launch.
4. Add both paths to a `features.yaml` block.

**Verifying the window is genuinely hidden on mac**
- Launch the agent against a profile and check
  `osascript -e 'tell application "System Events" to get name of every process whose visible is true'`
  or inspect Dock items while an agent window is open.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/remote_cocoa/` rules.
- [`../common/AGENTS.md`](../common/AGENTS.md) — the mojom field.
- [`../../../ui/AGENTS.md`](../../../ui/AGENTS.md) — the Windows/X11 counterparts.
