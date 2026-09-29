# `src/api/services/agents/` — agent harness service

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/api/services/`.

## What's here

One file: `agent-harness-service.ts` (~27 KB, 800+ lines). It is the
application service behind `/agents` — it owns agent definitions
(create/list/get/update), their liveness state, and the lifecycle of an agent
*turn*, bridging to the ACPX runtime in `../../../lib/agents/acpx-runtime.ts`.

## Contents

| Symbol | Purpose |
|---|---|
| `AgentHarnessService` | The service class. Constructed in `../../../server.ts` and passed to `../../../routes/agents.ts`. |
| `AgentLiveness` | `'working' \| 'idle' \| 'asleep' \| 'error'`. |
| `AgentActivity` | Activity payload surfaced alongside an agent definition. |
| `AgentDefinitionWithActivity` | `AgentDefinition` + activity, as returned by list/get. |
| `TurnLifecycleEvent` / `TurnLifecycleListener` | Hooks other subsystems use to observe a turn (start/finish/error). |
| `HarnessManagedVmAdapter` | `Extract<…>` over the managed-VM adapter union — currently Hermes. |
| `EnsureVmRuntimeReady` | Async callback injected by `server.ts` to warm a VM-backed adapter before use. |
| `UnknownAgentError`, `InvalidAgentUpdateError`, `HermesProviderConfigInvalidError`, `TurnAlreadyActiveError` | Typed errors the routes map to status codes. |

## Rules

**AH1 — One active turn per agent.** Starting a second concurrent turn throws
`TurnAlreadyActiveError`; do not "fix" this by queueing. The invariant is
backed by `ActiveTurnRegistry` in `../../../lib/agents/`.

**AH2 — VM readiness is injected, not imported.** `server.ts` passes
`ensureVmRuntimeReady`, which dispatches to
`ensureHermesRuntimeReady({ resourcesDir })` for the `hermes` adapter. The
harness must not import VM/container code directly — that is what keeps it
testable.

**AH3 — Extend the typed error set rather than throwing strings.** Each error
class exists so `routes/agents.ts` can map it to a specific HTTP response.
A bare `throw new Error(...)` collapses into a 500.

**AH4 — Persist through the injected store.** Agent definitions live in the
`agent_definitions` table; go through `db-agent-store.ts` /
`agent-store.ts` in `../../../lib/agents/`, never raw SQL.

**AH5 — Liveness is a server-owned projection.** `AgentLiveness` is derived
from turn state, not reported by the adapter. When you add a new terminal
turn outcome, update the mapping here so `/agents` reports it.

## Workflows

**Adding an agent lifecycle event:** extend `TurnLifecycleEvent`, emit it from
the turn runner in this file, and add a listener — the ACP/UI stream in
`../../../../lib/agents/acp-ui-message-stream.ts` is one such consumer.

**Adding an agent operation:** add a method on `AgentHarnessService`, expose it
in `../../../routes/agents.ts` with zod validation, and add coverage in
`../../../../tests/api/services/agents/agent-harness-service.test.ts`.

**Adding a new VM-backed adapter:** widen `HarnessManagedVmAdapter`, add a
case to the `ensureVmRuntimeReady` switch in `../../../server.ts`, and register
the runtime via `configure*Runtime()` in `../../../lib/agents/runtime/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/api/services/` conventions.
- [`../../routes/agents.ts`](../../routes/agents.ts) — the HTTP surface.
- [`../../../lib/agents/AGENTS.md`](../../../lib/agents/AGENTS.md) — ACPX runtime, stores, turn registry.
- [`../../../../tests/api/services/agents/AGENTS.md`](../../../../tests/api/services/agents/AGENTS.md) — tests.
