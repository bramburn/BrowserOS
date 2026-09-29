# `packages/cdp-protocol/src/` — generated source root

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/cdp-protocol`.

## What's here

The `src/` root of `@browseros/cdp-protocol`, and it contains exactly one
subdirectory: `generated/`. There is no hand-written source here — the
package `tsconfig.json` sets `rootDir: "src"` and includes `src/**/*` purely so
the generated tree typechecks.

## Contents

```
cdp-protocol/src/
└── generated/   ← 116 files: domains/, domain-apis/, protocol-api.ts, create-api.ts
```

## Rules

**CS1 — Nothing hand-written under `src/`.** If you need logic that the CDP
emitters can't produce, it belongs in `apps/server` or in
`../../../scripts/codegen/lib/`, not here.

**CS2 — `generated/` is wiped on every run.** `scripts/codegen/cdp-protocol.ts`
does `rmSync(GEN_DIR, { recursive: true, force: true })` before writing. Never
park scratch files in this tree.

**CS3 — Every `exports` entry in `package.json` points into
`src/generated/`.** If you add a top-level file under `src/`, you must also add
a `types` + `default` exports entry or nothing can import it.

## Workflows

**Typechecking this package:** `bun run typecheck` from
`packages/browseros-agent` (it runs `tsc --noEmit` in each workspace). There
are no tests here — all assertions live in the generator and its consumers.

**Navigating from a symbol to its origin:** find the symbol in
`generated/domains/<domain>.ts` for the type, then look for the matching
entry in `../../../scripts/codegen/lib/domain-emitter.ts` to understand how
the name is derived.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — cdp-protocol scope (rules CD1–CD6).
- [`generated/AGENTS.md`](generated/AGENTS.md) — the generated tree.
- [`../package.json`](../package.json) — the exports map.
- [`../../../scripts/codegen/AGENTS.md`](../../../scripts/codegen/AGENTS.md) — the generator.
