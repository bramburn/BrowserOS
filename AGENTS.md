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

## Strategic roadmap — read this before any non-trivial work

The fork's near-term focus is **bottom-up** and **gated**. Before touching
anything else, internalise this ordering:

| # | Priority | Why it's first |
|---|---|---|
| **1** | **Auto-update like Edge / Chrome** | Gating dependency — we cannot ship fast until updates Just Work. |
| **2** | **Browser robustness / stability** | A crashing browser is worse than no browser. |
| **3** | **Installation UX / UI** | First-run matters; current installer is a thin mini_installer wrapper. |
| **4** | **Custom Docusaurus site** | `docs-site/` for fork-specific docs (build, update server, release process). |
| 5 | MCP + agent hardening | **Deferred** until #1–#4 are shippable. |

**Rule for any coding agent working in this repo:**

- If a change relates to MCP / agent work and #1–#4 are not yet shippable,
  stop and surface the conflict to the user. Do not silently land agent
  patches against a moving browser target.
- "Shippable" for #1 means: end-to-end test of "ship v1 → ship v2 to
  update server → restart-into-v2 with tabs intact" passes.
- "Shippable" for #2 means: 24 h soak test on a clean profile passes
  without a crash; session restore verified across hard kill.
- "Shippable" for #3 means: a fresh user can install, import data from
  Chrome, and reach the default new-tab page in <60 s with no surprises.

See [`README.md`](README.md) § "Strategic roadmap" for the full version
with concrete deliverables per priority.

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

## Today's breakage (2026-09-19) — context for eventual MCP work

**Note**: per the strategic roadmap above, MCP / agent hardening is
**deferred** until priorities #1–#4 are shippable. This section is kept as
a reference for when we get to priority #5. Do not start fixing any of
this without first confirming the user has unblocked MCP work.

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

Given the session time budget and the strategic roadmap above:

