# Wait for the RE-RUN of fetch-chromium.ps1 (PID passed in) to reach a terminal
# state. The cache already holds the chromium/src mirror, so this run should be
# far quicker than the first.
param([int]$HostPid = 47312)
$ErrorActionPreference = 'Continue'
$StateFile = 'D:\browseros-build\logs\fetch-chromium-state.json'
$LogFile   = 'D:\browseros-build\logs\fetch-chromium.log'

while ($true) {
    $st = $null
    if (Test-Path $StateFile) {
        try { $st = Get-Content $StateFile -Raw | ConvertFrom-Json } catch {}
    }
    $alive = $null -ne (Get-Process -Id $HostPid -ErrorAction SilentlyContinue)
    $step = if ($st) { "$($st.step)/$($st.status)" } else { 'nostate' }
    $cache = 0
    if (Test-Path 'D:\browseros-build\.gclient_cache') {
        $cache = [math]::Round((Get-ChildItem 'D:\browseros-build\.gclient_cache' -Recurse -Force -ErrorAction SilentlyContinue | Measure-Object -Sum Length).Sum/1GB, 1)
    }
    $srcN = 0
    if (Test-Path 'D:\browseros-build\src') {
        $srcN = (Get-ChildItem 'D:\browseros-build\src' -Force -ErrorAction SilentlyContinue | Measure-Object).Count
    }
    $free = [math]::Round((Get-PSDrive D).Free/1GB, 1)
    Write-Output ("[{0}] {1} alive={2} cache={3}GB srcEntries={4} Dfree={5}GB" -f (Get-Date).ToString('HH:mm:ss'), $step, $alive, $cache, $srcN, $free)

    if ($st -and $st.status -ne 'running') {
        Write-Output "=== TERMINAL: step=$($st.step) status=$($st.status) detail=$($st.detail) ==="
        Write-Output "=== last 30 log lines ==="
        if (Test-Path $LogFile) { Get-Content $LogFile -Tail 30 }
        exit 0
    }
    if (-not $alive) {
        Write-Output "=== HOST EXITED (state was: $step) ==="
        Write-Output "=== last 30 log lines ==="
        if (Test-Path $LogFile) { Get-Content $LogFile -Tail 30 }
        exit 0
    }
    Start-Sleep -Seconds 180
}
