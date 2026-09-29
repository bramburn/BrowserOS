# `ui/ozone/` — Ozone platform backends

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../ui/AGENTS.md`](../AGENTS.md).

## What's here

Chromium's Ozone windowing abstraction on Linux. BrowserOS patches exactly one
backend — X11 — to support headless agent windows. The Wayland backend
(`ui/ozone/platform/wayland/`) is **not** patched: headless is documented as
unsupported there.

Feature block: **none** (unregistered in `features.yaml`).

## Contents

```
ozone/
└── platform/
    └── x11/
        ├── x11_window.h   ← + bool is_headless_
        └── x11_window.cc  ← SKIP_TASKBAR/SKIP_PAGER atoms; Show() short-circuit
```

Upstream siblings under `platform/` that BrowserOS leaves untouched: `wayland/`,
`headless/`, `wayland/host/`, `x11/`'s other files (`x11_display.cc`,
`x11_screen_ozone.cc`, `x11_surface_present.cc`, …).

## Rules

**OZN1 — X11 only.** Do not add a parallel patch under `wayland/` without
removing the "not yet supported" note in
`platform_window_init_properties.h`. A half-implemented Wayland headless path
would be worse than a documented gap.

**OZN2 — The two `is_headless_` insertions are a pair.** The member declaration
in `.h` and both uses in `.cc` (`Initialize` and `Show`) must ship together.

**OZN3 — `Show()` must not map the window.** The `is_headless_` branch returns
early, before `Map(inactive)`. Reaching `Map()` makes the window visible.

**OZN4 — The fake delegate notification is intentional.** The branch sets
`window_mapped_in_client_ = true` and calls
`OnWindowStateChanged(kUnknown, kNormal)` so the content compositor runs and
paints. Without it, the window never renders and CDP screenshots come back
blank.

**OZN5 — The `_NET_WM_STATE_SKIP_*` atoms are belt-and-suspenders.** The window
is never mapped, so a compliant WM never sees it; the atoms cover WMs that
would surface it anyway. Keep both.

**OZN6 — `x11_window.h` member placement matters.** The new member sits between
`is_security_surface_` and `is_occluded_`. Reordering is cosmetic but produces
conflict noise on every Chromium bump; keep the diff minimal.

**OZN7 — This is a Linux/X11 code path; it does not build on the Windows dev
host.** Validate on a Linux build host.

## Workflows

**Verifying headless on X11**
1. Launch the agent with a hidden window.
2. `xwininfo -root -tree | grep <window id>` — the window exists but is unmapped.
3. `wmctrl -l` — the window is absent from the list.
4. Take a CDP screenshot of the tab — it must return real pixels.

**Adding headless to Wayland**
1. Implement in `ui/ozone/platform/wayland/wayland_window.{h,cc}`.
2. Skip the surface map/commit, or set the closest analogue to
   `SKIP_TASKBAR` that the protocol offers.
3. Fake the delegate state change the way the X11 patch does.
4. Remove the "not yet supported" caveat from
   `platform_window_init_properties.h`.
5. Register the new paths in `features.yaml`.

**Rebasing after a Chromium bump**
- `x11_window.cc` changes with every X11 backend refactor. Reset to
  `BASE_COMMIT`, re-apply the three insertions, and extract.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/` subtree rules and the flag chain.
- [`platform/AGENTS.md`](platform/AGENTS.md) — next level down.
- [`../platform_window/AGENTS.md`](../platform_window/AGENTS.md) — where the flag is defined.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
