# `src/suites/` — Suite files and model variants

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the suite-era config model.

## What's here

A *suite* is a JSON file naming a dataset, an agent shape, graders, worker
count and the BrowserOS port block. `schema.ts` validates it.
`load-suite.ts` reads it and resolves `dataset` relative to the suite file
(leading `/` means repo/absolute). `resolve-variant.ts` turns CLI flags and
`EVAL_*` env vars into one `EvalVariant` — the model under test — and splits it
into a secret-bearing `agent` half and a publishable `publicMetadata` half.
`config-adapter.ts` converts a legacy `EvalConfig` file into the suite/variant
model so both config formats can drive the same runner.

```
suites/
├── schema.ts         ← EvalSuiteSchema, SuiteAgentSchema (agent type + executorBackend)
├── load-suite.ts     ← loadSuite: parse + resolve dataset path
├── resolve-variant.ts← resolveVariant: CLI > env > defaults
└── config-adapter.ts ← adaptEvalConfigFile: legacy config → { suite, variant, evalConfig }
```

## Rules

### SU1 — Suite files are the forward format
`EvalSuiteSchema` requires `id` and `dataset`, and defaults `graders: []`,
`workers: 1`, `restartBrowserPerTask: false`. `workers` is capped at 20 and
`timeoutMs` between 30s and 1h. `browseros` and `captcha` are optional in the
schema but **required to run** — `../cli/commands/suite.ts` `ensureRunnableSuite`
throws `suite browseros config is required to run suite commands`.

### SU2 — `executorBackend` is required for orchestrated suites
`SuiteAgentSchema.superRefine` adds the issue
`executorBackend is required for orchestrated suites` when `type` is
`orchestrated` or `orchestrator-executor` and the field is missing. The suite
`agent.type` vocabulary is `tool-loop | single | orchestrated |
orchestrator-executor | claude-code`; only `claude-code` skips the field.

### SU3 — Dataset paths resolve relative to the suite file
`loadSuite` resolves a non-`/`-prefixed `dataset` against the suite's own
directory (the shipped suites use `"../../data/<name>.jsonl"`). Moving a suite
file therefore moves what it points at.

### SU4 — Variant precedence is CLI, then env, then default
`resolveVariant` reads `provider`, `model`, `apiKey`, `baseUrl`,
`supportsImages` from options, then `EVAL_AGENT_*`, defaulting provider to
`openai-compatible`. `EVAL_AGENT_MODEL` is mandatory
(`EVAL_AGENT_MODEL is required`); `requireApiKey` makes
`EVAL_AGENT_API_KEY` mandatory too. `claude-code` short-circuits the whole
model requirement.

### SU5 — Never leak the key through `publicMetadata`
`publicMetadata.agent` carries `apiKeyConfigured: boolean` and `apiKeyEnv`, plus
`baseUrlHost` — never the key or the full URL. This object ends up in
`run.json` and the viewer manifest, which are uploaded to a public CDN.

### SU6 — Adapting a legacy config is lossless for running
`adaptEvalConfigFile` re-validates the file with `EvalConfigSchema`, derives the
suite `id` from the filename, maps `single → { type: 'tool-loop' }`,
`orchestrator-executor → { type: 'orchestrated', executorBackend }`, and
`claude-code → { type: 'claude-code' }`, and resolves the API key through
`apiKeyEnv` when the config stored an env var name. Run the adapter (not the raw
config) when you want the suite-era run ids and viewer manifest.

## Workflows

### Adding a suite
1. Copy an existing `configs/suites/*.json`, set `id` (the run id is derived
   from it), point `dataset` at a file in `../../data/`, pick `agent.type` and
   `graders`.
2. If orchestrated, set `agent.executorBackend`.
3. Fill in the `browseros` block (server_url + base ports).
4. Run: `bun run eval suite --suite configs/suites/<id>.json --variant <variant>`.

### Running one model across many suites
`--variant <id>` combined with `EVAL_AGENT_*` keeps the model choice out of the
suite files, so the same suite file can be run against several providers and
compared by the viewer.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the two config formats and who adapts which.
- [`../cli/commands/AGENTS.md`](../cli/commands/AGENTS.md) — `resolveSuiteCommand` / `suiteToEvalConfig`.
- [`../types/AGENTS.md`](../types/AGENTS.md) — `EvalConfigSchema` (legacy format).
- [`../../configs/suites/AGENTS.md`](../../configs/suites/AGENTS.md) — the shipped suites.
- [`../../configs/legacy/AGENTS.md`](../../configs/legacy/AGENTS.md) — the adapted configs.
- [`../../.env.example`](../../.env.example) — the `EVAL_*` variables this reads.
- [`../../../tests/suites/AGENTS.md`](../../tests/suites/AGENTS.md) — schema and adapter tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
