# `tests/lib/container/` — container layer tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/`.

## What's here

Three tests for `../../../../src/lib/container/`, all hermetic: the `podman`
invocation wrapper and the image loader are driven against a stubbed
transport, and the `ManagedContainer` state machine is exercised without a
real container. `managed/` mirrors the source subfolder.

## Contents

| File | Covers |
|---|---|
| `container-cli.test.ts` | `ContainerCli` — the `podman` invocation: argument construction, output parsing, error propagation. |
| `image-loader.test.ts` | `image-loader.ts` — resolving the image and loading it when absent. |
| `managed/managed-container.test.ts` | The `ManagedContainer` abstract base — state machine, lifecycle lock, gated `execute*`, listener notifications. See [`managed/AGENTS.md`](managed/AGENTS.md). |

## Rules

**CN-T1 — No real `podman`.** Stub the binary invocation; a test that
requires the container toolchain belongs in
[`../../integration/`](../../integration/AGENTS.md) (see `vm-smoke.test.ts`).
Note the WS3→WS6 release caveat in `CLAUDE.md`: `resources/bin/third_party/`
currently ships `limactl`, not `podman`, so builds cut from `dev` may not
have the binary at all.

**CN-T2 — The base-class test uses a minimal concrete subclass.**
`managed-container.test.ts` extends `ManagedContainer` with a tiny descriptor
so the shared lifecycle is covered once, for every future subclass.

**CN-T3 — `PathOutsideMountsError` must have a test.** Path escape is the
security boundary of the whole container layer; a case that attempts a path
outside the declared `MountRoot` is not optional.

**CN-T4 — State transitions are the point.** Assert the full transition
sequence (including `TRANSIENT_STATES` and the lock), not just the happy
path. The class exists to make those states deterministic.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Adding a container-backed adapter:** write the runtime test in
[`../agents/runtime/`](../agents/runtime/AGENTS.md) and extend
`managed/managed-container.test.ts` only if you changed the *base* class's
behaviour.

**Verifying against a real container runtime:** that's
[`../../integration/vm-smoke.test.ts`](../../integration/AGENTS.md).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/` conventions.
- [`../../../../src/lib/container/AGENTS.md`](../../../src/lib/container/AGENTS.md) — the module under test.
- [`./managed/AGENTS.md`](managed/AGENTS.md) — `ManagedContainer` state-machine tests.
- [`../../../../CLAUDE.md`](../../../../../CLAUDE.md) — the `limactl` vs `podman` release gate.
