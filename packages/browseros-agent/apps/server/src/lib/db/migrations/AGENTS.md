# `src/lib/db/migrations/` — generated SQL

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/db/`.

## What's here

The checked-in Drizzle migrations and the `meta/` bookkeeping that
`drizzle-orm`'s migrator reads. Three migrations exist today, applied in the
order listed in `meta/_journal.json`. This directory is **generated output**:
the only correct way to change it is `drizzle-kit generate`.

## Contents

| File | Contents |
|---|---|
| `0000_zippy_psylocke.sql` | Creates `agent_definitions` plus its unique index on `session_key` and indexes on `updated_at` and `(adapter, updated_at)`. |
| `0001_lazy_orphan.sql` | Second migration in the series. |
| `0002_chemical_whirlwind.sql` | Third migration in the series. |
| `meta/_journal.json` | `version: 7`, `dialect: sqlite`, entries for tags `0000_zippy_psylocke`, `0001_lazy_orphan`, `0002_chemical_whirlwind` with `breakpoints: true`. |
| `meta/0000_snapshot.json`, `meta/0001_snapshot.json`, `meta/0002_snapshot.json` | Per-migration schema snapshots used to diff the next generation. |

## Rules

**MG1 — Never hand-edit anything in this directory.** Editing a `.sql` file
or a `meta/*.json` desynchronises the journal from the snapshots and the next
`generate` produces a broken diff. Change `../schema/` and regenerate.

**MG2 — `meta/_journal.json` ordering is the apply order.** Entries are keyed
by `idx` and `tag`; a new migration appends with the next `idx`. The names
(`000N_<adjective>_<noun>`) are Drizzle Kit's generated slugs — keep them as
generated, don't rename for readability.

**MG3 — Read the generated SQL before you commit it.** Drizzle has no
concept of data-preserving rename: a renamed column appears as drop + add. The
checklist on a new migration is: no accidental drop, no type widening to
something SQLite can't do, and every new index is intended.

**MG4 — Migrations must be reachable at runtime.** `../client.ts` resolves
the folder from an explicit test path, the packaged `resourcesDir`, or the
source tree. If you add a packaging path, verify it lands here too — a
migration the installed binary can't find is a boot failure.

**MG5 — `breakpoints: true` is expected.** Statements are separated by
`--> statement-breakpoint` markers; the migrator relies on them.

## Workflows

**Generating the next migration:** 1. Edit `../schema/<table>.ts`. 2. From
`apps/server/`, run `bunx drizzle-kit generate --config drizzle.config.ts`
(or `bun run db:generate`). 3. Read the new `000N_*.sql` and the updated
`meta/_journal.json`. 4. `bun run test:lib` / `bun run test:integration`.

**Checking what a given install has applied:** read `meta/_journal.json` and
compare against the migrations directory shipped in `resourcesDir`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Drizzle + SQLite conventions.
- [`./meta/AGENTS.md`](./meta/AGENTS.md) — the journal and snapshots.
- [`../schema/AGENTS.md`](../schema/AGENTS.md) — the tables these migrations build.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
