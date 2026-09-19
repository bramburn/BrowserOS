---
title: Runbook
description: Operations runbook — what to do when X breaks.
---

# Runbook

Common operations on the fork's CI / build / release pipelines. See
the raw [`AGENTS.md`](https://github.com/bramburn/BrowserOS/blob/main/AGENTS.md)
for the development-side runbook (architecture, build phases, etc.).

## "gclient sync is stuck"

**Symptom**: `fetch-chromium.cmd` wrapper alive but `src/` is empty,
log shows "STALL DETECTED".

**Diagnosis**:

```powershell
Get-Process | Where-Object { $_.Name -eq 'git' } | Select-Object Id, CPU, WS
Get-ChildItem C:\browersos-build\src\.git\index.lock -ErrorAction SilentlyContinue
```

**If `git` is consuming CPU** (e.g. >100 s) and `WS` is growing
(e.g. >500 MB): the fetch is healthy, just slow. Wait.

**If `git` is at 0 CPU and 0 WS growth**: the fetch has died.
Restart with `--verbose`:

```powershell
cd C:\browersos-build\src
git clone --verbose https://chromium.googlesource.com/chromium/src.git src.tmp
# If src.tmp completes, rename to src/.
```

## "Self-hosted runner is offline"

**Symptom**: `release-windows.yml` runs queue and never start.

**Diagnosis**:

```powershell
gh api /repos/bramburn/BrowserOS/actions/runners --jq '.runners[] | {name, status, busy}'
Get-Service "actions.runner.*" -ErrorAction SilentlyContinue
```

**Fix**:

```powershell
cd C:\actions-runner
.\svc.cmd start
```

## "R2 upload fails with 403"

**Symptom**: `bun scripts/build/upload-to-r2.ts` errors with
`AccessDenied`.

**Diagnosis**: the `R2_*` secrets in GitHub have been rotated or
revoked.

**Fix**: re-create the R2 API token in Cloudflare dashboard, update
the four `R2_*` GitHub secrets, re-run the workflow.

## "Workflow needs a new R2 prefix"

The default `FORK_R2_PREFIX=browseros`. If you fork the fork (or
want a separate bucket for testing):

1. Settings → Variables → Actions → `FORK_R2_PREFIX` = your prefix.
2. Re-run any workflow.

The prefix is used in:
- `update-manifest.yml` (manifest R2 keys)
- `release.md` docs (URL examples)
- `update-server.md` docs (URL examples)

## "Mini_installer.exe is unsigned"

**Symptom**: Windows SmartScreen blocks the installer on first run.

**Diagnosis**: `sign_windows` step failed. Check the workflow log
for SSL.com eSigner errors.

**Fix**:

1. Confirm `CODE_SIGN_TOOL_PATH`, `ESIGNER_USERNAME`,
   `ESIGNER_PASSWORD`, `ESIGNER_TOTP_SECRET` secrets are correct.
2. Re-run the `release-windows.yml` workflow.
3. The signature only applies to the rebuilt `.exe`, so users
   previously blocked will need to download the new version.

## "Bump version PR won't merge"

**Symptom**: `release-windows.yml` opens a PR like
`bot/windows-version-main-v0.1.0` but it doesn't auto-merge.

**Diagnosis**: usually a failed CI check. Check the PR's
`Checks` tab.

**Fix**: address the failing check or merge manually with
`gh pr merge --squash --auto`.

## "Docusaurus build fails on Windows runner"

**Symptom**: `deploy-docs.yml` errors in the `Install dependencies`
or `Build website` step.

**Diagnosis**: missing Node 20+ on the runner, or `npm ci` failed.

**Fix**:

1. Add `actions/setup-node@v4` with `node-version: 20` (already in
   the workflow).
2. Ensure `docs-site/package-lock.json` is committed.
3. If a peer dependency conflict surfaces, `npm install` once on
   this dev box, commit the updated `package-lock.json`, push.

## "We hit a release we need to yank"

**Symptom**: a published version is broken / leaked secrets /
mis-tagged.

**Diagnosis**: check the GitHub Releases page and R2 bucket contents.

**Fix**:

```bash
# Delete the GitHub release
gh release delete browseros-windows-v<badver>

# Delete the tag
git push --delete origin browseros-windows-v<badver>

# Delete the R2 artifacts (manual via Cloudflare dashboard)
# or via aws-cli (R2 is S3-compatible):
aws --endpoint-url https://<account>.r2.cloudflarestorage.com \
    s3 rm s3://<bucket>/browseros/<badver>/ --recursive

# Regenerate manifests (the manifest generator picks the highest tag)
gh workflow run update-manifest.yml
```

After yanking, the in-browser updater will stop offering the bad
version once the next update manifest is published.