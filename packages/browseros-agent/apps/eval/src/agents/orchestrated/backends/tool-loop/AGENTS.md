# `src/agents/orchestrated/backends/tool-loop/` — BrowserOS tool-loop executor

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/agents/orchestrated/backends/`.

## What's here

Executes a delegated goal with the server's own agent, rather than with a
screenshot-reading model. `tool-loop-executor-backend.ts` creates an
`AiSdkAgent` from `@browseros/server/agent/tool-loop` over a real `Browser`
instance and the canonical `registry` from `@browseros/server/tools/registry`,
then runs one `toolLoopAgent.generate()` per delegation.
`tool-loop-executor-prompt.ts` holds the executor system prompt that replaces
the production agent prompt for eval runs.

```
tool-loop/
├── tool-loop-executor-backend.ts  ← ToolLoopExecutorBackend (execute/close/getTotalSteps)
└── tool-loop-executor-prompt.ts   ← TOOL_LOOP_EXECUTOR_SYSTEM_PROMPT
```

## Rules

### TL1 — The Browser instance is mandatory
`execute()` throws `Browser instance is required for tool-loop executor` when
`options.browser` is null. Callers get it from the orchestrator evaluator's CDP
connection, not from a global.

### TL2 — The eval run is isolated by four fields
Each `execute()` sets a fresh `conversationId`,
`userSystemPrompt: TOOL_LOOP_EXECUTOR_SYSTEM_PROMPT`, `evalMode: true`, and
`workingDir: /tmp/browseros-eval-executor-<uuid>`. All four must stay: they are
what let a trace be told apart from a real user session, and the working dir
keeps tool side effects out of shared state.

### TL3 — `dispose()` is mandatory and failures are ignored
`finally { if (agent) await agent.dispose().catch(() => {}) }`. Adding a second
resource means adding it to the same `finally`.

### TL4 — Step counting is tool-call based
`experimental_onToolCallFinish` increments `stepsUsed`, and
`actionsPerformed` is reported as the delta across the delegation. That number
feeds `LIMITS.maxTotalSteps` in the orchestrator — don't redefine it as
`onStepFinish` count.

### TL5 — Browser context is derived from the first page
`browserContext()` uses `browser.listPages()[0]` and returns `undefined` when
there is none. It is passed to `AiSdkAgent.create` so the agent starts with a
correct page id; it is not a per-step refresh.

## Workflows

### Changing what the executor is told to do
Edit `TOOL_LOOP_EXECUTOR_SYSTEM_PROMPT` only. It is injected as
`userSystemPrompt` and overrides the server default, so a production prompt
change does not reach eval traces.

### Comparing tool-loop against Clado
Both implement `ExecutorBackend` and return the same `DelegationResult` shape,
so the only difference to expect is `toolsUsed` (real tool names vs. a single
synthetic `clado_action_predict` event) and step granularity.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — backend contract and factory.
- [`../clado/AGENTS.md`](../clado/AGENTS.md) — the alternative backend.
- [`../../orchestrator-executor/AGENTS.md`](../../../orchestrator-executor/AGENTS.md) — the caller.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) — Bun monorepo parent.
