# `src/agents/orchestrator-executor/` — Orchestrator evaluator

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/agents/`; plan-then-act agent.

## What's here

`index.ts` (`OrchestratorExecutorEvaluator`) is the entry: it resolves provider
config for the orchestrator and the executor, opens a CDP connection to this
worker's Chrome, builds the `CaptureContext` callbacks, and hands an
`ExecutorFactory` to `orchestrator-agent.ts`. The orchestrator itself is an AI
SDK `ToolLoopAgent` with exactly one tool, `delegate(instruction)`, so it plans
in natural language and never sees the browser. `types.ts` holds the shared
result shape and the two budget constants.

```
orchestrator-executor/
├── index.ts                 ← OrchestratorExecutorEvaluator, config resolution
├── orchestrator-agent.ts    ← OrchestratorAgent: ToolLoopAgent + delegate tool
└── types.ts                 ← ExecutorResult, ExecutorConfig, ORCHESTRATOR_DEFAULTS, LIMITS
```

## Rules

### OE1 — Both models are mandatory
`resolveAgentConfig` throws `orchestrator.model is required in config` and
`executor.model is required in config` before anything else, and
`executor.baseUrl is required in config for clado-action provider`. These are
the first failures a user hits on a misconfigured suite — keep them early and
explicit.

### OE2 — The orchestrator's budget is `LIMITS` + `maxTurns`
`ORCHESTRATOR_DEFAULTS.maxTurns = 15` (overridable per config),
`LIMITS.maxTotalSteps = 300` across all delegations, and
`LIMITS.delegationTimeoutMs = 300_000` per delegation. The step check happens
inside the `delegate` execute handler, so an exhausted budget returns a
message to the model rather than throwing.

### OE3 — The orchestrator cannot see the browser
That constraint is written into `ORCHESTRATOR_SYSTEM_PROMPT` and is why every
delegation gets a fresh executor. If you add a browser tool to the orchestrator,
the prompt becomes false and the whole evaluation design breaks.

### OE4 — Working dirs are throwaway
Both the orchestrator and the executor get
`/tmp/browseros-eval-<role>-<uuid>`. Don't point them at a real repo: tool side
effects from an eval must not collide with anything.

### OE5 — A missing answer is a failure, not a fallback pass
`run()` falls back to `lastObservation` as the answer when the model produced no
text, but then sets `success = false`. Preserve that: an answer scraped from an
executor observation is not the orchestrator's conclusion.

## Workflows

### Comparing orchestrator vs single agent
Both write the same task artifacts and both are scored by the same graders, so
run the same suite twice with a different `agent.type` and diff
`metadata.json` (steps, duration, termination) plus `grades.json`.

### Reading a failed delegation
1. `metadata.json` for `termination_reason` and duration.
2. `messages.jsonl` for `delegate` tool inputs/outputs — the executor
   observation is embedded in the tool output text.
3. `screenshots/` keyed by the `screenshot: N` stamp on the tool-output events.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — agent folder rules.
- [`../orchestrated/AGENTS.md`](../orchestrated/AGENTS.md) — the executor backends this drives.
- [`../claude-code/AGENTS.md`](../claude-code/AGENTS.md) — the other non-single agent.
- [`../../types/AGENTS.md`](../../types/AGENTS.md) — `OrchestratorExecutorConfigSchema`.
- [`../../../../tests/agents/AGENTS.md`](../../../tests/agents/AGENTS.md) — tests.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — Bun monorepo parent.
