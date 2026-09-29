# `components/remote_cocoa/` — macOS headless-window IPC flag

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Two files forming the mojom half of the agent's hidden-window feature. The
`common/` mojom struct gains a `bool is_headless` field, and the `app_shim/`
bridge reads it to mark the NSWindow headless. Together they let BrowserOS open
windows that render and paint but never appear in Dock, Mission Control, or
Cmd-Tab.

Both files are **macOS-only** and are **not listed in any `features.yaml`
feature block** — they apply by path but never appear in an annotated commit.

## Contents

```
remote_cocoa/
├── app_shim/
│   └── native_widget_ns_window_bridge.mm   ← params->is_headless || Screen::Get()->IsHeadless()
└── common/
    └── native_widget_ns_window.mojom      ← + bool is_headless in NativeWidgetNSWindowInitParams
```

The consumer chain that sets `params->is_headless` lives outside this folder:

```
ui/platform_window/platform_window_init_properties.h   → bool headless
        ↓  ui/views/widget/desktop_aura/desktop_window_tree_host_platform.cc
   (mac) ui/views/cocoa/native_widget_mac_ns_window_host.mm  → window_params->is_headless
        ↓
   components/remote_cocoa/common  →  app_shim
```

## Rules

**RC1 — Mojom field and its reader must land together.** A mojom field that
nothing reads is dead weight; reading a field that the generator never
produces is a hard compile error on the mac build.

**RC2 — Mojom fields are append-only and never reordered.** The struct is
serialised across the browser↔app-shim boundary; inserting a field in the
middle breaks wire compatibility with any out-of-date shim.

**RC3 — `is_headless` is an OR, not a replacement.** The bridge condition is
`params->is_headless || display::Screen::Get()->IsHeadless()`. Removing the
second term breaks `--headless` mode.

**RC4 — Unregistered in `features.yaml`.** When you touch either file, add the
path to a feature block (the headless-window work is closest to
`mac-sparkle-updater`, but any block is better than none) so the change is
annotated and reviewable.

**RC5 — No Windows compile coverage.** `native_widget_ns_window_bridge.mm` is
Objective-C++ and only builds on mac. Verify on a mac build host; the Windows
dev box will happily pass this file over.

**RC6 — Cross-check the whole chain when changing the flag name.** Six files
across `ui/` and `components/remote_cocoa/` spell `headless` /
`is_headless`. Renaming one end alone is a build break you cannot see from
Windows.

## Workflows

**Changing the headless flag name**
1. `ui/platform_window/platform_window_init_properties.h` — the platform-level field.
2. `ui/views/widget/desktop_aura/desktop_window_tree_host_platform.cc` — the converter.
3. `ui/views/cocoa/native_widget_mac_ns_window_host.mm` — the mac assignment.
4. `components/remote_cocoa/common/native_widget_ns_window.mojom` — the mojom field.
5. `components/remote_cocoa/app_shim/native_widget_ns_window_bridge.mm` — the reader.
6. Build on mac; the Windows host compiles none of it.

**Adding a second headless nuance (e.g. skip-Dock on Windows)**
1. Extend `PlatformWindowInitProperties` in `ui/platform_window/`.
2. Implement per platform: `ui/ozone/platform/x11/`, `ui/views/widget/desktop_aura/`,
   `ui/views/cocoa/`, and the remote_cocoa mojom for the app-shim path.
3. Update the `// BrowserOS:` comment blocks — they are the only spec.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`../../ui/AGENTS.md`](../../ui/AGENTS.md) — the originating `headless` init param.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