1. ✅ **Fork + clone** (done — public at `bramburn/BrowserOS`, local at `C:\dev\BrowserOs`)
2. ✅ **Audit host toolchain** (done — see table above)
3. ✅ **Architectural review** (done — see diagrams)
4. ✅ **Document today's breakage** (this file; MCP work deferred per roadmap)
5. ✅ **Update README + AGENTS + WHY_FORK** to reflect the strategic priorities
6. ⏳ **Start Chromium build** — long-running, monitor via cron (prereereq for #1 stability + #2 auto-update)
7. ❌ **Build the bundled `browseros_server.exe` from source** — source not in this fork; revisit only when MCP work is unblocked
8. ❌ **Replace installed BrowserOS** — requires successful Chromium build

## Chromium sync — lessons learned (2026-09-19/20)

We attempted the Chromium source fetch twice. Both wrappers died silently. The
lessons are captured here so the next attempt doesn't repeat them. Full
incident log: `C:\temp\browseros-build\*.log` (still on disk).

### 1. The failure is HTTP 429, NOT cache corruption

`depot_tools`' `git_cache.py:762` raises `git_cache.ClobberNeeded()` on **any**
failed `+refs/heads/*:refs/heads/*` fetch — a blanket catch that mislabels
429s, network errors, and genuine corruption all as "corrupted cache". The
diagnostic that separates them:

```powershell
Select-String -Path 'C:\temp\browseros-build\gclient*.log' -Pattern '429|RESOURCE_EXHAUSTED|Short term server-time rate limit'
```

If you see `subject: "shared/shared_anonymous"` and `RESOURCE_EXHAUSTED`,
the cache is fine — you're hitting `chromium.googlesource.com`'s anonymous
short-term rate limit. Fix: `gclient sync -j1` (serial fetches stay under the
quota), not a cache wipe. Re-run with a 15-min cool-down between passes:
`C:\temp\browseros-build\sync-throttled.cmd` is already written and uses
8 passes × 900 s cool-down.

### 2. Detached `cmd.exe` wrappers die silently on this host (two failed attempts)

Both detach patterns below were tried; both died with **zero** Windows Event
Log entries (no Application Error, no WER, no Security logoff). The wrapper
just vanished mid-sync.

| Run | Pattern | Death time | What happened |
|---|---|---|---|
| v1 (PID 43300) | `Start-Process -WindowStyle Hidden` from bash | 76 min | Cache 102→148 mirrors (real work); died silently |
| v2 (PID 3648) | `wscript.exe → cmd /c start /B /MIN cmd /c <launcher>` | <10 min | Died almost immediately |

Hypotheses tested:
- Job-object cleanup from the bash task (v2 disproves this — `start /B` breaks the chain)
- Group Policy 1054 (recurring on this box, but GPO failure doesn't kill processes)
- SCM service churn (benign — services cycling during normal operation)
- OOM killer / Defender (Defender already disabled; plenty of disk + RAM)

**Unknown root cause.** Only known-stable hosts for long-running `gclient sync` here are:

1. **Self-hosted GH Actions runner** at `actions.runner.bramburn-BrowserOS.SERVER02`
   (installed 2026-09-19). It's a real Windows Service — the parent is the
   kernel, not a parent shell. Use `.github/workflows/release-windows.yml` with
   `runs-on: [self-hosted, Windows, browseros-builder-windows]`.
2. **Windows Scheduled Task** registered with the `SYSTEM` account, triggered by
   event/time. Immune to interactive-process churn.

**Don't** try yet another `Start-Process` / `wscript` / `start /B` variant.
Pick one of the two stable hosts above.

### 3. `mavis-trash` CLIXML stderr-capture quirk on large dirs

When trashing directories ≥10 GB, `mavis-trash.cmd` may exit 1 with a
`#< CLIXML` error in the captured output, even though the move completed.
Workaround that works:

```powershell
& 'C:\Users\bramburn\.minimax\bin\mavis-trash.cmd' $dir *> $null
# then verify with:
Test-Path $dir
```

`*> $null` redirects BOTH streams and avoids the PowerShell pipeline bug.
The 19 GB `_gclient_cache_full_20260919` succeeded this way; the 12 GB
`_src_partial_20260919` did not (stayed on disk).

### 4. Current disk state (after 2026-09-20 14:00 BST cleanup)

| Path | State | Size |
|---|---|---|
| `C:\browersos-build\.gclient_cache` | TRASHED ✓ | ~20 GB reclaimed |
| `C:\browersos-build\src` | STILL ON DISK | ~10 GB |
| `C:\browersos-build\_src_partial_20260919` | STILL ON DISK | ~12.7 GB |
| `C:\browersos-build\_src_restart_partial_20260919` | TRASHED ✓ | ~1.8 GB reclaimed |
| `C:\browersos-build\.gclient` | KEPT (config) | small |
| `C:\browersos-build\fetch-chromium.cmd` | KEPT (launcher) | small |
| `C:\temp\browseros-build\*` (logs + sync-throttled scripts) | KEPT (diagnostic) | small |

Disk free: ~119 GB. Total reclaim if you sweep `src/` and `_src_partial_20260919`:
~22 GB more.

To complete the cleanup when you return:
```powershell
& 'C:\Users\bramburn\.minimax\bin\mavis-trash.cmd' 'C:\browersos-build\src' *> $null
& 'C:\Users\bramburn\.minimax\bin\mavis-trash.cmd' 'C:\browersos-build\_src_partial_20260919' *> $null
```

### 5. Cron watchdog state

`browseros-build-watchdog` (cron id `4d85d9b8-4666-47e4-9ab0-c811dc6631f2`)
is **disabled** as of 2026-09-20. Re-enable with `enabled: true` only when a
sync is actually running.

## Recommended path (matches the strategic roadmap)

### Priority #1 — auto-update like Edge / Chrome (gating)

Before writing any auto-update code:

1. Confirm `bramburn-build.ps1` has produced a working Chromium at
   `C:\browersos-build\src\out\Default\chrome.exe`.
2. Audit the update URL config in the patches:
   `packages/browseros/chromium_patches/chrome/browser/browseros/server/` and
   any `components/update_client/` overrides. Identify every place the
   upstream update server URL is hard-coded.
3. Stand up a tiny nginx + signed-payload server in `tools/update-server/`
   (out of scope until #6 above completes).
4. Replace the URL + signing key with our own.
5. End-to-end test: ship v1 → ship v2 → restart-into-v2 with tabs intact.

### Priority #2 — browser robustness

After #1 is shippable:

- Wire the existing Chromium crash reporter (`components/crash/`) to our
  own backend (e.g. `sentry.io` or a self-hosted `crashpad` collector).
- Add a 24 h soak test profile and run it on every CI nightly.
- Verify session restore across hard kill (`taskkill /F` on `chrome.exe`).

### Priority #3 — installation UX / UI

After #2 is shippable:

- Audit `packages/browseros/chromium_patches/chrome/installer/`.
- Walk through the current install on a clean VM, time it, screenshot it,
  file the gaps.
- Patch the mini_installer flow with first-run wizard + data import.

### Priority #4 — Docusaurus site

Can start in parallel with #1 once we have the fork public:

- Scaffold `docs-site/` with Docusaurus 3 classic.
- Mirror `AGENTS.md`, `WHY_FORK.md`, and `docs/MCP_TOOL_SPEC.md` into the
  Docusaurus content tree.
- Wire CI to deploy on every merge to `main`.

### Priority #5 — MCP / agent hardening (only after #1–#4 ship)

**Deferred.** When the user unblocks MCP work:

- Either get upstream to publish the bundled server source (open issue +
  PR against `browseros-ai/BrowserOS`), OR
- Reverse-engineer the controller CRX
  (`nlnihljpboknmfagkikhkdblbedophja.crx`) to extract the protocol, OR
- Build a substitute `apps/server`-based MCP server that speaks the old
  protocol and translates to the new one.

## Files to know

- `WHY_FORK.md` — why we forked (motivation + scope)
- `docs/CI_AND_RELEASES.md` — full runbook for the fork's CI, release,
  and update pipeline (workflows, self-hosted runner, R2 layout, tag
  conventions, manual fallback). Read this before changing any
  workflow file.
- `tools/release/generate_update_manifests.py` — generates the
  Omaha-4 + appcast + JSON pointer from a release artifact.
- `packages/browseros/CHROMIUM_VERSION` — pinned Chromium version (146.0.7778.97)
- `packages/browseros/BASE_COMMIT` — exact Chromium commit
- `packages/browseros-agent/apps/server/src/tools/snapshot.ts` — modern snapshot impl
- `packages/browseros-agent/apps/server/src/browser/backends/cdp.ts` — CDP client
- `packages/browseros/chromium_patches/chrome/browser/browseros/server/` — C++ wrapper for the bundled MCP server
- `packages/browseros/chromium_patches/chrome/browser/browseros/bundled_extensions/` — CRX list (controller, agent, bug-reporter)

## CI, Releases, and Updates

The fork ships three workflow layers:

1. **Hosted runners** (`ubuntu-latest`) — fast surface: tests, lint,
   CLI / server / extension releases, and the **update-manifest**
   publisher. Inherited from upstream (CLI, server, extension,
   nightly macOS, test matrix) plus our new
   `.github/workflows/update-manifest.yml` and
   `.github/workflows/deploy-docs.yml`.
2. **Self-hosted macOS runner**
   (`[self-hosted, macOS, ARM64, browseros-builder]`) — nightly
   Chromium build for macOS arm64. Already wired by upstream.
3. **Self-hosted Windows runner**
   (`[self-hosted, Windows, browseros-builder-windows]`) — added
   2026-09-19 on this dev box. Runs the fork's Windows Chromium
   build via `.github/workflows/release-windows.yml`. Required
   because the Chromium build is 7-13 hours (hosted runners cap at
   6).

The release flow is: bump version → Chromium build (7-13 h) → sign
(SSL.com eSigner) → package `mini_installer.exe` → optional AIP
wrap → upload to R2 → create GitHub Release + tag
`browseros-windows-v<version>` → trigger update-manifest → publish
two XML manifests + a JSON pointer to R2.

### Update surfaces

- **In-browser**: Chromium's `update_client` polls an Omaha-4 XML
  at `https://cdn.bramburn.com/browseros/windows/update_check.xml`.
  Drives the "restart to update" toast.
- **External**: a Sparkle-style appcast RSS at
  `https://cdn.bramburn.com/browseros/windows/appcast.xml`. Used by
  `browseros-cli` (when `--check-update` lands) and external tools.
- **JSON pointer**: `https://cdn.bramburn.com/browseros/latest.json`
  for lightweight CLI checks.

Both manifests are generated by
[`tools/release/generate_update_manifests.py`](tools/release/generate_update_manifests.py)
and uploaded to R2 by
[`.github/workflows/update-manifest.yml`](.github/workflows/update-manifest.yml).
See [`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md) for the
full schema, R2 layout, and end-to-end test procedure.

### Advance Installer Pro (optional v2 polish)

AIP is installed at
`C:\Program Files (x86)\Caphyon\Advanced Installer 23.9\` on this
dev box. The release workflow has a `wrap_with_aip` input
(default `false`); when `true`, the workflow wraps the
`mini_installer.exe` in an AIP-built polished installer. The
project template lives at `installer/browseros.aip.template` (not
yet authored — see `installer/README.md`).

### Docusaurus site (priority #4)

The fork's docs site lives at `docs-site/` (Docusaurus 3 classic).
Content is mirrored from `AGENTS.md`, `WHY_FORK.md`, and
`docs/CI_AND_RELEASES.md`. Deployed to GitHub Pages by
`.github/workflows/deploy-docs.yml` on every push to `main`.
