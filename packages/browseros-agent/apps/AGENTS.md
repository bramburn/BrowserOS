# `apps/` — the four runnable BrowserOS apps

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.
> Cross-repo architecture:
> [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).
> Build pipeline: [`../../../AGENTS-build.md`](../../../AGENTS-build.md).

## What's here

Four independent products, one directory each. Three are Bun + TypeScript
apps that ride the monorepo `bun.lock` and are addressed by the root
`package.json` scripts; one is a standalone Go module with its own
`go.mod`, `go.sum`, and `Makefile`. They are siblings, not layers —
nothing in one app imports another app.

## Contents

```
apps/
├── server/     ← Bun MCP server (the open-source BrowserOS protocol)
│                 AGENTS.md, package.json, tsconfig.json,
│                 drizzle.config.ts, src/, tests/, graph/
├── agent/      ← WXT browser extension
│                 AGENTS.md, package.json, wxt.config.ts,
│                 web-ext.config.ts, codegen.ts, components.json,
│                 entrypoints/, components/, lib/, hooks/,
│                 schema/schema.graphql, styles/, public/, assets/
├── cli/        ← Go CLI — module `browseros-cli`, binary `browseros-cli`
│                 AGENTS.md, main.go, go.mod, go.sum, Makefile,
│                 integration_test.go, cmd/, mcp/, config/, output/,
│                 analytics/, update/, scripts/, npm/
└── eval/       ← Bun eval / benchmark harness
                  AGENTS.md, package.json, tsconfig.json, README.md,
                  src/, tests/, configs/, data/, scripts/
```

Each of the four has its own `AGENTS.md`, its own `.gitignore`, and its
own `.env.example`.

## Rules

### A1 — `apps/` holds products, `packages/` holds libraries
Keep the import graph one-way: `apps/* → packages/*`. Never
`packages/* → apps/*`, and never `apps/x → apps/y`. Shared code that
two apps need belongs in `packages/shared/`, not in whichever app
happened to need it first.

### A2 — Only `cli/` is a Go module under `apps/`
`cli/go.mod` declares `module browseros-cli`. The other three apps have
no `package.json` in `cli` terms — they are Bun workspaces. The
monorepo `workspaces` field is `["apps/*", "packages/*"]`, which means
`apps/cli` itself is **not** a workspace member (it has no
`package.json`) and `apps/cli/npm` is two levels deep so it is not
either. Do not add a `package.json` to `apps/cli` root.

### A3 — The other two Go binaries live in `tools/`, not here
`tools/dev/` is `module browseros-dev` and `tools/dogfood/` is
`module browseros-dogfood`. They are developer tooling with their own
`go.mod`, `Makefile`, and `cmd/` trees; they do not import
`browseros-cli`. Don't reach across from an app into `tools/`.

### A4 — Each app owns its env surface
Every app has its own `.env.example`. Don't hoist env var names to the
monorepo root or to `apps/` — the four apps are run independently in
production, often without the monorepo present.

### A5 — Ports and URLs are constants, not literals
Cross-app endpoints (server port, CDP port, extension URL) live in
`packages/shared/src/constants/`. `apps/cli` in particular takes its
server URL from the user's config, not from a hard-coded port — see
`cli/cmd/root.go`.

## Workflows

### "I'm adding a new app"
1. Create `apps/<name>/` with its own `package.json` (Bun) or `go.mod`
   (Go) and its own `AGENTS.md`.
2. Match the neighbours: a `.gitignore`, a `.env.example`, a `README.md`
   documenting how to build and run it.
3. If it's Bun, `apps/*` already globs it into the workspace — no root
   `package.json` edit needed beyond scripts.
4. If it's Go, do not add it to the Bun workspace. Give it a `Makefile`
   mirroring `cli/Makefile` — note there is **no `build` target**: the
   default target writes the binary, then `install`, `clean`, `vet`,
   `test`, `release`, `npm-version`, `npm-publish`.
5. Add it to the `apps/` tree in
   [`../AGENTS.md`](../AGENTS.md) and to
   [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).

### "I'm adding a capability the extension and the CLI both need"
1. Server-side behaviour goes in `server/src/` and is exposed as an MCP
   tool (register in `server/src/tools/registry.ts`).
2. The extension gets UI in `agent/components/` / `agent/entrypoints/`.
3. The CLI gets a thin wrapper in `cli/cmd/` calling the same tool via
   `mcp.Client.CallTool` — the CLI owns no browser logic of its own.
4. Shared *types* only go in `packages/shared/`. Shared *behaviour*
   stays in the server.

### "I'm not sure which app owns this behaviour"
- HTTP/agent loop/DB/CDP → `server/`.
- Side-panel UI, browser pages, extension settings → `agent/`.
- Terminal entry point, config file, self-update, npm packaging →
  `cli/`.
- Scoring an agent against a task suite → `eval/`.
- Build/serve/reset loops for local development → `tools/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`server/AGENTS.md`](server/AGENTS.md) — MCP server internals.
- [`agent/AGENTS.md`](agent/AGENTS.md) — WXT extension internals.
- [`cli/AGENTS.md`](cli/AGENTS.md) — Go CLI.
- [`eval/AGENTS.md`](eval/AGENTS.md) — eval harness.
- [`../packages/shared/AGENTS.md`](../packages/shared/AGENTS.md) — shared constants and types.
- [`../CLAUDE.md`](../CLAUDE.md) — coding guidelines (authoritative for the Bun apps).
