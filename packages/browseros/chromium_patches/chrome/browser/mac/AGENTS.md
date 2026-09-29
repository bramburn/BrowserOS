# `mac/` — Sparkle updater glue and macOS browser-main parts

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The macOS half of the browser auto-update. `sparkle_glue.{h,mm}` is a ~750-line
Objective-C++ wrapper over the vendored Sparkle framework: a `SparkleGlue`
singleton with a status enum, download progress, weak observers and
`checkForUpdates` / `installAndRelaunch`, plus a small C++ surface
(`SparkleEnabled()`, `IsUpdateReady()`, `InstallAndRelaunch()`) that plain
`.cc` files can call. `chrome_browser_main_extra_parts_mac.{h,mm}` adds a
`PreCreateMainMessageLoop()` override that triggers the Sparkle singleton as
early as possible.

## Contents

```
mac/
├── sparkle_glue.h    ← SparkleStatus enum (Idle → Checking → Downloading →
│                        Extracting → ReadyToInstall → Installing →
│                        UpToDate → Error), SparkleProgress, SparkleObserver
│                        protocol, SparkleGlue singleton, and the
│                        `namespace sparkle_glue` C++ shim
├── sparkle_glue.mm   ← implementation (the bulk of the directory)
├── chrome_browser_main_extra_parts_mac.h  ← adds PreCreateMainMessageLoop() override
└── chrome_browser_main_extra_parts_mac.mm ← calls sparkle_glue::SparkleEnabled()
                                              under BUILDFLAG(ENABLE_SPARKLE)
```

## Rules

**MAC1 — Every `#if` here is `BUILDFLAG(ENABLE_SPARKLE)`, and
`ENABLE_SPARKLE` defaults to `is_mac`.** It is declared in
`../sparkle_buildflags.gni`. The other consumers are
`../lifetime/application_lifetime.cc`,
`../upgrade_detector/upgrade_detector_impl.cc` and `../BUILD.gn`. All of them
must read the same buildflag; a locally-defined duplicate will drift.

**MAC2 — `SparkleGlue` is main-thread-only, and the header says so.**
"Thread-safety: All methods must be called on the main thread." Observers are
held *weakly* — a caller must keep its observer object alive itself. Do not
convert the observer list to strong references to fix a crash; that is a
lifetime bug in the observer, not in the glue.

**MAC3 — `SparkleEnabled()` reports initialisation, not update
availability.** `IsUpdateReady()` is the one `../lifetime/` gates on. Calling
the wrong one causes a relaunch into an empty install (see LT5 in
`lifetime/AGENTS.md`).

**MAC4 — Initialisation happens in `PreCreateMainMessageLoop()`, not
`PreEarlyInitialization()`.** `PreEarlyInitialization()` still only creates
the `display::ScopedNativeScreen`. The comment in the `.mm` explains the
choice: touching the Sparkle singleton triggers its full setup, including the
disabled-updates and read-only-filesystem checks, and that must happen before
the message loop takes ownership.

**MAC5 — The C++ shim is a `namespace`, not a class.**
`namespace sparkle_glue { bool SparkleEnabled(); bool IsUpdateReady();
void InstallAndRelaunch(); }`. The `.cc` files that call it
(`../lifetime/application_lifetime.cc`) forward-declare these three functions
rather than including the header, because the header is Objective-C++ (LT2 in
`lifetime/AGENTS.md`). Keep the shim to exactly these three entry points.

**MAC6 — `.mm` files here are macOS-only by construction.** Any `.cc` that
references them needs a matching `#if BUILDFLAG(IS_MAC)` /
`BUILDFLAG(ENABLE_SPARKLE)` guard, or the Linux and Windows builds break.

## Workflows

**Adding a new Sparkle capability**
1. Add the Objective-C surface to `sparkle_glue.h` (inside
   `NS_ASSUME_NONNULL_BEGIN`/`END`).
2. Implement it in `sparkle_glue.mm`.
3. If a `.cc` file needs it, add a free function to the
   `namespace sparkle_glue` block at the bottom of the header.
4. Update `../BUILD.gn` if the source list changes.

**Changing when Sparkle initialises**
1. Edit `PreCreateMainMessageLoop()` in
   `chrome_browser_main_extra_parts_mac.mm` only.
2. Keep `PreEarlyInitialization()` doing only the `ScopedNativeScreen`.
3. Confirm `../lifetime/application_lifetime.cc` still sees a ready instance by
   the time `AttemptRelaunch()` can run.

**Debugging a silent macOS update**
1. Run with `--browseros-sparkle-verbose` (declared in
   `../browseros/core/browseros_switches.h`).
2. Check `SparkleStatus` transitions: a stuck `Checking` is a framework
   problem, a stuck `Extracting` is a signature or filesystem problem.
3. Use `--browseros-sparkle-url` to point at a test appcast; `--sparkle-dry-run`
   and `--sparkle-spoof-version` exist for the same purpose.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay; BR2
  (`enable_sparkle` lives in exactly one place).
- [`../lifetime/AGENTS.md`](../lifetime/AGENTS.md) — the relaunch hook.
- [`../upgrade_detector/AGENTS.md`](../upgrade_detector/AGENTS.md) — zeroed
  annoyance thresholds under Sparkle.
- [`../browseros/core/AGENTS.md`](../browseros/core/AGENTS.md) — the
  `--browseros-sparkle-*` and `--sparkle-*` switches.
- [`../../../third_party/sparkle/AGENTS.md`](../../../third_party/sparkle/AGENTS.md)
  — the vendored framework this wraps.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
