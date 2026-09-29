# `src/agents/orchestrated/backends/` — Backend factory

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `src/agents/orchestrated/`.

## What's here

One file: `create-executor-backend.ts`. It exports
`createExecutorBackend(options)` and `backendKindForProvider(provider)`, and it
is the only place that decides whether a delegation runs through the BrowserOS
tool loop or through the Clado visual-action model. Options carry the
`ResolvedAgentConfig` template, the `Browser` instance (tool-loop only), the
`serverUrl` (clado only), `initialPageId`, `ExecutorCallbacks`, and an optional
pre-built `executor` that short-circuits construction.

## Rules

### BB1 — Precedence is: injected executor → explicit kind → provider name
`createExecutorBackend` returns `options.executor` if present; otherwise it uses
`options.backendKind ?? backendKindForProvider(options.provider ?? options.configTemplate?.provider ?? '')`.
Tests depend on the injected-executor path, so don't reorder it.

### BB2 — `required()` is the failure mode
Missing `configTemplate` (both backends) or `serverUrl` (clado only) throws
`"<name> is required"`. Tests assert on this; keep the message shape.

### BB3 — Backend subfolders stay pure
`clado/` and `tool-loop/` must not import the factory — the dependency edge is
one-way (factory → backend). `clado/types.ts` is imported by the factory for
`isCladoActionProvider`; nothing else in `clado/` should be imported from here.

## Workflows

### Choosing a backend for a new suite
`configs/suites/*.json` sets `agent.executorBackend` (`'tool-loop' | 'clado'`);
`suites/schema.ts` requires it for `orchestrated` / `orchestrator-executor`
suites. Legacy configs derive it in `suites/config-adapter.ts`
`executorBackend()`. Verify both paths produce the same backend.

### Tracing a backend choice at runtime
`agents/orchestrator-executor/index.ts` passes `executorBackend` through
`SuiteAgent` → `OrchestratorExecutorConfig`; when in doubt log
`config.agent.executor.provider` — `'clado-action'` is the sentinel.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the `ExecutorBackend` contract.
- [`./clado/AGENTS.md`](./clado/AGENTS.md) — the visual-action backend.
- [`./tool-loop/AGENTS.md`](./tool-loop/AGENTS.md) — the BrowserOS tool-loop backend.
- [`../../../suites/AGENTS.md`](../../../suites/AGENTS.md) — where `executorBackend` is declared.
- [`../../../../tests/agents/AGENTS.md`](../../../../tests/agents/AGENTS.md) — `executor-backend.test.ts`.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) — Bun monorepo parent.
