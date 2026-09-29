# `scripts/codegen/lib/` — CDP parser and emitters

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/scripts/codegen`.

## What's here

The five libraries the CDP generator is built from. `protocol-parser.ts` reads
and normalises Chromium's `protocol.json` into a `Protocol` object. `naming.ts`
converts protocol identifiers into file names and class names. `type-emitter.ts`
turns a domain's commands and events into TypeScript interfaces.
`domain-emitter.ts` and `domain-api-emitter.ts` assemble the per-domain output
files, and `protocol-api-emitter.ts` emits the two aggregate files
(`protocol-api.ts`, `create-api.ts`).

## Contents

```
scripts/codegen/lib/
├── protocol-parser.ts      ← ProtocolProperty/Type/Command/Event/Domain/Protocol interfaces + parseProtocol(path)
├── naming.ts               ← domainToKebab, toPascalCase, resolveRef
├── type-emitter.ts         ← XxxParams / XxxResult / XxxEvent interface emission
├── domain-emitter.ts       ← emitDomainFile(domain) → `domains/<kebab>.ts`
├── domain-api-emitter.ts   ← emitDomainApiFile(domain) → `domain-apis/<kebab>.ts`
└── protocol-api-emitter.ts ← emitProtocolApiFile(domains), emitCreateApiFile(domains)
```

## Rules

**CE1 — Emitters return strings; the caller writes files.** `cdp-protocol.ts`
does the `Bun.write`. Keep I/O out of this folder so emitters stay pure and
unit-testable.

**CE2 — `domainToKebab` is the single source of truth for filenames.**
Both the type emitter and the API emitter must call it, never inline a
different regex.

**CE3 — `toPascalCase` is the single source of class names.** `PageAPI`,
`DOMSnapshotAPI`, `WebAuthnAPI` all come from
`toPascalCase(domain) + 'API'`.

**CE4 — `resolveRef` is the only `$ref` handling.** A ref without a dot is a
local type; `Domain.Type` resolves to that domain's type. Adding a second
resolution path produces silently duplicated types.

**CE5 — Emit `// ── AUTO-GENERATED from CDP protocol. DO NOT EDIT. ──` as the
first line of every file.** Consumers (and humans) rely on it.

**CE6 — No imports between emitters beyond `naming.ts` and the parser's
types.** A dependency cycle here silently produces inconsistent output across
the two trees.

**CE7 — Changes here change every generated file.** After editing any emitter,
regenerate and diff the whole `packages/cdp-protocol/src/generated/` tree, not
just the one domain you touched.

## Workflows

**Renaming a generated type or class:** 1. Edit `naming.ts`. 2. Regenerate.
3. Fix every consumer import in `apps/server` and `apps/agent`.

**Supporting a new protocol JSON construct:** 1. Extend the relevant
interface in `protocol-parser.ts`. 2. Emit it from `type-emitter.ts`. 3.
Regenerate and check a domain that uses it (e.g. `page.ts` for most new
features).

**Debugging a missing interface:** check whether the command has a `returns`
block in `protocol.json`. A command with no return emits only
`XxxParams` and no `XxxResult`.

**Debugging a duplicated cross-domain type:** that's `resolveRef` inlining by
design (rule CG6 in the parent). Don't try to make one file import another.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — codegen scope (rules CG1–CG6).
- [`../cdp-protocol.ts`](../cdp-protocol.ts) — the driver.
- [`naming.ts`](naming.ts) — the naming rules everything else depends on.
- [`../../../../packages/cdp-protocol/AGENTS.md`](../../../packages/cdp-protocol/AGENTS.md)
  — the generated output.
