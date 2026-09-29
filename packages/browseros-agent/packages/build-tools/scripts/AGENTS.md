# `packages/build-tools/scripts/` — R2 upload entrypoints

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/build-tools`. Monorepo parent:
> [`../../../AGENTS.md`](../../../AGENTS.md).

## What's here

Two files that make up the whole upload surface of `@browseros/build-tools`.
`upload-to-r2.ts` is the executable entry (`bun run upload`, i.e. the
package's `upload` script) and does nothing but parse `--file` / `--key` /
`--content-type` and delegate. All S3 logic lives in `common/r2.ts`.

## Contents

```
build-tools/scripts/
├── upload-to-r2.ts   ← shebang `#!/usr/bin/env bun`; parseArgs, putFile, client.destroy()
└── common/
    └── r2.ts         ← createR2Client, getBucket, putFile + multipart/retry internals
```

## Rules

**SC1 — `upload-to-r2.ts` stays a thin shim.** Argument parsing, one
`putFile` call, and `client.destroy()` in `finally`. No retry logic, no
credential handling, no S3 imports in this file.

**SC2 — Use `node:util` `parseArgs`, not commander.** The package's own
`package.json` declares no `commander` dependency; the CLI uploader elsewhere
in the monorepo uses commander, this one deliberately does not.

**SC3 — Always `client.destroy()` in a `finally`.** A leaked S3 client keeps
the Bun process alive and hangs `bun run upload`.

**SC4 — `--file` and `--key` are mandatory; throw otherwise.** The script
throws `new Error('--file and --key required')` before constructing a client,
so a bad invocation never reaches the network.

**SC5 — Default content type is `application/octet-stream`.** Release
binaries must not be served as text.

## Workflows

**Uploading one file:** `bun --filter @browseros/build-tools run upload --
--file <local-path> --key <r2-key> [--content-type <type>]`. Success prints
`uploaded <file> to <bucket>/<key>`.

**Adding a second entrypoint (e.g. a delete or a listing command):** 1. New
`bun`-shebang file in this directory. 2. Reuse `createR2Client` / `getBucket`
from `common/r2.ts` — do not build a second S3 client. 3. Wrap in
`try/finally { client.destroy() }`. 4. Add an npm script to
`../package.json` and a README line.

**Changing multipart behaviour:** edit `common/r2.ts` only, and extend
`../tests/r2.test.ts` — the `FakeS3Client` there already exercises both a
failed `PutObject` and a failed first multipart part.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-tools scope (rules BT1–BT6).
- [`common/AGENTS.md`](common/AGENTS.md) — the R2 client implementation.
- [`../README.md`](../README.md) — what gets uploaded (Lima, Bun runtimes).
- [`../../../../scripts/build/AGENTS.md`](../../../scripts/build/AGENTS.md)
  — the build pipeline that reads the objects this uploads.
