# `entrypoints/app/scheduled-tasks/` — scheduled tasks (`/scheduled`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

The user-facing manager for cron-like jobs: list, create, edit, enable/disable,
run-now, delete, and inspect past runs. Two tabs (tasks / results) with per-row
result dialogs.

The state lives in extension storage and is mirrored to the local server; the alarms
that actually fire are created in the **service worker**
(`entrypoints/background/scheduledJobRuns.ts`), not here. This folder is UI plus
read/write through `useScheduledJobs` / `useScheduledJobRuns`.

## Contents

```
scheduled-tasks/
├── ScheduledTasksPage.tsx  ← tabs, dialogs, ?prefill deep-link handling
├── ScheduledTasksHeader.tsx
├── ScheduledTasksList.tsx
├── ScheduledTaskCard.tsx   ← one job row: enable toggle, run, edit, delete
├── NewScheduledTaskDialog.tsx ← create/edit form (largest file here)
├── ScheduledTaskResults.tsx← per-run history + retry/cancel
├── types.ts                ← re-exports ScheduledJob/ScheduledJobRun + a storage interface
├── and the newtab mirror: ../../../newtab/index/ScheduleResults.tsx
```

## Rules

- **ST1 — Types are re-exported from `types.ts`,** and the canonical definitions live
  in `@/lib/schedules/scheduleTypes`. Do not redefine `ScheduledJob` here; extend the
  lib type and re-export.
- **ST2 — Alarms are owned by the service worker.** This folder edits job records;
  `background/scheduledJobRuns.ts` reacts to them (`createAlarmFromJob`, sync-on-start).
  A change here that assumes an alarm is already set is a bug.
- **ST3 — `?prefill` deep links are consumed once.** The page uses a
  `prefillHandled` ref so the dialog does not reopen on re-render. Preserve that guard.
- **ST4 — Destructive and irreversible actions are confirmed:** job deletion uses
  `AlertDialog`; run cancellation and retry are explicit buttons with analytics.
- **ST5 — Track every mutation.** `NEW_SCHEDULED_TASK_CREATED`, `…_EDITED`,
  `…_TOGGLED`, `…_DELETED`, `…_TESTED`, `…_RETRIED`, `…_CANCELLED`,
  `…_VIEW_RESULTS` already exist in `@/lib/constants/analyticsEvents`. New actions
  need a new constant, not a copy of an existing event.
- **ST6 — Run history is window-bounded.** The worker keeps at most
  `MAX_RUNS_PER_JOB = 15` and fails runs older than 10 minutes as timed out. The UI
  should not assume more.

## Workflows

**Adding a job field**
1. Extend `ScheduledJob` in `@/lib/schedules/scheduleTypes.ts`.
2. Handle it in `NewScheduledTaskDialog.tsx` (form) and `ScheduledTaskCard.tsx`
   (display).
3. Check the worker still serialises it if the alarm or run payload needs it.
4. Existing stored jobs lack the field — read defensively and default.

**Adding a bulk action**
1. Add the handler in `ScheduledTasksPage.tsx`.
2. Iterate with the hooks from `useScheduledJobs`; do not write storage directly.
3. Add a tracking event constant.
4. Confirm the worker picks the change up on the next sync.

**Debugging a job that never fires**
1. Check the job record in extension storage (`scheduledJobStorage`).
2. Check `chrome.alarms` for `scheduled-job-<id>`.
3. Check the worker's `syncAlarmState()` — it recreates alarms for enabled jobs on
   service-worker start.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`../../../background/AGENTS.md`](../../background/AGENTS.md) — alarms and run execution.
- [`../../../../lib/schedules/`](../../../lib/schedules/) — storage, types, `createAlarmFromJob`, server call.
- [`../../../newtab/index/AGENTS.md`](../../newtab/index/AGENTS.md) — the new-tab mirror of run results.
- [`../../../../components/ai-elements/run-result-dialog.tsx`](../../../components/ai-elements/run-result-dialog.tsx) — shared result dialog.
