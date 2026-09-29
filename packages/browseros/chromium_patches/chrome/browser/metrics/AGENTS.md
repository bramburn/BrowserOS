# `metrics/` — ChromeMetricsServiceClient hook

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

A single-line diff. `chrome_metrics_service_client.cc` adds
`browseros_metrics::BrowserOSMetrics::Log("alive", 0.01);` at the top of
`ChromeMetricsServiceClient::NotifyApplicationNotIdle()`, and the matching
`#include "chrome/browser/browseros/metrics/browseros_metrics.h"`. This is
what produces the BrowserOS "alive" heartbeat that the PostHog service in
`../browseros/metrics/` forwards.

## Contents

```
metrics/
└── chrome_metrics_service_client.cc   ← NotifyApplicationNotIdle():
                                          Log("alive", 0.01) then
                                          metrics_service_->OnApplicationNotIdle()
```

## Rules

**METC1 — The call must precede `OnApplicationNotIdle()`.**
`OnApplicationNotIdle()` resets Chromium's own idle timer. The BrowserOS
heartbeat is emitted first so a user session that never goes idle still
produces samples.

**METC2 — The sample rate is `0.01`, and it is not a placeholder.**
One in a hundred sessions is reported. Changing it changes the backend volume
and the meaning of every dashboard built on it; that is a product decision,
not a code tidy.

**METC3 — Call the static facade, not the service.**
`BrowserOSMetrics::Log()` resolves the per-profile service itself. Reaching
into `BrowserOSMetricsServiceFactory` from here would add a
`//chrome/browser/profiles` dependency to the metrics target for no benefit.

**METC4 — Do not add further call sites to this file without checking the
target's deps.** The `chrome_metrics_service_client` GN target must list
`//chrome/browser/browseros/metrics`; it does today. A new consumer elsewhere
needs its own dep entry, not a reliance on transitive visibility.

## Workflows

**Changing the heartbeat cadence or name**
1. Edit the single `Log()` call in `NotifyApplicationNotIdle()`.
2. Keep the call before `OnApplicationNotIdle()`.
3. Re-extract; the patch should stay one line plus one include.

**Adding a second metrics hook in the browser process**
1. Prefer the static facade from wherever the signal already exists, rather
   than adding another branch here.
2. Add the `//chrome/browser/browseros/metrics` dep to that target's
   `BUILD.gn`.
3. Give the event a stable, dotted name (`"alive"` is the only current one)
   and an explicit sample rate.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../browseros/metrics/AGENTS.md`](../browseros/metrics/AGENTS.md) — the
  `BrowserOSMetrics` facade and the PostHog service behind it.
- [`../../../tools/metrics/AGENTS.md`](../../../tools/metrics/AGENTS.md) —
  UMA metadata for the Chromium-side histograms.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
