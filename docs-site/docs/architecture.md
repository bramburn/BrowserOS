---
title: Architecture
description: What's in the fork and how the pieces fit together.
---

# Architecture

The fork is a mirror of `browseros-ai/BrowserOS`. Three subsystems:

## 1. Chromium browser (C++)

Source code lives at `packages/browseros/`. The fork applies ~342
patches layered on top of Chromium 148 (pinned commit
`6b3fa66a923a9442c8ab0bc71b4b41ff24528d3b`). Build pipeline is the
Python CLI `browseros build` driven by
`packages/browseros/build/`.

Key fork-specific bits in `chromium_patches/chrome/browser/browseros/`:

- `server/` — C++ wrapper that launches and supervises the bundled
  MCP server subprocess.
- `extensions/` — installs the controller / agent / bug-reporter CRXs.
- `bundled_extensions/` — the CRX list.

## 2. Bundled MCP server (`browseros_server.exe`)

Precompiled binary at
`C:\Users\bramburn\AppData\Local\browseros\Application\<ver>\BrowserOSServer\default\resources\bin\browseros_server.exe`.

**Source is NOT in this public fork.** Two possibilities:

1. A BrowserOS-private repo not synced here.
2. Bundled in `chromium_files/` as a CRX that gets installed at first
   run (the controller CRX
   `nlnihljpboknmfagkikhkdblbedophja.crx` is the most likely candidate).

**Implication for the fork**: to actually patch the MCP server we hit
today on port 9200, we need either the private source OR to rebuild
the controller CRX from source. Neither is in this shallow clone.

For more, see [`bundled-mcp-server-source`](https://github.com/bramburn/BrowserOS/blob/main/AGENTS.md#bundled-mcp-server-source--not-in-this-fork)
in `AGENTS.md`.

## 3. Agent platform (TypeScript / Go)

Source code lives at `packages/browseros-agent/`. Built with Bun.

| App | Role |
|---|---|
| `apps/server` | Bun MCP server (port 9100). Source is here. |
| `apps/agent` | Browser extension UI (WXT + React). |
| `apps/cli` | Go CLI (browseros-cli, published on npm). |
| `apps/eval` | Benchmark framework. |
| `apps/controller-ext` | Chrome API bridge extension. |

## Data flow

```
User → Chrome browser (chromium fork)
  ↓ bundled MCP server (port 9200)
  ↓ JSON-RPC over HTTP
External CLI (browseros-cli, our C:\dev\browser-cli, etc.)

User → agent extension → Bun MCP server (port 9100)
  ↓ CDP over WebSocket
Chrome browser (any CDP-compatible Chrome)
```

The two servers speak **different protocols**. The bundled server
(9200) uses a BrowserOS-specific JSON-RPC schema (see
[MCP tool spec](mcp-tool-spec)). The Bun server (9100) uses the MCP
protocol with Bun MCP SDK.