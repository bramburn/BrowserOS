# `src/monitoring/` — tool-execution monitoring and LLM judge

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/`.

## What's here

Observability for agent tool use. `observer.ts` defines the
`ToolExecutionObserver` interface that the MCP tool wrapper calls on every
tool start and end; `service.ts` is the process-wide service that records
runs and, at the end of a run, asks the judge in `judge/` to evaluate the
transcript. `storage.ts` persists runs as files, `session-registry.ts` tracks
which runs are in flight, and `envelope.ts` builds the audit envelope handed
to the judge.

## Contents

| File | Purpose |
|---|---|
| `observer.ts` | `ToolExecutionObserver` (`onToolStart`, `onToolEnd`) plus `swallowMonitoringError()` — monitoring must never break a tool call. |
| `service.ts` | `getMonitoringService()` — the singleton that records tool calls and finalises runs. |
| `session-registry.ts` | Tracks active `MonitoringSessionContext`s keyed by `monitoringSessionId`. |
| `storage.ts` | File-backed persistence (`mkdir`, `appendFile`, `readFile`, `readdir`) for monitoring runs. |
| `envelope.ts` | `buildJudgeAuditEnvelope()` — assembles `MonitoringToolCallRecord[]` + finalization into the judge's input. |
| `types.ts` | `MonitoringSessionContext`, `MonitoringToolCallRecord`, `MonitoringChatTurn`, `MonitoringFinalization`, `MonitoringToolStartInput`, `MonitoringToolEndInput`. |
| `judge/` | The remote LLM judge — see [`judge/AGENTS.md`](judge/AGENTS.md). |

## Contents (data flow)

`api/services/mcp/register-mcp.ts` calls `observer.onToolStart` /
`onToolEnd` per tool call → `service.ts` appends to the in-flight run →
`session-registry.ts` closes the run when the scope ends →
`envelope.ts` → `judge/service.ts` → `judge/llm-judge.ts` (remote).

## Rules

**MO1 — Monitoring must never fail a tool call.** `swallowMonitoringError()`
exists for exactly this. Every call site is wrapped; a judge outage degrades
to "no judgement", never to a failed browser action.

**MO2 — Sessions are per MCP scope.** `routes/mcp.ts` reads
`X-BrowserOS-Scope-Id` (default `'ephemeral'`) and threads it into the
monitoring service. Don't introduce a second notion of session identity.

**MO3 — Records are structured, not free text.** Extend the `types.ts`
interfaces rather than packing extra data into a message string; the judge
parses these shapes.

**MO4 — Storage is append-oriented.** `storage.ts` uses `appendFile` plus a
directory read. A monitoring write that rewrites a run file will race with
concurrent tool calls.

**MO5 — The judge is remote and lazy.** `judge/service.ts` exposes a lazy
client, and `LazyMonitoringJudgeError` distinguishes "judge unavailable"
from "judge said no". Both are non-fatal by design.

## Workflows

**Adding a field to a monitoring record:** 1. Extend the type in `types.ts`.
2. Populate it at the `onToolStart` / `onToolEnd` call site in
`../api/services/mcp/register-mcp.ts`. 3. Confirm the judge envelope in
`envelope.ts` carries it. 4. Update `tests/monitoring-service.test.ts` and
`tests/monitoring-storage.test.ts`.

**Debugging a missing run:** check `session-registry.ts` for a session that
was never finalised, then `storage.ts` for the file. Debug runs can also be
driven manually via `POST /monitoring/debug/runs` and
`POST /monitoring/debug/runs/:id/finalize`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`./judge/AGENTS.md`](judge/AGENTS.md) — the LLM judge.
- [`../api/services/mcp/AGENTS.md`](../api/services/mcp/AGENTS.md) — the observer call site.
- [`../../../tests/AGENTS.md`](../../tests/AGENTS.md) — `monitoring-*.test.ts` coverage.
