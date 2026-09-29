# `src/lib/agents/hermes/` — Hermes path and provider mapping

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/agents/`.

## What's here

Two small pure modules specific to the Hermes adapter. `hermes-paths.ts`
computes host-side directories for Hermes state; `hermes-provider-map.ts`
translates BrowserOS LLM provider types into the small fixed set of
configurations Hermes itself understands. Hermes also has a runtime, but that
lives one level up in [`../runtime/`](../runtime/AGENTS.md).

## Contents

| File | Purpose |
|---|---|
| `hermes-paths.ts` | `getHermesHostStateDir()` → `<browserosDir>/vm/hermes`; `getHermesHarnessHostDir()` → `…/vm/hermes/harness`; `getHermesAgentHomeHostDir({ browserosDir, agentId })` → the per-agent home that receives `config.yaml` + `.env` at agent-create time. |
| `hermes-provider-map.ts` | Translation table from BrowserOS `LlmProviderConfig.type` values to Hermes runtime configuration. |

## Contents (path contract)

Host `<browserosDir>/vm/hermes/harness` is exposed to the Lima VM through the
existing `vm/` → `/mnt/browseros/vm` bind mount; the Hermes container then
bind-mounts the guest-side `/mnt/browseros/vm/hermes/harness` to
`/data/agents/harness`. That is why `HERMES_HOME` is a path the container can
actually open — the three hops are the invariant, not an accident.

## Rules

**HE1 — All Hermes host paths are computed here.** Never `join()` a Hermes
path inline elsewhere. The three-hop bind-mount chain is only correct if
every layer derives from these helpers.

**HE2 — Per-agent state lives under `…/harness/<agentId>`.** `config.yaml`
and `.env` are written once at agent-create time and stay constant across
turns; don't rewrite them per turn.

**HE3 — Provider translation is a lookup, not a fetch.** Hermes supports a
small fixed set; map BrowserOS provider types to it here. If a new provider
has no Hermes equivalent, map it to the closest one explicitly rather than
passing the raw type through — Hermes will reject it at runtime.

**HE4 — Keep both files dependency-free.** They import only `node:path` /
`node:fs` and `../../browseros-dir`. No config, no db, no container imports.

## Workflows

**Adding a Hermes-supported provider:** 1. Add the BrowserOS provider type →
Hermes config entry in `hermes-provider-map.ts`. 2. Verify the extension's
provider list still offers it for Hermes-backed agents.

**Debugging a missing agent home:** walk the chain —
`getHermesHostStateDir()` → `getHermesHarnessHostDir()` →
`getHermesAgentHomeHostDir({ agentId })` — then confirm the VM bind mount
exposes it at `/mnt/browseros/vm/hermes/harness`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — ACPX runtimes, stores, turn registry.
- [`../runtime/AGENTS.md`](../runtime/AGENTS.md) — `HermesContainerRuntime` and `ensureHermesRuntimeReady`.
- [`../../browseros-dir.ts`](../../browseros-dir.ts) — `getVmStateDir()` and the `~/.browseros` layout.
- [`../../../../tests/lib/agents/hermes/AGENTS.md`](../../../../tests/lib/agents/hermes/AGENTS.md) — tests.
