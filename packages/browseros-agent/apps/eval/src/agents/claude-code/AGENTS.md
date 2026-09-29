# `src/agents/claude-code/` — Claude Code CLI agent

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/agents/`; drives the external `claude` binary.

## What's here

Runs a real `claude` CLI process as the agent under test. `index.ts`
(`ClaudeCodeEvaluator`) writes a per-task `claude-code-mcp.json` MCP config into
the task output dir, builds the argv via `buildClaudeCodeArgs`, streams stdout
through `stream-parser.ts`, and persists `metadata.json` through
`capture.trajectorySaver`. `process-runner.ts` wraps `Bun.spawn` with an
abortable line reader; `stream-parser.ts` converts the CLI's stream-json lines
into the same `UIMessageStreamEvent` shape the other agents emit, so the viewer
and graders can treat a Claude Code run like any other run.

```
claude-code/
├── index.ts           ← ClaudeCodeEvaluator, prompt/argv/MCP-config builders
├── process-runner.ts  ← createClaudeCodeProcessRunner (injectable spawn), readLines, abort
└── stream-parser.ts   ← ClaudeCodeStreamParser, shouldCaptureScreenshotForTool
```

## Rules

### CC1 — The CLI talks to this run's MCP server
`buildClaudeCodeMcpConfig` points at `${config.browseros.server_url}/mcp`
(trimming a trailing slash and tolerating a URL that already ends in `/mcp`).
The `server_url` is the per-worker one injected by
`runs/task-worker-pool.ts`, so Claude Code drives the same BrowserOS instance
as the rest of the harness.

### CC2 — `shouldCaptureScreenshotForTool` defines the screenshot policy
Only `mcp__browseros__*` tools trigger a capture, and `__take_screenshot` is
excluded (it already returns an image). Changing the prefix breaks the
single-agent path too — `reporting/task-metrics.ts` `normalizeToolName` strips
the same prefix.

### CC3 — Non-zero exit is tolerated only if text was produced
`index.ts` throws on a non-zero exit code unless the parser already captured
final text; the timeout wrapper then records `terminationReason: 'timeout'`
or `'error'`. Don't swallow the exit code silently.

### CC4 — Keep `process-runner` injectable
`CreateClaudeCodeProcessRunnerDeps.spawn` exists so `tests/agents/claude-code-process-runner.test.ts`
can run without a real binary. Any new process behaviour must stay behind that
seam.

### CC5 — Token usage prefers the aggregate
`ClaudeCodeStreamParser.getTokenUsage()` returns `result.usage` when the CLI
emits it, otherwise the sum of per-message usage, otherwise `null`. Preserve
that precedence; `utils/token-usage.ts` has the `emptyTokenUsage`/`hasAnyTokenUsage`
contract used by the metrics builders.

## Workflows

### Changing what Claude Code is allowed to do
1. Edit `buildClaudeCodeArgs` (`index.ts`) for flags, or the MCP config builder
   for tools. The config schema that feeds them is
   `ClaudeCodeAgentConfigSchema` in `../../types/config.ts` (`claudePath`,
   `model`, `extraArgs`).
2. If you add a flag, check `buildClaudeCodePrompt` still states the contract
   the grader will later verify against.
3. Update `tests/agents/claude-code-evaluator.test.ts` and
   `tests/agents/claude-code-process-runner.test.ts`.

### Debugging a Claude Code run
1. Read the task dir: `metadata.json` (exit/termination),
   `messages.jsonl` (the normalized stream), `claude-code-mcp.json` (the MCP
   wiring that was actually used), `screenshots/`.
2. stderr is captured per line and reported via `ClaudeCodeRunResult.stderr` /
   `streamErrors`, not written to disk.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — agent folder rules (context, timeout, screenshot join key).
- [`../../capture/AGENTS.md`](../../capture/AGENTS.md) — where messages/screenshots are written.
- [`../../../configs/suites/AGENTS.md`](../../../configs/suites/AGENTS.md) — suites that select this agent.
- [`../../../../tests/agents/AGENTS.md`](../../../tests/agents/AGENTS.md) — tests for this folder.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — Bun monorepo parent.
