# `scripts/` — dev, build, and codegen scripts

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

The repository's Bun-run tooling layer: top-level entry scripts
(`run-test-suite.ts`, `run-bun-test.ts`, `generate-models.ts`,
`generate-docs.ts`, `patch-windows-exe.ts`) plus four subtrees — `build/`
(compiled-server release pipeline and the CLI artifact uploader), `codegen/`
(the CDP protocol generator that writes `packages/cdp-protocol`), `dev/` (the
CDP UI inspector and local dev launcher), and `eslint_rules/` (a local ESLint
plugin). `package.json` scripts such as `build:server`, `gen:cdp`, and
`test:all` point at these files.

## Contents

```
scripts/
├── tsconfig.json              ← extends ../tsconfig.json; outDir ./ignored, types ["bun"]
├── run-test-suite.ts          ← suite runner: all | main (server, agent, eval, build-script tests)
├── run-bun-test.ts            ← bun test wrapper; optional JUnit reporter via BROWSEROS_JUNIT_PATH
├── generate-models.ts         ← fetch models.dev/api.json → apps/agent/lib/llm-providers/models-dev-data.json
├── generate-docs.ts           ← talk to the built MCP server, write docs/tool-reference.md
├── patch-windows-exe.ts       ← rcedit metadata patch for a built .exe
├── build/                     ← server artifact pipeline (server.ts) + CLI uploader (cli.ts)
├── codegen/                   ← cdp-protocol.ts + lib/ emitters
├── dev/                       ← inspect-ui.ts (CDP UI inspector), start.ts, chat-cli.ts, mcp-test.sh
└── eslint_rules/              ← local-plugin.js, check-license-rule.js
```

## Rules

**SR1 — Bun only.** Every `.ts` here is `bun`-shebanged or run via
`bun run <script>` from `packages/browseros-agent`. No `node`, no `npx`.

**SR2 — Release scripts own secrets, dev scripts don't.** `build/` reads
`.env.production` files and R2 credentials; `dev/` must never require them.
Never add a credential read to a script that runs on a developer's machine.

**SR3 — `run-test-suite.ts` is the only place that defines the suite list.**
Adding a workspace's tests means adding an entry to `testSuites`, not a new
top-level npm script.

**SR4 — `generate-docs.ts` is stale by design; don't trust its imports.**
It imports `../build/src/cli` and `../build/src/tools/categories` and points
at `build/src/index.js` — paths from an older output layout. Fix the paths
before relying on it rather than assuming it works.

**SR5 — Don't add a barrel or an `index.ts` here.** Each script is a leaf
entry point; the only sharing happens inside `build/` and `codegen/lib/`.

**SR6 — `tsconfig.json` here has `outDir: ./ignored` — never commit build
output from this folder.** Typecheck with `bun run typecheck` instead.

## Workflows

**Running the whole suite:** `bun run test:all` (or `bun run test:main` for
tools + integration only). Build-script tests are included as the
`build script tests` entry, which shells out to
`./scripts/run-bun-test.ts ./scripts/build`.

**Building production server artifacts:** `bun run build:server`
(`scripts/build/server.ts --target=all`), `bun run build:server:ci` for
local-only zips, `bun run build:server:test` for a single darwin-arm64 target
with no upload. Details in `build/AGENTS.md`.

**Regenerating CDP types:** `CDP_PROTOCOL_JSON=<path> bun run gen:cdp`. See
`codegen/AGENTS.md`.

**Inspecting extension UI over CDP:** start `bun run dev:watch:new`, export
`BROWSEROS_CDP_PORT`, then `bun scripts/dev/inspect-ui.ts targets`. See
`dev/AGENTS.md`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`../CLAUDE.md`](../CLAUDE.md) — coding guidelines (`CLAUDE.md` § "Self-Testing UI Changes").
- [`build/AGENTS.md`](build/AGENTS.md) — release pipeline.
- [`codegen/AGENTS.md`](codegen/AGENTS.md) — CDP generator.
- [`dev/AGENTS.md`](dev/AGENTS.md) — dev launcher + CDP UI inspector.
- [`../packages/cdp-protocol/AGENTS.md`](../packages/cdp-protocol/AGENTS.md) — codegen output.
