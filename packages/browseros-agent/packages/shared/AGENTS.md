# `packages/shared/` — Shared constants & types

> Sub-package AGENTS file. The single source of truth for magic numbers,
> paths, URLs, log types, and shared configuration. **Per architecture
> Rule R2**, all shared magic constants live here — never inline in
> apps/. For the cross-repo architecture see
> [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).
> For the parent monorepo see [`../../AGENTS.md`](../../AGENTS.md).

## What's here

A pure-TypeScript package with no runtime dependencies (other than
type-only). Exports from named files (no `index.ts` per Rule R3):

```
packages/shared/
├── package.json
├── tsconfig.json
├── src/
│ ├── constants/
│ │ ├── ports.ts                    ← DEFAULT_PORTS, TEST_PORTS
│ │ ├── timeouts.ts                 ← TIMEOUTS
│ │ ├── limits.ts                   ← RATE_LIMITS, AGENT_LIMITS, etc.
│ │ ├── urls.ts                     ← EXTERNAL_URLS
│ │ ├── paths.ts                    ← PATHS (file-system paths)
│ │ └── exit-codes.ts               ← EXIT_CODES
│ ├── types/
│ │ └── logger.ts                   ← LoggerInterface, LogLevel
│ └── ...
└── README.md
```

## Opinionated rules

### H1 — One file per concern
Don't lump ports + timeouts + limits into a single `constants.ts`.
Each gets its own file. This keeps imports surgical:
`import { DEFAULT_PORTS } from '@browseros/shared/constants/ports'`.

### H2 — Use `const` objects, not enums
TypeScript enums are footguns (no isolatedModules, awkward type
narrowing). Use `as const` objects with derived types:

```typescript
export const TIMEOUTS = {
 SHORT: 5_000,
 MEDIUM: 30_000,
 LONG: 120_000,
} as const

export type Timeout = typeof TIMEOUTS[keyof typeof TIMEOUTS]
```

### H3 — `package.json` exports are mandatory
Each export must declare both `types` and `default`:

```json
"exports": {
 "./constants/ports": {
 "types": "./src/constants/ports.ts",
 "default": "./src/constants/ports.ts"
 }
}
```

The bundler relies on `types` for build-time resolution.

### H4 — No app-specific constants here
If a constant is used by only one app, it lives in that app's
`constants.ts` (or co-located). Only cross-app constants belong in
`shared/`. When in doubt, start app-local; promote to `shared/`
later.

### H5 — Backwards-compat matters
Once a constant is in `shared/`, changing its value can break callers
silently. Bump the value with care; if a value needs to change per
deployment, make it a config field instead.

## Opinionated workflow

### "I'm adding a new constant"
1. Pick the right file:
 - Ports? `constants/ports.ts`
 - Timeouts? `constants/timeouts.ts`
 - Limits / rate-limit? `constants/limits.ts`
 - External URLs (CDN, models.dev, etc.)? `constants/urls.ts`
 - File-system paths? `constants/paths.ts`
 - Process exit codes? `constants/exit-codes.ts`
2. If none fit, add a new file under `constants/`.
3. Add the constant with `as const`.
4. Add a type alias if it makes sense.
5. **Update `package.json` `exports`** to expose the new file.
6. Run `bun run typecheck` from `packages/browseros-agent`.

### "I'm changing an existing constant"
1. Find every caller: `grep -r '<OLD>' packages/browseros-agent/apps packages/browseros-agent/packages`.
2. Update each caller. If the value is semantic (e.g. timeout meaning
 changed), call this out in the PR description.
3. If the change is breaking, version-bump `package.json`.

### "I'm adding a new type"
1. Add at `types/<name>.ts`.
2. Use `interface` for object shapes, `type` for unions/aliases.
3. Update `package.json` `exports`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md) — overall map (R2: shared magic constants).
- [`../../CLAUDE.md`](../../CLAUDE.md) — coding guidelines.
