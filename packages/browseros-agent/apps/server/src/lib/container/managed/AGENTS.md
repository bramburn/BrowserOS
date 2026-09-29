# `src/lib/container/managed/` — managed container base class

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/container/`.

## What's here

The abstract base every container-backed agent adapter sits on.
`managed-container.ts` owns the state machine, the lifecycle lock, and the
gated `execute*` family; `types.ts` holds the shared value types (kept
separate so the base-class file stays readable); `errors.ts` holds the typed
failures. `HermesContainerRuntime` in
[`../../agents/runtime/hermes-container-runtime.ts`](../../agents/runtime/hermes-container-runtime.ts)
is the one concrete subclass today.

## Contents

| File | Purpose |
|---|---|
| `managed-container.ts` | `Abstract ManagedContainer` — state machine, lifecycle lock, gated `execute*` methods, `StateListener` / `Unsubscribe` change notifications. Exports `ManagedContainer` and `ManagedContainerDeps`. |
| `types.ts` | `ContainerDescriptor`, `ContainerState`, `ContainerStatusSnapshot`, `ExecResult`, `ExecSpec`, `MountRoot`, `Platform`, `ResetLevel`, `ResetOptions`, and the `TRANSIENT_STATES` set. |
| `errors.ts` | `ContainerNotReadyError`, `PathOutsideMountsError`, `ResetNotSupportedError`. |
| `index.ts` | Barrel re-exporting the class, types, and errors. |

## Rules

**MC1 — Subclasses describe, they don't reimplement the lifecycle.** The base
class already gates execution on readiness. A subclass that spawns the
container itself has created a second, unsynchronised state machine.

**MC2 — Readiness is checked, not assumed.** `execProcess` / `execOneShot`
throw `ContainerNotReadyError` until the state leaves `TRANSIENT_STATES`.
Callers must handle it — that is the difference between "starting" and
"failed".

**MC3 — Every path is checked against `MountRoot`.** `PathOutsideMountsError`
is a security boundary, not a usability error: the container runs with the
server's privileges over anything it can mount.

**MC4 — `ResetLevel` is negotiated, not assumed.** A container that cannot
honour a requested reset level throws `ResetNotSupportedError`; don't silently
degrade to a partial reset.

**MC5 — State changes are broadcast via `StateListener`.** The base class
owns subscription/unsubscription. If a subclass mutates `ContainerState`
directly, listeners never fire and the container status in the UI goes stale.

**MC6 — Do not import `podman` here.** The binary invocation lives in
`../container-cli.ts`; this layer is transport-agnostic.

## Workflows

**Adding a new container type:** 1. Extend `ManagedContainer`. 2. Provide a
`ContainerDescriptor` (image, mounts, platform) and a readiness probe. 3.
Extend `ContainerAgentRuntime` in `../../agents/runtime/` and export a
`configure*Runtime()`.

**Driving container status in the UI:** subscribe with the base class's
listener API; the `/agents` responses already surface liveness.

**Testing:** `tests/lib/container/managed/managed-container.test.ts` exercises
the state machine offline — extend it for new transitions rather than
spinning up a real container.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/container/` conventions and the `podman` caveat.
- [`../../agents/runtime/AGENTS.md`](../../agents/runtime/AGENTS.md) — the concrete runtime that extends this.
- [`../../../../tests/lib/container/managed/AGENTS.md`](../../../../tests/lib/container/managed/AGENTS.md) — base-class tests.
