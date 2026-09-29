# `scripts/dev/` — local dev launcher and CDP UI inspector

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Four developer utilities that never ship. `inspect-ui.ts` is the CDP UI
inspector documented in `CLAUDE.md` § "Self-Testing UI Changes" — it talks
straight to Chromium over the DevTools WebSocket so you can screenshot,
snapshot, click, and eval inside extension pages the MCP tools can't reach.
`start.ts` is an earlier TypeScript dev launcher (the maintained path is now
the Go CLI at `../../tools/dev`). `chat-cli.ts` posts a chat request to a
running agent server. `mcp-test.sh` curls a running MCP endpoint.

## Contents

```
scripts/dev/
├── inspect-ui.ts   ← CDP WebSocket client + CLI: targets, open-sidepanel, screenshot,
│                      snapshot, click, fill, eval
├── start.ts        ← Bun dev launcher (--watch / --manual, --new ports)
├── chat-cli.ts     ← POST a message to the HTTP agent server (--provider, --model, --port, …)
└── mcp-test.sh     ← curl + jq smoke test against http://127.0.0.1:<port>/mcp
```

## Rules

**DV1 — `inspect-ui.ts` defaults to the dev CDP port 9010.** The file states
`// Matches DEV_PORTS.cdp from @browseros/shared/constants/ports` — keep that
comment true, or import the constant instead of hardcoding.

**DV2 — The extension id is overridable via `BROWSEROS_EXTENSION_ID`.** The
default is the unpacked dev extension id; override it for a locally reloaded
build.

**DV3 — Target selectors accept an index or a URL substring.** `sidepanel`,
`newtab`, `chrome-extension://`, or `3` from the `targets` output. Rejecting
substring matching breaks the documented workflow.

**DV4 — Element ids come from the snapshot, not coordinates.** `[number]` in
`snapshot` output is a `backendDOMNodeId`; `click` / `fill` take that id. Don't
add a coordinate mode — it re-introduces the flakiness the script avoids.

**DV5 — `start.ts` is macOS-shaped.** It hardcodes
`/Applications/BrowserOS.app/Contents/MacOS/BrowserOS` and
`/tmp/browseros-dev`. Prefer `bun run dev:watch*` (which dispatches to the Go
`tools/dev` CLI) over extending this file.

**DV6 — `mcp-test.sh` is a smoke test, not a test suite.** It hardcodes a
default port of 9223 and much of it is commented out. `bun test` is the real
runner.

## Workflows

**Inspecting the side panel:** 1. `bun run dev:watch:new` and note the printed
CDP port. 2. `export BROWSEROS_CDP_PORT=<port>`. 3. `bun
scripts/dev/inspect-ui.ts targets`. 4. `… open-sidepanel`. 5. `… snapshot
sidepanel` → note element ids. 6. `… click sidepanel 142` / `… fill sidepanel
85 "text"`. 7. `… screenshot sidepanel /tmp/panel.png` and read the PNG.

**Adding an inspector command:** 1. Add a `case` to the command switch in
`inspect-ui.ts`. 2. Implement a `cmdX(cdp, ...)` function alongside the
existing ones. 3. Add the command to the usage text and to the
`Self-Testing UI Changes` section of `../../CLAUDE.md`.

**Debugging a chat request:** `bun --env-file=.env.dev
scripts/dev/chat-cli.ts --provider=openai --model=gpt-4o "message"`. It
truncates tool output to 50 chars unless `--show-full-output` is passed.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/` scope (rules SR1–SR6).
- [`../../CLAUDE.md`](../../CLAUDE.md) — § "Self-Testing UI Changes", the documented inspector workflow.
- [`../../package.json`](../../package.json) — `dev:watch*` scripts (Go CLI path).
- [`../../tools/dev/AGENTS.md`](../../tools/dev/AGENTS.md) — the maintained dev orchestrator.
- [`../../apps/server/AGENTS.md`](../../apps/server/AGENTS.md) — the server these scripts talk to.
