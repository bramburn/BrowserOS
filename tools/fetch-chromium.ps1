# Fetch the pinned Chromium source tree for BrowserOS.
#
# SOURCE-ONLY. This runs `gclient sync` to populate src/ with Chromium + its
# DEPS, then checks out the tag pinned in packages/browseros/CHROMIUM_VERSION
# and re-syncs so the DEPS match that tag. It does NOT apply BrowserOS
# patches, configure GN, or compile -- those stay with phases 2-3 of
# tools/bramburn-build.ps1.
#
# Why the tree is ~50 GB: ~30 GB of src/ plus a ~20 GB CIPD package cache.
# A subsequent autoninja run adds ~100 GB in src/out/Default.
#
# .gclient is written with cache_dir inside the build root on purpose. If
# cache_dir is left pointing at C:, gclient re-downloads the whole CIPD
# cache onto the system drive even when src/ lives on D: -- that is what
# burned ~20 GB on C:\browersos-build\.gclient_cache before it was trashed.
#
# Usage (from PowerShell, NOT from the bash tool):
#   & <repo>\tools\fetch-chromium.ps1
#
# Detached (recommended -- this takes 1-3 h):
#   Start-Process -FilePath "C:\Python312\python.exe" `
#     -ArgumentList "<repo>\tools\fetch-chromium.ps1" `
#     -WindowStyle Hidden `
#     -RedirectStandardOutput "<BuildRoot>\logs\fetch-stdout.log" `
#     -RedirectStandardError  "<BuildRoot>\logs\fetch-stderr.log"
#
# Then watch:
#   Get-Content <BuildRoot>\logs\fetch-chromium.log -Tail 40 -Wait

param(
    [string]$BuildRoot = "D:\browseros-build",
    [string]$DepotTools = "C:\dev\depot_tools",
    [switch]$SkipTagCheckout
)

$ErrorActionPreference = "Continue"
# Match bramburn-build.ps1: gclient emits Unicode and needs UTF-8 to log it.
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
# PS 5.1 defaults redirection to UTF-16, which makes the log unreadable.
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)

$ForkRoot = (Split-Path -Parent $PSScriptRoot)
$SrcDir = Join-Path $BuildRoot "src"
$LogDir = Join-Path $BuildRoot "logs"
$LogFile = Join-Path $LogDir "fetch-chromium.log"
$StateFile = Join-Path $LogDir "fetch-chromium-state.json"
$Gclient = Join-Path $DepotTools "gclient.bat"
$VersionFile = Join-Path $ForkRoot "packages\browseros\CHROMIUM_VERSION"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $line = "[" + (Get-Date).ToString("o") + "] [" + $Level + "] " + $Message + "`r`n"
    [System.IO.File]::AppendAllText($LogFile, $line, $Utf8NoBom)
    Write-Host $Message
}

function Write-State {
    param([string]$Step, [string]$Status, [string]$Detail = "")
    $state = @{
        step      = $Step
        status    = $Status
        detail    = $Detail
        buildRoot = $BuildRoot
        src       = $SrcDir
        updated   = (Get-Date).ToString("o")
    }
    [System.IO.File]::WriteAllText(
        $StateFile, ($state | ConvertTo-Json), $Utf8NoBom)
}

