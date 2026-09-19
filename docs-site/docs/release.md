---
title: Release
description: How to publish a release of the bramburn/BrowserOS fork.
---

# Publishing a release

The release pipeline has three stages:

```
[1] Build (bramburn-build.ps1, self-hosted runner, 7-13h)
  └─ produces BrowserOS_v<version>_win-x64.exe
[2] Publish (release-windows.yml, GitHub Actions, ~5 min)
  └─ uploads to R2 + creates GitHub Release + tags browseros-windows-v<version>
[3] Update manifest (update-manifest.yml, GitHub Actions, ~2 min)
  └─ writes Omaha-4 XML + appcast RSS to R2
```

## Triggering a release

### Manual (recommended for first release)

1. Open GitHub → Actions → **Release BrowserOS Windows** → **Run workflow**.
2. Fill in:
   - `version`: e.g. `0.1.0`. Must match `BROWSEROS_VERSION` after the bump.
   - `bump_mode`: `offset+build` for nightly-style, `none` for manual.
   - `upload_to_r2`: `true` to publish to CDN.
3. Watch the run. The bump step pushes a PR; the build runs on the
   self-hosted runner.

### Nightly (once #1 auto-update is shippable)

The workflow also runs on schedule `0 2 * * *` (02:00 UTC) with
`bump_mode=offset+build` and `upload_to_r2=true`. Every successful
nightly run produces a new candidate that the update server can serve.

## Tag conventions

| Tag pattern | Meaning |
|---|---|
| `browseros-windows-v<version>` | The fork's Windows Chromium build. |
| `browseros-cli-v<version>` | The Go CLI (npm). |
| `browseros-server-v<version>` | The Bun MCP server (AGPL npm). |
| `agent-extension-v<version>` | The browser extension (WXT/React). |

GitHub Releases are created from these tags. R2 has the same paths
mirrored under `<r2-prefix>/<artifact-path>`.

## R2 layout for browseros artifacts

```
<r2-prefix>/
  <version>/
    BrowserOS_v<version>_win-x64.exe            # the installer
  windows/
    update_check.xml                            # Omaha-4 response (in-browser updater)
    appcast.xml                                 # Sparkle RSS (browseros-cli)
  latest.json                                   # version pointer (CLI quick check)
```

Public URL prefix (via Cloudflare): `https://cdn.bramburn.com/<r2-prefix>/...`

## Required GitHub configuration

### Repo variables (Settings → Secrets and variables → Actions → Variables)

| Variable | Example | Used by |
|---|---|---|
| `BROWSEROS_REPO_PATH` | `C:\dev\BrowserOs` | Self-hosted runner (Windows + macOS) |
| `BROWSEROS_CHROMIUM_SRC` | `C:\browersos-build\src` | Self-hosted runner (Windows + macOS) |
| `BROWSEROS_NIGHTLY_REF` | `main` | Nightly macOS build |
| `FORK_R2_PREFIX` | `browseros` | Update-manifest R2 key prefix |
| `FORK_CDN_BASE` | `https://cdn.bramburn.com` | Public CDN base URL |
| `BROWSEROS_APP_ID` | `{B49B6F23-3D5C-4F7D-9F0F-1F5E5D4C4D9F}` | Stable Windows app GUID for Omaha-4 |

### Repo secrets

| Secret | Used by |
|---|---|
| `R2_ACCOUNT_ID` | R2 uploads (CLI + manifests) |
| `R2_ACCESS_KEY_ID` | R2 uploads |
| `R2_SECRET_ACCESS_KEY` | R2 uploads |
| `R2_BUCKET` | R2 uploads |
| `CODE_SIGN_TOOL_PATH` | `sign_windows` step (SSL.com eSigner) |
| `ESIGNER_USERNAME` / `_password` / `_TOTP_SECRET` | SSL.com eSigner auth |
| `POSTHOG_API_KEY` | CLI build (optional) |
| `NPM_TOKEN` | CLI npm publish |

## Manual fallback (no CI)

If the self-hosted runner is offline, you can publish manually:

1. Run `tools\bramburn-build.ps1` interactively to produce the `.exe`.
2. Compute SHA-256: `Get-FileHash .\BrowserOS_v<ver>_win-x64.exe`.
3. Upload to R2 manually:
   ```powershell
   bun packages/build-tools/scripts/upload-to-r2.ts `
     --file .\BrowserOS_v<ver>_win-x64.exe `
     --key "browseros/<ver>/BrowserOS_v<ver>_win-x64.exe" `
     --content-type application/octet-stream
   ```
4. Run `tools\release\generate_update_manifests.py` to write the
   manifest XML files, then upload them with the same Bun script.
5. Create the GitHub release + tag manually:
   ```bash
   git tag -a browseros-windows-v<ver> -m "browseros-windows v<ver>"
   git push origin browseros-windows-v<ver>
   gh release create browseros-windows-v<ver> --title "..." --notes "..."
   ```