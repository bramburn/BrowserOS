# `src/lib/db/migrations/meta/` — Drizzle journal and snapshots

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/db/migrations/`.

## What's here

Generated bookkeeping for the migrator. `_journal.json` is the ordered list
of applied migration tags; the three `000N_snapshot.json` files are
structural snapshots of the schema as of each migration, which is how
Drizzle Kit computes the diff for the *next* migration. This folder is never
edited by hand.

## Contents

| File | Purpose |
|---|---|
| `_journal.json` | `{ version: "7", dialect: "sqlite", entries: [...] }`. One entry per migration with `idx`, `version`, `when`, `tag`, `breakpoints: true`. |
| `0000_snapshot.json` | Schema state after `0000_zippy_psylocke` (`agent_definitions` + indexes). |
| `0001_snapshot.json` | Schema state after `0001_lazy_orphan`. |
| `0002_snapshot.json` | Schema state after `0002_chemical_whirlwind` (current). |

## Rules

**MM1 — Treat this folder as read-only.** Every byte is produced by
`drizzle-kit generate`. A manual edit here produces a migration whose SQL
disagrees with its snapshot, and the next generation will emit a duplicate or
a destructive diff.

**MM2 — `entries` is append-only.** The migrator applies in `idx` order; you
may not reorder, remove, or re-tag an entry. Rolling back means writing a new
forward migration, not editing the journal.

**MM3 — `when` is a build timestamp, not a semantic version.** Use the
migration `tag` when reasoning about order, not the index numbers in the
filename.

**MM4 — A missing snapshot breaks the next `generate`.** If you ever find
yourself reconstructing this folder, regenerate from `../../schema/` instead
of writing JSON by hand.

## Workflows

**Finding which migration introduced a column:** grep the `.sql` files in
`..` for the table/column name, then read the matching `000N_snapshot.json`
to see the before/after structure.

**After a schema change:** 1. Edit `../../schema/<table>.ts`. 2.
`bunx drizzle-kit generate`. 3. Confirm this folder gained exactly one
`_journal.json` entry and one snapshot. 4. Read the generated SQL.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the migration SQL files.
- [`../schema/AGENTS.md`](../../schema/AGENTS.md) — table definitions (the input to generation).
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
