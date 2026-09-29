# `chrome/browser/ui/webui/help/` — macOS Sparkle version updater

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The macOS auto-update path. It adds a `VersionUpdater` implementation
backed by the Sparkle framework and wires it into Chromium's existing
`version_updater_mac.mm` factory, replacing the Google Updater path when
Sparkle is enabled.

## Contents

```
help/
├── sparkle_version_updater_mac.h   ← NEW: class SparkleVersionUpdater : public
│                                     VersionUpdater; forward-declares the
│                                     SparkleVersionUpdaterBridge (@interface)
├── sparkle_version_updater_mac.mm  ← NEW: requires ARC; SparkleObserver bridge
│                                     class forwarding callbacks to C++; uses
│                                     chrome/browser/mac/sparkle_glue.h
└── version_updater_mac.mm         ← in CreateVersionUpdater():
      #if BUILDFLAG(ENABLE_SPARKLE)
        if (sparkle_glue::SparkleEnabled()) return new SparkleVersionUpdater();
```

## Rules

**HP1 — Everything is behind `BUILDFLAG(ENABLE_SPARKLE)`.** The factory
returns the Sparkle updater only when the buildflag is set *and*
`sparkle_glue::SparkleEnabled()` is true. The buildflag comes from
`chrome/browser/sparkle_buildflags.gni` (`enable_sparkle = is_mac`) and is
propagated through `chrome/browser/buildflags.gni` and
`chrome/BUILD.gn`.

**HP2 — ARC is mandatory.** `sparkle_version_updater_mac.mm` has
`#error "This file requires ARC support."`. Do not add manual
`retain`/`release`.

**HP3 — The Objective-C bridge is one class.**
`SparkleVersionUpdaterBridge` implements `SparkleObserver` and forwards to
the C++ `SparkleVersionUpdater`. Keep the ObjC surface minimal; the C++
class owns all state.

**HP4 — `sparkle_glue.h` is a separate target.**
`chrome/browser/mac/sparkle_glue.{h,mm}` is only compiled when
`enable_sparkle` is true (see `chrome/browser/BUILD.gn`). Including it
unguarded breaks Windows/Linux.

**HP5 — These files are added to `chrome/browser/ui/BUILD.gn`, not to
`webui/BUILD.gn`.** Look for the two `webui/help/sparkle_version_updater_mac.*`
lines in the `is_mac` sources block of `chrome/browser/ui/BUILD.gn`.

**HP6 — Owned by the `mac-sparkle-updater` feature** in
`packages/browseros/build/features.yaml`, together with
`chrome/app/app-Info.plist`, `chrome/browser/mac/*`,
`chrome/browser/sparkle_buildflags.gni`, and
`chrome/browser/upgrade_detector/upgrade_detector_impl.cc`. The Sparkle
`SUPublicEDKey` in `app-Info.plist` must match the shipped signing key.

## Workflows

**Switching the macOS updater off**
1. Set `enable_sparkle = false` in
   `chrome/browser/sparkle_buildflags.gni`.
2. Confirm `chrome/browser/buildflags.gni` no longer sets
   `enable_update_notifications` from it.
3. The factory falls back to Chromium's default updater; verify the
   `VLOG(1)` branches in `version_updater_mac.mm` still compile.

**Adding a new Sparkle callback**
1. Add the `- (void)…` method to `SparkleObserverBridge` in the `.mm`.
2. Forward it to a `VersionUpdater` virtual in the `.h`.
3. Keep the `#if BUILDFLAG(ENABLE_SPARKLE)` guard on any new include.

**Debugging "no update prompt on macOS"**
1. Is `enable_sparkle` true in the built args?
2. Does `sparkle_glue::SparkleEnabled()` return true?
3. Check the appcast URL in `chrome/app/app-Info.plist`
   (`SUFeedURL`) and the `kSentryMinidumpUrl`-style constants in
   `chrome/browser/browseros/server/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/webui/`.
- [`../../../mac/AGENTS.md`](../../../mac/AGENTS.md) — Sparkle glue
  and browser main extra parts.
- [`../../../../app/AGENTS.md`](../../../../app/AGENTS.md) —
  `app-Info.plist` Sparkle keys.
- [`../../../sparkle_buildflags.gni`](../../../sparkle_buildflags.gni)
  — `enable_sparkle`.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
