# `tests/integration/` — VM and container smoke tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

One file: `vm-smoke.test.ts`, the only test in the suite that needs the
sandboxing toolchain installed on the host. It exercises `ContainerCli`
(`../../../../src/lib/container/`), `LimaCli`, and `VmRuntime`
(`../../../../src/lib/vm/`) together — the one place the VM layer and the
container layer are tested as a pair, because in production the container runs
*inside* the VM.

## Contents

| File | Covers |
|---|---|
| `vm-smoke.test.ts` | `LimaCli`, `VmRuntime`, `ContainerCli` — binary presence, VM start/stop, and container execution inside the VM. Uses `existsSync` / `mkdtemp` / `rm` / `stat` against the real toolchain. |

## Rules

**IN1 — This group is environment-gated, not hermetic.** It requires `limactl`
(and, depending on the build, `podman` — see the WS3→WS6 caveat in
`CLAUDE.md`). It is the only group that cannot run in a bare checkout. Skip
locally when the toolchain is absent rather than trying to mock it.

**IN2 — The unit tests for both layers already exist and are offline.**
`../lib/vm/` and `../lib/container/` use `fake-limactl.ts` and temp
directories. This smoke test is for verifying the *real* binaries, not for
covering logic. Any new VM or container behaviour gets its unit test there
first.

**IN3 — Clean up the VM you create.** The test creates temp dirs and VM
instances; teardown must remove both. A leaked Lima VM holds disk and a
bind mount that later tests trip over.

**IN4 — `server.integration.test.ts` is in this group, not this folder.**
`../__helpers__/run-test-group.ts` groups it under `integration` explicitly
and excludes it from the `root` group. Don't move it.

## Workflows

**Running:** `bun run test:integration` from `apps/server/`.

**Before running:** confirm the sandbox toolchain is present —
`limactl` from `resources/bin/third_party/`, and check whether `podman` or
`limactl` is the expected container binary for your build.

**Adding an integration case:** put it here only if it needs real external
binaries or a real network peer. Anything testable with a fake belongs in
`../lib/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../../../src/lib/vm/AGENTS.md`](../../src/lib/vm/AGENTS.md) — the Lima layer.
- [`../../../../src/lib/container/AGENTS.md`](../../src/lib/container/AGENTS.md) — the podman layer.
- [`../../../../CLAUDE.md`](../../../../CLAUDE.md) — the `limactl` vs `podman` release-gating note.
