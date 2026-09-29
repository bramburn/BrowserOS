# `chrome/browser/ui/views/frame/` — native widget headless mode

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/](../../../../../../AGENTS.md).

## What's here

The three `BrowserNativeWidget` implementations. BrowserOS sets
`params.headless` from `Browser::is_hidden()` in each of them, which is how
agent-owned scratch Browsers become truly invisible to the OS compositor.

## Contents

```
frame/
├── browser_native_widget_ash.cc   ← params.headless = browser->is_hidden();
├── browser_native_widget_aura.cc  ← if (browser_view_):
│                                     params.headless =
│                                         browser_view_->browser()->is_hidden();
└── browser_native_widget_mac.mm   ← same as aura, Objective-C++ (macOS)
└── layout/                        ← see layout/AGENTS.md
```

## Rules

**FR1 — All three implementations must change together.** `ash` reads
`browser` directly; `aura` and `mac` read
`browser_view_->browser()` and null-check `browser_view_` first. Missing
one means hidden Browsers are visible on that platform only — with no
compile error and no test failure.

**FR2 — Headless is decided at widget construction.** `is_hidden()` is
`const` on `Browser`; a Browser cannot become visible after creation. The
widget parameters are therefore correct-by-construction and must not be
mutated later.

**FR3 — This is the only place `headless` is set.** A view that wants
"invisible" behaviour must go through
`Browser::CreateParams::hidden`, not by setting
`NativeWindow::SetParamDict` entries.

**FR4 — `browser_view_` can be null in aura/mac.** The null check is
required; the un-hidden default is correct (a normal window).

**FR5 — `layout/` holds tab-strip layout, not widget creation.** See
[`layout/AGENTS.md`](layout/AGENTS.md) for the vertical-tab grab handle.

## Workflows

**Adding a new native widget platform**
1. Add the `params.headless` assignment in the new implementation.
2. Keep the null-check shape used by `aura`/`mac`.
3. Update the "all three" list above if a fourth implementation appears.

**Debugging a hidden Browser appearing in the taskbar**
1. Confirm `Browser::CreateParams::hidden = true` at the creation site.
2. Confirm this platform's file sets `params.headless`.
3. Confirm the window was not created through a path that skips
   `BrowserNativeWidget` entirely.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../../browser.h`](../../browser.h) — `CreateParams::hidden`,
  `is_hidden()`, `PinHiddenTabVisibility()`.
- [`../../browser_list.h`](../../browser_list.h) —
  `ShouldShowBrowserInUserInterface()`.
- [`layout/AGENTS.md`](layout/AGENTS.md) — tab-strip layout constants.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
