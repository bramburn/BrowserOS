# `src/lib/db/schema/` — Drizzle table definitions

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/db/`.

## What's here

The TypeScript source of truth for the BrowserOS database. Two tables, each
in its own file, re-exported by `index.ts`. `drizzle.config.ts` points
Drizzle Kit at `schema/index.ts`, so adding a file here without exporting it
from `index.ts` means Drizzle Kit will not see the table.

## Contents

| File | Table | Notes |
|---|---|---|
| `index.ts` | — | `export * from './agents'` and `export * from './oauth'`. The only entry point Drizzle Kit reads. |
| `agents.ts` | `agent_definitions` | `id`, `name`, `adapter`, `model_id`, `reasoning_effort`, `permission_mode` (default `approve-all`), `session_key` (unique), `pinned`, `adapter_config_json`, `created_at`, `updated_at`. Indexed on `updated_at` and `(adapter, updated_at)`. |
| `oauth_tokens` | `oauth_tokens` | Defined in `oauth.ts`; read/written only by `../../clients/oauth/token-store.ts`. |

## Rules

**SC1 — Edit schema here first, then generate.** Never hand-write the SQL in
`../migrations/`. Run `bunx drizzle-kit generate` (or `bun run db:generate`
from `apps/server/`) and read the produced file.

**SC2 — New table file must be exported from `index.ts`.** The drizzle config
uses `./src/lib/db/schema/index.ts`; an unexported file is invisible to
`drizzle-kit` *and* to the `schema` object passed to `drizzle()` in
`../client.ts`, so the typed query builder won't know the table either.

**SC3 — Don't rename or drop a column in place.** Drizzle generates a
drop+add pair for renames, which silently destroys data. Add a new column,
backfill, then drop in a later migration.

**SC4 — Timestamps are integer epoch columns.** Follow the existing
`created_at` / `updated_at` shape rather than introducing a date type SQLite
doesn't have.

**SC5 — Serialise structured values as JSON text.** The precedent is
`adapter_config_json`; a `text` column holding `JSON.stringify` output, parsed
in the repo layer.

## Workflows

**Adding a table:** 1. Create `schema/<name>.ts` with `sqliteTable(...)`. 2.
Export it from `index.ts`. 3. `bun run db:generate`. 4. Review the generated
`../migrations/000N_*.sql`. 5. Add the repo module that queries it.

**Adding an index:** declare it on the table definition so it lands in the
same migration as the column.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Drizzle + SQLite conventions.
- [`../client.ts`](../client.ts) — `drizzle(sqlite, { schema })`.
- [`../../../drizzle.config.ts`](../../../../drizzle.config.ts) — `schema` and `out` paths.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
