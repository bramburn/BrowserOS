# `tests/viewer/` — Viewer manifest tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

`viewer-manifest.test.ts` covers `buildViewerManifest` in
`src/viewer/viewer-manifest.ts`: the emitted `schemaVersion`, the per-task
`paths` block, the synthesis of `metrics` when a task has none, the run-level
`buildRunMetrics` aggregate, and the stripping of `artifactId` from the public
task entry.

```
tests/viewer/
└── viewer-manifest.test.ts   ← buildViewerManifest shape + path contract
```

## Rules

### TV1 — The path table is the assertion
`tasks/<id>/{attempt.json, metadata.json, messages.jsonl, trace.jsonl,
grades.json, screenshots, grader-artifacts,
grader-artifacts/agisdk_state_diff/finish-state.json}` must be asserted in
full. The publisher (`tests/publishing/r2-viewer-compat.test.ts`) mirrors these
names; a change here without a change there produces a viewer that renders a run
header and no tasks.

### TV2 — `VIEWER_MANIFEST_SCHEMA_VERSION` is part of the contract
Bump it with any task-entry shape change. The test should fail on an
unbumped change; do not update the constant to make a test pass without
changing the shape.

### TV3 — `artifactId` must never be published
It is an internal alias used for the path prefix. Assert that it is absent from
the built task object.

### TV4 — Optional metadata stays optional
`suiteId`, `variantId`, `uploadedAt`, `reportPath`, `agentConfig`, `dataset` and
`summary` are only emitted when supplied (spread-conditional in
`buildViewerManifest`). Keep a fixture without them so the minimal manifest
stays valid — the uploader must handle partial runs.

## Workflows

### Adding a per-task field the viewer shows
1. Add it to `ViewerManifestTaskInput` in `src/viewer/viewer-manifest.ts`.
2. Add a fixture value and an assertion here.
3. Confirm the value is not secret: this file is uploaded publicly (see rule
   VW5 in `src/viewer/AGENTS.md`).

### Checking a published run renders
```bash
bun run eval publish --run <outputDir> --target r2
```
Then open the printed CDN URL. If the run loads but tasks are empty, compare
`viewer-manifest.json` in the run dir against the keys actually uploaded.

## Cross-references

- [`../../src/viewer/AGENTS.md`](../../src/viewer/AGENTS.md) — the folder under test.
- [`../../src/publishing/AGENTS.md`](../../src/publishing/AGENTS.md) — the uploader of this manifest.
- [`../publishing/AGENTS.md`](../publishing/AGENTS.md) — the compatibility counterpart.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
