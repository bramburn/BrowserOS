---
title: Advance Installer Pro
description: Optional AIP layer for polished install UX (v2).
---

# Advance Installer Pro (optional)

Advance Installer Pro (AIP) is a third-party Windows installer
authoring tool from Caphyon. It produces polished `.exe` installers
with rich wizard UIs, branding, prerequisites, and update hooks.

The fork uses AIP as an **optional v2 layer** on top of the upstream
`mini_installer`. The base install (file copy, registry, services)
still comes from `mini_installer` — AIP wraps it in a friendlier
shell.

## Where AIP lives

| Path | Purpose |
|---|---|
| `C:\Program Files (x86)\Caphyon\Advanced Installer 23.9\bin\x86\AdvancedInstaller.com` | CLI build entry point |
| `C:\Program Files (x86)\Caphyon\Advanced Installer 23.9\bin\x86\advinst.exe` | GUI editor |
| `installer/browseros.aip.template` | The fork's project template (this repo) |

AIP is licensed per-machine. The license is configured in the GUI
(`advinst.exe`) under `Help → License`. For CI, the license must be
exported and applied on the runner before any `AdvancedInstaller.com`
invocation.

## When to use AIP

| Symptom in mini_installer | AIP capability that fixes it |
|---|---|
| Bare "Installing BrowserOS..." dialog with no progress | AIP custom wizard pages with progress + branding |
| No first-run wizard | AIP launch conditions + custom actions |
| No Chrome / Edge / Firefox data import | AIP prerequisite + custom action that calls our import script |
| No "What's new" page on update | AIP launches the forked About page from a custom action |
| No clean uninstall | AIP tracks install metadata + offers user-data choice on uninstall |

## Build flow with AIP

```
[1] bramburn-build.ps1 phase 5: produces
    C:\browersos-build\src\out\Default\mini_installer.exe

[2] (optional) AdvancedInstaller.com /build installer/browseros.aip
    - input: the mini_installer.exe from step 1
    - output: BrowserOS-AIP-v<version>_win-x64.exe (polished)
```

## Enabling AIP in the release pipeline

The `release-windows.yml` workflow accepts an optional input:

```yaml
wrap_with_aip:
  description: "Wrap mini_installer.exe with Advance Installer Pro"
  type: boolean
  default: false
```

When `true`, the workflow runs an additional step that calls
`AdvancedInstaller.com /build installer/browseros.aip`. AIP must be
installed on the self-hosted runner (currently only this dev box).

When `false` (default), the polished `.exe` is skipped and only the
plain `mini_installer.exe` is published.

## Project template

A starting point lives at `installer/browseros.aip.template` in this
repo. It contains:

- AppInfo: name, version, publisher, icon
- Prerequisites: Visual C++ Redistributable x64 (the only one the
  fork actually needs)
- Launch conditions: Windows 10 1809+ x64
- Files: the `mini_installer.exe` from step 1
- Wizard pages: welcome + license + install + finish
- Custom actions: data import hook (TBD script), update hint

Filling out the wizard content (branding images, license text, copy)
is out of scope for the v1 release pipeline — that's a separate
"polish" pass.

## License handling for CI

The self-hosted runner needs an exported AIP license. To set this up:

1. On this dev box (with AIP installed and licensed), open
   `advinst.exe → Help → License → Export License`.
2. Save the exported file to a GitHub Actions secret
   (`AIP_LICENSE_BASE64`).
3. In the workflow's AIP step:
   ```powershell
   $bytes = [Convert]::FromBase64String($env:AIP_LICENSE_BASE64)
   [IO.File]::WriteAllBytes("$env:LOCALAPPDATA\Caphyon\License\advinst.lic", $bytes)
   ```
4. Then `AdvancedInstaller.com /build installer/browseros.aip`.

For dev iteration on this box, the license is already configured
globally, so manual builds Just Work.