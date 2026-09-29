# `lib/llm-providers/graphql/` — Provider upload documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib/llm-providers`.

## What's here

A single file, `uploadLlmProviderDocument.ts`, holding the codegen'd
GraphQL operations used to mirror the user's LLM provider configuration to
the BrowserOS cloud so it can follow them across devices. The consumer is
`../uploadLlmProvidersToGraphql.ts`.

## Contents

```
graphql/
└── uploadLlmProviderDocument.ts
    ├── CreateLlmProviderForUploadDocument   ← mutation createLlmProvider
    ├── UpdateLlmProviderForUploadDocument   ← mutation updateLlmProvider
    └── GetLlmProvidersByProfileIdDocument   ← query llmProviders by profileId
```

## Rules

- **PQD1 — Operations must be named.** The name becomes the TanStack Query
  cache key via `lib/graphql/getQueryKeyFromDocument.ts`.
- **PQD2 — Documents here carry non-sensitive config only.** No `apiKey`,
  `accessKeyId`, `secretAccessKey`, or `sessionToken` may appear in a
  selection set; the browser-local `providersStorage` is the only place
  secrets live.
- **PQD3 — Keep documents next to the feature**, in a `graphql/` subfolder —
  do not centralise operations into one "documents.ts" file. This is the
  pattern used by `lib/conversations/graphql/` and `lib/schedules/graphql/`.
- **PQD4 — After adding a field, update `schema/schema.graphql` first and run
  `bun run codegen`**, or the `graphql()` tag will not type-check.

## Workflows

**Adding a provider-upload field**
1. Add the field to the backend type in `schema/schema.graphql`.
2. `cd packages/browseros-agent && bun run codegen`.
3. Extend the existing operation's selection set here.
4. Map the value in `../uploadLlmProvidersToGraphql.ts`, excluding secrets.

**Adding a new provider operation**
1. Create the document in this file.
2. Call it with `execute()` from `../uploadLlmProvidersToGraphql.ts`, or
   `useGraphqlMutation()` from a component.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — provider module rules (LPV2, LPV3).
- [`../uploadLlmProvidersToGraphql.ts`](../uploadLlmProvidersToGraphql.ts) — the caller.
- [`../../graphql/AGENTS.md`](../../graphql/AGENTS.md) — `execute()` and the hooks.
- [`../../../../schema/AGENTS.md`](../../../schema/AGENTS.md) — the SDL source.
