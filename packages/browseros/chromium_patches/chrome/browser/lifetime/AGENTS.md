# `lifetime/` — relaunch path

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

One file, one hunk pair, one decision. `application_lifetime.cc` gets a
`#if BUILDFLAG(ENABLE_SPARKLE)` forward declaration of two functions in the
`sparkle_glue` namespace, and `AttemptRelaunch()` checks
`sparkle_glue::IsUpdateReady()` first — if a Sparkle update has been staged, it
calls `sparkle_glue::InstallAndRelaunch()` and returns, skipping
`AttemptRestart()`. Without an update ready, relaunch behaves exactly as
upstream Chromium.

## Contents

```
lifetime/
└── application_lifetime.cc   ← ENABLE_SPARKLE block + one branch in
                                AttemptRelaunch()
```

## Rules

**LT1 — The Sparkle branch must come before `AttemptRestart()`.**
`AttemptRestart()` relaunches via the platform's restart mechanism, which
discards the staged update. Returning early is the whole point of the patch;
reordering the two silently downgrades updates to a no-op.

**LT2 — The `sparkle_glue` functions are re-declared here, not included.**
The header is Objective-C++ (`chrome/browser/mac/sparkle_glue.h` imports
`<Foundation/Foundation.h>`), so `application_lifetime.cc` forward-declares the
two C++ entry points inside the same `BUILDFLAG(ENABLE_SPARKLE)` guard. Do not
"simplify" this to an `#include` — that pulls Objective-C into a `.cc` file
that compiles on every platform.

**LT3 — `ENABLE_SPARKLE` comes from `chrome/browser/sparkle_buildflags.gni`**
and defaults to `is_mac`. This file includes `chrome/browser/buildflags.h` for
it. The same buildflag gates `../mac/chrome_browser_main_extra_parts_mac.mm`
and `../upgrade_detector/upgrade_detector_impl.cc`; all three must agree.

**LT4 — `AttemptUserExit()` is not touched.** Quitting is not relaunching; a
user-initiated quit must not install a staged update. Keep the Sparkle hook
confined to `AttemptRelaunch()`.

**LT5 — `IsUpdateReady()` is the only gate.** Do not call
`sparkle_glue::SparkleEnabled()` here — that reports whether Sparkle is
*initialised*, not whether an update is *ready*. On a machine where Sparkle is
enabled but idle, this would relaunch into an install of nothing.

## Workflows

**Changing what happens on relaunch**
1. Edit only `AttemptRelaunch()` in `application_lifetime.cc`.
2. Keep the Sparkle guard and the early `return`.
3. Re-extract the diff; it is a two-hunk patch, so any extra hunk is a
   conflict on the next Chromium bump.

**Verifying Sparkle is driving relaunch on macOS**
1. Check `ENABLE_SPARKLE` is on (it defaults to `is_mac`).
2. Confirm `PreCreateMainMessageLoop()` in
   `../mac/chrome_browser_main_extra_parts_mac.mm` ran and initialised Sparkle.
3. Stage an update (`--browseros-sparkle-force-check`), then relaunch and
   confirm `IsUpdateReady()` short-circuits `AttemptRestart()`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay; BR2 covers
  `ENABLE_SPARKLE`.
- [`../mac/AGENTS.md`](../mac/AGENTS.md) — `sparkle_glue.{h,mm}` and the
  `PreCreateMainMessageLoop()` hook.
- [`../upgrade_detector/AGENTS.md`](../upgrade_detector/AGENTS.md) — the other
  consumer of `ENABLE_SPARKLE`; zeroed thresholds so the badge appears on
  `NotifyUpgradeReady()`.
- [`../browseros/core/AGENTS.md`](../browseros/core/AGENTS.md) — the
  `--browseros-sparkle-*` switches.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
