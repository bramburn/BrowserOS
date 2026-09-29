# `tests/lib/agents/hermes/` — Hermes helper tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/agents/`.

## What's here

One file: `hermes-paths-provider-map.test.ts`, covering the two pure helpers
in [`../../../../../src/lib/agents/hermes/`](../../../../src/lib/agents/hermes/AGENTS.md)
— `hermes-paths.ts` (host-side directory resolution) and
`hermes-provider-map.ts` (BrowserOS provider type → Hermes configuration).
Both are pure functions over paths and lookup tables, so the test is fast and
needs no container.

## Contents

| File | Covers |
|---|---|
| `hermes-paths-provider-map.test.ts` | `getHermesHostStateDir`, `getHermesHarnessHostDir`, `getHermesAgentHomeHostDir` — the three-level path chain under `<browserosDir>/vm/hermes`; and the provider translation table. |

## Rules

**HE-T1 — Assert the full path chain, not just the last segment.** The
invariant is `…/vm/hermes` → `…/vm/hermes/harness` → `…/harness/<agentId>`,
because each level is bind-mounted in turn (host → `/mnt/browseros/vm` in the
Lima guest → `/data/agents/harness` in the container). A test that only
checks the leaf would pass while the chain breaks.

**HE-T2 — Pass `browserosDir` explicitly.** Every helper takes an optional
base. Tests must supply a temp path rather than falling through to
`getVmStateDir()`, which reads the real `~/.browseros` layout.

**HE-T3 — The provider map is a closed set.** Hermes only understands a small
fixed set of configurations. A new BrowserOS provider must be mapped
explicitly; add the case here when you add it to the table.

**HE-T4 — No filesystem assertions beyond path strings.** These helpers build
paths; the writes that follow (`config.yaml`, `.env`) belong to
`agent-harness-service.ts` and are covered elsewhere.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Debugging a Hermes agent that can't find its home:** check the three
helpers in order, then confirm the Lima bind mount exposes the harness dir at
`/mnt/browseros/vm/hermes/harness`. The container runtime test is in
[`../runtime/hermes-container-runtime.test.ts`](../runtime/AGENTS.md).

**Adding a Hermes-supported provider:** add the entry in `hermes-provider-map.ts`,
then the expected mapping in this test.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/agents/` conventions.
- [`../../../../../src/lib/agents/hermes/AGENTS.md`](../../../../src/lib/agents/hermes/AGENTS.md) — the helpers under test.
- [`../runtime/AGENTS.md`](../runtime/AGENTS.md) — the Hermes container runtime.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
