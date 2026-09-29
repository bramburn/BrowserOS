# Apply every BrowserOS patch diff to the Chromium tree and report per-file.
#
# This is the step plan/feature-gating-and-native-server.md flags as the one
# that "aborts the entire prep phase" if it fails. Each file under
# chromium_patches/ is a unified diff that replaces or creates one Chromium
# file at the mirrored path, so:
#
#   - order does not matter, because the ownership audit guarantees each
#     Chromium file is claimed by exactly one feature block;
#   - a failure is per-file and reportable, unlike `gclient`/`git apply` on the
#     whole set which just stops at the first error.
#
# Usage:
#   .\apply-patches.ps1 -Mode Check    # dry run, no tree modification
#   .\apply-patches.ps1 -Mode Apply    # actually apply
#   .\apply-patches.ps1 -Mode Apply -Filter native_server

param(
    [string]$SrcDir = "D:\browseros-build\src",
    [string]$PatchRoot = "D:\BrowserOs\.worktrees\feat-auto-20260928-09de393e\packages\browseros\chromium_patches",
    [ValidateSet("Check", "Apply")]
    [string]$Mode = "Check",
    [string]$Filter = "",
    [string]$ReportPath = "D:\browseros-build\logs\patch-apply-report.txt"
)

$ErrorActionPreference = "Continue"
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)

if (-not (Test-Path $SrcDir)) { Write-Error "no Chromium tree at $SrcDir"; exit 1 }

$head = (git -C $SrcDir rev-parse HEAD 2>$null)
$ver = (Get-Content (Join-Path $SrcDir "chrome\VERSION") -ErrorAction SilentlyContinue) -join "."
Write-Host "tree HEAD : $head"
Write-Host "tree VER  : $ver"
Write-Host "mode      : $Mode"
Write-Host ""

# Every file under chromium_patches/ is a unified diff, regardless of the
# Chromium file's extension. Do NOT filter by extension: the set includes .idl,
# .mojom, .pdl, .grd, .grdp, .version, .release and friends, and an
# extension allowlist silently skips them -- which is exactly how
# chrome/common/extensions/api/browser_os.idl went missing and the build failed
# with "no known rule to make it". Detect a diff by its first line instead.
$patches = Get-ChildItem $PatchRoot -Recurse -File | Where-Object {
    $first = Get-Content $_.FullName -TotalCount 1 -ErrorAction SilentlyContinue
    $first -like "diff --git *"
}
if ($Filter) {
    $patches = $patches | Where-Object { $_.FullName -like "*$Filter*" }
}

$ok = 0; $fail = 0; $failures = @()
foreach ($p in $patches) {
    $rel = $p.FullName.Substring($PatchRoot.Length + 1) -replace '\\', '/'
    $args = @("apply", "--whitespace=nowarn", "-p1")
    if ($Mode -eq "Check") { $args += "--check" }
    $args += $p.FullName

    $out = & git -C $SrcDir @args 2>&1
    if ($LASTEXITCODE -eq 0) {
        $ok++
    } else {
        $fail++
        $msg = ($out | Select-Object -First 3) -join " | "
        $failures += "$rel :: $msg"
    }
}

Write-Host "RESULT: ok=$ok fail=$fail (mode=$Mode, total=$($patches.Count))"
if ($failures.Count -gt 0) {
    Write-Host ""
    Write-Host "FAILURES:"
    $failures | Select-Object -First 40 | ForEach-Object { Write-Host "  $_" }
    if ($failures.Count -gt 40) { Write-Host "  ... and $($failures.Count - 40) more" }
}

$report = @(
    "tree HEAD : $head"
    "tree VER  : $ver"
    "mode      : $Mode"
    "ok        : $ok"
    "fail      : $fail"
    ""
) + $failures
[System.IO.File]::WriteAllLines($ReportPath, $report, $Utf8NoBom)
Write-Host ""
Write-Host "report: $ReportPath"

if ($fail -gt 0) { exit 1 }
exit 0
