# `tests/suites/` — Suite-model tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

`schema.test.ts` exercises `EvalSuiteSchema` / `SuiteAgentSchema`: required
fields, the `workers` bounds, the `timeoutMs` bounds, defaults, and the
`superRefine` that makes `executorBackend` mandatory for orchestrated suites.
`config-adapter.test.ts` checks `adaptEvalConfigFile` maps each legacy config
agent type onto the right suite agent, resolves an env-var-named API key, and
derives the suite `id` from the file name.

```
tests/suites/
├── schema.test.ts        ← EvalSuiteSchema defaults, bounds, executorBackend rule
└── config-adapter.test.ts← legacy EvalConfig → { suite, variant, evalConfig }
```

## Rules

### TS1 — Fixtures, not the shipped suite files
Both tests build JSON objects inline. `configs/suites/*.json` are operator
files that change for operational reasons; a test that reads them will fail for
reasons unrelated to code. (If you want coverage of the shipped suites, add a
separate test that only asserts they *parse*.)

### TS2 — Env is injected
`adaptEvalConfigFile(path, { env })` and `resolveVariant({ env })` take an
explicit env record, so the adapter tests never touch `process.env`.

### TS3 — The mapping table is the test
The agent-type mapping is the load-bearing logic: `single → tool-loop`,
`orchestrator-executor → orchestrated` (+ `executorBackend` derived from
`clado-action`), `claude-code → claude-code`. Keep the test written as that
table — it is the fastest way to see what a config change did.

### TS4 — Error paths matter as much as the happy path
A suite without a `browseros` block parses fine and only fails at run time
(`suite browseros config is required to run suite commands`, raised in
`src/cli/commands/suite.ts`). The schema test should keep asserting that the
schema *allows* the omission, so the failure stays where it is today.

## Workflows

### Adding a suite field
1. Add it to `EvalSuiteSchema` in `src/suites/schema.ts` with a default if it
   is optional.
2. Map it in `adaptEvalConfigFile` (legacy → suite) and in
   `src/cli/commands/suite.ts` `suiteToEvalConfig` (suite → runnable config).
3. Extend `schema.test.ts` for the default/bounds and `config-adapter.test.ts`
   for the mapping.

### Verifying a shipped suite still parses
```bash
bun run eval suite --suite configs/suites/agisdk-daily-10.json --variant local
```
A zod failure names the offending field; `--variant` only matters for the model,
so any variant string works for a parse check.

## Cross-references

- [`../../src/suites/AGENTS.md`](../../src/suites/AGENTS.md) — the folder under test.
- [`../../src/types/AGENTS.md`](../../src/types/AGENTS.md) — the legacy `EvalConfigSchema`.
- [`../../configs/suites/AGENTS.md`](../../configs/suites/AGENTS.md) — the shipped suites.
- [`../cli/AGENTS.md`](../cli/AGENTS.md) — where the adapted config is consumed.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
