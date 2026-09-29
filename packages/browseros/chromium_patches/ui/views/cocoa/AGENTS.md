# `ui/views/cocoa/` — macOS headless window dispatch

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The single macOS-side line of the headless-window chain. When a
`NativeWidgetHost` builds its `NativeWidgetNSWindowInitParams`, it copies
`Widget::InitParams::headless` into the mojom field `is_headless`, which the
app-shim process reads in
`components/remote_cocoa/app_shim/native_widget_ns_window_bridge.mm`.

## Contents

```
cocoa/
└── native_widget_mac_ns_window_host.mm   ← + window_params->is_headless = params.headless;
```

The hunk sits next to the sibling assignments `is_translucent` and `is_tooltip`.

Feature block: **none** (unregistered in `features.yaml`).

## Rules

**COC1 — This is the macOS entry point of a four-file chain.** The flag's
journey is:
`ui/views/widget/widget.h` → `ui/views/cocoa/native_widget_mac_ns_window_host.mm`
→ `components/remote_cocoa/common/native_widget_ns_window.mojom` →
`components/remote_cocoa/app_shim/native_widget_ns_window_bridge.mm`.
Breaking any link compiles and silently does nothing.

**COC2 — macOS does not use `PlatformWindowInitProperties` for this.** The
Windows/Linux path routes through
`ui/views/widget/desktop_aura/desktop_window_tree_host_platform.cc`; macOS sets
the mojom directly. Do not add a redundant `PlatformWindowInitProperties` read
here.

**COC3 — ObjC++ (`native_widget_mac_ns_window_host.mm`); macOS build only.**
There is no Windows compile coverage. A typo ships undetected from this host.

**COC4 — Keep the assignment grouped with `is_translucent` / `is_tooltip`.** All
three are `NativeWidgetNSWindowInitParams` fields set in the same block.
Scattering the new one reduces readability and rebase survivability.

**COC5 — `is_headless` is a mojom field; add it first.** The mojom declaration
in `components/remote_cocoa/common/native_widget_ns_window.mojom` must exist
before this compiles.

## Workflows

**A headless window shows in the Dock on macOS**
1. Confirm this assignment exists and `params.headless` is `true` at the
   `BrowserWindow` construction site.
2. Confirm the mojom field is declared and generated.
3. Confirm the shim's condition includes `params->is_headless`.
4. Confirm the mojom/binary versions match — a stale app shim drops new fields.

**Renaming `is_headless`**
1. `components/remote_cocoa/common/native_widget_ns_window.mojom` (append-only:
   *add* a new field rather than renaming, and deprecate the old one).
2. `components/remote_cocoa/app_shim/native_widget_ns_window_bridge.mm`.
3. This file.
4. Build on macOS; the Windows host compiles none of the three.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/views/` rules and the two dispatch paths.
- [`../../components/remote_cocoa/AGENTS.md`](../../../components/remote_cocoa/AGENTS.md) — the mojom and shim.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
