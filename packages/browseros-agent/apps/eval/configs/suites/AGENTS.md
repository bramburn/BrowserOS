# `configs/suites/` — Suite configs (current format)

> Part of [`../AGENTS.md`](../AGENTS.md) in `configs/`; validated by `src/suites/schema.ts`.

## What's here

Three suite files, all targeting the AGI SDK / REAL Bench datasets, all using
the `single` (tool-loop) agent with the `agisdk_state_diff` grader. They differ
only in dataset, task count, and whether the browser restarts per task.

```
configs/suites/
├── agisdk-daily-10.json    ← id "agisdk-daily-10", dataset ../../data/agisdk-daily-10.jsonl,
│                             workers 1, restartBrowserPerTask true, timeoutMs 1800000
├── agisdk-real-smoke.json  ← small smoke set, same shape
└── agisdk-real.json        ← the full agisdk set
```

## Rules

### CS1 — These are the reference shape of a suite
`id` + `dataset` are required; `graders`, `workers`, `restartBrowserPerTask`
have defaults; `agent.type` is `single` here (the runner calls this
`tool-loop`); `timeoutMs` is 1 800 000 (30 min, the max allowed).

### CS2 — No API key lives in a suite
The model is supplied at run time: `--variant <id>` plus `EVAL_AGENT_PROVIDER`,
`EVAL_AGENT_MODEL`, `EVAL_AGENT_API_KEY`, `EVAL_AGENT_BASE_URL`. That is what
lets the same file be run against several providers and compared in the viewer.
`src/suites/resolve-variant.ts` throws `EVAL_AGENT_MODEL is required` if the
model is missing.

### CS3 — `browseros` is required at run time even though the schema allows omission
`src/cli/commands/suite.ts` `ensureRunnableSuite` throws
`suite browseros config is required to run suite commands`. Keep the block in
every suite.

### CS4 — The port block is a shared, exclusive resource
`base_cdp_port` / `base_server_port` / `base_extension_port` (9010 / 9110 /
9310) are incremented per worker; suite runs on the same host cannot overlap.
The dashboard also binds 9900.

### CS5 — `restartBrowserPerTask: true` costs time and buys isolation
It is what the benchmark suites use: each task gets a fresh profile and a fresh
server, so one task's login state cannot leak into the next. Turn it off only
when measuring steady-state agent behaviour rather than task correctness.

## Workflows

### Running one
```bash
cd apps/eval
bun run eval suite --suite configs/suites/agisdk-daily-10.json --variant local
bun run eval run   --suite configs/suites/agisdk-daily-10.json --variant local  # no publish
```

### Adding a suite
1. Copy `agisdk-daily-10.json`.
2. Set `id` (it becomes part of the run id via `createRunId` and the
   `results/<config>/` path), point `dataset` at `../../data/<file>.jsonl`.
3. Keep `browseros`; adjust `workers` and `timeoutMs` if the dataset is larger.
4. Run with `run` first, then `suite --publish r2`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — config-format rules shared with `legacy/`.
- [`../../src/suites/AGENTS.md`](../../src/suites/AGENTS.md) — the schema these files satisfy.
- [`../../src/AGENTS.md`](../../src/AGENTS.md) — harness map.
- [`../../data/AGENTS.md`](../../data/AGENTS.md) — the referenced datasets.
- [`../../.env.example`](../../.env.example) — the `EVAL_*` variables a variant reads.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
