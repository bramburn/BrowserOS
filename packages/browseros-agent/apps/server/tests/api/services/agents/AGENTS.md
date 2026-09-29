# `tests/api/services/agents/` — agent harness service tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/api/services/`.

## What's here

One file: `agent-harness-service.test.ts`, covering `AgentHarnessService` from
[`../../../../../src/api/services/agents/agent-harness-service.ts`](../../../../src/api/services/agents/AGENTS.md)
— the ~27 KB service behind every `/agents` endpoint. It is the single test
file for the largest service in the server, so its scope is worth stating
plainly: agent definitions, liveness, turn lifecycle, and the typed errors.

## Contents

| File | Covers |
|---|---|
| `agent-harness-service.test.ts` | `AgentHarnessService` — CRUD, `AgentLiveness` transitions, `TurnLifecycleEvent` emission, and `UnknownAgentError` / `InvalidAgentUpdateError` / `HermesProviderConfigInvalidError` / `TurnAlreadyActiveError`. |

## Rules

**AH-T1 — Inject the runtime, never spawn one.** The service receives its
ACPX runtime and its `EnsureVmRuntimeReady` callback as dependencies (that is
how `server.ts` wires them). A test that constructs a real runtime is a test
that needs a container.

**AH-T2 — `TurnAlreadyActiveError` is a feature.** There is a test asserting
that a second concurrent turn is rejected. Do not "fix" that by making the
service queue turns — the single-flight invariant is deliberate.

**AH-T3 — Assert the typed error, not its message.** The routes map error
*classes* to HTTP status codes. A test asserting on `err.message` will pass
while the route breaks.

**AH-T4 — This file tests the service, not the routes.** HTTP-level
behaviour for `/agents` lives in
[`../../routes/agents.test.ts`](../../routes/AGENTS.md). Keep the two
separate.

## Workflows

**Running:** `bun run test:api` from `apps/server/`.

**Adding coverage for a new service method:** add a `describe` block to
`agent-harness-service.test.ts` with stub deps. If the method needs a new
dependency, add it to the deps object in
`../../../../../src/api/server.ts` and here — never reach for a module-level
singleton.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/api/services/` conventions.
- [`../../../../../src/api/services/agents/AGENTS.md`](../../../../src/api/services/agents/AGENTS.md) — the service under test.
- [`../../../../../src/lib/agents/AGENTS.md`](../../../../src/lib/agents/AGENTS.md) — the ACPX runtime it drives.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
