# `lib/messaging/schedules/` — Scheduled-job message protocol

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib/messaging`.

## What's here

One file, `scheduleMessages.ts`: the typed channel the background service
worker and the scheduler UI use to start and cancel a scheduled job run.
It is built on `defineExtensionMessaging<ScheduleMessagesProtocol>()` and
re-exported under the `sendScheduleMessage` / `onScheduleMessage` aliases.

## Contents

```
schedules/
└── scheduleMessages.ts
    ├── RunScheduledJobData      { jobId: string }
    ├── CancelScheduledJobRunData{ runId: string }
    ├── RunScheduledJobResponse  { success: boolean, error?: string }
    └── aliases: sendScheduleMessage, onScheduleMessage
```

## Rules

- **SCHEDMSG1 — The protocol map is the contract.** `runScheduledJob(data)`
  and `cancelScheduledJobRun(data)` are the only two methods; adding one
  means editing the type map, not a cast at the call site.
- **SCHEDMSG2 — Always return `RunScheduledJobResponse`.** A rejected run is
  `{ success: false, error }`, not a thrown error, so the caller can surface
  a message without a try/catch.
- **SCHEDMSG3 — Send/import the aliases, not `sendMessage`/`onMessage`.** The
  alias is what identifies the channel at the call site.
- **SCHEDMSG4 — The handler side belongs to the background entrypoint.** This
  folder declares the protocol; `entrypoints/background/` registers
  `onScheduleMessage` and dispatches into `../schedules/`.
- **SCHEDMSG5 — `jobId` and `runId` are different identifiers.** `jobId`
  targets the schedule, `runId` targets one already-started execution. Don't
  interchange them.

## Workflows

**Running a job now (user-initiated)**
1. `await sendScheduleMessage.runScheduledJob({ jobId })`
2. Branch on `result.success`; show `result.error` when false.

**Cancelling an in-flight run**
1. `await sendScheduleMessage.cancelScheduledJobRun({ runId })`
2. The background handler aborts the request and records the run status.

**Adding a third scheduled-job message**
1. Add its request/response interfaces here.
2. Extend `ScheduleMessagesProtocol`.
3. Implement the handler in `entrypoints/background/` using
   `onScheduleMessage.<method>`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — messaging rules (MSG1–MSG5).
- [`../../schedules/AGENTS.md`](../../schedules/AGENTS.md) — `ScheduledJob` / `ScheduledJobRun` and `chrome.alarms`.
- [`../../entrypoints/AGENTS.md`](../../../entrypoints/AGENTS.md) — the background handlers.
