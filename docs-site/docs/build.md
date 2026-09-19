---
title: Build
description: How to build the bramburn/BrowserOS fork from source.
---

# Building from source

Two build paths exist. Pick the one that matches your context.

## Bun MCP server (fast, 2-5 min)

The Bun MCP server is the agent-platform MCP server (port 9100). It
is **not** the same as the bundled `browseros_server.exe` that ships
with the browser (port 9200).

```bash
cd packages/browseros-agent/apps/server
bun install
bun run build
```

## Chromium browser + bundled MCP (slow, 7-13 h)

The full Chromium + bundled MCP build. Required for:
- Producing a Windows `.exe` installer we can sign + publish.
- Verifying the fork's Chromium-side patches.
- End-to-end testing the auto-update flow.

### Prerequisites (verified on this host 2026-09-19)

| Tool | Version |
|---|---|
| Visual Studio | 2022 Community 14.44.35207 |
| Windows SDK | 10.0.26100.0 |
| depot_tools | latest (includes ninja) |
| Rust + cargo | latest |
| Bun | 1.4+ |
| Python | 3.12 (via `uv`) |

### Run interactively (simplest — works today)

```powershell
cd C:\dev\BrowserOs
$env:PYTHONIOENCODING = "utf-8"
& C:\dev\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 3
```

This runs phases 1-3 (setup → prep → build). Total wall time: 7-13 h
on this Windows box. Disk: 150 GB at `C:\browersos-build\src`.

### Run detached (for overnight)

Use the [CI runners](ci-runners) self-hosted runner. The build
workflow `.github/workflows/release-windows.yml` runs the same
orchestrator in detached mode via GitHub Actions.

## Five build phases

| Phase | Wall time | What it produces |
|---|---|---|
| 1 setup | 30-60 min | gclient sync + clean checkout (~50 GB at `C:\browersos-build\src`) |
| 2 prep | 5-15 min | configure + patches + replace + resources |
| 3 build | 6-12 h | autoninja `-t release -a x64` (+100 GB at `out\Default`) |
| 4 sign | 2-5 min | `sign_windows` via SSL.com eSigner |
| 5 package | 1-3 min | `package_windows` → `BrowserOS_v<ver>_win-x64.exe` |

The orchestrator at `tools/bramburn-build.ps1` handles all five.

## Where artifacts go

```
C:\browersos-build\src\out\Default\chrome.exe                     (~500 MB)
C:\browersos-build\src\out\Default\mini_installer.exe              (~50 MB)
C:\dev\BrowserOs\packages\browseros\releases\<version>\
  └── BrowserOS_v<version>_win-x64.exe                            (~150 MB)
```

The packaged `.exe` is what gets uploaded to R2 + GitHub Releases by
the [release pipeline](release).