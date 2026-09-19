# BrowserOS fork (bramburn) — Agent Guide

This is `bramburn/BrowserOS`, a public fork of `browseros-ai/BrowserOS` (AGPL-3.0).
See `WHY_FORK.md` (added 2026-09-19) for the motivation.

## TL;DR

- **Fork**: `github.com/bramburn/BrowserOS` (public, mirror of upstream)
- **Upstream**: `github.com/browseros-ai/BrowserOS`
- **Local checkout**: `C:\dev\BrowserOs` (shallow clone, depth=1, ~344 MB)
- **Bundled MCP server** (the one we hit today on `127.0.0.1:9200`): precompiled into `browseros_server.exe` — see "Bundled MCP server source" below
- **Build pipeline**: Python CLI (`packages/browseros/`) that fetches Chromium 146 + applies ~342 patches + builds via ninja (6-12 h on this Windows box)
- **Bun MCP server** (`packages/browseros-agent/apps/server`): source is here, builds in 2-5 min, used by BrowserOS neo (not bundled with the browser)

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│  BrowserOS Chromium (built by packages/browseros/build/)             │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  chrome/browser/browseros/server/ (C++ patches)                 │ │
│  │   - browseros_server_manager.cc : launches browseros_server.exe │ │
│  │   - browseros_server_proxy.cc   : IPC to the subprocess        │ │
│  │   - process_controller_impl.cc  : health checks, restart       │ │
│  └────────────────────────────────────────────────────────────────┘ │
│         │ spawns                                                    │
│         ▼                                                           │
│  browseros_server.exe  ◄─── THIS IS THE THING WE TALK TO             │
│  (92 MB, version 0.0.165)                                            │
│  listens on http://127.0.0.1:9200/mcp                                │
│                                                                     │
│  Source for this binary: NOT in this fork. See "Bundled MCP server  │
│  source" below.                                                      │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│  packages/browseros-agent/  (separate codebase)                      │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  apps/server/  (Bun + TypeScript)                               │ │
│  │   - src/main.ts        : server lifecycle                      │ │
│  │   - src/tools/*.ts     : 17+ browser automation tools          │ │
│  │     snapshot, dom, navigation, page-actions, history, etc.     │ │
│  │   - src/browser/*.ts   : CDP client (raw chrome devtools)      │ │
│  │   - src/api/server.ts  : HTTP API + MCP endpoint (port 9100)   │ │
│  └────────────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  tools/dev/ + tools/dogfood/  (Go CLI = browseros-cli)          │ │
│  └────────────────────────────────────────────────────────────────┘ │
│  Used by "BrowserOS neo" / agent platform, not the bundled browser.  │
└─────────────────────────────────────────────────────────────────────┘
```

## Bundled MCP server source — NOT in this fork

The `browseros_server.exe` that ships with the installed BrowserOS
(`C:\Users\bramburn\AppData\Local\browseros\Application\151.0.8162.137\BrowserOSServer\default\resources\bin\browseros_server.exe`)
is **not** built from any source visible in this repo.

What's here is:
- `chrome/browser/browseros/server/*.cc` — C++ Chromium-side process manager
- `chrome/browser/browsing_data/...` — BrowserOS-specific data patches
- 342 chromium_patches/ files that get layered on top of Chromium 146

But the actual MCP server (the JSON-RPC handlers for `tabs`, `act`, `snapshot`,
`navigate`, `read`, etc.) lives in a separate codebase. Two possibilities:

1. **BrowserOS-private repo** not synced to this public fork.
2. **Bundled in the chromium_files/** as a CRX/extension that gets
   installed at first run (the controller CRX `nlnihljpboknmfagkikhkdblbedophja.crx`
   is the most likely candidate — its manifest probably points to the MCP server).

**Practical implication**: to actually patch the MCP server we hit today
on port 9200, we need either the private source OR to rebuild the controller
CRX from source. Neither is in this shallow clone.

## Today's breakage (2026-09-19) — what we want to fix

BrowserOS 0.50.5 / MCP server 0.0.165 (released ~16-Sep-2026) shipped three
breaking changes that broke every wrapper in `C:\dev\browser-cli`:

| Change | Old | New |
|---|---|---|
| Tool rename | `evaluate` | `run` (Bun sandbox, **no page-DOM access**) |
| Tool rename | `window` | `windows` (now `list`/`create`/`close`/`activate`) |
| `tabs` action | `select` | removed — only `list`/`active`/`new`/`close` |
| New required arg | — | `session` on every tool call (handle from `_meta.com.browseros/session`) |
| `act` kinds added | — | `type_at`, `hover_at`, `drag_at`, `focus`, `check`, `uncheck`, `select` |
| `act.fill.fields[]` | — | new multi-field array form |

We patched `browser-cli` on 2026-09-19 to compensate:
- `src/browser_cli/mcp_client.py` now captures `_meta.com.browseros/session` and auto-injects `session` on every `call_tool`.
- All `evaluate` calls rewritten to `run` (legacy JS kept as fallback for logging only).
- All `tabs` `select` / `window` `focus` calls replaced by snapshot + `act kind=focus` shim.
- `_wait_for_url_changed` rewritten to use `tabs active` URL parsing.

But the **cleanest fix** is on the server side: pin the new session-required
behavior in a backwards-compat shim so existing clients don't break.

## Building this fork

### What's buildable on this Windows box (verified 2026-09-19)

| Tool | Status |
|---|---|
| VS2022 Community 14.44.35207 | ✓ |
| Windows SDK 10.0.26100.0 | ✓ |
| depot_tools | ✓ (includes ninja) |
| Rust (cargo) | ✓ |
| Bun | ✓ |
| Python 3.12 (via `py`) | ✓ |

### What's NOT buildable on this box

| Component | Issue |
|---|---|
| Full Chromium browser build | 4-8+ hours, 100 GB disk, 150k ninja targets. Doable but **not in a single session**. |
| Bundled MCP server (`browseros_server.exe`) | Source not in this repo. See "Bundled MCP server source" above. |
| Bun MCP server (`apps/server`) | Source is here. Builds in 2-5 min. Already working on port 9100 (BrowserOS neo). |

### Build the Bun MCP server (fast)

```powershell
cd C:\dev\BrowserOs\packages\browseros-agent\apps\server
bun install
bun run build   # produces ./dist
```

This produces a Bun-compiled single executable for the agent platform
MCP server. It can be run standalone (port 9100) as a CDP-driven browser
automation server — **not** the same protocol as port 9200.

### Build the Chromium browser (slow)

```powershell
# Install the build CLI
cd C:\dev\BrowserOs\packages\browseros
py -m pip install -e .

# Fetch Chromium source via depot_tools (gclient)
browseros setup
# ~30-60 min, ~50 GB at C:\browersos-build\src\

# Apply the 342 patches
browseros apply
# ~2-5 min

# Build (this is the long one)
browseros build
# ~6-12 hours on this Windows box, ~150 GB out/

# Package + sign
browseros package
browseros sign
```

The output binary is `out/Default/chrome.exe`. To replace the installed
BrowserOS:
- Backup `C:\Users\bramburn\AppData\Local\browseros\Application\151.0.8162.137\chrome.exe`
- Copy the build's `chrome.exe` over it
- Restart BrowserOS (the version mismatch in `server.json` is a known issue;
  patch the manifest to claim your build's version)
- Watch for the bundled `browseros_server.exe` — that needs a separate build
  pipeline OR a substitute that speaks the same JSON-RPC over `/mcp`.

## What we can deliver in a single session

Given the session time budget:

1. ✅ **Fork + clone** (done — public at `bramburn/BrowserOS`, local at `C:\dev\BrowserOs`)
2. ✅ **Audit host toolchain** (done — see table above)
3. ✅ **Architectural review** (done — see diagrams)
4. ✅ **Document today's breakage** (this file)
5. ⏳ **Start Chromium build** — long-running, monitor via cron
6. ❌ **Build the bundled `browseros_server.exe` from source** — source not in this fork
7. ❌ **Replace installed BrowserOS** — requires successful Chromium build

## Recommended path

### If you want to replace the installed BrowserOS browser (6-12 hours)

```powershell
# 1. Install build CLI
cd C:\dev\BrowserOs\packages\browseros
py -m pip install -e .

# 2. Start the long build (detached so the bash tool's 5-min timeout doesn't kill it)
$wrapper = @'
import os, time, subprocess
LOG = r'C:\temp\browseros-build.log'
open(LOG, 'w', encoding='utf-8').write(f'== start {time.strftime("%H:%M:%S")} ==\n')
proc = subprocess.Popen(
    ['browseros', 'build'],
    cwd=r'C:\dev\BrowserOs\packages\browseros',
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1,
)
for line in proc.stdout:
    with open(LOG, 'a', encoding='utf-8') as f: f.write(line)
proc.wait()
with open(LOG, 'a', encoding='utf-8') as f: f.write(f'EXIT {proc.returncode}\n')
'@
Set-Content -Path 'C:\temp\browseros-build-wrapper.py' -Value $wrapper -Encoding UTF8
Start-Process -FilePath 'py' -ArgumentList 'C:\temp\browseros-build-wrapper.py' `
  -WindowStyle Hidden `
  -RedirectStandardOutput 'C:\temp\browseros-build-stdout.log' `
  -RedirectStandardError 'C:\temp\browseros-build-stderr.log'
```

Then monitor with `Get-Process py` and the log file. On this machine
expect ~6-12 hours.

### If you just want to patch the MCP server

The bundled `browseros_server.exe` is not in this fork, so we can't patch it
directly. The realistic options are:

a. **Reverse-engineer the CRX**: extract `nlnihljpboknmfagkikhkdblbedophja.crx`
   from the installed app, find the JSON-RPC handlers, identify the protocol
   versions, and propose patches to the BrowserOS team via PR.

b. **Substitute**: build `apps/server` (Bun MCP server), point it at the
   installed Chromium's CDP port (9101), expose a backwards-compat layer
   that translates the old `evaluate`/`window`/`tabs.select` calls into
   the new `run`/`windows`/`act.focus` calls. Run it on port 9200 (kill
   the bundled one first, or use a different port and tell clients).

c. **Wait for upstream**: file an issue at `browseros-ai/BrowserOS`
   asking for backwards-compat shims in the bundled MCP server.

## Files to know

- `WHY_FORK.md` — why we forked (motivation + scope)
- `packages/browseros/CHROMIUM_VERSION` — pinned Chromium version (146.0.7778.97)
- `packages/browseros/BASE_COMMIT` — exact Chromium commit
- `packages/browseros-agent/apps/server/src/tools/snapshot.ts` — modern snapshot impl
- `packages/browseros-agent/apps/server/src/browser/backends/cdp.ts` — CDP client
- `packages/browseros/chromium_patches/chrome/browser/browseros/server/` — C++ wrapper for the bundled MCP server
- `packages/browseros/chromium_patches/chrome/browser/browseros/bundled_extensions/` — CRX list (controller, agent, bug-reporter)
