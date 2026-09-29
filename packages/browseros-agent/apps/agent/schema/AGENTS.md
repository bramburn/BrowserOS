# `schema/` — Hand-maintained GraphQL SDL

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

`schema.graphql` — the single source of truth for the BrowserOS cloud
GraphQL API as this extension sees it (~2,600 lines of Relay-style SDL:
`Query`, `Mutation`, `Node`, connections with `nodes`/`edges`/`totalCount`,
and the `*Payload` input/output type pairs). `codegen.ts` reads it as the
`schema` source and scans `./**/*.ts{,x}` for `graphql()` documents, emitting
`generated/graphql/`.

## Contents

```
schema/
└── schema.graphql   ← hand-written SDL; NOT generated from the server
```

## Rules

- **SCH1 — Hand-maintained. Never generate or overwrite this file.** Rule B8 in
  [`../../AGENTS.md`](../../AGENTS.md) and X5 in the parent apply.
- **SCH2 — After any edit, run `bun run codegen`** from
  `packages/browseros-agent`. `generated/graphql/` is what
  `lib/graphql/execute.ts` and every `graphql()` document type-check against;
  a stale codegen is a type error, not a runtime one.
- **SCH3 — `codegen.ts` honours `GRAPHQL_SCHEMA_PATH`.** When that env var is
  set it wins over this file, and the file's absence is not fatal. Local
  development without the env var uses this copy.
- **SCH4 — Documents live with the feature, not here.** Add a `graphql()`
  tagged query next to its consumer, e.g.
  `lib/conversations/graphql/uploadConversationDocument.ts` or
  `lib/schedules/graphql/syncSchedulesDocument.ts`.
- **SCH5 — SDL is Relay-generated output, so keep the doc-comment style.** Types
  are alphabetically ordered with triple-quoted descriptions; match it or diffs
  against the real backend get noisy.

## Workflows

**Adding a field the extension needs**
1. Edit `schema.graphql` to match the deployed backend.
2. `cd packages/browseros-agent && bun run codegen`.
3. Write the operation in the owning feature's `graphql/` folder using
   ``graphql(`query Name(...) { ... }`)``.
4. Run it via `lib/graphql/execute.ts` or the typed hooks
   (`useGraphqlQuery`, `useGraphqlMutation`).

**Verifying the schema matches the backend**
1. Set `GRAPHQL_SCHEMA_PATH` to a freshly exported SDL and run
   `bun run codegen` — a clean codegen is a schema match.
2. Never edit `generated/graphql/schema.graphql` (the `schema-ast` output) by hand.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root.
- [`../codegen.ts`](../codegen.ts) — the codegen config that consumes this file.
- [`../lib/graphql/AGENTS.md`](../lib/graphql/AGENTS.md) — `execute()` and the typed hooks.
- [`../../AGENTS.md`](../../AGENTS.md) — rule B8 (schema is hand-maintained).
- [`../package.json`](../package.json) — `codegen` / `codegen:watch` scripts.