# Run a native command and append its raw output to the log.
#
# Two things this gets right that a bare `& $exe args *>> $LogFile` does not:
#
#  1. WorkingDirectory. gclient calls GClient.LoadCurrentConfig(), which reads
#     .gclient from the *current directory* and its parents. Run gclient from
#     anywhere else and it aborts with "client not configured; see 'gclient
#     config'" -- which looks like a CIPD/auth failure but is really just a
#     missing .gclient. tools/bramburn-build.ps1 never hits this because the
#     browseros CLI chdir's for us.
#  2. Encoding. PowerShell's *>> redirection emits UTF-16 under PS 5.1, so the
#     captured gclient output comes back as spaced-out mojibake. Start-Process
#     redirects raw bytes, so the log stays readable UTF-8.
#
# Returns the process exit code.
function Invoke-Native {
    param(
        [string]$Exe,
        [string[]]$Args,
        [string]$WorkDir
    )
    $outTmp = Join-Path $LogDir "_native_out.tmp"
    $errTmp = Join-Path $LogDir "_native_err.tmp"
    foreach ($t in @($outTmp, $errTmp)) {
        if (Test-Path $t) { Remove-Item $t -Force }
    }

    $proc = Start-Process -FilePath $Exe -ArgumentList $Args -WorkingDirectory $WorkDir `
        -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $outTmp -RedirectStandardError $errTmp

    foreach ($t in @($outTmp, $errTmp)) {
        if ((Test-Path $t) -and (Get-Item $t).Length -gt 0) {
            $text = [System.IO.File]::ReadAllText($t)
            [System.IO.File]::AppendAllText($LogFile, $text, $Utf8NoBom)
        }
    }
    return $proc.ExitCode
}

# Read MAJOR/MINOR/BUILD/PATCH from CHROMIUM_VERSION into 148.0.7778.97.
function Get-PinnedVersion {
    if (-not (Test-Path $VersionFile)) {
        Write-Log "CHROMIUM_VERSION not found at $VersionFile" "ERROR"
        return $null
    }
    $kv = @{}
    foreach ($line in (Get-Content $VersionFile)) {
        if ($line -match '^\s*([A-Z_]+)\s*=\s*(.+?)\s*$') { $kv[$Matches[1]] = $Matches[2] }
    }
    if (-not $kv.ContainsKey("MAJOR") -or -not $kv.ContainsKey("BUILD")) { return $null }
    return "$($kv['MAJOR']).$($kv['MINOR']).$($kv['BUILD']).$($kv['PATCH'])"
}

New-Item -ItemType Directory -Force -Path $BuildRoot, $LogDir | Out-Null
if (Test-Path $LogFile) { Move-Item $LogFile ($LogFile + ".prev") -Force }

Write-Log "fetch-chromium.ps1 starting"
Write-Log ("BuildRoot:   " + $BuildRoot)
Write-Log ("SrcDir:      " + $SrcDir)
Write-Log ("DepotTools:  " + $DepotTools)
Write-Log ("depot_tools commit: " + (& git -C $DepotTools rev-parse --short HEAD 2>&1))

if (-not (Test-Path $Gclient)) {
    Write-Log "gclient not found at $Gclient" "ERROR"
    Write-State "preflight" "failed" "gclient missing"
    exit 1
}

# gclient resolves sibling depot_tools helpers off PATH; without this the
# sync fails partway in with a confusing "command not found".
if ($env:PATH -notlike "*$DepotTools*") {
    $env:PATH = $DepotTools + ";" + $env:PATH
    Write-Log "prepended depot_tools to PATH"
}

# .gclient is a Python file, so backslashes in cache_dir must be escaped.
$cacheDirPy = (Join-Path $BuildRoot ".gclient_cache").Replace('\', '\\')
$gclientConfig = @"
solutions = [
  {
    "name": "src",
    "url": "https://chromium.googlesource.com/chromium/src.git",
    "managed": False,
    "custom_deps": {},
    "custom_vars": {},
  },
]
target_os = ["win"]
cache_dir = "$cacheDirPy"
"@
$gclientPath = Join-Path $BuildRoot ".gclient"
[System.IO.File]::WriteAllText($gclientPath, $gclientConfig, $Utf8NoBom)
Write-Log "wrote $gclientPath (cache_dir -> $cacheDirPy)"

# Step 1: bootstrap. Clones src/ at the default branch and pulls DEPS.
# This has to happen before the tag checkout: src/ does not exist yet.
# WorkingDirectory is $BuildRoot, NOT $SrcDir -- src/ is what we are creating,
# and gclient has to find .gclient one level up from where it will place it.
Write-State "bootstrap-sync" "running"
Write-Log "=== step 1/2: gclient sync (bootstrap, ~50 GB, 1-3 h) ==="
# --shallow is fine at the bootstrap (we sync the default branch here); only
# the re-sync after moving to the older pin needs the extra history. --jobs 8
# is not optional: at the default -j 32 this trips googlesource's rate limiter
# and depot_tools reports it as ClobberNeeded "Corrupted cache."
$bootstrapExit = Invoke-Native -Exe $Gclient `
    -Args @("sync", "--no-history", "--shallow", "--jobs", "8") -WorkDir $BuildRoot
Write-Log ("gclient sync exit=" + $bootstrapExit)
if ($bootstrapExit -ne 0) {
    Write-Log "bootstrap sync failed -- see $LogFile" "ERROR"
    Write-State "bootstrap-sync" "failed" "exit $bootstrapExit"
    exit $bootstrapExit
}

if ($SkipTagCheckout) {
    Write-Log "-SkipTagCheckout set; leaving src at the default branch."
    Write-Log "Run 'browseros build --setup' to check out the pinned tag."
    Write-State "bootstrap-sync" "done" "skipped tag checkout"
    exit 0
}

# Step 2: move to the pinned commit, then re-sync so DEPS match that commit.
# Syncing DEPS at main and then checking out an old pin leaves the tree with
# the wrong dependency revisions -- the checkout and the DEPS must agree.
#
# We pin by BASE_COMMIT (a SHA), not by tag. chromium.googlesource.com's
# mirror of chromium/src stopped receiving release tags after M59 -- the
# highest tag there is 59.0.3065.2, and `git ls-remote --tags` returns ~9959
# tags with no 14x.* among them. So `git checkout tags/148.0.7778.97` can
# never resolve and fails with "couldn't find remote ref". BASE_COMMIT is the
# very commit that increments chrome/VERSION to the pinned string, so it is
# the same commit the tag would have named.
$version = Get-PinnedVersion
if (-not $version) {
    Write-Log "could not read pinned version" "ERROR"
    Write-State "tag-checkout" "failed" "CHROMIUM_VERSION unreadable"
    exit 1
}

$BaseCommitFile = Join-Path $ForkRoot "packages\browseros\BASE_COMMIT"
$baseCommit = $null
if (Test-Path $BaseCommitFile) {
    $baseCommit = (Get-Content $BaseCommitFile -Raw).Trim()
}
if (-not $baseCommit) {
    Write-Log "BASE_COMMIT missing or empty at $BaseCommitFile" "ERROR"
    Write-State "tag-checkout" "failed" "BASE_COMMIT unreadable"
    exit 1
}
if ($baseCommit -notmatch '^[0-9a-f]{40}$') {
    Write-Log "BASE_COMMIT is not a 40-char SHA: '$baseCommit'" "ERROR"
    Write-State "tag-checkout" "failed" "BASE_COMMIT malformed"
    exit 1
}

Write-State "tag-checkout" "running" $version
Write-Log ("=== step 2/2: checking out base commit " + $version + " (" + $baseCommit.Substring(0,12) + ") ===")

# googlesource accepts fetch-by-SHA, so this resolves the exact pinned commit
# without needing a tag or a full-history fetch.
$fetchExit = Invoke-Native -Exe "git" -Args @("fetch", "--force", "origin", $baseCommit) -WorkDir $SrcDir
if ($fetchExit -ne 0) {
    Write-Log "git fetch of base commit failed -- see $LogFile" "ERROR"
    Write-State "tag-checkout" "failed" "git fetch exit $fetchExit"
    exit 1
}

$checkoutExit = Invoke-Native -Exe "git" -Args @("checkout", $baseCommit) -WorkDir $SrcDir
if ($checkoutExit -ne 0) {
    Write-Log "git checkout $baseCommit failed" "ERROR"
    Write-State "tag-checkout" "failed" "git checkout exit $checkoutExit"
    exit 1
}

$head = (& git -C $SrcDir rev-parse HEAD 2>&1).ToString().Trim()
Write-Log ("HEAD is now " + $head)
if ($head -ne $baseCommit) {
    Write-Log "HEAD does not match BASE_COMMIT (got $head)" "ERROR"
    Write-State "tag-checkout" "failed" "HEAD mismatch after checkout"
    exit 1
}
Write-Log "HEAD matches BASE_COMMIT"

Write-Log "re-syncing DEPS to match the pinned commit"
# --no-history WITHOUT --shallow for the re-sync: a cache mirror bootstrapped
# with --shallow inherits a depth boundary from the default branch, and DEPS
# revisions for an older pinned commit can sit below it. gclient then reports
# "rejected <sha> because shallow roots are not allowed to be updated" and
# aborts the entire sync. --jobs 8 keeps us under googlesource's rate limiter
# (gclient defaults to -j 32, which trips HTTP 429; depot_tools then misreports
# it as ClobberNeeded "Corrupted cache."). Measured: dozens of 429s at 32,
# zero at 8.
$resyncExit = Invoke-Native -Exe $Gclient `
    -Args @("sync", "--no-history", "--jobs", "8") -WorkDir $BuildRoot
if ($resyncExit -ne 0) {
    Write-Log "DEPS re-sync failed -- see $LogFile" "ERROR"
    Write-State "tag-checkout" "failed" "resync exit $resyncExit"
    exit $resyncExit
}

$volume = (Split-Path -Qualifier $BuildRoot).TrimEnd(':')
$free = [math]::Round((Get-PSDrive -Name $volume).Free / 1GB, 1)
Write-Log "fetch complete at $version; $free GB free on ${volume}:"
Write-State "tag-checkout" "done" $version
Write-Log "next: & <repo>\tools\bramburn-build.ps1 -StopAfterPhase 1  (applies patches + GN config)"
exit 0
