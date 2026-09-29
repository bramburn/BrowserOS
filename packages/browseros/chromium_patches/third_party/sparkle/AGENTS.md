# `third_party/sparkle/` — macOS Sparkle framework wiring

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A `new file` BUILD.gn that BrowserOS adds to Chromium's `third_party/sparkle/`
directory. It wires the Sparkle.framework macOS auto-update framework into the
GN build: a bundle_data target to copy the framework into the app bundle, two
link configs (one for test binaries, one for the `chrome_framework` shared
library), and a headers-only group.

The framework binary itself is **not** in this repo — it is downloaded during
the build by `build/common/sparkle.py` / the `mac-sparkle-updater` feature.

Feature block: **`sparkle-third-party`**.

## Contents

```
sparkle/
└── BUILD.gn   ← new file, 55 lines
```

Targets it defines:

| Target | Kind | Purpose |
|---|---|---|
| `sparkle_framework_bundle` | `bundle_data` | copies `Sparkle.framework` into `{{bundle_contents_dir}}/Frameworks` |
| `sparkle_link_test` | `config` | `-rpath @executable_path/../Frameworks` + `-F` + `-framework Sparkle` |
| `sparkle_link_framework` | `config` | `-rpath @loader_path/Frameworks` for the shared library |
| `sparkle_headers` | `group` | headers-only, no linking |

## Rules

**SPK1 — `assert(is_mac)` at the top is load-bearing.** Sparkle is macOS-only.
Without the assert a Linux/Windows GN parse fails with a confusing error instead
of a clean one.

**SPK2 — Do not commit the framework binary.** It arrives via the build's
download step. Committing it would bloat the repo and bypass the update path.

**SPK3 — The two configs are not interchangeable.** `sparkle_link_test` uses
`@executable_path` (test binaries resolve relative to themselves);
`sparkle_link_framework` uses `@loader_path` (the dylib resolves relative to
its own load path). Copying an rpath between them breaks one of the two.

**SPK4 — `-F` paths come from `rebase_path(".", root_build_dir)`.** Don't
hardcode absolute paths or assume a checkout location.

**SPK5 — macOS only; zero coverage on the Windows dev host.** Changes here can
only be validated on a macOS build host. Treat a Sparkle change as untested
until built on mac.

**SPK6 — This is the only file in `third_party/sparkle/`.** Chromium has no
`third_party/sparkle` upstream, so this whole directory is BrowserOS-authored.
There is no upstream counterpart to drift against — you own it outright.

**SPK7 — The consuming code is under `chrome/`, not here.**
`chrome/browser/mac/sparkle_glue.{h,mm}`,
`chrome/browser/ui/webui/help/sparkle_version_updater_mac.{h,mm}`, and
`chrome/browser/BUILD.gn` reference these configs. Update both sides together.

## Workflows

**Building the macOS app and Sparkle.framework is missing**
1. Check the download step ran — `build/common/sparkle.py` fetches the
   framework into `<chromium_src>/third_party/sparkle/`.
2. Confirm the referencing targets are gated by `enable_sparkle` from
   `chrome/browser/sparkle_buildflags.gni`. `features.yaml` lists that file under
   `mac-sparkle-updater`, but it is not present in the overlay, so check the
   checkout rather than this tree.
3. Confirm `chrome/browser/mac/chrome_browser_main_extra_parts_mac.mm` adds the
   extra parts only when the buildflag is on.

**Adding a new consumer of Sparkle**
1. Reference `:sparkle_headers` for a headers-only consumer, or
   `:sparkle_link_framework` / `:sparkle_link_test` for a linking consumer.
2. Add the target under `chrome/browser/mac/sparkle_buildflags.gni` guards.
3. Add the path to the `mac-sparkle-updater` feature in `features.yaml`.

**Diagnosing a dyld failure at runtime**
- `@rpath` errors in the built app usually mean the `bundle_data` target did not
  run, so `BrowserOS.app/Contents/Frameworks/Sparkle.framework` is absent.
  Check the bundle step, not the linker flags.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `third_party/` subtree rules.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — `sparkle-third-party` and `mac-sparkle-updater` blocks.
- [`../../../build/common/sparkle.py`](../../../build/common/sparkle.py) — framework download.
