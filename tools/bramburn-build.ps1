# BrowserOS build orchestrator for bramburn/BrowserOS fork.
#
# Runs the full Chromium-from-source pipeline end-to-end, logging to a file
# so the bash tool's 5-min timeout doesn't kill it. Designed to be launched
# detached via `Start-Process -WindowStyle Hidden`, not from the bash tool
# directly (see AGENTS.md section "Recommended path").
#
# Phases (each is a separate browseros CLI invocation so a phase failure
# doesn't roll back successful work):
#   1. setup      (gclient sync + clean)
#   2. prep       (configure + patches + chromium_replace + string_replaces + resources)
#   3. build      (autoninja - this is the long one)
#   4. sign       (sign_windows)
#   5. package    (package_windows)
#
# State is recorded in $StateFile so the monitor can see progress without
# parsing the log.
#
# -StopAfterPhase controls how far to go:
#   0 (default) = run all phases
#   N           = run through phase N, then exit (1 = setup only, 2 = setup+prep, ...)
#
# Usage (from PowerShell, NOT from the bash tool):
#   Start-Process -FilePath "C:\Python312\python.exe" `
#     -ArgumentList "C:\dev\BrowserOs\tools\bramburn-build.ps1" `
#     -WindowStyle Hidden `
#     -RedirectStandardOutput "C:\temp\browseros-build-stdout.log" `
#     -RedirectStandardError "C:\temp\browseros-build-stderr.log"

param(
    [string]$ForkRoot = "C:\dev\BrowserOs",
    [string]$ChromiumSrc = "C:\browersos-build\src",
    [string]$LogDir = "C:\temp\browseros-build",
    [string]$BrowserosExe = "C:\Python312\Scripts\browseros.exe",
    [int]$StopAfterPhase = 0
)

$ErrorActionPreference = "Continue"
# Set UTF-8 for the whole process so child processes (browseros CLI) can
# log rocket emojis and other Unicode. Must be at script scope (not inside a
# function) for Start-Process to inherit it.
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
$LogFile = Join-Path $LogDir "browseros-build.log"
$StateFile = Join-Path $LogDir "browseros-build-state.json"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $ts = (Get-Date).ToString("o")
    $line = "[" + $ts + "] [" + $Level + "] " + $Message
    Add-Content -Path $LogFile -Value $line -Encoding UTF8
    Write-Host $line
}

function Write-State {
    param(
        [int]$Phase,
        [string]$PhaseName,
        [string]$Status,
        [string]$LastError = ""
    )
    $state = @{
        forkRoot     = $ForkRoot
        chromiumSrc  = $ChromiumSrc
        phase        = $Phase
        phaseName    = $PhaseName
        status       = $Status
        lastError    = $LastError
        updatedAt    = (Get-Date).ToString("o")
        logFile      = $LogFile
    }
    $tmp = $StateFile + ".tmp"
    $state | ConvertTo-Json -Depth 4 | Set-Content -Path $tmp -Encoding UTF8
    Move-Item -Force $tmp $StateFile
}

