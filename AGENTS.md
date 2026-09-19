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

BrowserOS 0.50.5 / MCP server 0.0.165 (released ~16-Sep-2026) shipped several
changes that broke wrappers in `C:\dev\browser-cli`:

| Change | Old | New | Notes |
|---|---|---|---|
| New tool | `evaluate` (page-context JS) | `evaluate` (still there) + `run` (Bun-sandbox JS) | See "evaluate vs run" below. NOT a rename. |
| Tool rename | `window` | `windows` | Now `list`/`create`/`close`/`activate`. |
| `tabs` action | `select` | removed | Only `list`/`active`/`new`/`close` remain. |
| New required arg | — | `session` on every tool call | Handle from `_meta.com.browseros/session`. |
| `act` kinds added | — | `type_at`, `hover_at`, `drag_at`, `focus`, `check`, `uncheck`, `select` | Now 15 kinds. |
| `act.fill.fields[]` | — | new multi-field array form | `{ref, value}[]`. |

### `evaluate` vs `run` — they're DIFFERENT tools, not renames

| | `evaluate` | `run` |
|---|---|---|
| Required args | `page`, `code` | `code` (no `page`) |
| Execution context | Page (has `document`, `window`) | Bun sandbox (no DOM) |
| `timeout` cap | 30000 ms hard | 30000 ms per inner evaluate/wait |
| Use for | Page interactions, DOM queries | Server-side JS, computation |

We patched `browser-cli` to use `run` everywhere (mistakenly assuming it
replaced `evaluate`). The capture above shows BOTH are present. The fix is
to route page-context JS to `evaluate(page=N, code=...)` and Bun-side JS to
`run(code=...)`.

See [`docs/MCP_TOOL_SPEC.md`](docs/MCP_TOOL_SPEC.md) for the canonical tool
spec (24 tools, full JSON Schemas).

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

#### Option A: Interactive (simplest — works today)

Run the orchestrator from an interactive PowerShell window, NOT from the
bash tool (which has a 5-min timeout that would kill mid-build):

```powershell
cd C:\dev\BrowserOs
$env:PYTHONIOENCODING = "utf-8"
& C:\dev\BrowserOs\tools\bramburn-build.ps1
# Default: run all 5 phases (setup -> prep -> build -> sign -> package)
# -StopAfterPhase 1: setup only (gclient fetch, 30-60 min)
# -StopAfterPhase 2: setup + prep (~1-2 h)
# -StopAfterPhase 3: setup + prep + build (the long one, 6-12 h)
```

The orchestrator logs to `C:\temp\browseros-build\browseros-build.log` and
writes a state JSON to `C:\temp\browseros-build\browseros-build-state.json`
so you can monitor progress from another shell with `Get-Content`.

#### Option B: Detached (for running overnight)

The detached path uses `C:\temp\browseros-build\detached-phase1.py` to
spawn the orchestrator. As of 2026-09-19 there's a known issue: `Start-Process
-Environment` doesn't reliably propagate `PYTHONIOENCODING=utf-8` to the
grandchild `browseros.exe` (it crashes on the rocket emoji in cp1252). The
workaround when the detached path fails:

```powershell
# Run interactively in a separate PowerShell window so it survives logout
Start-Process powershell.exe -ArgumentList '-NoProfile','-Command','while($true){& C:\dev\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 3; "build done, retrying..."; Start-Sleep 60}' -WindowStyle Hidden
```

#### Build phases (each ~time)

| Phase | Command | Wall time | Disk |
|---|---|---|---|
| 1 setup | `browseros build --setup` (gclient + clean) | 30-60 min | +50 GB at `C:\browersos-build\src` |
| 2 prep | `browseros build --prep` (configure + patches + replace + resources) | 5-15 min | +0 GB |
| 3 build | `browseros build --build` (autoninja, `-t release -a x64`) | 6-12 h | +100 GB at `C:\browersos-build\src\out` |
| 4 sign | `browseros build --sign` (sign_windows) | 2-5 min | +0 GB |
| 5 package | `browseros build --package` (package_windows) | 1-3 min | +0 GB |

Total: 7-13 hours wall time, 150 GB disk, requires the session to stay
alive or run via detached process.

#### Replacing the installed BrowserOS

After successful build:

```powershell
$INSTALLED = "C:\Users\bramburn\AppData\Local\browseros\Application\151.0.8162.137"
$BUILT = "C:\browersos-build\src\out\Default"

# Backup installed browser binary
Move-Item "$INSTALLED\chrome.exe" "$INSTALLED\chrome.exe.bak"

# Copy our build
Copy-Item "$BUILT\chrome.exe" "$INSTALLED\chrome.exe"

# Restart BrowserOS (the version mismatch in server.json is a known issue;
# patch it to claim your build's version)
```

The bundled `browseros_server.exe` (the MCP server) is NOT replaced by
this — that binary is precompiled and its source isn't in this fork.
Either replace it separately (requires source from BrowserOS) or live with
the published server.

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

See **"Build the Chromium browser (slow)"** above for two options:

- **Option A (interactive)** — run `bramburn-build.ps1` from a real PowerShell
  window. Works today, fully debugged. Best for "let it run overnight".
- **Option B (detached)** — uses `detached-phase1.py`. Has a known issue with
  `Start-Process -Environment` not reliably propagating `PYTHONIOENCODING=utf-8`
  to the browseros.exe grandchild. Needs another iteration before it's reliable.

For the immediate next session, recommend Option A from a separate
PowerShell window — start it before logging out, let it run overnight,
come back to a built Chromium at `C:\browersos-build\src\out\Default\chrome.exe`.

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
