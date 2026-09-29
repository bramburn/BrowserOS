# `tests/tools/filesystem/` — filesystem tool tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/tools/`.

## What's here

Tests for the agent-loop filesystem toolset in
[`../../../src/tools/filesystem/`](../../../src/tools/filesystem/AGENTS.md).
Every test here runs against a `mkdtemp` temp directory — **no browser, no
CDP port, no server boot** — which makes them the fastest tests in the suite
and the right place for anything filesystem-shaped.

## Contents

| File | Covers |
|---|---|
| `read.test.ts` | `createReadTool` plus the `MAX_READ_CHARS` / `MAX_READ_LINES` limits imported from `utils.ts`. |
| `write.test.ts` | `createWriteTool`. |
| `edit.test.ts` | `createEditTool`. |
| `bash.test.ts` | `createBashTool`. |
| `grep.test.ts` | `createGrepTool`. |
| `find.test.ts` | `createFindTool`. |
| `ls.test.ts` | `createLsTool`. |
| `utils.test.ts` | `FilesystemToolResult` shape and the shared truncation helpers. |

## Rules

**FF1 — These are the filesystem tools, not MCP tools.** They are not in
`src/tools/registry.ts` and are not reachable over `/mcp`; they are mounted by
the AI-SDK agent. Testing them here does not require the browser harness.

**FF2 — Assert on the typed result, not on stdout.** Each tool returns a
`FilesystemToolResult` from `../../../src/tools/filesystem/utils`. Import the
type and assert on it.

**FF3 — Import the limits, don't restate them.** `read.test.ts` imports
`MAX_READ_CHARS` and `MAX_READ_LINES` directly. If you hard-code the numbers
in the test, changing the limit silently breaks the suite.

**FF4 — `beforeEach`/`afterEach` with `mkdtemp` + `rm`.** Each test gets a
fresh directory and removes it afterwards. Shared state across tests in this
folder is a bug.

**FF5 — Bash is platform-sensitive.** `bash.test.ts` runs a real shell. Keep
the commands POSIX-portable and short; the group as a whole is macOS/Linux
oriented.

## Workflows

**Running:**
```bash
# from apps/server/
bun run test:tools:filesystem     # runs test:cleanup first
```

**Adding a test for a new filesystem tool:** 1. Create `<name>.test.ts`
beside the tool's `src/tools/filesystem/<name>.ts`. 2. `mkdir` a temp dir in
`beforeEach`, `rm -r` it in `afterEach`. 3. `const tool = create<Name>Tool(cwd)`
and assert on the returned result.

**Deciding where a test goes:** filesystem behaviour → here; browser tool
behaviour → `..`; pure helper in `src/lib` → `../../lib/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the browser-backed tool tests one level up.
- [`../../../src/tools/filesystem/AGENTS.md`](../../../src/tools/filesystem/AGENTS.md) — the tools under test.
- [`../__helpers__/AGENTS.md`](../../__helpers__/AGENTS.md) — the harness (not needed here).
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
