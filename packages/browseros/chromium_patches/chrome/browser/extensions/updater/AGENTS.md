# `extensions/updater/` — force-install of pending extensions

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

One new method on Chromium's `ExtensionUpdater`: `InstallPendingNow(CheckParams)`.
`CheckNow()` only updates extensions that are *already installed*; the
BrowserOS bundled extensions can be registered as pending (via
`PendingExtensionManager`) without being on disk yet, and they need a way to be
pulled in immediately at startup. `InstallPendingNow()` is that path: it walks
`params.ids`, looks each one up in the pending manager, and either routes it
through the update service or hands it to the downloader.

## Contents

```
updater/
├── extension_updater.h   ← declares InstallPendingNow() with the doc comment
│                           explaining the pending-vs-installed distinction
└── extension_updater.cc  ← the ~95-line implementation
```

## Rules

**UPD1 — `InstallPendingNow()` is not `CheckNow()` with a different flag.**
The distinction is in the header: `CheckNow()` checks *installed* extensions,
`InstallPendingNow()` targets entries in the `PendingExtensionManager`. Do not
merge them; the `GetById()` lookup returning `nullptr` is the intended skip.

**UPD2 — Skips are silent by design.** Three conditions `continue` to the next
ID: not in the pending manager, `!Manifest::IsAutoUpdateableLocation(install_source)`,
or neither the update service nor the downloader accepted it (in which case
`ReportFailure(..., DOWNLOADER_ADD_FAILED)` is called). A new skip reason must
report a `FailureReason` too.

**UPD3 — The `CHECK(enabled_ / alive_ / pending_extension_manager_)`s are
post-conditions, not validation.** They run after `ScopedProfileKeepAlive` is
acquired and after the `params.ids.empty()` early return. Moving them above
those turns a legal no-op into a crash.

**UPD4 — The callback contract is "always invoked".** Every early return runs
`params.callback` before returning, and the happy path is completed by
`NotifyIfFinished(request_id)`. A new early exit that forgets this hangs the
caller.

**UPD5 — Only `InstallPendingNow()` is BrowserOS's; the rest of
`ExtensionUpdater` is upstream Chromium.** The `extension_updater.h` diff is a
7-line addition in one hunk. Keep the patch minimal — every extra hunk is a
conflict on the next Chromium bump.

**UPD6 — `InstallStageTracker` reporting is not optional.** Each accepted ID is
reported as `Stage::DOWNLOADING` (or as a failure). Removing it silently
degrades the extensions WebUI's install progress.

## Workflows

**Forcing a bundled extension to install at startup**
1. Register the extension as pending in the
   `PendingExtensionManager` (via the BrowserOS external provider — see
   `../external_provider_impl.cc`).
2. Call `ExtensionUpdater::Get(profile)->InstallPendingNow(params)` with the
   extension IDs in `params.ids`.
3. Set `params.install_immediately` and `params.fetch_priority`; both are
   forwarded to the update check.
4. Handle `params.callback` — it always runs.

**Extending the pending-install path**
1. Add the case in the `for (const ExtensionId& id : params.ids)` loop.
2. Keep the `awaiting_downloader` / `awaiting_update_service` bookkeeping
   correct, or `NotifyIfFinished()` never fires.
3. Re-extract the diff; the `features.yaml` block for
   `chrome/browser/extensions/updater/` must list both `.h` and `.cc`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/` overlay; where the BrowserOS
  provider is constructed.
- [`../../browseros/extensions/AGENTS.md`](../../browseros/extensions/AGENTS.md) —
  the loader that decides when an install is needed.
- [`../extension_management.cc`](../extension_management.cc) — the update URL
  these downloads use.
- [`../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.
