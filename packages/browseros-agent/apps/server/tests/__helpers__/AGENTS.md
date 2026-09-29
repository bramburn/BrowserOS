# `tests/__helpers__/` — test harness

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

Everything the test suite borrows: process management for a real BrowserOS
and its server, the port/profile planner, the `bun test` group runner, the
environment preload, POSIX cleanup, and fakes for external binaries
(`limactl`, `ssh`). `index.ts` is the public surface — tests should import
from there, not reach into individual files.

## Contents

| File | Purpose |
|---|---|
| `index.ts` | The barrel. Re-exports `ensureBrowserOS`, `cleanupBrowserOS`, `withBrowser`, `cleanupWithBrowser`, `withMcpServer`, `asToolResult`, `html`, `killProcessOnPort`, and the `McpContentItem` / `TypedCallToolResult` types. |
| `test-env.ts` | Preloaded before every run: sets `NODE_ENV=test`, a temp `BROWSEROS_DIR`, and per-PID test ports (`BROWSEROS_TEST_CDP_PORT` / `_SERVER_PORT` / `_EXTENSION_PORT`, base `36000 + (pid % 1000) * 20`). |
| `with-browser.ts` | `withBrowser(cb)` / `cleanupWithBrowser()`. Async-mutex guarded, caches one `CdpBackend` + `Browser` across tests, and exposes `execute(tool, args)`. |
| `test-runtime.ts` | `createTestRuntimePlan()` — picks ports, binary, temp user-data dir, headless flag. Defaults `BROWSEROS_BINARY` to `/Applications/BrowserOS.app/Contents/MacOS/BrowserOS`. |
| `browser.ts` | Low-level browser process management: `spawnBrowser`, `killBrowser`, `getBrowserState`, `BrowserConfig`. |
| `server.ts` | Low-level MCP server process management (spawns `src/index.ts`). |
| `setup.ts` | The orchestrator: `ensureBrowserOS()` brings up server + browser + extension. |
| `utils.ts` | `killProcessOnPort` (lsof/kill), `withMcpServer(cb)` (a real MCP client over `http://127.0.0.1:<port>/mcp`), `asToolResult`, `html`. |
| `run-test-group.ts` | The `bun run test:<group>` runner: discovers test directories, builds the `bun test` command, injects `NODE_ENV=test`. |
| `acl-fixture-runner.ts` | Scores one ACL fixture with `LOG_LEVEL=silent` and `ACL_EMBEDDING_DISABLE` cleared. |
| `fake-limactl.ts` | Writes an executable stub `limactl` into a temp dir. |
| `fake-ssh.ts` | Writes an executable stub `ssh` into a temp dir. |
| `cleanup.sh` | Bash script that kills test processes on the test ports and removes orphaned temp dirs. |

## Rules

**H1 — Import from `index.ts`.** The barrel is the supported surface;
`with-browser.ts` and `utils.ts` are implementation detail. If a helper
isn't exported, export it there rather than importing the file directly.

**H2 — One browser per test process.** `with-browser.ts` caches the
`CdpBackend`/`Browser` behind an `async-mutex` and reuses it across tests.
Do not spawn a second browser inside a test; call `cleanupWithBrowser()` only
in teardown.

**H3 — `test-env.ts` is a preload, not an import.** It is passed to `bun test`
as `--preload` by `run-test-group.ts` and mutates `process.env` before
anything else runs. Importing it from a test file is wrong.

**H4 — The group runner is the only supported way to run the suite.**
`package.json` scripts call `run-test-group.ts`; it discovers directories
dynamically, so adding a test directory is enough. `console.log` is fine here
— this is a script, not server code.

**H5 — Fakes stub the binary, not the module.** `fake-limactl.ts` and
`fake-ssh.ts` write real executables into a temp dir so the production code
path (including argument construction) is exercised. Don't `mock()` the
module instead.

**H6 — POSIX-only helpers.** `killProcessOnPort` uses `lsof`/`kill`;
`cleanup.sh` is bash; the default binary path is macOS. Browser-backed groups
are macOS/Linux oriented; on Windows set `BROWSEROS_BINARY` and expect the
port-kill helper to fail silently.

## Workflows

**Writing a browser-backed test:**
```ts
import { describe, it } from 'bun:test'
import { withBrowser } from '../__helpers__'

await withBrowser(async ({ browser, execute }) => {
  await execute(take_snapshot, { page: ... })
})
```

**Running a group:** `bun run test:tools` / `test:api` / `test:lib` /
`test:agent` / `test:browser` / `test:root` / `test:integration` / `test:core`
/ `test:all`, all from `apps/server/`.

**Adding a fixture-driven test:** put the data in `../__fixtures__/` and the
runner in `acl-fixture-runner.ts` (or a sibling) — not in the test file.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and its group rules.
- [`../__fixtures__/AGENTS.md`](../__fixtures__/AGENTS.md) — static fixtures.
- [`../tools/AGENTS.md`](../tools/AGENTS.md) — the group that depends most on this harness.
- [`../../AGENTS.md`](../../AGENTS.md) — `apps/server/` MCP server internals.
