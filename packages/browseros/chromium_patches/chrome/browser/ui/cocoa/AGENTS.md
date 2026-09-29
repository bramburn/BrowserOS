# `chrome/browser/ui/cocoa/` — macOS dock icon

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

A small macOS-only pair: `dock_icon.h` / `dock_icon.mm` add a tint method
to Chromium's `DockIcon` class so the BrowserOS app icon can be recoloured
per release channel.

## Contents

```
cocoa/
├── dock_icon.h    ← declares - (void)setDockIconVariantColor:(NSColor*)color;
└── dock_icon.mm   ← AppIconWithVariantTint(): composites appIcon + tint
                     (SourceOver → Color fill → DestinationIn)
```

## Rules

**CO1 — macOS only.** Both files are Objective-C/Objective-C++ and are only
compiled on `is_mac`. The consumer
(`browser/chrome_browser_application_mac.mm`) is likewise mac-only. Nothing
here has a Windows/Linux equivalent.

**CO2 — The tint algorithm is fixed.** `AppIconWithVariantTint` draws the
base icon, fills with `NSCompositingOperationColor` to knock out the
colours, then re-masks with `NSCompositingOperationDestinationIn` to
restore the alpha channel. Changing the order produces a solid rectangle
instead of a tinted icon.

**CO3 — The variant list lives with the caller, not here.**
`chrome_browser_application_mac.mm` owns
`kBrowserOSDockIconVariants[]` (`dev` → `0x10b981` green, `alpha` →
`0xd946ef` magenta, `beta` → `0x06b6d4` cyan) and the `browseros::kDockIcon`
switch lookup. Add a channel there, not in `dock_icon.mm`.

**CO4 — Requires ARC.** The `.mm` uses ARC; do not introduce
`retain`/`release` calls.

## Workflows

**Adding a release-channel dock icon tint**
1. Add a `{name, rgb}` row to `kBrowserOSDockIconVariants` in
   `chrome/browser/chrome_browser_application_mac.mm`.
2. Pass `--dock-icon=<name>`; the switch constant is
   `browseros::kDockIcon` in
   `chrome/browser/browseros/core/browseros_switches.h`.
3. No change is needed in this directory.

**Changing how the icon is composited**
Edit `AppIconWithVariantTint` in `dock_icon.mm` only. Verify on a real
macOS build; there is no unit test and no Linux/Windows path.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../../chrome_browser_application_mac.mm`](../../chrome_browser_application_mac.mm)
  — the variant table and switch handling.
- [`../../browseros/core/AGENTS.md`](../../browseros/core/AGENTS.md) —
  `kDockIcon` switch constant.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
