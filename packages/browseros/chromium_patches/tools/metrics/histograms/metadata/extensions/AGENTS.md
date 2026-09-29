# `tools/metrics/histograms/metadata/extensions/` — extension enum mirrors

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../tools/AGENTS.md`](../../../../../tools/AGENTS.md).

## What's here

The UMA metadata mirror for two Chromium extension enums that BrowserOS
extends: `HistogramValue` (one entry per extension API function) and
`ExtensionPermission3` (one entry per API permission id). Both mirrors exist
because `extensions/browser/extension_function_histogram_value.h` and
`extensions/common/mojom/api_permission_id.mojom` gained BrowserOS values.

Feature block: **`api`**.

## Contents

```
extensions/
└── enums.xml
    ├── HistogramValue            + <int value="1962".."1986" label="DELETED_BROWSER_OS_* / BROWSER_OS_*">
    └── ExtensionPermission3      + <int value="266" label="kBrowserOS"/>
```

Each block is followed by its own anchor:

```xml
<!-- LINT.ThenChange(//extensions/browser/extension_function_histogram_value.h:HistogramValue) -->
<!-- ... Called by update_extension_permission.py. -->
```

## Rules

**EXTM1 — Two enums, one file, two anchors.** Do not add a BrowserOS value to one
and forget the other; they are separate enums with separate update scripts
(`update_extension_histograms.py` and `update_extension_permissions.py`).

**EXTM2 — 1962–1986 are frozen and partly `DELETED_`.** Retired
`browserOs` API functions keep their numeric slot under a `DELETED_` prefix so
old telemetry remains readable. Never renumber, never reuse, never compact the
range. Adding a new function means **1987**, not "the first free-looking gap".

**EXTM3 — 266 is the current `kBrowserOS` permission id.** The next BrowserOS
permission is 267. Verify against a current Chromium before allocating.

**EXTM4 — The label must match the C++ enumerator name exactly.** The PRESUBMIT
script does a string comparison; `BROWSER_OS_GETPAGELOADSTATUS` in the header
and `BROWSER_OS_GETPAGELOADSTATUS` in the XML must be identical, including the
absence of a prefix.

**EXTM5 — Don't reformat.** This file is ~3300 lines. Keep BrowserOS hunks
surgical; a reformat guarantees conflict on every Chromium bump.

**EXTM6 — The C++ side lives under `chromium_patches/extensions/`, not here.**
This folder is data only. Any `.h`, `.cc`, or `.mojom` change belongs in
`chromium_patches/extensions/{browser,common}/`.

## Workflows

**Adding a `browserOs` extension API function**
1. Append `BROWSER_OS_<NAME> = 1987` in
   `chromium_patches/extensions/browser/extension_function_histogram_value.h`.
2. Add `<int value="1987" label="BROWSER_OS_<NAME>"/>` here in the same change.
3. Add the IDL in `chrome/common/extensions/api/browser_os.idl`.
4. Implement in `chrome/browser/extensions/api/browser_os/`.
5. Register the paths under `api` in `features.yaml`.

**Retiring a `browserOs` function**
1. Rename the enumerator to `DELETED_<NAME>`; keep the value.
2. Keep the XML entry unchanged.
3. Remove the IDL + handler. Do not reclaim the number.

**Adding an extension permission**
1. Append `kFoo = 267` in `chromium_patches/extensions/common/mojom/api_permission_id.mojom`.
2. Add `<int value="267" label="kFoo"/>` to `ExtensionPermission3` here.
3. Add the display name in `chrome/common/extensions/permissions/chrome_api_permissions.cc`.

**Debugging a PRESUBMIT `update_extension_histograms.py` diff**
- Run the script in a Chromium checkout; it rewrites `enums.xml` from the header.
  Whatever it changes is the truth. Re-extract rather than patching the diff by
  hand.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — metadata folder rules.
- [`../../../../extensions/browser/AGENTS.md`](../../../../../extensions/browser/AGENTS.md) — the `HistogramValue` enum.
- [`../../../../extensions/common/mojom/AGENTS.md`](../../../../../extensions/common/mojom/AGENTS.md) — the `APIPermissionID` enum.
- [`../../../../../build/features.yaml`](../../../../../../build/features.yaml) — the `api` block.
