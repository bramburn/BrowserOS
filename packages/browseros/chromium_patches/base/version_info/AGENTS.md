# `base/version_info/` — BrowserOS version string generation

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Three files that make Chromium's build emit a *second* version macro,
`BROWSEROS_VERSION`, alongside Chrome's own `PRODUCT_VERSION`. This is the
mechanism the About page, the crash reporter, and the OTA updater use to
distinguish "which Chromium is this built on" from "which BrowserOS release is
this".

All three are in the **`branding`** feature block of `features.yaml`.

## Contents

```
version_info/
├── BUILD.gn                      ← + "//chrome/BROWSEROS_VERSION" in process_version("generate_version_info") sources
├── version_info.h                ← + GetBrowserOSVersionNumber() constexpr accessor
└── version_info_values.h.version ← + #define BROWSEROS_VERSION "@BROWSEROS_*.@"
```

The generator substitutes `@BROWSEROS_MAJOR@ / @MINOR@ / @BUILD@ / @PATCH@`
from `packages/browseros/chromium_patches/chrome/BROWSEROS_VERSION` and
writes the result to `out/Default/gen/base/version_info/version_info_values.h`.

## Rules

**VI1 — Three files, one change.** Touching the `.version` template without the
matching `sources` entry in `BUILD.gn` compiles cleanly and then fails on the
first `#include "base/version_info/version_info.h"` that references
`BROWSEROS_VERSION`. All three move together.

**VI2 — The accessor is `constexpr` and returns `std::string_view`.** Match
the existing shape of `GetVersionNumber()`. Do not introduce a `std::string`
copy — it lands in every caller's hot path and adds a heap allocation to a
header included by most of the browser.

**VI3 — `BROWSEROS_VERSION` is four dotted components.** The template emits
`@BROWSEROS_MAJOR@.@BROWSEROS_MINOR@.@BROWSEROS_BUILD@.@BROWSEROS_PATCH@`.
Don't collapse to three — `chrome/browser/browseros/server/` compares against a
four-component string.

**VI4 — Do not reuse `//chrome/VERSION` for the BrowserOS number.** The two
version files are deliberately independent; pointing `BROWSEROS_VERSION` at
`//chrome/VERSION` makes the OTA version check a no-op.

**VI5 — Renumbering is a product decision, not a patch edit.** Changing a
component in `chrome/BROWSEROS_VERSION` alters the shipped binary's version
and interacts with the updater's monotonicity checks.

## Workflows

**Adding a fourth derived value (e.g. a build channel string)**
1. Add the `#define` to `version_info_values.h.version`.
2. Add the corresponding source to `sources` in `version_info/BUILD.gn`.
3. Add a `constexpr std::string_view` accessor in `version_info.h`.
4. Keep all `base/version_info/*` entries under the `branding` feature.

**Bumping the BrowserOS version**
1. Edit `packages/browseros/chromium_patches/chrome/BROWSEROS_VERSION` only.
2. Nothing in this folder changes — the template interpolates it.
3. Confirm by checking `out/Default/gen/base/version_info/version_info_values.h`
   after a build.

**Debugging a `BROWSEROS_VERSION` undefined error**
1. Check `chrome/BROWSEROS_VERSION` exists in the Chromium source tree (`branding`
   lists it in `features.yaml`, but the patch file is currently absent from the
   overlay).
2. Check `BUILD.gn`'s `sources` list still names it.
3. Check the generated header under `out/Default/gen/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `base/` subtree rules.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `branding` block.
- [`../../../CHROMIUM_VERSION`](../../../CHROMIUM_VERSION) — the Chromium-side version (distinct from BrowserOS).
