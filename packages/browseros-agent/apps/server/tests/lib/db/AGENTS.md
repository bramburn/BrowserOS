# `tests/lib/db/` — database layer tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/`.

## What's here

One file: `index.test.ts`, covering the process-wide db handle lifecycle in
[`../../../../src/lib/db/index.ts`](../../../src/lib/db/AGENTS.md) —
`initializeDb()` idempotency, `getDbHandle()`'s failure before init,
`getDb()`'s typed accessor, and `closeDb()`. The test opens a temporary
SQLite file with migrations applied; the install's real database is never
touched.

## Contents

| File | Covers |
|---|---|
| `index.test.ts` | `initializeDb`, `getDbHandle`, `getDb`, `closeDb` — and the `DbError`-style guard that `getDbHandle()` throws when the db was never initialised. |

## Rules

**DB-T1 — Temp database, always.** Point `dbPath` at a `mkdtemp` file and
`migrationsDir` at `../../../../src/lib/db/migrations` (or pass
`runMigrations: false` where the test doesn't need tables). A test that
resolves the install's `getDbPath()` is a data-loss bug.

**DB-T2 — `closeDb()` in `afterEach`.** The handle is a module-level
singleton; leaving it open leaks a file lock and makes the next test in the
same process see stale state.

**DB-T3 — Migration-dependent tests run migrations.** `openBrowserOsDatabase`
applies them by default. If a test needs an empty schema, pass
`runMigrations: false` explicitly rather than deleting rows.

**DB-T4 — Query through the Drizzle instance, not `sqlite.exec()`.** The
point of these tests is that the typed layer works. `client.ts` is the only
place raw `exec` is appropriate (PRAGMAs).

**DB-T5 — Schema changes need a migration first.** If a test fails on a
missing column, generate the migration rather than creating the column
ad-hoc in the test.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Adding a test for a new repo module** (e.g. a new table's query layer):
1. `mkdtemp` a db path. 2. `initializeDb({ dbPath, migrationsDir })`. 3.
Exercise the repo. 4. `closeDb()` in `afterEach`.

**After a schema change:** `bunx drizzle-kit generate`, confirm
`../../../../src/lib/db/migrations/meta/_journal.json` gained one entry, then
re-run this test.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/` conventions.
- [`../../../../src/lib/db/AGENTS.md`](../../../src/lib/db/AGENTS.md) — the module under test.
- [`../../../../src/lib/db/migrations/AGENTS.md`](../../../src/lib/db/migrations/AGENTS.md) — the migrations applied here.
- [`../../__helpers__/AGENTS.md`](../../__helpers__/AGENTS.md) — the harness.
