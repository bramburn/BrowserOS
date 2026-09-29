# `packages/shared/src/schemas/` — zod schemas shared by server and extension

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/shared`.

## What's here

Three zod modules that define the wire contracts both `apps/server` and
`apps/agent` agree on. `llm.ts` pins the supported LLM provider enum and the
`LLMConfigSchema` (including the Azure, Bedrock, and ChatGPT-Pro/Codex
specific fields). `ui-stream.ts` models the Vercel AI SDK UI message stream
events. `browser-context.ts` models the tab / window / MCP-server context the
extension sends when targeting browser operations. Types are always derived
with `z.infer<>`, never hand-written alongside the schema.

## Contents

```
shared/src/schemas/
├── llm.ts              ← LLM_PROVIDERS (14 values), LLMProviderSchema, LLMConfigSchema,
│                          type LLMProvider, type LLMConfig
├── ui-stream.ts        ← per-event zod schemas (start, start-step, finish-step, finish,
│                          abort, error, text-start, text-delta, …) + the stream union
└── browser-context.ts  ← TabSchema, CustomMcpServerSchema, BrowserContextSchema + types
```

## Rules

**SC1 — `z.infer` is the only way types are declared here.** No
`interface LLMConfig` sitting next to `LLMConfigSchema`; they drift.

**SC2 — Explicit type annotations on each schema.** The schemas carry an
explicit `z.ZodObject<{...}>` annotation (see the `llm.ts` and `ui-stream.ts`
pattern) because the package is consumed with `isolatedDeclarations`. Keep
the annotation when you add a field.

**SC3 — Adding a provider is a three-place change.** The value in
`LLM_PROVIDERS`, the corresponding member of the `LLMProviderSchema`'s
explicit type list, and the `z.enum([...])` argument. Missing the middle one
is a type error; missing the third is a runtime gap.

**SC4 — Zod is this package's only runtime dependency.** Don't add another
validation library or a DTO mapper here.

**SC5 — These are wire contracts.** Adding an optional field is safe; renaming
or removing one breaks the extension ↔ server handshake at runtime with no
compile-time error on the other side.

## Workflows

**Adding a new LLM provider:** 1. Add to `LLM_PROVIDERS` in `llm.ts`. 2. Add
it to the `LLMProviderSchema` type parameter and to the `z.enum([...])` list.
3. If it needs extra fields (as Azure/Bedrock do), add them to the explicit
type annotation, the schema object, and `LLMConfig`. 4. Update the extension's
provider list and the server's `provider-factory.ts`.

**Adding a UI stream event:** 1. Add a `XxxEventSchema` with an explicit
`z.ZodObject` annotation in `ui-stream.ts`. 2. Add it to the stream union and
to any exported `z.infer` type. 3. Handle it in the extension's message
normalization.

**Consuming a schema:** `import { LLMConfigSchema } from
'@browseros/shared/schemas/llm'` then `.parse()` at the boundary. Don't
re-derive shapes by hand at the call site.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/` layout and the exports rule (SS1).
- [`../types/AGENTS.md`](../types/AGENTS.md) — hand-written type modules.
- [`../constants/AGENTS.md`](../constants/AGENTS.md) — related value modules.
- [`../../AGENTS.md`](../../AGENTS.md) — `@browseros/shared` package scope.
- [`../../../../apps/agent/AGENTS.md`](../../../../apps/agent/AGENTS.md) — extension consumer of these schemas.
