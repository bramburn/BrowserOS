# `src/graders/benchmark/` — State-diff graders

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/graders/`; deterministic, browser-backed pass/fail.

## What's here

Two graders that ask the *app under test* for its own state and diff it against
an expected state using third-party Python evaluators.
`agisdk-state-diff.ts` (`AgisdkStateDiffGrader`, name `agisdk_state_diff`) reads
the AGI SDK / REAL Bench `/finish` page and scores it with
`../python/agisdk-evaluate.py`. `infinity-state.ts` (`InfinityStateGrader`,
name `infinity_state`) calls a per-task verifier shipped with the
WebArena-Infinity app, via `../python/infinity-evaluate.py`.

```
benchmark/
├── agisdk-state-diff.ts  ← AgisdkStateDiffGrader
└── infinity-state.ts     ← InfinityStateGrader
```

## Rules

### BM1 — The task id is the entire addressing scheme
`AgisdkStateDiffGrader.extractTaskId` strips a leading `agisdk-`; the start URL
is then `https://evals-<siteId>.vercel.app` with `siteId` =
`taskId.replace(/-\d+$/, '')` (so `fly-unified-5` resolves to `fly-unified`).
If that derivation fails it falls back to scanning the message text for the
first `https://….vercel.app` URL, then gives up. `InfinityStateGrader` uses
`^infinity-(.+)-(task_.+)$`. Changing either regex changes which tasks can be
graded at all.

### BM2 — agisdk grades from the `/finish` page, not from local state
The grader navigates the live browser to `<origin>/finish`, polls
`evaluate_script` up to 20 times for a `<pre>` whose text starts with `{`, and
throws `No text content returned from /finish page` otherwise. This is why the
browser must still be alive at grading time — grading after the browser is
torn down will not reproduce.

### BM3 — The MCP endpoint must be in `GraderInput`
`GraderInput.mcpUrl` is optional in the type but required in practice here;
`runs/task-run-pipeline.ts` passes `${server_url}/mcp`. Missing URL → score 0
with a reasoning string, never a throw.

### BM4 — infinity needs `WEBARENA_INFINITY_DIR` and a live app server
`InfinityAppManager` (in `../../runner/`) starts the app before the task and
passes its URL as `input.infinityAppUrl`; `INFINITY_APP_URL` is the manual
fallback. The verifier path comes from
`metadata.additional.verifier_path`, resolved under `WEBARENA_INFINITY_DIR`.
Missing env → score 0, not a crash.

### BM5 — Write artifacts before and after evaluation
Both graders call `writeGraderJsonArtifact` for their inputs
(`context.json` / `verifier.json` + evaluator input) and their outputs before
returning. The agisdk grader additionally writes `finish-state.json`, which the
viewer manifest references by exact path — do not rename it.

## Workflows

### Running only the benchmark graders
Put `["agisdk_state_diff"]` (or `["infinity_state"]`) in a suite's `graders`
array. The per-task `graders` field in `data/*.jsonl` is what
`runs/task-run-pipeline.ts` actually runs.

### Handling an AGI SDK "soft pass"
`../python/agisdk-evaluate.py` re-marks a failed criterion as a soft pass when
`expected_value` is a case-insensitive substring of `actual_value`, unless
`AGISDK_STRICT_STRINGS=1`. Softened criteria are flagged in the artifact and in
the message ("Task passed (with N softened string criterion/criteria)"); the
score becomes 1.0 only when every criterion passes.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — grader registry, artifacts, and error rules.
- [`../grading/AGENTS.md`](../../grading/AGENTS.md) — `runPythonJsonEvaluator`, artifact paths.
- [`python/AGENTS.md`](../python/AGENTS.md) — the scripts these graders spawn.
- [`../../runner/AGENTS.md`](../../runner/AGENTS.md) — `InfinityAppManager` and the app server.
- [`../../../../data/AGENTS.md`](../../../data/AGENTS.md) — the dataset files these ids come from.
- [`../../../../../tests/grading/AGENTS.md`](../../../tests/grading/AGENTS.md) — artifact tests.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — Bun monorepo parent.
