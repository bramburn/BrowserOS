# `src/agents/orchestrated/backends/clado/` — Clado visual-action executor

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/agents/orchestrated/backends/`.

## What's here

A second `ExecutorBackend` that drives the browser from screenshots instead of
tool calls. `clado-action-executor.ts` is the loop: it screenshots the page
over MCP, POSTs the image plus the action history to the Clado action model,
normalises the returned action, applies it through an MCP tool call, and repeats
until the model returns `final_answer` or the step budget runs out.
`clado-client.ts` is the HTTP client (bearer auth, 6-minute request timeout).
`clado-browser-driver.ts` is the pure translation layer. `clado-actions.ts`
parses/history-formats predictions. `types.ts` holds the shared constants and
response types.

```
clado/
├── clado-action-executor.ts  ← CladoActionExecutor: the screenshot→action loop
├── clado-browser-driver.ts   ← pure helpers: coordinate mapping, arg normalisation
├── clado-client.ts           ← CladoActionClient: POST {instruction, image_base64, history}
├── clado-actions.ts          ← parse / history / signature / summary helpers
├── clado-executor-backend.ts ← CladoExecutorBackend: ExecutorBackend wrapper
└── types.ts                  ← CLADO_ACTION_PROVIDER, CLADO_PAGE_SCOPED_TOOLS, response types
```

## Rules

### CL1 — This backend is gated on the `clado-action` provider
`CladoActionExecutor`'s constructor throws unless
`isCladoActionProvider(config.provider)`, i.e. the literal
`CLADO_ACTION_PROVIDER = 'clado-action'`. `orchestrator-executor/index.ts`
additionally requires `executor.baseUrl` for that provider. Keep the sentinel
string in `types.ts` as the single source.

### CL2 — The step budget is `MAX_ACTIONS_PER_DELEGATION` (15)
`constants.ts` caps the loop. `MAX_CONSECUTIVE_PARSE_FAILURES = 3` aborts a
delegation that the model cannot answer coherently. Both are hard stops, not
retries.

### CL3 — Coordinates are 0–1000 normalised
`resolveCladoPoint` clamps to `[0, 999]`, defaults to the centre (500, 500) when
a coordinate is missing, and scales into the last known viewport. The viewport
is only known after the first screenshot — `execute()` resets `viewport` and
`lastPoint` on every call.

### CL4 — `prepareCladoToolArgs` is the MCP contract adapter
It rewrites `evaluate_script`'s `function` into `expression`, maps
`click_at`'s `dblClick` into `clickCount`, and injects `page: pageId` for every
tool listed in `CLADO_PAGE_SCOPED_TOOLS`. If you add a page-scoped tool, add it
to that set or it will be called without a page id.

### CL5 — Keys and scroll amounts are normalised, not passed through
`normalizeCladoPressKey` maps `C-a` → `Control+A` style chords and throws when
`key` is empty; `normalizeCladoDirection` falls back to `down`;
`normalizeCladoScrollAmount` clamps into 100–900px. These are the behaviours
`tests/agents/clado-browser-driver.test.ts` pins.

### CL6 — Credentials stay in the client
`clado-client.ts` sends the API key as a bearer header specifically so it does
not appear in process arguments or in the persisted action history. Keep it out
of `formatCladoHistory` output.

## Workflows

### Smoke-testing a Clado endpoint
`../../../../../scripts/test-clado-api.ts` health-checks the model, runs one
generate call, and prints every documented field. It can capture its own
screenshot over MCP from a running server (`BROWSEROS_URL`, default
`http://127.0.0.1:9110`).

### Adding a new Clado action
1. Extend `CladoAction` / `CladoActionResponse` in `types.ts`.
2. Handle it in `clado-action-executor.ts`'s dispatch.
3. Normalise its arguments in `clado-browser-driver.ts`.
4. Add a case to `tests/agents/clado-actions.test.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — backend contract and factory.
- [`../../../../constants.ts`](../../../../constants.ts) — `MAX_ACTIONS_PER_DELEGATION`, `CLADO_REQUEST_TIMEOUT_MS`.
- [`../../../../utils/AGENTS.md`](../../../../utils/AGENTS.md) — `McpClient` used for tool calls.
- [`../../../../../tests/agents/AGENTS.md`](../../../../../tests/agents/AGENTS.md) — Clado unit tests.
- [`../../../../../configs/suites/AGENTS.md`](../../../../../configs/suites/AGENTS.md) — suites that select this backend.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) — Bun monorepo parent.
