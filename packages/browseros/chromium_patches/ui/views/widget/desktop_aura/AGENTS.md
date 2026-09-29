# `ui/views/widget/desktop_aura/` — Aura headless window hosts

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The Aura tree-host layer: the bridge from `Widget::InitParams` to
`PlatformWindowInitProperties`, and the Windows host that honours the flag by
never calling `ShowWindow`.

## Contents

```
desktop_aura/
├── desktop_window_tree_host_platform.cc  ← ConvertWidgetInitParamsToInitProperties(): + properties.headless = params.headless
├── desktop_window_tree_host_win.h        ← + bool is_headless_ = false
└── desktop_window_tree_host_win.cc       ← Init(): capture; Show(): early return
```

The `Show()` branch:

```cpp
if (is_headless_) {
  // Headless: keep the aura side "visible" (so compositor runs + WebContents
  // sees a shown widget), but never call ShowWindow(SW_SHOW*) on the HWND.
  content_window()->Show();
  return;
}
```

Feature block: **none** (unregistered in `features.yaml`).

## Rules

**DAU1 — The `content_window()->Show()` before the return is required.** Without
it, the Aura widget never transitions to visible and the compositor does not
run — pages render blank and CDP screenshots fail. This is the Windows analogue
of X11's faked `OnWindowStateChanged`.

**DAU2 — The early return must precede every `ShowWindow` variant.** The hunk is
placed immediately after `OnAcceleratedWidgetMadeVisible(true)` and before the
`WindowShowState` handling that computes restore bounds. Do not move it down.

**DAU3 — `is_headless_` is read-only after `Init()`.** The member is captured in
`Init()` and consulted in `Show()`. Do not add a setter; the flag is fixed at
construction.

**DAU4 — `desktop_window_tree_host_platform.cc` is platform-generic and must
stay generic.** It copies the field and nothing else. Windows-only or
X11-only logic belongs in the respective backend.

**DAU5 — `_win` files are Windows-only at compile time.** A Linux build skips
them entirely, so a broken hunk can pass CI and fail on a developer's Windows
machine. Verify on a Windows build.

**DAU6 — Linux gets its headless behaviour from
`ui/ozone/platform/x11/x11_window.cc`, not from here.** The Aura Win host and the
X11 window are independent implementations of the same flag.

## Workflows

**Debugging "the window flashes before disappearing"**
1. Confirm the early return is before *all* `ShowWindow` calls in `Show()`.
2. Confirm `is_headless_` was set in `Init()` (it is the first statement).
3. Check for a second `Show()` call path — e.g. the re-activation path when
   restoring a window.

**Adding a new Aura host platform (e.g. a new Win flavour)**
1. Ensure it derives from `DesktopWindowTreeHostPlatform`, which gets the field
   via `desktop_window_tree_host_platform.cc`.
2. Implement the `Show()` suppression the way the Win host does.
3. Add the path to `features.yaml`.

**Rebasing**
- Aura files are moderately high churn. Reset to `BASE_COMMIT`, re-apply the
  three insertions, extract.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/views/widget/` rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `ui/views/` rules and the flag chain.
- [`../../../ozone/platform/x11/AGENTS.md`](../../../ozone/platform/x11/AGENTS.md) — the X11 implementation.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