# NOTE: parameter name must NOT be $Args (capital) because PowerShell uses
# $args (lowercase) as the automatic arguments variable inside functions,
# and capital $Args can shadow it inconsistently. Renamed to $BrowserArgs.
function Run-Browseros {
    param(
        [int]$Phase,
        [string]$PhaseName,
        [string[]]$BrowserArgs
    )
    Write-Log ("=== START phase " + $Phase + " (" + $PhaseName + ") ===")
    Write-Log ("browseros " + ($BrowserArgs -join ' '))
    Write-State -Phase $Phase -PhaseName $PhaseName -Status "running"

    # PS 5.1 doesn't support -Environment on Start-Process. Caller must
    # pre-set $env:PYTHONIOENCODING=utf-8 (the launch.cmd launcher does this)
    # so the .NET env block inherited by Start-Process already has it.

    $proc = Start-Process -FilePath $BrowserosExe `
        -ArgumentList $BrowserArgs `
        -WorkingDirectory $ForkRoot `
        -NoNewWindow `
        -RedirectStandardOutput ($LogFile + ".stdout") `
        -RedirectStandardError  ($LogFile + ".stderr") `
        -PassThru `
        -Wait

    Write-Log ("browseros exit code: " + $proc.ExitCode)

    if ($proc.ExitCode -ne 0) {
        Write-Log ("phase " + $Phase + " (" + $PhaseName + ") FAILED") "ERROR"
        Write-State -Phase $Phase -PhaseName $PhaseName -Status "failed" -LastError ("browseros exit " + $proc.ExitCode)
        return $false
    }
    Write-Log ("=== END phase " + $Phase + " (" + $PhaseName + ") ===")
    Write-State -Phase $Phase -PhaseName $PhaseName -Status "ok"
    return $true
}

function Should-RunPhase {
    param([int]$N)
    return ($StopAfterPhase -eq 0 -or $StopAfterPhase -ge $N)
}

function Should-StopAfter {
    param([int]$N)
    return ($StopAfterPhase -gt 0 -and $StopAfterPhase -eq $N)
}

if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
}

Write-Log "bramburn-build.ps1 starting"
Write-Log ("ForkRoot:     " + $ForkRoot)
Write-Log ("ChromiumSrc:  " + $ChromiumSrc)
Write-Log ("LogDir:       " + $LogDir)
Write-Log ("LogFile:      " + $LogFile)
Write-Log ("StateFile:    " + $StateFile)
Write-Log ("BrowserOS:    " + $BrowserosExe)
Write-Log ("StopAfterPhase: " + $StopAfterPhase + " (0 = run all)")

if (-not (Test-Path $BrowserosExe)) {
    Write-Log ("browseros.exe not found at " + $BrowserosExe) "ERROR"
    Write-State -Phase 0 -PhaseName "init" -Status "failed" -LastError "browseros.exe missing"
    exit 1
}

# Phase 1: setup (clean + git_setup + sparkle_setup)
if (Should-RunPhase 1) {
    $ok = Run-Browseros 1 "setup" @("build", "--setup", "--chromium-src", $ChromiumSrc)
    if (-not $ok) { exit 1 }
}
if (Should-StopAfter 1) { Write-Log "StopAfterPhase=1 reached, exiting"; exit 0 }

# Phase 2: prep (resources + chromium_replace + string_replaces + patches + configure)
if (Should-RunPhase 2) {
    $ok = Run-Browseros 2 "prep" @("build", "--prep", "--chromium-src", $ChromiumSrc)
    if (-not $ok) { exit 1 }
}
if (Should-StopAfter 2) { Write-Log "StopAfterPhase=2 reached, exiting"; exit 0 }

# Phase 3: build (autoninja compile, the long one)
if (Should-RunPhase 3) {
    Write-Log "=== START phase 3 (build -- autoninja, 6-12 hours) ==="
    Write-State -Phase 3 -PhaseName "build" -Status "running"

    # PS 5.1 doesn't support -Environment on Start-Process. Caller must
    # pre-set $env:PYTHONIOENCODING=utf-8 (the launch.cmd launcher does this)
    # so the .NET env block inherited by Start-Process already has it.

    $proc = Start-Process -FilePath $BrowserosExe `
        -ArgumentList @("build", "--build", "--chromium-src", $ChromiumSrc, "-t", "release", "-a", "x64") `
        -WorkingDirectory $ForkRoot `
        -NoNewWindow `
        -RedirectStandardOutput ($LogFile + ".stdout") `
        -RedirectStandardError  ($LogFile + ".stderr") `
        -PassThru `
        -Wait
    Write-Log ("build exit code: " + $proc.ExitCode)
    if ($proc.ExitCode -ne 0) {
        Write-Log "phase 3 (build) FAILED" "ERROR"
        Write-State -Phase 3 -PhaseName "build" -Status "failed" -LastError ("browseros exit " + $proc.ExitCode)
        exit 1
    }
    Write-Log "=== END phase 3 (build) ==="
    Write-State -Phase 3 -PhaseName "build" -Status "ok"
}
if (Should-StopAfter 3) { Write-Log "StopAfterPhase=3 reached, exiting"; exit 0 }

# Phase 4: sign
if (Should-RunPhase 4) {
    $ok = Run-Browseros 4 "sign" @("build", "--sign", "--chromium-src", $ChromiumSrc)
    if (-not $ok) { exit 1 }
}
if (Should-StopAfter 4) { Write-Log "StopAfterPhase=4 reached, exiting"; exit 0 }

# Phase 5: package
if (Should-RunPhase 5) {
    $ok = Run-Browseros 5 "package" @("build", "--package", "--chromium-src", $ChromiumSrc)
    if (-not $ok) { exit 1 }
}

Write-Log "bramburn-build.ps1 finished (all phases ok)"
Write-State -Phase 5 -PhaseName "package" -Status "ok"
exit 0
