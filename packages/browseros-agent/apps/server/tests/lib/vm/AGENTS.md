# `tests/lib/vm/` — Lima VM unit tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/`.

## What's here

Five tests for `../../../../src/lib/vm/`, all hermetic: `fake-limactl.ts`
from [`../../__helpers__/`](../../__helpers__/AGENTS.md) writes an executable
stub into a temp dir and puts it on `PATH`, so the real `LimaCli` argument
construction runs without Lima being installed. The real-toolchain
verification lives separately in
[`../../integration/vm-smoke.test.ts`](../../integration/AGENTS.md).

## Contents

| File | Covers |
|---|---|
| `lima-cli.test.ts` | `LimaCli` — binary presence checks, argument construction, stdout/stderr/exit handling against the fake. |
| `lima-config.test.ts` | `renderLimaTemplate()` — the generated VM config YAML. |
| `vm-runtime.test.ts` | `VmRuntime` — start/stop/status and the persisted state file. |
| `paths.test.ts` | VM and guest path resolution, including existence/executability checks. |
| `errors.test.ts` | The `VmError` hierarchy — callers distinguish types, so the types are the contract. |

## Rules

**VM-T1 — Use `fake-limactl.ts`, never a real Lima.** A test that finds the
stub must not fall through to a system Lima. The fake is what makes this
group hermetic and CI-safe.

**VM-T2 — `errors.test.ts` pins the type hierarchy.** New VM failure modes
belong as `VmError` subclasses; add them there. Callers branch on the type,
so renaming or collapsing an error breaks production callers silently.

**VM-T3 — Paths are asserted against the BrowserOS dir, not hard-coded.**
`paths.ts` resolves under `getVmStateDir()` from
`../../../../src/lib/browseros-dir.ts`. Override the base in the test; don't
hard-code `~/.browseros`.

**VM-T4 — The fake is per-test.** Each test that needs `limactl` creates its
own temp dir and cleans up in `afterEach`, so a mutated stub from one test
can't leak into the next.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`, or
`bun --env-file=.env.development test tests/lib/vm`.

**Adding a Lima command:** 1. Add the invocation to
`../../../../src/lib/vm/lima-cli.ts`. 2. Add a case to `lima-cli.test.ts`
using the fake stub, asserting on the arguments the wrapper would pass. 3. If
it's a new lifecycle milestone, add the event to `VM_TELEMETRY_EVENTS`.

**Verifying against real Lima:** that's
[`../../integration/vm-smoke.test.ts`](../../integration/AGENTS.md), not here.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/` conventions.
- [`../../../../src/lib/vm/AGENTS.md`](../../../src/lib/vm/AGENTS.md) — the module under test.
- [`../../__helpers__/AGENTS.md`](../../__helpers__/AGENTS.md) — `fake-limactl.ts` and `fake-ssh.ts`.
- [`../../../../CLAUDE.md`](../../../../../CLAUDE.md) — the `limactl` vs `podman` release-gating note.
