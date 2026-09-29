# `scripts/codegen/` — CDP protocol generator

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

`cdp-protocol.ts` is the generator behind `bun run gen:cdp`. It reads
Chromium's `protocol.json` (path from the `CDP_PROTOCOL_JSON` env var),
parses it, then **deletes and recreates**
`../../packages/cdp-protocol/src/generated/`, writing one type module and one
API-wrapper module per domain plus `protocol-api.ts`, `create-api.ts`, and a
fresh `exports` map in the package's `package.json`. The parsing and emission
logic lives in `lib/`.

## Contents

```
scripts/codegen/
├── cdp-protocol.ts      ← entry: parse → rmSync(GEN_DIR) → emit per domain → emit aggregates → rewrite package.json
└── lib/
    ├── protocol-parser.ts        ← Protocol/Domain/Command/Event/Type shapes + parseProtocol(path)
    ├── naming.ts                 ← domainToKebab, toPascalCase, resolveRef
    ├── type-emitter.ts           ← XxxParams / XxxResult / XxxEvent interfaces
    ├── domain-emitter.ts         ← one `domains/<kebab>.ts` per domain
    ├── domain-api-emitter.ts     ← one `domain-apis/<kebab>.ts` per domain
    └── protocol-api-emitter.ts   ← protocol-api.ts + create-api.ts
```

## Rules

**CG1 — The generator owns the output tree completely.** It runs
`rmSync(GEN_DIR, { recursive: true, force: true })` before writing, so nothing
under `../../packages/cdp-protocol/src/generated/` survives by accident. Never
hand-add a file there.

**CG2 — `CDP_PROTOCOL_JSON` is mandatory.** Missing env var ⇒ the script
prints `Set CDP_PROTOCOL_JSON to the path of browser_protocol.json` and exits
1 before touching the filesystem.

**CG3 — `domainToKebab` drives every filename.** `DOMSnapshot` → `dom-snapshot`,
`CSS` → `css`. Change `naming.ts` and regenerate; never rename a generated
file to taste.

**CG4 — Both trees are emitted per domain, in the same loop pass.** If a
domain is missing from `domains/` or from `domain-apis/`, the emitter is
broken — there is no allowlist.

**CG5 — The `exports` map is regenerated too.** `writePackageJson` emits
`./domains/<kebab>`, `./domain-apis/<kebab>`, `./protocol-api`, and
`./create-api`, each with `types` + `default`. Review that diff with the rest
of the output.

**CG6 — `$ref`s are resolved, not imported.** `resolveRef` splits
`Domain.Type` into a domain and a type name; the emitters inline the shape
into the consuming file. Cross-domain type sharing in the output is
intentional, not duplication to clean up.

## Workflows

**Regenerating:** `CDP_PROTOCOL_JSON=/path/to/protocol.json bun run gen:cdp`
from `packages/browseros-agent`. Then review the diff of
`../../packages/cdp-protocol/src/generated/`, that package's `package.json`,
and this `lib/` if you changed an emitter.

**Adding a new code generator for another schema:** 1. Add a sibling entry
script in this directory. 2. Read the input from an env var and fail fast
when it is unset (as `CDP_PROTOCOL_JSON` does). 3. Write output under the
target package's `src/`, with a `// ── AUTO-GENERATED … DO NOT EDIT. ──`
header in every file. 4. Register an npm script in `../../package.json`.

**Adding a field to a generated type:** it must exist in `protocol.json`
first. If it doesn't, add it upstream; do not patch `type-emitter.ts` to
invent a field.

**Changing a generated type's *name*:** change the naming rule in
`naming.ts`, regenerate, and fix every consumer's import in the same change.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/` scope (rules SR1–SR6).
- [`lib/AGENTS.md`](lib/AGENTS.md) — parser and emitters.
- [`../../packages/cdp-protocol/AGENTS.md`](../../packages/cdp-protocol/AGENTS.md) — the output package.
- [`../../packages/cdp-protocol/README.md`](../../packages/cdp-protocol/README.md) — regeneration instructions.
- [`../../package.json`](../../package.json) — the `gen:cdp` script.
