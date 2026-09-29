# Resume the Chromium sync past googlesource rate limiting.
#
# Why this exists: a plain `gclient sync` on this host downloads for ~45 min and
# then dies with HTTP 429 ("The requested URL returned error: 429") on whichever
# dep happens to be in flight. gclient turns that into
# `git_cache.ClobberNeeded` and marks that ONE cache mirror as corrupt, leaving
# the other ~120 mirrors intact. Re-running the sync from scratch throws all of
# that away.
#
# So: retry in a loop, and on failure remove only the cache mirror gclient
# actually complained about. Every attempt resumes from the caches that already
# succeeded, so forward progress is monotonic across attempts.
#
# A caution about the 503: chromium.googlesource.com's gitiles endpoint
# (…?format=TEXT) returns 503 from this host, but the git smart-HTTP endpoint
# (/info/refs?service=git-upload-pack) returns 200. The sync only needs the
# latter, so the 503 does not block this script.

param(
    [string]$BuildRoot = "D:\browseros-build",
    [string]$DepotTools = "C:\dev\depot_tools",
    [int]$MaxAttempts = 15,
    [int]$BackoffSeconds = 90
)

$ErrorActionPreference = "Continue"
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
$env:GIT_TERMINAL_PROMPT = "0"
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)

$LogDir = Join-Path $BuildRoot "logs"
$LogFile = Join-Path $LogDir "sync-resume.log"
$StateFile = Join-Path $LogDir "sync-resume-state.json"
$Gclient = Join-Path $DepotTools "gclient.bat"
$CacheDir = Join-Path $BuildRoot ".gclient_cache"

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $line = "[" + (Get-Date).ToString("HH:mm:ss") + "] [" + $Level + "] " + $Message + "`r`n"
    [System.IO.File]::AppendAllText($LogFile, $line, $Utf8NoBom)
    Write-Host $Message
}

function Write-State {
    param([string]$Status, [string]$Detail, [int]$Attempt)
    $state = @{
        status = $Status; detail = $Detail; attempt = $Attempt
        updated = (Get-Date).ToString("o")
    }
    [System.IO.File]::WriteAllText($StateFile, ($state | ConvertTo-Json), $Utf8NoBom)
}

function Get-CacheSizeGB {
    if (-not (Test-Path $CacheDir)) { return 0 }
    $sum = (Get-ChildItem $CacheDir -Recurse -Force -File -ErrorAction SilentlyContinue |
            Measure-Object -Sum Length).Sum
    return [math]::Round($sum / 1GB, 1)
}

# Run gclient, tee raw stdout/stderr to the log, return the exit code.
#
# Do NOT use Start-Process -Wait -PassThru here. Combined with output redirection
# it returns a Process whose ExitCode is $null, and it returns it *immediately*
# rather than waiting -- which is why an earlier version of this script logged
# "gclient exit=" with no value and no elapsed time, and why the very first
# fetch attempt looked like an instant failure. The .NET WaitForExit() call
# populates ExitCode reliably.
function Invoke-Gclient {
    param([string[]]$GclientArgs)
    $out = Join-Path $LogDir "_sync_out.tmp"
    $err = Join-Path $LogDir "_sync_err.tmp"
    $p = Start-Process -FilePath $Gclient -ArgumentList $GclientArgs `
        -WorkingDirectory $BuildRoot -NoNewWindow -PassThru `
        -RedirectStandardOutput $out -RedirectStandardError $err
    $p.WaitForExit()
    $code = $p.ExitCode
    foreach ($t in @($out, $err)) {
        if ((Test-Path $t) -and (Get-Item $t).Length -gt 0) {
            [System.IO.File]::AppendAllText($LogFile, [System.IO.File]::ReadAllText($t), $Utf8NoBom)
        }
    }
    return $code
}

# The cache mirror gclient just choked on, if we can name it.
function Get-ClobberedMirror {
    $tail = ""
    try { $tail = Get-Content $LogFile -Tail 400 -ErrorAction SilentlyContinue } catch { return $null }
    $m = [regex]::Matches(($tail -join "`n"), '\.gclient_cache\\([A-Za-z0-9_.\-]+)')
    if ($m.Count -eq 0) { return $null }
    return $m[$m.Count - 1].Groups[1].Value
}

if ($env:PATH -notlike "*$DepotTools*") { $env:PATH = $DepotTools + ";" + $env:PATH }

Write-Log "=== resume sync starting (buildRoot=$BuildRoot) ==="
Write-Log ("cache at start: " + (Get-CacheSizeGB) + " GB")

$exit = 1
for ($attempt = 1; $attempt -le $MaxAttempts; $attempt++) {
    Write-Log "--- attempt $attempt/$MaxAttempts ---"
    Write-State "running" "attempt $attempt" $attempt

    $exit = Invoke-Gclient -GclientArgs @("sync", "--no-history", "--shallow")
    $gb = Get-CacheSizeGB
    Write-Log ("gclient exit=$exit ; cache now $gb GB")

    if ($exit -eq 0) {
        Write-Log "SYNC COMPLETE"
        Write-State "done" "exit 0 on attempt $attempt" $attempt
        exit 0
    }

    $mirror = Get-ClobberedMirror
    if ($mirror) {
        $target = Join-Path $CacheDir $mirror
        Write-Log "clobbering cache mirror: $mirror" "WARN"
        if (Test-Path $target) {
            # Recoverable removal so a bad mirror can be re-fetched.
            & mavis-trash $target 2>&1 | Out-Null
            Write-Log "  removed (or scheduled for trash)"
        }
    } else {
        Write-Log "could not identify the clobbered mirror from the log" "WARN"
    }

    if ($attempt -lt $MaxAttempts) {
        Write-Log "sleeping $BackoffSeconds s before retry (429 backoff)"
        Start-Sleep -Seconds $BackoffSeconds
    }
}

Write-Log "giving up after $MaxAttempts attempts" "ERROR"
Write-State "failed" "exit $exit after $MaxAttempts attempts" $MaxAttempts
exit $exit
