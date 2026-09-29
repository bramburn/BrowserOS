# `graph/` — ignored scratch directory

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server`.

## What's here

This directory contains exactly one file: `.gitignore`, whose entire body is:

```
*
!.gitignore
```

There is no graph schema, no Drizzle config, and no code. The name is
misleading — the actual Drizzle schema lives in
[`../src/lib/db/schema/`](../src/lib/db/schema/) and the Drizzle Kit config
lives in [`../drizzle.config.ts`](../drizzle.config.ts).

## Contents

| Entry | Purpose |
|---|---|
| `.gitignore` | Ignores everything in this directory, including itself being un-ignorable. |

## Rules

**G1 — Treat `graph/` as untracked scratch space.** Anything you write here
is invisible to `git status` and will not be committed. Never place a
migration, schema definition, or source file here.

**G2 — Don't "fix" this by deleting the `.gitignore`.** The directory is
deliberately ignored at the repo level; removing the guard would surface
whatever local tooling happens to write there in your next commit.

**G3 — If you need a Drizzle schema, edit `../src/lib/db/schema/`.** Table
definitions are TypeScript (`schema/agents.ts`, `schema/oauth.ts`) and
`../drizzle.config.ts` points `schema` at `./src/lib/db/schema/index.ts`.

## Workflows

**Looking for the DB schema:** 1. Open
[`../src/lib/db/schema/index.ts`](../src/lib/db/schema/index.ts) — it
re-exports `./agents` and `./oauth`. 2. Read the two table files. 3. For
generated SQL, see `../src/lib/db/migrations/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`../src/lib/db/AGENTS.md`](../src/lib/db/AGENTS.md) — Drizzle client, schema, migrations.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
