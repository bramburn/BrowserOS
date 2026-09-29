# `src/utils/` — Shared helpers

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; small, dependency-free helpers.

## What's here

Eight modules. `mcp-client.ts` is the important one: the one-shot
`callMcpTool` used by the pipeline, screenshots and graders, plus the
persistent `McpClient` used by the Clado executor. The rest are narrow
utilities — provider/env resolution, timeouts, dataset metadata, token usage,
sleep, and the config validator that gates a run.

```
utils/
├── mcp-client.ts              ← callMcpTool (one-shot), McpClient (persistent)
├── resolve-provider-config.ts ← resolveProviderConfig (BrowserOS vs direct providers)
├── resolve-env.ts             ← resolveEnvValue: ALL_CAPS → process.env lookup
├── config-validator.ts        ← validateConfig, printValidationResult
├── with-eval-timeout.ts       ← withEvalTimeout, TerminationReason
├── token-usage.ts             ← emptyTokenUsage, hasAnyTokenUsage, addTokenUsageFromAiSdkStep
├── dataset-metadata.ts        ← extractDatasetMetadata
└── sleep.ts                   ← sleep(ms, signal?)
```

## Rules

### UT1 — Two MCP call styles, pick by frequency
`callMcpTool` opens a connection, calls, and tears down — correct for
occasional calls (navigation, `list_pages`, `evaluate_script`, screenshots
without a `Browser`). `McpClient` connects lazily and must be `close()`d
(`CladoActionExecutor.close()` does it). Both apply the same 65s per-call
timeout (`MCP_TOOL_TIMEOUT_MS`).

### UT2 — Env-var indirection is the only way to store a key
`resolveEnvValue` treats a value matching `^[A-Z][A-Z0-9_]*$` as an env var
name and substitutes it; anything else is a literal. `config-validator.ts`
validates that the referenced variables are actually set before a run starts.
Never inline a key in a config, suite, or task line.

### UT3 — The BrowserOS provider path is special
`resolveProviderConfig` intercepts `provider === 'browseros'`, requires
`BROWSEROS_CONFIG_URL`, fetches the config via
`@browseros/server/lib/clients/gateway`, and returns the upstream model with
`upstreamProvider` set. All other providers get their env fields resolved.
A new provider must either fit `LLMConfigSchema` or be added to this switch.

### UT4 — One timeout wrapper
`withEvalTimeout` owns AbortController creation, the timer, the error → 
`terminationReason` classification (`completed` / `max_steps` / `error` /
`timeout`), the `capture.addError('agent_execution', …)` call, and the
`finally` that clears the timer. Agents must not hand-roll a second one; the
`TerminationReason` value is persisted in `metadata.json` and read back by the
resume path in `../runs/task-run-pipeline.ts`.

### UT5 — Validation failures are lists, not exceptions
`validateConfig` returns `{ valid, errors, warnings, config }` and
`printValidationResult` renders it; `../runs/eval-runner.ts` throws a generic
`Configuration validation failed…` afterwards. Adding a check means adding to
`errors`/`warnings` and printing it in the same function. The server health
check is deliberately absent — the harness owns the browser/server lifecycle.

### UT6 — Tool-name prefixes are normalised once
`../reporting/task-metrics.ts` `normalizeToolName` is the single place that
strips `mcp__browseros__` / `mcp__…__` prefixes. Keep any new aggregation
behind it.

## Workflows

### Supporting a new provider in a suite
1. Confirm it is in `LLM_PROVIDERS` / `LLMConfigSchema`
   (`@browseros/shared/schemas/llm`).
2. Nothing to change in `resolveProviderConfig` unless it is a
   `browseros`-style remote config.
3. Set the key as an env var name in the suite/config and export it
   (`config-validator` warns if it is missing).

### Timing out a long agent run
`timeout_ms` in the config / `timeoutMs` in a suite is consumed by the agent
evaluators, which pass it to `withEvalTimeout`. `MCP_TOOL_TIMEOUT_MS` (65s) and
`SCREENSHOT_TIMEOUT_MS` are separate, per-tool budgets — raising one does not
raise the other.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness map and secret-handling rule.
- [`../capture/AGENTS.md`](../capture/AGENTS.md) — the main `callMcpTool` consumer.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — `withEvalTimeout` users.
- [`../agents/orchestrated/backends/clado/AGENTS.md`](../agents/orchestrated/backends/clado/AGENTS.md) — the `McpClient` owner.
- [`../constants.ts`](../constants.ts) — timeout constants that must stay distinct from MCP timeouts.
- [`../../../tests/utils/AGENTS.md`](../../tests/utils/AGENTS.md) — provider-config tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
