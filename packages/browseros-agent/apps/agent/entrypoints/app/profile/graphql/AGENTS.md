# `entrypoints/app/profile/graphql/` — profile GraphQL documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app/profile`.

## What's here

Two `graphql(...)` operations for the profile page. A leaf folder: one file, no
components, no hooks.

`GetProfileByUserIdDocument` reads first name, last name and avatar URL for a user.
`UpdateProfileByUserIdDocument` applies a `ProfilePatch!` and returns the updated
profile.

## Contents

```
graphql/
└── profileDocument.ts   ← GetProfileByUserId (query), UpdateProfileByUserId (mutation)
```

## Rules

- **G1 — One `<feature>Document.ts` per route.** Same convention as
  `../../ai-settings/graphql/aiSettingsDocument.ts`; don't merge them.
- **G2 — Import `graphql` from `@/generated/graphql/gql`.** Never hand-write
  typed-document helpers.
- **G3 — SDL is the source of truth.** New fields go into
  `apps/agent/schema/schema.graphql` first, then `bun run codegen`.
- **G4 — Selections must mirror the mutation's return selection.** Both return the
  same `rowId / firstName / lastName / avatarUrl` shape on purpose, so the cache
  entry updated by the mutation is the one the query reads.
- **G5 — Export the document only.** Callers infer types; no hand-written result
  interfaces.

## Workflows

**Adding a profile field**
1. Extend `ProfilePatch` and the profile type in `apps/agent/schema/schema.graphql`.
2. Run `bun run codegen`.
3. Add the field to the query selection **and** the mutation's profile selection.
4. Confirm the mutation's returned selection still matches the query's, or the cache
   will not update in place.

**Deriving a cache key**
1. `getQueryKeyFromDocument(GetProfileByUserIdDocument)` — never a literal array.
2. Pass `{ userId }` as the variables when matching the query key.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the consuming page.
- [`../../ai-settings/graphql/AGENTS.md`](../../ai-settings/graphql/AGENTS.md) — sibling folder, identical pattern.
- [`../../../../../schema/schema.graphql`](../../../../schema/schema.graphql) — hand-maintained SDL.
- [`../../../../../lib/graphql/`](../../../../lib/graphql/) — hooks and key derivation.
