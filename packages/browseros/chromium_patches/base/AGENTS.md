# `base/` — low-level Chromium patches

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../AGENTS.md).

## What's here

Two independent patches against the `base/` layer of Chromium — the code every
other target depends on, so changes here have the widest blast radius in the
build. `threading/` makes the BrowserOS server manager a friend of two
thread-restriction guards; `version_info/` teaches the version generator about
a second, BrowserOS-specific version string.

Both belong to different `features.yaml` blocks: `base/threading/` under
`server`, `base/version_info/` under `branding`.

## Contents

```
base/
├── threading/
│   └── thread_restrictions.h   ← friend declarations for browseros::BrowserOSServerManager
└── version_info/
    ├── BUILD.gn                      ← adds //chrome/BROWSEROS_VERSION to generate_version_info sources
    ├── version_info.h                ← adds GetBrowserOSVersionNumber()
    └── version_info_values.h.version ← template adding #define BROWSEROS_VERSION
```

## Rules

**BA1 — `base/` patches are API surface, not behaviour.** `base/` is included
by every target including `base/test`. Adding a `#include` here or changing a
signature can break test binaries that never link against `chrome/`.

**BA2 — The two subtrees are owned by different features.** Do not move
`base/version_info/*` under the `server` block just because both are "base" —
`features.yaml` drives commit boundaries, and `branding` is a reviewable
unit on its own.

**BA3 — `version_info_values.h.version` is a GN template, not a header.** Its
`@BROWSEROS_MAJOR@`-style placeholders are substituted from
`//chrome/BROWSEROS_VERSION` (declared in `BUILD.gn`). Editing the `.version`
file without editing `BUILD.gn` yields a header that never defines
`BROWSEROS_VERSION` and fails to compile every consumer.

**BA4 — Don't reuse `PRODUCT_VERSION` for BrowserOS versions.** The whole point
of `GetBrowserOSVersionNumber()` is that the two diverge; collapsing them
breaks the OTA/updater comparison in `chrome/browser/browseros/server/`.

**BA5 — Friend declarations are the sanctioned escape hatch here.** Never
delete `friend class browseros::BrowserOSServerManager;` to "fix" a build
error caused by new code violating a thread check — fix the code.

## Workflows

**Bumping the BrowserOS version**
1. Edit `packages/browseros/chromium_patches/chrome/BROWSEROS_VERSION` (not in
   this folder; `features.yaml` lists it under `branding`, but the patch file is
   currently absent from the overlay).
2. No change needed here — `version_info_values.h.version` already
   interpolates the four components.

**Adding a new thread-restriction exemption**
1. Edit `<chromium_src>/base/threading/thread_restrictions.h`.
2. Add the `namespace` forward declaration near the existing
   `blink` / `cc` blocks.
3. Add the `friend class` line to the specific guard class.
4. Extract the diff back to `chromium_patches/base/threading/`.

**Adding a new generated version field**
1. Add the `#define` to `version_info_values.h.version` with `@UPPER@` placeholders.
2. Add the source file to `sources` in `version_info/BUILD.gn`.
3. Add the accessor `constexpr` to `version_info.h` next to
   `GetBrowserOSVersionNumber()`.
4. List `base/version_info/*` under the `branding` feature in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview, rule F2 (path mirroring).
- [`../../build/features.yaml`](../../build/features.yaml) — `branding` and `server` blocks.
