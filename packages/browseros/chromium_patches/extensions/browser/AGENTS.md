# `extensions/browser/` — permanent service-worker keepalives

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The browser-side half of the extensions layer. Four patches: a GN dependency on
BrowserOS core, a new histogram enum block for the `browserOs` API, and the
keepalive bookkeeping that keeps BrowserOS's own extension service workers
alive forever.

Feature blocks: `api` for `BUILD.gn` and `extension_function_histogram_value.h`;
`misc` for `process_manager.{h,cc}`.

## Contents

```
browser/
├── BUILD.gn                             ← + "//chrome/browser/browseros/core" in browser_sources deps
├── extension_function_histogram_value.h ← +26 lines: DELETED_BROWSER_OS_* and BROWSER_OS_* = 1962..1986
├── process_manager.h                    ← + std::map<WorkerId, base::Uuid> browseros_permanent_keepalives_
└── process_manager.cc                   ← acquire in StartTracking..., release in StopTracking...
```

## Rules

**EXB1 — The keepalive is intentionally unbounded.** The acquire call passes
`ServiceWorkerExternalRequestTimeoutType::kDoesNotTimeout`, so a BrowserOS
extension's service worker never expires. This is the feature (the MCP bridge
must stay warm); it is not a leak to fix.

**EXB2 — Acquire and release are a matched pair.** Both use
`Activity::PROCESS_MANAGER` and the reason string
`"browseros_permanent_keepalive"`. They must use the *same* string; the reason
string keys a `base::expected`/CHECK in `DecrementServiceWorkerKeepaliveCount`.

**EXB3 — Release must happen in `StopTrackingServiceWorkerRunningInstance`
before the early `return` path is reached.** The new block sits after the
`worker_id.extension_id` lookup guard. Moving it above the guard decrements a
keepalive for a worker that was never registered → CHECK failure.

**EXB4 — `//chrome/browser/browseros/core` in `BUILD.gn` is a deliberate
layering exception.** `//extensions` must not depend on `//chrome`; this is the
single sanctioned case, needed for `browseros::IsBrowserOSExtension`. Removing
it breaks `process_manager.cc`.

**EXB5 — Histogram values 1962–1986 are frozen.** Retired functions keep their
slot as `DELETED_BROWSER_OS_*`. Never renumber, never reuse, never compact.

**EXB6 — `browseros_permanent_keepalives_` must be the second-to-last member.**
The diff places it immediately before `weak_ptr_factory_`; `base::` requires the
`WeakPtrFactory` to be last. Do not move it after.

**EXB7 — `process_manager.{h,cc}` is Chromium's file, not a BrowserOS one.**
Any BrowserOS-specific code added here should be a small, obviously-marked
block. Larger logic belongs in `chrome/browser/browseros/extensions/`.

## Workflows

**Adding another always-on extension**
1. Add the extension id to `IsBrowserOSExtension` in
   `chrome/browser/browseros/core/browseros_constants.h`.
2. No change needed here — the predicate drives the keepalive automatically.
3. Confirm the extension is installed by
   `chrome/browser/browseros/extensions/browseros_extension_loader.cc`.

**Diagnosing a service worker that dies anyway**
1. `chrome://serviceworker-internals` — is the worker for a BrowserOS extension
   id that `IsBrowserOSExtension` recognises?
2. Check `browseros_permanent_keepalives_` is populated (VLOG(1) line
   `"browseros: Added permanent keepalive for extension ..."`).
3. If it was added but still dies, another code path called
   `DecrementServiceWorkerKeepaliveCount` — look for a mismatched reason string.

**Adding a `browserOs` API function**
1. Add the IDL in `chrome/common/extensions/api/browser_os.idl`.
2. Implement in `chrome/browser/extensions/api/browser_os/`.
3. Append the next `HistogramValue` (1987+) here and mirror it in
   `chromium_patches/tools/metrics/histograms/metadata/extensions/enums.xml`.
4. Add the path under the `api` feature.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/` subtree rules.
- [`../../tools/metrics/histograms/metadata/extensions/AGENTS.md`](../../tools/metrics/histograms/metadata/extensions/AGENTS.md) — the enum mirrors.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `api` and `misc` blocks.
