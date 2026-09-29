# `tests/lib/agents/` — ACPX agent unit tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/`.

## What's here

Seventeen tests for `../../../../src/lib/agents/` — the ACPX process
runtime, the agent store, the adapter catalogue, and the turn bookkeeping. All
of it runs offline: the agent *process* is faked, the database is a temp
SQLite handle, and the on-disk agent state is a temp directory. `runtime/` and
`hermes/` mirror those source subfolders.

## Contents

| File | Covers |
|---|---|
| `acpx-runtime.test.ts` | `AcpxRuntime` — process supervision and the ACP session. |
| `acpx-runtime-context.test.ts` | Building the on-disk context handed to the agent. |
| `acpx-runtime-state.test.ts` | Hashed, atomically-written runtime state. |
| `acpx-agent-adapter.test.ts` | Binding a persisted `AgentDefinition` to an `AcpxRuntime`. |
| `acp-ui-message-stream.test.ts` | ACP `UIMessageChunk` → AI-SDK UI stream translation. |
| `active-turn-registry.test.ts` | Single-flight turn tracking (backs `TurnAlreadyActiveError`). |
| `agent-catalog.test.ts` | `AGENT_ADAPTER_CATALOG` — model lists, defaults, reasoning efforts. |
| `db-agent-store.test.ts` | The Drizzle implementation of the `AgentStore` interface over `agent_definitions`. |
| `message-queue.test.ts` | Durable per-agent message queue. |
| `bundled-bun.test.ts` | Resolution of the bundled Bun binary. |
| `runtime/` | Per-adapter runtimes — see [`runtime/AGENTS.md`](runtime/AGENTS.md). |
| `hermes/` | Hermes paths + provider map — see [`hermes/AGENTS.md`](hermes/AGENTS.md). |

## Rules

**AG-T1 — Fake the agent process, not the runtime interface.** The ACPX
runtime supervises a real third-party binary in production; tests substitute
the process boundary so the state machine, path resolution, and stream
translation are still exercised.

**AG-T2 — Temp directories for every on-disk concern.** `acpx-runtime-state`,
`message-queue`, and the Hermes harness paths all write files. Each test gets
its own `mkdtemp` and removes it in `afterEach` — the atomic
write-then-rename behaviour is easy to break with a shared directory.

**AG-T3 — `resetAgentRuntimeRegistry()` between tests.** The runtime registry
is a process-wide singleton that throws on duplicate `adapterId`. Without a
reset, registering the same adapter in a second test throws.

**AG-T4 — `db-agent-store.test.ts` needs a real SQLite handle, not a real
install.** `openBrowserOsDatabase({ dbPath: <temp>, migrationsDir,
runMigrations })` from `../../../../src/lib/db/client.ts` is the sanctioned
way. Never point a test at the user's `~/.browseros` database.

**AG-T5 — Catalog changes are data, not behaviour.** Adding a model to
`AGENT_ADAPTER_CATALOG` is a fixture-like change; assert the descriptor
shape, not a snapshot of the whole catalogue.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Adding an adapter:** 1. Extend the runtime in `runtime/`. 2. Add
`runtime/<adapter>.test.ts` mirroring an existing one. 3. Update
`agent-catalog.test.ts` for the new descriptor. 4. Cover the store write in
`db-agent-store.test.ts`.

**Testing a turn-lifecycle change:** `active-turn-registry.test.ts` first
(single-flight is a hard invariant), then
`acpx-agent-adapter.test.ts` for the binding, then the service-level test at
[`../../api/services/agents/`](../../api/services/agents/AGENTS.md).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/` conventions.
- [`../../../../src/lib/agents/AGENTS.md`](../../../src/lib/agents/AGENTS.md) — the module under test.
- [`./runtime/AGENTS.md`](runtime/AGENTS.md) — runtime tests.
- [`../../__helpers__/AGENTS.md`](../../__helpers__/AGENTS.md) — the harness.
