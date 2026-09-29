# `tests/runs/` — Run-orchestration tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

Three tests over the suite-era layout helpers in `src/runs/`.
`artifact-paths.test.ts` pins `createRunId` (path-safe segment, `__` joiners,
UTC timestamp) and five of the paths returned by `getRunPaths`.
`run-manifest.test.ts` covers `buildRunManifest`. `pipeline-compat.test.ts` guards
the legacy alias surface in `src/runner/` — that `ParallelExecutor`,
`TaskExecutor` and `runEval` still resolve to the `runs/` implementations.

```
tests/runs/
├── artifact-paths.test.ts  ← run ids, run dir, task dir, key artifact paths
├── run-manifest.test.ts    ← buildRunManifest
└── pipeline-compat.test.ts← runner/* re-exports still point at runs/*
```

## Rules

### TRS1 — Path assertions pin the names that matter
`getRunPaths` returns 12 properties
(`src/runs/artifact-paths.ts:31-45`); `artifact-paths.test.ts:20-34` asserts 5 of
them — `runDir`, `runManifest`, `viewerManifest`, `messages` and
`graderArtifacts`: one run dir, two run-level manifests, two per-task paths.
Those names are mirrored by the publisher and the viewer manifest, so a silent
rename is the failure mode this file exists to prevent.

### TRS2 — Timestamps are UTC and minute-resolution
`createRunId` uses `YYYY-MM-DD-HHMM` in UTC. `src/runs/eval-runner.ts` and
`src/reporting/run-summary.ts` (`extractConfigName`) use the same format. Change
one and all three must change together.

### TRS3 — `pipeline-compat.test.ts` is a deprecation alarm
It exists because `src/dashboard/server.ts` still imports through the
`src/runner/` shims. When you finally move that import to `src/runs/`, delete
the shims and this test in the same commit — don't leave a test guarding a layer
nothing uses.

### TRS4 — Neither test may start a browser
`TaskWorkerPool` and `TaskRunPipeline` are not unit tested: they spawn Chrome
and a Bun server. Keep these tests on the pure helpers; integration coverage is
a real run.

## Workflows

### Moving a module from `runner/` to `runs/`
1. Move the file and update importers (search for
   `from '../runner/…'` and `from './runner/…'`).
2. If the old name must keep working, add/keep a re-export shim in
   `src/runner/` — never both a copy and a shim.
3. Update `pipeline-compat.test.ts` to the new mapping.

### Adding a new artifact to the run layout
1. Add it to `getRunPaths` in `src/runs/artifact-paths.ts`.
2. Add the assertion to `artifact-paths.test.ts`.
3. Make the writer actually produce it (`src/capture/trajectory-saver.ts` or
   `src/runs/run-manifest.ts`) and the uploader include it
   (`src/publishing/r2-publisher.ts`).

## Cross-references

- [`../../src/runs/AGENTS.md`](../../src/runs/AGENTS.md) — the folder under test.
- [`../../src/runner/AGENTS.md`](../../src/runner/AGENTS.md) — the legacy shim layer.
- [`../../src/viewer/AGENTS.md`](../../src/viewer/AGENTS.md) — the same path names, from the viewer side.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
