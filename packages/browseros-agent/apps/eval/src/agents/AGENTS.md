# `src/agents/` — Agent evaluators

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the per-task agent implementations.

## What's here

One class per agent strategy, all satisfying `types.ts` `AgentEvaluator`
(`execute(): Promise<AgentResult>`). `index.ts` `createAgent(context)` switches
on `context.config.agent.type` and returns `SingleAgentEvaluator`,
`OrchestratorExecutorEvaluator`, or `ClaudeCodeEvaluator`. `types.ts` defines the
shared `AgentContext` (config, task, workerIndex, `initialPageId`, output dirs,
`capture`) and `AgentResult` (`metadata`, `messages`, `finalAnswer`).
`orchestrated/` holds the executor-backend abstraction shared by the
orchestrator path; `orchestrator-executor/` is the orchestrator evaluator
itself; `claude-code/` drives the external `claude` CLI.

```
agents/
├── index.ts                       ← createAgent(context) switch on agent.type
├── types.ts                       ← AgentContext, AgentResult, AgentEvaluator
├── single-agent.ts                ← BrowserOS ToolLoopAgent over CDP, per-tool screenshots
├── claude-code/
│   ├── index.ts                   ← ClaudeCodeEvaluator: spawns `claude`, maps stream → messages.jsonl
│   ├── process-runner.ts          ← Bun.spawn wrapper, line reader, abort handling
│   └── stream-parser.ts           ← stream-json → UIMessageStreamEvent + token usage
├── orchestrator-executor/
│   ├── index.ts                   ← OrchestratorExecutorEvaluator (CDP, capture wiring, timeout)
│   ├── orchestrator-agent.ts      ← AI SDK ToolLoopAgent with a single `delegate` tool
│   └── types.ts                   ← ExecutorResult, ORCHESTRATOR_DEFAULTS, LIMITS
└── orchestrated/
    ├── executor-backend.ts        ← ExecutorBackend interface + callbacks
    └── backends/
        ├── create-executor-backend.ts ← picks 'tool-loop' vs 'clado'
        ├── tool-loop/             ← AiSdkAgent-based delegation
        └── clado/                 ← visual (screenshot → action) delegation
```

## Rules

### AG1 — One task, one `AgentContext`, no cross-task state
`AgentContext.initialPageId` is resolved once per task by
`runs/task-run-pipeline.ts` (a fresh browser has exactly one page). Agents must
not re-derive it, and must not cache browser handles beyond `execute()`.

### AG2 — Wrap execution in `withEvalTimeout`
`utils/with-eval-timeout.ts` owns the AbortController, the error→`terminationReason`
mapping (`completed` | `max_steps` | `error` | `timeout`) and the `capture.addError`
call. Handlers that call `capture.screenshot.capture` must catch and swallow
errors — screenshot loss is non-fatal by design.

### AG3 — Screenshot index is the join key
`single-agent.ts` records `screenshotByToolCallId` and stamps
`{ screenshot: N }` on the matching `tool-output-*` event; the viewer uses this
to sync the transcript to the current image. Preserve that pairing in any new
agent, or the replayed trajectory loses sync.

### AG4 — Agent `type` literals come from `types/config.ts`
`AgentConfigSchema` is a `z.discriminatedUnion('type', ...)` over
`'single' | 'orchestrator-executor' | 'claude-code'`. `suites/schema.ts` uses the
suite vocabulary (`tool-loop`, `orchestrated`, `claude-code`);
`cli/commands/suite.ts` `suiteToEvalConfig` maps between them — `tool-loop`
becomes `type: 'single'`. Both sides need updating together.

### AG5 — Never hardcode the worker's CDP port
Use `config.browseros.base_cdp_port + ctx.workerIndex`, as
`single-agent.ts` does. `runner/browseros-app-manager.ts` allocates ports with
the same formula.

### AG6 — `createAgent` is exhaustive over the config union
`index.ts` has no `default` branch; that is deliberate. Adding a config variant
without an evaluator is a type error.

## Workflows

### Adding a delegation backend
1. Implement `ExecutorBackend` (`execute`, `close`, `getTotalSteps`, `kind`) in
   `orchestrated/backends/<kind>/`.
2. Register the branch in `orchestrated/backends/create-executor-backend.ts`.
3. Extend `ExecutorBackendKind` and the `backendKindForProvider` mapping.
4. Unit-test the pure helpers in `tests/agents/` (see the Clado
   `clado-browser-driver.test.ts` for the pattern).

### Debugging a single task without a dataset
Run the legacy path with a `--query` task source: `runner/task-loader.ts`
`createSingleTask` fabricates a `Task` with `dataset: 'manual'` and
`graders: ['performance_grader']`, so any runnable page can be smoke-tested.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/` map and harness-wide rules.
- [`../types/AGENTS.md`](../types/AGENTS.md) — config + task schemas every agent reads.
- [`../capture/AGENTS.md`](../capture/AGENTS.md) — screenshot/message/error capture the agents write into.
- [`../../tests/agents/AGENTS.md`](../../tests/agents/AGENTS.md) — tests for these files.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
