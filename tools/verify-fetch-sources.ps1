# Fetch individual Chromium source files at the pinned release tag.
#
# Why not a full gclient sync: the verification gaps in
# plan/feature-gating-and-native-server.md are all answerable by reading a
# handful of files. A 50 GB / 1-3 h sync is only needed for the actual
# compile, which is a separate, later step.
#
# Source of truth is raw.githubusercontent.com, NOT chromium.googlesource.com.
# googlesource is currently returning HTTP 503 to every client from this host
# (verified for both tags/ and commit SHAs, via Invoke-WebRequest and an
# independent HTTP client), so `gclient sync` against the canonical URL cannot
# work right now. The GitHub chromium/chromium mirror serves the same immutable
# release tags.

param(
    [string]$Tag = "148.0.7778.97",
    [string]$OutDir = "D:\browseros-build\verify"
)

$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$Base = "https://raw.githubusercontent.com/chromium/chromium/{0}" -f $Tag

$Files = @(
    # The about_flags.cc guard question -- is the BrowserOS block inside a
    # platform #if that excludes Windows?
    "chrome/browser/about_flags.cc",
    "chrome/browser/browser_features.h",
    "chrome/browser/browser_features.cc",
    "chrome/browser/flag_descriptions.h",
    "chrome/browser/chrome_browser_main.cc",
    "chrome/browser/BUILD.gn",
    "chrome/browser/browseros/BUILD.gn",
    # net::HttpServer shape, copied from BrowserOSServerProxy.
    "net/server/http_server.h",
    "net/server/http_server_request_info.h",
    "net/server/http_server_response_info.h",
    "net/socket/tcp_server_socket.h",
    "net/http/http_status_code.h",
    # base primitives the implementation leans on.
    "base/threading/thread.h",
    "base/feature_list.h",
    "base/synchronization/sync_event.h",
    "build/config/BUILDCONFIG.gn"
)

$ok = 0
$fail = 0
foreach ($f in $Files) {
    $url = "$Base/$f"
    $dest = Join-Path $OutDir ($f -replace '/', '\')
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
    try {
        $raw = (Invoke-WebRequest -Uri $url -UseBasicParsing -TimeoutSec 120).Content
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($raw)
        [System.IO.File]::WriteAllBytes($dest, $bytes)
        "{0,9:N0} bytes  {1}" -f $bytes.Length, $f
        $ok++
    } catch {
        "  FAILED  $f  -> $($_.Exception.Message)"
        $fail++
    }
}

Write-Output ""
Write-Output "fetched=$ok failed=$fail out=$OutDir"
if ($fail -gt 0) { exit 1 }
exit 0
