# `packages/shared/src/` — shared source root

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/shared`. The package-level file
> [`../AGENTS.md`](../AGENTS.md) already exists; this one covers `src/`.

## What's here

The source root of `@browseros/shared`, organised into four folders that map
1:1 onto the `package.json` `exports` prefixes: `constants/`, `types/`,
`schemas/`, and `sentry/`. Every file is a leaf module — there is no
`index.ts` and no file imports a sibling except `constants/role-aware-agents.ts`
(which imports one type from `../types/role-aware-agents`). The package
`tsconfig.json` sets `rootDir: "src"` and includes `src/**/*`.

## Contents

```
shared/src/
├── constants/   ← 8 files: ports, timeouts, limits, urls, paths, exit-codes, hermes, role-aware-agents
├── types/       ← 3 files: logger, server-config, role-aware-agents (pure types, no zod)
├── schemas/     ← 3 files: llm, ui-stream, browser-context (zod; types derived via z.infer)
└── sentry/      ← 1 file: sanitize.ts (sanitize / sanitizeEvent)
```

## Rules

**SS1 — One export entry per file, added to `package.json` in the same
change.** A new file that isn't in `exports` cannot be imported by any
consumer. Each entry needs both `types` and `default`.

**SS2 — `constants/` = `as const` objects, `types/` = interfaces,
`schemas/` = zod.** Don't put a zod schema in `types/` or a plain interface in
`schemas/`; the split is what lets consumers import types with `import type`
and pay nothing at runtime.

**SS3 — Derive types, don't duplicate them.** In `schemas/` every schema is
followed by `export type X = z.infer<typeof XSchema>`. In `constants/` the
pattern is `export type TimeoutKey = keyof typeof TIMEOUTS`.

**SS4 — No imports from `apps/`.** The only intra-package import is
`constants/role-aware-agents.ts → types/role-aware-agents.ts`. The graph must
stay `apps → packages`.

**SS5 — No barrel file, ever.** `src/index.ts` would defeat the per-file
`exports` map and pull every constant into every bundle.

## Workflows

**Adding a value:** 1. Pick the folder by kind (constant, type, zod schema,
Sentry helper). 2. Create `src/<folder>/<name>.ts` in kebab-case. 3. Add the
`exports` entry to `../package.json`. 4. Import as
`@browseros/shared/<folder>/<name>`.

**Typechecking:** `bun run typecheck` from `packages/browseros-agent` (runs
`tsc --noEmit` per workspace), or `tsc --noEmit` from this package.

**Tracing a constant to its callers:** every consumer imports the deep
subpath, so `grep -r "DEFAULT_PORTS" packages/browseros-agent` finds all of
them without any index re-export hiding the list.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `@browseros/shared` package scope (pre-existing).
- [`../../AGENTS.md`](../../AGENTS.md) — the `packages/` folder.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — coding guidelines (kebab-case, extensionless imports).
- [`constants/AGENTS.md`](constants/AGENTS.md) — ports, timeouts, limits, urls, paths, exit codes.
