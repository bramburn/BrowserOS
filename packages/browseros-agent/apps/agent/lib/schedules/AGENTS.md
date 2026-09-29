# `lib/schedules/` — Scheduled tasks (cron-like jobs)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The scheduled-task engine: user-defined recurring prompts that run
without the panel open. `scheduleTypes.ts` defines the persisted job and
run shapes, `createAlarmFromJob.ts` turns a job into a `chrome.alarms`
entry, `scheduleStorage.ts` owns the three storage items and the CRUD
hook (which re-arms alarms on every mutation), `getChatServerResponse.ts`
performs the actual run and parses the streamed result,
`refine-prompt.ts` asks the server to turn a rough prompt into a better
one, and `syncSchedulesToBackend.ts` mirrors the list to the cloud so it
follows the user across devices.

## Contents

```
schedules/
├── scheduleTypes.ts               ← ScheduledJob { id, name, query,
│                                    scheduleType: 'daily'|'hourly'|'minutes',
│                                    scheduleTime?, scheduleInterval?, enabled,
│                                    providerId?, createdAt, updatedAt,
│                                    lastRunAt? }, ScheduledJobRun,
│                                    ToolCallExecution
├── createAlarmFromJob.ts          ← daily → when + periodInMinutes 24 h;
│                                    hourly → interval × 60 min; minutes →
│                                    raw minutes; names alarms
│                                    `scheduled-job-<jobId>`
├── scheduleStorage.ts             ← scheduledJobStorage (local:scheduledJobs),
│                                    scheduledJobRunStorage,
│                                    pendingDeletionStorage,
│                                    useScheduledJobs() → addJob/updateJob/
│                                    deleteJob/runJob/toggleJob
├── scheduleSystemPrompt.ts        ← the one-line system prompt injected for
│                                    a scheduled run
├── getChatServerResponse.ts       ← runs the job against the agent server
│                                    (eventsource-parser), assembles provider +
│                                    MCP + personalization context, returns
│                                    text / finalResult / executionLog / toolCalls
├── refine-prompt.ts               ← POST refine endpoint; resolves the
│                                    provider to use (explicit id → default →
│                                    first → createDefaultBrowserOSProvider)
├── syncSchedulesToBackend.ts      ← two-way-ish sync: creates/updates/deletes
│                                    remote rows, ignores id/createdAt/lastRunAt
│                                    when diffing, honours pendingDeletionStorage
└── graphql/syncSchedulesDocument.ts
                                    ← Get / Create / Update / Delete
                                      ScheduledJob documents
```

## Rules

- **SCD1 — Every enabled job must have a matching `chrome.alarms` entry named
  `scheduled-job-<jobId>`.** Mutations in `scheduleStorage.ts` call
  `createAlarmFromJob`; adding a mutation path means re-arming or clearing
  the alarm in the same function.
- **SCD2 — `scheduleType` is a three-value union.** `daily` uses
  `scheduleTime` ("HH:MM"); `hourly` and `minutes` use
  `scheduleInterval`. Don't add a fourth kind without updating
  `createAlarmFromJob.ts` and the UI.
- **SCD3 — Deletions are two-phase.** Marking an id in
  `pendingDeletionStorage` then reconciling with the backend avoids deleting
  a remote row the user has not seen disappear; don't shortcut to a direct
  `deleteScheduledJob`.
- **SCD4 — The system prompt is fixed.** `scheduleSystemPrompt.ts` tells the
  agent the schedule is externally managed. Changing that text changes
  behaviour for every historical run.
- **SCD5 — Use `scheduleTypes.ts` for job/run shapes; don't redeclare them
  in the messaging protocol** — `../messaging/schedules/scheduleMessages.ts`
  references `jobId`/`runId` against these types.
- **SCD6 — Backend sync is diff-based.** `syncSchedulesToBackend.ts` ignores
  `id`, `createdAt` and `lastRunAt` when comparing, so local-only metadata
  never triggers a remote write.

## Workflows

**Adding a new schedule kind**
1. Extend `ScheduledJob['scheduleType']` in `scheduleTypes.ts`.
2. Add the `chrome.alarms` mapping in `createAlarmFromJob.ts`.
3. Add the picker/labels in `entrypoints/app/scheduled-tasks/`.
4. Check `syncSchedulesToBackend.ts` maps the new field to the remote column.

**Running a job manually**
1. `sendScheduleMessage.runScheduledJob({ jobId })` from
   `../messaging/schedules/`.
2. The background handler calls `getChatServerResponse`, writes a
   `ScheduledJobRun` into `scheduledJobRunStorage`, and stamps `lastRunAt`.

**Adding a field the backend should store**
1. Update `schema/schema.graphql`, run `bun run codegen`.
2. Extend the document in `graphql/syncSchedulesDocument.ts`.
3. Map it in `syncSchedulesToBackend.ts` and the local type.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`../messaging/schedules/AGENTS.md`](../messaging/schedules/AGENTS.md) — the run/cancel channel.
- [`../llm-providers/AGENTS.md`](../llm-providers/AGENTS.md) — `providerId` resolution and the default provider.
- [`../graphql/AGENTS.md`](../graphql/AGENTS.md) — `execute()` used by the sync.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the background alarm handler and the scheduled-tasks route.
