# CI and Releases — bramburn/BrowserOS

**Date**: 2026-09-19
**Scope**: GitHub Actions CI, self-hosted Windows runner, R2-backed
release pipeline, Omaha-4 + appcast update manifests.

This doc is the in-repo developer reference. The rendered version
lives at <https://bramburn.github.io/BrowserOS/> (Docusaurus). Source
of truth for both is the same set of `.github/workflows/*.yml` files
plus `tools/release/generate_update_manifests.py`.

## What exists today

### Inherited from upstream (no changes)

| Workflow | Trigger | Runner | Role |
|---|---|---|---|
| `release-cli.yml` | workflow_dispatch + tag | ubuntu-latest | Go CLI → npm + R2 + GitHub Release |
| `release-server.yml` | workflow_dispatch | ubuntu-latest | Bun MCP server → GitHub Release + npm |
| `release-agent-extension.yml` | workflow_dispatch | ubuntu-latest | WXT/React extension → GitHub Release |
| `release-agent-sdk.yml` | workflow_dispatch (disabled) | ubuntu-latest | npm SDK publish (currently disabled) |
| `nightly-macos-build.yml` | cron + workflow_dispatch | self-hosted macOS | Nightly Chromium build for macOS arm64 |
| `test.yml` | PR + workflow_dispatch | ubuntu-latest | Bun test matrix on `packages/browseros-agent/**` |
| `code-quality.yml`, `audit.yml`, `eval-weekly.yml`, … | various | ubuntu-latest | Lint, audit, eval benchmarks |

### Added for the fork

| Workflow | Trigger | Runner | Role |
|---|---|---|---|
| `release-windows.yml` | workflow_dispatch + cron | **self-hosted Windows** | Chromium + bundled MCP from source → sign → package → R2 + GitHub Release → tag `browseros-windows-v<ver>` |
| `update-manifest.yml` | workflow_run + cron + workflow_dispatch | ubuntu-latest | Generate Omaha-4 + Sparkle appcast + JSON pointer; upload to R2 |
| `deploy-docs.yml` | push to main + workflow_dispatch | ubuntu-latest | Build Docusaurus site → deploy to GitHub Pages |

## Self-hosted Windows runner

| Property | Value |
|---|---|
| Label | `self-hosted, Windows, browseros-builder-windows` |
| Path | `C:\actions-runner` |
| Service | `actions.runner.bramburn-BrowserOS.browseros-builder-windows` |
| Required tools | VS2022 Community 14.44, Win10 SDK 10.0.26100, depot_tools, Rust, Bun 1.4+, Python 3.12 |
| Disk | 150 GB+ free at `C:\browersos-build\src` |

The runner is provisioned on this dev box. It auto-starts on boot.
See [`docs-site/docs/ci-runners.md`](../docs-site/docs/ci-runners.md)
for the full install / disable / verify procedures.

## R2 layout for browseros artifacts

```
R2_BUCKET/
  browseros/
    <version>/
      BrowserOS_v<version>_win-x64.exe     # the installer
    windows/
      update_check.xml                     # Omaha-4 response (in-browser updater)
      appcast.xml                          # Sparkle RSS (browseros-cli)
    latest.json                            # JSON version pointer
```

Cloudflare fronts R2 at `https://cdn.bramburn.com/browseros/...`.
The fork's Chromium build is patched at build time to point
`components/update_client` at
`https://cdn.bramburn.com/browseros/windows/update_check.xml`.

## Release flow

```
1. developer edits code in packages/browseros/chromium_patches/
2. opens PR; CI runs (test.yml + code-quality.yml on ubuntu)
3. merge to main
4. developer OR nightly cron triggers release-windows.yml
   ├─ bumps BROWSEROS_VERSION (via bump_version.py)
   ├─ pushes a "chore(release): build v<X>" PR
   ├─ runs bramburn-build.ps1 (self-hosted runner, 7-13 h)
   ├─ signs via SSL.com eSigner
   ├─ packages mini_installer.exe
   ├─ (optional) wraps with AIP if wrap_with_aip=true
   ├─ uploads .exe to R2 at browseros/<version>/...
   ├─ creates GitHub Release browseros-windows-v<version>
   └─ triggers update-manifest.yml (workflow_run)
5. update-manifest.yml fires on Ubuntu
   ├─ downloads the .exe from GitHub Releases
   ├─ computes SHA-256
   ├─ runs tools/release/generate_update_manifests.py
   ├─ uploads update_check.xml, appcast.xml, latest.json to R2
   └─ installed BrowserOS v<X> instances see the update on next poll
```

## Tag conventions

