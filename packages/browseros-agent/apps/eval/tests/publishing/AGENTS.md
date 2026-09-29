# `tests/publishing/` — R2 publisher tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

Two tests over `src/publishing/` and the viewer manifest it depends on.
`r2-publisher.test.ts` drives `R2Publisher` with a fake `R2Client` (the
`R2PublisherOptions.client` seam) and a temp run directory, asserting on the
keys requested and the manifest produced. `r2-viewer-compat.test.ts` checks the
uploaded layout against `src/viewer/viewer-manifest.ts` — the compatibility
contract between what the harness writes and what the static viewer can read.

```
tests/publishing/
├── r2-publisher.test.ts      ← upload jobs, content types, manifest, skip/overwrite
└── r2-viewer-compat.test.ts  ← viewer-manifest paths vs. uploaded object keys
```

## Rules

### TPUB1 — Always inject the client; never touch R2
`R2PublisherOptions.client` replaces the S3 client. The tests must not make
network calls, and must not require `EVAL_R2_*` in the environment — pass
`loadR2ConfigFromEnv({...})` a literal env object.

### TPUB2 — Credentials are inputs, never assertions
Test fixtures use obviously fake keys. If a real key ever appears in this
folder, treat it as an incident: the publisher's output is public.

### TPUB3 — Path compatibility is the point of `r2-viewer-compat.test.ts`
It pins that every `ViewerManifestTaskPaths` entry corresponds to a key the
publisher actually uploads, and that the schema version matches. When you add
an artifact, this is the test that should fail first — update the manifest, the
content-type map and this test together.

### TPUB4 — Publish idempotence is asserted
`skippedRuns` (already uploaded) is part of the contract, as is
`uploadedFiles` counting. Don't "fix" a re-publish to overwrite silently.

## Workflows

### Adding an artifact to the published viewer
1. Write it into the task dir (`src/capture/AGENTS.md`).
2. Add its path to `ViewerManifestTaskPaths` + `taskPaths()` in
   `src/viewer/viewer-manifest.ts`; bump `VIEWER_MANIFEST_SCHEMA_VERSION`.
3. Add the extension to `CONTENT_TYPES` in `src/publishing/r2-publisher.ts`.
4. Update `r2-viewer-compat.test.ts` and `tests/viewer/viewer-manifest.test.ts`.

### Debugging a publish that half-succeeded
`R2Publisher` uploads with bounded concurrency (20) and reports counts rather
than a transaction. If a run is partially uploaded, re-running is safe: already
present objects are reported under `skippedRuns`.

## Cross-references

- [`../../src/publishing/AGENTS.md`](../../src/publishing/AGENTS.md) — the folder under test.
- [`../../src/viewer/AGENTS.md`](../../src/viewer/AGENTS.md) — the manifest contract.
- [`../../src/cli/commands/AGENTS.md`](../../src/cli/commands/AGENTS.md) — the `publish` command.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
