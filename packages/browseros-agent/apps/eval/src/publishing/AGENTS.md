# `src/publishing/` — R2 upload

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; ships a finished run to the public viewer.

## What's here

`r2-publisher.ts` uploads a completed run directory to Cloudflare R2 (S3
API), builds the `viewer-manifest.json` that the static viewer reads, and
prints the viewer URL. It can publish a single run dir or every timestamped run
under a config dir. `r2-manifest.ts` holds the small type surface and aliases
the viewer manifest types.

```
publishing/
├── r2-publisher.ts  ← R2Publisher, publishPathToR2, loadR2ConfigFromEnv, contentTypeForPath
└── r2-manifest.ts   ← R2UploadConfig, R2RunManifest, R2PublishRunResult
```

## Rules

### PB1 — Credentials come from `EVAL_R2_*` env vars
`loadR2ConfigFromEnv` requires `EVAL_R2_ACCOUNT_ID`,
`EVAL_R2_ACCESS_KEY_ID`, `EVAL_R2_SECRET_ACCESS_KEY` and throws a message
naming all three when any is missing. Defaults: bucket `browseros-eval`, CDN
base `https://eval.browseros.com` (trailing slashes trimmed). `accountId` is
accepted but the S3 endpoint is built from the access key/secret; the value is
carried for parity with the Cloudflare API.

### PB2 — Everything under the run dir is public
The publisher walks the run directory and uploads task artifacts, screenshots
and manifests. Before publishing, confirm the run dir contains no `.env`, no
credentials, and that the dataset itself is not embedded in the manifest. The
manifest is built from a `ViewerManifest` that deliberately omits
`artifactId` and raw provider keys (see `../suites/AGENTS.md`).

### PB3 — Content types are extension-driven
`contentTypeForPath` maps `.json`, `.jsonl` (as `application/x-ndjson`),
`.png`, `.html`, defaulting to `application/octet-stream`. A new artifact type
needs a mapping here or it downloads instead of rendering.

### PB4 — Idempotence is a feature
`publishPathToR2` distinguishes uploaded runs from `skippedRuns` (already
present) and the CLI prints `<runId>: already uploaded, skipping`. Re-running a
publish is safe; deleting the remote prefix is not something this code does.

### PB5 — Concurrency is bounded at 20
`DEFAULT_CONCURRENCY = 20` upload jobs at a time. A run with hundreds of
screenshots will otherwise open hundreds of sockets.

### PB6 — The viewer manifest is versioned
`../viewer/viewer-manifest.ts` exports `VIEWER_MANIFEST_SCHEMA_VERSION = 3` and
the R2 manifest is the same object. Bump the constant when the task entry
shape changes so a stale viewer is detectable.

## Workflows

### Publishing after a local run
```bash
bun run eval publish --run results/<config>/<timestamp> --target r2
# or, for every run under a config:
bun scripts/upload-run.ts results/<config>
```
The CDN URL for each run is printed on stdout.

### Adding a new artifact to the viewer
1. Write it into the task dir (see `../capture/AGENTS.md`).
2. Add its relative path to `ViewerManifestTaskPaths` in
   `../viewer/viewer-manifest.ts` and bump `VIEWER_MANIFEST_SCHEMA_VERSION`.
3. Add the extension to `CONTENT_TYPES`.
4. Extend `tests/publishing/r2-viewer-compat.test.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness artifact rules.
- [`../viewer/AGENTS.md`](../viewer/AGENTS.md) — the manifest being uploaded.
- [`../cli/commands/AGENTS.md`](../cli/commands/AGENTS.md) — the `publish` command.
- [`../../scripts/AGENTS.md`](../../scripts/AGENTS.md) — `upload-run.ts`, `weekly-report.ts`.
- [`../../../tests/publishing/AGENTS.md`](../../tests/publishing/AGENTS.md) — publisher tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
