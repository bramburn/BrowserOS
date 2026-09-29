# `lib/schedules/graphql/` — Scheduled-job GraphQL documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib/schedules`.

## What's here

One file, `syncSchedulesDocument.ts`, holding the four codegen'd
operations the scheduler uses to mirror jobs to the BrowserOS cloud.
The consumer is `../syncSchedulesToBackend.ts`.

## Contents

```
graphql/
└── syncSchedulesDocument.ts
    ├── GetScheduledJobsByProfileIdDocument   ← query, first: 100, selects
    │                                            rowId, name, query,
    │                                            scheduleType, scheduleTime,
    │                                            scheduleInterval, enabled,
    │                                            llmProviderId, createdAt,
    │                                            updatedAt, lastRunAt
    ├── CreateScheduledJobDocument            ← mutation createScheduledJob
    ├── UpdateScheduledJobDocument            ← mutation updateScheduledJob
    └── DeleteScheduledJobDocument            ← mutation deleteScheduledJob
                                                (input: { rowId })
```

## Rules

- **SCG1 — Named operations only.** The name is the TanStack Query cache key
  (`../../graphql/getQueryKeyFromDocument.ts`).
- **SCG2 — The remote field is `llmProviderId`; the local field is
  `providerId`.** `../syncSchedulesToBackend.ts` maps between them. Don't
  "fix" the mismatch inside a selection set.
- **SCG3 — The list query is capped at `first: 100`.** More than 100 jobs
  requires pagination here *and* in `syncSchedulesToBackend.ts`.
- **SCG4 — Delete takes an input object, not a bare `String` argument**
  (`input: { rowId: $rowId }`) — that is the generated Relay-style shape.
- **SCG5 — Update `schema/schema.graphql` and re-run codegen before editing
  a selection set**, or the `graphql()` tag will not type-check.

## Workflows

**Adding a synced field**
1. Add the column to `schema/schema.graphql`; `bun run codegen`.
2. Extend the selection set in `GetScheduledJobsByProfileIdDocument` and the
   relevant mutation input.
3. Map local ↔ remote in `../syncSchedulesToBackend.ts`.
4. Extend `RemoteScheduledJob` and `toComparable()` there.

**Adding a new scheduled-job operation**
1. Add it to this file with a descriptive `…Document` export name.
2. Call it via `execute()` from `../syncSchedulesToBackend.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — schedule rules (SCD6).
- [`../syncSchedulesToBackend.ts`](../syncSchedulesToBackend.ts) — the caller and the diff logic.
- [`../scheduleTypes.ts`](../scheduleTypes.ts) — the local `ScheduledJob` shape.
- [`../../graphql/AGENTS.md`](../../graphql/AGENTS.md) — `execute()` and the hooks.
- [`../../../../schema/AGENTS.md`](../../../schema/AGENTS.md) — the SDL source.
