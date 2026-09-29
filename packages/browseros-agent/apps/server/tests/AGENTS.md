# `tests/` — server test suite

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/`.

## What's here

The whole test suite for the Bun MCP server, mirroring the `src/` tree. It is
split by *how much environment a test needs*, not just by subject: `tools/`
drives a real Chromium over CDP, `lib/` and `api/` are offline mocks, and
`integration/` needs the VM/container toolchain. `__helpers__/` holds the
harness; `__fixtures__/` holds static fixtures. Test files use `bun:test`
(`describe` / `it` / `expect`) with `node:assert` — never Jest or Vitest.

## Contents

```
tests/
├── __helpers__/    ← harness: browser + server lifecycle, test group runner, fakes
├── __fixtures__/   ← static fixtures: snapshot HTML, server hooks, acl/*.json
├── tools/          ← MCP tool tests — REQUIRE a real browser / CDP port
│   └── filesystem/ ← offline agent-loop filesystem tool tests
├── browser/
│   └── backends/   ← cdp.test.ts (MockWebSocket, no browser)
├── api/            ← offline route + service tests (factories called directly)
│   ├── routes/
│   └── services/{agents,klavis}/
├── lib/            ← offline unit tests for src/lib/**, mirroring the tree
│   ├── agents/{hermes,runtime}/
│   ├── clients/oauth/
│   ├── container/managed/
│   ├── db/
│   └── vm/
├── agent/          ← agent loop + compaction tests (offline)
├── integration/    ← vm-smoke.test.ts — needs Lima/podman
└── *.test.ts       ← root group: config, index, main, build, monitoring-*, …
```

## Rules

**T1 — Pick the group by environment need, not by subject.** A test that
needs a live Chromium belongs under `tools/`. A test that can run against a
fake belongs under `lib/`, `api/`, or `agent/`. Moving a passing offline test
into `tools/` buys nothing and makes CI slower.

**T2 — `tools/` tests use `withBrowser()` from `__helpers__/with-browser.ts`.**
It spawns/reuses a real BrowserOS process, connects `CdpBackend`, and hands
you `{ browser, execute }`. Never construct a `Browser` directly in a tool
test — you will leak a browser process.

**T3 — Route tests call the factory directly.** `tests/api/routes/*.test.ts`
import `createHealthRoute` etc. and exercise the returned `Hono` app. No
server boot, no port. That's what makes them fast and deterministic.

**T4 — The group runner is `__helpers__/run-test-group.ts`.** `package.json`
maps `test:tools`, `test:api`, `test:browser`, `test:lib`, `test:agent`,
`test:root`, `test:integration`, `test:core`, and `test:all` onto it. It
discovers directories dynamically, so a new test directory is picked up
automatically. Don't add a bespoke npm script for a single file.

**T5 — `__fixtures__/` is for data, `__helpers__/` is for behaviour.** A new
captured element or HTML snapshot goes in `__fixtures__/`; a new spawn/teardown
utility goes in `__helpers__/`.

**T6 — The harness is POSIX-oriented.** `__helpers__/utils.ts` shells out to
`lsof` / `kill` and `test-runtime.ts` defaults `BROWSEROS_BINARY` to
`/Applications/BrowserOS.app/...`, and `cleanup.sh` is a bash script. Browser
and integration groups are macOS/Linux oriented.

## Workflows

**Running everything:**
```bash
# from apps/server/
bun run test            # = test:all
bun run test:core       # agent + api + root
bun run test:lib        # offline only
bun run test:tools      # needs a browser
```

**Running one file:**
`bun --env-file=.env.development test tests/<path>/<file>.test.ts`

**Adding a test:** 1. Choose the directory by environment need (T1). 2.
Import the narrowest helper from `__helpers__/index.ts` — it re-exports
`withBrowser`, `cleanupWithBrowser`, `withMcpServer`, `asToolResult`, `html`,
`killProcessOnPort`, `ensureBrowserOS`, `cleanupBrowserOS`. 3. Name the file
`<subject>.test.ts` (kebab-case).

**Adding a new test directory:** create it and add tests; the group runner
picks it up. Only touch `package.json` if you want a dedicated `test:<name>`
alias.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`../tests/__helpers__/AGENTS.md`](../tests/__helpers__/AGENTS.md) — the test harness.
- [`../tests/tools/AGENTS.md`](../tests/tools/AGENTS.md) — browser-backed tool tests.
- [`../tests/lib/AGENTS.md`](../tests/lib/AGENTS.md) — offline unit tests.
- [`../../CLAUDE.md`](../../../CLAUDE.md) — `bun test`, never Jest/Vitest.
