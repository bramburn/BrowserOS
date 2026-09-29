# `tests/lib/agents/runtime/` — agent runtime unit tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/agents/`.

## What's here

Six tests for `../../../../../src/lib/agents/runtime/` — one per runtime
implementation plus the registry. Each constructs its runtime with injected
dependencies and a faked process boundary, so the tests cover readiness,
configuration, and error typing without launching Claude Code, Codex, or a
container.

## Contents

| File | Covers |
|---|---|
| `registry.test.ts` | `AgentRuntimeRegistry` — `register` / `get` / `list` / `unregister`, the duplicate-`adapterId` throw, `getAgentRuntimeRegistry()`, and `resetAgentRuntimeRegistry()`. |
| `host-process-agent-runtime.test.ts` | The `HostProcessAgentRuntime` abstract base and `buildHostProcessProbeEnv()`. |
| `claude-host-process-runtime.test.ts` | `ClaudeRuntime` + `configureClaudeRuntime` / `getClaudeRuntime` / `prepareClaudeCodeContext`. |
| `codex-host-process-runtime.test.ts` | `CodexRuntime` + `configureCodexRuntime` / `getCodexRuntime` / `prepareCodexContext`. |
| `container-agent-runtime.test.ts` | The `ContainerAgentRuntime` abstract base. |
| `hermes-container-runtime.test.ts` | `HermesContainerRuntime` + `ensureHermesRuntimeReady` / `prepareHermesContext`. |

## Rules

**RT-T1 — `resetAgentRuntimeRegistry()` in `afterEach`.** The registry is a
module-level singleton and `register()` throws on a duplicate `adapterId`;
without a reset, the second test in a file fails for the wrong reason.

**RT-T2 — Test the abstract bases directly.** `host-process-agent-runtime.test.ts`
and `container-agent-runtime.test.ts` use a minimal concrete subclass. That is
how the shared lifecycle is covered without a real agent.

**RT-T3 — Readiness is a typed error.** Assert on `RuntimeNotReadyError` and
`ActionNotSupportedError` classes. A test that asserts on the message will
survive a refactor that breaks `instanceof` checks in production callers.

**RT-T4 — "Not configured" and "configured but not started" are different.**
That distinction is the reason `RuntimeNotReadyError` exists. Preserve it
when adding a state.

**RT-T5 — No real process, no real container.** A runtime test that spawns a
binary belongs in [`../../integration/`](../../../integration/AGENTS.md), not here.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Adding a runtime:** 1. Implement it in
`../../../../../src/lib/agents/runtime/` extending the appropriate base. 2.
Export `configure*` / `get*` / `prepare*` from `index.ts`. 3. Add
`<adapter>.test.ts` here, copying the closest existing file as the template. 4.
Wire the `configure*Runtime()` call in
`../../../../../src/main.ts` and the adapter into
`agent-catalog.ts`.

**Testing a new VM-backed adapter:** also cover the `ensureVmRuntimeReady`
callback shape, since `../../../../../src/api/server.ts` dispatches on the
adapter id there.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/agents/` conventions.
- [`../../../../../src/lib/agents/runtime/AGENTS.md`](../../../../src/lib/agents/runtime/AGENTS.md) — the runtimes under test.
- [`../../../../../src/main.ts`](../../../../src/main.ts) — where `configure*Runtime()` is called.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
