---
title: CI runners
description: Self-hosted GitHub Actions runners for the bramburn/BrowserOS fork.
---

# CI runners

The fork uses two kinds of runners:

## Hosted runners (ubuntu-latest)

Fast surface: tests, lint, CLI/server/extension releases. The
existing fork workflows (`release-cli.yml`, `release-server.yml`,
`release-agent-extension.yml`, `test.yml`, `code-quality.yml`, etc.)
all use `runs-on: ubuntu-latest`. Hosted runners are fine for
anything that finishes in <6 hours.

## Self-hosted runners

For the Chromium-from-source Windows build, we need a runner that:

- Runs on Windows (the Chromium build target).
- Has the host toolchain: VS2022, Win10 SDK, depot_tools, Rust,
  Bun, Python 3.12.
- Has 150 GB+ free disk for the build artifacts.
- Runs for up to 12 hours per job (hosted runners cap at 6).

The fork currently has:

| Label | OS | Host | Registered |
|---|---|---|---|
| `[self-hosted, macOS, ARM64, browseros-builder]` | macOS ARM64 | (upstream's mac mini) | upstream |
| `[self-hosted, Windows, browseros-builder-windows]` | Windows x64 | **This dev box** | added 2026-09-19 |

## Installing the Windows runner

The runner is a Windows service that auto-starts on boot. Steps to
reinstall:

### 1. Download the latest runner

```powershell
mkdir C:\actions-runner -Force
Invoke-WebRequest -Uri https://github.com/actions/runner/releases/latest/download/actions-runner-win-x64-<ver>.zip -OutFile C:\actions-runner\runner.zip
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("C:\actions-runner\runner.zip", "C:\actions-runner")
```

### 2. Get a registration token

```powershell
$token = gh api -X POST /repos/bramburn/BrowserOS/actions/runners/registration-token --jq .token
# Token expires in ~1 hour.
```

### 3. Configure

```powershell
cd C:\actions-runner
.\config.cmd --url https://github.com/bramburn/BrowserOS `
  --token $token `
  --labels browseros-builder-windows `
  --runnergroup default `
  --work _work `
  --replace
```

### 4. Install as Windows service

```powershell
.\svc.cmd install
.\svc.cmd start
```

The service runs as `NT AUTHORITY\SYSTEM` by default. For builds that
need user-level resources (e.g. the user's `~/.cargo`), switch to a
local user account via `.\svc.cmd install --account <DOMAIN>\<USER>
--password <PW>` and grant the necessary permissions.

### 5. Verify

```powershell
gh api /repos/bramburn/BrowserOS/actions/runners --jq '.runners[] | select(.labels[].name == "browseros-builder-windows") | {name, status, busy}'
```

Expected: `status: online`, `busy: false`.

## Disabling the runner

When the box is offline for maintenance or to save electricity:

```powershell
cd C:\actions-runner
.\svc.cmd stop    # pauses picking up jobs
.\svc.cmd uninstall   # removes the Windows service entirely
```

The Chromium source tree and R2 artifacts are not affected. Jobs that
fire while the runner is offline will queue and run when it's back.

## Disabling jobs without removing the runner

If you want the runner to stay online but pause specific workflows:

- Add `if: false` to a workflow's trigger, OR
- Use GitHub's "Disable workflow" button in the Actions UI.

The `release-windows.yml` workflow already has `cancel-in-progress:
false` so a queued build doesn't accidentally cancel a sibling.

## Disk management

A single Chromium build consumes ~150 GB. Two consecutive builds
will exhaust 300 GB. The self-hosted runner should have a cron job
that cleans `C:\browersos-build\src\out` between builds:

```powershell
# Optional cleanup cron (add to Task Scheduler)
Get-ChildItem C:\browersos-build\src\out -Recurse -Force -ErrorAction SilentlyContinue |
  Where-Object { $_.LastWriteTime -lt (Get-Date).AddDays(-7) } |
  Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
```

Or use `bramburn-build.ps1`'s own `clean` module which knows the right
paths to scrub.