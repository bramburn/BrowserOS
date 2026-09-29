# `src/graders/python/` — Python evaluator helpers

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/graders/`; third-party scoring code invoked from TS.

## What's here

Two standalone Python 3 scripts, each reading one JSON object from stdin and
writing one JSON object to stdout. They exist because the benchmark evaluators
(AGI SDK's `WebCloneEvaluator`, WebArena-Infinity's per-task `verify()`) are
Python, and the harness is TypeScript.
`agisdk-evaluate.py` is invoked by `../benchmark/agisdk-state-diff.ts` via
`EVAL_SCRIPT`; `infinity-evaluate.py` is resolved relative to
`import.meta.dir` in `../benchmark/infinity-state.ts`.

```
python/
├── agisdk-evaluate.py   ← WebCloneEvaluator(task_id).evaluate(env_state, model_response)
└── infinity-evaluate.py ← importlib-loads a verifier module and calls verify(server_url)
```

## Rules

### PY1 — stdout is the result channel, nothing else may touch it
Both scripts must emit exactly one JSON object on stdout. `agisdk-evaluate.py`
saves `sys.stdout`, points it at `sys.stderr` for the duration of the agisdk
evaluation (whose logger writes to stdout), then restores it. Any new `print`
in these files will corrupt the parse in
`../../grading/python-evaluator.ts` (`Failed to parse Python evaluator output`).

### PY2 — Failure is a value, not a traceback
Both scripts catch broadly and print
`{"reward": 0, "pass": false, "message": ...}`. A missing dependency
(`pip install agisdk`) is reported the same way, and the process still exits 0
so the TS side records a grade rather than a crash.

### PY3 — agisdk softening is opt-out
`_soft_string_match` re-marks a failed criterion as passed when `expected_value`
is a non-empty case-insensitive substring of `actual_value`, unless
`AGISDK_STRICT_STRINGS=1`. If all criteria pass after softening, the reward is
forced to 1.0 and the message says how many were softened. This is a scoring
policy decision — change it deliberately and record it.

### PY4 — The input contract is small and stable
`agisdk-evaluate.py`: `{ task_id, env_state, model_response }` → 
`{ reward, pass, message, per_criterion[] }`, where each criterion is
`{ passed, detail, softened? }`. `infinity-evaluate.py`:
`{ app_server_url, verifier_path }` → `{ pass, reward, message }`.

### PY5 — Verifiers are loaded by path, not imported
`infinity-evaluate.py` uses `importlib.util.spec_from_file_location` and calls
`verify(server_url) -> (passed, message)`. A verifier without a callable
`verify` raises an `AttributeError` that names the module's public names — keep
that diagnostic.

## Workflows

### Testing a verifier change without a full run
1. Start the app server (the harness does this per task via
   `../../runner/infinity-app-manager.ts`).
2. `echo '{"app_server_url": "http://localhost:8000", "verifier_path": "…/verify.py"}' | python infinity-evaluate.py`
3. Re-grade the task: `bun run eval grade --run <outputDir>`.

### Making agisdk strict for a comparison run
Set `AGISDK_STRICT_STRINGS=1` in the env of the run; `grader-artifacts/…/evaluator-output.json`
records whether softening applied, so strict and lenient runs are
distinguishable after the fact.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — grader registry, the python bridge, artifact names.
- [`../benchmark/AGENTS.md`](../benchmark/AGENTS.md) — the callers of these scripts.
- [`../../grading/AGENTS.md`](../../grading/AGENTS.md) — `runPythonJsonEvaluator` (spawn, timeout, parse).
- [`../../../../data/AGENTS.md`](../../../data/AGENTS.md) — datasets whose `verifier_path` these scripts load.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — Bun monorepo parent.
