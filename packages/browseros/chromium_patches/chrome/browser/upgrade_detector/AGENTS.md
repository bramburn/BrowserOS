# `upgrade_detector/` — Sparkle-aware update thresholds

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

`upgrade_detector_impl.cc` is patched in five places, all under
`BUILDFLAG(ENABLE_SPARKLE)` (true on macOS, the platform that uses Sparkle).
Under Sparkle, `ShouldDetectOutdatedBuilds()` returns `false` outright,
`DoCalculateThresholds()` zeroes all five annoyance stages so the menu-bar
badge appears on the same call stack as `NotifyUpgradeReady()`, and
`Init()` skips the `installed_version_poller_` because Sparkle drives versions
through the appcast rather than a filesystem poll. The five
`kDefault*Threshold` constants become `[[maybe_unused]]` for the same reason.
Three `VLOG(1)` lines are added for tracing.

## Contents

```
upgrade_detector/
└── upgrade_detector_impl.cc   ← ENABLE_SPARKLE: ShouldDetectOutdatedBuilds() → false
                                  DoCalculateThresholds(): all 5 stages = base::TimeDelta()
                                  Init(): no installed_version_poller_
                                  [[maybe_unused]] on the 5 kDefault*Threshold constants
                                  3× VLOG(1) in UpgradeDetected(),
                                  NotifyOnUpgradeWithTimePassed(), OnUpdate()
                                  - DCHECK(!stages_[0].is_zero()) in
                                  GetThresholdForLevel()
```

## Rules

**UD1 — Zeroed stages are the Sparkle contract, not a bug.**
`DoCalculateThresholds()` sets every stage to `base::TimeDelta()` under Sparkle
so the badge surfaces immediately, with the inline comment explaining it:
"no timer delay needed." Restoring the day/hour thresholds under Sparkle
reintroduces a wait the Sparkle path has no way to satisfy.

**UD2 — The removed `DCHECK(!stages_[0].is_zero())` is required.**
`GetThresholdForLevel()` had a `DCHECK` that the first stage is non-zero. That
holds for the Chromium timer path and is *false* for the zeroed Sparkle stages,
so the patch deletes it. Re-adding it crashes the Sparkle build in debug.

**UD3 — `[[maybe_unused]]` on the threshold constants is required under
Sparkle.** The whole `GetRelaunchNotificationPeriod()` / `GetRelaunchWindowPolicyValue()`
block is inside the `#else` of the Sparkle branch, which makes the constants
unused. Dropping the attribute is a `-Wunused-const-variable` error.

**UD4 — `installed_version_poller_` must not be constructed under Sparkle.**
`Init()` only creates it inside `#if !BUILDFLAG(ENABLE_SPARKLE)`. Sparkle
reports available versions by calling `NotifyUpgradeReady()`; the poller would
race it and report a stale state.

**UD5 — `ShouldDetectOutdatedBuilds()` returns `false` under Sparkle, and it
comes first in the `#if` chain.** It is checked before the
`ENABLE_UPDATE_NOTIFICATIONS && !IS_CHROMEOS` branch and its brand-code logic
(organic brand check) is unchanged. Reordering changes which builds show the
"outdated" bubble.

**UD6 — The `#if`/`#else`/`#endif` structure in `DoCalculateThresholds()` must
stay balanced and in that order.** The patch adds `#if BUILDFLAG(ENABLE_SPARKLE)`
at the top of the function body and `#endif` after the existing block.

**UD7 — The `VLOG(1)` traces are diagnostic only.** They log the upgrade type,
the stage transition with the computed `next_delay`, and `BuildState`'s update
type. Keep them at `VLOG(1)`; raising them to `LOG(INFO)` floods the log on
every stage tick.

## Workflows

**Debugging a missing update badge on macOS**
1. Confirm `ENABLE_SPARKLE` is on (it defaults to `is_mac`).
2. Run with `--v=1` and look for the `UpgradeDetector:` VLOG lines in
   `UpgradeDetected()` and `NotifyOnUpgradeWithTimePassed()`.
3. If `UpgradeDetected()` fires but the stage does not change, check
   `DoCalculateThresholds()` — the Sparkle branch must be zeroing all five
   stages.
4. If nothing fires at all, check `../mac/chrome_browser_main_extra_parts_mac.mm`
   initialised Sparkle (`PreCreateMainMessageLoop()`).

**Adding a new annoyance stage**
1. Add the constant next to the `kDefault*Threshold` block — with
   `[[maybe_unused]]`.
2. Add a `stages_[kStagesIndex…] = base::TimeDelta();` line in the Sparkle
   branch of `DoCalculateThresholds()`.
3. Add the real threshold in the `#else` branch.
4. Leave `GetThresholdForLevel()` alone — it must not regain a non-zero check.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay; BR2 on
  `ENABLE_SPARKLE`.
- [`../lifetime/AGENTS.md`](../lifetime/AGENTS.md) — the relaunch path that installs
  a staged Sparkle update.
- [`../mac/AGENTS.md`](../mac/AGENTS.md) — `sparkle_glue.{h,mm}` and Sparkle init.
- [`../browseros/core/AGENTS.md`](../browseros/core/AGENTS.md) — the
  `--browseros-sparkle-*` and `--sparkle-*` switches.
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — the separate,
  non-Sparkle OTA path for the `browseros_server` sidecar.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
