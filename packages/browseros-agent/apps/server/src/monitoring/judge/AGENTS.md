# `src/monitoring/judge/` — remote LLM judge

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/monitoring/`.

## What's here

The evaluator that scores a completed agent run. `llm-judge.ts` is the remote
client (configuration resolution, request shape, `LazyMonitoringJudgeError`);
`service.ts` is a thin factory that builds it lazily so a judge outage costs
nothing at startup; `types.ts` holds the judge's own input/output shapes. The
judge is deliberately decoupled from the rest of monitoring — it runs after a
run is finalised, never during a tool call.

## Contents

| File | Purpose |
|---|---|
| `llm-judge.ts` | `resolveLazyMonitoringJudgeConfig()`, `RemoteLazyMonitoringJudgeClient`, `LazyMonitoringJudgeError`. The remote call. |
| `service.ts` | `createLazyMonitoringJudgeService()` — the lazy client factory used by `../service.ts`. |
| `types.ts` | `LazyMonitoringJudgeInput`, `LazyMonitoringJudgment`, `LazyMonitoringPolicyDimension`, plus the monitoring types it builds on. |

## Contents (inputs)

The judge consumes a `JudgeAuditEnvelope` built by
[`../envelope.ts`](../envelope.ts): a
`MonitoringSessionContext`, the ordered `MonitoringToolCallRecord`s, the
chat turns, and the finalization marker. It returns a judgment with
per-dimension scores.

## Rules

**JD1 — Lazy, and failure is non-fatal.** `createLazyMonitoringJudgeService()`
defers all network work to first use. A judge that is unreachable must leave
the run recorded and unjudged — never surface as a tool error or a 500.

**JD2 — `LazyMonitoringJudgeError` is the only error type.** It means
"judge unavailable", not "run was bad". A judgement that *declines* to score
is a result, not an error; do not throw it.

**JD3 — Don't call the judge inline.** It runs on run finalisation in
`../service.ts`. Calling it from a tool handler would put a network round
trip in the agent's critical path.

**JD4 — The envelope is the contract.** Judge input is the structure from
`../envelope.ts`, not raw tool arguments. If you add a signal, extend the
envelope so existing judges keep working.

**JD5 — Config from `EXTERNAL_URLS` / `TIMEOUTS`.** No literals in
`llm-judge.ts`.

## Workflows

**Adding a judging dimension:** 1. Extend `LazyMonitoringPolicyDimension` in
`types.ts`. 2. Handle it in the request/response mapping in `llm-judge.ts`.
3. Surface it in `buildJudgeAuditEnvelope()` if it needs new input.

**Disabling the judge:** stop calling the service in `../service.ts`. Do not
delete the folder — the audit trail in `../storage.ts` is independent of it.

**Testing:** `tests/monitoring-judge.test.ts` covers this module; it runs
offline (no browser required).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — monitoring service, observer, storage.
- [`../envelope.ts`](../envelope.ts) — the judge input builder.
- [`../types.ts`](../types.ts) — `MonitoringToolCallRecord` and friends.
- [`../../../../tests/AGENTS.md`](../../../tests/AGENTS.md) — `monitoring-judge.test.ts`.
