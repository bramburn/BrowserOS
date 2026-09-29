# `tests/lib/container/managed/` — ManagedContainer tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/container/`.

## What's here

One file: `managed-container.test.ts`, covering the abstract
`ManagedContainer` base in
[`../../../../../src/lib/container/managed/`](../../../../src/lib/container/managed/AGENTS.md).
The test defines a minimal concrete subclass with a tiny descriptor, so the
shared lifecycle — state machine, lifecycle lock, gated `execute*` family,
listener notifications — is verified once for every future container-backed
agent adapter. No real container is involved.

## Contents

| File | Covers |
|---|---|
| `managed-container.test.ts` | `ManagedContainer` — `ContainerState` transitions, `TRANSIENT_STATES` polling, `ContainerNotReadyError` on early exec, `PathOutsideMountsError` on mount escape, `ResetNotSupportedError`, and `StateListener` / `Unsubscribe` behaviour. |

## Rules

**MC-T1 — Test the base, not a concrete adapter.** If a new container type
needs a different assertion, either the base class's contract changed (add it
here) or the test belongs in the adapter's own test file.

**MC-T2 — A minimal subclass is the fixture.** Keep the test's descriptor
tiny. A rich descriptor makes the test pass for the wrong reasons and hides
which state transitions are actually covered.

**MC-T3 — Assert the typed errors.** `ContainerNotReadyError`,
`PathOutsideMountsError`, and `ResetNotSupportedError` are how callers tell
"starting" from "failed" and "reset unsupported" from "reset done". Assert on
the class, not the message.

**MC-T4 — Unsubscribe in teardown.** The base class broadcasts state changes
to listeners; a leaked subscription outlives the test and produces
cross-test noise.

**MC-T5 — No container runtime.** A test needing real `podman` or Lima
belongs in [`../../../integration/`](../../../integration/AGENTS.md).

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Changing the state machine:** 1. Update
`../../../../../src/lib/container/managed/managed-container.ts`. 2. Add the
transition to this test, including the `TRANSIENT_STATES` membership.
3. Confirm `HermesContainerRuntime` still satisfies the contract
([`../agents/runtime/hermes-container-runtime.test.ts`](../../agents/runtime/AGENTS.md)).

**Adding a reset level:** extend `ResetLevel` / `ResetOptions` in
`managed/types.ts`, add a case here, and keep `ResetNotSupportedError` for
containers that cannot honour the requested level.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/container/` conventions.
- [`../../../../../src/lib/container/managed/AGENTS.md`](../../../../src/lib/container/managed/AGENTS.md) — the class under test.
- [`../agents/runtime/AGENTS.md`](../../agents/runtime/AGENTS.md) — the concrete runtime.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
