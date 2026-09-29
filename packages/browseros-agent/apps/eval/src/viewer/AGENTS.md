# `src/viewer/` — Viewer manifest builder

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the JSON index the static viewer reads.

## What's here

One file, `viewer-manifest.ts`. It defines the compact per-run JSON document
that the published viewer fetches instead of listing the bucket: run-level
identity, aggregate metrics, and one entry per task pointing at that task's
artifacts by relative path. `../publishing/r2-publisher.ts` builds it from a run
directory and uploads it next to the artifacts.

```
viewer/
└── viewer-manifest.ts  ← VIEWER_MANIFEST_SCHEMA_VERSION, ViewerManifest,
                          ViewerManifestTask(Input), buildViewerManifest
```

## Rules

### VW1 — Paths are relative and fixed
`taskPaths(queryId)` returns exactly:
`tasks/<id>/attempt.json`, `metadata.json`, `messages.jsonl`, `trace.jsonl`,
`grades.json`, `screenshots`, `grader-artifacts`, and
`grader-artifacts/agisdk_state_diff/finish-state.json`. The publisher mirrors
these under `runs/<runId>/`. A new artifact becomes visible only after being
added here (and matched by the uploader).

### VW2 — The schema version is the compatibility signal
`VIEWER_MANIFEST_SCHEMA_VERSION = 3`. Bump it whenever the task entry shape
changes; `tests/publishing/r2-viewer-compat.test.ts` and
`tests/viewer/viewer-manifest.test.ts` assert on it.

### VW3 — `artifactId` is an internal alias
A task entry may carry `artifactId` different from `queryId` (used when the
uploaded id differs from the dataset id). `buildViewerManifest` strips it from
the public object and uses it for the path prefix instead. It must never appear
in the published manifest.

### VW4 — Metrics are filled in, never omitted
If a task has no `metrics`, the builder synthesises them from
`screenshotCount` + `durationMs` (steps = screenshots), so the viewer never has
to special-case a missing block. Run-level `metrics` always come from
`buildRunMetrics` over the task metrics.

### VW5 — Secrets must not reach this file
The manifest is uploaded to a public CDN. It carries the *variant metadata*
(`apiKeyConfigured: boolean`, `baseUrlHost`) from
`../suites/resolve-variant.ts`, not the variant itself. Never spread a
resolved provider config into a manifest task.

## Workflows

### Adding a per-task artifact the viewer should show
1. Write the file into the task dir (see `../capture/AGENTS.md`).
2. Add the relative path to `ViewerManifestTaskPaths` + `taskPaths()` here, and
   a field on `ViewerManifestTaskInput`.
3. Ensure the uploader actually uploads that file (extension mapping in
   `../publishing/r2-publisher.ts`).
4. Bump `VIEWER_MANIFEST_SCHEMA_VERSION` and update
   `tests/viewer/viewer-manifest.test.ts`.

### Debugging a viewer that shows a run but no tasks
Compare the manifest's `tasks[].paths` against the uploaded prefix. The most
common cause is a run produced before the artifact was added, published with
the current schema version.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness artifact rules.
- [`../publishing/AGENTS.md`](../publishing/AGENTS.md) — the uploader of this manifest.
- [`../reporting/AGENTS.md`](../reporting/AGENTS.md) — `buildRunMetrics` embedded here.
- [`../suites/AGENTS.md`](../suites/AGENTS.md) — `publicMetadata` (the only agent info exposed).
- [`../dashboard/AGENTS.md`](../dashboard/AGENTS.md) — `viewer.html`, the local counterpart.
- [`../../../tests/viewer/AGENTS.md`](../../tests/viewer/AGENTS.md) — manifest tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
