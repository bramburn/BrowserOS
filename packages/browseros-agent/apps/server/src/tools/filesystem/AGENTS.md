# `src/tools/filesystem/` — agent-loop filesystem toolset

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/tools/`.

## What's here

Seven filesystem tools (`read`, `write`, `edit`, `bash`, `grep`, `find`,
`ls`) that give the AI agent a shell-ish working directory. They are built by
`build-toolset.ts` into a Vercel AI SDK `ToolSet` and consumed by exactly one
caller: `../../agent/ai-sdk-agent.ts`. Each factory is
`create<Name>Tool(cwd)` and each returns a tool whose result is a
`FilesystemToolResult` defined in `utils.ts`.

## Contents

| File | Purpose |
|---|---|
| `build-toolset.ts` | `buildFilesystemToolSet(cwd)` — assembles the 7 tools under the `filesystem_*` names. |
| `read.ts` | `createReadTool` — reads files with `MAX_READ_CHARS` / `MAX_READ_LINES` caps. |
| `write.ts` | `createWriteTool` — create/overwrite a file. |
| `edit.ts` | `createEditTool` — string-replace edits. |
| `bash.ts` | `createBashTool` — runs a shell command in the working dir. |
| `grep.ts` | `createGrepTool` — content search. |
| `find.ts` | `createFindTool` — filename / glob search. |
| `ls.ts` | `createLsTool` — directory listing. |
| `utils.ts` | Shared result type plus the read-size limits the tests assert on. |

## Rules

**FS1 — This folder is NOT the MCP registry.** Nothing here is registered in
`../registry.ts`, so these tools are invisible to an MCP client connecting to
`/mcp`. They exist for the in-process AI-SDK agent only. Adding a file here
does not make it a browser tool; adding a browser tool does not touch
`build-toolset.ts`.

**FS2 — Add new tools in two places.** Create `<name>.ts` with a
`create<Name>Tool(cwd)` factory, then add the key to the object literal in
`build-toolset.ts` using the `filesystem_<name>` naming convention.

**FS3 — `cwd` is the sandbox root, not a per-call argument.** It is bound at
toolset construction. Callers that need a different scope construct a new
toolset.

**FS4 — Output caps are constants, not magic numbers.** `MAX_READ_CHARS` and
`MAX_READ_LINES` are exported from `utils.ts` and asserted directly by
`tests/tools/filesystem/read.test.ts`. Truncation is a feature — the
compaction layer in `../../agent/compaction/` also bounds tool output.

**FS5 — Tests here are pure and offline.** Unlike the browser tool tests,
these run against a `mkdtemp` temp directory and need no CDP port
(`bun run test:tools:filesystem`).

## Workflows

**Adding a filesystem tool:** 1. Create `<name>.ts` exporting
`create<Name>Tool(cwd: string)`. 2. Return a `FilesystemToolResult` from
`utils.ts`. 3. Register it in `build-toolset.ts` (FS2). 4. Add
`tests/tools/filesystem/<name>.test.ts` using `mkdtemp` + `afterEach` cleanup.

**Changing read truncation:** edit the limits in `utils.ts`, then update
`tests/tools/filesystem/read.test.ts`, which imports those exact constants.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` conventions and the registry rule.
- [`../../agent/AGENTS.md`](../../agent/AGENTS.md) — the only consumer of this toolset.
- [`../../../../tests/tools/filesystem/AGENTS.md`](../../../tests/tools/filesystem/AGENTS.md) — offline tests for these tools.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
