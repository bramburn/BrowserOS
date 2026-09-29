# `packages/cdp-protocol/src/generated/` — codegen output root

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/cdp-protocol`.

## What's here

The output directory of `../../../../scripts/codegen/cdp-protocol.ts`. It
holds two parallel per-domain trees — `domains/` (pure types) and
`domain-apis/` (wrapper classes) — plus the two aggregate files
`protocol-api.ts` and `create-api.ts`. Every file begins with the marker
comment `// ── AUTO-GENERATED from CDP protocol. DO NOT EDIT. ──`.

## Contents

```
cdp-protocol/src/generated/
├── domains/        ← 57 × `<kebab-domain>.ts`, types only
├── domain-apis/    ← 57 × `<kebab-domain>.ts`, `<Pascal>API` classes
├── protocol-api.ts ← the aggregated ProtocolApi type across all domains
└── create-api.ts   ← createProtocolApi(send, on): per-domain Proxy factory
```

## Rules

**CG1 — Treat everything here as build output.** If a name looks wrong, the
fix belongs in the emitters (`domain-emitter.ts`, `domain-api-emitter.ts`,
`protocol-api-emitter.ts`, `type-emitter.ts`, `naming.ts`).

**CG2 — `domains/` and `domain-apis/` must stay name-aligned.** Both trees use
`domainToKebab(domain)`, so `DOMSnapshot` → `dom-snapshot.ts` in each. A
mismatch means a naming-rule change, not a manual file.

**CG3 — `protocol-api.ts` and `create-api.ts` are generated together.**
`createProtocolApi` casts its returned object `as unknown as ProtocolApi`, so
a missing entry in one file is a silent type hole until runtime. Regenerate
rather than patching.

**CG4 — Only `create-api.ts` and the `domain-apis/` classes are runtime
values.** `domains/` is type-only and must be imported with `import type`.

## Workflows

**Regenerating:** `CDP_PROTOCOL_JSON=<protocol.json path> bun run gen:cdp`
from `packages/browseros-agent`. The script recreates this whole directory,
then rewrites `packages/cdp-protocol/package.json` exports. Review the full
diff of this directory plus that `package.json` before committing.

**Finding which emitter produced a line:** `type-emitter.ts` builds the
`XxxParams` / `XxxResult` / `XxxEvent` interfaces; `domain-emitter.ts` and
`domain-api-emitter.ts` wrap them per domain; `protocol-api-emitter.ts` emits
both aggregate files.

**Adding a whole new domain:** it appears automatically once it exists in
`protocol.json` — confirm the new `domains/<kebab>.ts`, the matching
`domain-apis/<kebab>.ts`, both `package.json` exports, and the new key in
`create-api.ts` all landed in the same run.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — cdp-protocol scope (rules CD1–CD6).
- [`domains/AGENTS.md`](domains/AGENTS.md) — the type files.
- [`domain-apis/AGENTS.md`](domain-apis/AGENTS.md) — the API wrapper files.
- [`../../../../scripts/codegen/AGENTS.md`](../../../../scripts/codegen/AGENTS.md)
  — the generator that writes this directory.