| Pattern | Meaning |
|---|---|
| `browseros-windows-v<semver>` | The fork's Windows Chromium build. |
| `browseros-cli-v<semver>` | Upstream-inherited CLI release. |
| `browseros-server-v<semver>` | Upstream-inherited MCP server release. |
| `agent-extension-v<semver>` | Upstream-inherited extension release. |

All tags are immutable. New releases get new tags. Yanking a release
requires deleting both the GitHub Release and the R2 artifacts.

## Required GitHub configuration

### Repository variables (`Settings → Secrets and variables → Actions → Variables`)

| Variable | Example | Used by |
|---|---|---|
| `BROWSEROS_REPO_PATH` | `C:\dev\BrowserOs` | Self-hosted runners |
| `BROWSEROS_CHROMIUM_SRC` | `C:\browersos-build\src` | Self-hosted runners |
| `BROWSEROS_NIGHTLY_REF` | `main` | Nightly macOS build |
| `FORK_R2_PREFIX` | `browseros` | Update-manifest R2 key prefix |
| `FORK_CDN_BASE` | `https://cdn.bramburn.com` | Public CDN base URL |
| `BROWSEROS_APP_ID` | `{B49B6F23-3D5C-4F7D-9F0F-1F5E5D4C4D9F}` | Stable Windows app GUID for Omaha-4 |

### Repository secrets

| Secret | Used by |
|---|---|
| `R2_ACCOUNT_ID` | R2 uploads (CLI + manifests) |
| `R2_ACCESS_KEY_ID` | R2 uploads |
| `R2_SECRET_ACCESS_KEY` | R2 uploads |
| `R2_BUCKET` | R2 uploads |
| `CODE_SIGN_TOOL_PATH` | `sign_windows` (SSL.com eSigner) |
| `ESIGNER_USERNAME` / `_PASSWORD` / `_TOTP_SECRET` | SSL.com eSigner |
| `POSTHOG_API_KEY` | CLI build (optional) |
| `NPM_TOKEN` | CLI npm publish |

## Manual fallback

If the self-hosted runner is offline and you need to publish:

```powershell
# 1. Run the build interactively
cd C:\dev\BrowserOs
$env:PYTHONIOENCODING = "utf-8"
& tools\bramburn-build.ps1 -StopAfterPhase 5

# 2. Compute the SHA-256
Get-FileHash .\packages\browseros\releases\<version>\BrowserOS_v<version>_win-x64.exe

# 3. Upload to R2
bun packages/build-tools/scripts/upload-to-r2.ts `
  --file .\packages\browseros\releases\<version>\BrowserOS_v<version>_win-x64.exe `
  --key "browseros/<version>/BrowserOS_v<version>_win-x64.exe" `
  --content-type application/octet-stream

# 4. Generate manifests
python tools/release/generate_update_manifests.py `
  --version <version> --sha256 <sha256> --size <bytes> `
  --app-id "{B49B6F23-...}" --r2-prefix browseros `
  --cdn-base https://cdn.bramburn.com --output-dir /tmp/out

# 5. Upload each manifest
foreach ($f in 'update_check.xml','appcast.xml','latest.json') {
  bun packages/build-tools/scripts/upload-to-r2.ts `
    --file "/tmp/out/$f" --key "browseros/windows/$f" `
    --content-type application/xml
}

# 6. Tag + GitHub release
git tag -a browseros-windows-v<version> -m "browseros-windows v<version>"
git push origin browseros-windows-v<version>
gh release create browseros-windows-v<version> `
  --title "BrowserOS Windows - v<version>" `
  --notes-file /tmp/release-notes.md `
  .\packages\browseros\releases\<version>\BrowserOS_v<version>_win-x64.exe
```

## Advance Installer Pro (optional v2 polish)

AIP lives at `C:\Program Files (x86)\Caphyon\Advanced Installer 23.9\`.
CLI: `bin\x86\AdvancedInstaller.com`. GUI: `bin\x86\advinst.exe`.

Use it as a **wrap layer** on top of `mini_installer.exe`, not as a
replacement. The base install (files, registry, services) still
comes from Chromium's `mini_installer`; AIP adds wizard pages,
branding, and prerequisites.

The release workflow exposes `wrap_with_aip: bool` (default `false`).
When `true` and a `browseros.aip` exists in `installer/`, the
workflow runs the AIP build step on the Windows runner.

See [`docs-site/docs/advance-installer.md`](../docs-site/docs/advance-installer.md)
for the full integration guide.

## Debugging

Run [`docs-site/docs/runbook.md`](../docs-site/docs/runbook.md) for
the on-call playbook: stuck gclient, offline runner, R2 403, unsigned
binary, bad version bump PR, Docusaurus build failure, version
yanking.