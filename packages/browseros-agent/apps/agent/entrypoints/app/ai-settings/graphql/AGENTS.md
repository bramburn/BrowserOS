# `entrypoints/app/ai-settings/graphql/` — AI settings GraphQL documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app/ai-settings`.

## What's here

Two `graphql(...)` operations used by the AI settings page. This is a leaf folder:
one file, no components, no hooks.

`GetRemoteLlmProvidersDocument` loads the user's providers from the remote backend
keyed by `profileId` (it does **not** include the API key in the selection).
`DeleteRemoteLlmProviderDocument` removes one by `rowId`.

## Contents

```
graphql/
└── aiSettingsDocument.ts   ← GetRemoteLlmProviders (query), DeleteRemoteLlmProvider (mutation)
```

## Rules

- **G1 — Documents live in a `graphql/` folder, named `<feature>Document.ts`.** Both
  the `app/ai-settings` and `app/profile` routes follow this; keep the convention when
  adding a third.
- **G2 — `graphql` comes from `@/generated/graphql/gql`,** not from a hand-written
  typed-document helper. Codegen owns the types.
- **G3 — The SDL is the source of truth.** Any field you add must exist in
  `apps/agent/schema/schema.graphql` before you run `bun run codegen`
  (from `packages/browseros-agent`).
- **G4 — Never select secret material.** API keys are not in these selections and must
  not be — the browser holds them in extension storage, not in the profile record.
- **G5 — Export the document, never the result type.** Callers derive types from the
  generated document.

## Workflows

**Adding a query here**
1. Add the field to `apps/agent/schema/schema.graphql` if it is new.
2. Run `bun run codegen` from `packages/browseros-agent`.
3. Add the `graphql(\`...\`)` block to `aiSettingsDocument.ts` with a `PascalCase`
   name ending in `Document`.
4. Consume it from the parent folder with `useGraphqlQuery` /
   `useGraphqlMutation`; invalidate with
   `getQueryKeyFromDocument(GetRemoteLlmProvidersDocument)`.

**Checking a document type**
1. Find the generated artifact under `apps/agent/generated/graphql/`.
2. Do not hand-write result types here — they will drift from the SDL.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the consuming page and its rules.
- [`../../profile/graphql/AGENTS.md`](../../profile/graphql/AGENTS.md) — sibling folder, same pattern.
- [`../../../../../schema/schema.graphql`](../../../../schema/schema.graphql) — hand-maintained SDL.
- [`../../../../../lib/graphql/`](../../../../lib/graphql/) — `useGraphqlQuery`, `useGraphqlMutation`, `getQueryKeyFromDocument`.
