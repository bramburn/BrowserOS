# `packages/build-tools/` — release artifacts & Lima VM template

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

`@browseros/build-tools` is a private, non-published workspace package with
two jobs. `scripts/upload-to-r2.ts` (+ `scripts/common/r2.ts`) pushes release
files to Cloudflare R2 with automatic single-part vs multipart selection and
per-part retry. `template/browseros-vm.yaml` is the committed Lima template
that defines the BrowserOS VM — a rootless-containerd Ubuntu 24.04 minimal
image that `limactl` consumes directly, with no custom disk build step.
`tests/` covers both with `bun test`.

## Contents

```
build-tools/
├── package.json           ← @browseros/build-tools, private; upload / test / typecheck
├── tsconfig.json          ← extends ../../tsconfig.json, rootDir ".", includes scripts+tests
├── README.md              ← setup, Lima dev loop, Lima/Bun runtime upload commands
├── scripts/
│ ├── upload-to-r2.ts      ← CLI entry: --file, --key, --content-type
│ └── common/r2.ts         ← createR2Client, getBucket, putFile (multipart + retry)
├── template/
│ └── browseros-vm.yaml   ← Lima template: vz VM, 4 CPU / 4GiB / 20GiB, rootless containerd
└── tests/
    ├── r2.test.ts         ← FakeS3Client: failed PutObject + failed first multipart part
    └── vm-template.test.ts← asserts the Lima template's content and forbidden strings
```

## Rules

**BT1 — Credentials come from env, and only from env.** `scripts/common/r2.ts`
`required()` throws on a missing `R2_ACCOUNT_ID`, `R2_ACCESS_KEY_ID`,
`R2_SECRET_ACCESS_KEY`, or `R2_BUCKET`. Never add a default, a fallback, or a
committed value.

**BT2 — Multipart threshold is 64 MiB, part size is 64 MiB, 3 attempts.**
`putFile` switches to multipart above `DEFAULT_MULTIPART_THRESHOLD_BYTES`.
S3 rejects parts below 5 MiB, so keep `partSizeBytes` well above
`MIN_MULTIPART_PART_SIZE_BYTES`.

**BT3 — Retries must rebuild the body.** `sendWithRetry` re-invokes a command
*factory* because a consumed `ReadStream` cannot be replayed. Any new upload
path must do the same and must `destroy()` the stream on failure.

**BT4 — The Lima template is a contract, guarded by a test.** `tests/vm-template.test.ts`
asserts the template contains `system: false` / `user: true` containerd and a
`nerdctl` probe, and that it does *not* contain `podman`, `debian`,
`sudo nerdctl`, or `/var/run/containerd/containerd.sock`. Update the test in
the same change if a value genuinely has to move.

**BT5 — Keep `template/` honest.** The host forwards
`/run/user/{{.UID}}/containerd-rootless/containerd.sock` to
`{{.Dir}}/sock/containerd.sock`. Changing either end breaks the server's
`limactl` runtime; change them together.

**BT6 — No `index.ts`.** This package is referenced by path
(`@browseros/build-tools` has no `exports` map) and only the root
`package.json` scripts (`upload`, `test`, `typecheck`) are used.

## Workflows

**Uploading a vendor runtime binary:** 1. Build Chromium/upstream artifact.
2. `bun --filter @browseros/build-tools run upload -- --file <path> --key
third_party/<name> --content-type application/octet-stream`. 3. Add the
matching rule to `scripts/build/config/server-prod-resources.json`. 4. Ensure
`R2_DOWNLOAD_PREFIX=artifacts/vendor` in `apps/server/.env.production`.

**Changing the VM shape:** 1. Edit `template/browseros-vm.yaml`. 2. Run
`bun test tests/vm-template.test.ts` and update the assertions to match the
intended new shape. 3. Smoke test with `limactl start --name
browseros-vm-dev packages/browseros-agent/packages/build-tools/template/browseros-vm.yaml`.

**Debugging a failed large upload:** 1. Read the thrown error — it names the
bucket/key/part. 2. Check part size ≥ 5 MiB. 3. Re-run; multipart aborts are
cleaned up by the `AbortMultipartUploadCommand` path in the `catch`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`README.md`](README.md) — Lima dev loop and runtime upload commands.
- [`../shared/AGENTS.md`](../shared/AGENTS.md) — shared constants (dependency).
- [`../../scripts/build/AGENTS.md`](../../scripts/build/AGENTS.md) — the
  server artifact pipeline that consumes this template and these R2 keys.
