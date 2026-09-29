# `src/lib/agents/` — ACPX agent runtimes and stores

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/`.

## What's here

Everything behind the `/agents` surface except the HTTP handler: the ACPX
process runtime that hosts third-party coding agents (Claude Code, Codex,
Hermes), the agent catalogue and persistence layer, and the per-agent turn
bookkeeping. `acpx-runtime.ts` is the largest file (~28 KB) — it spawns and
supervises the agent process and speaks the Agent Client Protocol over stdio.
`runtime/` holds the per-adapter runtime implementations; `hermes/` holds
Hermes-specific path and provider mapping.

## Contents

| File | Purpose |
|---|---|
| `acpx-runtime.ts` | `AcpxRuntime` — process supervision + ACP session for an agent. |
| `acpx-agent-adapter.ts` | Binds a persisted `AgentDefinition` to an `AcpxRuntime`. |
| `acpx-agent-common.ts` | Shared helpers for the ACPX agent path. |
| `acpx-runtime-context.ts` | Builds the on-disk context handed to the agent (uses `DEFAULT_PORTS`). |
| `acpx-runtime-state.ts` | Hashed, atomically-written runtime state (`writeFile` + `rename`). |
| `acpx-runtime-templates.ts` | `SOUL_TEMPLATE` and the other scaffolding written into the agent's home directory. |
| `acp-ui-message-stream.ts` | Translates ACP `UIMessageChunk` output into the AI-SDK stream the extension renders. |
| `active-turn-registry.ts` | Single-flight turn tracking; backs `TurnAlreadyActiveError`. |
| `message-queue.ts` | Durable per-agent message queue (atomic file writes). |
| `agent-catalog.ts` | `AGENT_ADAPTER_CATALOG` — per-adapter model list, default model, reasoning-effort options, and `modelControl` policy. |
| `agent-types.ts` | `AgentAdapter = 'claude' \| 'codex' \| 'hermes'`, `AgentAdapterDescriptor`, `AgentDefinition`. |
| `agent-store.ts` | The `AgentStore` interface. |
| `db-agent-store.ts` | Drizzle implementation over the `agent_definitions` table. |
| `adapter-health.ts` | Availability probing per adapter. |
| `bundled-bun.ts` | Resolves the bundled Bun binary used to run the agent. |
| `types.ts` | Shared types re-exported for consumers. |
| `runtime/` | Per-adapter runtimes — see [`runtime/AGENTS.md`](runtime/AGENTS.md). |
| `hermes/` | Hermes paths + provider map — see [`hermes/AGENTS.md`](hermes/AGENTS.md). |

## Rules

**AG1 — New adapter means three edits.** Add it to `AgentAdapter` in
`agent-types.ts`, to `AGENT_ADAPTER_CATALOG` in `agent-catalog.ts`, and
implement a runtime in `runtime/` registered via `configure<Name>Runtime()`.
`adapter-health.ts` and the `/agents` UI derive from those.

**AG2 — The runtime registry is the only wiring point.** `runtime/registry.ts`
throws on a duplicate `adapterId`. `main.ts` calls `configureClaudeRuntime()`
and `configureCodexRuntime()` at startup — a new adapter needs its configure
call there.

**AG3 — Persist only through `AgentStore`.** `db-agent-store.ts` is the Drizzle
implementation; no other module may query `agent_definitions` directly.

**AG4 — Turn state is single-flight.** `active-turn-registry.ts` is the only
place that decides whether a turn may start. Do not add a parallel check in
`api/services/agents/agent-harness-service.ts`.

**AG5 — Write agent state atomically.** `acpx-runtime-state.ts` and
`message-queue.ts` use `writeFile` to a temp path then `rename`. A partial
write here corrupts an agent's identity on disk.

**AG6 — On-disk layout comes from `hermes/hermes-paths.ts` and
`../browseros-dir.ts`.** The host paths must stay reachable from inside the
Lima VM through the existing `/mnt/browseros/vm` bind mount.

## Workflows

**Adding a new agent adapter:** 1. Widen `AgentAdapter` in `agent-types.ts`.
2. Add a `AgentAdapterDescriptor` to `AGENT_ADAPTER_CATALOG` (models, default
model id, reasoning efforts, `modelControl`). 3. Implement a runtime in
`runtime/` (extend `host-process-agent-runtime.ts` or
`container-agent-runtime.ts`). 4. Export `configure<Name>Runtime()` from
`runtime/index.ts` and call it from `../../../main.ts`. 5. Register it in
`runtime/registry.ts`. 6. Add tests under `../../../../tests/lib/agents/`.

**Tracing one agent turn:** `api/services/agents/agent-harness-service.ts` →
`active-turn-registry.ts` → `acpx-agent-adapter.ts` → `acpx-runtime.ts` →
ACP stdio → `acp-ui-message-stream.ts` → the HTTP stream.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/` conventions.
- [`./runtime/AGENTS.md`](./runtime/AGENTS.md) — Claude / Codex / Hermes runtimes.
- [`./hermes/AGENTS.md`](./hermes/AGENTS.md) — Hermes path + provider mapping.
- [`../../api/services/agents/AGENTS.md`](../../api/services/agents/AGENTS.md) — the service that drives all of this.
- [`../../../../tests/lib/agents/AGENTS.md`](../../../tests/lib/agents/AGENTS.md) — mock-only tests.
