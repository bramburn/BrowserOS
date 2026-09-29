# `src/lib/db/` — Drizzle + SQLite

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/`.

## What's here

The persistence layer: a `bun:sqlite` database wrapped by Drizzle, opened
once per process. `client.ts` does the low-level open + migrate;
`index.ts` holds the process-wide singleton (`initializeDb`, `getDbHandle`,
`getDb`, `closeDb`); `schema/` contains the two table definitions; and
`migrations/` holds the three Drizzle-generated SQL migrations plus the
`meta/` journal and snapshots. The database is per-install, under
`getDbPath()` from `../browseros-dir.ts`.

## Contents

```
db/
├── client.ts        ← openBrowserOsDatabase(), resolveMigrationsDir(), DbHandle
├── index.ts         ← initializeDb() / getDbHandle() / getDb() / closeDb()
├── schema/
│   ├── index.ts     ← re-exports ./agents and ./oauth
│   ├── agents.ts    ← agent_definitions table
│   └── oauth.ts     ← oauth_tokens table
└── migrations/
    ├── 0000_zippy_psylocke.sql       ← agent_definitions + indexes
    ├── 0001_lazy_orphan.sql
    ├── 0002_chemical_whirlwind.sql
    └── meta/
        ├── _journal.json             ← ordered list of applied tags
        ├── 0000_snapshot.json
        ├── 0001_snapshot.json
        └── 0002_snapshot.json
```

## Contents (schema)

| Table | File | Purpose |
|---|---|---|
| `agent_definitions` | `schema/agents.ts` | id, name, adapter, model_id, reasoning_effort, permission_mode, session_key (unique), pinned, adapter_config_json, created_at, updated_at. |
| `oauth_tokens` | `schema/oauth.ts` | Provider OAuth tokens, read/written by `../clients/oauth/token-store.ts`. |

## Rules

**DB1 — Drizzle only, never raw SQL against the runtime DB.** Query with
`getDb()` from `index.ts`. `client.ts` is the one place that calls
`sqlite.exec()` and only for PRAGMAs (`journal_mode = WAL`,
`foreign_keys = ON`).

**DB2 — Migrations are generated, not hand-written.**
`bunx drizzle-kit generate --config ../../../drizzle.config.ts` (or
`bun run db:generate` from `apps/server/`). Never edit a file in
`migrations/` or `migrations/meta/` by hand — the journal ordering and the
snapshots are what `drizzle-orm`'s `migrate()` trusts.

**DB3 — `initializeDb()` is called once, by `main.ts`.** It is idempotent:
a second call returns the existing handle. `getDbHandle()` throws if it was
never initialised, and `getDb()` returns the typed Drizzle instance.

**DB4 — Schema changes start in `schema/`.** Edit the table file, regenerate,
read the produced `.sql` to confirm it does what you intended, then update the
repos that query it (`../agents/db-agent-store.ts`,
`../clients/oauth/token-store.ts`).

**DB5 — Migrations resolve by resource, not by relative path.** `client.ts`
picks an explicit test path, the packaged `resourcesDir`, or the source tree.
When a new install flow ships a db, the migration folder must be reachable
through one of those.

## Workflows

**Adding a table or column:** 1. Edit `schema/<table>.ts` (or add a new file
and export it from `schema/index.ts`). 2. `bun run db:generate` from
`apps/server/`. 3. Read the new `migrations/000N_*.sql` — verify no
destructive rewrite of existing columns. 4. Update the consuming repo module.
5. `bun run test:lib` (or `bun run test:integration` for the VM/container
path).

**Opening an ad-hoc db in a test:** call `openBrowserOsDatabase({ dbPath,
migrationsDir, runMigrations })` from `client.ts` — it is exported for exactly
that, and `runMigrations: false` gives a raw handle.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/` conventions.
- [`../../../drizzle.config.ts`](../../../drizzle.config.ts) — schema path + migrations out dir.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
- [`../../../../tests/lib/db/AGENTS.md`](../../../tests/lib/db/AGENTS.md) — db tests.
