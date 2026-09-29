# `tests/agents/` — Agent-level tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

Six unit tests covering the parts of `src/agents/` that can run without a
browser: the Clado action parsing/normalisation helpers, the Clado→MCP
translation layer, the executor-backend factory, and the Claude Code evaluator
(stream parsing, process spawning, arg/MCP-config building). The agents
themselves (`single-agent.ts`, `orchestrator-executor/`) are not unit tested —
they need CDP and an LLM.

```
tests/agents/
├── clado-actions.test.ts            ← parse/format/signature helpers
├── clado-browser-driver.test.ts     ← coordinate mapping, arg normalisation
├── executor-backend.test.ts         ← createExecutorBackend selection rules
├── claude-code-evaluator.test.ts    ← evaluator + prompt/argv/MCP-config builders
├── claude-code-process-runner.test.ts ← injected spawn: lines, stderr, abort, exit
└── claude-code-stream-parser.test.ts  ← stream-json → UIMessageStreamEvent, token usage
```

## Rules

### TA1 — Test the pure helpers, not the loops
`clado-browser-driver` and `clado-actions` are pure by design
(`src/agents/orchestrated/backends/clado/AGENTS.md`); their tests are the reason
that separation exists. If you find yourself needing a browser to test a Clado
helper, the helper belongs in `clado-browser-driver.ts`.

### TA2 — Use the injected spawn, never a real `claude`
`createClaudeCodeProcessRunner({ spawn })` is the seam. The tests supply a fake
spawner that yields fixed stdout lines; do not add a test that shells out.

### TA3 — Pin the documented constants
`MAX_ACTIONS_PER_DELEGATION` (15), `MAX_CONSECUTIVE_PARSE_FAILURES` (3),
`CLADO_REQUEST_TIMEOUT_MS` and the `clado-action` sentinel are behaviour
contracts, not implementation details. If a test has to change because one of
them changed, that is the point.

## Workflows

### Changing the Clado action contract
1. Update `src/agents/orchestrated/backends/clado/types.ts`.
2. Update `clado-actions.test.ts` for parsing and
   `clado-browser-driver.test.ts` for argument translation.
3. Re-check `src/agents/orchestrated/backends/clado/AGENTS.md` (CL4/CL5) still
   describes the behaviour.

### Adding a backend selection rule
Extend `executor-backend.test.ts` rather than a new file: the factory has one
decision table (injected executor → explicit kind → provider name) and the test
should read as that table.

## Cross-references

- [`../../src/agents/AGENTS.md`](../../src/agents/AGENTS.md) — the folder under test.
- [`../../src/agents/orchestrated/backends/clado/AGENTS.md`](../../src/agents/orchestrated/backends/clado/AGENTS.md) — Clado rules.
- [`../../src/agents/claude-code/AGENTS.md`](../../src/agents/claude-code/AGENTS.md) — Claude Code rules.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
