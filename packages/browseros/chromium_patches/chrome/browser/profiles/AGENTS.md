# `profiles/` — KeyedService construction

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

One file, two additions.
`chrome_browser_main_extra_parts_profiles.cc` is the list of every
`KeyedServiceFactory::GetInstance()` that Chromium constructs at browser
startup, and the patch adds
`browseros_metrics::BrowserOSMetricsServiceFactory::GetInstance();` to it,
next to `BitmapFetcherServiceFactory::GetInstance()` and
`BluetoothChooserContextFactory::GetInstance()`. Without that call the
PostHog `KeyedService` is never instantiated and every `CaptureEvent()` is a
silent no-op. The `#include` is added in Chromium's own (unsorted) include
block.

## Contents

```
profiles/
└── chrome_browser_main_extra_parts_profiles.cc   ← +#include
                                                    browseros/metrics/browseros_metrics_service_factory.h
                                                    + BrowserOSMetricsServiceFactory::GetInstance();
```

## Rules

**PROF1 — A `KeyedServiceFactory` that is not constructed here never exists.**
This file is the single construction point for browser-process keyed services.
Adding a factory is a one-line change here; forgetting it produces a service
that compiles, links, and silently does nothing.

**PROF2 — The placement is alphabetical-ish within a guarded block, and the
`TOOLKIT_VIEWS` `#if` matters.** The new call sits *before* the
`#if defined(TOOLKIT_VIEWS)` block, so it is constructed on every platform. Do
not move it inside that guard.

**PROF3 — Do not re-sort the include list.** The new
`chrome/browser/browseros/metrics/browseros_metrics_service_factory.h` is
inserted in Chromium's existing order (between `consent_auditor_factory.h` and
`content_index_provider_factory.h`), which is already not alphabetical. Sorting
the block would produce a large, conflict-prone diff.

**PROF4 — This is the only place the metrics service is wired.** The service
itself lives in `../browseros/metrics/`; the profile-scoped ids it maintains
come from prefs registered in `../prefs/browser_prefs.cc`. Neither of those
needs to know about this call site.

## Workflows

**Adding a new BrowserOS KeyedService**
1. Write the service and factory under `../browseros/metrics/` (or a sibling
   `browseros/` subdirectory) and add them to that directory's `BUILD.gn`.
2. Add the `#include` here in Chromium's existing order.
3. Add `YourFactory::GetInstance();` to the list, above the
   `#if defined(TOOLKIT_VIEWS)` block.
4. Extract the diff and add the factory path to the same `features.yaml` block.

**Confirming the metrics service is alive**
1. Look for a `BrowserOSMetrics` request in the network log pointing at the
   PostHog endpoint.
2. If there is none, check this file first (PROF1), then
   `../browseros/metrics/browseros_metrics_service_factory.cc` for a dependency
   that failed to resolve the `KeyedServiceFactory` context.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../browseros/metrics/AGENTS.md`](../browseros/metrics/AGENTS.md) — the
  service and factory constructed here.
- [`../prefs/AGENTS.md`](../prefs/AGENTS.md) — where the metrics prefs are
  registered.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
