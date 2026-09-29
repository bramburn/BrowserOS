# `apps/eval/` — Eval / benchmark harness

> Index page for the eval app. The detail lives in the 44 per-folder
> `AGENTS.md` files listed under [Defer to the folder
> file](#defer-to-the-folder-file); this page states only what is true of the
> app as a whole. Do not duplicate a folder's rules here — update that folder's
> file instead.
> Parent: [`../../AGENTS.md`](../../AGENTS.md) ·
> [`../AGENTS.md`](../AGENTS.md) ·
> cross-repo map: [`../../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md).

## What's here

A benchmark harness that runs an LLM agent against a real BrowserOS stack
(Chrome + Bun MCP server) over a dataset of tasks, then scores each task with
registered graders and publishes the run for a static viewer. It is a
standalone Bun app with its own graders: nothing under `apps/eval/` imports
the server's ACL scorer or monitoring judge.

```
apps/eval/
├── AGENTS.md                        ← this index
├── package.json                     ← scripts: eval, test, typecheck
├── tsconfig.json, .env.example, .gitignore
├── README.md                        ← operator-facing notes
├── src/                             ← harness source (25 AGENTS.md files)
├── tests/                           ← mirrors src/ (13 AGENTS.md files)
├── configs/                         ← run configs, two JSON formats
├── data/                            ← benchmark task datasets (JSONL)
└── scripts/                         ← dataset builders + report generators
```

## Run

```bash
cd apps/eval
cp .env.example .env.development     # the `eval` script reads this via --env-file

bun run eval suite --suite configs/suites/agisdk-daily-10.json --variant local
bun run eval run     --config configs/legacy/browseros-agent-weekly.json
bun run eval grade   --run results/<config>/<utc-timestamp>
bun run eval publish --run results/<config>/<utc-timestamp> --target r2
bun run eval -c configs/legacy/<config>.json    # legacy single-run form
```

`bun run eval` maps to `bun --env-file=.env.development run src/index.ts` in
this app's `package.json`; there is no monorepo-level `eval` script, so run it
from `apps/eval`. Tests: `bun test ./tests`, or `bun run test` to go through
the shared runner at `../../scripts/run-bun-test.ts`. There is no `start`
script.

## Opinionated rules

### EVX1 — This file is an index, not a second source of truth
Every directory below `apps/eval/` has its own `AGENTS.md` with the rules and
workflows for the code in it. A change that makes a folder doc wrong updates
that folder's file; it does not grow this page.

### EVX2 — There are two config formats, and `configs/suites/` is the forward one
`configs/suites/*.json` is validated by `EvalSuiteSchema`
(`src/suites/schema.ts`). `configs/legacy/*.json` is the older
`EvalConfigSchema` (`src/types/config.ts`) and is converted into the
suite/variant model by `src/suites/config-adapter.ts`. Both formats drive the
same runner. See [`configs/AGENTS.md`](configs/AGENTS.md).

### EVX3 — The CLI is `suite | run | grade | publish`, plus a legacy form
`bun run eval <command>`, with `-c <config.json>` (and a bare `bun run eval`
for the dashboard) as the legacy entry point. `src/cli/args.ts` keeps the four
commands in a `COMMANDS` set and treats a first token that is not in it as the
legacy form. See [`src/cli/AGENTS.md`](src/cli/AGENTS.md).

### EVX4 — `data/` is benchmark task input, not recorded trajectories
Each line of a `data/*.jsonl` file is one task matching `TaskSchema`
(`query_id`, `dataset`, `query`, optional `graders` / `start_url` /
`setup_script`, plus a `metadata` bag). A run *produces* trajectories under its
output dir; `data/` only *consumes* task inputs. See
[`data/AGENTS.md`](data/AGENTS.md).

### EVX5 — Agents are the system under test, and they drive the real stack
`src/agents/` implements single, orchestrator-executor and claude-code
evaluators. Each runs a real BrowserOS instance over CDP and the real MCP
endpoint; the unit tests never launch one. See
[`src/AGENTS.md`](src/AGENTS.md).

## Defer to the folder file

| Folder | File | Owns |
| --- | --- | --- |
| `src/` | [`src/AGENTS.md`](src/AGENTS.md) | harness map, run layout, `runs/` vs `runner/`, grader registration |
| `src/cli/` | [`src/cli/AGENTS.md`](src/cli/AGENTS.md) | argv parsing, dispatch, usage text |
| `src/cli/commands/` | [`src/cli/commands/AGENTS.md`](src/cli/commands/AGENTS.md) | the four command implementations |
| `src/agents/` | [`src/agents/AGENTS.md`](src/agents/AGENTS.md) | agent context, timeouts, screenshot join key |
| `src/agents/claude-code/` | [`src/agents/claude-code/AGENTS.md`](src/agents/claude-code/AGENTS.md) | `claude` binary, argv, MCP config, stream parsing |
| `src/agents/orchestrated/` | [`src/agents/orchestrated/AGENTS.md`](src/agents/orchestrated/AGENTS.md) | orchestrator loop, delegation budget |
| `src/agents/orchestrated/backends/` | [`backends/AGENTS.md`](src/agents/orchestrated/backends/AGENTS.md) | executor backend contract |
| `src/agents/orchestrated/backends/clado/` | [`clado/AGENTS.md`](src/agents/orchestrated/backends/clado/AGENTS.md) | Clado visual action executor |
| `src/agents/orchestrated/backends/tool-loop/` | [`tool-loop/AGENTS.md`](src/agents/orchestrated/backends/tool-loop/AGENTS.md) | plain tool loop |
| `src/agents/orchestrator-executor/` | [`AGENTS.md`](src/agents/orchestrator-executor/AGENTS.md) | executor-typed agent variant |
| `src/suites/` | [`src/suites/AGENTS.md`](src/suites/AGENTS.md) | `EvalSuiteSchema`, variant resolution, legacy adapter |
| `src/runs/` | [`src/runs/AGENTS.md`](src/runs/AGENTS.md) | `runEval`, worker pool, per-task pipeline, artifact paths |
| `src/runner/` | [`src/runner/AGENTS.md`](src/runner/AGENTS.md) | legacy re-export shims + real process/port code |
| `src/grading/` | [`src/grading/AGENTS.md`](src/grading/AGENTS.md) | grader registry, precedence, artifacts |
| `src/graders/` | [`src/graders/AGENTS.md`](src/graders/AGENTS.md) | concrete grader classes |
| `src/graders/benchmark/` | [`AGENTS.md`](src/graders/benchmark/AGENTS.md) | agisdk / infinity / webbench graders |
| `src/graders/performance/` | [`AGENTS.md`](src/graders/performance/AGENTS.md) | performance grader |
| `src/graders/python/` | [`AGENTS.md`](src/graders/python/AGENTS.md) | Python helper scripts |
| `src/publishing/` | [`src/publishing/AGENTS.md`](src/publishing/AGENTS.md) | R2 upload, env credentials |
| `src/reporting/` | [`src/reporting/AGENTS.md`](src/reporting/AGENTS.md) | per-task/per-run metrics, summaries |
| `src/viewer/` | [`src/viewer/AGENTS.md`](src/viewer/AGENTS.md) | `viewer-manifest.json` contract |
| `src/dashboard/` | [`src/dashboard/AGENTS.md`](src/dashboard/AGENTS.md) | local Hono dashboard on 9900 |
| `src/capture/` | [`src/capture/AGENTS.md`](src/capture/AGENTS.md) | trajectory writing, message log, screenshots |
| `src/types/` | [`src/types/AGENTS.md`](src/types/AGENTS.md) | shared zod schemas |
| `src/utils/` | [`src/utils/AGENTS.md`](src/utils/AGENTS.md) | MCP client, env/provider config, timeouts |
| `configs/` | [`configs/AGENTS.md`](configs/AGENTS.md) | both config formats, ports, dataset paths |
| `configs/suites/` | [`AGENTS.md`](configs/suites/AGENTS.md) | the shipped suite files |
| `configs/legacy/` | [`AGENTS.md`](configs/legacy/AGENTS.md) | the shipped legacy configs |
| `data/` | [`data/AGENTS.md`](data/AGENTS.md) | task datasets, `TaskSchema` conformance |
| `data/webbench/` | [`AGENTS.md`](data/webbench/AGENTS.md) | raw benchmark CSVs |
| `scripts/` | [`scripts/AGENTS.md`](scripts/AGENTS.md) | dataset builders, upload/report scripts |
| `tests/` | [`tests/AGENTS.md`](tests/AGENTS.md) | test-tree conventions, DI seams, env injection |
| `tests/agents/` | [`AGENTS.md`](tests/agents/AGENTS.md) | agent unit tests |
| `tests/cli/` | [`AGENTS.md`](tests/cli/AGENTS.md) | argv + suite command tests |
| `tests/grading/` | [`AGENTS.md`](tests/grading/AGENTS.md) | registry, grader artifacts, python layout |
| `tests/publishing/` | [`AGENTS.md`](tests/publishing/AGENTS.md) | R2 publisher, viewer compat |
| `tests/reporting/` | [`AGENTS.md`](tests/reporting/AGENTS.md) | run summary, report script |
| `tests/runs/` | [`AGENTS.md`](tests/runs/AGENTS.md) | artifact paths, run manifest, pipeline compat |
| `tests/suites/` | [`AGENTS.md`](tests/suites/AGENTS.md) | suite schema, config adapter |
| `tests/utils/` | [`AGENTS.md`](tests/utils/AGENTS.md) | provider config resolution |
| `tests/viewer/` | [`AGENTS.md`](tests/viewer/AGENTS.md) | viewer manifest |
| `tests/capture/` | [`AGENTS.md`](tests/capture/AGENTS.md) | captcha waiter |
| `tests/dashboard/` | [`AGENTS.md`](tests/dashboard/AGENTS.md) | dashboard auto-open |
| `tests/e2e/` | [`AGENTS.md`](tests/e2e/AGENTS.md) | manual, non-`*.test.ts` scripts |

## Workflows

### Adding a benchmark dataset
1. Write the tasks as JSONL under [`data/`](data/AGENTS.md), one `TaskSchema`
   object per line.
2. Add a run config under [`configs/suites/`](configs/suites/AGENTS.md) pointing
   at the file (`"dataset": "../../data/<name>.jsonl"`), and name the graders on
   the task lines.
3. If a Python builder can produce it, add a `scripts/build-*.py` instead of
   hand-editing — see [`scripts/AGENTS.md`](scripts/AGENTS.md).
4. Smoke it with `bun run eval run --suite …` before publishing with
   `bun run eval suite --suite … --publish r2`.

### Adding a grader
Follow [the `src/grading/` and `src/graders/` files](src/grading/AGENTS.md); the
one app-level constraint is that a grader is reachable by name from
`metadata.grader_results`, because that is what `grade` re-runs.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — `apps/` sibling map.
- [`src/AGENTS.md`](src/AGENTS.md) — the code under test.
- [`tests/AGENTS.md`](tests/AGENTS.md) — the test tree that mirrors it.
- [`configs/AGENTS.md`](configs/AGENTS.md) — the two config formats.
- [`data/AGENTS.md`](data/AGENTS.md) — the datasets configs point at.
- [`scripts/AGENTS.md`](scripts/AGENTS.md) — the dataset builders.
- [`.env.example`](.env.example) — every variable the harness reads.
- [`README.md`](README.md) — operator-facing usage notes.
