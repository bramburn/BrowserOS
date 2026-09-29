# `ui/ozone/platform/x11/` — X11 headless windows

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The X11 implementation of the headless-window flag. `X11Window` records whether
it is headless, sets the skip-taskbar/skip-pager atoms at creation, and — the
important part — returns from `Show()` without ever calling `Map()`, while
faking the delegate notification the compositor needs.

Feature block: **none** (unregistered in `features.yaml`).

## Contents

```
x11/
├── x11_window.h   ← + bool is_headless_ = false  (after is_security_surface_)
└── x11_window.cc
    ├── X11Window::Initialize()  ← is_headless_ = properties.headless; +2 atoms
    └── X11Window::Show()        ← early return when is_headless_
```

The `Show()` branch in full:

```cpp
if (is_headless_) {
  // Headless: the XWindow stays unmapped so no WM sees it. Notify the
  // delegate as if we mapped so the content compositor runs and paints.
  window_mapped_in_client_ = true;
  platform_window_delegate_->OnWindowStateChanged(
      PlatformWindowState::kUnknown, PlatformWindowState::kNormal);
  return;
}

Map(inactive);
```

## Rules

**X11W1 — Never reach `Map()` for a headless window.** The early return in
`Show()` is the entire feature. Any refactor that moves the `is_headless_` check
after `Map(inactive)` makes agent windows visible.

**X11W2 — The fake `OnWindowStateChanged` call is load-bearing.** It is what
tells the compositor the window is shown, so the page renders and CDP
screenshots return pixels. Removing it stalls rendering while leaving the window
correctly hidden — a confusing failure.

**X11W3 — `window_mapped_in_client_ = true` is a deliberate lie** about the X
server state, set so Chromium's own visibility bookkeeping stays consistent.
Do not "fix" it to reflect the real X state.

**X11W4 — Both `_NET_WM_STATE_SKIP_*` atoms or neither.** `SKIP_TASKBAR` alone
leaves the window in the pager; `SKIP_PAGER` alone leaves it in the taskbar.
They are a set.

**X11W5 — `is_headless_` is captured once in `Initialize()`.** It mirrors
`PlatformWindowInitProperties::headless`; it is never re-read. Toggling the
init property after window creation has no effect.

**X11W6 — Only X11 gets this today.** Wayland is explicitly unsupported (see
the comment in `ui/platform_window/platform_window_init_properties.h`). Do not
assume the agent's headless windows work on a Wayland session.

**X11W7 — Linux-only code path; the Windows dev host does not compile it.**
Validate on a Linux build host.

**X11W8 — `x11_window.cc` is a high-drift file.** X11 backend refactors touch
it regularly. On conflict: reset to `BASE_COMMIT`, re-apply the two
insertions, extract.

## Workflows

**Verifying on a live X11 session**
1. Launch the agent with a hidden window.
2. `xwininfo -root -tree` — the XWindow exists but shows `Map State: IsUnMapped`.
3. `wmctrl -l` / `xdotool search --name` — the window is not listed.
4. `import -window <id>` or a CDP screenshot — real pixels are produced.

**Making a headless window temporarily visible for debugging**
1. Do **not** add a debug flag to this file.
2. Instead, temporarily comment out the `is_headless_` branch locally in
   `<chromium_src>` and debug there.
3. Never extract a debug-only change back into `chromium_patches/`.

**Adding a new X11 headless nuance**
1. Add the atom(s) in the same `if (is_headless_)` block in `Initialize()`.
2. Keep the `Show()` short-circuit first and unconditional.
3. Update the comment in `platform_window_init_properties.h`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/ozone/platform/` rules.
- [`../../../platform_window/AGENTS.md`](../../../platform_window/AGENTS.md) — where `headless` is defined.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
