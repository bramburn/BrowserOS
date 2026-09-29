# `extensions/` — service-worker keepalives and permission ids

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md).

## What's here

Five patches in Chromium's `extensions/` layer, split across the browser and
common halves:

- **`browser/`** — BrowserOS's bundled extensions get a *permanent* service
  worker keepalive so their workers are never killed by inactivity (the agent
  extension is a long-lived MCP bridge; a killed worker means a dead tool), plus
  the histogram values for the `browserOs` extension API.
- **`common/mojom/`** — a new `APIPermissionID` (`kBrowserOS = 266`) so the
  `browserOs` API is gated like any other extension permission.

Features: `misc` (`process_manager.*`), `api` (`BUILD.gn`,
`extension_function_histogram_value.h`, `api_permission_id.mojom`).

## Contents

```
extensions/
├── browser/
│   ├── BUILD.gn                             ← + //chrome/browser/browseros/core dep
│   ├── extension_function_histogram_value.h ← +BROWSER_OS_* = 1962..1986
│   ├── process_manager.h                    ← + browseros_permanent_keepalives_ map
│   └── process_manager.cc                   ← keepalive acquire/release
└── common/
    └── mojom/
        └── api_permission_id.mojom          ← + kBrowserOS = 266
```

## Rules

**EX1 — The keepalive is a leak by design.** `IncrementServiceWorkerKeepaliveCount`
with `kDoesNotTimeout` is never balanced for a BrowserOS extension until its
worker stops. That is the feature: the MCP server bridge must stay warm. Do not
"fix" it by adding a timeout.

**EX2 — Acquire and release must stay paired.** The `+` block in
`StartTrackingServiceWorkerRunningInstance` and the `-` block in
`StopTrackingServiceWorkerRunningInstance` use the same
`Activity::PROCESS_MANAGER` and the same `"browseros_permanent_keepalive"`
reason string. Changing one without the other leaks a keepalive UUID.

**EX3 — `IsBrowserOSExtension` is the gate; do not widen it.** The predicate
lives in `chrome/browser/browseros/core/browseros_constants.h` and
`extensions/browser/process_manager.cc` includes it directly. Adding a
`chrome/` include to a `//extensions` target creates a layering inversion —
it is allowed only for this one symbol, and it is why
`extensions/browser/BUILD.gn` was patched.

**EX4 — `kBrowserOS = 266` is append-only.** `APIPermissionID` is a mojom enum
mirrored in
`tools/metrics/histograms/metadata/extensions/enums.xml`. Renumbering breaks
stored permission grants.

**EX5 — Histogram values 1962–1986 are frozen.** Many are prefixed
`DELETED_BROWSER_OS_` — retired API functions that must keep their slot so old
telemetry stays readable. Never renumber and never compact the range.

**EX6 — `//chrome/browser/browseros/core` in `extensions/browser/BUILD.gn` is a
deliberate layering exception.** Chromium keeps `extensions/` free of `chrome/`
dependencies. This one is required; removing it breaks `process_manager.cc`.

## Workflows

**Retiring a `browserOs` extension API function**
1. Keep the enum entry; rename it to `DELETED_<NAME>` rather than removing it
   (see `DELETED_BROWSER_OS_CLICK` for the established pattern).
2. Remove the IDL from `chrome/common/extensions/api/browser_os.idl`.
3. Remove the handler from `chrome/browser/extensions/api/browser_os/`.
4. Leave the `HistogramValue` and the `enums.xml` line untouched.
5. Note the IDL removal under the `api` feature — it needs a deprecation story,
   not a silent delete.

**Adding a new BrowserOS extension**
1. Add its id to `IsBrowserOSExtension` in
   `chrome/browser/browseros/core/browseros_constants.h`.
2. It automatically gets a permanent keepalive — verify that is what you want.
3. Confirm `extensions/browser/BUILD.gn` still has the `browseros/core` dep.

**Debugging "the agent extension's service worker keeps dying"**
1. Check `IsBrowserOSExtension` returns `true` for the installed extension id.
2. Check `browseros_permanent_keepalives_` is populated after
   `StartTrackingServiceWorkerRunningInstance`.
3. Check the acquire/release UUIDs match — an unbalanced release would be a
   `process_manager.cc` bug.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../tools/metrics/histograms/metadata/extensions/AGENTS.md`](../tools/metrics/histograms/metadata/extensions/AGENTS.md) — the enum mirrors.
- [`../../build/features.yaml`](../../build/features.yaml) — `api` and `misc` blocks.
