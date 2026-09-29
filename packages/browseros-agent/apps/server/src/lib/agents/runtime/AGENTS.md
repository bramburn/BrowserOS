# `src/lib/agents/runtime/` — per-adapter agent runtimes

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/agents/`.

## What's here

The `AgentRuntime` layer: one class per adapter, two abstract bases, and a
registry. `claude-host-process-runtime.ts` and `codex-host-process-runtime.ts`
run their agent as a host process; `hermes-container-runtime.ts` runs inside
a container. `registry.ts` holds the process-wide registry (and a
test-only `resetAgentRuntimeRegistry()`), and `index.ts` is the barrel every
other module imports from.

## Contents

| File | Purpose |
|---|---|
| `agent-runtime.ts` | The top-level `AgentRuntime` interface every adapter implements, plus the shared `descriptor` contract. |
| `types.ts` | Shared runtime types. Pure types, no behaviour. |
| `host-process-agent-runtime.ts` | Abstract base for host-process runtimes; `HostProcessAgentRuntime` + `buildHostProcessProbeEnv()`. |
| `container-agent-runtime.ts` | Abstract base for container-backed runtimes. |
| `claude-host-process-runtime.ts` | `ClaudeRuntime`, `configureClaudeRuntime`, `getClaudeRuntime`, `prepareClaudeCodeContext`. |
| `codex-host-process-runtime.ts` | `CodexRuntime`, `configureCodexRuntime`, `getCodexRuntime`, `prepareCodexContext`. |
| `hermes-container-runtime.ts` | `HermesContainerRuntime`, `configureHermesRuntime`, `getHermesRuntime`, `ensureHermesRuntimeReady`, `prepareHermesContext`. |
| `registry.ts` | `AgentRuntimeRegistry` (throws on duplicate `adapterId`), `getAgentRuntimeRegistry()`, `resetAgentRuntimeRegistry()`. |
| `errors.ts` | `ActionNotSupportedError`, `RuntimeNotReadyError`. |
| `index.ts` | Barrel re-exporting every runtime, config fn, and error. |

## Rules

**RT1 — Import runtimes from `index.ts`, never from the concrete file.** It
keeps the barrel the single surface and makes a runtime swap a one-file change.

**RT2 — Registration is explicit and happens at startup.** `main.ts` calls
`configureClaudeRuntime()` and `configureCodexRuntime()` before
`initCoreServices()`. Hermes is configured lazily and *prepared* through
`ensureHermesRuntimeReady({ resourcesDir })` from
`../../../api/server.ts`. A new adapter needs its `configure*Runtime()` call
in `main.ts` or it will not exist at runtime.

**RT3 — Extend the right base class.** Host-process adapters extend
`HostProcessAgentRuntime`; sandboxed adapters extend
`ContainerAgentRuntime`. Mixing the two (a container adapter that spawns a
host process, or vice versa) breaks the readiness/error contract.

**RT4 — Readiness is a typed error, not a boolean.**
`RuntimeNotReadyError` distinguishes "not configured" from "configured but
not started"; `ActionNotSupportedError` distinguishes "this adapter cannot do
that" from "it failed". Callers in
`../../../api/services/agents/agent-harness-service.ts` map these to responses.

**RT5 — `resetAgentRuntimeRegistry()` is test-only.** Production code never
calls it; if you find a production call site, that's a bug.

**RT6 — `types.ts` stays type-only.** Behaviour belongs in the runtime
files; keeping this file pure lets every module import it without a cycle.

## Workflows

**Adding a host-process adapter:** 1. Extend `HostProcessAgentRuntime`. 2.
Implement `configure/get/prepare` functions. 3. Export them from `index.ts`.
4. Call `configure<Name>Runtime()` in `../../../main.ts`. 5. Add it to
`AgentAdapter` and `AGENT_ADAPTER_CATALOG` in `../`. 6. Add a test mirroring
`tests/lib/agents/runtime/claude-host-process-runtime.test.ts`.

**Adding a container adapter:** same, but extend `ContainerAgentRuntime` and
provide a readiness probe (as `ensureHermesRuntimeReady` does), then wire the
`EnsureVmRuntimeReady` callback in `../../../api/server.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — ACPX runtimes, stores, turn registry.
- [`../../../main.ts`](../../../main.ts) — where `configure*Runtime()` is called.
- [`../../../api/server.ts`](../../../api/server.ts) — `ensureHermesRuntimeReady` wiring.
- [`../../../../tests/lib/agents/runtime/AGENTS.md`](../../../../tests/lib/agents/runtime/AGENTS.md) — runtime tests.
