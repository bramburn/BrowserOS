# `ui/platform_window/` — the `headless` init property

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The origin of the headless-window feature. One field is appended to
`ui::PlatformWindowInitProperties`, the platform-agnostic struct that carries
per-window creation intent from the views layer down to the platform window
implementation.

## Contents

```
platform_window/
└── platform_window_init_properties.h   ← + bool headless = false  (last member)
```

The comment above the field is the specification:

> When true, the native window is created but never mapped/shown to the OS
> compositor. Used by BrowserOS agent hidden windows: compositor still runs (so
> pages render, screenshots work), but WM/taskbar/overview never see it.
> On X11: SKIP_TASKBAR + SKIP_PAGER hints, XUnmapWindow. On Wayland: not yet
> supported.

## Rules

**PLW1 — Append at the end of the struct.** The field is added after
`compositor_memory_limit_mb`. Inserting elsewhere changes the aggregate
initialisation order used by every caller.

**PLW2 — Default is `false` and must stay `false`.** `widget_unittest.cc` has an
explicit `HeadlessInitParamDefaultsFalse` test. A `true` default would make
every Chromium window invisible on platforms that honour the flag.

**PLW3 — "Headless" here means unmapped, not `headless_display`.** The
compositor must keep running. Do not use this flag to short-circuit
compositor creation.

**PLW4 — The comment is the contract, including the Wayland caveat.** It is the
only record that Wayland is unimplemented. Update it when a platform is added
rather than deleting the parenthetical.

**PLW5 — The flag is consumed, not self-acting.** Setting it here does nothing
on its own; the per-platform implementations read it. A change to this field
requires matching changes in `ui/ozone/platform/x11/`,
`ui/views/widget/desktop_aura/`, `ui/views/cocoa/`, and
`components/remote_cocoa/`.

**PLW6 — `ui/platform_window/` also holds `platform_window.h`, `platform_window_delegate.h`,
etc. upstream.** None are patched. Adding a patch here means a new file in this
overlay directory.

## Workflows

**Adding a new platform-window creation property**
1. Append the field with a `//` comment stating the OS-level effect and which
   platforms implement it.
2. Thread it from `Widget::InitParams` in
   `ui/views/widget/desktop_aura/desktop_window_tree_host_platform.cc`.
3. Implement it in every platform backend that supports it.
4. Add a default-value test alongside `HeadlessInitParamDefaultsFalse` in
   `chromium_patches/ui/views/widget/widget_unittest.cc`.
5. Register all touched paths in `features.yaml`.

**Debugging "the headless flag does nothing"**
1. Confirm the field is set on the `PlatformWindowInitProperties` (log at
   `desktop_window_tree_host_platform.cc`).
2. Confirm the platform backend reads it — only X11 and Windows currently do.
3. On macOS, confirm it reached `PlatformWindowInitProperties` *and* was
   separately copied into the mojom `is_headless`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/` subtree rules and the full flag chain.
- [`../views/AGENTS.md`](../views/AGENTS.md) — the views-layer consumers.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
