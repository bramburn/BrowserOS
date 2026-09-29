# `packages/build-tools/tests/` — build-tools test suite

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/build-tools`.

## What's here

Two `bun test` files, one per concern. `r2.test.ts` exercises the real
`putFile` from `../scripts/common/r2.ts` against a hand-written `FakeS3Client`
that fails the first `PutObject` and the first multipart part. `vm-template.test.ts`
reads `../template/browseros-vm.yaml` as text and asserts the properties the
server's Lima runtime depends on. No network, no credentials, no R2 account
required.

## Contents

```
build-tools/tests/
├── r2.test.ts          ← putFile retry/multipart behaviour via FakeS3Client
└── vm-template.test.ts ← browseros-vm.yaml content assertions (positive + negative)
```

## Rules

**TS1 — No real S3, ever.** Use `FakeS3Client`; `createR2Client()` needs live
R2 env vars and must never be called from a test.

**TS2 — Use `afterEach` cleanup for temp dirs.** Both files use
`mkdtemp(path.join(tmpdir(), '...'))` and `rm(dir, { recursive: true, force:
true })` with a null guard, so a failed assertion cannot leak a temp dir.

**TS3 — `vm-template.test.ts` asserts absences too.** The `expect(yaml).
not.toContain(...)` lines (`podman`, `debian`, `sudo nerdctl`,
`/var/run/containerd/containerd.sock`) are the point of the test — keep them
when you extend the positive assertions.

**TS4 — Cast the fake, don't widen the production type.** Tests pass
`client as unknown as S3Client` rather than changing `putFile`'s signature.

**TS5 — Test files live only here.** `bun test` from
`packages/browseros-agent` does not pick up this package; run
`bun --filter @browseros/build-tools run test` (which is `bun test` with
`tests/` as cwd) or `bun test packages/build-tools` from the monorepo root.

## Workflows

**Running these tests:** from `packages/browseros-agent`,
`bun test packages/build-tools`. Single file:
`bun test packages/build-tools/tests/vm-template.test.ts`.

**Adding a case for a new upload behaviour:** 1. Add a failing flag to
`FakeS3Client.send` (e.g. `failedComplete = true`). 2. Write the
`it(...)` in `r2.test.ts`. 3. Change `../scripts/common/r2.ts`. 4. Re-run.

**Changing the VM template:** run `vm-template.test.ts` first; treat any red
assertion as "the template and the server's expectations disagree", not as a
test to be edited — unless you *intend* to move the value, in which case
update both in one change (rule TM6 in `../template/AGENTS.md`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-tools scope (rules BT1–BT6).
- [`../template/AGENTS.md`](../template/AGENTS.md) — what the VM test guards.
- [`../scripts/common/AGENTS.md`](../scripts/common/AGENTS.md) — the code under test.
- [`../../shared/AGENTS.md`](../../shared/AGENTS.md) — shared constants.
