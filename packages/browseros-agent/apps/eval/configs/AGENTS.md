# `configs/` — Eval run configurations

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/eval`; two config formats.

## What's here

Two directories holding JSON run definitions. `suites/` is the current format,
validated by `src/suites/schema.ts` (`EvalSuiteSchema`): `id`, `dataset`, an
`agent` block, `graders`, `workers`, `restartBrowserPerTask`, `timeoutMs`, and a
required-at-runtime `browseros` block. `legacy/` holds the older
`src/types/config.ts` `EvalConfigSchema` format (`num_workers`,
`restart_server_per_task`, `snake_case` keys), which
`src/suites/config-adapter.ts` converts into a suite + variant.

```
configs/
├── suites/    ← suite-era: agisdk-daily-10, agisdk-real-smoke, agisdk-real
└── legacy/    ← config-era: weekly runs, per-model runs, clado, infinity, smoke sets
```

## Rules

### CFG1 — New runs go in `configs/suites/`
`configs/legacy/` exists to be read, not extended: those files use a different
schema, and every consumer that understands them does so through the adapter.
Add new configs as suites.

### CFG2 — Keys are literal port numbers, and they are the shared resource
Every file repeats the same `browseros` block (`base_cdp_port` 9010,
`base_server_port` 9110, `base_extension_port` 9310, `load_extensions`,
`headless`). Worker *N* adds *N* to each, so `workers: 3` needs 9010–9012,
9110–9112, 9310–9312 free. Two configs with the same ports cannot run
concurrently on one machine.

### CFG3 — Dataset paths are relative to the config file
Both formats resolve a non-`/`-prefixed dataset against the config's own
directory; the shipped files use `"../../data/<name>.jsonl"`. Moving a config
moves what it points at.

### CFG4 — API keys are env var names
`apiKey` holds an `ALL_CAPS` name (e.g. `OPENROUTER_API_KEY`), resolved at run
time by `src/utils/resolve-env.ts`; `src/utils/config-validator.ts` warns when
the variable is unset. Never inline a key. Suite files deliberately carry no
key at all — the model comes from `--variant` and `EVAL_AGENT_*`.

### CFG5 — `server_url` is informational for the worker pool
`browseros.server_url` is the base; `TaskWorkerPool` overrides it per worker
with `appManager.getServerUrl()`. It still must be a valid URL
(`z.string().url()`) because the schema validates it.

### CFG6 — `graders` and the dataset's per-task `graders` are both real
`configs/**.json` declares the default grader set; each line in the referenced
`data/*.jsonl` carries its own `graders` array, and that per-task list is what
the pipeline runs. Changing only the config does not change what a given task
is graded by.

### CFG7 — Don't run two configs at once on one host
See CFG2. The dashboard port (9900) is also shared, and `runEval` starts it
unconditionally.

## Workflows

### Running one
```bash
cd apps/eval
bun run eval suite --suite configs/suites/agisdk-daily-10.json --variant local
bun run eval run     --config configs/legacy/browseros-agent-weekly.json
```
Output lands in `results/<config>/<UTC timestamp>/` unless `output_dir` is set.

### Adding a suite for a new dataset
1. Put the dataset in `../../data/` (see `../data/AGENTS.md`).
2. Copy `configs/suites/agisdk-daily-10.json`, set `id`, `dataset`, `graders`.
3. Keep the `browseros` block and pick a free `timeoutMs` (≤ 3 600 000).
4. Run it with `run` first, then `suite --publish r2` once the numbers look right.

## Cross-references

- [`../src/suites/AGENTS.md`](../src/suites/AGENTS.md) — the suite schema and variant resolution.
- [`../src/types/AGENTS.md`](../src/types/AGENTS.md) — the legacy `EvalConfigSchema`.
- [`suites/AGENTS.md`](suites/AGENTS.md) — the current-format files.
- [`legacy/AGENTS.md`](legacy/AGENTS.md) — the config-era files.
- [`../data/AGENTS.md`](../data/AGENTS.md) — the datasets these configs point at.
- [`../.env.example`](../.env.example) — the variables the `api_key_env`/`apiKey` names refer to.
- [`../src/AGENTS.md`](../src/AGENTS.md) — harness map.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
