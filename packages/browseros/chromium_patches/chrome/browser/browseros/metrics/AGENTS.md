# `browseros/metrics/` — PostHog analytics

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

A small analytics stack with two halves. `browseros_metrics.{h,cc}` is a
static facade — `BrowserOSMetrics::Log(event_name)` with optional sample rate
and properties — that any file can call without a `Profile*`. Behind it,
`browseros_metrics_service.{h,cc}` is a per-profile `KeyedService` that owns two
stable identifiers (a per-profile client id and a per-installation install id)
and POSTs events to the PostHog API. `browseros_metrics_service_factory.{h,cc}`
is the `KeyedServiceFactory`; it is instantiated from
`chrome/browser/profiles/chrome_browser_main_extra_parts_profiles.cc`.

## Contents

```
metrics/
├── BUILD.gn                       ← source_set "metrics", 8 explicit sources
├── browseros_metrics.{h,cc}       ← static Log(event, sample_rate) and
│                                   Log(event, base::DictValue, sample_rate)
├── browseros_metrics_service.{h,cc} ← KeyedService: CaptureEvent(), GetClientId(),
│                                     GetInstallId(), SendEventToPostHog(),
│                                     AddDefaultProperties(), Shutdown()
├── browseros_metrics_service_factory.{h,cc} ← KeyedServiceFactory
├── browseros_metrics_prefs.{h,cc} ← RegisterProfilePrefs() and
│                                     RegisterLocalStatePrefs()
└── (consumers elsewhere: ../../metrics/chrome_metrics_service_client.cc,
    ../server/browseros_server_updater.cc,
    ../extensions/browseros_extension_maintainer.cc,
    ../../extensions/api/browser_os/browser_os_api.cc,
    ../../ui/views/side_panel/clash_of_gpts/clash_of_gpts_coordinator.cc,
    ../../ui/views/side_panel/third_party_llm/third_party_llm_panel_coordinator.cc,
    ../../ui/webui/settings/browseros_metrics_handler.cc)
```

## Rules

**MET1 — Callers use the static facade, never the service.**
`BrowserOSMetrics::Log("alive", 0.01)` is the entire public surface for
instrumentation sites. Reaching for
`BrowserOSMetricsServiceFactory::GetForProfile(profile)` from a call site that
already has no `Profile*` is the reason the facade exists. The in-tree call
sites are `../../metrics/chrome_metrics_service_client.cc`
(`NotifyApplicationNotIdle`), `../server/browseros_server_updater.cc`,
`../extensions/browseros_extension_maintainer.cc`,
`../../extensions/api/browser_os/browser_os_api.cc`, the two side-panel
coordinators under `../../ui/views/side_panel/`, and
`../../ui/webui/settings/browseros_metrics_handler.cc`. Note that
`../server/browseros_server_manager.cc` itself does **not** log — the
server's events come from the updater.

**MET2 — Pass an explicit sample rate for high-frequency events.**
The `alive` heartbeat uses `0.01`. `Log()`'s default is `1.0` (log
everything), so a new call site in a hot path that omits the argument is
almost always a bug.

**MET3 — `browseros_metrics_prefs.h` has two registration functions, not one.**
`RegisterProfilePrefs(user_prefs::PrefRegistrySyncable*)` and
`RegisterLocalStatePrefs(PrefRegistrySimple*)`. They are wired from
`../../prefs/browser_prefs.cc` (`RegisterProfilePrefs` inside
`RegisterProfilePrefs`, `RegisterLocalStatePrefs` inside `RegisterLocalState`).
A new analytics pref must state which registry it belongs to.

**MET4 — Client id is per-profile, install id is per-installation.**
`InitializeClientId()` and `InitializeInstallId()` are separate and the
distinction is load-bearing for PostHog identity resolution. Do not collapse
them, and do not regenerate the install id on profile reset.

**MET5 — The factory must stay in the browser-main-parts list.**
`BrowserOSMetricsServiceFactory::GetInstance()` is constructed in
`chrome/browser/profiles/chrome_browser_main_extra_parts_profiles.cc` next to
the other `GetInstance()` calls. Removing it makes the `KeyedService` never
instantiate and every `CaptureEvent()` a silent no-op.

**MET6 — New sources go into `source_set("metrics")` explicitly.** The
`sources` list in `BUILD.gn` enumerates all eight files, and it is the only
target in that file — the `KeyedServiceFactory` is one of the eight, not a
separate target.

## Workflows

**Instrumenting a new event**
1. Call `browseros_metrics::BrowserOSMetrics::Log("name", sample_rate)` at the
   call site; add `base::DictValue` properties only if the event needs them.
2. Include `chrome/browser/browseros/metrics/browseros_metrics.h` — that
   header alone, not the service header.
3. Ensure the including target already deps on
   `//chrome/browser/browseros/metrics` in its `BUILD.gn`; the server
   `BUILD.gn` does, most others do not.

**Adding a persisted analytics pref**
1. Declare and register it in `browseros_metrics_prefs.{h,cc}`, choosing
   profile vs local state explicitly (MET3).
2. Re-extract the diff and add the path to the same `features.yaml` block as
   the rest of `chrome/browser/browseros/metrics/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros/` root overlay.
- [`../server/AGENTS.md`](../server/AGENTS.md) — logs server lifecycle events.
- [`../../metrics/AGENTS.md`](../../metrics/AGENTS.md) — the
  `ChromeMetricsServiceClient` hook that emits the `alive` heartbeat.
- [`../../profiles/AGENTS.md`](../../profiles/AGENTS.md) — where the
  `KeyedServiceFactory` is constructed.
- [`../../prefs/AGENTS.md`](../../prefs/AGENTS.md) — pref registration wiring.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.
