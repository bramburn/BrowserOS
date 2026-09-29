# `components/remote_cocoa/common/` — the `is_headless` mojom field

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A single-hunk unified diff adding one boolean to a mojom struct that crosses
the Chromium browser↔app-shim process boundary on macOS:

```
struct NativeWidgetNSWindowInitParams {
  ...
  bool is_tooltip;
  bool is_headless;   // ← added
};
```

Feature block: **none** (unregistered in `features.yaml`).

## Contents

```
common/
└── native_widget_ns_window.mojom   ← +3 lines (comment pair + field)
```

The reader is `../app_shim/native_widget_ns_window_bridge.mm`, which calls
`[window_ setIsHeadless:YES]` when the flag is set.

## Rules

**RCC1 — Append at the end of the struct.** `NativeWidgetNSWindowInitParams` is
mojom-serialised; inserting before existing fields shifts the wire layout and
breaks any shim built from a different revision.

**RCC2 — The mojom comment is the spec.** "InitWindow installs the per-window
headless swizzler so the NSWindow never appears in Dock, Mission Control, or
Cmd-Tab" is the only written contract for this field. Update it whenever the
behaviour changes.

**RCC3 — Ship with its reader.** A field in this file without a matching branch
in `../app_shim/native_widget_ns_window_bridge.mm` is a no-op that will be
mistaken for a working feature.

**RCC4 — Mojom regeneration is not part of the patch.** The generated bindings
come from Chromium's build; the patch only edits the `.mojom` source.

## Workflows

**Adding another init param**
1. Append the field to `NativeWidgetNSWindowInitParams` with a `//` comment
   stating the OS-level effect.
2. Set it in `ui/views/cocoa/native_widget_mac_ns_window_host.mm`.
3. Consume it in `components/remote_cocoa/app_shim/native_widget_ns_window_bridge.mm`.
4. Add both paths to a `features.yaml` block.

**Diagnosing "headless flag has no effect on mac"**
1. Confirm the browser sets it: check
   `ui/views/cocoa/native_widget_mac_ns_window_host.mm` assigns
   `window_params->is_headless`.
2. Confirm the shim reads it: check the bridge's condition includes
   `params->is_headless`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/remote_cocoa/` rules.
- [`../app_shim/AGENTS.md`](../app_shim/AGENTS.md) — the reader.
- [`../../ui/AGENTS.md`](../../../ui/AGENTS.md) — where the flag originates.
