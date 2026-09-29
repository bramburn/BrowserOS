# `packages/build-tools/scripts/common/` — R2 S3 client helpers

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/build-tools`.

## What's here

The single S3/R2 layer for the build-tools package: `r2.ts`. It builds a
Cloudflare R2 `S3Client` from env credentials, resolves the bucket name, and
uploads a file either as a single `PutObject` or as a multipart upload,
retrying failed sends with a freshly created read stream. No other file lives
here.

## Contents

```
build-tools/scripts/common/
└── r2.ts   ← required(), createR2Client, getBucket, putFile, putFileMultipart,
              putObjectFromFile, uploadPartFromFile, destroyReadStream, sendWithRetry
```

## Rules

**RC1 — Credentials only via `required()`.** `R2_ACCOUNT_ID`,
`R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY` (and `R2_BUCKET` via `getBucket`)
throw `missing env var: <name>` when absent or blank. Never add a default.

**RC2 — Region is the literal `'auto'`** with endpoint
`https://<accountId>.r2.cloudflarestorage.com`. That is R2's S3-compat mode;
don't substitute a real region.

**RC3 — Replayability is the whole retry design.** `sendWithRetry` takes a
zero-arg factory, not a promise, because a consumed `fs.ReadStream` body
cannot be resent. Every `client.send` call inside it must be wrapped in a
closure that creates a *new* stream.

**RC4 — Destroy streams on every failure path.** `putObjectFromFile` and
`uploadPartFromFile` both `destroy()` in `catch` and await `close` via
`destroyReadStream`. Skipping this leaks file descriptors on flaky uploads.

**RC5 — Multipart must abort on error.** The `catch` in `putFileMultipart`
issues `AbortMultipartUploadCommand` (errors swallowed) before rethrowing, so
a partial upload does not accumulate in the bucket.

**RC6 — Keep the 5 MiB floor.** `putFileMultipart` rejects
`partSizeBytes < MIN_MULTIPART_PART_SIZE_BYTES`. S3 itself enforces it.

**RC7 — Types-only exports stay here.** `PutFileOptions` is the public
surface callers pass in; the internals below it are unexported on purpose.

## Workflows

**Retrying with different sizes:** call
`putFile(client, bucket, key, path, contentType, { multipartThresholdBytes,
partSizeBytes, maxAttempts, retryDelayMs })`. Delay is linear —
`retryDelayMs * attempt` (default 1000 ms base, 3 attempts).

**Adding a download counterpart:** add `downloadObjectToFile` beside the
upload helpers here rather than in the entrypoint, and mirror the
stream-cleanup discipline above. (`scripts/build/server/r2.ts` in this repo
already has its own separate download helper — that is a different code path,
not this one.)

**Testing a change:** `bun test packages/build-tools/tests/r2.test.ts` from
`packages/browseros-agent`. `FakeS3Client` in that test fails the first
`PutObject` and the first multipart part, which is exactly the behaviour that
must keep working.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-tools scope (rules BT1–BT6).
- [`../upload-to-r2.ts`](../upload-to-r2.ts) — the only caller.
- [`../../tests/r2.test.ts`](../../tests/r2.test.ts) — `FakeS3Client` tests.
- [`../../../../scripts/build/server/AGENTS.md`](../../../../scripts/build/server/AGENTS.md)
  — the separate R2 client used by the server artifact pipeline.
