# Drive `gclient sync` to completion on a tree pinned to an OLD release tag.
#
# The problem this solves
# ----------------------
# The gclient cache mirrors are built with `--depth 10000` from the default
# branch. When you then pin src to an older release tag, some DEPS revisions sit
# *below* the mirror's shallow root, so git refuses them:
#
#   warning: rejected <sha> because shallow roots are not allowed to be updated
#   error: Could not read <sha>
#   Error: git rev-parse FETCH_HEAD returned non-zero exit status 128 in <src path>
#
# gclient surfaces that as a hard failure and stops, even though only one dep is
# affected. `--no-history --shallow` is the wrong pairing for an old tag.
#
# What this does
# --------------
# Runs sync in a loop. On each failure it:
#   1. reads the failing checkout path and the unreadable revision from stderr,
#   2. maps that path to its cache mirror via the checkout's own `origin` URL,
#   3. asks the mirror to fetch that exact revision by SHA,
#   4. falls back to deepening the mirror if the host refuses fetch-by-SHA.
# Then retries. Each cycle settles at least one dep and never discards the ones
# already correct, so it converges.
#
# Usage:
#   .\sync-tag-loop.ps1 -MaxCycles 25

param(
    [string]$BuildRoot = "D:\browseros-build",
    [string]$DepotTools = "C:\dev\depot_tools",
    [int]$MaxCycles = 25,
    [int]$Deepen = 20000
)

$ErrorActionPreference = "Continue"
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
$env:GIT_TERMINAL_PROMPT = "0"
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)

$LogDir = Join-Path $BuildRoot "logs"
$LogFile = Join-Path $LogDir "sync-tag-loop.log"
$StateFile = Join-Path $LogDir "sync-tag-loop-state.json"
$Gclient = Join-Path $DepotTools "gclient.bat"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $line = "[" + (Get-Date).ToString("HH:mm:ss") + "] [" + $Level + "] " + $Message + "`r`n"
    [System.IO.File]::AppendAllText($LogFile, $line, $Utf8NoBom)
    Write-Host $Message
}

function Write-State {
    param([string]$Status, [string]$Detail, [int]$Cycle)
    $s = @{ status = $Status; detail = $Detail; cycle = $Cycle; updated = (Get-Date).ToString("o") }
    [System.IO.File]::WriteAllText($StateFile, ($s | ConvertTo-Json), $Utf8NoBom)
}

if ($env:PATH -notlike "*$DepotTools*") { $env:PATH = $DepotTools + ";" + $env:PATH }

Write-Log "=== tag sync loop starting ==="

for ($cycle = 1; $cycle -le $MaxCycles; $cycle++) {
    $o = Join-Path $LogDir "_loop_out.tmp"
    $e = Join-Path $LogDir "_loop_err.tmp"
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process -FilePath $Gclient -ArgumentList @("sync", "--no-history", "--shallow") `
        -WorkingDirectory $BuildRoot -NoNewWindow -PassThru `
        -RedirectStandardOutput $o -RedirectStandardError $e
    $p.WaitForExit()
    $secs = [math]::Round($sw.Elapsed.TotalSeconds, 1)
    Write-Log "cycle $cycle : gclient finished in $secs s"
    foreach ($t in @($o, $e)) {
        if ((Test-Path $t) -and (Get-Item $t).Length -gt 0) {
            [System.IO.File]::AppendAllText($LogFile, [System.IO.File]::ReadAllText($t), $Utf8NoBom)
        }
    }

    $errText = ""
    if (Test-Path $e) { $errText = [System.IO.File]::ReadAllText($e) }

    $failed = $errText -match "returned non-zero exit status"
    if (-not $failed) {
        Write-Log "NO DEP ERRORS - sync converged on cycle $cycle"
        Write-State "done" "clean on cycle $cycle" $cycle
        exit 0
    }

    # The revision git could not read, and the checkout that wanted it.
    $sha = $null
    if ($errText -match "Could not read ([0-9a-f]{40})") { $sha = $Matches[1] }
    $depPath = $null
    if ($errText -match "exit status \d+ in ([A-Za-z]:\\[^\r\n]+?)\s*\r?\n") { $depPath = $Matches[1].Trim() }
    if (-not $sha -or -not $depPath) {
        Write-Log "could not parse failure (sha=$sha dep=$depPath) - see log" "WARN"
        Write-State "failed" "unparseable failure on cycle $cycle" $cycle
        exit 2
    }

    $mirror = (& git -C $depPath remote get-url origin 2>$null)
    if (-not $mirror -or -not (Test-Path $mirror)) {
        Write-Log "no mirror for $depPath (origin='$mirror') - see log" "ERROR"
        Write-State "failed" "no mirror for $depPath" $cycle
        exit 3
    }

    Write-Log "  dep    : $depPath"
    Write-Log "  want   : $sha"
    Write-Log "  mirror : $mirror"

    # Strategy 1: ask for the exact revision.
    $f1 = & git -C $mirror fetch --depth 1 origin $sha 2>&1
    if ($LASTEXITCODE -eq 0 -and (& git -C $mirror cat-file -t $sha 2>$null) -eq "commit") {
        Write-Log "  fixed by targeted fetch of $sha" "OK"
        continue
    }

    # Strategy 2: the host refuses fetch-by-SHA, so deepen instead.
    Write-Log "  targeted fetch refused; deepening mirror by $Deepen"
    $f2 = & git -C $mirror fetch --deepen $Deepen origin 2>&1
    [System.IO.File]::AppendAllText($LogFile, ($f2 | Out-String), $Utf8NoBom)
    if ((& git -C $mirror cat-file -t $sha 2>$null) -eq "commit") {
        Write-Log "  fixed by deepening to reach $sha" "OK"
        continue
    }

    Write-Log "  could not obtain $sha - aborting" "ERROR"
    Write-State "failed" "unobtainable revision $sha for $depPath" $cycle
    exit 4
}

Write-Log "exhausted $MaxCycles cycles" "WARN"
Write-State "failed" "cycle limit reached" $MaxCycles
exit 5
