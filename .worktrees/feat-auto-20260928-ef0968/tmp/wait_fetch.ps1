# Wait for fetch-chromium.ps1 to finish (state.status != running) or its host
# process to exit. Prints a heartbeat so the output stream shows progress.
$ErrorActionPreference = 'Continue'
$StateFile = 'D:\browseros-build\logs\fetch-chromium-state.json'
$LogFile   = 'D:\browseros-build\logs\fetch-chromium.log'
$HostPid   = 14036   # powershell.exe running fetch-chromium.ps1

function Get-Status {
    if (Test-Path $StateFile) {
        try { return (Get-Content $StateFile -Raw | ConvertFrom-Json) } catch { return $null }
    }
    return $null
}

$lastStep = ''
while ($true) {
    $st = Get-Status
    $alive = $null -ne (Get-Process -Id $HostPid -ErrorAction SilentlyContinue)
    $step = if ($st) { "$($st.step)/$($st.status)" } else { 'nostate' }
    $cache = 0
    if (Test-Path 'D:\browseros-build\.gclient_cache') {
        $cache = [math]::Round((Get-ChildItem 'D:\browseros-build\.gclient_cache' -Recurse -Force -ErrorAction SilentlyContinue | Measure-Object -Sum Length).Sum/1GB, 1)
    }
    $srcSize = 0
    if (Test-Path 'D:\browseros-build\src') {
        $srcSize = [math]::Round((Get-ChildItem 'D:\browseros-build\src' -Force -ErrorAction SilentlyContinue | Measure-Object).Count, 0)
    }
    $free = [math]::Round((Get-PSDrive D).Free/1GB, 1)
    Write-Output ("[{0}] {1} alive={2} cache={3}GB srcEntries={4} Dfree={5}GB" -f (Get-Date).ToString('HH:mm:ss'), $step, $alive, $cache, $srcSize, $free)

    if ($st -and $st.status -ne 'running') {
        Write-Output "=== TERMINAL STATE: step=$($st.step) status=$($st.status) detail=$($st.detail) ==="
        Write-Output "=== last 40 log lines ==="
        if (Test-Path $LogFile) { Get-Content $LogFile -Tail 40 }
        exit 0
    }
    if (-not $alive) {
        Write-Output "=== HOST PROCESS EXITED (state still: $step) ==="
        Write-Output "=== last 40 log lines ==="
        if (Test-Path $LogFile) { Get-Content $LogFile -Tail 40 }
        exit 0
    }
    Start-Sleep -Seconds 120
}
