# Advance Installer Pro project template

This directory is the starting point for the **optional** AIP layer
that polishes the upstream `mini_installer.exe` into a friendly
Windows installer.

See [`docs-site/docs/advance-installer.md`](../docs-site/docs/advance-installer.md)
for the full writeup, and [`docs/UPDATE_SERVER.md`](../docs/UPDATE_SERVER.md)
for the release/update flow.

## Files

| File | Role |
|---|---|
| `browseros.aip.template` | The AIP project source. **You edit this in `advinst.exe` (the GUI) and save as `browseros.aip`.** |
| `README.md` | This file. |

## Build (manual, on this dev box)

```powershell
cd installer
# Open in the GUI to author:
& "C:\Program Files (x86)\Caphyon\Advanced Installer 23.9\bin\x86\advinst.exe" browseros.aip
# Build from the command line:
& "C:\Program Files (x86)\Caphyon\Advanced Installer 23.9\bin\x86\AdvancedInstaller.com" /build browseros.aip
```

Output: `installer\BrowserOS-Setup-v<version>_win-x64.exe` (rename as
needed for the release pipeline).

## Build (via the release workflow)

The `release-windows.yml` workflow has an `wrap_with_aip` input
(default `false`). When `true` and a `browseros.aip` exists in this
directory, the workflow runs `AdvancedInstaller.com /build` on the
self-hosted runner and uploads the resulting `.exe` to R2 + GitHub
Release alongside the plain `mini_installer.exe`.