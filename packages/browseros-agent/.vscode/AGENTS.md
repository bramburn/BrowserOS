# `.vscode/` — VS Code launch configuration

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

A single VS Code debug configuration, [`launch.json`](launch.json), with one
entry: **"Debug BrowserOS Server"**. It launches `apps/server/src/index.ts`
under the Bun debugger with the package root as the workspace folder, so F5
drops a debugger on the MCP server with no extra setup.

Nothing else — no `settings.json`, `extensions.json`, `tasks.json`, or
`snippets/`. Editor *settings* for this repo live at the repository root
(`D:\BrowserOs\.vscode\settings.json`); this directory is launch/debug only.
No build, lint, or test tooling reads this file.

## Contents

```
.vscode/
└── launch.json    ← 488 B, schema version 0.2.0, one configuration
```

The one configuration, field by field:

| Field | Value | Why it matters |
|---|---|---|
| `type` | `bun` | Requires a Bun-aware VS Code debugger extension |
| `name` | `Debug BrowserOS Server` | The name shown in the debug dropdown |
| `request` / `program` | `launch` / `src/index.ts` | Entry point, resolved against `cwd` |
| `cwd` | `${workspaceFolder}/apps/server` | Runs the server with the right relative paths |
| `env.BUN_ENV_FILE` | `.env.development` | Bun loads the dev env file automatically |
| `internalConsoleOptions` | `openOnSessionStart` | Integrated console opens immediately |
| `stopOnEntry` / `watchMode` | `false` | No auto-break, no rebuild-on-save |

## Rules

**VS1 — Open `packages/browseros-agent/` as the workspace root.**
`${workspaceFolder}` must resolve to the package directory; opening the repo
root instead makes `cwd` point at a non-existent `apps/server` and the
launcher fails.

**VS2 — `BUN_ENV_FILE` is the dev-env mechanism.** It matches the repo's
"no dotenv" rule (Bun loads `.env` automatically) and the
`bun --env-file=.env.development …` convention in
[`../CLAUDE.md`](../CLAUDE.md). Don't add a `dotenv` preload to work around a
missing variable.

**VS3 — Debug the server through `src/index.ts`.** That is the entry that
early-exits without Bun and reaches `Application.start()` in `src/main.ts`
(Sentry init, config load, shutdown hooks). Launching `src/main.ts` or a tool
file directly skips that lifecycle.

**VS4 — Keep `stopOnEntry` and `watchMode` false.** The server is
long-running and CDP-connected; auto-relaunch on save fights the browser
connection.

**VS5 — Add a new configuration, don't mutate this one.** It is the canonical
"debug the MCP server" recipe; an agent or extension config belongs in its own
entry with a distinct `name`.

**VS6 — Don't put secrets here.** `env` is committed; keep keys in
`apps/server/.env.development` (copied into worktrees by the Worktrunk hook in
[`../.config/wt.toml`](../.config/wt.toml)).

## Workflows

**Attaching a debugger to the MCP server**
1. Open `packages/browseros-agent/` as the folder in VS Code.
2. F5 (or pick "Debug BrowserOS Server" in the Run and Debug panel).
3. Set breakpoints in `apps/server/src/api/routes/*.ts` or
   `apps/server/src/tools/*.ts`.
4. The console opens on start; the server loads `.env.development` via
   `BUN_ENV_FILE`.

**When the launcher cannot find the entry point**
1. Confirm the opened folder is `packages/browseros-agent`, not `D:\BrowserOs`.
2. Confirm a Bun-capable debugger is installed — `"type": "bun"` is not
   understood by the default Node debugger.
3. Confirm `apps/server/src/index.ts` exists and Bun itself is on PATH
   (`package.json` declares `"engines": { "node": "please-use-bun" }`).

**Debugging the extension instead**
1. This config does not cover `apps/agent/` — load the WXT-built extension
   in a BrowserOS/Chrome instance and use the CDP inspector described in
   [`../.claude/skills/test-ui/SKILL.md`](../.claude/skills/test-ui/SKILL.md).

## Cross-references

- [`launch.json`](launch.json) — the only file here.
- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`../CLAUDE.md`](../CLAUDE.md) — Bun conventions and the `dev:watch` flow.
- [`../apps/server/AGENTS.md`](../apps/server/AGENTS.md) — the code this config launches.
- [`../.config/AGENTS.md`](../.config/AGENTS.md) — Worktrunk, which creates
  worktrees where this launch config also works.
- [`../.claude/skills/test-ui/AGENTS.md`](../.claude/skills/test-ui/SKILL.md) — the extension-UI testing path.
