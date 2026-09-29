# `tests/grading/` — Grader tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

Six tests over `src/graders/` and `src/grading/`. Three of them
(`agisdk-artifacts`, `infinity-artifacts`, `performance-artifacts`) assert the
**artifact layout** each grader must write into a task dir — the contract the
viewer and the re-grade path depend on. `grader-registry.test.ts` pins the name →
class mapping and the pass/fail precedence order. `python-evaluator.test.ts`
covers the JSON-in/JSON-out bridge. `python-script-layout.test.ts` checks the
two Python files exist and keep their docstring contract, without running them.

```
tests/grading/
├── grader-registry.test.ts       ← createGrader mapping + PASS_FAIL_GRADER_ORDER
├── agisdk-artifacts.test.ts      ← context.json / evaluator-input / evaluator-output / finish-state.json
├── infinity-artifacts.test.ts    ← verifier.json + evaluator artifacts
├── performance-artifacts.test.ts ← metrics.json + axes.json
├── python-evaluator.test.ts      ← runPythonJsonEvaluator (parse, non-zero exit, timeout)
└── python-script-layout.test.ts  ← script presence + CLI contract (no execution)
```

## Rules

### TG1 — Artifact filenames are the assertions
These tests exist because `src/viewer/viewer-manifest.ts` hard-codes
`tasks/<id>/grader-artifacts/agisdk_state_diff/finish-state.json` and
`src/capture/trajectory-saver.ts` expects `grades.json` alongside. Renaming a
grader artifact without updating the tests, the viewer manifest and the
uploader is a three-place change; the tests are how you notice you missed one.

### TG2 — Grade with a temp task dir and a synthetic `GraderInput`
`GraderInput` is plain data (`task`, `messages`, `screenshotCount`,
`finalAnswer`, `taskArtifactDir`, `outputDir`). Point `taskArtifactDir` at a
temp directory and assert on what appears under `grader-artifacts/<name>/`.

### TG3 — Don't execute the Python helpers here
`python-evaluator.test.ts` uses a trivial inline script; the real helpers need
`agisdk` / `webarena-infinity` installed. `python-script-layout.test.ts` checks
the files statically instead — extend it when you add a helper, and put real
behaviour in `src/graders/python/AGENTS.md` rather than in a flaky test.

### TG4 — Unknown grader names are not errors
`createGrader('nope')` warns and returns `null`, and `runConfiguredGraders`
skips it. Keep a test asserting that, so a typo'd grader in a config surfaces
as "no grades" rather than an exception.

## Workflows

### Adding a grader's artifacts
1. Write them through `src/grading/artifacts.ts`.
2. Add assertions to the matching `*-artifacts.test.ts`, or a new file following
   the same shape.
3. If the viewer must show them, add the path to
   `src/viewer/viewer-manifest.ts` and bump `VIEWER_MANIFEST_SCHEMA_VERSION`.

### Re-grading confidence check
`grader-registry.test.ts` should still list the same precedence order as
`src/grading/grader-registry.ts` **and** the duplicated array in
`src/reporting/run-summary.ts`. If the two copies ever disagree, the run summary
and the weekly report will report different pass rates.

## Cross-references

- [`../../src/graders/AGENTS.md`](../../src/graders/AGENTS.md) — the folder under test.
- [`../../src/grading/AGENTS.md`](../../src/grading/AGENTS.md) — registry, artifacts, python bridge.
- [`../../src/viewer/AGENTS.md`](../../src/viewer/AGENTS.md) — the consumer of these paths.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
