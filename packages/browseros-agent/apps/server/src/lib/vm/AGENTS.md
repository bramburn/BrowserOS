# `src/lib/vm/` — Lima VM integration

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/`.

## What's here

Everything the server knows about the bundled Lima virtual machine.
`lima-cli.ts` wraps the `limactl` binary, `lima-config.ts` renders the VM
template, `paths.ts` resolves VM and state directories, `vm-runtime.ts` is
the lifecycle façade, `telemetry.ts` enumerates VM events, and `errors.ts`
carries the `VmError` hierarchy. `index.ts` is the barrel — import from here.

## Contents

| File | Purpose |
|---|---|
| `lima-cli.ts` | `LimaCli` — the `limactl` invocation wrapper; checks the binary exists before use. |
| `lima-config.ts` | `renderLimaTemplate(...)` — produces the VM config YAML. |
| `vm-runtime.ts` | `VmRuntime` — start/stop/status/state for a VM; reads and writes the state file. |
| `paths.ts` | Resolves VM and guest paths, with existence/executability checks. |
| `telemetry.ts` | `VM_TELEMETRY_EVENTS` — the canonical VM event-name set. |
| `errors.ts` | `VmError` and its subclasses. |
| `index.ts` | Barrel re-exporting all of the above. |

## Contents (context)

`resources/bin/third_party/` currently ships `limactl`, not `podman` — the
ContainerRuntime migration (WS6) is what will switch the sandbox back to
`podman`. See `CLAUDE.md` § "Release gating — bundled-VM runtime migration"
and [`../container/`](../container/AGENTS.md).

## Rules

**VM1 — `limactl` only in `lima-cli.ts`.** No other file shells out to a VM
binary. `paths.ts` may *check* for it; it must not invoke it.

**VM2 — VM errors are `VmError` subclasses.** Callers distinguish "no VM",
"VM not running", "limactl missing" by type. A bare `Error` collapses them.

**VM3 — Event names come from `VM_TELEMETRY_EVENTS`.** Add a new lifecycle
milestone there; don't free-type an event string.

**VM4 — VM state lives under the BrowserOS directory.** All host paths go
through `paths.ts` and `../browseros-dir.ts` so the `/mnt/browseros/vm` bind
mount (relied on by
[`../agents/hermes/hermes-paths.ts`](../agents/hermes/hermes-paths.ts)) stays
correct.

**VM5 — The VM is optional.** The server runs fine without one; every VM path
must degrade to "unavailable", not throw at startup.

**VM6 — Test with the fake.** `tests/__helpers__/fake-limactl.ts` writes an
executable stub into a temp dir; `fake-ssh.ts` does the same for the SSH
transport. Use them instead of installing Lima.

## Workflows

**Adding a VM lifecycle step:** 1. Add the `limactl` invocation in
`lima-cli.ts`. 2. Expose it on `VmRuntime`. 3. Add the event to
`VM_TELEMETRY_EVENTS`. 4. Emit with `metrics.log(...)`. 5. Extend
`tests/lib/vm/` using `fake-limactl.ts`.

**Smoke-testing the whole sandbox:** `tests/integration/vm-smoke.test.ts`
exercises `LimaCli`, `VmRuntime`, and `ContainerCli` together — the one test
that touches both `vm/` and `container/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/` conventions.
- [`../container/AGENTS.md`](../container/AGENTS.md) — the podman layer.
- [`../../../../CLAUDE.md`](../../../../../CLAUDE.md) — release gating for `limactl` vs `podman`.
- [`../../../../tests/lib/vm/AGENTS.md`](../../../tests/lib/vm/AGENTS.md) — VM tests.
