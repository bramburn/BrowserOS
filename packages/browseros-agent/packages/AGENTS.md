# `packages/` — shared library workspaces

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Three Bun workspace packages that `apps/*` depend on and that must never
depend on `apps/*`. `@browseros/shared` holds cross-cutting constants,
types, and zod schemas; `@browseros/cdp-protocol` holds the generated
Chrome DevTools Protocol types and API wrappers; `@browseros/build-tools`
publishes release artifacts to Cloudflare R2 and owns the Lima VM template.

They are **not** interchangeable in shape. `shared` and `cdp-protocol`
publish an `exports` map with one subpath per file (each carrying `types`
and `default`) and have no `index.ts` barrel. `build-tools` is
`"private": true`, has **no `exports` map at all**, and is reached only by
path from the root scripts and from the `bun --filter` CLI. `shared` also
has a real runtime dependency — `zod@^3.24.2` — because its `schemas/`
and `constants/role-aware-agents` files build schemas at import time. It
is not a type-only package.

## Contents

```
packages/
├── shared/                       ← @browseros/shared — constants, types, schemas
│ ├── AGENTS.md, package.json, tsconfig.json      (no README.md)
│ └── src/{constants,types,schemas,sentry}/
├── cdp-protocol/                 ← @browseros/cdp-protocol — generated CDP bindings
│ ├── AGENTS.md, README.md, package.json, tsconfig.json
│ └── src/generated/{domains,domain-apis}/ + protocol-api.ts, create-api.ts
└── build-tools/                  ← @browseros/build-tools — R2 uploads + Lima template
    ├── AGENTS.md, README.md, package.json, tsconfig.json, .env.sample
    └── scripts/{,common/}, template/, tests/
```

## Rules

**PK1 — One-way import graph.** `apps/* → packages/*`, never the reverse.
Neither `shared` nor `cdp-protocol` may import from `apps/`; `build-tools`
is the one package that reads app files (`apps/server/.env.production`,
`apps/cli/scripts/install.sh`) but only as *paths on disk* at build time, not
as TypeScript imports.

**PK2 — No `index.ts` barrel anywhere in this tree.** In `shared` and
`cdp-protocol`, each `package.json` lists one `exports` entry per file,
each with both `types` and `default`. Adding a file without adding its
`exports` entry means nothing can import it. `build-tools` is the
exception: it is `private` with **no `exports` map** — don't "fix" it by
adding one, and don't route imports to it.

**PK3 — Deep imports, always.** `import { DEFAULT_PORTS } from
'@browseros/shared/constants/ports'`, never
`from '@browseros/shared'` — there is no root export to import from.

**PK4 — `cdp-protocol/src/generated/` is generated output.** It carries
`// ── AUTO-GENERATED from CDP protocol. DO NOT EDIT. ──` headers. Fix the
emitters in `scripts/codegen/`, never the generated files.

**PK5 — `build-tools` is macOS/R2-facing.** Its scripts need real R2
credentials (`R2_ACCOUNT_ID`, `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY`,
`R2_BUCKET`) and a `zip` on PATH. Never add a step that requires those
during a normal `bun test`.

## Workflows

**Adding a new shared constant:** 1. Put it in the matching file under
`shared/src/constants/`. 2. Add the `./constants/<name>` entry to
`packages/shared/package.json` `exports` with `types` + `default`. 3. Import
it as `@browseros/shared/constants/<name>`. 4. Run
`bun run typecheck` from `packages/browseros-agent`.

**Regenerating CDP types after a Chromium protocol bump:** 1. Build Chromium
so `protocol.json` exists. 2. `CDP_PROTOCOL_JSON=<path> bun run gen:cdp`.
3. Review the regenerated `packages/cdp-protocol/src/generated/**` plus its
`package.json` exports — the codegen rewrites the export list too.

**Adding a release artifact to a server bundle:** 1. Add a rule to
`scripts/build/config/server-prod-resources.json` with `source.type` of
`r2` or `local`, a `destination`, and optional `os`/`arch` filters. 2. Upload
the R2-backed object with `bun --filter @browseros/build-tools run upload`.
3. Verify with `bun run build:server:ci`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`../CLAUDE.md`](../CLAUDE.md) — coding guidelines (authoritative).
- [`shared/AGENTS.md`](shared/AGENTS.md) — shared constants & types.
- [`cdp-protocol/AGENTS.md`](cdp-protocol/AGENTS.md) — generated CDP bindings.
- [`build-tools/AGENTS.md`](build-tools/AGENTS.md) — R2 uploads & Lima template.
- [`../scripts/AGENTS.md`](../scripts/AGENTS.md) — the codegen and build scripts.
