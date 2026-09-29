# `src/agents/orchestrated/` — Executor-backend abstraction

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/agents/`; how a delegated goal is actually executed.

## What's here

`executor-backend.ts` is the seam between the orchestrator (which plans) and
whatever actually drives the browser (which acts). It defines
`ExecutorBackend` (`kind`, `execute(instruction, signal)`, `close()`,
`getTotalSteps()`), the `DelegationResult`/`ExecutorResult` alias, the
`ExecutorCallbacks` used to stream tool calls and steps into the capture layer,
and `ExecutorBackendKind = 'tool-loop' | 'clado'`.
`backends/create-executor-backend.ts` is the factory: it returns an injected
`options.executor` if present, else picks a backend from an explicit
`backendKind` or from the provider name.

```
orchestrated/
├── executor-backend.ts              ← ExecutorBackend, ExecutorCallbacks, DelegationResult
└── backends/
    ├── create-executor-backend.ts    ← createExecutorBackend + backendKindForProvider
    ├── tool-loop/
    │   ├── tool-loop-executor-backend.ts  ← AiSdkAgent delegation, evalMode: true
    │   └── tool-loop-executor-prompt.ts   ← executor system prompt
    └── clado/                             ← screenshot → action delegation
```

## Rules

### ORC1 — Backends are stateless per delegation
`createExecutorBackend` is called per delegation by
`orchestrator-executor/index.ts`; the orchestrator agent prompt states "every
delegation uses a fresh executor with clean context". Don't cache a browser or
an agent across delegations.

### ORC2 — Provider decides the default backend
`backendKindForProvider` returns `'clado'` for
`isCladoActionProvider(provider)` (`'clado-action'`), else `'tool-loop'`.
`suites/config-adapter.ts` computes the same mapping from a legacy config so a
suite and its config produce identical runs.

### ORC3 — `required()` guards are the config contract
`create-executor-backend.ts` throws `${name} is required` for missing
`configTemplate` / `serverUrl` rather than defaulting. Adding a backend means
deciding which of those it genuinely needs.

### ORC4 — `evalMode: true` is set by the tool-loop backend
`tool-loop-executor-backend.ts` builds its `ResolvedAgentConfig` with
`evalMode: true`, a fresh `conversationId`, a `/tmp/browseros-eval-executor-<uuid>`
working dir, and `TOOL_LOOP_EXECUTOR_SYSTEM_PROMPT`. These four together are
what make the delegated run distinguishable from a user session in traces.

### ORC5 — Report status honestly
`DelegationResult.status` is `'done' | 'blocked' | 'timeout'`. The tool-loop
backend maps a thrown error to `timeout` when the signal aborted, otherwise
`blocked`, and downgrades a `done` result to `timeout` if the signal aborted
afterwards. The orchestrator's `LIMITS.maxTotalSteps` and the next
`delegate()` call depend on this being right.

## Workflows

### Adding a backend
1. Implement `ExecutorBackend` under `backends/<kind>/`.
2. Add the literal to `ExecutorBackendKind` in `executor-backend.ts`.
3. Extend `backendKindForProvider` / the `if` chain in
   `create-executor-backend.ts`.
4. Add the suite-facing enum value in `suites/schema.ts`
   (`SuiteAgentSchema.executorBackend`) and the mapping in
   `cli/commands/suite.ts` `suiteToEvalConfig`.
5. Add tests under `tests/agents/`.

### Tracing why a delegation failed
`orchestrator-agent.ts` renders the backend result back to the orchestrator as
an `Executor Result` text block (status, actions, URL, observation). Read that
observation in `messages.jsonl` — the raw tool payloads are not in the text, so
for the detail use the per-tool stream events.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — agent folder rules.
- [`../orchestrator-executor/AGENTS.md`](../orchestrator-executor/AGENTS.md) — the caller of these backends.
- [`../../constants.ts`](../../constants.ts) — `MAX_ACTIONS_PER_DELEGATION`, `CLADO_REQUEST_TIMEOUT_MS`.
- [`../../../tests/agents/AGENTS.md`](../../../tests/agents/AGENTS.md) — `executor-backend.test.ts` and the Clado driver tests.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — Bun monorepo parent.
