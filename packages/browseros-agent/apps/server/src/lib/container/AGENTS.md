# `src/lib/container/` — podman container layer

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/`.

## What's here

The container backend for sandboxed agent adapters. `container-cli.ts` wraps
the `podman` binary; `image-loader.ts` resolves and prepares images;
`types.ts` holds the shared value types; `managed/` contains the abstract
`ManagedContainer` base that the Hermes container runtime extends. Note the
release caveat in `CLAUDE.md`: between the Lima cutover (WS3) and the
ContainerRuntime migration (WS6), `resources/bin/third_party/` ships
`limactl` instead of `podman`, so `container-cli.ts` cannot find its binary on
builds cut from `dev`.

## Contents

| File | Purpose |
|---|---|
| `container-cli.ts` | `ContainerCli` — the `podman` invocation wrapper (run, exec, ps, rm). |
| `image-loader.ts` | Resolves the image to use and loads it if absent. |
| `types.ts` | `LogFn` and the shared container value types. |
| `index.ts` | Barrel: `container-cli`, `image-loader`, `managed`, `types`. |
| `managed/` | `ManagedContainer` base + types + errors. See [`managed/AGENTS.md`](managed/AGENTS.md). |

## Contents (context)

The VM side of the same idea lives in [`../vm/`](../vm/AGENTS.md) (Lima).
`container/` is host-side `podman`; `vm/` is the Lima virtual machine the
containers run inside.

## Rules

**CN1 — `ManagedContainer` owns the lifecycle; subclasses own the spec.** The
abstract base in `managed/` implements the state machine, the lifecycle lock,
and the gated `execute*` family. A concrete container (e.g. Hermes) supplies
its descriptor and readiness probe and nothing else.

**CN2 — `container-cli.ts` is the only file that shells out to `podman`.** If
you need a new container operation, add it there so error handling and
argument quoting stay consistent.

**CN3 — Path safety is enforced at the mount boundary.** `PathOutsideMountsError`
exists because a container that can see the host filesystem can read anything
the server can. Validate every path against the declared `MountRoot` before
exec.

**CN4 — Don't cut a release branch from `dev` during the WS3→WS6 window.**
See `CLAUDE.md` § "Release gating — bundled-VM runtime migration". This is a
release-process constraint, not a code one, but it belongs here because
`container-cli.ts` is the code that breaks.

**CN5 — State transitions go through the base class.** `TRANSIENT_STATES` in
`managed/types.ts` is the set the base class polls; a subclass inventing its
own readiness signal desynchronises the UI's container status.

## Workflows

**Adding a container-backed agent adapter:** 1. Extend `ManagedContainer` from
`managed/managed-container.ts`. 2. Supply a `ContainerDescriptor` and a
readiness probe. 3. Extend `ContainerAgentRuntime` in
`../agents/runtime/container-agent-runtime.ts` and register the adapter.

**Testing:** `tests/lib/container/` covers `container-cli` and `image-loader`
offline; `managed/managed-container.test.ts` drives the base class state
machine without a real container.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/` conventions.
- [`./managed/AGENTS.md`](managed/AGENTS.md) — `ManagedContainer` base class.
- [`../vm/AGENTS.md`](../vm/AGENTS.md) — the Lima VM these containers run in.
- [`../../../../CLAUDE.md`](../../../../../CLAUDE.md) — the release-gating note on `limactl` vs `podman`.
