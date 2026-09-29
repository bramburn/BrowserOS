# `entrypoints/background/` — MV3 service worker

> Part of the WXT entry points in `apps/agent/entrypoints`. Parent guide:
> [`../../AGENTS.md`](../../AGENTS.md).

## What's here

The extension's background service worker — the only context that is always alive.
`index.ts` is a WXT `defineBackground` entry that wires up capabilities, storage
sync, message handlers, and the browser-action click. It is the **only** file in
`entrypoints/` that uses the `<name>/index.ts` background convention, and it calls
`defineBackground(() => {…})` with **no second argument** — there is no
`type: 'module'` (or `type: 'classic'`) setting here, so `background.type` in the
generated manifest is whatever WXT defaults to. Do not document a script type this
repo does not set. `scheduledJobRuns.ts` holds the alarm-driven scheduler, which
is the only part with real logic.

The worker is the relay between UI contexts and the local server: it resolves the
live server URL, proxies health checks and MCP tool fetches, and owns the
`chrome.alarms` lifecycle for scheduled tasks.

## Contents

```
background/
├── index.ts             ← defineBackground(): listeners, sync setup, message router
└── scheduledJobRuns.ts  ← alarm handler, run bookkeeping, staleness cleanup, queue
```

## Rules

- **BG1 — Side effects are registered once, at the top of `defineBackground`.** All
  listeners (`chrome.action.onClicked`, `runtime.onInstalled`, `runtime.onMessage`,
  `tabs.onRemoved`, `sessionStorage.watch`, `onServerMessage`) are added there. Do not
  add a listener from a component or from a lazy import.
- **BG2 — New worker-side requests need a message type on both ends.** Add the
  `onServerMessage('yourType', handler)` in `index.ts` and call it with
  `sendServerMessage('yourType', payload)` from the UI. There is no shared typed
  channel in this codebase — mismatched strings fail silently.
- **BG3 — Server URLs are resolved, never cached here.** `getMcpServerUrl()` /
  `getHealthCheckUrl()` read the live port pref; cache the *result* only for the
  duration of one call.
- **BG4 — Scheduled runs are bounded and self-healing.** `MAX_RUNS_PER_JOB = 15` and
  `STALE_TIMEOUT_MS = 10 min`: a run still marked `running` after 10 minutes is
  rewritten as failed with "Job timed out!". Do not remove either constant without a
  replacement.
- **BG5 — Alarms are reconciled on start.** `syncAlarmState()` recreates
  `scheduled-job-<id>` alarms for every enabled job, so a service-worker restart does
  not lose the schedule. Keep that call.
- **BG6 — Storage migrations go through the `onInstalled` UPDATE branch**, like
  `cleanupLegacyToolApprovalStorage()`. Use `@wxt-dev/storage`'s
  `removeItems` for the prefixed keys.
- **BG7 — Abort in-flight work on purpose.** `runAbortControllers` is module-scope
  precisely so the worker can cancel a run; keep the map, do not scope it inside a
  function.
- **BG8 — `chrome.sidePanel.setOptions({ enabled: false })` runs first.** The panel is
  opened programmatically from the action click; keep this line at the top.

## Workflows

**Adding a message the UI can call**
1. Add the handler in `index.ts` with `onServerMessage('name', async (payload) => …)`.
2. Return a serialisable result object (or `{ error: string }`); never throw across
   the boundary.
3. Call it with `sendServerMessage('name', payload)` from the UI.
4. Add a tracking event if the action is user-visible.

**Adding a storage sync**
1. Implement `setup<Thing>SyncToBackend` in the matching `lib/<feature>/storage.ts`.
2. Call it from `defineBackground` alongside the existing `setupLlmProviders*` and
   `setupScheduledJobsSyncToBackend` calls.
3. Re-sync on session change in the `sessionStorage.watch` block.

**Adding a new alarm-driven job type**
1. Model the alarm in `scheduledJobRuns.ts` (create/reconcile/dispatch).
2. Keep the constants (`MAX_RUNS_PER_JOB`, `STALE_TIMEOUT_MS`) shared with the new type.
3. Add the run record shape to `ScheduledJobRun` in `@/lib/schedules/scheduleTypes`.

**Verifying the worker**
1. Reload the extension; open `chrome://extensions` → the service worker link.
2. Errors surface in that console first, before any UI symptom.
3. `bun scripts/dev/inspect-ui.ts eval sidepanel "…"` for the UI side.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals, rules X1–X10.
- [`../../wxt.config.ts`](../../wxt.config.ts) — permissions (`alarms`, `sidePanel`, `tabs`, `storage`, …).
- [`../app/mcp-settings/AGENTS.md`](../app/mcp-settings/AGENTS.md) — the main consumer of `checkHealth` / `fetchMcpTools`.
- [`../app/scheduled-tasks/AGENTS.md`](../app/scheduled-tasks/AGENTS.md) — the UI for the alarms this folder fires.
- [`../../lib/messaging/server/serverMessages.ts`](../../lib/messaging/server/serverMessages.ts) — the message channel.
- [`../../lib/schedules/`](../../lib/schedules/) — job storage, types, server call.
